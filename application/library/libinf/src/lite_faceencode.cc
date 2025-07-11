#ifndef USE_NCNN
#include "lite_faceencode.h"
#include <cstdio>
#include <ctime>

#ifndef USE_MICROLITE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/lite/interpreter.h"
#pragma GCC diagnostic pop
#endif

#include "inf_image.h"
#include "inf_types.h"

#include "inf_log.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"
#include "inf_adapter.h"

int LiteFaceEncodeBase::LoadModels(const char *model_path)
{
	SetupVerboseModeFromEnvironment();

	// 1. Prepare the interpreter
	if (!PrepareInterpreter(model_path)) {
		inf_log_err("Failed to prepare model interpreter!");
		return -1;
	}

	size_t used_bytes = GetArenaUsedBytes();
	if (used_bytes > 0) {
		inf_log_notice("%s had used %zd bytes of arena.", model_path, used_bytes);
	}

	// 2. Fetch model info
	if (!CollectModelInfo()) {
		inf_log_err("Failed to collect model information!");
		return -1;
	}

	if (m_config->dtype == InfUnknownType) {
		inf_log_err("Unknown dtype for model %s!", model_path);
		return -1;
	}

	inf_log_notice("TFLite model input: %dx%dx%d (%s), output: %d",
		m_input_dim[0], m_input_dim[1], m_input_dim[2], GetDTypeString(m_config->dtype),
		m_encode_dim);

	return 0;
}

int LiteFaceEncodeBase::EncodeFace(const InfImage* img, const MPI_RECT_POINT_S* roi, std::vector<float>& face)
{
	if (m_config->dtype != Inf8U && m_config->dtype != Inf8S) {
		inf_log_warn("Dtype (%d) not supported!", m_config->dtype);
		return -1;
	}

	const int input_h = m_input_dim[0];
	const int input_w = m_input_dim[1];
	const int input_chn = m_input_dim[2];
	const int output_dim = m_encode_dim;

	const int align_margin = m_config->align_margin;
	face.resize(output_dim, 0.0f);

	Pads pad{};
	MPI_RECT_POINT_S box = *roi;

	// Expand box by margin
	box.sx = box.sx - align_margin / 2;
	box.sy = box.sy - align_margin / 2;
	box.ex = box.ex + align_margin / 2;
	box.ey = box.ey + align_margin / 2;

	// Get padding info
	GetPadInfo(img, pad, box);
	int is_pad = pad.left || pad.right || pad.top || pad.bot;

	int sx = box.sx;
	int sy = box.sy;
	int ex = box.ex;
	int ey = box.ey;

	InfImage wimg;
	wimg.w = m_input_dim[1];
	wimg.h = m_input_dim[0];
	wimg.c = m_input_dim[2];
	wimg.buf_owner = 0;

	wimg.dtype = static_cast<InfDataType>(GetImageType((int)m_config->dtype, input_chn));
	wimg.data = static_cast<uint8_t*>(InputTensorBuffer(0));

	if (m_verbose)
		TIC(start);

	if (is_pad) {
		Inf_ImcropPadResize(img, sx, sy, ex, ey, pad.top, pad.bot, pad.left, pad.right, &wimg, input_w,
		                    input_h);
	} else {
		Inf_ImcropResize(img, sx, sy, ex, ey, &wimg, input_w, input_h);
	}

	if (m_verbose)
		TOC("Preprocess WImage for obj", start);

	if (m_verbose)
		TIC(start);

	if (Invoke() != kTfLiteOk) {
		inf_log_err("Cannot Invoke tflite model!");
		return 0;
	}

	if (m_verbose)
		TOC("Inference Call", start);

	if (m_config->dtype == Inf8U) {
		auto output_addr = static_cast<uint8_t*>(OutputTensorBuffer(0));
		for (int j = 0; j < output_dim; j++) {
			face[j] = QuantConvert(m_config->quant, output_addr[j]);
		}
	} else {
		auto output_addr = static_cast<int8_t*>(OutputTensorBuffer(0));
		for (int j = 0; j < output_dim; j++)
			face[j] = QuantConvert(m_config->quant, output_addr[j]);
	}

	return 0;
}

int LiteFaceEncodeBase::EncodeFace(std::vector<float> &face)
{
	if (!m_input_set) {
		return -1;
	}

	const int output_dim = m_encode_dim;

	face.resize(output_dim, 0.0f);

	if (m_verbose)
		TIC(start);

	if (Invoke() != kTfLiteOk) {
		inf_log_warn("Cannot Invoke tflite model!");
		return 0;
	}

	if (m_verbose)
		TOC("Inference Call", start);

	if (m_config->dtype == Inf8U) {
		auto output_addr = static_cast<uint8_t*>(OutputTensorBuffer(0));
		for (int j = 0; j < output_dim; j++) {
			face[j] = QuantConvert(m_config->quant, output_addr[j]);
		}
	} else {
		auto output_addr = static_cast<int8_t*>(OutputTensorBuffer(0));
		for (int j = 0; j < output_dim; j++) {
			face[j] = QuantConvert(m_config->quant, output_addr[j]);
		}
	}

	m_input_set = false;

	return 0;
}

int LiteFaceEncodeBase::SetFaceEncodeImage(const InfImage *img, const MPI_RECT_POINT_S* roi)
{
	if (m_config->dtype != Inf8U && m_config->dtype != Inf8S) {
		inf_log_warn("Dtype (%d) not supported!", m_config->dtype);
		return -1;
	}

	const int input_h = m_input_dim[0];
	const int input_w = m_input_dim[1];
	const int input_chn = m_input_dim[2];

	const int align_margin = m_config->align_margin;

	Pads pad{};
	MPI_RECT_POINT_S box = *roi;

	// Expand box by margin
	box.sx = box.sx - align_margin / 2;
	box.sy = box.sy - align_margin / 2;
	box.ex = box.ex + align_margin / 2;
	box.ey = box.ey + align_margin / 2;

	// Get padding info
	GetPadInfo(img, pad, box);
	int is_pad = pad.left || pad.right || pad.top || pad.bot;

	int sx = box.sx;
	int sy = box.sy;
	int ex = box.ex;
	int ey = box.ey;

	InfImage wimg;
	wimg.w = m_input_dim[1];
	wimg.h = m_input_dim[0];
	wimg.c = m_input_dim[2];
	wimg.buf_owner = 0;

	wimg.dtype = static_cast<InfDataType>(GetImageType((int)m_config->dtype, input_chn));
	wimg.data = static_cast<uint8_t*>(InputTensorBuffer(0));

	// this if guard is redundant
	if (m_config->dtype == Inf8U || m_config->dtype == Inf8S) {

		if (m_verbose)
			TIC(start);

		if (is_pad) {
			Inf_ImcropPadResize(img, sx, sy, ex, ey,
			                    pad.top, pad.bot, pad.left, pad.right,
			                    &wimg, input_w,input_h);
		} else {
			Inf_ImcropResize(img, sx, sy, ex, ey,
			                 &wimg, input_w, input_h);
		}

		m_input_set = true;

		if (m_verbose)
			TOC("Preprocess WImage for obj", start);

	} else {
		inf_log_warn("Currently not support floating point inference!");
	}
	return 0;
}

InfImage LiteFaceEncode::GetInputImage()
{
	return utils::lite::GetInputImage(m_model);
}

bool LiteFaceEncode::PrepareInterpreter(const std::string& model_path)
{
	if (inf_tf_adapter::LiteFaceEncode_LoadModel(*this, model_path, m_model, m_model_fb)) {
		inf_log_err("Cannot find %s for %s!", model_path.c_str(), __func__);
		return false;
	}

	if (m_model->AllocateTensors() != kTfLiteOk) {
		inf_log_err("Cannot allocate tensor!");
		return false;
	}

	inf_tf_adapter::setNumThreads(*m_model, m_num_thread);
	return true;
}

bool LiteFaceEncode::CollectModelInfo()
{
	if (inf_tf_adapter::numsOfOutputTensor(*m_model) != 1) {
		inf_log_err("Number of output is not correct!");
		return false;
	}

	const TfLiteQuantizationParams* p = &m_model->output_tensor(0)->params;
	m_config->quant.zero = p->zero_point;
	m_config->quant.scale = p->scale;

	const TfLiteIntArray* outdims = m_model->output_tensor(0)->dims;
	m_encode_dim = outdims->data[1];

	const TfLiteIntArray* indims = m_model->input_tensor(0)->dims;
	m_input_dim[0] = indims->data[1];
	m_input_dim[1] = indims->data[2];
	m_input_dim[2] = indims->data[3];

	m_config->dtype = utils::lite::GetDataType(m_model->input_tensor(0)->type);
	m_type = m_config->dtype;

	return true;
}

#endif // USE_NCNN