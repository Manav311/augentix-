#include <iostream>
#include <cstdio>
#include <cstdint>
#include <memory>


#ifdef USE_TFLITE
#include "lite_mtcnn.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"
using namespace tflite;
#endif /* USE_TFLITE */

#ifdef USE_ARMNN
#include <armnn/BackendId.hpp>
#include <armnn/IRuntime.hpp>
#include <armnn/Utils.hpp>
#include <armnn/BackendRegistry.hpp>

#include <armnnTfLiteParser/ITfLiteParser.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/variant.hpp>
//#include "eaif_armnn_utils.h"
using namespace armnn;
#endif /* USE_ARMNN */

#include "eaif_engine_demo.h"
#include "eaif_common.h"
#include "eaif_trc.h"
#include "eaif_image.h"
#include "eaif_utils.h"

static struct timespec start;

using namespace eaif::image;
using namespace std;

#define MINIMAL_CHECK(x)                              \
	if (!(x)) {                                                \
	fprintf(stderr, "Error at %s:%d\n", __FILE__, __LINE__); \
	exit(1);                                                 \
	}

int eaif::EngineDemo::RunMinimumArmnn(const char* tflite_model, const char *img_file)
{
#ifdef USE_ARMNN
	// load network
	eaif_info_h("ARMNN inference!%s\n","");
	armnn::utils::ArmnnModel infer_model;
	//armnn::utils::LoadModels(tflite_model, infer_model);
	using IParser = armnnTfLiteParser::ITfLiteParser;
	infer_model.model = IParser::Create();
	TIMER_FUNC("Load model ", infer_model.network = infer_model.model->CreateNetworkFromBinaryFile(tflite_model));

	std::vector<armnn::BackendId> cpu_infer;
	if (m_cpu_infer == 0)
		cpu_infer.push_back(armnn::Compute::CpuAcc);
	else if (m_cpu_infer == 1)
		cpu_infer.push_back(armnn::Compute::CpuRef);
	armnn::IRuntime::CreationOptions options;
	infer_model.runtime = armnn::IRuntime::Create(options);
	TIC(start);
	IOptimizedNetworkPtr optimizedNet = armnn::Optimize(
		*infer_model.network,
		cpu_infer,
		infer_model.runtime->GetDeviceSpec()
	);
	TOC("Optimize", start);
	// Load the optimized network onto the runtime device
	TIC(start);
	infer_model.runtime->LoadNetwork(infer_model.networkId, std::move(optimizedNet));
	TOC("Onload runtime ", start);
	// get network info
	std::vector<std::string> input_names = infer_model.model->GetSubgraphInputTensorNames(0);
	std::vector<std::string> output_names= infer_model.model->GetSubgraphOutputTensorNames(0);

	std::vector<BindingPointInfo> input_bindings(input_names.size());
	std::vector<BindingPointInfo> output_bindings(output_names.size());

	std::vector<QuantInfo> quant(output_names.size());

	eaif_info_l("input tensor info:%s\n","");
	for (int i = 0; i < (int)input_names.size(); ++i) {
		std::cout << input_names[i] << "\ndims:[";
		input_bindings[i] = infer_model.model->GetNetworkInputBindingInfo(0, input_names[i]);
		for (int j = 0; j < (int)input_bindings[i].second.GetShape().GetNumDimensions(); ++j) {
			std::cout << input_bindings[i].second.GetShape()[j] << " ";
		}
		std::cout << "]\n";
	}

	auto type = (input_bindings[0].second.GetDataType() == armnn::DataType::Float32 ) ? Eaif32F : Eaif8U;
	infer_model.type = type;

	auto input_tensor_shape = input_bindings[0].second.GetShape();
	int h = input_tensor_shape[1];
	int w = input_tensor_shape[2];

	if (type == Eaif8U || type == Eaif8S)
		armnn::utils::GetQuantInfo(infer_model.model, quant);

	this->SetNumThreads();
	eaif_info_h("Using %d cpu !\n", m_nthread);
	//int c = inputTensorShape[3];
	eaif_info_l("network type: %s\n", (type == Eaif8U)? "uint8" : "float32");
	eaif_info_l("output tensor info:%s\n","");
	for (int i = 0; i < (int)output_names.size(); ++i) {
		std::cout << output_names[i] << "\ndims:[";
		output_bindings[i] = infer_model.model->GetNetworkOutputBindingInfo(0, output_names[i]);
		for (int j = 0; j < (int)output_bindings[i].second.GetShape().GetNumDimensions(); ++j)
			std::cout << output_bindings[i].second.GetShape()[j] << " ";
		std::cout << "] zero: " << quant[i].GetZero() << " scale: " << quant[i].GetScale();
		std::cout << "\n";
	}

	armnn::InputTensors* input_tensors = &infer_model.input_tensors;
	armnn::OutputTensors* output_tensors = &infer_model.output_tensors;
	TIMER_FUNC("MakeModelIO ", armnn::utils::InitModelIO(infer_model, *input_tensors, *output_tensors));

	uint8_t* _iinput = nullptr;
	float* _finput = nullptr;
	if (type == Eaif8U)
		_iinput = boost::get<std::vector<uint8_t>>(infer_model.input_data[0]).data();
	else
		_finput = boost::get<std::vector<float>>(infer_model.input_dataf[0]).data();

	if (m_debug)
	    infer_model.runtime->GetProfiler(infer_model.networkId)->EnableProfiling(true);
	for (int i = 0; i < m_iter; i++) {
		if (type == Eaif8U || type == Eaif8S) {
			WImage img(h, w, Eaif8UC3, _iinput);
			TIMER_FUNC("Read image", eaif::image::Imread(img_file, img, m_w, m_h));
			TIMER_FUNC("Inference", (infer_model.runtime->EnqueueWorkload(infer_model.networkId, *input_tensors, *output_tensors)));
		} else {
			int wanted_width = m_w;
			int wanted_height = m_h;
			WImage img(wanted_height, wanted_width, Eaif32FC3, _finput);
			WImage raw;
			eaif::image::Imread(img_file, raw);
			TIMER_FUNC("Process WImage", eaif::image::ImresizeNorm(raw, img, wanted_width, wanted_height, m_zero, m_scale));
			TIMER_FUNC("Inference", (infer_model.runtime->EnqueueWorkload(infer_model.networkId, *input_tensors, *output_tensors)));
		}
	}
	if (m_debug)
	    infer_model.runtime->GetProfiler(infer_model.networkId)->Print(std::cerr);
	
	if (type == Eaif8U || type == Eaif8S) {
		std::vector<uint8_t*> data(output_bindings.size());

		for (size_t i = 0; i < output_bindings.size(); ++i) {
			data[i] = boost::get<std::vector<uint8_t>>(infer_model.output_data[i]).data();
			if (m_debug == 2) {
				eaif_info_h("dump size: %d [%u]\n", output_bindings[i].second.GetNumElements(), i);
				Dump(data[i], output_bindings[i].second.GetNumElements(), i);
			}
		}

		armnn::utils::PrintOutput(output_bindings, data, m_range);

	} else {
		std::vector<float*> data(output_bindings.size());

		for (size_t i = 0; i < output_bindings.size(); ++i) {
			data[i] = boost::get<std::vector<float>>(infer_model.output_dataf[i]).data();
			if (m_debug == 2) {
				eaif_info_h("dump size: %d [%u]\n", output_bindings[i].second.GetNumElements() * sizeof(float), i);
				Dump((const uint8_t *)data[i], output_bindings[i].second.GetNumElements() * sizeof(float), i);
			}
		}
		armnn::utils::PrintOutput(output_bindings, data, m_range);
	}
#endif // USE_ARMNN
	return 0;

}

int eaif::EngineDemo::RunMinimumTflite(const char* tflite_model, const char *img_file)
{
#ifdef USE_TFLITE

	// Load model
	std::unique_ptr<tflite::FlatBufferModel> model;
	std::unique_ptr<Interpreter> interpreter;
	TIMER_FUNC("Load Model", tflite::utils::LoadModels(tflite_model, interpreter, model));

	MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
	printf("=== Pre-invoke Interpreter State ===\n");
	if (m_verbose) tflite::PrintInterpreterState(interpreter.get());

	/* Get input/output tensor index */
	const std::vector<int> inputs = interpreter->inputs();
	const std::vector<int> outputs = interpreter->outputs();

	auto etype = interpreter->tensor(inputs[0])->type;
	auto type = (etype == kTfLiteUInt8) ? Eaif8U : (etype == kTfLiteInt8) ? Eaif8S : Eaif32F;
	int input = interpreter->inputs()[0];
	eaif_info_h("input: %d\n", interpreter->inputs()[0]);
	eaif_info_h("model type: %s\n", eaif_GetDTypeString(type));
	eaif_info_h("tensors size: %d\n", (int)interpreter->tensors_size());
    eaif_info_h("nodes size: %d\n", (int)interpreter->nodes_size());
    eaif_info_h("number of inputs: %d\n", (int)interpreter->inputs().size());
    eaif_info_h("input(0) name: %s\n", interpreter->GetInputName(0));
    eaif_info_h("number of outputs: %d\n", (int)outputs.size());
    eaif_info_h("output(0) name: %s\n", interpreter->GetOutputName(0));

	TfLiteIntArray* dims = interpreter->tensor(input)->dims;
	int wanted_height = dims->data[1];
	int wanted_width = dims->data[2];
	int wanted_channels = dims->data[3];

	
	eaif_info_h("inputs[0] dims: [%d %d %d %d]\n", dims->data[0], wanted_height, wanted_width, wanted_channels);
	
	for (auto i : outputs) {
		TfLiteIntArray* odims = interpreter->tensor(i)->dims;
		auto size = odims->size; eaif_info_h("outputs[%d] [", i);
		for (int j = 0; j < size; j++) {
			printf("%d ", odims->data[j]);
		}
		printf("]\n");
	}

	vector<QuantInfo> qinfos(outputs.size());
	tflite::utils::GetQuantInfo(interpreter, qinfos.data(), outputs.size());

	if (type == Eaif8U || type == Eaif8S) {
		int i = 0;
		for (auto& qinfo : qinfos) {
			eaif_info_h("outputs[%d] zero:%d scale:%.9f\n", i++, qinfo.m_zero, qinfo.m_scale);
		}
	}

	interpreter->SetNumThreads(m_nthread);
	eaif_info_h("Using %d cpu for model inference!\n", m_nthread);

	int batch = 1; //, batch_size = m_dim[0] * m_dim[1] * 3;
	std::vector<int> __dims{batch,m_h,m_w,3};

	if (__dims[1] != wanted_height || __dims[2] != wanted_width) {
		TIMER_FUNC("Resize Tensor", interpreter->ResizeInputTensor(input, __dims));
	    
		TIC(start);
		MINIMAL_CHECK(interpreter->AllocateTensors() == kTfLiteOk);
		TOC("Allocate Memory", start);
		eaif_info_l(" input: %d\n",interpreter->inputs()[0]);
		eaif_info_l("tensors size: %d\n",interpreter->tensors_size());
		eaif_info_l("nodes size: %d\n",interpreter->nodes_size());
		eaif_info_l("number of inputs: %d\n" ,interpreter->inputs().size());
		eaif_info_l("input(0) name: %s\n",interpreter->GetInputName(0));
		eaif_info_l("number of outputs: %d\n",outputs.size());
		for (auto i : outputs) {
			TfLiteIntArray* odims = interpreter->tensor(i)->dims;
			auto size = odims->size; eaif_info_h("outputs[%d] [", i);
			for (int j = 0; j < size; j++) {
				printf("%d ", odims->data[j]);
			}
		printf("]\n");
		}
    }

	dims = interpreter->tensor(input)->dims;
	wanted_height = (int)dims->data[1];
	wanted_width = (int)dims->data[2];
	wanted_channels = dims->data[3];

	eaif_info_l("[INFO] inputs[0] dims: [%d %d %d %d]\n", dims->data[0], wanted_height, wanted_width, wanted_channels);


	uint8_t* _iinput = nullptr;
	float* _finput = nullptr;
	if (type == Eaif8U)
		_iinput = interpreter->typed_tensor<uint8_t>(input);
	else if (type == Eaif8S)
		_iinput = (uint8_t*)interpreter->typed_tensor<int8_t>(input);
	else
		_finput = interpreter->typed_tensor<float>(input);
	// Fill input buffers

	for (int i = 0; i < m_iter; i++) {
		if (type == Eaif8U || type == Eaif8S) {
			if (img_file == nullptr)
				memset(interpreter->typed_tensor<uint8_t>(input), 0, sizeof(uint8_t) *  wanted_height * wanted_width * wanted_channels);
			else {
				WImage img(wanted_height, wanted_width, Eaif8UC3, _iinput);
				eaif::image::Imread(img_file, img, wanted_width, wanted_height);
#ifdef RESIZE_TFLITE
				tflite::utils::resize<uint8_t>(_iinput,
					img.data, img.rows, img.cols, img.channels(), wanted_height,
	                    wanted_width, wanted_channels, 0);
#endif /* !RESIZE_TFLITE */
			}
		} else if (type == Eaif32F) {
			if (img_file == nullptr)
				memset(interpreter->typed_tensor<float>(input), 0, sizeof(float) *  wanted_height * wanted_width * wanted_channels);
			else {
				WImage img(wanted_height, wanted_width, Eaif32FC3, _finput);
				WImage raw;
				eaif::image::Imread(img_file, raw);
				TIMER_FUNC("Process WImage", eaif::image::ImresizeNorm(raw, img, wanted_width, wanted_height, m_zero, m_scale));
				if ((int)dims->data[0] > 1) {
					int img_size = wanted_width * wanted_height * img.channels() * sizeof(float);
					for (int j = 1; j < (int)dims->data[0]; ++j) {
						memcpy((uint8_t*)_finput + img_size * j, (uint8_t*) _finput, img_size);
					}
				}
			}
		}
		// Run inference
		TIMER_FUNC("Inference", interpreter->Invoke() == kTfLiteOk);
	}

	// Run inference
	TIMER(interpreter->Invoke() == kTfLiteOk);
	if (m_verbose) {
		for (int p = 0; p < 10; ++p) {
			TIMER(interpreter->Invoke() == kTfLiteOk);
		}
	}
	printf("\n\n=== Post-invoke Interpreter State ===\n");
	//if (m_verbose) tflite::PrintInterpreterState(interpreter.get());

	// Read output buffers
	if (m_debug == 2) {
		for (size_t i = 0; i < outputs.size(); ++i) {
			auto* dim = interpreter->tensor(outputs[i])->dims;
			int eles = 1;
			for (int k = 0; k < dim->size; ++k)
				eles *= dim->data[k];
			if (type == Eaif8U) {
				eaif_info_h("dump size: %d [%d]\n", eles, (int)i); Dump(interpreter->typed_tensor<uint8_t>(outputs[i]), eles, i);
			} else if (type == Eaif8S) {
				uint8_t* addr = (uint8_t*)interpreter->typed_tensor<int8_t>(outputs[i]);
				eaif_info_h("dump size: %d [%d]\n", eles, (int)i); Dump(addr, eles, i);
			} else {
				eaif_info_h("dump size: %d [%d]\n", eles*4, (int)i); Dump((const uint8_t*)interpreter->typed_tensor<float>(outputs[i]), eles*sizeof(float), i);
			}
		}
	}
	tflite::utils::PrintOutput(interpreter.get(), m_range);
#endif // USE_TFLITE
	return 0;
}

int eaif::EngineDemo::RunMinimum(const char *tflite_model, const char *img_file)
{
	if (m_engine == 1)
		RunMinimumTflite(tflite_model, img_file);
	else if (m_engine == 0)
		RunMinimumArmnn(tflite_model, img_file);
	else
		assert(0);
	return 0;
}

