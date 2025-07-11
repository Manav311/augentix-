#include "eaif_utils.h"
#include "eaif_data.h"
#include "eaif_trc.h"
#include "detect_model.h"


#include <vector>
#include <cstdlib>
#include <algorithm>

using namespace std;

/** Convert to detections -> filter conf + convert xywh + class_prob mul
 *
 * @param[in] detected_boxes buffer
 * @param[in] NMS config
 *
 * @return detections
 */
void ConvertToDetectionsV1(const float* detected_boxes,
                            const detect::NmsConfig& config,
                            vector<Detection>& detections)
{
    const size_t element_step = static_cast<size_t>(
         yolov5::box_ele + yolov5::prob_ele + config.num_classes);

    for (unsigned int i = 0; i < config.num_boxes; ++i)
    {
        const float* cur_box = &detected_boxes[i * element_step];
        if (cur_box[4] > config.confidence_threshold)
        {
            Detection det;
            det.box = {
                       cur_box[0] - cur_box[2] / 2.0f,
                       cur_box[1] - cur_box[3] / 2.0f,
                       cur_box[0] + cur_box[2] / 2.0f,
                       cur_box[1] + cur_box[3] / 2.0f
                   };
            det.confidence = cur_box[4];

            det.prob.resize(static_cast<size_t>(config.num_classes), 0);
            for (unsigned int c = 0; c < config.num_classes; ++c)
            {
                const float class_prob = det.confidence * cur_box[5 + c];
                if (class_prob > config.confidence_threshold)
                {
                    det.prob[c] = class_prob;
                }
            }
            //DEB("%d - [%.2f %.2f %.2f %.2f _ %.2f] [%.2f]", i, det.box.c[0], det.box.c[1], det.box.c[2], det.box.c[3], det.confidence, det.prob[0]);
            detections.emplace_back(std::move(det));
        }
    }
    //DEB("total candid %lu\n", detections.size());
    return;
}

void ConvertToDetectionsV2(const float* detected_boxes,
                           const detect::NmsConfig& config,
                           vector<Detection>& detections)
{
    const size_t element_step = static_cast<size_t>(
         yolov4::box_ele + yolov4::prob_ele + config.num_classes);

    for (unsigned int i = 0; i < config.num_boxes; ++i)
    {
        const float* cur_box = &detected_boxes[i * element_step];
        if (cur_box[4] > config.confidence_threshold)
        {
            Detection det;
            det.box = {
                       cur_box[0] - cur_box[2] / 2.0f,
                       cur_box[1] - cur_box[3] / 2.0f,
                       cur_box[0] + cur_box[2] / 2.0f,
                       cur_box[1] + cur_box[3] / 2.0f
                   };
            det.confidence =  cur_box[4];
            det.prob.resize(static_cast<size_t>(config.num_classes), 0);
            for (unsigned int c = 0; c < config.num_classes; ++c)
            {
                const float class_prob = cur_box[5 + c];
                if (class_prob > config.confidence_threshold) {
                    det.prob[c] = class_prob;
                }
            }
            //DEB("%d - [%.2f %.2f %.2f %.2f _ %.2f] [%.2f]", i, det.box.c[0], det.box.c[1], det.box.c[2], det.box.c[3], det.confidence, det.prob[0]);
            detections.emplace_back(std::move(det));
        }
    }
    //DEB("total candid %lu\n", detections.size());
    return;
}

/** Calculate Intersection Over Union of two boxes
 *
 * @param[in] box1 First box
 * @param[in] box2 Second box
 *
 * @return The IoU of the two boxes
 */
float IoU(const Rect<float>& box1, const Rect<float>& box2)
{
    const float area1 = (box1.ex - box1.sx) * (box1.ey - box1.sy);
    const float area2 = (box2.ex - box2.sx) * (box2.ey - box2.sy);
    float overlap;
    if (area1 <= 0 || area2 <= 0)
    {
        overlap = 0.0f;
    }
    else
    {
        const auto y_min_intersection = std::max<float>(box1.sy, box2.sy);
        const auto x_min_intersection = std::max<float>(box1.sx, box2.sx);
        const auto y_max_intersection = std::min<float>(box1.ey, box2.ey);
        const auto x_max_intersection = std::min<float>(box1.ex, box2.ex);
        const auto area_intersection =
            std::max<float>(y_max_intersection - y_min_intersection, 0.0f) *
            std::max<float>(x_max_intersection - x_min_intersection, 0.0f);
        overlap = area_intersection / (area1 + area2 - area_intersection);
    }
    return overlap;
}


void detect::NmsV1(const detect::NmsConfig& config,
         const float* detected_boxes,
         vector<Detection>& detections) {
    // Get detections that comply with the expected confidence threshold
    ConvertToDetectionsV1(detected_boxes, config, detections);

    const unsigned int num_detections = static_cast<unsigned int>(detections.size());
    for (unsigned int c = 0; c < config.num_classes; ++c)
    {
        // Sort classes
        std::sort(detections.begin(), detections.begin() + static_cast<std::ptrdiff_t>(num_detections),
                  [c](Detection& detection1, Detection& detection2)
                    {
                        return (detection1.prob[c] > detection2.prob[c]);
                    });
        // Clear detections with high IoU
        for (unsigned int d = 0; d < num_detections; ++d)
        {
            //DEB("%d [%.2f %.2f %.2f %.2f] _ %.2f %2.f", d, detections[d].box.c[0], detections[d].box.c[1], detections[d].box.c[2],
             //   detections[d].box.c[3], detections[d].confidence, detections[d].prob[c]);
            // Check if class is already cleared/invalidated
            if (detections[d].prob[c] == 0.f)
            {
                continue;
            }
            // Filter out boxes on IoU threshold
            const Rect<float>& box1 = detections[d].box;
            for (unsigned int b = d + 1; b < num_detections; ++b)
            {
                const Rect<float>& box2 = detections[b].box;
                if (IoU(box1, box2) > config.iou_threshold)
                {
                    detections[b].prob[c] = 0.f;
                }
            }
        }
    }
    return;
}


void detect::NmsV2(const detect::NmsConfig& config,
         const float* detected_boxes,
         vector<Detection>& detections) {
    // Get detections that comply with the expected confidence threshold
    ConvertToDetectionsV2(detected_boxes, config, detections);
    //DEB("config %d %d %.2f %.2f %d", config.num_classes, config.num_boxes, config.confidence_threshold, config.iou_threshold, detections.size());
    const unsigned int num_detections = static_cast<unsigned int>(detections.size());
    for (unsigned int c = 0; c < config.num_classes; ++c)
    {
        // Sort classes
        std::sort(detections.begin(), detections.begin() + static_cast<std::ptrdiff_t>(num_detections),
                  [c](Detection& detection1, Detection& detection2)
                    {
                        return (detection1.prob[c] > detection2.prob[c]);
                    });
        // Clear detections with high IoU
        for (unsigned int d = 0; d < num_detections; ++d)
        {
            //DEB("%d [%.2f %.2f %.2f %.2f] _ %.2f %2.f", d, detections[d].box.c[0], detections[d].box.c[1], detections[d].box.c[2],
             //   detections[d].box.c[3], detections[d].confidence, detections[d].prob[c]);
            // Check if class is already cleared/invalidated
            if (detections[d].prob[c] == 0.f)
            {
                continue;
            }
            // Filter out boxes on IoU threshold
            const Rect<float>& box1 = detections[d].box;
            for (unsigned int b = d + 1; b < num_detections; ++b)
            {
                const Rect<float>& box2 = detections[b].box;
                //DEB("%d [%.2f %.2f %.2f %.2f] vs %d [%.2f %.2f %.2f %.2f] => iou:%.4f ",
                //    d, box1.c[0], box1.c[1], box1.c[2], box1.c[3], b, box2.c[0], box2.c[1], box2.c[2], box2.c[3],
                //    iou(box1, box2));
                if (IoU(box1, box2) > config.iou_threshold)
                {
                    detections[b].prob[c] = 0.f;
                }
            }
        }
    }
    return;
}

// deprecated
void NmsV2_1(const detect::NmsConfig& config,
         const float* detected_boxes,
         vector<Detection>& detections)
{
    // Get detections that comply with the expected confidence threshold
    ConvertToDetectionsV2(detected_boxes, config, detections);
    //DEB("config %d %d %.2f %.2f %d", config.num_classes, config.num_boxes, config.confidence_threshold, config.iou_threshold, detections.size());
    const unsigned int num_detections = static_cast<unsigned int>(detections.size());

    for (unsigned int n = 0; n < num_detections; ++n)
    {
        int any = 0;
        for (unsigned int c = 0; c < config.num_classes; ++c)
            any |= detections[n].prob[c] > 0;
        if (!any)
            continue;

        const Rect<float>& box1 = detections[n].box;
        for (unsigned int j = n+1; j < num_detections; ++j)
        {
            const Rect<float>& box2 = detections[j].box;
            if (IoU(box1, box2) > config.iou_threshold)
            {
                for (unsigned int k = 0; k < config.num_classes; ++k)
                {
                    if (detections[n].prob[k] < detections[j].prob[k])
                        detections[n].prob[k] = 0.0f;
                    else
                        detections[j].prob[k] = 0.0f;
                }
            }
        }
    }

    return;
}

void detect::ScaleCoord(vector<Detection>& detections, const Shape& img_sz_in, const Shape& img_sz_out)
{
    float ratio_w = static_cast<float>(img_sz_out.w) / img_sz_in.w;
    float ratio_h = static_cast<float>(img_sz_out.h) / img_sz_in.h;
    //printf("%.2f %.2f \n", ratio_w, ratio_h);
    for (unsigned int i = 0; i < detections.size(); ++i) {
        auto& det = detections[i];
        det.box.sx *= ratio_w;
        det.box.ex *= ratio_w;
        det.box.sy *= ratio_h;
        det.box.ey *= ratio_h;
    }
}

void detect::ScaleCoord(Detection& det, const Shape& img_sz_in, const Shape& img_sz_out)
{
    float ratio_w = static_cast<float>(img_sz_out.w) / img_sz_in.w;
    float ratio_h = static_cast<float>(img_sz_out.h) / img_sz_in.h;
    det.box.sx *= ratio_w;
    det.box.ex *= ratio_w;
    det.box.sy *= ratio_h;
    det.box.ey *= ratio_h;
}

static inline void ScaleCoord(Detection& det, float ratio_w, float ratio_h)
{
    det.box.sx *= ratio_w;
    det.box.ex *= ratio_w;
    det.box.sy *= ratio_h;
    det.box.ey *= ratio_h;
}


static void FilterOutput(std::vector<Detection>& dets, std::vector<Detection>& detections, const Shape& img_sz_in, const Shape& img_sz_out)
{
    float ratio_w = static_cast<float>(img_sz_out.w) / img_sz_in.w;
    float ratio_h = static_cast<float>(img_sz_out.h) / img_sz_in.h;

    for (auto det : dets) {
        for (unsigned int c = 0; c < det.prob.size(); ++c) {
            if (det.prob[c] != 0.0f) {
                ScaleCoord(det, ratio_w, ratio_h);
                detections.push_back(det.copy());
                break;
                //printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.2f label:%s\n",
                //++i, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence,
                //yolov5.dataset_ == data::COCO ? data::coco::labels[c].c_str() : data::voc::labels[c].c_str());
            }
        }
    }
}

static void SortDetection(const std::vector<Detection>& in_dets, const std::vector<int> indexes, std::vector<Detection>& out_dets)
{
    if (indexes.size() == 0)
        return;

    for (auto idx : indexes) {
        if (idx < 0)
            break;
        auto det = in_dets[idx];
        for (unsigned int a = 0; a < det.prob.size(); ++a) {
            if (det.prob[a] != 0.0f) {
                Detection new_det{det.box, det.prob[a], {}, vector<int>(1,static_cast<int>(a))};
                out_dets.push_back(new_det);
            }
        }
    }
    std::sort(out_dets.begin(), out_dets.end(),
                  [](Detection& det1, Detection& det2)
                    {
                        return (det1.confidence > det2.confidence);
                    });
}


static void FilterOutputV2(std::vector<Detection>& dets, std::vector<Detection>& detections, const Shape& img_sz_in, const Shape& img_sz_out)
{
    float ratio_w = static_cast<float>(img_sz_out.w) / img_sz_in.w;
    float ratio_h = static_cast<float>(img_sz_out.h) / img_sz_in.h;
    std::vector<int> indexes(dets.size(),-1);
    int i = 0;
    int j = 0;
    for (auto& det : dets) {
        for (unsigned int c = 0; c < det.prob.size(); ++c) {
            if (det.prob[c] != 0.0f) {
                ScaleCoord(det, ratio_w, ratio_h);
                indexes[j++] = i;
                break;
                //printf("%d detection pos:[%.2f %.2f %.2f %.2f] prob:%.2f label:%s\n",
                //++i, det.box.sx, det.box.sy, det.box.ex, det.box.ey, det.confidence,
                //yolov5.dataset_ == data::COCO ? data::coco::labels[c].c_str() : data::voc::labels[c].c_str());

            }
        }
        i++;
    }
    SortDetection(dets, indexes, detections);
}


void detect::PostProcessV1(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh, const Shape& img_sz_net, const Shape& img_sz_raw, std::vector<Detection>& detections)
{
    std::vector<Detection> dets;
    //printf("dump size (%d+%d)*%d*%d >>>>>> %d\n", num_class, 5, num_output, sizeof(float), (num_class+5)*num_output*sizeof(float));
    //    dump((const uint8_t*)output, (num_class+5)*num_output*sizeof(float), 0);
    detect::NmsConfig config = {static_cast<uint32_t>(num_classes), static_cast<uint32_t>(num_output), conf_thresh, iou_thresh};
    NmsV1(config, output, dets);
    FilterOutput(dets, detections, img_sz_net, img_sz_raw);
}

void detect::PostProcessV1_1(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh, const Shape& img_sz_net, const Shape& img_sz_raw, std::vector<Detection>& detections)
{
    std::vector<Detection> dets;
    detect::NmsConfig config = {static_cast<uint32_t>(num_classes), static_cast<uint32_t>(num_output), conf_thresh, iou_thresh};
    NmsV1(config, output, dets);
    FilterOutputV2(dets, detections, img_sz_net, img_sz_raw);
}

void detect::PostProcessV2(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh, const Shape& img_sz_net, const Shape& img_sz_raw, std::vector<Detection>& detections)
{
    std::vector<Detection> dets;
    detect::NmsConfig config = {static_cast<uint32_t>(num_classes), static_cast<uint32_t>(num_output), conf_thresh, iou_thresh};
    NmsV2(config, output, dets);
    FilterOutput(dets, detections, img_sz_net, img_sz_raw);
    //ScaleCoord(detections, img_sz_net, img_sz_raw);
}

void detect::PostProcessV2_1(const float *output, int num_output, int num_classes, float conf_thresh, float iou_thresh, const Shape& img_sz_net, const Shape& img_sz_raw, std::vector<Detection>& detections)
{
    //int num_class = data::getNumClass(dataset);
    std::vector<Detection> dets;
    detect::NmsConfig config = {static_cast<uint32_t>(num_classes), static_cast<uint32_t>(num_output), conf_thresh, iou_thresh};
    NmsV2(config, output, dets);
    FilterOutputV2(dets, detections, img_sz_net, img_sz_raw);
    //ScaleCoord(detections, img_sz_net, img_sz_raw);
}
