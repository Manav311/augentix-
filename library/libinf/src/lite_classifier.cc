#ifndef USE_NCNN
#include "inf_classifier.h"

#include <cstdio>
#include <ctime>

#ifndef USE_MICROLITE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/lite/interpreter.h"
#pragma GCC diagnostic pop
#endif

#include "inf_types.h"
#include "inf_image.h"
#include "inf_model.h"

#include "inf_log.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"
#include "lite_classifier.h"
#include "vftr_dump.h"
#include "eaif_dump_define.h"

int LiteClassifierBase::LoadModels(const std::string& model_path)
{
	SetupVerboseModeFromEnvironment();

	// 1. Prepare the interpreter: Create interpreter and allocate tensors.
	if (!PrepareInterpreter(model_path)) {
		return -1;
	}

	size_t used_bytes = GetArenaUsedBytes();
	if (used_bytes > 0) {
		inf_log_notice("%s had used %zd bytes of arena.", model_path.c_str(), used_bytes);
	}

	// 2. Collect model info
	CollectModelInfo();

	// 3. check some prerequisites
	if (m_nums_output_tensor != 1) {
		inf_log_err("Number of output is incorrect! Expected 1 but got %zd.", m_nums_output_tensor);
		return -1;
	}

	if (m_config->dtype == InfUnknownType) {
		inf_log_err("Unknown model dtype for %s!", model_path.c_str());
		return -1;
	}

	inf_log_notice("TFLite model input dim is %dx%dx%d (%s). Output: %dx%d",
			   m_input_dim[0], m_input_dim[1], m_input_dim[2], GetDTypeString(m_config->dtype),
	           m_output_dim[0], m_output_dim[1]);

	return 0;
}

int LiteClassifierBase::Classify(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, InfResultList* result)
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

		if (resize_aspect_ratio) {
//			Inf_ImcropResizeAspectRatio(img, box.sx, box.sy, box.ex, box.ey, &wimg, input_w, input_h, &m_config->norm_zeros[0],
//                                        &m_config->norm_scales[0], m_config->input_int8_scale);
			Inf_ImcropResizeAspectRatio(img, box.sx, box.sy, box.ex, box.ey, &wimg, input_w, input_h);
		} else {
//			Inf_ImcropResize(img, box.sx, box.sy, box.ex, box.ey, &wimg, input_w, input_h, &m_config->norm_zeros[0],
//                             &m_config->norm_scales[0], m_config->input_int8_scale);
			Inf_ImcropResize(img, box.sx, box.sy, box.ex, box.ey, &wimg, input_w, input_h);
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

		if (Invoke() != kTfLiteOk) {
			inf_log_warn("Cannot Invoke tflite model!");
			return 0;
		}

		if (m_verbose)
			TOC("Inference Call", start);

		std::vector<float> output_buf;
		if (m_config->dtype == Inf8U) {
			auto cursor = static_cast<uint8_t*>(OutputTensorBuffer(0));
			std::for_each(cursor, cursor + output_dim,
			              [&](uint8_t n) { output_buf.push_back(QuantConvert(m_config->quant, n)); });
		} else {
			auto cursor = static_cast<int8_t*>(OutputTensorBuffer(0));
			std::for_each(cursor, cursor + output_dim,
			              [&](int8_t n) { output_buf.push_back(QuantConvert(m_config->quant, n)); });
		}

		if (m_verbose) {
			PrintResult(output_dim, output_buf.data());
		}

		if (m_config->output_type == InfSigmoid) {
			Sigmoid(output_buf.data(), m_config->labels.size);
		} else if (m_config->output_type == InfSoftmax) {
			Softmax(output_buf.data(), m_config->labels.size);
		}


		PostProcess(output_buf.data(), m_config->labels.size, conf_thresh, m_config->topk,
		            &m_config->filter_cls, &m_config->filter_out_cls, single_result);

		single_result->id = obj.id;
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

bool LiteClassifier::PrepareInterpreter(const std::string& model_path)
{
	if (inf_tf_adapter::LiteClassifier_LoadModel(*this, model_path, m_model, m_model_fb)) {
		inf_log_err("Cannot find %s for %s!", model_path.c_str(), __func__);
		return false;
	}

	if (m_model->AllocateTensors() != kTfLiteOk) {
		inf_log_err("Cannot allocate tensor!");
		return false;
	}

	return true;
}

bool LiteClassifier::CollectModelInfo()
{
	m_nums_output_tensor = inf_tf_adapter::numsOfOutputTensor(*m_model);

	const TfLiteQuantizationParams* p = &m_model->output_tensor(0)->params;
	m_config->quant.zero = p->zero_point;
	m_config->quant.scale = p->scale;

	const TfLiteIntArray* outdims = m_model->output_tensor(0)->dims;

	m_output_dim[0] = outdims->data[0];
	m_output_dim[1] = outdims->data[1];

	const TfLiteIntArray* indims = m_model->input_tensor(0)->dims;
	m_input_dim[0] = indims->data[1];
	m_input_dim[1] = indims->data[2];
	m_input_dim[2] = indims->data[3];

	m_config->dtype = utils::lite::GetDataType(m_model->input_tensor(0)->type);
	return true;
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
