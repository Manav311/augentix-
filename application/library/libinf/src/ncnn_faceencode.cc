#ifdef USE_NCNN
#include "ncnn_faceencode.h"
#include <cstdio>
#include <ctime>

#include "inf_image.h"
#include "inf_types.h"

#include "inf_log.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"
#include "inf_adapter.h"

int NcnnFaceEncode::LoadModels(const char *model_path)
{
	SetupVerboseModeFromEnvironment();

	if (m_verbose)
		TIC(start);

	// Load model files
	const std::string orig_path = model_path;
	std::string model_path_ = orig_path + ".param";
	const char* param_path_char = model_path_.c_str();
	if (m_model.load_param(param_path_char)){
		inf_log_err("Cannot find %s for %s!", param_path_char, __func__);
		return -1;
	}

	model_path_ = orig_path + ".bin";
	const char* bin_path_char = model_path_.c_str();
	if (m_model.load_model(bin_path_char)){
		inf_log_err("Cannot find %s for %s!", bin_path_char, __func__);
		return -1;
	}

	// TODO: Add output parameter in config files
	m_encode_dim = 128;

	m_input_dim[1] = m_config->w; // m_config->w
	m_input_dim[0] = m_config->h; // m_config->h
	m_input_dim[2] = m_config->c; // m_config->c

	// Create and allocate input buffer
	in_Mat.create(m_input_dim[1], m_input_dim[0], m_input_dim[2], (size_t)1u);

	if (m_verbose)
		TOC("Load FaceEncode", start);

	inf_log_notice("FaceEncode model input: %dx%dx%d (%s), output: %d",
		m_input_dim[0], m_input_dim[1], m_input_dim[2], GetDTypeString(m_config->dtype),
		m_encode_dim);

	return 0;
}

int NcnnFaceEncode::EncodeFace(const InfImage* img, const MPI_RECT_POINT_S* roi, std::vector<float>& face)
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
		Inf_ImcropPadResizeNorm(img, sx, sy, ex, ey, pad.top, pad.bot, pad.left, pad.right, &wimg, input_w,
		                    input_h, &m_config->norm_zeros[0],
                            &m_config->norm_scales[0], m_config->input_int8_scale);
	} else {
		Inf_ImcropResizeNorm(img, sx, sy, ex, ey, &wimg, input_w, input_h, &m_config->norm_zeros[0],
                         &m_config->norm_scales[0], m_config->input_int8_scale);
	}

	if (m_verbose)
		TOC("Preprocess WImage for obj", start);

	if (m_verbose)
		TIC(start);

	ncnn::Extractor model_ex = m_model.create_extractor();
	// Insert image to ncnn model
	if(model_ex.input("images", in_Mat) == -1){
		inf_log_err("ncnn input error");
		return 0;
	}
	// Extract output from ncnn model
	if(model_ex.extract("output0", out_Mat) == -1) {
		inf_log_err("ncnn extract error");
		return 0;
	}

	if (m_verbose)
		TOC("Inference Call", start);


	auto output_addr = static_cast<float*>(OutputTensorBuffer(0));
	for (int j = 0; j < output_dim; j++)
		face[j] = output_addr[j];

	out_Mat.release();

	return 0;
}

int NcnnFaceEncode::EncodeFace(std::vector<float> &face)
{
	if (!m_input_set) {
		return -1;
	}

	const int output_dim = m_encode_dim;

	face.resize(output_dim, 0.0f);

	if (m_verbose)
		TIC(start);

	ncnn::Extractor ex = m_model.create_extractor();
	// Insert image to ncnn model
	if(ex.input("images", in_Mat) == -1){
		inf_log_err("ncnn input error");
		return -1;
	}
	// Extract output from ncnn model
	if(ex.extract("output0", out_Mat) == -1) {
		inf_log_err("ncnn extract error");
		return -1;
	}

	if (m_verbose)
		TOC("Inference Call", start);


	auto output_addr = static_cast<float*>(OutputTensorBuffer(0));
	for (int j = 0; j < output_dim; j++)
		face[j] = output_addr[j];

	out_Mat.release();

	m_input_set = false;

	return 0;
}

int NcnnFaceEncode::SetFaceEncodeImage(const InfImage *img, const MPI_RECT_POINT_S* roi)
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
			Inf_ImcropPadResizeNorm(img, sx, sy, ex, ey,
			                    pad.top, pad.bot, pad.left, pad.right,
			                    &wimg, input_w,input_h, &m_config->norm_zeros[0],
                                &m_config->norm_scales[0], m_config->input_int8_scale);
		} else {
			Inf_ImcropResizeNorm(img, sx, sy, ex, ey,
			                 &wimg, input_w, input_h, &m_config->norm_zeros[0],
                             &m_config->norm_scales[0], m_config->input_int8_scale);
		}

		m_input_set = true;

		if (m_verbose)
			TOC("Preprocess WImage for obj", start);

	} else {
		inf_log_warn("Currently not support floating point inference!");
	}
	return 0;
}

// Duplicate in ncnn_scrfd
InfImage NcnnFaceEncode::GetInputImage()
{
	InfImage img{};

	img.h = m_input_dim[0];
	img.w = m_input_dim[1];
	img.c = m_input_dim[2];

	int dtype = Inf8U;

	img.buf_owner = 0;
	img.data = static_cast<uint8_t*>(InputTensorBuffer(0));

	img.dtype = static_cast<InfDataType>(dtype | ((img.c - 1) << 3));

	return img;
}

#endif // USE_NCNN