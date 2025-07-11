#ifdef USE_NCNN
#include "ncnn_scrfd.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <sys/time.h>

#include "inf_types.h"
#include "inf_image.h"
#include "inf_model.h"

#include "inf_log.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"

#include "vftr_dump.h"
#include "eaif_dump_define.h"

#include "inf_adapter.h"


int NcnnScrfd::LoadModels(const char* model_dir, const InfStrList* model_paths)
{
	SetupVerboseModeFromEnvironment();

	if (model_paths->size != 1) {
		inf_log_warn("Number of model paths do not equal to 1!");
		return -1;
	}

	if (m_verbose)
		TIC(start);

	// Load model files
	const std::string orig_dir = model_dir;
	const std::string orig_path = model_paths->data[0];
	std::string model_path_ = orig_dir + "\\" + orig_path + ".param";
	const char* param_path_char = model_path_.c_str();
	if (m_model.load_param(param_path_char)){
		inf_log_err("Cannot find %s for %s!", param_path_char, __func__);
		return -1;
	}

	model_path_ = orig_dir + "\\" + orig_path + ".bin";
	const char* bin_path_char = model_path_.c_str();
	if (m_model.load_model(bin_path_char)){
		inf_log_err("Cannot find %s for %s!", bin_path_char, __func__);
		return -1;
	}

	// Init input/output info from config.ini
	m_input_dim[1] = m_config->w; // m_config->w
	m_input_dim[0] = m_config->h; // m_config->h
	m_input_dim[2] = m_config->c; // m_config->c

	// TODO:: Output size should be hard-coded in ncnn
	// m_output_dim[0] = 1;
	// m_output_dim[1] = m_config->labels.size;
	// m_output_dim[2] = m_config->labels.size;

	// Create and allocate input buffer
	in_Mat.create(m_input_dim[1], m_input_dim[0], m_input_dim[2], (size_t)1u);


	if (m_verbose)
		TOC("Load SCRFD", start);

	inf_log_notice("SCRFD model input dimension is %dx%dx%d (%s)",
	               m_input_dim[0], m_input_dim[1], m_input_dim[2], GetDTypeString(m_type));

	return 0;
}

void NcnnScrfd::SetupConfig(InfModelInfo* conf)
{
	m_use_kps = conf->use_kps;
	m_feature_output_pairs = conf->feature_output_pairs;
	m_num_anchors_per_feature = conf->num_anchors_per_feature;
	memcpy(m_feature_stride, conf->feature_stride,
	       sizeof(int) *m_feature_output_pairs * m_num_anchors_per_feature);
	m_verbose = conf->verbose;
	m_debug = conf->debug;
	m_num_thread = conf->num_threads;
}

int NcnnScrfd::RunNetwork(const InfImage& img, const Shape& size, int chn, InfDataType mtype,
						  const MPI_RECT_POINT_S* roi, const Pads* pads, int verbose)
{
	/* Get input/output tensor index */
	int ret = 0;
	InfDataType dtype = static_cast<InfDataType>(GetImageType(mtype, chn));
	uint8_t* input_addr;

	if (mtype == Inf8U || mtype == Inf8S) {
		input_addr = static_cast<uint8_t*>(InputTensorBuffer(0));
	} else {
		inf_log_warn("%s does not support floating point inference", __func__);
		return 0;
	}

	InfImage dst{size.w, size.h, chn, input_addr, 0, dtype};

	if (verbose)
		TIC(start);

	if (!roi)
		Inf_ImresizeNorm(&img, size.w, size.h, &dst, &m_config->norm_zeros[0],
                         &m_config->norm_scales[0], m_config->input_int8_scale);
	else if (!pads)
		Inf_ImcropResizeNorm(&img, roi->sx, roi->sy, roi->ex, roi->ey, &dst, size.w, size.h, &m_config->norm_zeros[0],
                         &m_config->norm_scales[0], m_config->input_int8_scale);
	else
		Inf_ImcropPadResizeNorm(&img, roi->sx, roi->sy, roi->ex, roi->ey,
		                    pads->top, pads->bot, pads->left, pads->right, &dst, size.w, size.h, &m_config->norm_zeros[0],
                            &m_config->norm_scales[0], m_config->input_int8_scale);

	if (verbose)
	{
		TOC("Image preprocess", start);
		TIC(start);
	}

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

	if (verbose)
		TOC("Invoke", start);

	// TODO: Add out_Mat.release() at somewhere

	return ret;
}

//int NcnnScrfd::GetOutputInfo()
//{
//	size_t numsOutput = inf_tf_adapter::numsOfOutputTensor(*m_model);
//	if (numsOutput >= ScrfdMaxOutputSize) {
//		inf_log_notice("Cannot handle (%d) of network output, should be smaller than %d!",
//			static_cast<int>(numsOutput), ScrfdMaxOutputSize);
//		return -1;
//	}
//
//	/* Assign Quantization parameters and output dimension version */
//	for (size_t i = 0; i < numsOutput; i++) {
//		const TfLiteIntArray* dim = m_model->output_tensor(i)->dims;
//		const TfLiteQuantizationParams* p = &m_model->output_tensor(i)->params;
//
//		QuantInfo& info = m_qinfo[i];
//		info.zero = p->zero_point;
//		info.scale = p->scale;
//		int *output_dim = m_output_dim[i];
//
//		if (dim->size != 3) {
//			inf_log_notice("Scrfd Network output#%d dim Size (%d) not supported, should be (3)!",
//				static_cast<int>(i), static_cast<int>(dim->size));
//			return -1;
//		}
//		/* Only if the tensor content is not transposed */
//		output_dim[0] = 1;
//		output_dim[1] = std::max(dim->data[1], dim->data[2]);
//		output_dim[2] = std::min(dim->data[1], dim->data[2]);
//	}
//
//	/* Sort and assign dimension ind with descending order */
//	/* So that feature stride is start from large to small */
//	std::vector<int> ind(numsOutput,0);
//
//	for (size_t i = 0; i < ind.size(); ++i) {
//		ind[i] = i;
//	}
//
//	std::sort(ind.begin(), ind.end(),
//	          [&](const int a, const int b) {
//		          return m_output_dim[a][1] > m_output_dim[b][1];
//	          });
//
//	int ind_divident = (m_use_kps) ? 3 : 2;
//
//	for (size_t i = 0; i < ind.size(); i++) {
//		int output_ind = i / ind_divident;
//		if (!m_output_prob_idx[output_ind] && m_output_dim[ind[i]][2] == 1) {
//			m_output_prob_idx[output_ind] = ind[i];
//		} else if (!m_output_reg_idx[output_ind] && m_output_dim[ind[i]][2] == 4) {
//			m_output_reg_idx[output_ind] = ind[i];
//		} else if (m_use_kps && !m_output_landmark_idx[output_ind] &&
//			m_output_dim[ind[i]][2] == 10) {
//			m_output_landmark_idx[output_ind] = ind[i];
//		}
//	}
//	return 0;
//}

InfImage NcnnScrfd::GetInputImage()
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


int NcnnScrfd::TRunNet(const InfImage& img, const MPI_RECT_POINT_S* roi, std::vector<FaceBox>& face_list)
{
	const int height = m_input_dim[0];
	const int width = m_input_dim[1];
	const int chn = m_input_dim[2];
	const Shape size{width, height};
	Scaler scale_factor{};
	Pads pads{};
	int need_padding = 0;
	std::vector<FaceBox> feature_list;

	if (roi) {
		if (roi->sx < 0) pads.left = -roi->sx;
		if (roi->sy < 0) pads.top = -roi->sy;
		if (roi->ex >= img.w) pads.right = roi->ex + 1 - img.w;
		if (roi->ey >= img.h) pads.bot = roi->ey + 1 - img.h;

		if (pads.left || pads.top || pads.right || pads.bot) {
			need_padding = 1;
		}
		scale_factor.w = static_cast<float>(roi->ex - roi->sx + 1) / width;
		scale_factor.h = static_cast<float>(roi->ey - roi->sy + 1) / height;
	} else {
		scale_factor.w = static_cast<float>(img.w) / width;
		scale_factor.h = static_cast<float>(img.h) / height;
	}

	int ret;
	if (need_padding) {
		ret = RunNetwork(img, size, chn, m_type, roi, &pads, m_verbose);
	} else {
		ret = RunNetwork(img, size, chn, m_type, roi, nullptr, m_verbose);
	}

	if (ret) {
		return -1;
	}

	if (m_verbose) TIC(start);

	for (int i = 0; i < m_feature_output_pairs; i++) {
		const int conf_index = m_output_prob_idx[i];
		const int reg_index = m_output_reg_idx[i];
		const float* conf_data_ptr = static_cast<float*>(OutputTensorBuffer(conf_index));
		const float* reg_data_ptr = static_cast<float*>(OutputTensorBuffer(reg_index));

		std::vector<FaceBox> face_list_local;

		ScrfdPostProcessConfig conf = {};
		conf.num_anchors = m_num_anchors_per_feature;
		conf.feature_stride = m_feature_stride[i];
		conf.number_output = m_output_dim[conf_index][1];
		conf.conf_thresh = m_config->conf_thresh.data[0];
		conf.iou_thresh = m_config->iou_thresh;
		conf.input.h = m_input_dim[0];
		conf.input.w = m_input_dim[1];

		ScrfdPostProcess(conf_data_ptr, reg_data_ptr, conf, face_list_local);

		if (!face_list_local.empty())
			feature_list.insert(feature_list.end(), face_list_local.begin(), face_list_local.end());
	}

	NmsBoxes(feature_list, m_config->iou_thresh, NMS_UNION, face_list);

	if (roi) {
		for (auto& box : face_list) {
			box.x0 = box.x0 * scale_factor.w + roi->sx;
			box.y0 = box.y0 * scale_factor.h + roi->sy;
			box.x1 = box.x1 * scale_factor.w + roi->sx;
			box.y1 = box.y1 * scale_factor.h + roi->sy;
		}
	} else {
		for (auto& box : face_list) {
			box.x0 = box.x0 * scale_factor.w;
			box.y0 = box.y0 * scale_factor.h;
			box.x1 = box.x1 * scale_factor.w;
			box.y1 = box.y1 * scale_factor.h;
		}
	}

	if (m_debug) {
		// 1. write image
		WriteDebugInfo(roi, scale_factor, face_list);
	}

	if (m_verbose) {
		char msg[256] = {};
		int len = sprintf(msg, "PostProcess (#%d detections) ", static_cast<int>(face_list.size()));
		if (!face_list.empty()) {
			sprintf(&msg[len], "obj-0: [%.2f, %.2f, %.2f, %.2f]",
			face_list[0].x0, face_list[0].y0, face_list[0].x1, face_list[0].y1);
		}
		TOC(msg, start);
	}

	// Posprocessing
	/*
	ScrfdPostProcessConfig conf{
	int m_feature_output_pairs = 0; // fmc
	int m_num_anchors_per_feature = 0; // num_anchors
	int m_use_kps = 0; // use_kps
	int m_feature_stride[5]; // feat_stride_fpn
	}

	_PostProcess(a, )
	*/
	return 0;
}

int NcnnScrfd::FaceDetect(const InfImage* img, std::vector<FaceBox>& face_list)
{
	return TRunNet(*img, nullptr, face_list);
}

int NcnnScrfd::FaceDetect(const InfImage* img, const MPI_RECT_POINT_S& roi, std::vector<FaceBox>& face_list)
{
	return TRunNet(*img, &roi, face_list);
}

int NcnnScrfd::FaceDetect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, std::vector<FaceBox>& face_list)
{
#define PAD_PIX_SIZE (32)

	retIfNull(img && obj_list);

	MPI_RECT_POINT_S group_roi = {};
	MPI_RECT_POINT_S input_roi = {};
	const Shape img_shape{img->w, img->h};
	const Shape input{m_input_dim[1], m_input_dim[0]};

	if (!obj_list->obj_num)
		return 0;

#ifndef _HOST_LINUX
	VFTR_dumpStart();
	VFTR_dumpWithJiffies(obj_list, sizeof(*obj_list), 
						 COMPOSE_EAIF_FLAG_VER_1(InfObjList, InfObjList), obj_list->timestamp);
	VFTR_dumpEnd();
#endif

	Grouping(obj_list, group_roi);

	PadAndRescale(PAD_PIX_SIZE, img_shape, input, group_roi, input_roi);

	FaceDetect(img, input_roi, face_list);

	return 0;

#undef PAD_PIX_SIZE
}

int NcnnScrfd::Detect(const InfImage* img, InfDetList* result)
{
	std::vector<FaceBox> face_list;
	int ret = FaceDetect(img, face_list);
	TransformResult(face_list, result, 1.0f, m_config->labels.size);

#ifndef _HOST_LINUX
	// get current timestamp
	struct timeval tv;
	gettimeofday(&tv, NULL);
	TIMESPEC_S timespec;
	timespec.tv_sec = tv.tv_sec;
	timespec.tv_nsec = tv.tv_usec * 1000;
	struct tm localtime;
	localtime_r(&timespec.tv_sec, &localtime);
	timespec.tv_sec = mktime(&localtime);

	// dump InfImage
	InfImage wimg = GetInputImage();
	VFTR_dumpStart();
	VFTR_dump(&wimg, sizeof(wimg), 
			COMPOSE_EAIF_FLAG_VER_1(InfImage, InfImage), timespec);
	VFTR_dump(wimg.data, wimg.w * wimg.h * wimg.c,
			COMPOSE_EAIF_FLAG_VER_1(InfU8Array, InfImage), timespec);

	// dump detection result
	VFTR_dump(result, sizeof(*result),
			COMPOSE_EAIF_FLAG_VER_1(InfDetList, InfDetList), timespec);
	VFTR_dumpEnd();

	for (int i = 0; i < result->size; ++i ) {
		InfDetResult *single_result = &result->data[i];

		// dump face img
		InfImage face_img;
		auto& sx = single_result->rect.sx;
		auto& sy = single_result->rect.sy;
		auto& ex = single_result->rect.ex;
		auto& ey = single_result->rect.ey;
		face_img.w = ex - sx + 1;
		face_img.h = ey - sy + 1;
		face_img.c = wimg.c;
		face_img.buf_owner = 0;
		face_img.dtype = static_cast<InfDataType>(GetImageType(m_type, wimg.c));
		face_img.data = static_cast<uint8_t*>(InputTensorBuffer(0));

		// hint: roi and margin and padding will effect result img
		Inf_ImcropResizeNorm(img, sx, sy, ex, ey, &face_img, face_img.w, face_img.h, &m_config->norm_zeros[0],
                         &m_config->norm_scales[0], m_config->input_int8_scale);

		VFTR_dumpStart();
		VFTR_dump(single_result, sizeof(*single_result),
				COMPOSE_EAIF_FLAG_VER_1(InfDetResult, InfDetList), timespec);
		VFTR_dump(&face_img, sizeof(face_img), 
			COMPOSE_EAIF_FLAG_VER_1(InfFaceImage, InfImage), timespec);
		VFTR_dump(face_img.data, face_img.w * face_img.h * face_img.c,
			COMPOSE_EAIF_FLAG_VER_1(InfFaceU8Array, InfFaceImage), timespec);
		VFTR_dumpEnd();
	}
#endif // _HOST_LINUX

	return ret;
}

int NcnnScrfd::Detect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, InfDetList* result)
{
	std::vector<FaceBox> face_list;
	int ret = FaceDetect(img, obj_list, face_list);
	TransformResult(face_list, result, 1.0f, m_config->labels.size);

#ifndef _HOST_LINUX
	// get current timestamp
	struct timeval tv;
	gettimeofday(&tv, NULL);
	TIMESPEC_S timespec;
	timespec.tv_sec = tv.tv_sec;
	timespec.tv_nsec = tv.tv_usec * 1000;
	struct tm localtime;
	localtime_r(&timespec.tv_sec, &localtime);
	timespec.tv_sec = mktime(&localtime);

	// dump InfImage
	InfImage wimg = GetInputImage();
	VFTR_dumpStart();
	VFTR_dump(&wimg, sizeof(wimg), 
			COMPOSE_EAIF_FLAG_VER_1(InfImage, InfImage), timespec);
	VFTR_dump(wimg.data, wimg.w * wimg.h * wimg.c,
			COMPOSE_EAIF_FLAG_VER_1(InfU8Array, InfImage), timespec);

	// dump detection result
	VFTR_dump(result, sizeof(*result),
			COMPOSE_EAIF_FLAG_VER_1(InfDetList, InfDetList), timespec);
	VFTR_dumpEnd();

	for (int i = 0; i < result->size; ++i ) {
		InfDetResult *single_result = &result->data[i];

		// dump face img
		InfImage face_img;
		auto& sx = single_result->rect.sx;
		auto& sy = single_result->rect.sy;
		auto& ex = single_result->rect.ex;
		auto& ey = single_result->rect.ey;
		face_img.w = ex - sx + 1;
		face_img.h = ey - sy + 1;
		face_img.c = wimg.c;
		face_img.buf_owner = 0;
		face_img.dtype = static_cast<InfDataType>(GetImageType(m_type, wimg.c));
		face_img.data = static_cast<uint8_t*>(InputTensorBuffer(0));

		// hint: roi and margin and padding will effect result img
		Inf_ImcropResizeNorm(img, sx, sy, ex, ey, &face_img, face_img.w, face_img.h, &m_config->norm_zeros[0],
                         &m_config->norm_scales[0], m_config->input_int8_scale);

		VFTR_dumpStart();
		VFTR_dump(single_result, sizeof(*single_result),
				COMPOSE_EAIF_FLAG_VER_1(InfDetResult, InfDetList), timespec);
		VFTR_dump(&face_img, sizeof(face_img), 
			COMPOSE_EAIF_FLAG_VER_1(InfFaceImage, InfImage), timespec);
		VFTR_dump(face_img.data, face_img.w * face_img.h * face_img.c,
			COMPOSE_EAIF_FLAG_VER_1(InfFaceU8Array, InfFaceImage), timespec);
		VFTR_dumpEnd();
	}
#endif // _HOST_LINUX

	return ret;
}

void NcnnScrfd::WriteDebugInfo(const MPI_RECT_POINT_S *roi, const Scaler& scale_factor,
							   const std::vector<FaceBox>& face_list)
{
	inf_log_warn("Please use vftr_dump");
}

#endif // USE_NCNN