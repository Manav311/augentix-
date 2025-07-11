#ifndef USE_NCNN
#ifndef LITE_SCRFD_H_
#define LITE_SCRFD_H_

#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <cassert>
#include <cstdint>

#ifndef USE_MICROLITE
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#else
#include "tensorflow/lite/micro/micro_interpreter.h"
#endif

#include "inf_adapter.h"
#include "inf_types.h"
#include "inf_model_internal.h"
#include "inf_face_internal.h"
#include "inf_utils_lite.h"

class LiteScrfdBase : public InfFaceDetect, protected LiteModelTraits
{
public:
	LiteScrfdBase(InfModelInfo* info)
	{
		m_config = info; // take ownership of info
		SetupConfig(info);
	}

	~LiteScrfdBase() override
	{
		ReleaseConfig(m_config);
		delete m_config;
	}

	void SetupConfig(InfModelInfo* conf);
	int LoadModels(const char* model_dir, const InfStrList* model_paths) override;

	int FaceDetect(const InfImage* img, std::vector<FaceBox>& result) override;
	int FaceDetect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, std::vector<FaceBox>& result) override;
	int Detect(const InfImage* img, InfDetList* result) override;
	int Detect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, InfDetList* result) override;

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
	QuantInfo m_qinfo[ScrfdMaxOutputSize];
	int m_output_dim[ScrfdMaxOutputSize][3];

	int m_output_prob_idx[5]{};
	int m_output_reg_idx[5]{};
	int m_output_landmark_idx[5]{};

private:
	template <typename Tbuffer>
	int TRunNet(const InfImage& img, const MPI_RECT_POINT_S* roi, std::vector<FaceBox>& face_list);
	int FaceDetect(const InfImage* img, const MPI_RECT_POINT_S& roi, std::vector<FaceBox>& result);
	void WriteDebugInfo(const MPI_RECT_POINT_S* roi, const Scaler& scale_factor,
	                    const std::vector<FaceBox>& face_list);

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

class LiteScrfd : public LiteScrfdBase
{
public:
	LiteScrfd(InfModelInfo* info)
	        : LiteScrfdBase(info)
	{
	}

	InfImage GetInputImage() override;
	void SetModelThreads(int nthread) override;

protected:
	bool PrepareInterpreter(const std::string& model_path) override;
	bool CollectModelInfo() override;
	void* InputTensorBuffer(size_t input_index) override
	{
		return m_model->input_tensor(input_index)->data.data;
	}
	void* OutputTensorBuffer(size_t output_index) override
	{
		return m_model->output_tensor(output_index)->data.data;
	}
	TfLiteStatus Invoke() override
	{
		return m_model->Invoke();
	}
	size_t GetArenaUsedBytes() override
	{
		return inf_tf_adapter::getArenaUsedBytes(*m_model);
	}

private:
	int GetOutputInfo();

#ifdef USE_MICROLITE
	friend int inf_tf_adapter::LiteScrfd_LoadModel(LiteScrfd& detector, const std::string& model_path,
	                                               std::unique_ptr<tflite::MicroInterpreter>& model,
	                                               std::unique_ptr<unsigned char[]>& model_fb);
	std::unique_ptr<unsigned char[]> m_model_fb;
	std::unique_ptr<tflite::MicroInterpreter> m_model;
	uint8_t m_arena[768 * 1024];
#else
	std::unique_ptr<tflite::Interpreter> m_model;
	std::unique_ptr<tflite::FlatBufferModel> m_model_fb;
#endif
};

#endif /* LITE_SCRFD_H_*/
#endif // USE_NCNN