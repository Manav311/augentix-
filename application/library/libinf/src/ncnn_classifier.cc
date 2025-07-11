#ifdef USE_NCNN
#include "inf_classifier.h"

#include <cstdio>
#include <ctime>

#include "inf_types.h"
#include "inf_image.h"
#include "inf_model.h"

#include "inf_log.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"
#include "ncnn_classifier.h"
#include "vftr_dump.h"
#include "eaif_dump_define.h"

int NcnnClassifier::LoadModels(const std::string& model_path)
{
	SetupVerboseModeFromEnvironment();

	// Init model options (can skipped)
	m_model.opt.use_vulkan_compute = false;
	m_model.opt.use_int8_inference = true;
	m_model.opt.use_a53_a55_optimized_kernel = true;
	m_model.opt.use_fp16_packed = false;
	m_model.opt.use_fp16_storage = false;
	m_model.opt.use_fp16_arithmetic = false;
	m_model.opt.num_threads = 1;

	// Load model files
	std::string model_path_ = model_path + ".param";
	const char* param_path_char = model_path_.c_str();
	if (m_model.load_param(param_path_char)){
		inf_log_err("Cannot find %s for %s!", param_path_char, __func__);
		return -1;
	}

	model_path_ = model_path + ".bin";
	const char* bin_path_char = model_path_.c_str();
	if (m_model.load_model(bin_path_char)){
		inf_log_err("Cannot find %s for %s!", bin_path_char, __func__);
		return -1;
	}

	// Init input/output info from config.ini
	m_input_dim[1] = m_config->w; // m_config->w
	m_input_dim[0] = m_config->h; // m_config->h
	m_input_dim[2] = m_config->c; // m_config->c

	m_output_dim[0] = 1;
	m_output_dim[1] = m_config->labels.size;

	m_config->dtype = Inf8S; // Always Inf8S for ncnn

	// Create and allocate input buffer
	in_Mat.create(m_input_dim[1], m_input_dim[0], m_input_dim[2], (size_t)1u);

	if (m_config->dtype == InfUnknownType) {
		inf_log_err("Unknown model dtype for %s!", model_path.c_str());
		return -1;
	}

	inf_log_notice("NCNN model input dim is %dx%dx%d (%s), output: %dx%d, normalization factors: %f %f %f",
				   m_input_dim[0], m_input_dim[1], m_input_dim[2], GetDTypeString(m_config->dtype),
				   m_output_dim[0], m_output_dim[1],
				   m_config->norm_zeros[0], m_config->norm_scales[0], m_config->input_int8_scale);

	return 0;
}

int NcnnClassifier::Classify(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, InfResultList* result)
{
	const int input_h = m_input_dim[0];
	const int input_w = m_input_dim[1];
	const int input_chn = m_input_dim[2];
	const int output_dim = m_output_dim[1];
	const int resize_aspect_ratio = m_config->resize_aspect_ratio;
	const float conf_thresh = m_config->conf_thresh.data[0];
	result->size = obj_list->obj_num;
	result->data = static_cast<InfResult*>(calloc(result->size, sizeof(InfResult)));

	InfImage wimg;

	wimg.w = m_input_dim[1];
	wimg.h = m_input_dim[0];
	wimg.c = m_input_dim[2];
	wimg.buf_owner = 0;

	// Why don't we check this early?
	if (m_config->dtype != Inf8U && m_config->dtype != Inf8S) {
		inf_log_warn("Currently not support floating point inference!");
		return 0;
	}

	wimg.dtype = static_cast<InfDataType>(GetImageType(m_config->dtype, input_chn));
	wimg.data = static_cast<uint8_t*>(InputTensorBuffer(0));

#ifndef _HOST_LINUX
	VFTR_dumpStart();
	VFTR_dumpWithJiffies(obj_list, sizeof(*obj_list), 
						 COMPOSE_EAIF_FLAG_VER_1(InfObjList, InfObjList), obj_list->timestamp);
	VFTR_dumpEnd();
#endif
	for (int i = 0; i < obj_list->obj_num; ++i) {
		InfResult* single_result = &result->data[i];

		auto& obj = obj_list->obj[i];
		auto& box = obj.rect;

		if (m_verbose)
			TIC(start);

        // ncnn Mat is aligned by 16, but Inf_Image is not
		if (resize_aspect_ratio) {
			Inf_ImcropResizeNormAspectRatio(img, box.sx, box.sy, box.ex, box.ey, &wimg, input_w, input_h, &m_config->norm_zeros[0],
                                        &m_config->norm_scales[0], m_config->input_int8_scale);
		} else {
			Inf_ImcropResizeNorm(img, box.sx, box.sy, box.ex, box.ey, &wimg, input_w, input_h, &m_config->norm_zeros[0],
                                        &m_config->norm_scales[0], m_config->input_int8_scale);
		}

#ifndef _HOST_LINUX
		VFTR_dumpStart();
		VFTR_dumpWithJiffies(&wimg, sizeof(wimg), 
				COMPOSE_EAIF_FLAG_VER_1(InfImage, InfImage), obj_list->timestamp);
		VFTR_dumpWithJiffies(wimg.data, wimg.w * wimg.h * wimg.c,
				COMPOSE_EAIF_FLAG_VER_1(InfU8Array, InfImage), obj_list->timestamp);
		VFTR_dumpEnd();
#endif

		if (m_verbose) {
			TOC("Preprocess WImage for obj", start);
			TIC(start);
		}

		ncnn::Extractor ex = m_model.create_extractor();
		// Insert image to ncnn model
		if(ex.input("images", in_Mat) == -1){
			inf_log_err("ncnn input error");
			return 0;
		}
		// Extract output from ncnn model
		if(ex.extract("output0", out_Mat) == -1) {
			inf_log_err("ncnn extract error");
			return 0;
		}

		if (m_verbose)
			TOC("Inference Call", start);

		std::vector<float> output_buf;

		auto cursor = static_cast<float*>(OutputTensorBuffer(0));
		std::for_each(cursor, cursor + output_dim,
					  [&](auto n) { output_buf.push_back(n); });

		if (m_verbose) {
			PrintResult(output_dim, output_buf.data());
		}

		if (m_config->output_type == InfSigmoid) {
			Sigmoid(output_buf.data(), m_config->labels.size);
		} else if (m_config->output_type == InfSoftmax) {
			Softmax(output_buf.data(), m_config->labels.size);
		}

		if (m_verbose) {
			PrintResult(output_dim, output_buf.data());
		}

		PostProcess(output_buf.data(), m_config->labels.size, conf_thresh, m_config->topk,
		            &m_config->filter_cls, &m_config->filter_out_cls, single_result);

		single_result->id = obj.id;

		out_Mat.release();

	}
#ifndef _HOST_LINUX
		VFTR_dumpStart();
		VFTR_dumpWithJiffies(result, sizeof(*result),
				COMPOSE_EAIF_FLAG_VER_1(InfResultList, InfResultList), obj_list->timestamp);

		for (int i = 0; i < result->size; ++i ) {
			InfResult *single_result = &result->data[i];

			VFTR_dumpWithJiffies(single_result, sizeof(*single_result),
					COMPOSE_EAIF_FLAG_VER_1(InfResult, InfResultList), obj_list->timestamp);
			VFTR_dumpWithJiffies(single_result->cls, sizeof(int) * single_result->cls_num,
					COMPOSE_EAIF_FLAG_VER_1(InfIntArray, InfResultList), obj_list->timestamp);
			VFTR_dumpWithJiffies(single_result->prob, sizeof(float) * single_result->prob_num,
					COMPOSE_EAIF_FLAG_VER_1(InfFloatArray, InfResultList), obj_list->timestamp);
		}

		VFTR_dumpEnd();
#endif

	return 0;
}


/**
 * @brief Invoke classification model with image.
 * @details
 * @param[in] ctx              model context
 * @param[in] img              image input
 * @param[in] obj_list         obj list w.r.t current image
 * @param[out] result          classification result
 * @retval 0                   success
 * @retval -EFAULT             input variables are null.
 * @see Inf_InitModel()
 */
extern "C" int Inf_InvokeClassify(InfModelCtx* ctx, const InfImage* img,
								  const MPI_IVA_OBJ_LIST_S* obj_list, InfResultList* result)
{
	retIfNull(ctx && ctx->model && img && obj_list && result);
	if (ctx->info->inference_type != InfRunClassify) {
		inf_log_err("Incompatible model type (%s) inference %s!",
					GetInfTypeStr(ctx->info->inference_type), __func__);
		return 0;
	}

	auto inf_model = reinterpret_cast<InfModel*>(ctx->model);
	return inf_model->Classify(img, obj_list, result);
}

#endif // USE_NCNN
