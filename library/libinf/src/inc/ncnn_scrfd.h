#ifdef USE_NCNN
#ifndef NCNN_SCRFD_H_
#define NCNN_SCRFD_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <cassert>
#include <cstdint>

#include "net.h" // ncnn

#include "inf_adapter.h"
#include "inf_types.h"
#include "inf_model_internal.h"
#include "inf_face_internal.h"

class NcnnScrfd : public InfFaceDetect
{
public:
	NcnnScrfd(InfModelInfo* info)
	{
		m_config = info; // take ownership of info
		SetupConfig(info);
	}

	~NcnnScrfd() override
	{
		ReleaseConfig(m_config);
		delete m_config;
	}

	void SetupConfig(InfModelInfo* conf);
	int LoadModels(const char* model_dir, const InfStrList* model_paths) override;
	InfImage GetInputImage() override;
	int FaceDetect(const InfImage* img, std::vector<FaceBox>& result) override;
	int FaceDetect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, std::vector<FaceBox>& result) override;
	int Detect(const InfImage* img, InfDetList* result) override;
	int Detect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, InfDetList* result) override;
	void *InputTensorBuffer(size_t input_index)
	{
		return in_Mat.data;
	}
	void *OutputTensorBuffer(size_t output_index)
	{
		return out_Mat.data;
	}

protected:
	static constexpr int ScrfdMaxOutputSize = 10;

	int RunNetwork(const InfImage& img, const Shape& size, int chn, InfDataType mtype, const MPI_RECT_POINT_S* roi,
	               const Pads* pads, int verbose);

	int m_input_dim[3];

	/* describe Scrfd output */
	int m_feature_output_pairs = 0; // fmc
	int m_num_anchors_per_feature = 0; // num_anchors
	int m_use_kps = 0; // use_kps
	int m_feature_stride[10]; // feat_stride_fpn

	/* ascending dimension of feature output index */
	/* need to parse output index*/
	int m_output_dim[ScrfdMaxOutputSize][3];

	int m_output_prob_idx[5]{};
	int m_output_reg_idx[5]{};
	int m_output_landmark_idx[5]{};

	ncnn::Net m_model;

private:
	int TRunNet(const InfImage& img, const MPI_RECT_POINT_S* roi, std::vector<FaceBox>& face_list);
	int FaceDetect(const InfImage* img, const MPI_RECT_POINT_S& roi, std::vector<FaceBox>& result);
	void WriteDebugInfo(const MPI_RECT_POINT_S* roi, const Scaler& scale_factor,
	                    const std::vector<FaceBox>& face_list);
	int GetOutputInfo();

	ncnn::Mat in_Mat;
	ncnn::Mat out_Mat;
	/* postprocessing step
	1. inf -> output -> unquant
	vector<bbox> boxes;
	for i in prediction(scores, bbox)
		if scores[i] > conf_thresh:
			2. bbox_pre * stride
			3. generate anchor-center meshgrid x,y
			3.a (input_height // stride) X (input_width // stride)
			3.b array number is multiplied by num_anchor of 3.a
			4. distance2bbox(anchor_center, prediction box)
			4.a x1 = anchor-center_i_x - box_0
			4.b y1 = anchor-center_i_y - box_1
			4.c x2 = anchor-center_i_x + box_2
			4.d y2 = anchor-center_i_y + box_3
			boxes.push_back([x1,y1,x2,y2,score])
	5. nms(boxes)
	5.a sort scores
	5.b for each box
	5.c     calc iou score
	5.d     if iou score < thresh: keep
	           -> else filter.
	return
	*/
};

#endif /* NCNN_SCRFD_H_*/
#endif // USE_NCNN