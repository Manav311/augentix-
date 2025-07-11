#ifndef YOLO_MODEL_
#define YOLO_MODEL_

#include <memory>
#include <vector>

#include "eaif_common.h"
#include "eaif_model.h"
#include "detect_model.h"

class Yolov5Model : public eaif::Model {
    public:
	Yolov5Model()
	        : model(nullptr)
	        , num_thread_(1)
	        , m_verbose(0)
	        , m_debug(0){};
	virtual ~Yolov5Model(){};
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Detect(const void *WImg, std::vector<Detection> &detections, const ModelConfig &conf) override;
	virtual void SetDebug(int debug) override
	{
		m_debug = debug;
		if (model)
			model->SetDebug(debug);
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

	std::unique_ptr<DetectModel> model;
	int num_thread_;
	int m_verbose;
	int m_debug;
};

#endif /* YOLO_MODEL_ */