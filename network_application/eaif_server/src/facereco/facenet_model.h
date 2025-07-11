#ifndef FACENET_MODEL_H_
#define FACENET_MODEL_H_

#include <vector>

#include <stdint.h>

#include "eaif_common.h"
#include "eaif_model.h"

class FacenetModel {
    public:
	FacenetModel()
	        : encode_dim_(0){};
	virtual ~FacenetModel(){};
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) = 0;
	virtual int EncodeFace(const void *img, const Detection &det, std::vector<uint8_t> &face_encode,
	                       const ModelConfig &conf) = 0;
	virtual int EncodeFace(const void *img, const Detection &det, std::vector<float> &face_encode,
	                       const ModelConfig &conf) = 0;

	virtual void SetDebug(int debug){};
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

	// const char *model_path_ = "facenet_float.tflite";
	// const char* model_path_ = "facenet_quant.tflite";

	//float zero_ = 127.5;
	//float scale_ = 0.0078125;
	int encode_dim_;

    protected:
	EaifDataType m_type; // model type (quant / floating inference)
	int m_debug;
	int m_num_thread;
	int m_verbose;
};

#endif /* !FACENET_MODEL_H_ */