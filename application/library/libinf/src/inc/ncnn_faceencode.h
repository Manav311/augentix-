#ifdef USE_NCNN
#ifndef NCNN_FACEENCODE_H_
#define NCNN_FACEENCODE_H_

#include "net.h" // ncnn

#include "inf_types.h"
#include "inf_model.h"
#include "inf_face_internal.h"
#include "inf_adapter.h"

class NcnnFaceEncode : public InfFaceEncode
{
public:
	NcnnFaceEncode(InfModelInfo* info)
	{
		m_config = info; // take ownership of info
	}
	~NcnnFaceEncode() override
	{
		ReleaseConfig(m_config);
		delete m_config;
	}

	int EncodeFace(const InfImage* img, const MPI_RECT_POINT_S* roi, std::vector<float>& face) override;
	int EncodeFace(std::vector<float>& face) override;
	int LoadModels(const char* model_path) override;
	InfImage GetInputImage() override;
	int SetFaceEncodeImage(const InfImage* img, const MPI_RECT_POINT_S* roi) override;
	void *InputTensorBuffer(size_t input_index)
	{
		return in_Mat.data;
	}
	void *OutputTensorBuffer(size_t output_index)
	{
		return out_Mat.data;
	}

protected:
	ncnn::Net m_model;

private:
	ncnn::Mat in_Mat;
	ncnn::Mat out_Mat;

};

#endif // NCNN_FACEENCODE_H_
#endif // USE_NCNN