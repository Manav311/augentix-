#ifndef USE_NCNN
#include "inf_adapter.h"
#include "inf_utils_lite.h"
#include "inf_utils.h"
#include "inf_log.h"

#include "lite_classifier.h"
#include "lite_mtcnn.h"
#include "lite_scrfd.h"
#include "lite_faceencode.h"

#ifndef USE_MICROLITE
int inf_tf_adapter::LiteClassifier_LoadModel(LiteClassifier& classifier, const std::string& model_path,
                                             std::unique_ptr<tflite::Interpreter>& model,
                                             std::unique_ptr<tflite::FlatBufferModel>& model_fb)
{
	return utils::lite::LoadModels(model_path.c_str(), model, model_fb);
}

int inf_tf_adapter::LiteMtcnn_LoadPnetModel(LiteMtcnn& detector, const std::string& model_path,
                                        std::unique_ptr<tflite::Interpreter>& model,
                                        std::unique_ptr<tflite::FlatBufferModel>& model_fb)
{
	return utils::lite::LoadModels(model_path.c_str(), model, model_fb);
}

int inf_tf_adapter::LiteMtcnn_LoadRnetModel(LiteMtcnn& detector, const std::string& model_path,
                                            std::unique_ptr<tflite::Interpreter>& model,
                                            std::unique_ptr<tflite::FlatBufferModel>& model_fb)
{
	return utils::lite::LoadModels(model_path.c_str(), model, model_fb);
}

int inf_tf_adapter::LiteMtcnn_LoadOnetModel(LiteMtcnn& detector, const std::string& model_path,
                                            std::unique_ptr<tflite::Interpreter>& model,
                                            std::unique_ptr<tflite::FlatBufferModel>& model_fb)
{
	return utils::lite::LoadModels(model_path.c_str(), model, model_fb);
}

int inf_tf_adapter::LiteScrfd_LoadModel(LiteScrfd& detector, const std::string& model_path,
					std::unique_ptr<tflite::Interpreter>& model,
					std::unique_ptr<tflite::FlatBufferModel>& model_fb)
{
	return utils::lite::LoadModels(model_path.c_str(), model, model_fb);
}

int inf_tf_adapter::LiteFaceEncode_LoadModel(LiteFaceEncode& encoder, const std::string& model_path,
					     std::unique_ptr<tflite::Interpreter>& model,
					     std::unique_ptr<tflite::FlatBufferModel>& model_fb)
{
	return utils::lite::LoadModels(model_path.c_str(), model, model_fb);
}

size_t inf_tf_adapter::numsOfOutputTensor(const tflite::Interpreter& model)
{
	return model.outputs().size();
}

void inf_tf_adapter::setNumThreads(tflite::Interpreter& model, int num_thread)
{
	model.SetNumThreads(num_thread);
}

void inf_tf_adapter::showArenaUsedBytes(const tflite::Interpreter& model, const std::string& model_name)
{
	//tflite DOESN'T support this.
}

size_t inf_tf_adapter::getArenaUsedBytes(const tflite::Interpreter& model)
{
	return 0;
}

void inf_tf_adapter::resizeInputTensor(tflite::Interpreter& model, int input_index, const std::vector<int>& new_dims)
{
	model.ResizeInputTensor(model.inputs()[input_index], new_dims);
}
#else
size_t inf_tf_adapter::numsOfOutputTensor(const tflite::MicroInterpreter& model)
{
	return model.outputs_size();
}

void inf_tf_adapter::setNumThreads(tflite::MicroInterpreter& model, int num_thread)
{
	//	tflite-micro DOESN'T support multi-threading.
}

void inf_tf_adapter::showArenaUsedBytes(const tflite::MicroInterpreter& model, const std::string& model_name)
{
	inf_log_notice("%s had used %d bytes of arena.", model_name.c_str(), model.arena_used_bytes());
}

size_t inf_tf_adapter::getArenaUsedBytes(const tflite::MicroInterpreter& model)
{
	return model.arena_used_bytes();
}

void inf_tf_adapter::resizeInputTensor(tflite::MicroInterpreter& model, int input_index,
                                       const std::vector<int>& new_dims)
{
	//Currently I have no idea how to achieve this.
}

static tflite::AllOpsResolver _all_ops_resolver;

int inf_tf_adapter::LiteClassifier_LoadModel(LiteClassifier& classifier, const std::string& model_path,
                                             std::unique_ptr<tflite::MicroInterpreter>& model,
                                             std::unique_ptr<unsigned char[]>& model_fb)
{
	static tflite::MicroErrorReporter error_reporter;
	int model_size;
	model_fb = std::unique_ptr<unsigned char[]>(LoadModelData(model_path.c_str(), &model_size));
	const tflite::Model* schema_model = tflite::GetModel(model_fb.get());
	if (schema_model->version() != TFLITE_SCHEMA_VERSION) {
		TF_LITE_REPORT_ERROR(static_cast<void*>(&error_reporter),
		                     "Model provided is schema version %d not equal "
		                     "to supported version %d.",
		                     schema_model->version(), TFLITE_SCHEMA_VERSION);
		return -1;
	}

	model = std::unique_ptr<tflite::MicroInterpreter>(
	        new tflite::MicroInterpreter(schema_model, _all_ops_resolver,
	                                     classifier.m_arena, sizeof(classifier.m_arena), &error_reporter));
	return 0;
}

static int LiteMtcnn_LoadSubmodel(LiteMtcnn& detector, const std::string& model_path,
                                  std::unique_ptr<tflite::MicroInterpreter>& model,
                                  std::unique_ptr<unsigned char[]>& model_fb,
                                  uint8_t *arena, size_t arena_size)
{
	static tflite::MicroErrorReporter error_reporter;
	int model_size;
	model_fb = std::unique_ptr<unsigned char[]>(LoadModelData(model_path.c_str(), &model_size));
	const tflite::Model* schema_model = tflite::GetModel(model_fb.get());
	if (schema_model->version() != TFLITE_SCHEMA_VERSION) {
		TF_LITE_REPORT_ERROR(static_cast<void*>(&error_reporter),
		                     "Model provided is schema version %d not equal "
		                     "to supported version %d.",
		                     schema_model->version(), TFLITE_SCHEMA_VERSION);
		return -1;
	}

	model = std::unique_ptr<tflite::MicroInterpreter>(
	        new tflite::MicroInterpreter(schema_model, _all_ops_resolver,
	                                     arena,arena_size,
	                                     &error_reporter));
	return 0;
}

int inf_tf_adapter::LiteMtcnn_LoadPnetModel(LiteMtcnn& detector, const std::string& model_path,
                                            std::unique_ptr<tflite::MicroInterpreter>& model,
                                            std::unique_ptr<unsigned char[]>& model_fb)
{
	return LiteMtcnn_LoadSubmodel(detector, model_path, model, model_fb,
	                              detector.m_pnet_arena, sizeof(detector.m_pnet_arena));
}

int inf_tf_adapter::LiteMtcnn_LoadRnetModel(LiteMtcnn& detector, const std::string& model_path,
					    std::unique_ptr<tflite::MicroInterpreter>& model,
					    std::unique_ptr<unsigned char[]>& model_fb)
{
	return LiteMtcnn_LoadSubmodel(detector, model_path, model, model_fb,
	                              detector.m_rnet_arena, sizeof(detector.m_rnet_arena));
}
int inf_tf_adapter::LiteMtcnn_LoadOnetModel(LiteMtcnn& detector, const std::string& model_path,
					    std::unique_ptr<tflite::MicroInterpreter>& model,
					    std::unique_ptr<unsigned char[]>& model_fb)
{
	return LiteMtcnn_LoadSubmodel(detector, model_path, model, model_fb,
	                              detector.m_onet_arena, sizeof(detector.m_onet_arena));
}
int inf_tf_adapter::LiteScrfd_LoadModel(LiteScrfd& detector, const std::string& model_path,
                                        std::unique_ptr<tflite::MicroInterpreter>& model,
                                        std::unique_ptr<unsigned char[]>& model_fb)
{
	static tflite::MicroErrorReporter error_reporter;
	int model_size;
	model_fb = std::unique_ptr<unsigned char[]>(LoadModelData(model_path.c_str(), &model_size));
	const tflite::Model* schema_model = tflite::GetModel(model_fb.get());
	if (schema_model->version() != TFLITE_SCHEMA_VERSION) {
		TF_LITE_REPORT_ERROR(static_cast<void*>(&error_reporter),
		                     "Model provided is schema version %d not equal "
		                     "to supported version %d.",
		                     schema_model->version(), TFLITE_SCHEMA_VERSION);
		return -1;
	}

	model = std::unique_ptr<tflite::MicroInterpreter>(
	        new tflite::MicroInterpreter(schema_model, _all_ops_resolver,
	                                     detector.m_arena, sizeof(detector.m_arena), &error_reporter));
	return 0;
}
int inf_tf_adapter::LiteFaceEncode_LoadModel(LiteFaceEncode& encoder, const std::string& model_path,
                                             std::unique_ptr<tflite::MicroInterpreter>& model,
                                             std::unique_ptr<unsigned char[]>& model_fb)
{
	static tflite::MicroErrorReporter error_reporter;
	int model_size;
	model_fb = std::unique_ptr<unsigned char[]>(LoadModelData(model_path.c_str(), &model_size));
	const tflite::Model* schema_model = tflite::GetModel(model_fb.get());
	if (schema_model->version() != TFLITE_SCHEMA_VERSION) {
		TF_LITE_REPORT_ERROR(static_cast<void*>(&error_reporter),
		                     "Model provided is schema version %d not equal "
		                     "to supported version %d.",
		                     schema_model->version(), TFLITE_SCHEMA_VERSION);
		return -1;
	}

	model = std::unique_ptr<tflite::MicroInterpreter>(
	        new tflite::MicroInterpreter(schema_model, _all_ops_resolver,
	                                     encoder.m_arena, sizeof(encoder.m_arena), &error_reporter));
	return 0;
}
#endif
#endif // USE_NCNN