#ifndef EAIF_UTILS_H_
#define EAIF_UTILS_H_

#include <sys/stat.h>
#include <cerrno>
#include <cstdio>
#include <iostream>
#include <vector>
#include <math.h>

#include "eaif_common.h"

using std::cerr;
using std::endl;

int DecodeJsonStr2Object(const char *str, size_t size, std::vector<ObjectAttr> &obj_list);

int EncodeObject2JsonStr(const std::vector<ObjectAttr> &obj_list, char *str);

namespace dataset
{
typedef std::vector<std::vector<std::string> > String2d;

int CreateDir(const char *dir_path);

class Wider {
    public:
	Wider(std::string &data_dir_in, std::string &dst_dir_in)
	        : data_dir(data_dir_in)
	        , dst_dir(dst_dir_in)
	        , data_list_file(data_dir_in + std::string("/wider_face_val_bbx_gt.txt"))
	{
		if (mkdir(dst_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
			if (errno == EEXIST)
				return;
			cerr << "Cannot open dir " << dst_dir_in << endl;
		}
	};
	void GetValList(std::vector<std::string> &img_list_abs, std::vector<std::string> &pred_list);
	std::vector<std::string> img_list;
	std::string data_dir;
	std::string dst_dir;
	std::string data_list_file;
};

class Fddb {
    public:
	Fddb(std::string &data_dir_in, std::string &dst_dir_in)
	        : data_dir(data_dir_in)
	        , dst_dir(dst_dir_in)
	        , total_imgs_(0)
	{
		if (mkdir(dst_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
			if (errno == EEXIST)
				return;
			cerr << "Cannot open dir " << dst_dir_in << endl;
		}
	};
	void GetValList(String2d &img_list_abs, std::vector<std::string> &pred_list);
	String2d img_list;
	const char *data_fold_fmt = "/FDDB-fold-%02d.txt";
	const char *data_det_fold_fmt = "/FDDB-det-fold-%02d.txt";
	std::string data_dir;
	std::string dst_dir;
	int total_imgs_;
};

} // namespace dataset

namespace eaif
{
namespace utils
{
int ForceValidThreadNum(int &thread_num);

} // namespace utils
} // namespace eaif

#ifdef USE_TFLITE
#include "tensorflow/lite/builtin_op_data.h"
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/string_util.h"

namespace tflite
{
namespace utils
{
int LoadModels(const char *tflite_model, std::unique_ptr<tflite::Interpreter> &interpreter,
               std::unique_ptr<tflite::FlatBufferModel> &model);

EaifDataType GetDataType(const tflite::Interpreter *interpreter, int tensor_idx);
void GetQuantInfo(const std::unique_ptr<tflite::Interpreter> &model, QuantInfo *info, int num_output);

void PrintOutput(const std::unique_ptr<tflite::Interpreter> &interpreter, int range[2]);
void PrintOutput(const tflite::Interpreter *interpreter, int range[2]);
void PrintOutput(const std::unique_ptr<tflite::Interpreter> &interpreter, int range[2], const char *img_file,
                 const char *dst_dir_in, int flip);

template <class T>
void Resize(T *out, uint8_t *in, int image_height, int image_width, int image_channels, int wanted_height,
            int wanted_width, int wanted_channels, int input_floating);
void SetThreadNum(int thread_num);

} // namespace utils
} // namespace tflite

#endif /* !USE_TFLITE */

#ifdef USE_ARMNN
#include <istream>
#include <string>
#include <vector>

#include <boost/format.hpp>
#include <boost/program_options.hpp>
#include <boost/variant.hpp>

#include <armnn/BackendId.hpp>
#include <armnn/BackendRegistry.hpp>
#include <armnn/IRuntime.hpp>
#include <armnn/Tensor.hpp>
#include <armnn/TypesUtils.hpp>
#include <armnnTfLiteParser/ITfLiteParser.hpp>

#include "eaif_common.h"

namespace armnn
{
namespace utils
{
class ContainerVisitor {
    public:
	template <typename T> void operator()(std::vector<T> &x)
	{
		x.resize(size);
	}
	int size;
};

enum CpuInferType { CpuAcc = 0, CpuRef };

class ArmnnModel {
    public:
	ArmnnModel(void)
	        : network(nullptr, armnn::INetwork::Destroy)
	        , model(nullptr, armnnTfLiteParser::ITfLiteParser::Destroy)
	        , runtime(nullptr, armnn::IRuntime::Destroy)
	        , type(Eaif32F)
	        , cpu_infer_type(0)
	        , set_profile(0){};
	// armnn_model(const char* tflite_model, std::vector<armnn::BackendId>
	// computeDevice);
	~ArmnnModel(){};
	int Invoke(void);
	armnn::INetworkPtr network;
	armnnTfLiteParser::ITfLiteParserPtr model;
	armnn::NetworkId networkId;
	armnn::IRuntimePtr runtime;
	armnn::InputTensors input_tensors;
	armnn::OutputTensors output_tensors;
	std::vector<boost::variant<std::vector<uint8_t> > > input_data;
	std::vector<boost::variant<std::vector<uint8_t> > > output_data;
	std::vector<boost::variant<std::vector<float> > > input_dataf;
	std::vector<boost::variant<std::vector<float> > > output_dataf;

	EaifDataType type;
	int cpu_infer_type;
	int set_profile;
};

int LoadModels(const char *tflite_model, ArmnnModel &model);
int LoadModels(const char *tflite_model, ArmnnModel &model, std::vector<armnn::BackendId> compute_device);
void GetQuantInfo(const armnnTfLiteParser::ITfLiteParserPtr &model, std::vector<QuantInfo> &info);
void InitModelIO(ArmnnModel &model, armnn::InputTensors &input_tensors, armnn::OutputTensors &output_tensors);
void SetDebugMode(int val);
void SetThreadNum(int thread_num);

template <typename T>
armnn::InputTensors MakeInputTensors(const std::vector<armnn::BindingPointInfo> &input_bindings,
                                     std::vector<boost::variant<std::vector<T> > > &input_data_containers);
template <typename T>
armnn::OutputTensors MakeOutputTensors(std::vector<armnn::BindingPointInfo> &output_bindings,
                                       std::vector<boost::variant<std::vector<T> > > &output_data_containers);

void PrintOutput(armnn::utils::ArmnnModel &model, int range[2]);
template <typename T, int D>
void PrintOutput(int output_dims[], std::vector<std::vector<T> > &output_data, int range[2]);
template <typename T>

void PrintOutput(std::vector<BindingPointInfo> &output_bindings, std::vector<T *> &output_data, int range[2]);

void PrintOutput(armnn::utils::ArmnnModel &model, int range[2], const char *img_file, const char *dst_dir_in, int flip);
template <typename T, int D>
void PrintOutput(int output_dims[], std::vector<std::vector<T> > &output_data, int range[2], const char *img_file,
                 const char *dst_dir_in, int flip);
template <typename T>
void PrintOutput(std::vector<BindingPointInfo> &output_bindings, std::vector<T *> &output_data, int range[2],
                 const char *img_file, const char *dst_dir_in, int flip);

} // namespace utils
} // namespace armnn

#endif /* !USE_ARMNN */

#ifdef USE_MPI
extern "C" {
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_index.h"
}
#endif

namespace mpi
{
namespace utils
{
class MpiCtx {
    public:
	MpiCtx();
	~MpiCtx();
#ifdef USE_MPI
	MPI_ECHN snapshot_chn;
	MPI_BCHN bchn;
	MPI_VIDEO_FRAME_INFO_S info;
	MPI_WIN snapshot_win;
#endif
	int GetServiceY(int video_window, int height, int width, uint8_t **data);
	int GetServiceRgb(int video_window, int height, int width, uint8_t **data);
	int GetServiceYuv(int video_window, int height, int width, uint8_t **data);
	int ReleaseFrameIfAny(void);
};
int GetServiceJpeg(uint8_t **data, int target_chn);
} // utils
} // mpi

#endif /* !EAIF_UTILS_H_ */
