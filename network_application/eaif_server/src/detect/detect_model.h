#ifndef DETECT_MODEL_H_
#define DETECT_MODEL_H_

#include <string>
#include <vector>

#include "eaif_common.h"
#include "eaif_data.h"
#include "eaif_model.h"

class DetectModel {
    public:
	DetectModel(void)
	{
		postprocess_method_ = 10;
		m_type = Eaif32F;
		m_verbose = 0;
		m_nthread = -1;
	}
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) = 0;
	virtual int Detect(const void *img, std::vector<Detection> &detections, const ModelConfig &conf) = 0;
	virtual void SetDebug(int debug){};
	virtual void SetCpuType(int cpu_type){};
	virtual ~DetectModel(){};

	void SetVerbose(int verbose)
	{
		m_verbose = verbose;
	}
	void SetPostProcessMethod(int method)
	{
		postprocess_method_ = method;
	}
	int GetPostProcessMethod(void)
	{
		return postprocess_method_;
	}
	void SetNumThreads(int nthread)
	{
		m_nthread = nthread;
	}

	int postprocess_method_;
	EaifDataType m_type;

    protected:
	int m_verbose;
	int m_nthread;
};

namespace detect
{
struct NmsConfig {
	NmsConfig(unsigned int classes, unsigned int boxes, float confidence, float iou)
	        : num_classes(classes)
	        , num_boxes(boxes)
	        , confidence_threshold(confidence)
	        , iou_threshold(iou){};

	unsigned int num_classes{ 0 }; /**< Number of classes in the detected boxes */
	unsigned int num_boxes{ 0 }; /**< Number of detected boxes */
	float confidence_threshold{ 0.8f }; /**< Inclusion confidence threshold for a box */
	float iou_threshold{ 0.8f }; /**< Inclusion threshold for Intersection-Over-Union */
};

// for yolov5
void PostProcessV1(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh,
                   const Shape &img_sz_net, const Shape &img_sz_raw, std::vector<Detection> &detections);
// for yolov4
void PostProcessV2(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh,
                   const Shape &img_sz_net, const Shape &img_sz_raw, std::vector<Detection> &detections);
// for yolov5 (sorted)
void PostProcessV1_1(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh,
                     const Shape &img_sz_net, const Shape &img_sz_raw, std::vector<Detection> &detections);
// for yolov4 (sorted)
void PostProcessV2_1(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh,
                     const Shape &img_sz_net, const Shape &img_sz_raw, std::vector<Detection> &detections);

enum ModelType { YOLO, SSD, EFF };

void NmsV1(const NmsConfig &config, const float *detected_boxes, std::vector<Detection> &detections);

void NmsV2(const NmsConfig &config, const float *detected_boxes, std::vector<Detection> &detections);

void ScaleCoord(std::vector<Detection> &detections, const Shape &img_sz_in, const Shape &img_sz_out);
void ScaleCoord(Detection &det, const Shape &img_sz_in, const Shape &img_sz_out);

} // namespace detect

namespace yolov5
{
constexpr int pos_x = 0;
constexpr int pos_y = 1;
constexpr int pos_w = 2;
constexpr int pos_h = 3;
constexpr int pos_prob = 4;
constexpr int box_ele = 4;
constexpr int prob_ele = 1;
constexpr int cls_ele = 80;
} // namespace yolov5

namespace yolov4
{
constexpr int pos_x = 0;
constexpr int pos_y = 1;
constexpr int pos_w = 2;
constexpr int pos_h = 3;
constexpr int pos_prob = 4;
constexpr int box_ele = 4;
constexpr int prob_ele = 1;
constexpr int cls_ele = 80;
} // namespace yolov4

#endif /* DETECT_MODEL_H_ */
