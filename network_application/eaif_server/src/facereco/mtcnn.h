#ifndef MTCNN_H_
#define MTCNN_H_

#include <iostream>
#include <string>
#include <vector>

#include <assert.h>
#include <stdint.h>

#include "eaif_common.h"
#include "eaif_model.h"

class Mtcnn {
    public:
	Mtcnn(void)
	{
		SetFactorMinSize(40, 0.709);
		SetThreshold(0.6, 0.7, 0.9);
		std::vector<float> nms_th{0.5, 0.7, 0.7};
		SetNmsThreshold(0.7, nms_th);
	};

	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) = 0;
	virtual int FaceDetect(const void *img, std::vector<FaceBox<int> > &face_list, const ModelConfig &config) = 0;
	virtual int FaceDetect(const void *img, std::vector<FaceBox<float> > &face_list, const ModelConfig &config) = 0;
	virtual const QuantInfo &GetOutputQuantInfo(int index) const
	{
		return empty;
	};
	virtual ~Mtcnn(void){};
	virtual void SetDebug(int debug)
	{
		m_debug = debug;
	};

	EaifDataType GetModelType(void)
	{
		return m_type;
	};

	void SetNumThreads(int nthread)
	{
		m_num_thread = nthread;
	}

	void SetVerbose(int verbose)
	{
		m_verbose = verbose;
	}

	void SetupConfig(const ModelConfig &conf)
	{
		SetFactorMinSize(conf.window_min_size, conf.window_scale_factor);
		SetThreshold(conf.conf_thresh[0], conf.conf_thresh[1], conf.conf_thresh[2]);
		SetNmsThreshold(conf.iou_thresh, conf.nms_internal_thresh);
		SetNormParam(conf.zeros.data(), conf.stds.data());
		if (m_verbose)
			eaif_info_h("MTCNN thr %.4f,%.4f,%.4f inter nms %.4f->%.4f,%.4f %.4f\n",
				pnet_thresholdf_,
				rnet_thresholdf_,
				onet_thresholdf_,
				inter_nms_thf_[0],
				inter_nms_thf_[1],
				inter_nms_thf_[2],
				nms_thf_);
	}

    private:
	void SetNormParam(const float zeros[3], const float stds[3])
	{
		zero_[0] = zeros[0];
		zero_[1] = zeros[1];
		zero_[2] = zeros[2];
		scale_[0] = stds[0];
		scale_[1] = stds[1];
		scale_[2] = stds[2];
	}
	void SetThreshold(float p, float r, float o)
	{
		pnet_thresholdf_ = p;
		rnet_thresholdf_ = r;
		onet_thresholdf_ = o;
		pnet_threshold_ = EAIF_FR_BIT_U8_VAL * p; // 0.6;
		rnet_threshold_ = EAIF_FR_BIT_U8_VAL * r; // 0.7;
		onet_threshold_ = EAIF_FR_BIT_U8_VAL * o; // 0.9;
	}

	void SetNmsThreshold(float nms_th, const std::vector<float>& inter_nms_th)
	{
		assert(nms_th > 0.0f);

		nms_thf_ = nms_th;
		nms_th_ = EAIF_FR_BIT_U8_VAL * nms_th; // 0.7

		int nms_size = inter_nms_th.size();
		inter_nms_thf_.resize(nms_size);
		inter_nms_th_.resize(nms_size);
		for (int i = 0; i < nms_size; i++) {
			assert(inter_nms_th[i] > 0.0f);
			inter_nms_thf_[i] = inter_nms_th[i];
			inter_nms_th_[i] = EAIF_FR_BIT_U8_VAL * inter_nms_th[i]; // 0.5
		}
	}

	void SetFactorMinSize(int min_size, float factor)
	{
		assert(factor > 0.0f);
		assert(min_size > 5);
		factor_ = factor;
		min_size_ = min_size;
	}

    protected:
	int pnet_threshold_;
	int rnet_threshold_;
	int onet_threshold_;
	int nms_th_;
	std::vector<int> inter_nms_th_;

	const int pnet_index_ = 0;
	const int rnet_index_ = 1;
	const int onet_index_ = 2;

	float pnet_thresholdf_;
	float rnet_thresholdf_;
	float onet_thresholdf_;
	float nms_thf_;
	std::vector<float> inter_nms_thf_;

	int min_size_ = 40;
	float factor_ = 0.709;

	float zero_[3]; // = 127.5;
	float scale_[3]; // = 0.0078125;

	QuantInfo empty;

	EaifDataType m_type; // model type (quant / floating inference)
	int m_debug;
	int m_num_thread;
	int m_verbose;
};

#endif /* MTCNN_H_*/
