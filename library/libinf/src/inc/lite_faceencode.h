#ifndef USE_NCNN
#ifndef LITE_FACEENCODE_H_
#define LITE_FACEENCODE_H_

#ifndef USE_MICROLITE
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#else
#include "tensorflow/lite/micro/micro_interpreter.h"
#endif

#include "inf_types.h"
#include "inf_model.h"
#include "inf_face_internal.h"
#include "inf_adapter.h"
#include "inf_utils_lite.h"

class LiteFaceEncodeBase : public InfFaceEncode, protected LiteModelTraits
{
public:
	LiteFaceEncodeBase(InfModelInfo* info)
	{
		m_config = info; // take ownership of info
	}
	~LiteFaceEncodeBase() override
	{
		ReleaseConfig(m_config);
		delete m_config;
	}

	int EncodeFace(const InfImage* img, const MPI_RECT_POINT_S* roi, std::vector<float>& face) override;
	int EncodeFace(std::vector<float>& face) override;
	int LoadModels(const char* model_path) override;
	int SetFaceEncodeImage(const InfImage* img, const MPI_RECT_POINT_S* roi) override;
};

class LiteFaceEncode : public LiteFaceEncodeBase
{
public:
	LiteFaceEncode(InfModelInfo* info)
	        : LiteFaceEncodeBase(info)
	{
	}

	InfImage GetInputImage() override;

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
#ifdef USE_MICROLITE
	friend int inf_tf_adapter::LiteFaceEncode_LoadModel(LiteFaceEncode &encoder, const std::string &model_path,
	                                                    std::unique_ptr<tflite::MicroInterpreter> &model,
	                                                    std::unique_ptr<unsigned char[]> &model_fb);
	std::unique_ptr<unsigned char[]> m_model_fb;
	std::unique_ptr<tflite::MicroInterpreter> m_model;
	uint8_t m_arena[768 * 1024];
#else
	std::unique_ptr<tflite::Interpreter> m_model;
	std::unique_ptr<tflite::FlatBufferModel> m_model_fb;
#endif
};

#endif // LITE_FACEENCODE_H_
#endif // USE_NCNN