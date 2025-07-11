#ifdef USE_ARMNN
#include <iostream>
#include <vector>

#include <stdint.h>

#include "eaif_common.h"
#include "eaif_image.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include "armnn_facenet_model.h"

using namespace eaif::image;
using namespace std;

static struct timespec start;

int ArmnnFacenetModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	string facenet_model_path = model_dir + "/" + model_path[0];

	if (m_verbose) TIC(start);

	armnn::utils::LoadModels(facenet_model_path.c_str(), model_);

	m_type = model_.type;
	if (m_type) armnn::utils::GetQuantInfo(model_.model, model_info_);

	armnn::utils::InitModelIO(model_, model_.input_tensors, model_.output_tensors);

	const auto& output_shape = model_.output_tensors[0].second.GetShape();
	encode_dim_ = output_shape[1];

	const auto& input_shape = model_.input_tensors[0].second.GetShape();
	input_dim_[0] = input_shape[1];
	input_dim_[1] = input_shape[2];

	eaif_info_h("Facenet model is using %s.\n", eaif_GetDTypeString(m_type));
	eaif_info_h("Facenet model input dim is %d-%d\n", input_dim_[0], input_dim_[1]);
	eaif_info_h("Facenet model encode dim is %d\n", encode_dim_);

	if (m_verbose) TOC("Load Facenet Model", start);

	return 0;
}

template<typename Tbuffer, typename Tface>
int ArmnnFacenetModel::EncodeAnyFace(const void* img, const Detection& detection, std::vector<Tface>& face_encode, const ModelConfig &conf)
{
	if ((int)face_encode.size() != encode_dim_) {
		eaif_err("Input vector size is not correct!\n");
		return 0;
	}

	WImage *pimg = (WImage*) img;

	int sx = Max((int)detection.box.sx - conf.align_margin / 2, 0);
	int sy = Max((int)detection.box.sy - conf.align_margin / 2, 0);
	int ex = Min((int)detection.box.ex + conf.align_margin / 2, pimg->cols - 1);
	int ey = Min((int)detection.box.ey + conf.align_margin / 2, pimg->rows - 1);

	/* Get input/output tensor index */
	armnn::utils::ArmnnModel* inference = &model_;

	Tbuffer* input_addr = (Tbuffer*)inference->input_tensors[0].second.GetMemoryArea();
	Tbuffer* output_addr = (Tbuffer*)inference->output_tensors[0].second.GetMemoryArea();

	if (m_type == Eaif8U) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		COND_TIMER_FUNC(
			m_verbose,
			"Facenet Preprocess Image",
			ImcropResize(*pimg, sx, sy, ex, ey,
				wimg, input_dim_[1], input_dim_[0])
		);
	} else if (m_type == Eaif32F) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif32FC3, input_addr);
		COND_TIMER_FUNC(
			m_verbose,
			"Facenet Preprocess Image",
			ImcropResizeNorm(*pimg, sx, sy, ex, ey,
				wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data())
		);
	} else {
		eaif_warn("Data type not supported!\n");
		return 0;
	}

	COND_TIMER_FUNC(
		m_verbose,
		"Facenet Inference",
		inference->Invoke()
	);

	if (m_type == Eaif8U) {
		for (size_t i = 0; i < face_encode.size(); i++) // mean substraction important for cosine similarity
			face_encode[i] = static_cast<float>(output_addr[i]) - model_info_[0].m_zero;
	} else if (m_type == Eaif32F) {
		memcpy(&face_encode[0], output_addr, sizeof(Tface) * encode_dim_);
	}

	return 0;
}

int ArmnnFacenetModel::EncodeFace(const void* img, const Detection& detection, std::vector<uint8_t>& face_encode, const ModelConfig &conf)
{
	return EncodeAnyFace<uint8_t, uint8_t>(img, detection, face_encode, conf);
}

int ArmnnFacenetModel::EncodeFace(const void* img, const Detection& detection, std::vector<float>& face_encode, const ModelConfig &conf)
{
	if (m_type == Eaif8U)
		return EncodeAnyFace<uint8_t, float>(img, detection, face_encode, conf);
	else if (m_type == Eaif32F)
		return EncodeAnyFace<float, float>(img, detection, face_encode, conf);
	else {
		eaif_warn("Unsupport model type!\n");
		return 0;
	}
}

#endif /* !USE_ARMNN */