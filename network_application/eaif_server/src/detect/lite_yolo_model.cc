#ifdef USE_TFLITE

#include <string>
#include <vector>

#include "eaif_utils.h"
#include "eaif_trc.h"
#include "eaif_image.h"

#include "lite_yolo_model.h"

using namespace std;
using namespace eaif::image;

static struct timespec start;

int LiteYolov5Model::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose) TIC(start);
	string model_loc = model_dir + "/" + model_path[0];
	tflite::utils::LoadModels(model_loc.c_str(), model_, model_fb_);
	tflite::utils::GetQuantInfo(model_, &model_info_, 1);

	const int output_idx = model_->outputs()[0];
	const int input_idx = model_->inputs()[0];
	const TfLiteIntArray* outdims = model_->tensor(output_idx)->dims;

#if 1
	assert(outdims->size == 3);
#endif
	output_dim_[0] = outdims->data[1];
	output_dim_[1] = outdims->data[2];

	const TfLiteIntArray* indims = model_->tensor(input_idx)->dims;
	input_dim_[0] = indims->data[1];
	input_dim_[1] = indims->data[2];

	type_ = (model_->tensor(input_idx)->type == kTfLiteUInt8) ? Eaif8U : Eaif32F;

	eaif_info_h("YOLOV5 model type is %s\n", (type_ == Eaif8U) ? "uint8" : "float32");
	eaif_info_h("YOLOV5 model number of output is %dx%d\n", output_dim_[0], output_dim_[1]);

	model_->SetNumThreads(m_nthread);

	eaif_check(kTfLiteOk == model_->AllocateTensors());
	if (m_verbose) TOC("Load model",start);
	SetPostProcessMethod(10);
	return 0;
}


int LiteYolov5Model::Detect(const void* img, std::vector<Detection>& detections, const ModelConfig& conf)
{
	/* Get input/output tensor index */

	tflite::Interpreter* interpreter = model_.get();
	const int input = interpreter->inputs()[0];
	const int output_idx = interpreter->outputs()[0];

	// TBD for quantize model
	float* input_addr = interpreter->typed_tensor<float>(input);
	float* output_addr = interpreter->typed_tensor<float>(output_idx);

	WImage *pimg = (WImage*)img;
	WImage wimg;
	const Shape img_sz_net(input_dim_[1], input_dim_[0]);
	const Shape img_sz_raw(pimg->cols, pimg->rows);

	if (m_verbose) TIC(start);
	if (type_ == Eaif8U) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		Imresize(*pimg, wimg, input_dim_[1], input_dim_[0]);
	} else {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif32FC3, input_addr);
		ImresizeNorm(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data());
	}
	if (m_verbose) TOC("Preprocess WImage", start);

	if (m_verbose) TIC(start);
	eaif_check(interpreter->Invoke() == kTfLiteOk);
	if (m_verbose) TOC("Inference Call", start);

#if 0
	auto output_ids = interpreter->outputs();
	float *output_addr_;
	for (unsigned int i = 0; i < output_ids.size(); ++i) {
		output_addr_ = interpreter->typed_tensor<float>(output_ids[i]);
		const TfLiteIntArray* outdims = interpreter->tensor(output_ids[i])->dims;
		int total_size = 1;
		for (int j = 0; j < outdims->size; ++j) {
			total_size *= outdims->data[j];
		}
		eaif_info_h("dump size: %d \n", total_size);
		dump((const uint8_t*)output_addr_, total_size * sizeof(float), i);
	}
#endif
	// TBD for quantize model convert to floating point

	if (GetPostProcessMethod() == 10) {
		COND_TIMER_FUNC(m_verbose, "PostProcess",
		detect::PostProcessV1(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections));
	} else if (GetPostProcessMethod() == 11) {
		detect::PostProcessV1_1(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections);
	}
	return 0;
}


int LiteYolov4Model::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose) TIC(start);
	string model_loc = model_dir + "/" + model_path[0];
	tflite::utils::LoadModels(model_loc.c_str(), model_, model_fb_);
	tflite::utils::GetQuantInfo(model_, &model_info_, 1);

	const int output_idx = model_->outputs()[0];
	const int input_idx = model_->inputs()[0];
	const TfLiteIntArray* outdims = model_->tensor(output_idx)->dims;

#if 1
	assert(outdims->size == 3);
#endif
	output_dim_[0] = outdims->data[1];
	output_dim_[1] = outdims->data[2];

	const TfLiteIntArray* indims = model_->tensor(input_idx)->dims;
	input_dim_[0] = indims->data[1];
	input_dim_[1] = indims->data[2];

	type_ = (model_->tensor(input_idx)->type == kTfLiteUInt8) ? Eaif8U : Eaif32F;

	eaif_info_h("YOLOV4 model type is %s\n", (type_ == Eaif8U) ? "uint8" : "float32");
	eaif_info_h("YOLOV4 model number of output is %dx%d\n", output_dim_[0], output_dim_[1]);

	model_->SetNumThreads(m_nthread);

	eaif_check(kTfLiteOk == model_->AllocateTensors());
	if (m_verbose) TOC("Load model",start);
	SetPostProcessMethod(20);
	return 0;
}

int LiteYolov4Model::Detect(const void* img, std::vector<Detection>& detections, const ModelConfig& conf)
{
	/* Get input/output tensor index */
	tflite::Interpreter* interpreter = model_.get();
	const int input = interpreter->inputs()[0];
	const int output_idx = interpreter->outputs()[0];

	float* input_addr = interpreter->typed_tensor<float>(input);
	float* output_addr = interpreter->typed_tensor<float>(output_idx);

	WImage *pimg = (WImage*)img;
	WImage wimg;
	const Shape img_sz_net(input_dim_[1], input_dim_[0]);
	const Shape img_sz_raw(pimg->cols, pimg->rows);

	if (m_verbose) TIC(start);
	if (type_ == Eaif8U) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		Imresize(*pimg, wimg, input_dim_[1], input_dim_[0]);
	} else {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif32FC3, input_addr);
		ImresizeNorm(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data());
	}
	if (m_verbose) TOC("Preprocess WImage", start);

	if (m_verbose) TIC(start);
	eaif_check(interpreter->Invoke() == kTfLiteOk);
	if (m_verbose) TOC("Inference Call", start);

#if 0
	auto output_ids = interpreter->outputs();
	float *output_addr_;
	for (unsigned int i = 0; i < output_ids.size(); ++i) {
		output_addr_ = interpreter->typed_tensor<float>(output_ids[i]);
		const TfLiteIntArray* outdims = interpreter->tensor(output_ids[i])->dims;
		int total_size = 1;
		for (int j = 0; j < outdims->size; ++j) {
			total_size *= outdims->data[j];
		}
		eaif_info_h("dump size: %d \n", total_size);
		dump((const uint8_t*)output_addr_, total_size * sizeof(float), i);
	}
#endif
	if (GetPostProcessMethod() == 20) {
		COND_TIMER_FUNC(m_verbose, "PostProcess",
		detect::PostProcessV2(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections));
	} else if (GetPostProcessMethod() == 21) {
		detect::PostProcessV2_1(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections);
	}
	return 0;
}
#endif /* USE_TFLITE */
