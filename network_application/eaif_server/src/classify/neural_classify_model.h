#ifndef NEURAL_CLASSIFY_MODEL_
#define NEURAL_CLASSIFY_MODEL_

#include <memory>
#include <vector>

#include "eaif_common.h"
#include "eaif_model.h"

#include "classify_model.h"

class NeuralClassifyModel : public eaif::Model {
    public:
	NeuralClassifyModel()
	        : model(nullptr)
	        , num_thread_(0)
	        , m_verbose(0)
	        , m_debug(0){};
	virtual ~NeuralClassifyModel() override{};
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                            std::vector<Classification> &results, const ModelConfig &conf) override;
	virtual int Classify(const void *WImg, Classification &result, const ModelConfig &conf) override;
	virtual void SetDebug(int debug) override
	{
		m_debug = debug;
		if (model)
			model->SetDebug(m_debug);
	};
	virtual void SetVerbose(int verbose) override
	{
		m_verbose = verbose;
		if (model)
			model->SetVerbose(verbose);
	};
	void SetNumThreads(int nthread)
	{
		num_thread_ = nthread;
	};
	std::unique_ptr<ClassifyModel> model;
	int num_thread_;
	int m_verbose;
	int m_debug;
};

#endif /* NEURAL_CLASSIFY_MODEL_ */