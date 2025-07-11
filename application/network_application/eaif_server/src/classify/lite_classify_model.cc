#ifdef USE_TFLITE
#include <algorithm>

#include "eaif_image.h"
#include "eaif_trc.h"

#include "lite_classify_model.h"

using namespace std;
using namespace eaif::image;

static struct timespec start;

int LiteClassifyModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose)
		TIC(start);
	string model_loc = model_dir + "/" + model_path[0];
	tflite::utils::LoadModels(model_loc.c_str(), model_, model_fb_);
	tflite::utils::GetQuantInfo(model_, &model_info_, 1);

	const int output_idx = model_->outputs()[0];
	const int input_idx = model_->inputs()[0];
	const TfLiteIntArray *outdims = model_->tensor(output_idx)->dims;

#if 0
    assert(outdims->size < 3);
#endif
	output_dim_.resize(2);
	output_dim_[0] = outdims->data[0];
	output_dim_[1] = outdims->data[1];

	const TfLiteIntArray *indims = model_->tensor(input_idx)->dims;
	input_dim_[0] = indims->data[1];
	input_dim_[1] = indims->data[2];
	input_dim_[2] = indims->data[3];

	// TODO connect other data type
	m_type = tflite::utils::GetDataType(model_.get(), input_idx);
	if (m_type == EaifUnknownType)
		return -1;

	eaif_info_h("tflite model input dim is %dx%dx%d type is %s\n",
		input_dim_[0], input_dim_[1], input_dim_[2], eaif_GetDTypeString(m_type));
	eaif_info_h("tflite model number of output is %dx%d\n", output_dim_[0], output_dim_[1]);

	model_->SetNumThreads(m_num_thread);

	eaif_check(kTfLiteOk == model_->AllocateTensors());
	if (m_verbose)
		TOC("Load model", start);
	return 0;
}

static void printResult(int output_dim, float *output_buf)
{
	char msg[1024] = {};
	int size = 0;
	size = sprintf(msg, "model ouput result: ");
	for (int k = 0; k < output_dim; k++)
		size += sprintf(&msg[size], "%.4f,", output_buf[k]);
	eaif_info_h("%s\n", msg);
}

int LiteClassifyModel::Classify(const void *Wimg, Classification &result, const ModelConfig& conf)
{
	tflite::Interpreter *interpreter = model_.get();
	const int input = interpreter->inputs()[0];
	const int output_idx = interpreter->outputs()[0];

	uint8_t *input_addr = nullptr;
	uint8_t *output_addr = nullptr;
	vector<float> output_buf;

	
	if (m_type == Eaif8U) {
		input_addr = interpreter->typed_tensor<uint8_t>(input);
		output_addr = interpreter->typed_tensor<uint8_t>(output_idx);
	} else if (m_type == Eaif8S) {
		input_addr = (uint8_t*)interpreter->typed_tensor<int8_t>(input);
		output_addr = (uint8_t*)interpreter->typed_tensor<int8_t>(output_idx);
	} else if (m_type == Eaif32F) {
		input_addr = (uint8_t*)interpreter->typed_tensor<float>(input);
		output_addr = (uint8_t*)interpreter->typed_tensor<float>(output_idx);
	} else {
		result.idx = 0;
		result.cls.resize(0);
		eaif_warn("Data type Not supported!");
		return 0;
	}

	WImage *pimg = (WImage *)Wimg;

	const Shape img_sz_net(input_dim_[1], input_dim_[0]);
	const Shape img_sz_raw(pimg->cols, pimg->rows);

	const int input_chn = input_dim_[2];

	int imgtype = eaif_GetImageType(m_type, input_chn);
	WImage wimg(input_dim_[0], input_dim_[1], imgtype, input_addr);

	if (m_verbose)
		TIC(start);
	if (m_type == Eaif8U || m_type == Eaif8S) {
		//WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		Imresize(*pimg, wimg, input_dim_[1], input_dim_[0]);
	} else {
		if (conf.resize_keep_ratio)
			ImresizeNormAspectRatio(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data());
		else
			ImresizeNorm(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data());
	}
	if (m_verbose)
		TOC("Preprocess WImage", start);

	if (m_verbose)
		TIC(start);

	eaif_check(interpreter->Invoke() == kTfLiteOk);


	if (m_verbose)
		TOC("Inference Call", start);

	if (m_verbose)
		TIC(start);

	if (m_type == Eaif8U) {
		output_buf.resize(output_dim_[1],0.0);
		for (int j = 0; j < output_dim_[1]; j++)
			output_buf[j] = model_info_.Convertf(output_addr[j]);

		if (m_verbose) {
			printResult(output_dim_[1], output_buf.data());
		}

		if (conf.activation_type == eaif::engine::Linear)
			classify::Sigmoid(output_buf.data(), conf.num_classes);

		classify::PostProcess(output_buf.data(), conf.num_classes, conf.conf_thresh[0], conf.topk, result);
	} else if (m_type == Eaif8S) {
		output_buf.resize(output_dim_[1],0.0);
		int8_t *output_addri = (int8_t*)output_addr;

		for (int j = 0; j < output_dim_[1]; j++)
			output_buf[j] = model_info_.Convertf(output_addri[j]);

		if (m_verbose) {
			printResult(output_dim_[1], output_buf.data());
		}

		if (conf.activation_type == eaif::engine::Linear)
			classify::Sigmoid(output_buf.data(), conf.num_classes);

		classify::PostProcess(output_buf.data(), conf.num_classes, conf.conf_thresh[0], conf.topk, result);

	} else if (m_type == Eaif32F) {
		float *output_addrf = (float*)output_addr;

		if (m_verbose) {
			printResult(output_dim_[1], output_addrf);
		}

		if (conf.activation_type == eaif::engine::Linear)
			classify::Sigmoid((float*)output_addrf, conf.num_classes);
		classify::PostProcess((float*)output_addrf, conf.num_classes, conf.conf_thresh[0], conf.topk, result);
	}

	if (m_verbose)
		TOC("PostProcessing", start);

	return 0;
}

int LiteClassifyModel::ClassifyObjList(const void *Wimg, const vector<ObjectAttr> &obj_list,
                                           vector<Classification> &results, const ModelConfig& conf)
{
	size_t i;
	tflite::Interpreter *interpreter = model_.get();
	const int input = interpreter->inputs()[0];
	const int output_idx = interpreter->outputs()[0];

	WImage *pimg = (WImage *)Wimg;

	const Shape img_sz_net(input_dim_[1], input_dim_[0]);
	const Shape img_sz_raw(pimg->cols, pimg->rows);
	const int input_chn = input_dim_[2];

	int imgtype = eaif_GetImageType(m_type, input_chn);
	if (m_type == Eaif8U) {
		vector<float> output_buf(output_dim_[1],0.0);

		uint8_t *input_addr = interpreter->typed_tensor<uint8_t>(input);
		uint8_t *output_addr = interpreter->typed_tensor<uint8_t>(output_idx);

		WImage wimg(input_dim_[0], input_dim_[1], imgtype, static_cast<uint8_t*>(input_addr));
		for (i = 0; i < obj_list.size(); ++i) {
			Classification result;
			auto &box = obj_list[i].box;

			if (m_verbose)
				TIC(start);
			ImcropResize(*pimg, box.sx, box.sy, box.ex, box.ey, wimg, input_dim_[1], input_dim_[0]);
			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);
			eaif_check(interpreter->Invoke() == kTfLiteOk);

			if (m_verbose)
				TOC("Inference Call", start);

			for (int j = 0; j < output_dim_[1]; j++)
				output_buf[j] = model_info_.Convertf(output_addr[j]);

			if (m_verbose) {
				printResult(output_dim_[1], output_buf.data());
			}

			if (conf.activation_type == eaif::engine::Linear)
				classify::Sigmoid(output_buf.data(), conf.num_classes);

			classify::PostProcess(output_buf.data(), conf.num_classes, conf.conf_thresh[0], conf.topk, result);
			result.idx = obj_list[i].idx;
			results.push_back(std::move(result));
		}
	} else if (m_type == Eaif8S) {
		vector<float> output_buf(output_dim_[1],0.0);

		int8_t *input_addr = interpreter->typed_tensor<int8_t>(input);
		int8_t *output_addr = interpreter->typed_tensor<int8_t>(output_idx);

		WImage wimg(input_dim_[0], input_dim_[1], imgtype, reinterpret_cast<uint8_t*>(input_addr));
		for (i = 0; i < obj_list.size(); ++i) {
			Classification result;
			auto &box = obj_list[i].box;

			if (m_verbose)
				TIC(start);
			ImcropResize(*pimg, box.sx, box.sy, box.ex, box.ey, wimg, input_dim_[1], input_dim_[0]);
			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);
			eaif_check(interpreter->Invoke() == kTfLiteOk);

			if (m_verbose)
				TOC("Inference Call", start);

			for (int j = 0; j < output_dim_[1]; j++)
				output_buf[j] = model_info_.Convertf(output_addr[j]);

			if (m_verbose) {
				printResult(output_dim_[1], output_buf.data());
			}

			if (conf.activation_type == eaif::engine::Linear)
				classify::Sigmoid(output_buf.data(), conf.num_classes);

			classify::PostProcess(output_buf.data(), conf.num_classes, conf.conf_thresh[0], conf.topk, result);
			result.idx = obj_list[i].idx;
			results.push_back(std::move(result));
		}
	} else {
		//using ImresizePtr = void (*)(const WImage&, WImage&, int, int, const float[3], const float[3]);

		//ImresizePtr p_resize_function = (conf.resize_keep_ratio) ? ImresizeNormAspectRatio<WImage> : ImresizeNorm<WImage>;
		float *input_addr = interpreter->typed_tensor<float>(input);
		float *output_addr = interpreter->typed_tensor<float>(output_idx);
		WImage wimg(input_dim_[0], input_dim_[1], imgtype, static_cast<float*>(input_addr));

		for (i = 0; i < obj_list.size(); ++i) {
			Classification result;
			auto &box = obj_list[i].box;

			if (m_verbose)
				TIC(start);

			ImcropResizeNorm(*pimg, box.sx, box.sy, box.ex, box.ey,
				wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data());

			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			if (m_verbose)
				TIC(start);
			eaif_check(interpreter->Invoke() == kTfLiteOk);

			if (m_verbose) {
				printResult(output_dim_[1], output_addr);
			}

			if (m_verbose)
				TOC("Inference Call", start);
			if (conf.activation_type == eaif::engine::Linear)
				classify::Sigmoid(output_addr, conf.num_classes);
			classify::PostProcess(output_addr, conf.num_classes, conf.conf_thresh[0], conf.topk, result);
			result.idx = obj_list[i].idx;
			results.push_back(std::move(result));
		}
	}
	return 0;
}

#endif /* USE_TFLITE */
