#include <errno.h>
#include <string.h>
#include <stdlib.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <string>
#include <thread>
#include <vector>

#include "eaif_trc.h"
#include "eaif_utils.h"

using std::cerr;
using std::endl;

typedef struct {
	int dim[4];
} Dim4;

template<typename T>
Dim4 GetDim4(T odims, int index)
{
	Dim4 dim = {};
	dim.dim[0] = 0;
	dim.dim[1] = index / (odims[2] * odims[3]);
	int remain = index % (odims[2] * odims[3]);
	dim.dim[2] = remain / odims[3];
	dim.dim[3] = remain % odims[3];
	return dim;
}

namespace dataset {

int CreateDir(const char* dir_path)
{
	if (mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1)
	{
		if( errno == EEXIST )
			return 0;
	    cerr << "Cannot open dir " << dir_path << endl;
	    return -1;
	}
	return 0;
}

void Wider::GetValList(std::vector<std::string>& img_list_abs, std::vector<std::string>& pred_list)
{
	std::ifstream fin(data_list_file.c_str());
	if (fin.is_open()) {
		std::string s;
		while (fin >> s) {
			if ((int)s.find(".jpg") != -1) {
				img_list.push_back(s);
				img_list_abs.push_back(data_dir + "/images/" + s);
				s.replace(s.end()-3, s.end(), "txt");
				pred_list.push_back(dst_dir + "/" + s.c_str());
			}
		}
		fin.close();
		eaif_info_h("Total %d image from Widerface validation set\n", (int)img_list.size());
		return;
	}
	cerr << "Invalid list file " << data_list_file << endl;
	return;
}

void Fddb::GetValList(String2d& img_list_abs, std::vector<std::string>& pred_list)
{
	const int number_folds = 10;
	int total_img = 0;
	for (int i = 1; i < number_folds+1; ++i) {
		char filename[256] = {};
		sprintf(filename, data_fold_fmt, i);
		std::string file_name = data_dir + "/FDDB-folds" + filename;
		sprintf(filename, data_det_fold_fmt, i);
		std::string det_file_name = std::string(filename);
		std::ifstream fin(file_name.c_str());
		if (fin.is_open()) {
			std::vector<std::string> fold_img_list;
			std::vector<std::string> fold_img_list_abs;
			std::string s;
			while (fin >> s) {
				total_img++;
				//printf("%d/%d %s:%s\n", i, number_folds, s.c_str(), (data_dir + "/originalPics/" + s + ".jpg").c_str());
				fold_img_list.push_back(s);
				fold_img_list_abs.push_back(data_dir + "/originalPics/" + s + ".jpg");
			}
			fin.close();
			img_list_abs.push_back(fold_img_list_abs);
			img_list.push_back(fold_img_list);
			eaif_info_h("Total %d image from FDDB validation set fold:%d\n", (int)fold_img_list_abs.size(), i);
		} else {
			cerr << "Invalid list file " << file_name.c_str() << endl;
		}

		pred_list.push_back(dst_dir + det_file_name);
	}
	total_imgs_=total_img;
	eaif_info_h("Total %d image from FDDB validation in %d folds\n", total_img, number_folds);
	return;
}



} // dataset

namespace eaif {
namespace utils {

int ForceValidThreadNum(int& thread_num)
{
	const auto processor_count = std::thread::hardware_concurrency();
	if (thread_num == 0) {
		thread_num = processor_count;
	} else {
		thread_num = Clamp(thread_num, 1, (int)processor_count);
	}
	eaif_info_h("\nUsing #%d core%s for model inference\n", thread_num, (thread_num > 1) ? "s": "");
	return EAIF_SUCCESS;
}

} // utils
} // eaif

#ifdef USE_ARMNN

#include <arm_compute/runtime/Scheduler.h> // inference number of core

#include <armnn/Types.hpp>
#include <armnn/Tensor.hpp>
#include <armnn/TypesUtils.hpp>

#include <armnn/BackendId.hpp>
#include <armnn/IRuntime.hpp>
#include <armnn/Utils.hpp>
#include <armnn/BackendRegistry.hpp>

#include <armnnTfLiteParser/ITfLiteParser.hpp>

#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <boost/program_options.hpp>

namespace armnn {
namespace utils {

#if 0
template<typename typedTensorHandle>
void debugCallback(armnn::LayerGuid guid, unsigned int slotIndex, armnn::ITensorHandle* tensorHandle)
{
	static int cnt = 0;
	auto shape = tensorHandle->GetShape();
	auto len = shape.GetNumDimensions();
	auto ele = shape.GetNumElements();
	const float* map = tensorHandle->Map();
	dump(map, ele * sizeof(float), cnt);
	tensorHandle->Unmap();
}
#endif

void SetDebugMode(int val)
{
	armnn::ConfigureLogging(1, 0, static_cast<armnn::LogSeverity>(val));
}

void SetThreadNum(int thread_num)
{
	arm_compute::Scheduler::get().set_num_threads(thread_num);
}

int ArmnnModel::Invoke(void)
{
	if (set_profile) runtime->GetProfiler(networkId)->EnableProfiling(true);
	//if (set_profile) runtime->RegisterDebugCallback(networkId, debugCallback)
	runtime->EnqueueWorkload(networkId, input_tensors, output_tensors);
	if (set_profile) runtime->GetProfiler(networkId)->Print(std::cerr);
	return 0;
}

static EaifDataType GetModelInputType(armnnTfLiteParser::ITfLiteParserPtr& model)
{
	std::string input_name = model->GetSubgraphInputTensorNames(0)[0];
	std::vector<BindingPointInfo> inputBindings{model->GetNetworkInputBindingInfo(0, input_name)};

	auto datatype = inputBindings[0].second.GetDataType();
	switch (datatype) {
		case armnn::DataType::Float32: {
			return Eaif32F;
		}
		case armnn::DataType::QAsymmU8: {
			return Eaif8U;
		}
		case armnn::DataType::QSymmS8: {
			return Eaif8S;
		}
		default :{
			eaif_warn("Unsupport model datatype :%d %s, try to use EAIF_32F...\n", (int)datatype, eaif_GetDTypeString((int)datatype));
			return Eaif32F;
		}
	}
}

template<typename T>
void PrintOutput(std::vector<BindingPointInfo>& outputBindings, std::vector<T*>& outputData,
  int range[2])
{
	static const char *fmt[] = {"%d ", "%.4f "};
	int type = (outputBindings[0].second.GetDataType() == armnn::DataType::Float32) ? 1 : 0;
	int st = range[0];
	int ed = range[1];

	eaif_info_h("output: %s\n", "");
	for (size_t i = 0; i < outputBindings.size(); ++i) {
		armnn::TensorInfo oinfo = outputBindings[i].second;
		auto* outputs_addr = outputData[i];
		auto _dims = oinfo.GetShape();
		printf("output:[%d] Size:%d\n", i, _dims.GetNumDimensions());
		int dimJ = _dims[1];
		int total_ele = 1;
		printf("out[%d]: dim:[", i);
		for (size_t j = 0; j < _dims.GetNumDimensions(); ++j) {
			printf("%d,", _dims[j]);
			total_ele *= _dims[j];
		}
		printf("]\n");
		assert(st < total_ele);
		assert(ed <= total_ele);

		if (_dims.GetNumDimensions() == 4) {
			Dim4 dims_s = GetDim4(_dims, st);
			Dim4 dims_e = GetDim4(_dims, ed);
			printf("From ele:%d:[%d,%d,%d,%d] to %d:[%d,%d,%d,%d] : \n",
				st, dims_s.dim[0], dims_s.dim[1], dims_s.dim[2], dims_s.dim[3],
				ed, dims_e.dim[0], dims_e.dim[1], dims_e.dim[2], dims_e.dim[3]);
			for (int j = st; j < ed; ++j) {
				printf(fmt[type], outputs_addr[j]);
			}
		} else {
			for (int j = 0; j < dimJ; ++j)
				printf(fmt[type], outputs_addr[j]);
		}
		printf("\n");
	}
}

template<typename T, int D>
void PrintOutput(int output_dims[], std::vector<std::vector<T>>& outputData, int range[2])
{
	std::vector<BindingPointInfo> info;
	std::vector<T*> outs_addr;

	for (size_t i = 0; i < outputData.size(); ++i) {
		const uint32_t last_dim = outputData[i].size() / output_dims[0] / output_dims[1];
		const uint32_t dim[] = {1, (uint32_t)output_dims[0], (uint32_t)output_dims[1], last_dim};
		if (D == Eaif8U) {
			auto binding = BindingPointInfo{
				i,
				armnn::TensorInfo(armnn::TensorShape(4, dim), armnn::DataType::QAsymmU8, 0.0f, 0)
			};
			info.push_back(binding);
			outs_addr.push_back(outputData[i].data());
		} else {
			auto binding = BindingPointInfo{
				i,
				armnn::TensorInfo(armnn::TensorShape(4, dim), armnn::DataType::Float32, 0.0f, 0)
			};
			info.push_back(binding);
			outs_addr.push_back(outputData[i].data());
		}
	}
	PrintOutput(info, outs_addr, range);
}


void PrintOutput(ArmnnModel& model, int range[2])
{
	auto names = model.model->GetSubgraphOutputTensorNames(0);
	std::vector<BindingPointInfo> info;
	int type = GetModelInputType(model.model);

	if (type == Eaif8U) {
		std::vector<uint8_t*> data;

		for (size_t i = 0; i < names.size(); ++i) {
			info.push_back(model.model->GetNetworkOutputBindingInfo(0,names[i]));
			data.push_back((uint8_t*)model.output_tensors[i].second.GetMemoryArea());
		}
		PrintOutput(info, data, range);
	} else {
		std::vector<float*> data;

		for (size_t i = 0; i < names.size(); ++i) {
			info.push_back(model.model->GetNetworkOutputBindingInfo(0,names[i]));
			data.push_back((float*)model.output_tensors[i].second.GetMemoryArea());
		}
		PrintOutput(info, data, range);
	}
}

template<typename T>
void PrintOutput(std::vector<BindingPointInfo>& output_bindings, std::vector<T*>& output_data,
  int range[2], const char* img_file, const char* dst_dir_in, int flip)
{
	static const char *fmt[] = {"%d ", "%.4f "};
	int st = range[0];
	int ed = range[1];
	char ss[256];
	char bin_file[256];
	sprintf(ss, "%s", img_file);
	char *p = strrchr(ss, '/');
	if (!p) {
		p = ss;
	} else {
		p++;
	}
	if (flip) sprintf(bin_file, "%s/%s.flip.embedding", dst_dir_in, p);
	else      sprintf(bin_file, "%s/%s.embedding", dst_dir_in, p);
	FILE *fp = fopen(bin_file, "wb");
	printf("Write embedding to %s\n", bin_file);
	int type = (output_bindings[0].second.GetDataType() == armnn::DataType::Float32) ? 1 : 0;

	eaif_info_h("output: %s\n", "");
	for (size_t i = 0; i < output_bindings.size(); ++i) {
		armnn::TensorInfo oinfo = output_bindings[i].second;
		auto* outputs_addr = output_data[i];
		auto _dims = oinfo.GetShape();
		printf("output:[%d] Size:%d\n", i, _dims.GetNumDimensions());
		int dimJ = _dims[1];
		int total_ele = 1;
		printf("out[%d]: dim:[", i);
		for (size_t j = 0; j < _dims.GetNumDimensions(); ++j) {
			printf("%d,", _dims[j]);
			total_ele *= _dims[j];
		}
		printf("]\n");
		assert(st < total_ele);
		assert(ed <= total_ele);

		if (_dims.GetNumDimensions() == 4) {
			Dim4 dims_s = GetDim4(_dims, st);
			Dim4 dims_e = GetDim4(_dims, ed);
			printf("From ele:%d:[%d,%d,%d,%d] to %d:[%d,%d,%d,%d] : \n",
				st, dims_s.dim[0], dims_s.dim[1], dims_s.dim[2], dims_s.dim[3],
				ed, dims_e.dim[0], dims_e.dim[1], dims_e.dim[2], dims_e.dim[3]);
			for (int j = st; j < ed; ++j) {
				printf(fmt[type], outputs_addr[j]);
			}
		} else {
			for (int j = 0; j < dimJ; ++j)
				printf(fmt[type], outputs_addr[j]);
			fwrite(outputs_addr, sizeof(float), dimJ, fp);
		}
		printf("\n");
	}
	fclose(fp);
}

template<typename T, int D>
void PrintOutput(int output_dims[], std::vector<std::vector<T>>& output_data, int range[2], const char* img_file, const char* dst_dir_in, int flip)
{
	std::vector<BindingPointInfo> info;
	std::vector<T*> outs_addr;

	for (size_t i = 0; i < output_data.size(); ++i) {
		const uint32_t last_dim = output_data[i].size() / output_dims[0] / output_dims[1];
		const uint32_t dim[] = {1, (uint32_t)output_dims[0], (uint32_t)output_dims[1], last_dim};
		if (D == Eaif8U) {
			auto binding = BindingPointInfo{
				i,
				armnn::TensorInfo(armnn::TensorShape(4, dim), armnn::DataType::QAsymmU8, 0.0f, 0)
			};
			info.push_back(binding);
			outs_addr.push_back(output_data[i].data());
		} else {
			auto binding = BindingPointInfo{
				i,
				armnn::TensorInfo(armnn::TensorShape(4, dim), armnn::DataType::Float32, 0.0f, 0)
			};
			info.push_back(binding);
			outs_addr.push_back(output_data[i].data());
		}
	}
	PrintOutput(info, outs_addr, range, img_file, dst_dir_in, flip);
}


void PrintOutput(ArmnnModel& model, int range[2], const char* img_file, const char* dst_dir_in, int flip)
{
	auto names = model.model->GetSubgraphOutputTensorNames(0);
	std::vector<BindingPointInfo> info;
	int type = GetModelInputType(model.model);

	if (type == Eaif8U) {
		std::vector<uint8_t*> data;

		for (size_t i = 0; i < names.size(); ++i) {
			info.push_back(model.model->GetNetworkOutputBindingInfo(0,names[i]));
			data.push_back((uint8_t*)model.output_tensors[i].second.GetMemoryArea());
		}
		PrintOutput(info, data, range, img_file, dst_dir_in, flip);
	} else {
		std::vector<float*> data;

		for (size_t i = 0; i < names.size(); ++i) {
			info.push_back(model.model->GetNetworkOutputBindingInfo(0,names[i]));
			data.push_back((float*)model.output_tensors[i].second.GetMemoryArea());
		}
		PrintOutput(info, data, range, img_file, dst_dir_in, flip);
	}
}

template<typename T>
armnn::InputTensors MakeInputTensors(const std::vector<armnn::BindingPointInfo>& input_bindings,
                                     std::vector<boost::variant<std::vector<T>>>& input_data_containers)
{
	T* data = boost::get<std::vector<T>>(input_data_containers[0]).data();
	armnn::InputTensors input_tensors{
		{input_bindings[0].first, armnn::ConstTensor(input_bindings[0].second, data)}
	};
    return input_tensors;
}

template<typename T>
armnn::OutputTensors MakeOutputTensors(std::vector<armnn::BindingPointInfo>& output_bindings,
    std::vector<boost::variant<std::vector<T>>>& output_data_containers)
{
    const size_t num_outputs = output_bindings.size();
    armnn::OutputTensors output_tensors(num_outputs);

    for (size_t i = 0; i < num_outputs; i++)
    {
        const armnn::BindingPointInfo& output_binding = output_bindings[i];
        boost::variant<std::vector<T>>& output_data = output_data_containers[i];
        T* data = boost::get<std::vector<T>>(output_data).data();
        output_tensors[i] = std::make_pair(
        	output_binding.first,
        	armnn::Tensor(output_binding.second, data)
        );
    }
    return output_tensors;
}

void InitModelIO(ArmnnModel& model, armnn::InputTensors& input_tensors, armnn::OutputTensors& output_tensors)
{

	std::vector<std::string> input_names = model.model->GetSubgraphInputTensorNames(0);
	std::vector<std::string> output_names= model.model->GetSubgraphOutputTensorNames(0);

	std::vector<BindingPointInfo> input_bindings{model.model->GetNetworkInputBindingInfo(0, input_names[0])};
	std::vector<BindingPointInfo> output_bindings(output_names.size());
	auto input_tensor_shape = input_bindings[0].second.GetShape();
	int h = input_tensor_shape[1]; 	int w = input_tensor_shape[2]; 	int c = input_tensor_shape[3];
	int input_size = h * w * c;

	int type = model.type;

	if (type == Eaif8U || type == Eaif8S) {

		model.output_data.resize(output_names.size());

		using TContainer = boost::variant<std::vector<uint8_t>>;
		model.input_data.push_back(TContainer{std::vector<uint8_t>(input_size)});

		for (int i = 0; i < (int)output_names.size(); ++i) {
			output_bindings[i] = model.model->GetNetworkOutputBindingInfo(0, output_names[i]);
			auto shape = output_bindings[i].second.GetShape();
			int size = 1;
			for (int j = 0; j < (int)shape.GetNumDimensions(); ++j)
				size *= shape[j];
			ContainerVisitor visitor;
			visitor.size = size;
			boost::apply_visitor(visitor, model.output_data[i]);
		}

		input_tensors = MakeInputTensors(input_bindings, model.input_data);
		output_tensors = MakeOutputTensors(output_bindings, model.output_data);

	} else {

		model.output_dataf.resize(output_names.size());

		using TContainer = boost::variant<std::vector<float>>;
		model.input_dataf.push_back(TContainer{std::vector<float>(input_size)});

		for (int i = 0; i < (int)output_names.size(); ++i) {
			output_bindings[i] = model.model->GetNetworkOutputBindingInfo(0, output_names[i]);
			auto shape = output_bindings[i].second.GetShape();
			int size = 1;
			for (int j = 0; j < (int)shape.GetNumDimensions(); ++j)
				size *= shape[j];
			ContainerVisitor visitor;
			visitor.size = size;
			boost::apply_visitor(visitor, model.output_dataf[i]);
		}

		input_tensors = MakeInputTensors(input_bindings, model.input_dataf);
		output_tensors = MakeOutputTensors(output_bindings, model.output_dataf);
	}

}

int LoadModels(const char* tflite_model,
	armnn::INetworkPtr& network,
	armnnTfLiteParser::ITfLiteParserPtr& model,
	armnn::NetworkId& networkId,
	armnn::IRuntimePtr& runtime,
	std::vector<armnn::BackendId> compute_device,
	EaifDataType& model_type)
{
	eaif_info_h("Create network from file %s!\n", tflite_model);
	using IParser = armnnTfLiteParser::ITfLiteParser;
	model = IParser::Create();
	network = model->CreateNetworkFromBinaryFile(tflite_model);

	eaif_info_h("Optimize network to runtime!\n");
	armnn::IRuntime::CreationOptions options;
	runtime = armnn::IRuntime::Create(options);
	IOptimizedNetworkPtr optimized_net = armnn::Optimize(*network, compute_device, runtime->GetDeviceSpec());

	// Load the optimized network onto the runtime device
	eaif_info_h("Loading network to runtime!\n");
	runtime->LoadNetwork(networkId, std::move(optimized_net));

	model_type = GetModelInputType(model);
	eaif_info_h("Fininsh onload network input type : %s!\n",  eaif_GetDTypeString(model_type));
	return 0;
}

int LoadModels(const char* tflite_model,
	ArmnnModel& model,
	std::vector<armnn::BackendId> compute_device)
{
    armnn::utils::LoadModels(tflite_model,
		model.network,
		model.model,
		model.networkId,
		model.runtime,
		compute_device,
		model.type);
		return 0;
}

int LoadModels(const char* tflite_model,
	ArmnnModel& model)
{
	std::vector<armnn::BackendId> backend_device;
	switch (model.cpu_infer_type) {
		case armnn::utils::CpuAcc:
			backend_device.push_back(armnn::Compute::CpuAcc);
			break;
		case armnn::utils::CpuRef:
			backend_device.push_back(armnn::Compute::CpuRef);
			break;
		default:
			backend_device.push_back(armnn::Compute::CpuAcc);
			break;
	}
	return armnn::utils::LoadModels(tflite_model,
		model.network,
		model.model,
		model.networkId,
		model.runtime,
		backend_device,
		model.type);
}

void GetQuantInfo(const armnnTfLiteParser::ITfLiteParserPtr& model, std::vector<QuantInfo>& info)
{
	std::vector<std::string> output_names= model->GetSubgraphOutputTensorNames(0);
 
 	if (info.size() > 0) {
		if (output_names.size() != info.size()) {
			eaif_err("number of output(model:%d) is not consistent to input quant info dim(%d)!\n",
				output_names.size(), info.size());
			exit(0);
		}

		for (int i = 0; i < (int)output_names.size(); ++i) {
			auto output_binding = model->GetNetworkOutputBindingInfo(0, output_names[i]);
			armnn::TensorInfo& p = output_binding.second;
			info[i].set(p.GetQuantizationOffset(), p.GetQuantizationScale());
		}
	} else {

		for (int i = 0; i < (int)output_names.size(); ++i) {
			auto output_binding = model->GetNetworkOutputBindingInfo(0, output_names[i]);
			armnn::TensorInfo& p = output_binding.second;
			info.push_back(QuantInfo(p.GetQuantizationOffset(), p.GetQuantizationScale()));
			eaif_info_l("quant info[%d]: scale: %.4f zero:%d\n", i, info[i].m_scale, info[i].m_zero);
		}
	}
}

} // utils 
} // armnn

#endif

#ifdef USE_TFLITE
#include "eaif_utils.h"

void tflite::utils::PrintOutput(const std::unique_ptr<tflite::Interpreter>& interpreter, int range[2])
{
	PrintOutput(interpreter.get(), range);
}

void tflite::utils::PrintOutput(const std::unique_ptr<tflite::Interpreter>& interpreter, int range[2], const char* img_file, const char* dst_dir_in, int flip)
{
	// TBD
	PrintOutput(interpreter.get(), range);
}

static void PrintOutputInt(const tflite::Interpreter *interpreter, int range[2])
{
	static const char *fmt[] = {"%d ", "%.3f "};

	auto outputs = interpreter->outputs();
	int idx = 0;
	int st = range[0];
	int ed = range[1];
	for (int i = 0; i < (int)outputs.size(); ++i) {
		TfLiteIntArray* _dims = interpreter->tensor(outputs[i])->dims;
		printf("Size:%d\n", _dims->size);
		int dimJ = _dims->data[1];
		int total_ele = 1;
		printf("out[%d]: dim:[", i);
		for (int j = 0; j < _dims->size; ++j) {
			printf("%d,", _dims->data[j]);
			total_ele *= _dims->data[j];
		}
		printf("]\n");
		assert(st < total_ele);
		assert(ed <= total_ele);
		auto *outs = interpreter->typed_tensor<uint8_t>(outputs[i]);
		if (_dims->size == 4) {
			//int num_channel = _dims->data[_dims->size-1];
			Dim4 dims_s = GetDim4(_dims->data, range[0]);
			Dim4 dims_e = GetDim4(_dims->data, range[1]);
			printf("From ele:%d:[%d,%d,%d,%d] to %d:[%d,%d,%d,%d] :\n",
				st, dims_s.dim[0], dims_s.dim[1], dims_s.dim[2], dims_s.dim[3],
				ed, dims_e.dim[0], dims_e.dim[1], dims_e.dim[2], dims_e.dim[3]);
			for (int j = st; j < ed; ++j) {
				printf(fmt[idx], outs[j]);
			}
			//printf("\n");
			//printf("[ch:%d col %d] ",channel, _row);
			//channel = 1;
			//for (int j = 0; j < dimJ; ++j) {
			//	printf(fmt[idx], outs[ _row * dimJ + j * num_channel + channel]);
			//}
		} else {
			for (int j = 0; j < dimJ; ++j) {
				printf(fmt[idx], outs[j]);
			}
		}
		printf("\n");
	}
}

static void PrintOutputFloat(const tflite::Interpreter *interpreter, int range[2])
{
	static const char *fmt[] = {"%d ", "%.3f "};

	auto outputs = interpreter->outputs();
	int idx = 1;
	int st = range[0];
	int ed = range[1];
	for (int i = 0; i < (int)outputs.size(); ++i) {
		TfLiteIntArray* _dims = interpreter->tensor(outputs[i])->dims;
		printf("Size:%d\n", _dims->size);
		int dimJ = _dims->data[1];
		int total_ele = 1;
		printf("out[%d]: dim:[", i);
		for (int j = 0; j < _dims->size; ++j) {
			printf("%d,", _dims->data[j]);
			total_ele *= _dims->data[j];
		}
		printf("]\n");
		assert(st < total_ele);
		assert(ed <= total_ele);

		auto *outs = interpreter->typed_tensor<float>(outputs[i]);
		if (_dims->size == 4) {
			//int num_channel = _dims->data[_dims->size-1];
			Dim4 dims_s = GetDim4(_dims->data, range[0]);
			Dim4 dims_e = GetDim4(_dims->data, range[1]);
			printf("from ele:%d:[%d,%d,%d,%d] to %d:[%d,%d,%d,%d] : \n",
				st, dims_s.dim[0], dims_s.dim[1], dims_s.dim[2], dims_s.dim[3],
				ed, dims_e.dim[0], dims_e.dim[1], dims_e.dim[2], dims_e.dim[3]);
			for (int j = st; j < ed; ++j) {
				printf(fmt[idx], outs[j]);
			}
			//pr
		} else {
			if (_dims->data[0] != 1) {
				for (int j = st; j < ed; ++j) {
					fprintf(stderr, fmt[idx], outs[j]);
				}
			} else {
				for (int j = 0; j < dimJ; ++j) {
					fprintf(stderr, fmt[idx], outs[j]);
				}
			}
		}
		printf("\n");
	}
}

void tflite::utils::PrintOutput(const tflite::Interpreter *interpreter, int range[2])
{
	auto outputs = interpreter->outputs();
	int idx = (interpreter->tensor(outputs[0])->type == kTfLiteUInt8)? 0 : 1;

	if (idx == 0)
		PrintOutputInt(interpreter, range);
	else
		PrintOutputFloat(interpreter, range);
}

EaifDataType tflite::utils::GetDataType(const tflite::Interpreter *interpreter, int tensor_idx)
{
	switch (interpreter->tensor(tensor_idx)->type) {
		case kTfLiteUInt8: {
			return Eaif8U;
			break;
		} case kTfLiteInt8: {
			return Eaif8S;
			break;
		} case kTfLiteFloat32: {
			return Eaif32F;
			break;
		} default: {
			eaif_err("Lite inference is not implemented for this datatype yet %d!\n", interpreter->tensor(tensor_idx)->type);
			return EaifUnknownType;
		}
	}
}

int tflite::utils::LoadModels(const char* tflite_model,
	std::unique_ptr<tflite::Interpreter>& interpreter,
	std::unique_ptr<tflite::FlatBufferModel>& model)
{
	model =	tflite::FlatBufferModel::BuildFromFile(tflite_model);
	if (model == nullptr) {
		eaif_err("Cannot find %s.\n", tflite_model);
		interpreter.reset();
		return -1;
	}

	// Build the interpreter
	tflite::ops::builtin::BuiltinOpResolver resolver;
	tflite::InterpreterBuilder builder(*model, resolver);
	builder(&interpreter);
	if (interpreter == nullptr) {
		eaif_err("Cannot find %s.\n", tflite_model);
		return -1;
	}
	return 0;
}

void tflite::utils::GetQuantInfo(const std::unique_ptr<tflite::Interpreter> &model, QuantInfo *info, int num_output)
{
	std::vector<int> outputs = model->outputs();
	if (outputs.size() != (uint32_t) num_output) {
		eaif_err("number of output is not correct!\n");
	}
	for (uint32_t i = 0; i < outputs.size() ; ++i) {
		const TfLiteQuantizationParams *p = &model->tensor(outputs[i])->params;
		info[i].set(p->zero_point, p->scale);
		eaif_info_l("out:%d float: %.9f, zero: %d\n", i, info[i].m_scale, info[i].m_zero);
	}
}

template <class T>
void tflite::utils::Resize(T* out, uint8_t* in, int image_height, int image_width,
						int image_channels, int wanted_height, int wanted_width,
						int wanted_channels, int input_floating) {
	int number_of_pixels = image_height * image_width * image_channels;
	std::unique_ptr<Interpreter> interpreter(new Interpreter);

	int base_index = 0;

	// two inputs: input and new_sizes
	interpreter->AddTensors(2, &base_index);
	// one output
	interpreter->AddTensors(1, &base_index);
	// set input and output tensors
	interpreter->SetInputs({0, 1});
	interpreter->SetOutputs({2});

	// set parameters of tensors
	TfLiteQuantizationParams quant;
	interpreter->SetTensorParametersReadWrite(
			0, kTfLiteUInt8, "input",
			{1, image_height, image_width, image_channels}, quant);
	interpreter->SetTensorParametersReadWrite(1, kTfLiteInt32, "new_size", {2},
																						quant);
	interpreter->SetTensorParametersReadWrite(
			2, kTfLiteUInt8, "output",
			{1, wanted_height, wanted_width, wanted_channels}, quant);

	ops::builtin::BuiltinOpResolver resolver;
	const TfLiteRegistration* resize_op =
			resolver.FindOp(BuiltinOperator_RESIZE_BILINEAR, 1);
	auto* params = reinterpret_cast<TfLiteResizeBilinearParams*>(
			malloc(sizeof(TfLiteResizeBilinearParams)));
	params->align_corners = false;
	interpreter->AddNodeWithParameters({0, 1}, {2}, nullptr, 0, params, resize_op,
																		 nullptr);

	interpreter->AllocateTensors();

	// fill input image
	// in[] are integers, cannot do memcpy() directly
	auto *input = interpreter->typed_tensor<T>(0);
	memcpy(input, in, number_of_pixels * 1);

	// fill new_sizes
	interpreter->typed_tensor<int>(1)[0] = wanted_height;
	interpreter->typed_tensor<int>(1)[1] = wanted_width;

	interpreter->Invoke();

	auto output = interpreter->typed_tensor<T>(2);
	auto output_number_of_pixels = wanted_height * wanted_width * wanted_channels;

	if (input_floating) { 
		for (int i = 0; i < output_number_of_pixels; i++)
				 out[i] = (output[i] - 127) * 0.0078125;
				 //out[i] = (output[i] - s->input_mean) / s->input_std;
	} else {
		memcpy((T *)out, output, sizeof(T)*output_number_of_pixels);
	}
}

#endif /* !USE_TFLITE */

#ifdef USE_MPI
extern "C" {
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_index.h"
#include "mpi_sys.h"
}
#endif /* !USE_MPI */

mpi::utils::MpiCtx::MpiCtx(void)
{
#ifdef USE_MPI
	MPI_SYS_init();
	MPI_initBitStreamSystem();
	snapshot_chn = MPI_ENC_CHN(3);
	bchn = MPI_createBitStreamChn(snapshot_chn);
#endif
}

mpi::utils::MpiCtx::~MpiCtx(void)
{
#ifdef USE_MPI
	MPI_destroyBitStreamChn(bchn);
	MPI_exitBitStreamSystem();
	MPI_SYS_exit();
#endif
}

int mpi::utils::GetServiceJpeg(uint8_t **data, int target_chn)
{
	int size = 0;
#ifdef USE_MPI
#define WAIT_FOREVER (-1)
	MPI_ECHN echn = MPI_ENC_CHN((uint8_t)target_chn);
	int total_size = 0;
	uint32_t err = 0;
	MPI_STREAM_PARAMS_S stream_param;
	eaif_info_l("Taking snapshot of encoder channel (c=%d)\n", echn.chn);
	err = MPI_ENC_getChnFrame(echn, &stream_param, WAIT_FOREVER);
	if (err != MPI_SUCCESS) {
		MPI_ENC_releaseChnFrame(echn, &stream_param);
		printf("[EAIF SERVER] WARN:: Failed to take snapshot.\n");
		return 0;
	}
	for (size_t i = 0; i < stream_param.seg_cnt; i++) {
		total_size += stream_param.seg[i].size;
	}
	*data = new uint8_t[total_size];
	for (size_t i = 0; i < stream_param.seg_cnt; i++) {
		memcpy(&((*data)[size]), stream_param.seg[i].uaddr, stream_param.seg[i].size);
		size += stream_param.seg[i].size;
	}
	eaif_info_l("seg_cnt = %d size = %d \n", stream_param.seg_cnt, size);
	MPI_ENC_releaseChnFrame(echn, &stream_param);
	return size;
#else /* USE_MPI */
	eaif_warn("%s is disabled!\n", __func__);
	return size;
#endif /* !USE_MPI */
}


int mpi::utils::MpiCtx::GetServiceY(int video_window, int height, int width, uint8_t **data)
{
#ifdef USE_MPI
#define EAIF_GETWINFRAME_TRY_MAX 2
	info.size = 0;
	snapshot_win.value = video_window;
	info.height = height;
	info.width = width;
	info.type = MPI_SNAPSHOT_Y;
	eaif_info_l("Taking snapshot of video window (d:c:w=%d:%d:%d)\n",
		snapshot_win.dev, snapshot_win.chn, snapshot_win.win);

	int err = 0;
	int repeat = 0;
	int try_time = 0;
	do {
		repeat = 0;
		err = MPI_DEV_getWinFrame(snapshot_win, &info, 1200);
		if (err == -EAGAIN) {
			MPI_DEV_releaseWinFrame(snapshot_win, &info);
			eaif_warn("GetWinFrame Timeout retry ... #%d/%d\n", try_time+1, EAIF_GETWINFRAME_TRY_MAX);
			if (try_time == EAIF_GETWINFRAME_TRY_MAX) {
				eaif_warn("MPI_DEV_getWinFrame is too Busy! exit request\n");
				break;
			}
			try_time += 1;
			repeat = 1;
		} else if (err == -ENODATA) {
			eaif_warn("No Data from MPI!\n");
			break;
		}
	} while ( repeat );

	if (err != MPI_SUCCESS) {
		printf("Failed to take Y snapshot.\n");
		goto end;
	}
	*data = (uint8_t*)info.uaddr;
	eaif_info_l("P5\n%d %d\n255\n", info.width, info.height);
end:
	return info.size;
#else /* USE_MPI */
	eaif_warn("%s is disabled!\n", __func__);
	return 0;
#endif /* !USE_MPI */
}

int mpi::utils::MpiCtx::GetServiceRgb(int video_window, int height, int width, uint8_t **data)
{
#ifdef USE_MPI
	info.size = 0;
	uint32_t err;
	snapshot_win.value = video_window;
	info.height = height;
	info.width = width;
	info.type = MPI_SNAPSHOT_RGB;
	eaif_info_l("Taking snapshot of video window (d:c:w=%d:%d:%d)\n",
		snapshot_win.dev, snapshot_win.chn, snapshot_win.win);
	err = MPI_DEV_getWinFrame(snapshot_win, &info, 1200);
	if (err != MPI_SUCCESS) {
		printf("Failed to take RGB snapshot.\n");
		goto end;
	}
	*data = (uint8_t*)info.uaddr;
	eaif_info_l("P5\n%d %d\n255\n", info.width, info.height);
end:
	return info.size;
#else /* USE_MPI */
	eaif_warn("%s is disabled!\n", __func__);
	return 0;
#endif /* !USE_MPI */
}

int mpi::utils::MpiCtx::GetServiceYuv(int video_window, int height, int width, uint8_t **data)
{
#ifdef USE_MPI
	info.size = 0;
	uint32_t err;
	snapshot_win.value = video_window;
	info.height = height;
	info.width = width;
	info.type = MPI_SNAPSHOT_NV12;
	eaif_info_l("Taking snapshot of video window (d:c:w=%d:%d:%d)\n",
		snapshot_win.dev, snapshot_win.chn, snapshot_win.win);
	err = MPI_DEV_getWinFrame(snapshot_win, &info, 1200);
	if (err != MPI_SUCCESS) {
		printf("Failed to take YUV snapshot.\n");
		goto end;
	}
	*data = (uint8_t*)info.uaddr;
	eaif_info_l("P5\n%d %d\n255\n", info.width, info.height);
end:
	return info.size;
#else /* USE_MPI */
	eaif_warn("%s is disabled!\n", __func__);
	return 0;
#endif /* !USE_MPI */
}

int mpi::utils::MpiCtx::ReleaseFrameIfAny(void)
{
#ifdef USE_MPI
	if (info.uaddr) {
		int err = MPI_DEV_releaseWinFrame(snapshot_win, &info);
		if (err != MPI_SUCCESS) {
			printf("Failed to take snapshot.\n");
			return err;
		}
		memset(&info, 0, sizeof(MPI_VIDEO_FRAME_INFO_S));
	}
	return 0;
#else /* USE_MPI */
	eaif_warn("%s is disabled!\n", __func__);
	return 0;
#endif /* !USE_MPI */
}
