#ifndef HUMAN_CLASSIFY_MODEL_H_
#define HUMAN_CLASSIFY_MODEL_H_

#include <memory>
#include <string>
#include <vector>

#include "eaif_common.h"
#include "eaif_model.h"
#include "classify_model.h"

class HumanClassifyModel : public eaif::Model {
    public:
	HumanClassifyModel(){};
	~HumanClassifyModel() override{};
	int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                    std::vector<Classification> &results, const ModelConfig &conf) override;
	int Classify(const void *WImg, Classification &result, const ModelConfig &conf) override;

	void SetDebug(int debug) override{};

	void SetVerbose(int verbose) override{};

	void SetClassifierDL(std::shared_ptr<eaif::Model> classifier)
	{
		m_classifier_dl = classifier;
	};
	void SetClassifierCV(std::shared_ptr<eaif::Model> classifier)
	{
		m_classifier_cv = classifier;
	}

	static const int DLModelIndex = 0;
	static const int CVModelIndex = 1;
	static const int DLConfIndex = 2;
	static const int CVConfIndex = 3;

    private:
	std::shared_ptr<eaif::Model> m_classifier_dl;
	std::shared_ptr<eaif::Model> m_classifier_cv;
	ModelConfig m_conf_dl;
	ModelConfig m_conf_cv;
};

#endif /* HUMAN_CLASSIFIER_H_ */