#ifndef ARMNN_YOLO_MODEL_H_
#define ARMNN_YOLO_MODEL_H_
#ifdef USE_ARMNN

#include <string>
#include <vector>

#include "detect_model.h"
#include "eaif_utils.h"

class ArmnnYolov5Model : public DetectModel {
    public:
	ArmnnYolov5Model() = default;
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf) override;
	virtual ~ArmnnYolov5Model(){};

	void SetCpuType(int cpu_type)
	{
		model_.cpu_infer_type = cpu_type;
	};
	void SetDebug(int debug)
	{
		model_.set_profile = debug;
	};

	armnn::utils::ArmnnModel model_;
	std::vector<QuantInfo> model_info_;
	int input_dim_[2];
	int output_dim_[2];
	int type_;
};

class ArmnnYolov4Model : public DetectModel {
    public:
	ArmnnYolov4Model() = default;
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf) override;
	virtual ~ArmnnYolov4Model(){};

	void SetCpuType(int cpu_type)
	{
		model_.cpu_infer_type = cpu_type;
	};
	void SetDebug(int debug)
	{
		model_.set_profile = debug;
	};
	armnn::utils::ArmnnModel model_;
	std::vector<QuantInfo> model_info_;
	int input_dim_[2];
	int output_dim_[2];
	int type_;
};

#endif /* !USE_ARMNN */
#endif /* !ARMNN_YOLO_MODEL_H_ */
