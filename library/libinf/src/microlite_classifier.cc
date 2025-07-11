#ifdef USE_MICROINF

#include <stdio.h>
#include <string.h>
#include <time.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/schema/schema_generated.h"
#pragma GCC diagnostic pop

#include "inf_classifier.h"
#include "lite_utils.h"
#include "lite_trc.h"
#include "model.h"

static struct timespec start;

class InfClassifier
{
    public:
	InfClassifier(const char *model_config);
	int Classify(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfResultList *result);
	int LoadModels(const char *model_path);
	~InfClassifier();

	const tflite::Model *m_model_fb = nullptr;

	tflite::MicroInterpreter *m_model = nullptr;
	tflite::MicroErrorReporter micro_error_reporter;

	InfModelInfo m_config = {};
	uint8_t *m_tensor_arena = nullptr;
	uint8_t *m_model_data = nullptr;
	int m_model_len = 0;
	int m_tensor_size = 0;
	int m_input_dim[3] = { 0 }; /* h x w x c */
	int m_output_dim[4] = { 0 }; /* b x result x reserved*/
	int m_verbose = 0;
	int m_debug = 0;
	int m_num_thread = 1;
};

InfClassifier::InfClassifier(const char* model_config)
{
	ParseModelConfig(model_config, &m_config);
}

InfClassifier::~InfClassifier()
{
	ReleaseConfig(&m_config);
	delete m_model;
	m_model = 0;
	delete[] m_tensor_arena;
	m_tensor_arena = 0;
#ifndef STATIC_MODEL
	free(m_model_data);
	m_model_data = 0;
#endif // STATIC_MODEL
}

int Inf_InitModel(InfClassifierCtx *ctx, const char* config)
{
	InfClassifier *model = new InfClassifier(config);
	ctx->info = &model->m_config;
	int ret = model->LoadModels(model->m_config.model_path);
	if (ret) {
		delete model;
	} else {
		ctx->model = (void*) model;
	}
	return ret;
}

int Inf_Invoke(InfClassifierCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfResultList *result)
{
	InfClassifier *inf_model = (InfClassifier*)ctx->model;
	return inf_model->Classify(img, obj_list, result);
}

int Inf_ReleaseModel(InfClassifierCtx *ctx)
{
	delete (InfClassifier*)ctx->model;
	ctx->model = nullptr;
	ctx->info = nullptr;
	return 0;
}

int Inf_ReleaseResult(InfResultList *result)
{
	if (!result->size)
		return 0;

	for (int i = 0; i < result->size; i++)
	{
		auto& obj = result->data[i];
		free(obj.cls);
		free(obj.prob);
	}
	free(result->data);
	result->data = nullptr;
	result->size = 0;
	return 0;
}

int Inf_Setup(InfClassifierCtx *ctx, int verbose, int debug, int num_thread)
{
	InfClassifier* model = (InfClassifier*)ctx->model;
	model->m_verbose = verbose;
	model->m_debug = debug;
	model->m_num_thread = num_thread;
	return 0;
}

int InfClassifier::LoadModels(const char *model_path)
{
	// 1. Build the interpreter
#ifdef STATIC_MODEL
	m_model_fb = ::tflite::GetModel(g_model_data);
#else
	m_model_data = LoadModelData(model_path, &m_model_len);
	if (!m_model_data)
		return -1;
	m_model_fb = ::tflite::GetModel(m_model_data);
#endif // STATIC_MODEL

	if (m_model_fb == nullptr) {
		//inf_log_err("Cannot parse %s model data", g_model_name);
		inf_log_err("Cannot parse model data from %s!", model_path);
		return -1;
	}

	if (m_model_fb->version() != TFINF_SCHEMA_VERSION) {
		inf_log_err("Model version does not match. Schema: %d vs model: %d.", TFINF_SCHEMA_VERSION, m_model_fb->version());
		return -1;
	}

	REGISTER_STATIC_OP_RESOLVER(micro_op_resolver);

	m_tensor_size = MODEL_ARENA_SIZE;
	m_tensor_arena = new uint8_t[MODEL_ARENA_SIZE];

	m_model = new tflite::MicroInterpreter(
		m_model_fb, micro_op_resolver, m_tensor_arena, m_tensor_size, &micro_error_reporter);

	inf_check(m_model->AllocateTensors() == kTfInfOk);
	inf_log_notice("model arena used total %u bytes", (uint32_t)m_model->arena_used_bytes());
	// 2.1 Fetch Quant info
	TfInfTensor* input = m_model->input(0);
	TfInfTensor* output = m_model->output(0);
	const TfInfQuantizationParams *p = &output->params;
	m_config.quant_zero = p->zero_point;
	m_config.quant_scale = p->scale;

	const TfInfIntArray *outdims = output->dims;
	m_output_dim[0] = outdims->data[0];
	m_output_dim[1] = outdims->data[1];

	const TfInfIntArray *indims = input->dims;
	m_input_dim[0] = indims->data[1];
	m_input_dim[1] = indims->data[2];
	m_input_dim[2] = indims->data[3];

	switch (input->type) {
		case kTfInfUInt8: {
			m_config.dtype = Inf8U;
			break;
		} case kTfInfInt8: {
			m_config.dtype = Inf8S;
			break;
		} case kTfInfFloat32: {
			m_config.dtype = Inf32F;
			break;
		} default: {
			inf_log_err("Inf classify inference is not implemented for dtype %d yet!", input->type);
			return -1;
		}
	}

	inf_log_notice("Microlite model input dim is %dx%dx%d (%d), output is %dx%d",
		m_input_dim[0], m_input_dim[1], m_input_dim[2], m_config.dtype, m_output_dim[0], m_output_dim[1]);

	return 0;
}

int InfClassifier::Classify(const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, InfResultList *result)
{
	int i;
	tflite::MicroInterpreter *interpreter = m_model;

	const int input_idx = 0;
	const int output_idx = 0;

	const int input_chn = m_input_dim[2];

	result->size = obj_list->obj_num;
	result->data = (InfResult*) calloc(result->size, sizeof(InfResult));

	float *output_buf = new float[m_output_dim[1]];

	if (m_config.dtype == Inf8U ) {

		const int zero = m_config.quant_zero;
		const float scale = m_config.quant_scale;

		uint8_t *input_addr = interpreter->typed_input_tensor<uint8_t>(input_idx);
		uint8_t *output_addr = interpreter->typed_output_tensor<uint8_t>(output_idx);
		InfResult *single_result = nullptr;
		InfImage wimg;

		wimg.w = m_input_dim[1];
		wimg.h = m_input_dim[0];
		wimg.c = m_config.c;
		wimg.dtype = (InfDataType)GetImageType((int)Inf8U, input_chn);
		wimg.data = (uint8_t*)input_addr;
		wimg.buf_owner = 0;

		for (i = 0; i < obj_list->obj_num; ++i) {

			single_result = &result->data[i];

			auto &obj = obj_list->obj[i];
			auto &box = obj.rect;

			if (m_verbose)
				TIC(start);

			ImcropResize(img, box.sx, box.sy, box.ex, box.ey, &wimg, m_input_dim[1], m_input_dim[0]);

			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);

			if (interpreter->Invoke() != kTfInfOk) {
				inf_log_err("Cannot Invoke tflite model!");
				return 0;
			}

			if (m_verbose)
				TOC("Inference Call", start);

			for (int j = 0; j < m_output_dim[1]; j++)
				output_buf[j] = QuantConvert(output_addr[j], zero, scale);

			if (m_verbose)
				PrintResult(m_output_dim[1], output_buf);

			if (m_config.output_type == InfSigmoid)
				Sigmoid(output_buf, m_config.labels.size);

			PostProcess(output_buf, m_config.labels.size,
				m_config.conf_thresh, m_config.topk,
				&m_config.filter_cls, &m_config.filter_out_cls, single_result);

			single_result->id = obj.id;
		}

	} else if (m_config.dtype == Inf8S ) {

		const int zero = m_config.quant_zero;
		const float scale = m_config.quant_scale;

		int8_t *input_addr = interpreter->typed_input_tensor<int8_t>(input_idx);
		int8_t *output_addr = interpreter->typed_output_tensor<int8_t>(output_idx);
		InfResult *single_result = nullptr;
		InfImage wimg;

		wimg.w = m_input_dim[1];
		wimg.h = m_input_dim[0];
		wimg.c = m_config.c;
		wimg.dtype = (InfDataType)GetImageType((int)Inf8S, input_chn);
		wimg.data = (uint8_t*)input_addr;
		wimg.buf_owner = 0;

		for (i = 0; i < obj_list->obj_num; ++i) {

			single_result = &result->data[i];

			auto &obj = obj_list->obj[i];
			auto &box = obj.rect;

			if (m_verbose)
				TIC(start);

			ImcropResize(img, box.sx, box.sy, box.ex, box.ey, &wimg, m_input_dim[1], m_input_dim[0]);

			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);

			if (interpreter->Invoke() != kTfInfOk) {
				inf_log_err("Cannot Invoke tflite model!");
				return 0;
			}

			if (m_verbose)
				TOC("Inference Call", start);

			for (int j = 0; j < m_output_dim[1]; j++)
				output_buf[j] = QuantConvertS(output_addr[j], zero, scale);

			if (m_verbose)
				PrintResult(m_output_dim[1], output_buf);

			if (m_config.output_type == InfSigmoid)
				Sigmoid(output_buf, m_config.labels.size);

			PostProcess(output_buf, m_config.labels.size,
				m_config.conf_thresh, m_config.topk,
				&m_config.filter_cls, &m_config.filter_out_cls, single_result);

			single_result->id = obj.id;
		}
	} else {
		inf_log_err("Currently not support floating point inference!");
	}
	delete[] output_buf;

	return 0;
}

#endif // USE_MICROINF