#ifdef USE_ARMNN

#include <fstream>
#include <iostream>
#include <utility>
#include <vector>

#include <assert.h>
#include <stdio.h>
#include <stdio.h>

#include "eaif_common.h"
#include "eaif_image.h"
#include "eaif_trc.h"
#include "eaif_utils.h"

#include <boost/format.hpp>
#include <boost/variant.hpp>
#include <boost/program_options.hpp>

#include <armnn/Tensor.hpp>
#include <armnn/TypesUtils.hpp>
#include <armnn/BackendId.hpp>
#include <armnn/IRuntime.hpp>
#include <armnn/BackendRegistry.hpp>
#include <armnnTfLiteParser/ITfLiteParser.hpp>

#include "armnn_mtcnn.h"
#include "facereco_common.h"

using namespace eaif::image;
//#define LITEDEB
static struct timespec start;

int ArmnnMtcnn::LoadModels(const std::string &model_dir, const std::vector<std::string> &model_path)
{
    std::string pnet_fname = model_dir + "/" + model_path[0];
    std::string rnet_fname = model_dir + "/" + model_path[1];
    std::string onet_fname = model_dir + "/" + model_path[2];

    if (m_verbose) TIC(start);

	armnn::utils::LoadModels(pnet_fname.c_str(), pnet_model_);
	armnn::utils::LoadModels(rnet_fname.c_str(), rnet_model_);
	armnn::utils::LoadModels(onet_fname.c_str(), onet_model_);

	if (pnet_model_.type == Eaif8U) armnn::utils::GetQuantInfo(pnet_model_.model, pnet_info_);
	if (rnet_model_.type == Eaif8U) armnn::utils::GetQuantInfo(rnet_model_.model, rnet_info_);
	if (onet_model_.type == Eaif8U) armnn::utils::GetQuantInfo(onet_model_.model, onet_info_);

	assert(pnet_model_.type == rnet_model_.type);
	assert(rnet_model_.type == onet_model_.type);
	type_ = pnet_model_.type;
	m_type = (EaifDataType)type_;

	armnn::utils::InitModelIO(pnet_model_, pnet_model_.input_tensors, pnet_model_.output_tensors);
	armnn::utils::InitModelIO(rnet_model_, rnet_model_.input_tensors, rnet_model_.output_tensors);
	armnn::utils::InitModelIO(onet_model_, onet_model_.input_tensors, onet_model_.output_tensors);

	if (m_verbose) TOC("Load MTCNN", start);

	return 0;
}

/*@brief copy image patch to network input addr
 *@[in] img cv::image 
 *@[in] imsize image w,h, c
 *@[in] isize network input size h, w
 *@[in] offset offset index of image to be copied to dst
 *@[out] dst address of network input
**/
template<typename T>
static void CopyOnePatch(const T* src, T* dst, const Shape& imsize, const Shape& network_input_size, const size_t offset)
{
	const int imc = 3;
	const int dst_increment = network_input_size.w * imc * sizeof(T);
	const int src_increment = imsize.w * imc * sizeof(T);
	const int copy_size = dst_increment;
	size_t src_offset = offset * sizeof(T);
	size_t dst_offset = 0;
	uint8_t* src_addr = (uint8_t*) src;
	uint8_t* dst_addr = (uint8_t*) dst;
	for (int h = 0; h < network_input_size.h; ++h) {
		memcpy(&dst_addr[dst_offset], &src_addr[src_offset], copy_size);
		src_offset += src_increment;
		dst_offset += dst_increment;
	}
}

/*@brief copy network output to output vectors addr
 *@[in] outputs network output
 *@[in] iw network output width
 *@[in] r row index
 *@[in] c column index
 *@[out] dst_vector dst vector to store network output
**/
template<typename T>
static void CopyNetworkOutput(const armnn::OutputTensors& outputs, std::vector<std::vector<T>>& dst_vector, int iw, int r, int c)
{
	const int output_num = outputs.size();
	for (int i = 0; i < output_num; ++i) {
		const armnn::Tensor& tensor = outputs[i].second;
		uint8_t* src = (uint8_t*)tensor.GetMemoryArea();
		uint8_t* dst = (uint8_t*)dst_vector[i].data();
		int last_dim = tensor.GetNumBytes();
		memcpy(&dst[r * iw * last_dim + c * last_dim], src, last_dim);
	}
}

/* for pnet inference */
// [in] inference model
// [in] img image file raw
// [in] size image size [w, h]
// [in] osize output dimension [ h, w]
// [in] stride number of stride 2
// [out] outputs output vector (num output)
template<>
int ArmnnMtcnn::RunNetwork(armnn::utils::ArmnnModel *inference,
	const void* img, const Shape& imsize, const Shape& outsize, std::vector<std::vector<uint8_t>>& outputs, int stride)
{

	const WImage* pimg = (const WImage*)img;
	WImage wimg;

	const int channel = 3;

	// pnet output calc
	auto input_shape = inference->input_tensors[0].second.GetShape();
	Shape network_input_shape((int)input_shape[2], (int)input_shape[1]); // {network input h,w}

	const int row_offset_mul = imsize.w * channel * stride;
	const int col_offset_mul = channel * stride;


	Imresize(*pimg, wimg, imsize.w, imsize.h);
	// runNetwork
	uint8_t* input_addr = boost::get<std::vector<uint8_t>>(inference->input_data[0]).data();

	for (int r = 0; r < outsize.h; ++r) {
		const size_t row_offset = r * row_offset_mul;
		for (int c = 0; c < outsize.w; ++c) {
			const size_t offset = row_offset + c * col_offset_mul;
			CopyOnePatch((const uint8_t*)wimg.data, input_addr, imsize, network_input_shape, offset);
			inference->Invoke();
			CopyNetworkOutput(inference->output_tensors, outputs, outsize.w, r, c);
		}
	}

	return 0;
}

template<>
int ArmnnMtcnn::RunNetwork(armnn::utils::ArmnnModel *inference,
	const void* img, const Shape& imsize, const Shape& outsize, std::vector<std::vector<float>>& outputs, int stride)
{

	const WImage* pimg = (const WImage*)img;
	WImage wimg;

	const int channel = 3;

	// pnet output calc
	auto input_shape = inference->input_tensors[0].second.GetShape();
	Shape network_input_shape((int)input_shape[2], (int)input_shape[1]); // {network input h,w}

	const int row_offset_mul = imsize.w * channel * stride;
	const int col_offset_mul = channel * stride;

	//float zeros[] = {zero_[0], zero_[1], zero_[2]};
	//float scales[] = {scale_[0], scale_[1], scale_[2]};
	ImresizeNorm(*pimg, wimg, imsize.w, imsize.h, zero_, scale_);
	// runNetwork
	float* input_addr = boost::get<std::vector<float>>(inference->input_dataf[0]).data();

	for (int r = 0; r < outsize.h; ++r) {
		const size_t row_offset = r * row_offset_mul;
		for (int c = 0; c < outsize.w; ++c) {
			const size_t offset = row_offset + c * col_offset_mul;
			CopyOnePatch((const float*)wimg.data, input_addr, imsize, network_input_shape, offset);
			inference->Invoke();
			CopyNetworkOutput(inference->output_tensors, outputs, outsize.w, r, c);
		}
	}
	return 0;
}

template<typename Timage, typename T>
static void CopyOnePatch(const Timage& img, FaceBox<T>& input_box, uint8_t* data_to, int height, int width)
{
	Timage resized(height, width, Eaif8UC3, data_to); // CV_8UC3
	Timage chop_img = eaif::image::Imcrop(img, input_box.px0, input_box.py0,
		input_box.px1, input_box.py1);

	int pad_top = std::abs(input_box.py0 - input_box.y0);
	int pad_left = std::abs(input_box.px0 - input_box.x0);
	int pad_bottom = std::abs(input_box.py1 - input_box.y1);
	int pad_right = std::abs(input_box.px1 - input_box.x1);

	eaif::image::ImcopyMakeBorder(chop_img, chop_img, pad_top, pad_bottom, pad_left, pad_right);
	eaif::image::Imresize(chop_img, resized, width, height);
}

template<typename Timage, typename T>
static void CopyOnePatch(const Timage& img, FaceBox<T>& input_box, float* data_to, int height, int width, float zeros[], float scales[])
{
	Timage resized(height, width, Eaif32FC3, data_to); // CV_8UC3
	Timage chop_img = eaif::image::Imcrop(img, input_box.px0, input_box.py0,
		input_box.px1, input_box.py1);

	int pad_top = std::abs(input_box.py0 - input_box.y0);
	int pad_left = std::abs(input_box.px0 - input_box.x0);
	int pad_bottom = std::abs(input_box.py1 - input_box.y1);
	int pad_right = std::abs(input_box.px1 - input_box.x1);

	eaif::image::ImcopyMakeBorder(chop_img, chop_img, pad_top, pad_bottom, pad_left, pad_right);
	eaif::image::ImresizeNorm(chop_img, resized, width, height, zeros, scales);
}

// for r/o net
template<typename T>
int ArmnnMtcnn::RunNetwork(armnn::utils::ArmnnModel *inference,
	const void* img, const Shape& size, FaceBox<T>& box)
{
	const WImage* pimg = (const WImage*)img;

	/* Get input/output tensor index */
	auto dims = inference->input_tensors[0].second.GetShape();
	int wanted_height = dims[1];
	int wanted_width = dims[2];
	if (type_ == Eaif8U) {
		uint8_t* input_addr = boost::get<std::vector<uint8_t>>(inference->input_data[0]).data();
		CopyOnePatch(*pimg, box, input_addr, wanted_height, wanted_width);
	} else {
		float* input_addr = boost::get<std::vector<float>>(inference->input_dataf[0]).data();
		CopyOnePatch(*pimg, box, input_addr, wanted_height, wanted_width, zero_, scale_);
	}
	inference->Invoke();
	return 0;
}

template<>
void ArmnnMtcnn::RunPNet(const void *img, ScaleWindow<int>& win, std::vector<FaceBox<int>>& box_list)
{
	int scale_h = EAIF_FR_DOWN(win.h);
	int scale_w = EAIF_FR_DOWN(win.w);
	Shape win_size(scale_w, scale_h);
	int scale=win.scale;
	int stride = 2;

	armnn::utils::ArmnnModel* inference = &pnet_model_;

	Shape output_dims(win_size.w / 2 - 5, win_size.h / 2 -5); // h, w
	int size_element = output_dims.w * output_dims.h;
	int output_num = inference->output_tensors.size();

	// Setup output size //
	std::vector<std::vector<uint8_t>> data_vector(output_num);
	for (int i = 0; i < output_num; ++i) {
		auto& tensor = inference->output_tensors[i].second;
		data_vector[i].resize(size_element * tensor.GetNumElements());
	}
	// size[w, h] output_dim[h, w]
	RunNetwork(inference, img, win_size, output_dims, data_vector, stride);

	//runNetwork(pnet_model_, img, size);

	const uint8_t *conf_data = data_vector[0].data();
	const uint8_t *reg_data = data_vector[1].data();

	int feature_h= output_dims.h;
	int feature_w= output_dims.w;

	int conf_size = feature_h * feature_w * 2;

	std::vector<FaceBox<int>> candidate_boxes;
#ifdef LITEDEB
	extern int g_m_row;
	armnn::utils::print_output(output_dims, data_vector, g_m_row);
	eaif_ele(conf_size, "d");
	eaif_ele(scale, "d");
	eaif_ele(pnet_threshold_, "d");
	eaif_ele(feature_h, "d");
	eaif_ele(feature_w, "d");
	eaif_ele(conf_data, "p");
	eaif_ele(reg_data, "p");
	printf("\n");
#endif
	GenerateBoundingBox(conf_data, conf_size, reg_data, 
			scale, pnet_threshold_, feature_h, feature_w, candidate_boxes, false);

	NmsBoxes(candidate_boxes, inter_nms_th_[0], NMS_UNION, box_list);
}

template<>
void ArmnnMtcnn::RunRNet(const void *img, std::vector<FaceBox<int>>& pnet_boxes, std::vector<FaceBox<int>>& output_boxes)
{
	//int batch = 1;

	const WImage *pimg = (const WImage *)img;

	int height = 24;
	int width = 24;
	Shape network_input_shape(width, height);
	int num_box = pnet_boxes.size();

	armnn::utils::ArmnnModel* inference = &rnet_model_;

	auto& output_tensors = inference->output_tensors;
	auto* conf_data = (const uint8_t*)output_tensors[0].second.GetMemoryArea();
	auto* reg_data = (const uint8_t*)output_tensors[1].second.GetMemoryArea();

	eaif_info_l("rnet input: %s\n","");
    eaif_info_l("input(0) name: %s\n",inference->model->GetSubgraphInputTensorNames(0)[0].c_str());
    eaif_info_l("number of outputs: %d\n",inference->output_tensors.size());
	eaif_info_l("[INFO] inputs[0] dims: [%d %d %d %d]\n", num_box, height, width, 3);

	for (int i = 0; i < num_box; ++i) {
		RunNetwork(inference, pimg, network_input_shape, pnet_boxes[i]);

		FaceBox<int> output_box;
		FaceBox<int>& input_box=pnet_boxes[i];

		if(conf_data[1]>rnet_threshold_) {
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
		eaif_info_l("%d conf:%d [%d %d %d %d] reg:[%d %d %d %d]\n",
				i, conf_data[1], pnet_boxes[i].x0, pnet_boxes[i].y0, pnet_boxes[i].x1, pnet_boxes[i].y1,
				reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
}

template<>
void ArmnnMtcnn::RunONet(const void* img, std::vector<FaceBox<int>>& rnet_boxes, std::vector<FaceBox<int>>& output_boxes)
{
	int batch=rnet_boxes.size();
	//int channel = 3;
	int height = 48;
	int width = 48;
	Shape network_input_shape(width, height);

	armnn::utils::ArmnnModel* inference = &onet_model_;

	const auto& output_tensors = inference->output_tensors;
	auto* conf_data = (const uint8_t *)output_tensors[output_prob_idx_].second.GetMemoryArea();
	auto* reg_data = (const uint8_t *)output_tensors[output_reg_idx_].second.GetMemoryArea();
	auto* points_data= (const uint8_t *)output_tensors[output_landmark_idx_].second.GetMemoryArea();

	for(int i=0; i<batch; i++) {

		RunNetwork(inference, img, network_input_shape, rnet_boxes[i]);
			FaceBox<int> output_box;

			FaceBox<int>& input_box=rnet_boxes[i];
		if(conf_data[1]>onet_threshold_) {
			//FaceBox output_box;

			//FaceBox& input_box=rnet_boxes[i];

			output_box.x0=input_box.x0;
			output_box.y0=input_box.y0;
			output_box.x1=input_box.x1;
			output_box.y1=input_box.y1;

			output_box.score = conf_data[1];

			output_box.regress[0]=reg_data[0];
			output_box.regress[1]=reg_data[1];
			output_box.regress[2]=reg_data[2];
			output_box.regress[3]=reg_data[3];

			/*Note: switched x,y points value too..*/
			for (int j = 0; j<5; j++){
				output_box.landmark.x[j] = *(points_data + j);
				output_box.landmark.y[j] = *(points_data + j+5);
			}
			output_boxes.push_back(output_box);
		}
		eaif_info_l("%d conf:%d [%d %d %d %d] reg:[%d %d %d %d]\n",
		i, conf_data[1], rnet_boxes[i].x0, rnet_boxes[i].y0, rnet_boxes[i].x1, rnet_boxes[i].y1,
		reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
}

int ArmnnMtcnn::FaceDetect(const void* img, std::vector<FaceBox<int>>& face_list, const ModelConfig &conf)
{
	SetupConfig(conf);

	WImage *working_img = (WImage*) img;

	int img_h=working_img->rows;
	int img_w=working_img->cols;

	std::vector<ScaleWindow<int>> win_list;

	std::vector<FaceBox<int>> total_pnet_boxes;
	std::vector<FaceBox<int>> total_rnet_boxes;
	std::vector<FaceBox<int>> total_onet_boxes;

	CalPyramidList(img_h,img_w,min_size_,factor_,win_list);

	#if 0
	for (auto win : win_list) {
		eaif_info_l("[(%d %d %d):(%.2f %.2f %.2f)]\n", win.h, win.w, win.scale,
			EAIF_FR_BIT_2_FLOAT(win.h), EAIF_FR_BIT_2_FLOAT(win.w), EAIF_FR_BIT_2_FLOAT(win.scale));
	}
	#endif

	for(unsigned int i=0; i<win_list.size(); i++)
	{
		std::vector<FaceBox<int> > boxes;

		RunPNet(working_img, win_list[i], boxes);

		total_pnet_boxes.insert(total_pnet_boxes.end(), boxes.begin(), boxes.end());
	}
	eaif_info_l("stage one: %d total_pnet_boxes \n", total_pnet_boxes.size());

	std::vector<FaceBox<int> > pnet_boxes;

	ProcessBoxes(total_pnet_boxes, img_h, img_w, pnet_boxes, pnet_info_[1], inter_nms_th_[1]);

    if(pnet_boxes.size()==0)
          return 0;

	// RNet
	std::vector<FaceBox<int>> rnet_boxes;

	RunRNet(working_img, pnet_boxes, total_rnet_boxes);

	ProcessBoxes(total_rnet_boxes, img_h, img_w, rnet_boxes, rnet_info_[1],inter_nms_th_[2]);

	eaif_info_l("stage two: %d total_pnet_boxes \n", rnet_boxes.size());
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
	RunONet(working_img, rnet_boxes, total_onet_boxes);

	//calculate the landmark
	const QuantInfo *info = &onet_info_[2];
	for(unsigned int i=0;i<total_onet_boxes.size();i++) {

		FaceBox<int>& box = total_onet_boxes[i];

		int h = box.x1-box.x0+1;
		int w = box.y1-box.y0+1;

		for(int j = 0; j<5; j++) {
			box.landmark.x[j]= box.x0+
				EAIF_FR_DOWN(w * info->Convert(box.landmark.x[j]))-1;
			box.landmark.y[j]= box.y0+
			    EAIF_FR_DOWN(h * info->Convert(box.landmark.y[j]))-1;
		}

	}

	//Get Final Result
	RegressBoxes(total_onet_boxes, onet_info_[output_reg_idx_]);
	NmsBoxes(total_onet_boxes, nms_th_, NMS_MIN, face_list);
	eaif_info_l("stage three: %d total_pnet_boxes \n", total_onet_boxes.size());
	return 0;
}

template<>
void ArmnnMtcnn::RunPNet(const void *img, ScaleWindow<float>& win, std::vector<FaceBox<float> >& box_list)
{
	int scale_h = win.h;
	int scale_w = win.w;
	Shape win_size(scale_w, scale_h);
	float scale = win.scale;
	int stride = 2;

	armnn::utils::ArmnnModel* inference = &pnet_model_;

	Shape output_dims(win_size.w / 2 - 5, win_size.h / 2 -5); // h, w
	int size_element = output_dims.w * output_dims.h;
	int output_num = inference->output_tensors.size();

	// Setup output size //
	std::vector<std::vector<float>> data_vector(output_num);
	for (int i = 0; i < output_num; ++i) {
		auto& tensor = inference->output_tensors[i].second;
		data_vector[i].resize(size_element * tensor.GetNumElements());
	}
	// size[w, h] output_dim[h, w]
	RunNetwork(inference, img, win_size, output_dims, data_vector, stride);
	//runNetwork(pnet_model_, img, size);

	const float *conf_data = data_vector[0].data();
	const float *reg_data = data_vector[1].data();

	int feature_h= output_dims.h;
	int feature_w= output_dims.w;

	int conf_size = feature_h * feature_w * 2;

	std::vector<FaceBox<float> > candidate_boxes;
#ifdef ARMNNDEB
	extern int g_m_row;
	armnn::utils::PrintOutput(output_dims, data_vector, g_m_row);
	eaif_ele(conf_size, "d");
	eaif_ele(scale, ".2f");
	eaif_ele(pnet_thresholdf_, ".2f");
	eaif_ele(feature_h, "d");
	eaif_ele(feature_w, "d");
	eaif_ele(conf_data, ".2f");
	eaif_ele(reg_data, ".2f");
	printf("\n");
#endif
	GenerateBoundingBox(conf_data, conf_size, reg_data, 
			scale, pnet_thresholdf_, feature_h, feature_w, candidate_boxes, false);

	NmsBoxes(candidate_boxes, inter_nms_thf_[0], NMS_UNION, box_list);
}

template<>
void ArmnnMtcnn::RunRNet(const void *img, std::vector<FaceBox<float>>& pnet_boxes, std::vector<FaceBox<float>>& output_boxes)
{
	//int batch = 1;

	const WImage *pimg = (const WImage *)img;

	int height = 24;
	int width = 24;
	Shape network_input_size(width, height);
	int num_box = pnet_boxes.size();

	armnn::utils::ArmnnModel* inference = &rnet_model_;

	auto& output_tensors = inference->output_tensors;
	auto* conf_data = (const float*)output_tensors[0].second.GetMemoryArea();
	auto* reg_data = (const float*)output_tensors[1].second.GetMemoryArea();

	eaif_info_l("rnet input: %s\n","");
    eaif_info_l("input(0) name: %s\n",inference->model->GetSubgraphInputTensorNames(0)[0].c_str());
    eaif_info_l("number of outputs: %d\n",inference->output_tensors.size());
	eaif_info_l("[INFO] inputs[0] dims: [%d %d %d %d]\n", num_box, height, width, 3);

	for (int i = 0; i < num_box; ++i) {
		RunNetwork(inference, pimg, network_input_size, pnet_boxes[i]);

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
}

template<>
void ArmnnMtcnn::RunONet(const void* img, std::vector<FaceBox<float>>& rnet_boxes, std::vector<FaceBox<float>>& output_boxes)
{
	int batch=rnet_boxes.size();
	//int channel = 3;
	int height = 48;
	int width = 48;
	Shape network_input_shape(width, height);

	armnn::utils::ArmnnModel* inference = &onet_model_;

	const auto& output_tensors = inference->output_tensors;
	auto* conf_data = (const float *)output_tensors[output_prob_idx_].second.GetMemoryArea();
	auto* reg_data = (const float *)output_tensors[output_reg_idx_].second.GetMemoryArea();
	auto* points_data= (const float *)output_tensors[output_landmark_idx_].second.GetMemoryArea();

	for(int i=0; i<batch; i++) {

		RunNetwork(inference, img, network_input_shape, rnet_boxes[i]);
		FaceBox<float> output_box;
		FaceBox<float>& input_box=rnet_boxes[i];

		if(conf_data[1]>onet_thresholdf_) {
			//FaceBox output_box;

			//FaceBox& input_box=rnet_boxes[i];

			output_box.x0=input_box.x0;
			output_box.y0=input_box.y0;
			output_box.x1=input_box.x1;
			output_box.y1=input_box.y1;

			output_box.score = conf_data[1];

			output_box.regress[0]=reg_data[0];
			output_box.regress[1]=reg_data[1];
			output_box.regress[2]=reg_data[2];
			output_box.regress[3]=reg_data[3];

			/*Note: switched x,y points value too..*/
			for (int j = 0; j<5; j++){
				output_box.landmark.x[j] = *(points_data + j);
				output_box.landmark.y[j] = *(points_data + j+5);
			}
			output_boxes.push_back(output_box);
		}
		eaif_info_l("%d conf:%.2f [%.2f %.2f %.2f %.2f] reg:[%.2f %.2f %.2f %.2f]\n",
		i, conf_data[1], rnet_boxes[i].x0, rnet_boxes[i].y0, rnet_boxes[i].x1, rnet_boxes[i].y1,
		reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
}

int ArmnnMtcnn::FaceDetect(const void* img, std::vector<FaceBox<float>>& face_list, const ModelConfig &conf)
{
	SetupConfig(conf);

	WImage *working_img = (WImage*) img;

	int img_h=working_img->rows;
	int img_w=working_img->cols;

	std::vector<ScaleWindow<float> > win_list;

	std::vector<FaceBox<float> > total_pnet_boxes;
	std::vector<FaceBox<float> > total_rnet_boxes;
	std::vector<FaceBox<float> > total_onet_boxes;

	CalPyramidList(img_h,img_w,min_size_,factor_,win_list);

	#if 0
	for (auto win : win_list) {
		eaif_info_l("[(%d %d %d):(%.2f %.2f %.2f)]\n", win.h, win.w, win.scale,
			EAIF_FR_BIT_2_FLOAT(win.h), EAIF_FR_BIT_2_FLOAT(win.w), EAIF_FR_BIT_2_FLOAT(win.scale));
	}
	#endif

	for(unsigned int i=0;i<win_list.size();i++)
	{
		std::vector<FaceBox<float>>boxes;

		RunPNet(working_img,win_list[i],boxes);

		total_pnet_boxes.insert(total_pnet_boxes.end(),boxes.begin(),boxes.end());
	}
	eaif_info_l("stage one: %d total_pnet_boxes \n", total_pnet_boxes.size());

	std::vector<FaceBox<float> > pnet_boxes;

	ProcessBoxes(total_pnet_boxes,img_h,img_w,pnet_boxes,inter_nms_thf_[1]);

    if(pnet_boxes.size()==0)
          return 0;

	// RNet
	std::vector<FaceBox<float> > rnet_boxes;

	RunRNet(working_img, pnet_boxes, total_rnet_boxes);

	ProcessBoxes(total_rnet_boxes,img_h,img_w,rnet_boxes,inter_nms_thf_[2]);

	eaif_info_l("stage two: %d total_rnet_boxes \n", rnet_boxes.size());
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
	RunONet(working_img, rnet_boxes, total_onet_boxes);

	//calculate the landmark
	for(unsigned int i=0; i<total_onet_boxes.size(); i++) {

		FaceBox<float>& box=total_onet_boxes[i];

		int h=box.x1-box.x0+1;
		int w=box.y1-box.y0+1;

		for(int j=0;j<5;j++) {
			box.landmark.x[j]= box.x0+w * box.landmark.x[j]-1;
			box.landmark.y[j]= box.y0+h * box.landmark.y[j]-1;
		}

	}

	//Get Final Result
	RegressBoxes(total_onet_boxes);
	NmsBoxes(total_onet_boxes, nms_thf_, NMS_MIN, face_list);
	eaif_info_h("Stage three: %d total_onet_boxes \n", total_onet_boxes.size());
	return 0;
}

#if 0
static mtcnn* armnn_creator(void)
{
	return new armnn_mtcnn();
}

REGISTER_MTCNN_CREATOR(tensorflow, lite_creator);
#endif

#endif /* USE_ARMNN */