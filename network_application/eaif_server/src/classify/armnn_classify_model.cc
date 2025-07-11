#ifdef USE_ARMNN
#include "eaif_utils.h"
#include "eaif_image.h"
#include "eaif_trc.h"

#include "armnn_classify_model.h"

using namespace std;
using namespace eaif::image;

static struct timespec start;

using ImresizePtr = void (*)(const WImage&, WImage&, int, int, const float[3], const float[3]);

int ArmnnClassifyModel::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (m_verbose) TIC(start);
	string model_loc = model_dir + "/" + model_path[0];
	armnn::utils::LoadModels(model_loc.c_str(), model_);

	m_type = model_.type;
	if (m_type == Eaif8U) armnn::utils::GetQuantInfo(model_.model, model_info_);

	armnn::utils::InitModelIO(model_, model_.input_tensors, model_.output_tensors);
	if (m_verbose) TOC("Load model", start);

	const auto& output_shape = model_.output_tensors[0].second.GetShape();

	assert(output_shape.GetNumDimensions() == 2);
	output_dim_[0] = output_shape[0];
	output_dim_[1] = output_shape[1];

	const auto& input_shape = model_.input_tensors[0].second.GetShape();
	input_dim_[0] = input_shape[1];
	input_dim_[1] = input_shape[2];
	input_dim_[2] = input_shape[3];

	eaif_info_h("classifier model input dim is %dx%dx%d type is %s\n",
		input_dim_[0], input_dim_[1], input_dim_[2], (m_type == Eaif8U) ? "uint8" : "float32");
	eaif_info_h("classifier model number of output is %dx%d\n", output_dim_[0], output_dim_[1]);
	return 0;
}

int ArmnnClassifyModel::Classify(const void *Wimg, Classification &result, const ModelConfig& conf)
{
	WImage *pimg = (WImage*) Wimg;

	armnn::utils::ArmnnModel* inference = &model_;

	// TBD for quantize model
	float* input_addr = (float*)inference->input_tensors[0].second.GetMemoryArea();
	float* output_addr = (float*)inference->output_tensors[0].second.GetMemoryArea();

	const Shape img_sz_net = {input_dim_[1], input_dim_[0]};
	const Shape img_sz_raw = {pimg->cols, pimg->rows};

	const int input_chn = input_dim_[2];

	int imgtype = eaif_GetImageType(m_type, input_chn);
	WImage wimg(input_dim_[0], input_dim_[1], imgtype, input_addr);

	if (m_type == Eaif8U) {
		Imresize(*pimg, wimg, input_dim_[1], input_dim_[0]);
	} else {
		ImresizePtr p_resize_function = (conf.resize_keep_ratio) ? ImresizeNormAspectRatio<WImage> : ImresizeNorm<WImage>;
		COND_TIMER_FUNC(m_verbose, "Preprocess WImage", p_resize_function(*pimg, wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data()));
	}

	//dump((const unsigned char *)input_addr, input_dim_[1]* input_dim_[0] * 3 * sizeof(float), 0);
	COND_TIMER_FUNC(m_verbose, "Inference Call", inference->Invoke());
	//dump((const unsigned char *)output_addr, output_dim_[1]* output_dim_[0] * sizeof(float), 1);


	// TBD for quantize model
	if (m_verbose)
		TIC(start);

	if (m_type == Eaif8U) {
		vector<float> output_buf(output_dim_[1],0.0);

		for (int j = 0; j < output_dim_[1]; j++)
			output_buf[j] = model_info_[0].Convertf(output_addr[j]);

		if (conf.activation_type == eaif::engine::Linear)
			classify::Sigmoid(output_buf.data(), conf.num_classes);

		classify::PostProcess(output_buf.data(), conf.num_classes, conf.conf_thresh[0], conf.topk, result);

	} else if (m_type == Eaif32F) {
	
		if (conf.activation_type == eaif::engine::Linear)
			classify::Sigmoid(output_addr, conf.num_classes);
		classify::PostProcess(output_addr, conf.num_classes, conf.conf_thresh[0], conf.topk, result);
	}

	if (m_verbose)
		TOC("PostProcessing", start);
	return 0;
}

int ArmnnClassifyModel::ClassifyObjList(const void *Wimg, const vector<ObjectAttr> &obj_list,
                                           vector<Classification> &results, const ModelConfig& conf)
{
	size_t i;
	WImage *pimg = (WImage*) Wimg;

	armnn::utils::ArmnnModel* inference = &model_;

	// TBD for quantize model
	uint8_t* input_addr = (uint8_t*)inference->input_tensors[0].second.GetMemoryArea();
	uint8_t* output_addr = (uint8_t*)inference->output_tensors[0].second.GetMemoryArea();

	const Shape img_sz_net = {input_dim_[1], input_dim_[0]};
	const Shape img_sz_raw = {pimg->cols, pimg->rows};
	const int input_chn = input_dim_[2];

	int imgtype = eaif_GetImageType(m_type, input_chn);
	WImage wimg(input_dim_[0], input_dim_[1], imgtype, input_addr);

	if (m_type == Eaif8U) {
		vector<float> output_buf(output_dim_[1],0.0);

		for (i = 0; i < obj_list.size(); ++i) {
			//Classification result;
			auto &box = obj_list[i].box; 
			if (m_verbose)
				TIC(start);
			ImcropResize(*pimg, box.sx, box.sy, box.ex, box.ey,
				wimg, input_dim_[1], input_dim_[0]);
			if (m_verbose)
				TOC("Preprocess WImage for obj", start);
			
			COND_TIMER_FUNC(m_verbose, "Inference Call", inference->Invoke());

			for (int j = 0; j < output_dim_[1]; j++)
				output_buf[j] = model_info_[0].Convertf(output_addr[j]);

			eaif_info_h("model ouput result: %f,%f,%f,%f\n", output_buf[0],output_buf[1],output_buf[2],output_buf[3]);

			if (conf.activation_type == eaif::engine::Linear)
				classify::Sigmoid(output_buf.data(), conf.num_classes);

			classify::PostProcess(output_buf.data(), conf.num_classes, conf.conf_thresh[0], conf.topk, result);
			result.idx = obj_list[i].idx;
			results.push_back(std::move(result));
		}
		
	} else {
		//ImresizePtr p_resize_function = (conf.resize_keep_ratio) ? ImresizeNormAspectRatio<WImage> : ImresizeNorm<WImage>;

		for (i = 0; i < obj_list.size(); ++i) {
			Classification result;
			auto &box = obj_list[i].box;

			if (m_verbose)
				TIC(start);
			ImcropResizeNorm(*pimg, box.sx, box.sy, box.ex, box.ey,
				wimg, input_dim_[1], input_dim_[0], conf.zeros.data(), conf.stds.data());
			if (m_verbose)
				TOC("Preprocess WImage for obj", start);

			COND_TIMER_FUNC(m_verbose, "Inference Call", inference->Invoke());
			
			eaif_info_h("model ouput result: %f,%f,%f,%f\n", output_addr[0],output_addr[1],output_addr[2],output_addr[3]);

			if (conf.activation_type == eaif::engine::Linear)
				classify::Sigmoid((float*)output_addr, conf.num_classes);
			classify::PostProcess((float*)output_addr, conf.num_classes, conf.conf_thresh[0], conf.topk, result);
			result.idx = obj_list[i].idx;
			results.push_back(std::move(result));
		}
		
	}
	return 0;
}

#endif /* USE_ARMNN */