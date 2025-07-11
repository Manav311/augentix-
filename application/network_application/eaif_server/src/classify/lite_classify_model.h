#ifndef LITE_CLASSIFY_MODEL_H_
#define LITE_CLASSIFY_MODEL_H_
#ifdef USE_TFLITE

#include <memory>
#include <string>
#include <vector>

#include "classify_model.h"
#include "eaif_common.h"
#include "eaif_utils.h"

class LiteClassifyModel : public ClassifyModel {
    public:
	LiteClassifyModel() = default;
	virtual int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	virtual int Classify(const void *Wimg, Classification &result, const ModelConfig &conf) override;
	virtual int ClassifyObjList(const void *Wimg, const std::vector<ObjectAttr> &obj_list,
	                            std::vector<Classification> &results, const ModelConfig &conf) override;
	virtual void SetDebug(int debug) override{};
	virtual ~LiteClassifyModel(){};

	std::unique_ptr<tflite::Interpreter> model_;
	std::unique_ptr<tflite::FlatBufferModel> model_fb_;

	QuantInfo model_info_;
	int input_dim_[3]; /* h x w x c*/
	std::vector<int> output_dim_;
};

#endif /* !USE_TFLITE */
#endif /* !LITE_CLASSIFY_MODEL_H_ */