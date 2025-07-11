#ifdef USE_ARMNN
#include "eaif_utils.h"
#include "eaif_trc.h"
#include "eaif_image.h"

#include "armnn_yolo_model.h"

using namespace eaif::image;
using namespace std;

static struct timespec start;

int ArmnnYolov5Model::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose) TIC(start);
	string model_loc = model_dir + "/" + model_path[0];
	armnn::utils::LoadModels(model_loc.c_str(), model_);

	m_type = model_.type;
	if (m_type) armnn::utils::GetQuantInfo(model_.model, model_info_);

	armnn::utils::InitModelIO(model_, model_.input_tensors, model_.output_tensors);
	if (m_verbose) TOC("Load model", start);

	const auto& output_shape = model_.output_tensors[0].second.GetShape();
	assert(output_shape.GetNumDimensions() == 3);
	output_dim_[0] = output_shape[1];
	output_dim_[1] = output_shape[2];

	const auto& input_shape = model_.input_tensors[0].second.GetShape();
	input_dim_[0] = input_shape[1];
	input_dim_[1] = input_shape[2];

	eaif_info_h("YOLOV5 model type is %s\n", (m_type == Eaif8U) ? "uint8" : "float32");
	eaif_info_h("YOLOV5 model number of output is %dx%d\n", output_dim_[0], output_dim_[1]);
	SetPostProcessMethod(10);
	return 0;
}


int ArmnnYolov5Model::Detect(const void* img, std::vector<Detection>& detections, const ModelConfig& conf)
{
	WImage *pimg = (WImage*) img;

	armnn::utils::ArmnnModel* inference = &model_;

	// TBD for quantize model
	float* input_addr = (float*)inference->input_tensors[0].second.GetMemoryArea();
	float* output_addr = (float*)inference->output_tensors[0].second.GetMemoryArea();

	const Shape img_sz_net = {input_dim_[1], input_dim_[0]};
	const Shape img_sz_raw = {pimg->cols, pimg->rows};
	if (m_type == Eaif8U) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		COND_TIMER_FUNC(m_verbose, "Preprocess WImage", Imresize(*pimg, wimg, input_dim_[1], input_dim_[0]));
	} else {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif32FC3, input_addr);
		COND_TIMER_FUNC(m_verbose, "Preprocess WImage", ImresizeNorm(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data()));
	}

	//dump((const unsigned char *)input_addr, input_dim_[1]* input_dim_[0] * 3 * sizeof(float), 0);
	COND_TIMER_FUNC(m_verbose, "Inference Call", inference->Invoke());
	//dump((const unsigned char *)output_addr, output_dim_[1]* output_dim_[0] * sizeof(float), 1);


	// TBD for quantize model

	if (m_verbose) TIC(start);
	if (GetPostProcessMethod() == 10) {
		detect::PostProcessV1(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections);
	} else if (GetPostProcessMethod() == 11) {
		detect::PostProcessV1_1(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections);
	}
	if (m_verbose) TOC("PostProcess", start);
	return 0;
}


int ArmnnYolov4Model::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose) TIC(start);
	string model_loc = model_dir + "/" + model_path[0];
	armnn::utils::LoadModels(model_loc.c_str(), model_);

	m_type = model_.type;
	if (m_type) armnn::utils::GetQuantInfo(model_.model, model_info_);

	armnn::utils::InitModelIO(model_, model_.input_tensors, model_.output_tensors);
	if (m_verbose) TOC("Load model",start);

	const auto& output_shape = model_.output_tensors[0].second.GetShape();
	output_dim_[0] = output_shape[1];
	output_dim_[1] = output_shape[2];

	const auto& input_shape = model_.input_tensors[0].second.GetShape();
	input_dim_[0] = input_shape[1];
	input_dim_[1] = input_shape[2];

	SetPostProcessMethod(20);
	eaif_info_h("YOLOV4 model type is %s\n", (m_type == Eaif8U) ? "uint8" : "float32");
	eaif_info_h("YOLOV4 model number of output is %dx%d\n", output_dim_[0], output_dim_[1]);
	return 0;
}

int ArmnnYolov4Model::Detect(const void* img, std::vector<Detection>& detections, const ModelConfig& conf)
{
	/* Get input/output tensor index */
	WImage *pimg = (WImage*) img;

	/* Get input/output tensor index */
	armnn::utils::ArmnnModel* inference = &model_;
	// TBD for quantize model
	float* input_addr = (float*)inference->input_tensors[0].second.GetMemoryArea();
	float* output_addr = (float*)inference->output_tensors[0].second.GetMemoryArea();

	const Shape img_sz_net = {input_dim_[1], input_dim_[0]};
	const Shape img_sz_raw = {pimg->cols, pimg->rows};

	if (m_type == Eaif8U) {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif8UC3, input_addr);
		COND_TIMER_FUNC(m_verbose, "Preprocess WImage", Imresize(*pimg, wimg, input_dim_[1], input_dim_[0]));
	} else {
		WImage wimg(input_dim_[0], input_dim_[1], Eaif32FC3, input_addr);
		COND_TIMER_FUNC(m_verbose, "Preprocess WImage", ImresizeNorm(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data()));
	}


	COND_TIMER_FUNC(m_verbose, "Inference Call", inference->Invoke());
	// TBD for quantize model
	if (GetPostProcessMethod() == 20) {
		COND_TIMER_FUNC(m_verbose, "PostProcess",
		detect::PostProcessV2(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections));
	} else if (GetPostProcessMethod() == 21) {
		COND_TIMER_FUNC(m_verbose, "PostProcess",
		detect::PostProcessV2_1(output_addr, output_dim_[0], conf.num_classes,
						conf.conf_thresh[0], conf.iou_thresh,
		                img_sz_net, img_sz_raw, detections));
	}

	return 0;
}
#endif /* USE_TFarmnn */