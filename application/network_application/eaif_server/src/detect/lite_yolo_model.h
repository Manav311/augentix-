#ifndef LITE_YOLO_MODEL_H_
#define LITE_YOLO_MODEL_H_
#ifdef USE_TFLITE

#include <memory>
#include <string>
#include <vector>

#include "detect_model.h"
#include "eaif_common.h"
#include "eaif_utils.h"

class LiteYolov5Model : public DetectModel {
    public:
	LiteYolov5Model() = default;
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf) override;
	virtual ~LiteYolov5Model(){};

	std::unique_ptr<tflite::Interpreter> model_;
	std::unique_ptr<tflite::FlatBufferModel> model_fb_;

	QuantInfo model_info_;
	int input_dim_[2];
	int output_dim_[2];
	int type_;
};

class LiteYolov4Model : public DetectModel {
    public:
	LiteYolov4Model() = default;
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf) override;
	virtual ~LiteYolov4Model(){};
	std::unique_ptr<tflite::Interpreter> model_;
	std::unique_ptr<tflite::FlatBufferModel> model_fb_;

	QuantInfo model_info_;
	int input_dim_[2];
	int output_dim_[2];
	int type_;
};

#endif /* !USE_TFLITE */
#endif /* !LITE_YOLO_MODEL_H_ */
