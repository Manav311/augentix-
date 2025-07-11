#ifndef EAIF_LITE_MTCNN_H_
#define EAIF_LITE_MTCNN_H_
#ifdef USE_TFLITE

#include <memory>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/model.h"

#include "eaif_common.h"
#include "mtcnn.h"

class LiteMtcnn : public Mtcnn {
    public:
	LiteMtcnn() = default;
	~LiteMtcnn(void) override{};
	int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	int FaceDetect(const void *img, std::vector<FaceBox<int> > &face_list, const ModelConfig &conf) override
	{
		return 0;
	};

	// unused function
	int FaceDetect(const void *img, std::vector<FaceBox<float> > &face_list, const ModelConfig &conf) override;

	const QuantInfo &GetOutputQuantInfo(int index) const override
	{
		return onet_info_[index];
	};

	int m_row;

    protected:
	int RunPNet(const void *img, ScaleWindow<float> &win, std::vector<FaceBox<float> > &box_list);
	int RunRNet(const void *img, std::vector<FaceBox<float> > &pnet_boxes,
	            std::vector<FaceBox<float> > &output_boxes);
	int RunONet(const void *img, std::vector<FaceBox<float> > &rnet_boxes,
	            std::vector<FaceBox<float> > &output_boxes);

	int RunNetwork(tflite::Interpreter *interpreter, const void *img, const Shape &size);

	int RunNetwork(tflite::Interpreter *interpreter, const void *img, const Shape &size, FaceBox<float> &box);

	void GetQuantInfo(const std::unique_ptr<tflite::Interpreter> &model, QuantInfo *info, int net_idx, int prob_num,
	                  int coord_num, int landmark_num);

	// private:

	std::unique_ptr<tflite::Interpreter> pnet_model_;
	std::unique_ptr<tflite::Interpreter> rnet_model_;
	std::unique_ptr<tflite::Interpreter> onet_model_;
	std::unique_ptr<tflite::FlatBufferModel> pnet_model_fb_;
	std::unique_ptr<tflite::FlatBufferModel> rnet_model_fb_;
	std::unique_ptr<tflite::FlatBufferModel> onet_model_fb_;

	QuantInfo pnet_info_[2];
	QuantInfo rnet_info_[2];
	QuantInfo onet_info_[3];

	int output_prob_idx_[3];
	int output_reg_idx_[3];
	int output_landmark_idx_;

	int type_;
};

#endif /* !USE_TFLITE */
#endif /* !EAIF_LITE_MTCNN_H_ */