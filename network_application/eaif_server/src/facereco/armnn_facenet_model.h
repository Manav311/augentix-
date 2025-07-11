#ifndef ARMNN_FACENET_MODEL_H_
#define ARMNN_FACENET_MODEL_H_
#ifdef USE_ARMNN

#include <memory>
#include <vector>

#include "eaif_common.h"
#include "eaif_utils.h"

#include "facenet_model.h"

class ArmnnFacenetModel : public FacenetModel {
    public:
	ArmnnFacenetModel(){};
	~ArmnnFacenetModel() override{};
	int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	int EncodeFace(const void *img, const Detection &detection, std::vector<uint8_t> &face_encode,
	               const ModelConfig &conf) override;
	int EncodeFace(const void *img, const Detection &detection, std::vector<float> &face_encode,
	               const ModelConfig &conf) override;

	template<typename Tbuffer, typename Tface>
	int EncodeAnyFace(const void *img, const Detection &detection, std::vector<Tface> &face_encode,
	                  const ModelConfig &conf);
	void SetCpuType(int cpu_infer_type)
	{
		model_.cpu_infer_type = cpu_infer_type;
	}

	armnn::utils::ArmnnModel model_;
	std::vector<QuantInfo> model_info_;
	int input_dim_[2];
};

#endif /* !USE_ARMNN */
#endif /* !ARMNN_FACENET_MODEL_H_ */