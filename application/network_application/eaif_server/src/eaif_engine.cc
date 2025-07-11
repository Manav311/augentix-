#include <assert.h>
#include <string.h>

#include <memory>
#include <string>
#include <vector>

#include "eaif_data.h"
#include "eaif_engine.h"
#include "eaif_image.h"
#include "eaif_model.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include "facereco_model.h"

using namespace std;

enum eaif::engine::EngineType eaif::engine::GetEngineType(const char* engine_str)
{
	if (!strcmp(engine_str, "armnn"))
		return eaif::engine::Armnn;
	else if (!strcmp(engine_str, "tflite"))
		return eaif::engine::TfLite;
	else if (!strcmp(engine_str, "other"))
		return eaif::engine::Other;
	else {
		eaif_warn("invalid enginer str %s\n", engine_str);
		assert(0);
	}
}

enum eaif::engine::InferenceType eaif::engine::GetInferenceType(const char* inference_str)
{
	if (!strcmp(inference_str, "classify"))
		return eaif::engine::Classify;
	else if (!strcmp(inference_str, "detect"))
		return eaif::engine::Detect;
	else if (!strcmp(inference_str, "facereco"))
		return eaif::engine::FaceReco;
	else if (!strcmp(inference_str, "infer"))
		return eaif::engine::Infer;
	else {
		eaif_warn("invalid inference_str %s\n", inference_str);
		assert(0);
	}
}

enum eaif::engine::ActivationType eaif::engine::GetActivationType(const char* activation_str)
{
	if (!strcmp(activation_str, "linear"))
		return eaif::engine::Linear;
	else if (!strcmp(activation_str, "sigmoid"))
		return eaif::engine::Sigmoid;
	else {
		eaif_warn("invalidactivation_str %s\n", activation_str);
		assert(0);
	}
}

enum eaif::engine::FaceInferType eaif::engine::GetFaceInferType(const char *face_infer_str)
{
	if (!strcmp(face_infer_str, "none"))
		return eaif::engine::FaceInferNone;
	else if (!strcmp(face_infer_str, "mtcnn"))
		return eaif::engine::Mtcnn;
	else if (!strcmp(face_infer_str, "facenet"))
		return eaif::engine::Facenet;
	else if (!strcmp(face_infer_str, "both"))
		return eaif::engine::FaceInferBoth;
	else {
		eaif_warn("invalid face_infer_str %s\n", face_infer_str);
		assert(0);
	}
}

enum eaif::engine::ModelType eaif::engine::GetModelType(const char* model_type_str)
{
	if (!strcmp(model_type_str, "neuralclassify"))
		return eaif::engine::ModelTypeNeuralClassify;
	else if (!strcmp(model_type_str, "c4"))
		return eaif::engine::ModelTypeC4;
	else if (!strcmp(model_type_str, "yolov4"))
		return eaif::engine::ModelTypeYolov4;
	else if (!strcmp(model_type_str, "yolov5"))
		return eaif::engine::ModelTypeYolov5;
	else if (!strcmp(model_type_str, "mtcnn"))
		return eaif::engine::ModelTypeMtcnn;
	else if (!strcmp(model_type_str, "facenet"))
		return eaif::engine::ModelTypeFaceNet;
	else if (!strcmp(model_type_str, "facereco"))
		return eaif::engine::ModelTypeFaceReco;
	else {
		eaif_warn("Unknown model type str %s\n", model_type_str);
		return eaif::engine::ModelTypeUnknown;
	}
}


static int SetupLocal(EngineConfig &engine_conf, vector<string>& model_paths)
{
	FILE *fp = fopen(engine_conf.config_path.c_str(), "r");
	char html_path[256] = {};
	char face_bin_path[256] = {};
	if (fp == nullptr) {
		eaif_warn("Cannot find %s\n", engine_conf.config_path.c_str());
		return EAIF_FAILURE;
	}
	if ((fscanf(fp, "num_thread=%d\n", &engine_conf.num_thread) != 1) ||
		(fscanf(fp, "debug=%d\n", &engine_conf.debug) != 1) ||
		(fscanf(fp, "verbose=%d\n", &engine_conf.verbose) != 1) ||
        (fscanf(fp, "html=%s\n", html_path) != 1) ||
        (fscanf(fp, "facebin=%s\n", face_bin_path) != 1)) {
		eaif_warn("Wrong global engine config!\n");
		fclose(fp);
		return EAIF_FAILURE;
	}

	int start = 0, nread;
	size_t size;
	char *str = nullptr;
	engine_conf.template_path = string(html_path);
	engine_conf.face_bin_path = string(face_bin_path);

	eaif::utils::ForceValidThreadNum(engine_conf.num_thread);

	while ((nread = getline(&str, &size, fp)) != -1) {
		if (size) {
			if (str[0] == '#') {
				if (strlen(str) > 1)
					if (str[1] != '#')
						cout << "Skip " << str << "\n";
				continue;
			}
			if (start && (strlen(str) >= 3)) {
				model_paths.push_back(string(str).erase(nread-1));
			}
			if (!start && !strcmp(str, "[model_config_paths]\n"))
				start = 1;
		}
	}
	if (str) free(str);
	str = nullptr;
	fclose(fp);
	if (model_paths.size() == 0) {
		eaif_warn("No model is specified in engine config!\n");
		//return EAIF_FAILURE;
	}
	return EAIF_SUCCESS;
}

int eaif::Engine::GetVerbose(void) const
{
	return m_config.verbose;
}

int eaif::Engine::Setup(string& engine_config)
{
	m_config.config_path = engine_config;

	vector<string> model_paths;

	if (SetupLocal(m_config, model_paths) == EAIF_FAILURE)
		return EAIF_FAILURE;

#ifdef USE_ARMNN
	armnn::utils::SetDebugMode(m_config.debug);
	armnn::utils::SetThreadNum(m_config.num_thread);
#endif

	for (auto model_path : model_paths) {
		shared_ptr<eaif::Model> model_ptr = eaif::ModelFactory::CreateModelInstance(*this, model_path);
		if (!model_ptr) {
			eaif_warn("Cannot create model from %s\n", model_path.c_str());
			continue;
		}
		models.push_back(model_ptr);
	}
	return EAIF_SUCCESS;
}

void eaif::Engine::Update(void)
{
	models.clear();
	Setup(m_config.config_path);
	return;
}

void eaif::Engine::QueryModelNames(eaif::engine::InferenceType type, vector<string>& str) const
{
	for (auto& model_ptr : models) {
		if (model_ptr->config.inference_type == type) {
			str.push_back(model_ptr->config.model_name);
		}
	}
}

int eaif::Engine::QueryModelDetail(const string& model_str, ModelConfig& conf) const
{
	for (auto& model_ptr : models) {
		if (strcmp(model_ptr->config.model_name.c_str(), model_str.c_str()) == 0) {
			conf = model_ptr->config;
			return EAIF_SUCCESS;
		}
	}
	return EAIF_FAILURE;
}

inline void AssignGeneralModelInfo(const ModelConfig &config, ModelInfo &info)
{
	info.name = config.model_name.c_str();
	info.inference_type = config.inference_type;
	info.dataset = config.dataset;
	info.labels = &config.labels;
	info.topk = config.topk;
	info.filter_cls = &config.filter_cls;
	info.filter_out_cls = &config.filter_out_cls;
	info.channels = config.channels;
}

inline void AssignFaceRecoModelInfo(const shared_ptr<eaif::Model> host_model_ptr, ModelInfo &info)
{
	const ModelConfig &host_config = host_model_ptr->config;
	info.name = host_config.model_name.c_str();
	info.inference_type = host_config.inference_type;
	info.filter_cls = &host_config.filter_cls;
	info.filter_out_cls = &host_config.filter_out_cls;
	info.channels = host_config.channels;
	info.topk = host_config.topk;

	const shared_ptr<FacerecoModel> facemodel = dynamic_pointer_cast<FacerecoModel>(host_model_ptr);
	if (host_config.face_infer_type == eaif::engine::Mtcnn) {
		info.dataset = facemodel->GetMtcnnConfig().dataset;
		info.labels = &facemodel->GetMtcnnConfig().labels;
	} else if (host_config.face_infer_type == eaif::engine::Facenet ||
		host_config.face_infer_type == eaif::engine::FaceInferBoth) {
		info.dataset = facemodel->GetFacenetConfig().dataset;
		info.labels = facemodel->GetFaceNameList();
	} else {
		info.dataset = host_config.dataset;
		info.labels = &host_config.labels;
	}
}

int eaif::Engine::QueryModelInfo(const string& model_str, ModelInfo& info) const
{
	int i = 0;
	for (auto& model_ptr : models) {
		const ModelConfig& config = model_ptr->config;
		if (strcmp(config.model_name.c_str(), model_str.c_str()) == 0) {
			info.index = i;
			if (config.inference_type == eaif::engine::FaceReco)
				AssignFaceRecoModelInfo(model_ptr, info);
			else
				AssignGeneralModelInfo(config, info);
			return EAIF_SUCCESS;
		}
		i++;
	}
	return EAIF_FAILURE;
}

void eaif::Engine::ClearModelStates(void)
{
	for (auto& model : models)
		model->ClearModelState();
}

void eaif::Engine::GetModelStates(vector<StatePair>& states) const
{
	for (auto& model : models)
		states.push_back(StatePair(model->config.model_name,model->GetModelState()));
}

string eaif::Engine::GetTemplatePath(void) const
{
    return m_config.template_path;
}

int eaif::Engine::QueryAvailModel(vector<ModelInfo>& infos) const
{
	infos.resize(models.size());
	for (size_t i = 0; i < models.size(); ++i) {
		infos[i].name = models[i]->config.model_name.c_str();
		infos[i].index = i;
		infos[i].inference_type = models[i]->config.inference_type;
		infos[i].dataset = models[i]->config.dataset;
	}
	return EAIF_SUCCESS;
}

int eaif::Engine::ClassifyObjList(int model_index, const void *Wimg, const vector<ObjectAttr >& obj_list,
                              vector<Classification>& results)
{
	return models[model_index]->ClassifyObjList(Wimg, obj_list, results);
}

int eaif::Engine::Classify(int model_index, const void* Wimg, Classification& results)
{
	return models[model_index]->Classify(Wimg, results);	
}

int eaif::Engine::Detect(int model_index, const void* Wimg, vector<Detection>& results)
{
	return models[model_index]->Detect(Wimg, results);
}

int eaif::Engine::RegisterFace(int model_index, const void *Wimg, const string& face_name, int is_full_image)
{
	shared_ptr<FacerecoModel> model = dynamic_pointer_cast<FacerecoModel>(models[model_index]);
	if (!model) {
		return EAIF_FAILURE;
	}
	int ret = EAIF_SUCCESS;
	if (is_full_image) {
		const WImage *wimg = (const WImage*)Wimg;
		Detection det = {{0.0f, 0.0f, (float)(wimg->cols-1), (float)(wimg->rows-1)}};
		ret = model->Register(Wimg, face_name, det, model->config);
	} else {
		ret = model->Register(Wimg, face_name);
	}
	return ret;
}

int eaif::Engine::LoadFaceData(int model_index)
{
	shared_ptr<FacerecoModel> model = dynamic_pointer_cast<FacerecoModel>(models[model_index]);
	if (!model) {
		return EAIF_FAILURE;
	}
	int ret = model->LoadFaceData(m_config.face_bin_path);
	if (ret != EAIF_SUCCESS) {
		eaif_warn("Cannot Load face data from %s\n", m_config.face_bin_path.c_str());
	}
	return ret;
}

int eaif::Engine::SaveFaceData(int model_index) const
{
	shared_ptr<FacerecoModel> model = dynamic_pointer_cast<FacerecoModel>(models[model_index]);
	if (!model) {
		return EAIF_FAILURE;
	}
	int ret = model->SaveFaceData(m_config.face_bin_path);
	if (ret != EAIF_SUCCESS) {
		eaif_warn("Cannot Load face data from %s\n", m_config.face_bin_path.c_str());
	}
	return ret;
}

int eaif::Engine::QueryFaceInfo(int model_index, vector<string> &face_info) const
{
	shared_ptr<FacerecoModel> model = dynamic_pointer_cast<FacerecoModel>(models[model_index]);
	if (!model) {
		return EAIF_FAILURE;
	}
	model->QueryFaceInfo(face_info);
	return EAIF_SUCCESS;
}
