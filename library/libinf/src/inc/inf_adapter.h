#ifndef USE_NCNN
#ifndef INF_ADAPTER_H_
#define INF_ADAPTER_H_

#include <memory>

#ifdef USE_MICROLITE
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#else
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model_builder.h"
#endif

class LiteClassifier;
class LiteMtcnn;
class LiteScrfd;
class LiteFaceEncode;

namespace inf_tf_adapter
{
#ifndef USE_MICROLITE
int LiteScrfd_LoadModel(LiteScrfd& detector, const std::string& model_path, std::unique_ptr<tflite::Interpreter>& model,
                        std::unique_ptr<tflite::FlatBufferModel>& model_fb);
int LiteClassifier_LoadModel(LiteClassifier& classifier, const std::string& model_path,
                             std::unique_ptr<tflite::Interpreter>& model,
                             std::unique_ptr<tflite::FlatBufferModel>& model_fb);
int LiteMtcnn_LoadPnetModel(LiteMtcnn& detector, const std::string& model_path,
                            std::unique_ptr<tflite::Interpreter>& model,
                            std::unique_ptr<tflite::FlatBufferModel>& model_fb);
int LiteMtcnn_LoadRnetModel(LiteMtcnn& detector, const std::string& model_path,
                            std::unique_ptr<tflite::Interpreter>& model,
                            std::unique_ptr<tflite::FlatBufferModel>& model_fb);
int LiteMtcnn_LoadOnetModel(LiteMtcnn& detector, const std::string& model_path,
                            std::unique_ptr<tflite::Interpreter>& model,
                            std::unique_ptr<tflite::FlatBufferModel>& model_fb);

int LiteFaceEncode_LoadModel(LiteFaceEncode& encoder, const std::string& model_path,
                             std::unique_ptr<tflite::Interpreter>& model,
                             std::unique_ptr<tflite::FlatBufferModel>& model_fb);
size_t numsOfOutputTensor(const tflite::Interpreter& model);

void setNumThreads(tflite::Interpreter& model, int num_thread);
void showArenaUsedBytes(const tflite::Interpreter& model, const std::string& model_name);
size_t getArenaUsedBytes(const tflite::Interpreter& model);
void resizeInputTensor(tflite::Interpreter& model, int input_index, const std::vector<int>& new_dims);
#else
int LiteClassifier_LoadModel(LiteClassifier& classifier, const std::string& model_path,
                             std::unique_ptr<tflite::MicroInterpreter>& model,
                             std::unique_ptr<unsigned char[]>& model_fb);
int LiteMtcnn_LoadPnetModel(LiteMtcnn& detector, const std::string& model_path,
                            std::unique_ptr<tflite::MicroInterpreter>& model,
                            std::unique_ptr<unsigned char[]>& model_fb);
int LiteMtcnn_LoadRnetModel(LiteMtcnn& detector, const std::string& model_path,
                            std::unique_ptr<tflite::MicroInterpreter>& model,
                            std::unique_ptr<unsigned char[]>& model_fb);
int LiteMtcnn_LoadOnetModel(LiteMtcnn& detector, const std::string& model_path,
                            std::unique_ptr<tflite::MicroInterpreter>& model,
                            std::unique_ptr<unsigned char[]>& model_fb);

int LiteScrfd_LoadModel(LiteScrfd& detector, const std::string& model_path,
                        std::unique_ptr<tflite::MicroInterpreter>& model, std::unique_ptr<unsigned char[]>& model_fb);
int LiteFaceEncode_LoadModel(LiteFaceEncode& encoder, const std::string& model_path,
                             std::unique_ptr<tflite::MicroInterpreter>& model,
                             std::unique_ptr<unsigned char[]>& model_fb);

size_t numsOfOutputTensor(const tflite::MicroInterpreter& model);
//void* inputTensorBuffer(const tflite::MicroInterpreter& model, size_t input_index);
void setNumThreads(tflite::MicroInterpreter& model, int num_thread);

void showArenaUsedBytes(const tflite::MicroInterpreter& model, const std::string& model_name);
size_t getArenaUsedBytes(const tflite::MicroInterpreter& model);
void resizeInputTensor(tflite::MicroInterpreter& model, int input_index, const std::vector<int>& new_dims);
#endif
}

#endif //INF_ADAPTER_H_
#endif // USE_NCNN