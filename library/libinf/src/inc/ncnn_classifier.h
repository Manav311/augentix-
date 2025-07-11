#ifdef USE_NCNN

#include "net.h" // ncnn

#include "inf_types.h"
#include "inf_model.h"
#include "inf_utils.h"

#include "inf_model_internal.h"

class NcnnClassifier : public InfModel
{
public:
	NcnnClassifier(InfModelInfo *info)
	{
		m_config = info; // take ownership of info
	}
	~NcnnClassifier() override
	{
		ReleaseConfig(m_config);
		delete m_config;
	}

	int LoadModels(const std::string& model_path);
	int Classify(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfResultList *result) override;
	void *InputTensorBuffer(size_t input_index)
	{
		return in_Mat.data;
	}
	void *OutputTensorBuffer(size_t output_index)
	{
		return out_Mat.data;
	}

protected:
	int m_input_dim[3]; /* h x w x c */
	int m_output_dim[4]; /* b x result x reserved*/

	ncnn::Net m_model;

	size_t m_nums_output_tensor;

private:
	ncnn::Mat in_Mat;
	ncnn::Mat out_Mat;

};

#endif // USE_NCNN