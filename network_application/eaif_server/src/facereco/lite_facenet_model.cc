#ifdef USE_TFLITE
#include <iostream>
#include <vector>

#include <stdint.h>

#include "eaif_common.h"
#include "eaif_image.h"
#include "eaif_model.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include "lite_facenet_model.h"

using namespace std;
using namespace eaif::image;

static struct timespec start;

int LiteFacenetModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	string tflite_model = model_dir + "/" + model_path[0];

	if (m_verbose) TIC(start);

	tflite::utils::LoadModels(tflite_model.c_str(), model_, model_fb_);
	tflite::utils::GetQuantInfo(model_, &model_info_, 1);

	model_->SetNumThreads(m_num_thread);

	const int output_idx = model_->outputs()[0];
	const int input_idx = model_->inputs()[0];
	const TfLiteIntArray* outdims = model_->tensor(output_idx)->dims;

	assert(outdims->size == 2);
	encode_dim_ = outdims->data[1];

	const TfLiteIntArray* indims = model_->tensor(input_idx)->dims;
	input_dim_[0] = indims->data[1];
	input_dim_[1] = indims->data[2];

	m_type = tflite::utils::GetDataType(model_.get(), input_idx);
	assert(m_type != -1);

	eaif_info_h("Facenet model type is %s\n", eaif_GetDTypeString(m_type));
	eaif_info_h("Facenet model input dim is %d-%d\n", input_dim_[0], input_dim_[1]);
	eaif_info_h("Facenet model encode dim is %d\n", encode_dim_);

	eaif_check(kTfLiteOk == model_->AllocateTensors());

	if (m_verbose) TOC("Load Facenet Model", start);

	return 0;
}

template<typename Tbuffer, typename Tface>
int LiteFacenetModel::EncodeAnyFace(const void* img, const Detection &detection, std::vector<Tface>& face_encode, const ModelConfig &conf)
{
	if ((int)face_encode.size() != encode_dim_) {
		eaif_err("Input vector size is not correct!\n");
		return 0;
	}
	/* Get input/output tensor index */
	tflite::Interpreter* interpreter = model_.get();
	const int input = interpreter->inputs()[0];
	const int output_idx = interpreter->outputs()[0];

	Tbuffer* input_addr = interpreter->typed_tensor<Tbuffer>(input);

	WImage *pimg = (WImage*)img;

	int sx = Max((int)detection.box.sx - conf.align_margin / 2, 0);
	int sy = Max((int)detection.box.sy - conf.align_margin / 2, 0);
	int ex = Min((int)detection.box.ex + conf.align_margin / 2, pimg->cols - 1);
	int ey = Min((int)detection.box.ey + conf.align_margin / 2, pimg->rows - 1);

	if (m_type == Eaif8U) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		COND_TIMER_FUNC(
			m_verbose,
			"Facenet Preprocess Image",
			ImcropResize(*pimg, sx, sy, ex, ey,
				wimg, input_dim_[1], input_dim_[0])
		);
	} else if (m_type == Eaif8S) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8SC3, input_addr);
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

	int ret = 0;

	COND_TIMER_FUNC(
		m_verbose,
		"Facenet Inference",
		ret = interpreter->Invoke()
	);

	eaif_check(ret == kTfLiteOk);

	Tbuffer* output_addr = interpreter->typed_tensor<Tbuffer>(output_idx);

	if (m_type == Eaif8U || m_type == Eaif8S) {
		for (size_t i = 0; i < face_encode.size(); i++) // mean substraction important for cosine similarity
			face_encode[i] = static_cast<float>(output_addr[i]) - model_info_.m_zero;
	} else if (m_type == Eaif32F) {
		memcpy(&face_encode[0], output_addr, sizeof(Tbuffer) * encode_dim_);
	}

	return 0;
}

int LiteFacenetModel::EncodeFace(const void* img, const Detection &detection, std::vector<uint8_t>& face_encode, const ModelConfig &conf)
{
	return EncodeAnyFace<uint8_t, uint8_t>(img, detection, face_encode, conf);
}

int LiteFacenetModel::EncodeFace(const void* img, const Detection &detection, std::vector<float>& face_encode, const ModelConfig &conf)
{
	if (m_type == Eaif8U)
		return EncodeAnyFace<uint8_t, float>(img, detection, face_encode, conf);
	else if (m_type == Eaif8S)
		return EncodeAnyFace<int8_t, float>(img, detection, face_encode, conf);
	else if (m_type == Eaif32F)
		return EncodeAnyFace<float, float>(img, detection, face_encode, conf);
	else {
		eaif_warn("Unsupport model type!\n");
		return 0;
	}
}

#endif /* !USE_TFLITE */