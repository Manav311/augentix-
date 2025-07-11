#ifndef LITE_FACENET_MODEL_H_
#define LITE_FACENET_MODEL_H_
#ifdef USE_TFLITE
#include <memory>
#include <vector>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"

#include "eaif_common.h"
#include "eaif_model.h"

#include "facenet_model.h"

class LiteFacenetModel : public FacenetModel {
    public:
	LiteFacenetModel(){};
	~LiteFacenetModel() override{};
	int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	int EncodeFace(const void *img, const Detection &detection, std::vector<uint8_t> &face_encode,
	               const ModelConfig &conf) override;
	int EncodeFace(const void *img, const Detection &detection, std::vector<float> &face_encode,
	               const ModelConfig &conf) override;

	// int encode_face(const void *img, std::vector<uint8_t>& face_encode);
	template<typename Tbuffer, typename Tface>
	int EncodeAnyFace(const void *img, const Detection &detection, std::vector<Tface> &face_encode,
	                  const ModelConfig &conf);
	std::unique_ptr<tflite::Interpreter> model_;
	std::unique_ptr<tflite::FlatBufferModel> model_fb_;

	QuantInfo model_info_;
	int input_dim_[2];
	int type_;
};

#endif /* !USE_TFLITE */
#endif /* !FACENET_MODEL_H_ */