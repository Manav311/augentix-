#ifndef ARMNN_MTCNN_H_
#define ARMNN_MTCNN_H_
#ifdef USE_ARMNN

#include <memory>

#include <armnn/BackendId.hpp>
#include <armnn/BackendRegistry.hpp>
#include <armnn/IRuntime.hpp>
#include <armnn/Utils.hpp>
#include <armnnTfLiteParser/ITfLiteParser.hpp>

#include "eaif_common.h"
#include "eaif_utils.h"

#include "mtcnn.h"

class ArmnnMtcnn : public Mtcnn {
    public:
	ArmnnMtcnn() = default;

	int LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path) override;
	int FaceDetect(const void *img, std::vector<FaceBox<int> > &face_list, const ModelConfig &conf) override;
	int FaceDetect(const void *img, std::vector<FaceBox<float> > &face_list, const ModelConfig &conf) override;

	const QuantInfo &GetOutputQuantInfo(int index) const override
	{
		return onet_info_[index];
	};

	~ArmnnMtcnn() override{};

	// protected:
	template <typename T> void RunPNet(const void *img, ScaleWindow<T> &win, std::vector<FaceBox<T> > &box_list);
	template <typename T>
	void RunRNet(const void *img, std::vector<FaceBox<T> > &pnet_boxes, std::vector<FaceBox<T> > &output_boxes);
	template <typename T>
	void RunONet(const void *img, std::vector<FaceBox<T> > &rnet_boxes, std::vector<FaceBox<T> > &output_boxes);

	// int runNetwork(armnn::utils::ArmnnModel *inference, const void* img, const
	// int size[]);
	template <typename T>
	int RunNetwork(armnn::utils::ArmnnModel *inference, const void *img, const Shape &imsize, const Shape &outsize,
	               std::vector<std::vector<T> > &outputs, int stride);
	template <typename T>
	int RunNetwork(armnn::utils::ArmnnModel *inference, const void *img, const Shape &size, FaceBox<T> &box);
	void SetCpuType(int cpu_infer_type)
	{
		pnet_model_.cpu_infer_type = cpu_infer_type;
		rnet_model_.cpu_infer_type = cpu_infer_type;
		onet_model_.cpu_infer_type = cpu_infer_type;
	};

	// private:

	armnn::utils::ArmnnModel pnet_model_;
	armnn::utils::ArmnnModel rnet_model_;
	armnn::utils::ArmnnModel onet_model_;

	std::vector<QuantInfo> pnet_info_;
	std::vector<QuantInfo> rnet_info_;
	std::vector<QuantInfo> onet_info_;

	const int output_prob_idx_ = 0;
	const int output_reg_idx_ = 1;
	const int output_landmark_idx_ = 2;

	int type_;
	int m_row;
	// private:
};

#endif /* !USE_ARMNN */
#endif /* !ARMNN_MTCNN_H_ */