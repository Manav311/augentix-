#ifdef USE_TFLITE

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include <assert.h>
#include <stdio.h>

#include "tensorflow/lite/interpreter.h"
#include "tensorflow/lite/kernels/register.h"
#include "tensorflow/lite/model.h"
#include "tensorflow/lite/optional_debug_tools.h"

#include "eaif_common.h"
#include "eaif_engine.h"
#include "eaif_image.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include "facereco_common.h"
#include "lite_mtcnn.h"

using namespace eaif::image;
using namespace std;

static struct timespec start;

#define LITE_MTCNN_DEBUG

int LiteMtcnn::LoadModels(const string &model_dir, const vector<string> &model_path)
{
	if (model_path.size() != 3) {
		eaif_warn("Number of model paths do not equal to 3!\n");
		return -1;
	}

    std::string pnet_fname = model_dir + "/" + model_path[0];
    std::string rnet_fname = model_dir + "/" + model_path[1];
    std::string onet_fname = model_dir + "/" + model_path[2];

    if (m_verbose) TIC(start);

	tflite::utils::LoadModels(pnet_fname.c_str(), pnet_model_, pnet_model_fb_);
	tflite::utils::LoadModels(rnet_fname.c_str(), rnet_model_, rnet_model_fb_);
	tflite::utils::LoadModels(onet_fname.c_str(), onet_model_, onet_model_fb_);

	pnet_model_->SetNumThreads(m_num_thread);
	rnet_model_->SetNumThreads(m_num_thread);
	onet_model_->SetNumThreads(m_num_thread);
	
	int input_idx_pnet = pnet_model_->inputs()[0];
	int input_idx_rnet = rnet_model_->inputs()[0];
	int input_idx_onet = onet_model_->inputs()[0];
	/* three model should have same type */
	type_ =  tflite::utils::GetDataType(pnet_model_.get(), input_idx_pnet);
	assert(type_ == tflite::utils::GetDataType(rnet_model_.get(), input_idx_rnet));
	assert(type_ == tflite::utils::GetDataType(onet_model_.get(), input_idx_onet));
	m_type = (EaifDataType)type_;

	if (type_ == Eaif8U || type_ == Eaif8S) {
		GetQuantInfo(pnet_model_, pnet_info_, pnet_index_, 2, 4, 0);
		GetQuantInfo(rnet_model_, rnet_info_, rnet_index_, 2, 4, 0);
		GetQuantInfo(onet_model_, onet_info_, onet_index_, 2, 4, 10);
	}

	if(pnet_model_== nullptr || rnet_model_ == nullptr || onet_model_ == nullptr) {
		eaif_err("Cannot init model\n");
		return -1;
	}

	if (rnet_model_->AllocateTensors() != kTfLiteOk || onet_model_->AllocateTensors() != kTfLiteOk) {
		eaif_err("Cannot allocate tensor\n!");
		return -1;
	}

	if (m_verbose) TOC("Load MTCNN", start);

	eaif_info_h("Mtcnn model type is %s\n", eaif_GetDTypeString(m_type));
	return 0;
}

void LiteMtcnn::GetQuantInfo(const std::unique_ptr<tflite::Interpreter> &model, QuantInfo *info, int net_idx, int prob_num, int coord_num, int landmark_num)
{
	std::vector<int> outputs = model->outputs();
	int num_output = (prob_num != 0) + (coord_num != 0) + (landmark_num != 0);
	if (outputs.size() != (uint32_t) num_output) {
		eaif_err("number of output is not correct!\n");
	}

#define SET_QUANT_INFO(net_idx, x, y) \
	{ \
		output_prob_idx_[net_idx] = x; \
		output_reg_idx_[net_idx] = y; \
		const TfLiteQuantizationParams *p = &model->tensor(outputs[0])->params; \
		info[0].set(p->zero_point, p->scale); \
		p = &model->tensor(outputs[1])->params; \
		info[1].set(p->zero_point, p->scale); \
	}

#define SET_QUANT_OINFO(net_idx, x, y, z) \
	{ \
		output_prob_idx_[net_idx] = x; \
		output_reg_idx_[net_idx] = y; \
		output_landmark_idx_ = z; \
		const TfLiteQuantizationParams *p = &model->tensor(outputs[0])->params; \
		info[0].set(p->zero_point, p->scale); \
		p = &model->tensor(outputs[1])->params; \
		info[1].set(p->zero_point, p->scale); \
		p = &model->tensor(outputs[2])->params; \
		info[2].set(p->zero_point, p->scale); \
	}

	auto dims = model->tensor(outputs[0])->dims;
	/* MTCNN output tensor may be permuted */
	if (net_idx == pnet_index_) { // pnet
		assert(dims->size == 4);
		if (dims->data[3] == prob_num) {
			SET_QUANT_INFO(pnet_index_, 0, 1);
		} else if (dims->data[3] == coord_num) {
			SET_QUANT_INFO(pnet_index_, 1, 0);
		}
	} else if (net_idx == rnet_index_) {
		assert(dims->size == 2);
		if (dims->data[1] == prob_num) {
			SET_QUANT_INFO(rnet_index_, 0, 1);
		} else if (dims->data[1] == coord_num) {
			SET_QUANT_INFO(rnet_index_, 1, 0)
		}
	} else if (net_idx == onet_index_) {
		assert(dims->size == 2);
		auto dims1 = model->tensor(outputs[1])->dims;
		if (dims->data[1] == prob_num  && dims1->data[1] == coord_num) {
			SET_QUANT_OINFO(onet_index_, 0, 1, 2);
		} else if (dims->data[1] == prob_num  && dims1->data[1] == landmark_num) {
			SET_QUANT_OINFO(onet_index_, 0, 2, 1);
		} else if (dims->data[1] == coord_num  && dims1->data[1] == prob_num) {
			SET_QUANT_OINFO(onet_index_, 1, 0, 2);
		} else if (dims->data[1] == coord_num  && dims1->data[1] == landmark_num) {
			SET_QUANT_OINFO(onet_index_, 1, 2, 0);
		} else if (dims->data[1] == landmark_num  && dims1->data[1] == prob_num) {
			SET_QUANT_OINFO(onet_index_, 2, 0, 1);
		} else if (dims->data[1] == landmark_num  && dims1->data[1] == coord_num) {
			SET_QUANT_OINFO(onet_index_, 2, 1, 0);
		} else {
			eaif_err("Unknown Onet output dimesion!\n");
			assert(0);
		}
	}

	eaif_info_l("out:%d float: %.9f, zero: %d\n", i, info[i].m_scale, info[i].m_zero);
}

int LiteMtcnn::RunNetwork(tflite::Interpreter *interpreter,
	const void* img, const Shape& size)
{

	/* Get input/output tensor index */
	const int input = interpreter->inputs()[0];

	std::vector<int> __dims{1,size.h,size.w,3};
    interpreter->ResizeInputTensor(input, __dims);
	int ret = interpreter->AllocateTensors();
	if (ret != kTfLiteOk) {
		eaif_err("Cannot allocate buffer!\n");
		return -1;
	}

	WImage *pimg = (WImage*)img;

	// FIXME: get channel number instead of fixed 3.
	if (type_ == Eaif8U) {
		uint8_t* input_addr = interpreter->typed_tensor<uint8_t>(input);
		WImage wimg(size.h, size.w, Eaif8UC3, input_addr);
		eaif::image::Imresize(*pimg, wimg, size.w, size.h);
	} else if (type_ == Eaif8S) {
		int8_t* input_addr = interpreter->typed_tensor<int8_t>(input);
		WImage wimg(size.h, size.w, Eaif8SC3, input_addr);
		eaif::image::Imresize(*pimg, wimg, size.w, size.h);
	} else if (type_ == Eaif32F)  {
		float* input_addr = interpreter->typed_tensor<float>(input);
		WImage wimg(size.h, size.w, Eaif32FC3, input_addr);
		eaif::image::ImresizeNorm(*pimg, wimg, size.w, size.h, zero_, scale_);
	} else {
		eaif_err("Datatype not supported!\n");
		assert(0);
	}
	// size [] => w, h
	//printf("%d %d => %d %d\n", size)
	eaif_check(interpreter->Invoke() == kTfLiteOk);

	return 0;
}

template<typename Timage>
static void CopyOnePatch(const Timage& img, FaceBox<float>& input_box, uint8_t* data_to, int height, int width)
{
	Timage resized(height, width, Eaif8UC3, data_to); // CV_8UC3 = 16

	int pad_top = std::abs(input_box.py0 - input_box.y0);
	int pad_left = std::abs(input_box.px0 - input_box.x0);
	int pad_bottom = std::abs(input_box.py1 - input_box.y1);
	int pad_right = std::abs(input_box.px1 - input_box.x1);

	ImcropPadResize(img, input_box.px0, input_box.py0, input_box.px1, input_box.py1,
		pad_top, pad_bottom, pad_left, pad_right, resized, width, height);
}

template<typename Timage>
static void CopyOnePatch(const Timage& img, FaceBox<float>& input_box, int8_t* data_to, int height, int width)
{
	Timage resized(height, width, Eaif8SC3, data_to); // CV_8UC3 = 16

	int pad_top = std::abs(input_box.py0 - input_box.y0);
	int pad_left = std::abs(input_box.px0 - input_box.x0);
	int pad_bottom = std::abs(input_box.py1 - input_box.y1);
	int pad_right = std::abs(input_box.px1 - input_box.x1);

	ImcropPadResize(img, input_box.px0, input_box.py0, input_box.px1, input_box.py1,
		pad_top, pad_bottom, pad_left, pad_right, resized, width, height);
}

template<typename Timage>
static void CopyOnePatch(const Timage& img, FaceBox<float>& input_box, float* data_to, int height, int width, float zeros[], float scales[])
{
	Timage resized(height, width, Eaif32FC3, data_to); // CV_32FC3

	int pad_top = std::abs(input_box.py0 - input_box.y0);
	int pad_left = std::abs(input_box.px0 - input_box.x0);
	int pad_bottom = std::abs(input_box.py1 - input_box.y1);
	int pad_right = std::abs(input_box.px1 - input_box.x1);

	ImcropPadResizeNorm(img, input_box.px0, input_box.py0, input_box.px1, input_box.py1,
		pad_top, pad_bottom, pad_left, pad_right, resized, width, height, zeros, scales);
}

int LiteMtcnn::RunNetwork(tflite::Interpreter *interpreter,
	const void* img, const Shape& size, FaceBox<float>& box)
{
	WImage *pimg = (WImage*)img;

	/* Get input/output tensor index */
	const int input = interpreter->inputs()[0];

	auto dims = interpreter->tensor(input)->dims;
	int wanted_height = dims->data[1];
	int wanted_width = dims->data[2];

	if (type_ == Eaif8U) {
		uint8_t* input_addr = interpreter->typed_tensor<uint8_t>(input);
		CopyOnePatch(*pimg, box, input_addr, wanted_height, wanted_width);
	} else if (type_ == Eaif8S) {
		int8_t* input_addr = interpreter->typed_tensor<int8_t>(input);
		CopyOnePatch(*pimg, box, input_addr, wanted_height, wanted_width);
	} else if (type_ == Eaif32F) {
		float* input_addr = interpreter->typed_tensor<float>(input);
		CopyOnePatch(*pimg, box, input_addr, wanted_height, wanted_width, zero_, scale_);
	}
	eaif_check(interpreter->Invoke() == kTfLiteOk);
	return 0;
}

int LiteMtcnn::RunPNet(const void *img, ScaleWindow<float>& win, std::vector<FaceBox<float>>& box_list)
{
	int scale_h = win.h;
	int scale_w = win.w;
	Shape win_wize(scale_w, scale_h);
	float scale = win.scale;

	tflite::Interpreter *interpreter = static_cast<tflite::Interpreter*>(pnet_model_.get());
	const std::vector<int> outputs = interpreter->outputs();

	int ret = RunNetwork(interpreter, img, win_wize);
	if (ret) {
		eaif_warn("Fail to run at [%d,%d]\n", win.h, win.w);
		return 0;
	}

	const int conf_index = output_prob_idx_[pnet_index_];
	const int reg_index = output_reg_idx_[pnet_index_];
	const TfLiteTensor *conf_tensor = interpreter->tensor(outputs[conf_index]);
	

	int feature_h= conf_tensor->dims->data[1];
	int feature_w= conf_tensor->dims->data[2];

	int conf_size = feature_h * feature_w * 2;

	std::vector<FaceBox<float> > candidate_boxes;

	if (type_ == Eaif32F) {
		const float *conf_data = interpreter->typed_tensor<float>(outputs[conf_index]);
		const float *reg_data =  interpreter->typed_tensor<float>(outputs[reg_index]);
		GenerateBoundingBox(conf_data, conf_size, (float*)reg_data, 
			scale, pnet_thresholdf_, feature_h, feature_w, candidate_boxes, false);
	} else if (type_ == Eaif8U) {
		const uint8_t *conf_data = interpreter->typed_tensor<uint8_t>(outputs[conf_index]);
		const uint8_t *reg_data = interpreter->typed_tensor<uint8_t>(outputs[reg_index]);
		const QuantInfo &info_conf = pnet_info_[conf_index];
		const QuantInfo &info_reg = pnet_info_[reg_index];
		GenerateBoundingBox(info_conf, conf_data, conf_size, info_reg, reg_data, 
			scale, pnet_thresholdf_, feature_h, feature_w, candidate_boxes, false);
	} else if (type_ == Eaif8S) {
		const int8_t *conf_data = interpreter->typed_tensor<int8_t>(outputs[conf_index]);
		const int8_t *reg_data = interpreter->typed_tensor<int8_t>(outputs[reg_index]);
		const QuantInfo &info_conf = pnet_info_[conf_index];
		const QuantInfo &info_reg = pnet_info_[reg_index];
		GenerateBoundingBox(info_conf, conf_data, conf_size, info_reg, reg_data, 
			scale, pnet_thresholdf_, feature_h, feature_w, candidate_boxes, false);
	}

#define VALIDATE 0

#if VALIDATE
	static int dcnt = 0;
	static int boxsize = 0;
#endif

	std::vector<FaceBox<float> > tmp_box = candidate_boxes;
	NmsBoxes(candidate_boxes, inter_nms_thf_[0], NMS_UNION, box_list);

#if VALIDATE

	if (dcnt == 0) {
		const std::vector<int> inputs = interpreter->inputs();
		const TfLiteTensor *input_tensor = interpreter->tensor(inputs[0]);
		const float *img_data = interpreter->typed_tensor<float>(inputs[0]);
		int input_h = input_tensor->dims->data[1], input_w = input_tensor->dims->data[2];
		string dim = to_string(feature_h) + "_" + to_string(feature_w);
		Dump((const uint8_t*)img_data, input_h * input_w * 3 * sizeof(float), "pnet_sc1_img.bin");
		Dump((const uint8_t*)conf_data, feature_h * feature_w * 2 * sizeof(float), ("pnet_sc1_conf_" + dim + "_2.bin").c_str() );
		Dump((const uint8_t*)reg_data, feature_h * feature_w * 4 * sizeof(float), ("pnet_sc1_reg_" + dim + "_4.bin").c_str() );
		Dump(tmp_box, ("pnet_sc1_box_" + to_string(candidate_boxes.size()) + "_9.bin").c_str() );
		Dump(box_list, ("pnet_sc1_nms0_5_" + to_string(box_list.size()) + "_9.bin").c_str() );
		dcnt++;
		boxsize += box_list.size();
		eaif_info_h("\nPNET inference information\n");
		eaif_info_h("===========================\n");
		eaif_info_h("sc1 input h :%d x w :%d x 3 (scale win(w:%d,h:%d))\n", input_h, input_w, scale_w, scale_h);
		eaif_info_h("conf shape h:%d x w :%d x 2\n", feature_h, feature_w);
		eaif_info_h("reg shape h :%d x w :%d x 4\n", feature_h, feature_w);
		eaif_info_h("box shape num:%d x 9\n", (int)candidate_boxes.size());
		eaif_info_h("nmsout shape num:%d x 9\n", (int)box_list.size());
		eaif_info_h("sum box list num:%d x 9\n\n", boxsize);
	} else {
		const std::vector<int> inputs = interpreter->inputs();
		const TfLiteTensor *input_tensor = interpreter->tensor(inputs[0]);
		int input_h = input_tensor->dims->data[1], input_w = input_tensor->dims->data[2];
		boxsize += box_list.size();
		eaif_info_h("\nPNET inference information\n");
		eaif_info_h("===========================\n");
		eaif_info_h("sc1 input h :%d x w :%d x 3 (scale win(w:%d,h:%d))\n", input_h, input_w, scale_w, scale_h);
		eaif_info_h("conf shape h:%d x w :%d x 2\n", feature_h, feature_w);
		eaif_info_h("reg shape h :%d x w :%d x 4\n", feature_h, feature_w);
		eaif_info_h("box shape num:%d x 9\n", (int)candidate_boxes.size());
		eaif_info_h("nmsout shape num:%d x 9\n", (int)box_list.size());
		eaif_info_h("sum box list num:%d x 9\n\n", boxsize);
	}
#endif
	return 0;
}


int LiteMtcnn::RunRNet(const void *img, std::vector<FaceBox<float>>& pnet_boxes, std::vector<FaceBox<float>>& output_boxes)
{
	//int batch = 1;
	int height = 24;
	int width = 24;
	Shape size(width, height);
	int num_box = pnet_boxes.size();

	tflite::Interpreter *interpreter = static_cast<tflite::Interpreter*>(rnet_model_.get());

	int ret = interpreter->AllocateTensors();
	eaif_check(ret == kTfLiteOk);

	const std::vector<int> outputs = interpreter->outputs();
	const float *conf_data = NULL;
	const float *reg_data = NULL;

	vector<float> conf_arr(2, 0);
	vector<float> reg_arr(4, 0);
	const QuantInfo *conf_info = NULL;
	const QuantInfo *reg_info = NULL;
	const uint8_t *conf_data_ptr = NULL;
	const uint8_t *reg_data_ptr = NULL;
	const int8_t *conf_data_iptr = NULL;
	const int8_t *reg_data_iptr = NULL;

	const int conf_index = output_prob_idx_[rnet_index_];
	const int reg_index = output_reg_idx_[rnet_index_];

	if (type_ == Eaif32F) {
		conf_data = interpreter->typed_tensor<float>(outputs[conf_index]);
		reg_data = interpreter->typed_tensor<float>(outputs[reg_index]);
	} else if (type_ == Eaif8U) {
		conf_data_ptr = interpreter->typed_tensor<uint8_t>(outputs[conf_index]);
		reg_data_ptr = interpreter->typed_tensor<uint8_t>(outputs[reg_index]);
		conf_info = &rnet_info_[conf_index];
		reg_info = &rnet_info_[reg_index];
		conf_data = conf_arr.data();
		reg_data = reg_arr.data();
	} else if (type_ == Eaif8S){
		conf_data_iptr = interpreter->typed_tensor<int8_t>(outputs[conf_index]);
		reg_data_iptr = interpreter->typed_tensor<int8_t>(outputs[reg_index]);
		conf_info = &rnet_info_[conf_index];
		reg_info = &rnet_info_[reg_index];
		conf_data = conf_arr.data();
		reg_data = reg_arr.data();
	} else {
		eaif_warn("Datatype Not support %d\n", type_);
		return -1;
	}

	eaif_info_l("rnet input: %d\n",interpreter->inputs()[0]);
	eaif_info_l("tensors size: %d\n",interpreter->tensors_size());
    eaif_info_l("nodes size: %d\n",interpreter->nodes_size());
    eaif_info_l("input(0) name: %s\n",interpreter->GetInputName(0));
    eaif_info_l("number of outputs: %d\n",outputs.size());
	eaif_info_l("[INFO] inputs[0] dims: [%d %d %d %d]\n", num_box, height, width, 3);

	for (int i = 0; i < num_box; ++i) {
		RunNetwork(interpreter, img, size, pnet_boxes[i]);

		if (type_ == Eaif8U) {
			conf_arr[1] = conf_info->Convertf(conf_data_ptr[1]);
			for (int j = 0; j < 4; j++)
				reg_arr[j] = reg_info->Convertf(reg_data_ptr[j]);
		} else if (type_ == Eaif8S) {
			conf_arr[1] = conf_info->Convertf(conf_data_iptr[1]);
			for (int j = 0; j < 4; j++)
				reg_arr[j] = reg_info->Convertf(reg_data_iptr[j]);
		}

		FaceBox<float> output_box;
		FaceBox<float>& input_box=pnet_boxes[i];

		if(conf_data[1]>rnet_thresholdf_) {
			output_box.x0=input_box.x0;
			output_box.y0=input_box.y0;
			output_box.x1=input_box.x1;
			output_box.y1=input_box.y1;
			output_box.score = conf_data[1];
			/*Note: check if regress's value is swaped here!!!*/
			output_box.regress[0]=reg_data[0];
			output_box.regress[1]=reg_data[1];
			output_box.regress[2]=reg_data[2];
			output_box.regress[3]=reg_data[3];
			output_boxes.push_back(output_box);
		}
		eaif_info_l("%d conf:%.2f [%.2f %.2f %.2f %.2f] reg:[%.2f %.2f %.2f %.2f]\n",
				i, conf_data[1], pnet_boxes[i].x0, pnet_boxes[i].y0, pnet_boxes[i].x1, pnet_boxes[i].y1,
				reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
	return 0;
}

int LiteMtcnn::RunONet(const void* img, std::vector<FaceBox<float> >& rnet_boxes, std::vector<FaceBox<float>>& output_boxes)
{
	int batch=rnet_boxes.size();
	//int channel = 3;
	int height = 48;
	int width = 48;
	Shape size(width, height);

	tflite::Interpreter* interpreter = static_cast<tflite::Interpreter*>(onet_model_.get());

	const std::vector<int> outputs = interpreter->outputs();
	const float* conf_data = NULL;
	const float* reg_data = NULL;
	const float* points_data= NULL;

	vector<float> conf_arr(2, 0);
	vector<float> reg_arr(4, 0);
	vector<float> points_arr(10, 0);
	const QuantInfo *conf_info = NULL;
	const QuantInfo *reg_info = NULL;
	const QuantInfo *points_info = NULL;

	const uint8_t *conf_data_ptr = NULL;
	const uint8_t *reg_data_ptr = NULL;
	const uint8_t *points_data_ptr = NULL;

	const int8_t *conf_data_iptr = NULL;
	const int8_t *reg_data_iptr = NULL;
	const int8_t *points_data_iptr = NULL;

	const int conf_index = output_prob_idx_[onet_index_];
	const int reg_index = output_reg_idx_[onet_index_];
	const int landmark_index = output_landmark_idx_;

	if (type_ == Eaif32F) {
		conf_data = interpreter->typed_tensor<float>(outputs[conf_index]);
		reg_data = interpreter->typed_tensor<float>(outputs[reg_index]);
		points_data= interpreter->typed_tensor<float>(outputs[landmark_index]);
	} else if (type_ == Eaif8U) {
		conf_data_ptr = interpreter->typed_tensor<uint8_t>(outputs[conf_index]);
		reg_data_ptr = interpreter->typed_tensor<uint8_t>(outputs[reg_index]);
		points_data_ptr = interpreter->typed_tensor<uint8_t>(outputs[landmark_index]);
		conf_info = &onet_info_[conf_index];
		reg_info = &onet_info_[reg_index];
		points_info = &onet_info_[landmark_index];
		conf_data = conf_arr.data();
		reg_data = reg_arr.data();
		points_data = points_arr.data();
	} else if (type_ == Eaif8S) {
		conf_data_iptr = interpreter->typed_tensor<int8_t>(outputs[conf_index]);
		reg_data_iptr = interpreter->typed_tensor<int8_t>(outputs[reg_index]);
		points_data_iptr = interpreter->typed_tensor<int8_t>(outputs[landmark_index]);
		conf_info = &onet_info_[conf_index];
		reg_info = &onet_info_[reg_index];
		points_info = &onet_info_[landmark_index];
		conf_data = conf_arr.data();
		reg_data = reg_arr.data();
		points_data = points_arr.data();
	}

	for(int i=0; i<batch; i++) {
		RunNetwork(interpreter, img, size, rnet_boxes[i]);

		if (type_ == Eaif8U) {
			conf_arr[1] = conf_info->Convertf(conf_data_ptr[1]);
			for (int j = 0; j < 4; j++)
				reg_arr[j] = reg_info->Convertf(reg_data_ptr[j]);
			for (int j = 0; j < 10; j++)
				points_arr[j] = points_info->Convertf(points_data_ptr[j]);
		} else if (type_ == Eaif8S) {
			conf_arr[1] = conf_info->Convertf(conf_data_iptr[1]);
			for (int j = 0; j < 4; j++)
				reg_arr[j] = reg_info->Convertf(reg_data_iptr[j]);
			for (int j = 0; j < 10; j++)
				points_arr[j] = points_info->Convertf(points_data_iptr[j]);
		}

		FaceBox<float> output_box;
		FaceBox<float>& input_box=rnet_boxes[i];
		if(conf_data[1] > onet_thresholdf_) {
			//FaceBox output_box;

			//FaceBox& input_box=rnet_boxes[i];

			output_box.x0=input_box.x0;
			output_box.y0=input_box.y0;
			output_box.x1=input_box.x1;
			output_box.y1=input_box.y1;

			output_box.score = conf_data[1];

			output_box.regress[0] = reg_data[0];
			output_box.regress[1] = reg_data[1];
			output_box.regress[2] = reg_data[2];
			output_box.regress[3] = reg_data[3];

			/*Note: switched x,y points value too..*/
			for (int j = 0; j<5; j++){
				output_box.landmark.x[j] = *(points_data + j);
				output_box.landmark.y[j] = *(points_data + j+5);
			}
			output_boxes.push_back(output_box);
		}
		eaif_info_l("%d conf:%.2f [%.4f %.4f %.4f %.4f] reg:[%.4f %.4f %.4f %.4f]\n",
		i, conf_data[1], rnet_boxes[i].x0, rnet_boxes[i].y0, rnet_boxes[i].x1, rnet_boxes[i].y1,
		reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
	return 0;
}


int LiteMtcnn::FaceDetect(const void *img, std::vector<FaceBox<float> >& face_list, const ModelConfig &conf)
{
	SetupConfig(conf);

	WImage *working_img = (WImage*) img;

	int img_h=working_img->rows;
	int img_w=working_img->cols;

	std::vector<ScaleWindow<float> > win_list;

	std::vector<FaceBox<float> > total_pnet_boxes;
	std::vector<FaceBox<float> > total_rnet_boxes;
	std::vector<FaceBox<float> > total_onet_boxes;

	CalPyramidList(img_h, img_w, min_size_, factor_, win_list);

#ifdef LITE_MTCNN_DEBUG
	for (auto win : win_list) {
		printf("[(%d %d %.2f)]", win.h, win.w, win.scale);
	}
	printf("\n");
#endif

	if (m_verbose) TIC(start);

	for(unsigned int i=0; i<win_list.size(); i++)
	{
		std::vector<FaceBox<float> > boxes;
		RunPNet(working_img, win_list[i], boxes);
		total_pnet_boxes.insert(total_pnet_boxes.end(), boxes.begin(), boxes.end());
	}

	eaif_info_l("stage one: %d total pre-process pnet_boxes\n", (int)total_pnet_boxes.size());

	std::vector<FaceBox<float> > pnet_boxes;

	ProcessBoxes(total_pnet_boxes, img_h, img_w, pnet_boxes, inter_nms_thf_[1]);
	eaif_info_l("stage one->: %d total post-process pnet_boxes\n", (int)pnet_boxes.size());

	if (m_verbose) TOC("Lite PNet Inference", start);

    if(pnet_boxes.size()==0)
          return 0;
	// RNet
    if (m_verbose) TIC(start);
	std::vector<FaceBox<float> > rnet_boxes;
	RunRNet(working_img, pnet_boxes, total_rnet_boxes);
	ProcessBoxes(total_rnet_boxes, img_h, img_w, rnet_boxes, inter_nms_thf_[2]);
	if (m_verbose) TOC("Lite RNet Inference", start);

	eaif_info_l("stage two: %d total_rnet_boxes \n", (int)rnet_boxes.size());
    if(rnet_boxes.size()==0)
          return 0;
    #if 0
    int k = 0;
	for (auto box : rnet_boxes) {
		eaif_info_l("[%d s:%d (%d %d %d %d) reg:(%d %d %d %d)]\n",
			k++, box.score, box.x0, box.y0, box.x1, box.y1, box.regress[0], box.regress[1],
			box.regress[2], box.regress[3]);
	}
	#endif
	//ONet
	if (m_verbose) TIC(start);
	RunONet(working_img, rnet_boxes, total_onet_boxes);
	for(unsigned int i=0; i<total_onet_boxes.size(); i++) {

		FaceBox<float>& box = total_onet_boxes[i];

		int h=box.x1-box.x0+1;
		int w=box.y1-box.y0+1;

		for(int j=0;j<5;j++) {
			box.landmark.x[j] = box.x0+w * box.landmark.x[j] - 1;
			box.landmark.y[j] = box.y0+h * box.landmark.y[j] - 1;
		}
	}

	//Get Final Result
	if (m_verbose) TOC("Lite ONet Inference", start);

	RegressBoxes(total_onet_boxes);
	NmsBoxes(total_onet_boxes, nms_thf_, NMS_MIN, face_list);
	eaif_info_h("Stage one: %d=>%d, two: %d three: %d total_onet_boxes \n", (int)total_pnet_boxes.size(), (int)pnet_boxes.size(), (int)rnet_boxes.size(), (int)face_list.size());
	return EAIF_SUCCESS;
}


#endif /* !USE_TFLITE */
