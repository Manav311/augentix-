#include <cstring>
#include <memory>
#include <string>
#include <vector>

#include "eaif_engine.h"
#include "eaif_model.h"
#include "eaif_image.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include "c4_classify_model.h"
#include "human_classify_model.h"
#include "neural_classify_model.h"
#include "yolo_model.h"
#include "facereco_model.h"

using namespace std;
using namespace eaif::image;

int eaif::Model::ClassifyObjList(const void *Wimg, const vector<ObjectAttr> &obj_list, vector<Classification> &results)
{
	return ClassifyObjList(Wimg, obj_list, results, config);
}

int eaif::Model::Classify(const void *Wimg, Classification &result)
{
	return Classify(Wimg, result, config);
}

int eaif::Model::Detect(const void *Wimg, vector<Detection> &detections)
{
	return Detect(Wimg, detections, config);
}

void eaif::Model::UpdateState(int num)
{
	m_state.infer_count += num;
}

const ModelState& eaif::Model::GetModelState(void) const
{
	return m_state;
}

void eaif::Model::ClearModelState(void)
{
	m_state.infer_count = 0;
}

void eaif::Model::CalcImgPreResizeRatio(const void *Wimg)
{
	WImage *src_img = (WImage*) Wimg;
	m_pre_resize_ratio = (float)config.image_pre_resize / Max(src_img->rows, src_img->cols);
}

shared_ptr<eaif::Model> eaif::ModelFactory::CreateModelInstance(const eaif::Engine &engine, const string &model_path)
{
	eaif_info_h("Setting up %s\n", model_path.c_str());
	ModelConfig config;
	config.Parse(model_path.c_str());

	if (strstr(config.model_name.c_str(), "yolov5") != nullptr) {
		Yolov5Model *ymodel = new Yolov5Model();
		ymodel->config = config;
		ymodel->SetNumThreads(engine.m_config.num_thread);
		ymodel->SetDebug(engine.m_config.debug);
		ymodel->SetVerbose(engine.m_config.verbose);
		int ret = ymodel->LoadModels(config.model_dir, config.model_path);
		if (ret == EAIF_FAILURE) {
			delete ymodel;
			return nullptr;
		}
		return shared_ptr<eaif::Model>(ymodel);
	}

	if (strstr(config.model_name.c_str(), "shuffleNetV2") != nullptr) {
		NeuralClassifyModel *nmodel = new NeuralClassifyModel();
		nmodel->config = config;
		nmodel->SetNumThreads(engine.m_config.num_thread);
		nmodel->SetDebug(engine.m_config.debug);
		nmodel->SetVerbose(engine.m_config.verbose);
		int ret = nmodel->LoadModels(config.model_dir, config.model_path);
		if (ret == EAIF_FAILURE) {
			delete nmodel;
			return nullptr;
		}
		return shared_ptr<eaif::Model>(nmodel);
	}

	if (strstr(config.model_name.c_str(), "C4") != nullptr) {
#ifdef USE_C4
		C4ClassifyModel *c4model = new C4ClassifyModel();
		c4model->config = config;
		c4model->SetDebug(engine.m_config.debug);
		c4model->SetVerbose(engine.m_config.verbose);
		int ret = c4model->LoadModels(config.model_dir, config.model_path);
		if (ret == EAIF_FAILURE) {
			delete c4model;
			return nullptr;
		}
		return shared_ptr<eaif::Model>(c4model);
#else // !USE_C4
		eaif_warn("C4 module is disabled!\n");
		return nullptr;
#endif // USE_C4
	}

#if 1
	if (strstr(config.model_name.c_str(), "human_classify") != nullptr) {
		if (config.model_path.size() != 4)
			return nullptr;

		shared_ptr<eaif::Model> model_cv;
		shared_ptr<eaif::Model> model_dl;
		ModelInfo info_cv, info_dl;

		engine.QueryModelInfo(config.model_path[HumanClassifyModel::CVModelIndex], info_cv);
		engine.QueryModelInfo(config.model_path[HumanClassifyModel::DLModelIndex], info_dl);
		if (info_cv.index == -1 || info_dl.index == -1) {
			return nullptr;
		}

		HumanClassifyModel *hc_model = new HumanClassifyModel();
		hc_model->config = config;
		hc_model->SetClassifierDL(engine.models[info_dl.index]);
		hc_model->SetClassifierCV(engine.models[info_cv.index]);
		int ret = hc_model->LoadModels(config.model_dir, config.model_path);
		if (ret != EAIF_SUCCESS) {
			delete hc_model;
			return nullptr;
		}
		return shared_ptr<eaif::Model>(hc_model);
	}

	if (strstr(config.model_name.c_str(), "facereco") != nullptr) {
		FacerecoModel *facereco_model = new FacerecoModel();
		facereco_model->config = config;
		facereco_model->SetNumThreads(engine.m_config.num_thread);
		facereco_model->SetDebug(engine.m_config.debug);
		facereco_model->SetVerbose(engine.m_config.verbose);
		int ret = facereco_model->LoadModels(config.model_dir, config.model_path);
		if (ret == EAIF_FAILURE) {
			delete facereco_model;
			return nullptr;
		}
		return shared_ptr<eaif::Model>(facereco_model);
	}

#endif
#if 0
	if (strstr(config.model_name.c_str(), "human_classification") != nullptr) {
		HumanClassifyModel* hc_model = new HumanClassifyModel();
		hc_model->config = config;
		
	}
#endif
	return nullptr;
}
