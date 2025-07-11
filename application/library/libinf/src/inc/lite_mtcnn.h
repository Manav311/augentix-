#ifndef USE_NCNN
#ifndef LITE_MTCNN_H_
#define LITE_MTCNN_H_

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

#include "inf_types.h"

#include "inf_model_internal.h"
#include "inf_face_internal.h"
#include "inf_adapter.h"

class LiteMtcnn : public InfFaceDetect
{
public:
	LiteMtcnn()
	{
		SetFactorMinSize(40, 0.709);
		SetThreshold(0.6, 0.7, 0.9);
		std::vector<float> nms_th{ 0.5, 0.7, 0.7 };
		SetNmsThreshold(0.7, nms_th);
	}

	~LiteMtcnn(void) override;

	int FaceDetect(const InfImage *img, std::vector<FaceBox> &result) override;
	int FaceDetect(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, std::vector<FaceBox> &result) override;
	int Detect(const InfImage *img, InfDetList *result) override;
	int Detect(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfDetList *result) override;

	int LoadModels(const char *model_dir, const InfStrList *model_paths) override;

	void SetModelThreads(int nthread) override;

	void SetupConfig(const InfModelInfo *conf)
	{
		SetFactorMinSize(conf->window_min_size, conf->window_scale_factor);
		SetThreshold(conf->conf_thresh.data[0], conf->conf_thresh.data[1], conf->conf_thresh.data[2]);
		SetNmsThreshold(conf->iou_thresh, conf->nms_internal_thresh);
		//SetNormParam(conf.zeros.data, conf.stds.data);
		if (m_verbose)
			inf_log_notice("MTCNN thr %.4f,%.4f,%.4f inter nms %.4f->%.4f,%.4f %.4f\n", m_pnet_threshold,
			               m_rnet_threshold, m_onet_threshold, m_inter_nms_th[0], m_inter_nms_th[1],
			               m_inter_nms_th[2], m_nms_th);
	}

	void SetFactorMinSize(int min_size, float factor)
	{
		assert(factor > 0.0f);
		assert(min_size > 5);
		m_factor = factor;
		m_min_size = min_size;
	}

	void SetThreshold(float p, float r, float o)
	{
		m_pnet_threshold = p;
		m_rnet_threshold = r;
		m_onet_threshold = o;
	}

	void SetNmsThreshold(float nms_th, const std::vector<float> &inter_nms_th)
	{
		assert(nms_th > 0.0f);
		m_nms_th = nms_th;

		int nms_size = inter_nms_th.size();
		m_inter_nms_th.resize(nms_size);
		for (int i = 0; i < nms_size; i++) {
			assert(inter_nms_th[i] > 0.0f);
			m_inter_nms_th[i] = inter_nms_th[i];
		}
	}

	void SetNmsThreshold(float nms_th, const InfFloatList &inter_nms_th)
	{
		assert(nms_th > 0.0f);
		m_nms_th = nms_th;

		int nms_size = inter_nms_th.size;
		m_inter_nms_th.resize(nms_size);
		for (int i = 0; i < nms_size; i++) {
			assert(inter_nms_th.data[i] > 0.0f);
			m_inter_nms_th[i] = inter_nms_th.data[i];
		}
	}

	// void SetNormParam(const float zeros[3], const float stds[3])
	// {
	// 	m_zero[0] = zeros[0];
	// 	m_zero[1] = zeros[1];
	// 	m_zero[2] = zeros[2];

	// 	m_scale[0] = stds[0];
	// 	m_scale[1] = stds[1];
	// 	m_scale[2] = stds[2];
	// }

protected:
	template <typename IC>
	void GetQuantInfo(const std::unique_ptr<IC> &model, QuantInfo *info, int net_idx, int prob_num, int coord_num,
	                  int landmark_num);
	int RunPNet(const InfImage *img, ScaleWindow &win, std::vector<FaceBox> &box_list);
	int RunRNet(const InfImage *img, std::vector<FaceBox> &pnet_boxes, std::vector<FaceBox> &output_boxes);
	int RunONet(const InfImage *img, std::vector<FaceBox> &rnet_boxes, std::vector<FaceBox> &output_boxes);

	template <typename Tbuffer, InfDataType mtype>
	int TRunRNet(const InfImage *img, std::vector<FaceBox> &pnet_boxes, std::vector<FaceBox> &output_boxes);
	template <typename Tbuffer, InfDataType mtype>
	int TRunONet(const InfImage *img, std::vector<FaceBox> &rnet_boxes, std::vector<FaceBox> &output_boxes);

	// int RunNetwork(tflite::Interpreter* interpreter, const InfImage* img, const Shape& Size, int chn);
	// int RunNetwork(tflite::Interpreter* interpreter, const InfImage* img, const Shape& Size, FaceBox& box, int chn);

private:
#ifdef USE_MICROLITE
	friend int inf_tf_adapter::LiteMtcnn_LoadPnetModel(LiteMtcnn &detector, const std::string &model_path,
	                                                   std::unique_ptr<tflite::MicroInterpreter> &model,
	                                                   std::unique_ptr<unsigned char[]> &model_fb);
	friend int inf_tf_adapter::LiteMtcnn_LoadRnetModel(LiteMtcnn &detector, const std::string &model_path,
	                                                   std::unique_ptr<tflite::MicroInterpreter> &model,
	                                                   std::unique_ptr<unsigned char[]> &model_fb);
	friend int inf_tf_adapter::LiteMtcnn_LoadOnetModel(LiteMtcnn &detector, const std::string &model_path,
	                                                   std::unique_ptr<tflite::MicroInterpreter> &model,
	                                                   std::unique_ptr<unsigned char[]> &model_fb);
	std::unique_ptr<unsigned char[]> m_pnet_model_fb;
	std::unique_ptr<tflite::MicroInterpreter> m_pnet_model;
	uint8_t m_pnet_arena[768 * 1024];
	std::unique_ptr<unsigned char[]> m_rnet_model_fb;
	std::unique_ptr<tflite::MicroInterpreter> m_rnet_model;
	uint8_t m_rnet_arena[768 * 1024];
	std::unique_ptr<unsigned char[]> m_onet_model_fb;
	std::unique_ptr<tflite::MicroInterpreter> m_onet_model;
	uint8_t m_onet_arena[768 * 1024];
#else
	std::unique_ptr<tflite::Interpreter> m_pnet_model;
	std::unique_ptr<tflite::Interpreter> m_rnet_model;
	std::unique_ptr<tflite::Interpreter> m_onet_model;
	std::unique_ptr<tflite::FlatBufferModel> m_pnet_model_fb;
	std::unique_ptr<tflite::FlatBufferModel> m_rnet_model_fb;
	std::unique_ptr<tflite::FlatBufferModel> m_onet_model_fb;
#endif

	QuantInfo m_pnet_info[2];
	QuantInfo m_rnet_info[2];
	QuantInfo m_onet_info[3];

	float m_pnet_threshold;
	float m_rnet_threshold;
	float m_onet_threshold;
	float m_nms_th;
	float m_factor;
	float m_min_size;
	std::vector<float> m_inter_nms_th;

	const int *m_input_dim[3];

	int m_output_prob_idx[3];
	int m_output_reg_idx[3];
	int m_output_landmark_idx;

	const int m_pnet_index = 0;
	const int m_rnet_index = 1;
	const int m_onet_index = 2;
};

#endif /* INF_MTCNN_H_*/
#endif // USE_NCNN