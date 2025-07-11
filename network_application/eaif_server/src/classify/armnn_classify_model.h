#ifndef ARMNN_CLASSIFY_MODEL_H_
#define ARMNN_CLASSIFY_MODEL_H_
#ifdef USE_ARMNN

#include <vector>

#include "eaif_utils.h"

#include "classify_model.h"

class ArmnnClassifyModel : public ClassifyModel {
    public:
	ArmnnClassifyModel() = default;
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Classify(const void *Wimg, Classification &result, const ModelConfig &conf) override;
	virtual int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                            std::vector<Classification> &results, const ModelConfig &conf) override;
	virtual ~ArmnnClassifyModel(){};
	virtual void SetDebug(int debug) override
	{
		model_.set_profile = debug;
	};
	void SetCpuType(int cpu_type)
	{
		model_.cpu_infer_type = cpu_type;
	};
	armnn::utils::ArmnnModel model_;
	std::vector<QuantInfo> model_info_;
	int input_dim_[2]; /* h x w */
	int output_dim_[2];
	int type_;
};

#endif /* !USE_ARMNN */
#endif /* !ARMNN_CLASSIFY_MODEL_H_ */