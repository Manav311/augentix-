#ifdef USE_TFLITE

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"

#include "lite_classifier.h"
#include "lite_utils.h"
#include "lite_trc.h"

static struct timespec start;

class LiteClassifier
{
    public:
	LiteClassifier(const char *model_config);
	int Classify(const LiteImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, LiteResultList *result);
	int LoadModels(const char *model_path);
	~LiteClassifier();

	std::unique_ptr<tflite::Interpreter> m_model;
	std::unique_ptr<tflite::FlatBufferModel> m_model_fb;

	LiteModelInfo m_config = {};
	int m_input_dim[3]; /* h x w x c */
	int m_output_dim[4]; /* b x result x reserved*/
	int m_verbose = 0;
	int m_debug = 0;
	int m_num_thread = 1;

	int m_snapshot_cnt = 0;
	char m_snapshot_prefix[256] = {};
};

LiteClassifier::LiteClassifier(const char* model_config)
{
	ParseModelConfig(model_config, &m_config);
}

LiteClassifier::~LiteClassifier()
{
	ReleaseConfig(&m_config);
}


int LiteClassifier::LoadModels(const char *model_path)
{
	// 1. Build the interpreter
	m_model_fb = tflite::FlatBufferModel::BuildFromFile(model_path);
	if (m_model_fb == nullptr) {
		lite_err("Cannot find %s.\n", model_path);
		m_model.reset();
		return -1;
	}

	tflite::ops::builtin::BuiltinOpResolver resolver;
	tflite::InterpreterBuilder builder(*m_model_fb, resolver);
	builder(&m_model);
	if (m_model == nullptr) {
		lite_err("Cannot find %s.\n", model_path);
		return -1;
	}

	// 2. Fetch model info
	std::vector<int> outputs = m_model->outputs();
	if (outputs.size() != 1) {
		lite_err("number of output is not correct!\n");
	}

	// 2.1 Fetch Quant info
	const TfLiteQuantizationParams *p = &m_model->tensor(outputs[0])->params;
	m_config.quant_zero = p->zero_point;
	m_config.quant_scale = p->scale;

	const int output_idx = m_model->outputs()[0];
	const int input_idx = m_model->inputs()[0];
	const TfLiteIntArray *outdims = m_model->tensor(output_idx)->dims;

	m_output_dim[0] = outdims->data[0];
	m_output_dim[1] = outdims->data[1];

	const TfLiteIntArray *indims = m_model->tensor(input_idx)->dims;
	m_input_dim[0] = indims->data[1];
	m_input_dim[1] = indims->data[2];
	m_input_dim[2] = indims->data[3];

	switch (m_model->tensor(input_idx)->type) {
		case kTfLiteUInt8: {
			m_config.dtype = Lite8U;
			break;
		} case kTfLiteInt8: {
			m_config.dtype = Lite8S;
			break;
		} case kTfLiteFloat32: {
			m_config.dtype = Lite32F;
			break;
		} default: {
			lite_err("Lite classify inference is not implemented for this datatype yet %d!\n", m_model->tensor(input_idx)->type);
			return -1;
		}
	}

	lite_info_h("tflite model input dim is %dx%dx%d type is %d\n",
		m_input_dim[0], m_input_dim[1], m_input_dim[2], m_config.dtype);
	lite_info_h("tflite model number of output is %dx%d\n", m_output_dim[0], m_output_dim[1]);

	m_model->SetNumThreads(m_num_thread);

	lite_check(m_model->AllocateTensors() == kTfLiteOk);

	return 0;
}

int LiteClassifier::Classify(const LiteImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, LiteResultList *result)
{
	int i;
	tflite::Interpreter *interpreter = m_model.get();

	const int input = interpreter->inputs()[0];
	const int output_idx = interpreter->outputs()[0];

	const int input_chn = m_input_dim[2];
	const int resize_aspect_ratio = m_config.resize_aspect_ratio;

	result->size = obj_list->obj_num;
	result->data = (LiteResult*) calloc(result->size, sizeof(LiteResult));

	if (m_config.dtype == Lite8U) {
		std::vector<float> output_buf(m_output_dim[1],0.0);

		const int zero = m_config.quant_zero;
		const float scale = m_config.quant_scale;

		uint8_t *input_addr = interpreter->typed_tensor<uint8_t>(input);
		uint8_t *output_addr = interpreter->typed_tensor<uint8_t>(output_idx);
		LiteResult *single_result = nullptr;
		LiteImage wimg;

		wimg.w = m_input_dim[1];
		wimg.h = m_input_dim[0];
		wimg.c = m_config.c;
		wimg.dtype = (LiteDataType)GetImageType((int)Lite8U, input_chn);
		wimg.data = (uint8_t*)input_addr;
		wimg.buf_owner = 0;

		for (i = 0; i < obj_list->obj_num; ++i) {

			single_result = &result->data[i];

			auto &obj = obj_list->obj[i];
			auto &box = obj.rect;

			if (m_verbose)
				TIC(start);

			if (resize_aspect_ratio)
				ImcropResizeAspectRatio(img, box.sx, box.sy, box.ex, box.ey, &wimg, m_input_dim[1], m_input_dim[0]);
			else
				ImcropResize(img, box.sx, box.sy, box.ex, box.ey, &wimg, m_input_dim[1], m_input_dim[0]);

			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);

			if (interpreter->Invoke() != kTfLiteOk) {
				lite_warn("Cannot Invoke tflite model!\n");
				return 0;
			}

			if (m_verbose)
				TOC("Inference Call", start);

			for (int j = 0; j < m_output_dim[1]; j++)
				output_buf[j] = QuantConvert(output_addr[j], zero, scale);

			if (m_verbose)
				PrintResult(m_output_dim[1], output_buf.data());

			if (m_config.output_type == LiteSigmoid)
				Sigmoid(output_buf.data(), m_config.labels.size);

			PostProcess(output_buf.data(), m_config.labels.size,
				m_config.conf_thresh, m_config.topk,
				&m_config.filter_cls, &m_config.filter_out_cls, single_result);

			single_result->id = obj.id;
		}
	} else if (m_config.dtype == Lite8S ) {
		std::vector<float> output_buf(m_output_dim[1],0.0);

		const int zero = m_config.quant_zero;
		const float scale = m_config.quant_scale;

		int8_t *input_addr = interpreter->typed_tensor<int8_t>(input);
		int8_t *output_addr = interpreter->typed_tensor<int8_t>(output_idx);
		LiteResult *single_result = nullptr;
		LiteImage wimg;

		wimg.w = m_input_dim[1];
		wimg.h = m_input_dim[0];
		wimg.c = m_config.c;
		wimg.dtype = (LiteDataType)GetImageType((int)Lite8S, input_chn);
		wimg.data = (uint8_t*)input_addr;
		wimg.buf_owner = 0;

		for (i = 0; i < obj_list->obj_num; ++i) {

			single_result = &result->data[i];

			auto &obj = obj_list->obj[i];
			auto &box = obj.rect;

			if (m_verbose)
				TIC(start);

			if (resize_aspect_ratio)
				ImcropResizeAspectRatio(img, box.sx, box.sy, box.ex, box.ey, &wimg, m_input_dim[1], m_input_dim[0]);
			else
				ImcropResize(img, box.sx, box.sy, box.ex, box.ey, &wimg, m_input_dim[1], m_input_dim[0]);

			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);

			if (interpreter->Invoke() != kTfLiteOk) {
				lite_warn("Cannot Invoke tflite model!\n");
				return 0;
			}

			if (m_verbose)
				TOC("Inference Call", start);

			for (int j = 0; j < m_output_dim[1]; j++)
				output_buf[j] = QuantConvertS(output_addr[j], zero, scale);

			if (m_verbose)
				PrintResult(m_output_dim[1], output_buf.data());

			if (m_config.output_type == LiteSigmoid)
				Sigmoid(output_buf.data(), m_config.labels.size);

			if (m_debug) {
				char snapshot_img_name[256] = {};
				sprintf(snapshot_img_name, SNAPSHOT_FORMAT, m_snapshot_prefix,
					output_buf[0] >= m_config.conf_thresh ? "pos" : "neg",
					m_snapshot_cnt,
					output_buf[0]);
				ImsavePgm(snapshot_img_name, &wimg);
				m_snapshot_cnt++;
			}

			PostProcess(output_buf.data(), m_config.labels.size,
				m_config.conf_thresh, m_config.topk,
				&m_config.filter_cls, &m_config.filter_out_cls, single_result);

			single_result->id = obj.id;
		}
	} else {
		lite_warn("Currently not support floating point inference!\n");
	}
	return 0;
}

int Lite_InitModel(LiteClassifierCtx *ctx, const char* config)
{
	LiteClassifier *model = new LiteClassifier(config);
	ctx->info = &model->m_config;
	int ret = model->LoadModels(model->m_config.model_path);
	if (ret) {
		delete model;
	} else {
		ctx->model = (void*) model;
	}
	return ret;
}

int Lite_Invoke(LiteClassifierCtx *ctx, const LiteImage *img, const MPI_IVA_OBJ_LIST_S *obj_list, LiteResultList *result)
{
	LiteClassifier *lite_model = (LiteClassifier*)ctx->model;
	return lite_model->Classify(img, obj_list, result);
}

int Lite_ReleaseModel(LiteClassifierCtx *ctx)
{
	delete (LiteClassifier*)ctx->model;
	ctx->model = nullptr;
	ctx->info = nullptr;
	return 0;
}

int Lite_ReleaseResult(LiteResultList *result)
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

int Lite_Setup(LiteClassifierCtx *ctx, int verbose, int debug, int num_thread)
{
	LiteClassifier* model = (LiteClassifier*)ctx->model;
	model->m_verbose = verbose;
	model->m_debug = debug;
	model->m_num_thread = num_thread;
	if (debug) {
		model->m_debug = SetupDebugTool(model->m_snapshot_prefix);
	}
	return 0;
}

#endif // USE_TFLITE