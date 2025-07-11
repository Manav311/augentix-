#ifndef USE_NCNN
#ifndef LITE_CLASSIFIER_INTERNAL_
#define LITE_CLASSIFIER_INTERNAL_

#ifdef USE_MICROLITE
#include "tensorflow/lite/micro/micro_interpreter.h"
#else
#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"
#endif

#include "inf_types.h"
#include "inf_model.h"
#include "inf_utils.h"

#include "inf_utils_lite.h"
#include "inf_model_internal.h"
#include "inf_adapter.h"

class LiteClassifierBase : public InfModel, protected LiteModelTraits
{
public:
	LiteClassifierBase(InfModelInfo *info)
	{
		m_config = info; // take ownership of info
	}
	~LiteClassifierBase() override
	{
		ReleaseConfig(m_config);
		delete m_config;
	}

	int LoadModels(const std::string &model_path);
	int Classify(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfResultList *result) override;

    protected:
	int m_input_dim[3]; /* h x w x c */
	int m_output_dim[4]; /* b x result x reserved*/

	size_t m_nums_output_tensor;
};

class LiteClassifier : public LiteClassifierBase
{
public:
	LiteClassifier(InfModelInfo *info): LiteClassifierBase(info)
	{
	}

protected:
	bool PrepareInterpreter(const std::string &model_path) override;
	bool CollectModelInfo() override;
	void *InputTensorBuffer(size_t input_index) override
	{
		return m_model->input_tensor(input_index)->data.data;
	}
	void *OutputTensorBuffer(size_t output_index) override
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
	friend int inf_tf_adapter::LiteClassifier_LoadModel(LiteClassifier &classifier, const std::string &model_path,
	                                                    std::unique_ptr<tflite::MicroInterpreter> &model,
	                                                    std::unique_ptr<unsigned char[]> &model_fb);
	// the order of these 2 unique_ptr fields is IMPORTANT!!!
	std::unique_ptr<unsigned char[]> m_model_fb;
	std::unique_ptr<tflite::MicroInterpreter> m_model;
	uint8_t m_arena[768 * 1024];
#else
	std::unique_ptr<tflite::Interpreter> m_model;
	std::unique_ptr<tflite::FlatBufferModel> m_model_fb;
#endif
};

#endif // INF_CLASSIFIER_INTERNAL_
#endif // USE_NCNN