#ifndef FACENET_COMMON_H_
#define FACENET_COMMON_H_

#include <vector>
#include <stdint.h>

#include "eaif_common.h"

#define NMS_UNION 1
#define NMS_MIN 2

template <typename T>
void NmsBoxes(std::vector<FaceBox<T> > &input, T threshold, int type, std::vector<FaceBox<T> > &output);

void RegressBoxes(std::vector<FaceBox<int> > &rects, const QuantInfo &info);
void RegressBoxes(std::vector<FaceBox<float> > &rects);

void ProcessBoxes(std::vector<FaceBox<int> > &input, int img_h, int img_w, std::vector<FaceBox<int> > &rects,
                  const QuantInfo &info, int nms_th);
void ProcessBoxes(std::vector<FaceBox<float> > &input, int img_h, int img_w, std::vector<FaceBox<float> > &rects,
                  float nms_th);

template <typename T> void SquareBoxes(std::vector<FaceBox<T> > &rects);

template <typename T> void Padding(int img_h, int img_w, std::vector<FaceBox<T> > &rects);

void GenerateBoundingBox(const QuantInfo &info_conf, const uint8_t *confidence_data, int confidence_size,
                         const QuantInfo &info_reg, const uint8_t *reg_data, float scale, float threshold,
                         int feature_h, int feature_w, std::vector<FaceBox<float> > &output, bool transposed);

void GenerateBoundingBox(const QuantInfo &info_conf, const int8_t *confidence_data, int confidence_size,
                         const QuantInfo &info_reg, const int8_t *reg_data, float scale, float threshold, int feature_h,
                         int feature_w, std::vector<FaceBox<float> > &output, bool transposed);

void GenerateBoundingBox(const float *confidence_data, int confidence_size, const float *reg_data, float scale,
                         float threshold, int feature_h, int feature_w, std::vector<FaceBox<float> > &output,
                         bool transposed);

void GenerateBoundingBox(const float *confidence_data, int confidence_size, const float *reg_data, double scale,
                         double threshold, int feature_h, int feature_w, std::vector<FaceBox<double> > &output,
                         bool transposed);

void SetInputBuffer(std::vector<void *> input_channels, float *input_data, const int height, const int width);

template <typename T>
void CalPyramidList(int height, int width, int min_size, float factor, std::vector<ScaleWindow<T> > &list);

template <typename T> void CalLandmark(std::vector<FaceBox<T> > &box_list);

template <typename T> void SetBoxBound(std::vector<FaceBox<T> > &box_list, int img_h, int img_w);

void TransformResult(const std::vector<FaceBox<int> > &iface_list, std::vector<FaceBox<float> > &face_list,
                     const QuantInfo &box_info, const QuantInfo &score_info, const QuantInfo &reg_info);
void TransformResult(const std::vector<FaceBox<float> > &face_list, std::vector<Detection> &detections,
                     float resize_ratio = 1.0f);
void TransformResult(const std::vector<FaceBox<int> > &face_list, std::vector<Detection> &detections,
                     const QuantInfo &info, float resize_ratio = 1.0f);

/* get current time: in us */
unsigned long GetCurTime(void);

#endif /* !FACENET_COMMON_H_ */
