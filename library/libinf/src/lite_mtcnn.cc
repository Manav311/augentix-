#ifndef USE_NCNN
#include <libgen.h>
#include <cstdint>
#include <cstring>

#ifndef USE_MICROLITE
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#include "tensorflow/lite/interpreter.h"
#pragma GCC diagnostic pop
#endif

#include "inf_types.h"
#include "inf_image.h"

#include "inf_log.h"
#include "inf_utils.h"
#include "inf_utils_lite.h"

#include "lite_mtcnn.h"
#include "inf_adapter.h"

#define DYNAMIC_MIN_FACE (TRUE)

void LiteMtcnn::SetModelThreads(int nthreads)
{
	m_config->num_threads = nthreads;
	m_num_thread = nthreads;
	inf_tf_adapter::setNumThreads(*m_pnet_model, nthreads);
	inf_tf_adapter::setNumThreads(*m_rnet_model, nthreads);
	inf_tf_adapter::setNumThreads(*m_onet_model, nthreads);
}

LiteMtcnn::~LiteMtcnn()
{
	ReleaseConfig(m_config);
	delete m_config;
	m_config = nullptr;
}

int LiteMtcnn::LoadModels(const char* model_dir, const InfStrList* model_paths)
{
	if (model_paths->size != 3) {
		inf_log_warn("Number of model paths do not equal to 3!");
		return -1;
	}
	char pnet_fname[256] = {};
	char rnet_fname[256] = {};
	char onet_fname[256] = {};

	snprintf(pnet_fname, 255, "%s/%s", model_dir, model_paths->data[0]);
	snprintf(rnet_fname, 255, "%s/%s", model_dir, model_paths->data[1]);
	snprintf(onet_fname, 255, "%s/%s", model_dir, model_paths->data[2]);

	if (m_verbose) TIC(start);

	inf_tf_adapter::LiteMtcnn_LoadPnetModel(*this, pnet_fname, m_pnet_model, m_pnet_model_fb);
	inf_tf_adapter::LiteMtcnn_LoadRnetModel(*this, rnet_fname, m_rnet_model, m_rnet_model_fb);
	inf_tf_adapter::LiteMtcnn_LoadOnetModel(*this, onet_fname, m_onet_model, m_onet_model_fb);

	if (m_pnet_model == nullptr || m_rnet_model == nullptr || m_onet_model == nullptr) {
		inf_log_err("Cannot init model");
		return -1;
	}

	if (m_pnet_model->AllocateTensors() != kTfLiteOk || m_rnet_model->AllocateTensors() != kTfLiteOk || m_onet_model->AllocateTensors() != kTfLiteOk) {
		inf_log_err("Cannot allocate tensor!");
		return -1;
	}
	inf_tf_adapter::showArenaUsedBytes(*m_pnet_model, "pnet");
	inf_tf_adapter::showArenaUsedBytes(*m_rnet_model, "rnet");
	inf_tf_adapter::showArenaUsedBytes(*m_onet_model, "onet");

	inf_tf_adapter::setNumThreads(*m_pnet_model, m_num_thread);
	inf_tf_adapter::setNumThreads(*m_rnet_model, m_num_thread);
	inf_tf_adapter::setNumThreads(*m_onet_model, m_num_thread);

	m_input_dim[0] = m_pnet_model->input_tensor(0)->dims->data;
	m_input_dim[1] = m_rnet_model->input_tensor(0)->dims->data;
	m_input_dim[2] = m_onet_model->input_tensor(0)->dims->data;

	m_type = utils::lite::GetDataType(m_pnet_model->input_tensor(0)->type);
	assert(m_type == utils::lite::GetDataType(m_rnet_model->input_tensor(0)->type));
	assert(m_type == utils::lite::GetDataType(m_onet_model->input_tensor(0)->type));

	if (m_type == Inf8U || m_type == Inf8S) {
		GetQuantInfo(m_pnet_model, m_pnet_info, m_pnet_index, 2, 4, 0);
		GetQuantInfo(m_rnet_model, m_rnet_info, m_rnet_index, 2, 4, 0);
		GetQuantInfo(m_onet_model, m_onet_info, m_onet_index, 2, 4, 10);
	} else {
		inf_log_err("Current MTCNN model can only support 8S/8U tflite model!");
		return -1;
	}

	std::vector<int> _dims{1, 12, 12, m_input_dim[0][3]};
	//TODO: how to achieve this in microlite???
	//m_pnet_model->ResizeInputTensor(input_idx_pnet, _dims);
	inf_tf_adapter::resizeInputTensor(*m_pnet_model, 0, _dims);

	if (m_pnet_model->AllocateTensors() != kTfLiteOk || m_rnet_model->AllocateTensors() != kTfLiteOk || m_onet_model->AllocateTensors() != kTfLiteOk) {
		inf_log_err("Cannot allocate tensor!");
		return -1;
	}

	if (m_verbose) TOC("Load MTCNN", start);

	inf_log_notice("Mtcnn model type is %s", GetDTypeString(m_type));
	return 0;
}

template<typename IC>
void LiteMtcnn::GetQuantInfo(const std::unique_ptr<IC>& model, QuantInfo* info, int net_idx, int prob_num, int coord_num, int landmark_num)
{
	int num_output = (prob_num != 0) + (coord_num != 0) + (landmark_num != 0);
	if (inf_tf_adapter::numsOfOutputTensor(*model) != static_cast<size_t>(num_output)) {
		inf_log_err("number of output is not correct!");
		return;
	}

#define SET_QUANT_INFO(net_idx, x, y) \
	do { \
		m_output_prob_idx[net_idx] = x; \
		m_output_reg_idx[net_idx] = y; \
		const TfLiteQuantizationParams *p = &model->output_tensor(0)->params; \
		info[0].zero = p->zero_point; \
		info[0].scale = p->scale; \
		p = &model->output_tensor(1)->params; \
		info[1].zero = p->zero_point; \
		info[1].scale = p->scale; \
	} while (0)

#define SET_QUANT_OINFO(net_idx, x, y, z) \
	do { \
		m_output_prob_idx[net_idx] = x; \
		m_output_reg_idx[net_idx] = y; \
		m_output_landmark_idx = z; \
		const TfLiteQuantizationParams *p = &model->output_tensor(0)->params; \
		info[0].zero = p->zero_point; \
		info[0].scale = p->scale; \
		p = &model->output_tensor(1)->params; \
		info[1].zero = p->zero_point; \
		info[1].scale = p->scale; \
		p = &model->output_tensor(2)->params; \
		info[2].zero = p->zero_point; \
		info[2].scale = p->scale; \
	} while (0)

	auto dims = model->output_tensor(0)->dims;
	/* MTCNN output tensor may be permuted */
	if (net_idx == m_pnet_index) { // pnet
		assert(dims->size == 4);
		if (dims->data[3] == prob_num) {
			SET_QUANT_INFO(net_idx, 0, 1);
		} else if (dims->data[3] == coord_num) {
			SET_QUANT_INFO(net_idx, 1, 0);
		}
	} else if (net_idx == m_rnet_index) {
		assert(dims->size == 2);
		if (dims->data[1] == prob_num) {
			SET_QUANT_INFO(net_idx, 0, 1);
		} else if (dims->data[1] == coord_num) {
			SET_QUANT_INFO(net_idx, 1, 0);
		}
	} else if (net_idx == m_onet_index) {
		assert(dims->size == 2);
		auto dims1 = model->output_tensor(1)->dims;
		if (dims->data[1] == prob_num && dims1->data[1] == coord_num) {
			SET_QUANT_OINFO(net_idx, 0, 1, 2);
		} else if (dims->data[1] == prob_num && dims1->data[1] == landmark_num) {
			SET_QUANT_OINFO(net_idx, 0, 2, 1);
		} else if (dims->data[1] == coord_num && dims1->data[1] == prob_num) {
			SET_QUANT_OINFO(net_idx, 1, 0, 2);
		} else if (dims->data[1] == coord_num && dims1->data[1] == landmark_num) {
			SET_QUANT_OINFO(net_idx, 2, 0, 1);
		} else if (dims->data[1] == landmark_num && dims1->data[1] == prob_num) {
			SET_QUANT_OINFO(net_idx, 1, 2, 0);
		} else if (dims->data[1] == landmark_num && dims1->data[1] == coord_num) {
			SET_QUANT_OINFO(net_idx, 2, 1, 0);
		} else {
			inf_log_err("Unknown Onet output dimesion!");
			assert(0);
		}
	}

	inf_log_debug("out:%d float: %.9f, zero: %d", i, info[i].scale, info[i].zero);
}

template<typename IC>
static int RunNetwork(IC *interpreter,
                      const InfImage* img, const Shape& size, int chn, InfDataType mtype)
{

	/* Get input/output tensor index */
	std::vector<int> _dims{1, size.h, size.w, chn};
	inf_tf_adapter::resizeInputTensor(*interpreter, 0, _dims);
	int ret = interpreter->AllocateTensors();
	if (ret != kTfLiteOk) {
		inf_log_err("Cannot allocate buffer!");
		return -1;
	}

	InfDataType m = static_cast<InfDataType>(GetImageType(mtype, chn));
	if (mtype == Inf8U) {
		uint8_t* input_addr = interpreter->template typed_input_tensor<uint8_t>(0);
		InfImage wimg{size.w, size.h, chn, input_addr, 0, m};
		Inf_Imresize(img, size.w, size.h, &wimg);
	} else if (mtype == Inf8S) {
		int8_t* input_addr = interpreter->template typed_input_tensor<int8_t>(0);
		InfImage wimg{size.w, size.h, chn, reinterpret_cast<uint8_t*>(input_addr), 0, m};
		Inf_Imresize(img, size.w, size.h, &wimg);
	} else {
		inf_log_err("Datatype %d not supported!", mtype);
		assert(0);
	}
	// Size [] => w, h
	//printf("%d %d => %d %d\n", Size)
	if (interpreter->Invoke() != kTfLiteOk) {
		inf_log_warn("Cannot Invoke tflite model!");
	}

	return 0;
}

static void CopyOnePatch(const InfImage* img, FaceBox& input_box, uint8_t* data_to, int height, int width, int chn, InfDataType dtype)
{
	InfImage resized{height, width, chn, data_to, 0, dtype}; // CV_8UC3 = 16

	int pad_top = std::abs(input_box.py0 - input_box.y0);
	int pad_left = std::abs(input_box.px0 - input_box.x0);
	int pad_bottom = std::abs(input_box.py1 - input_box.y1);
	int pad_right = std::abs(input_box.px1 - input_box.x1);

	Inf_ImcropPadResize(img, input_box.px0, input_box.py0, input_box.px1, input_box.py1,
                        pad_top, pad_bottom, pad_left, pad_right, &resized, width, height);
}

template<typename IC>
static int RunNetwork(IC* interpreter, const InfImage* img,
                      const Shape& size, FaceBox& box, int chn, InfDataType mtype)
{
	/* Get input/output tensor index */
	auto dims = interpreter->input_tensor(0)->dims;
	int wanted_height = dims->data[1];
	int wanted_width = dims->data[2];
	int wanted_chn = dims->data[3];

	InfDataType dtype = static_cast<InfDataType>(GetImageType(mtype, wanted_chn));

	if (mtype == Inf8U) {
		uint8_t* input_addr = interpreter->template typed_input_tensor<uint8_t>(0);
		CopyOnePatch(img, box, input_addr, wanted_height, wanted_width, wanted_chn, dtype);
	} else if (mtype == Inf8S) {
		int8_t* input_addr = interpreter->template typed_input_tensor<int8_t>(0);
		CopyOnePatch(img, box, (uint8_t*)input_addr, wanted_height, wanted_width, wanted_chn, dtype);
	} else {
		inf_log_warn("%s does not support floating point inference.", __func__);
		return 0;
	}

	if (interpreter->Invoke() != kTfLiteOk) {
		return -1;
	}
	return 0;
}

int LiteMtcnn::RunPNet(const InfImage* img, ScaleWindow& win, std::vector<FaceBox>& box_list)
{
	int scale_h = win.h;
	int scale_w = win.w;
	Shape win_wize{scale_w, scale_h};
	float scale = win.scale;
	m_input_dim[m_pnet_index] = m_pnet_model->input_tensor(0)->dims->data;
	int chn = m_input_dim[m_pnet_index][3];

	int ret = RunNetwork(m_pnet_model.get(), img, win_wize, chn, m_type);
	if (ret) {
		inf_log_warn("Fail to run at [%d,%d].", win.h, win.w);
		return ret;
	}

	const int conf_index = m_output_prob_idx[m_pnet_index];
	const int reg_index = m_output_reg_idx[m_pnet_index];
	const TfLiteTensor* conf_tensor = m_pnet_model->output_tensor(conf_index);

	int feature_h= conf_tensor->dims->data[1];
	int feature_w= conf_tensor->dims->data[2];

	int conf_size = feature_h * feature_w * 2;

	std::vector<FaceBox> candidate_boxes;

	const QuantInfo &info_conf = m_pnet_info[conf_index];
	const QuantInfo &info_reg = m_pnet_info[reg_index];

	if (m_type == Inf8U) {
		const uint8_t *conf_data = m_pnet_model->typed_output_tensor<uint8_t>(conf_index);
		const uint8_t *reg_data = m_pnet_model->typed_output_tensor<uint8_t>(reg_index);
		GenerateBoundingBox(info_conf, conf_data, conf_size, info_reg, reg_data,
			scale, m_pnet_threshold, feature_h, feature_w, candidate_boxes, false);
	} else if (m_type == Inf8S) {
		const int8_t *conf_data = m_pnet_model->typed_output_tensor<int8_t>(conf_index);
		const int8_t *reg_data = m_pnet_model->typed_output_tensor<int8_t>(reg_index);
		GenerateBoundingBox(info_conf, conf_data, conf_size, info_reg, reg_data,
			scale, m_pnet_threshold, feature_h, feature_w, candidate_boxes, false);
	} else {
		assert(0);
	}

	//std::vector<FaceBox> tmp_box = candidate_boxes;
	NmsBoxes(candidate_boxes, m_inter_nms_th[0], NMS_UNION, box_list);

	return 0;
}

template<typename Tbuffer, InfDataType mtype>
int LiteMtcnn::TRunRNet(const InfImage* img, std::vector<FaceBox>& pnet_boxes,
	std::vector<FaceBox>& output_boxes)
{
	const int height = m_input_dim[m_rnet_index][1];
	const int width = m_input_dim[m_rnet_index][2];
	const int chn = m_input_dim[m_rnet_index][3];
	const Shape size{width, height};

	const int conf_index = m_output_prob_idx[m_rnet_index];
	const int reg_index = m_output_reg_idx[m_rnet_index];

	float conf_data[2]{};
	float reg_data[4]{};
	int num_box = pnet_boxes.size();

	const Tbuffer *conf_data_ptr = m_rnet_model->template typed_output_tensor<Tbuffer>(conf_index);
	const Tbuffer *reg_data_ptr = m_rnet_model->template typed_output_tensor<Tbuffer>(reg_index);
	const QuantInfo& conf_info = m_rnet_info[conf_index];
	const QuantInfo& reg_info = m_rnet_info[reg_index];

	inf_log_debug("rnet input: %d",interpreter->inputs()[0]);
	inf_log_debug("tensors Size: %d",interpreter->tensors_size());
	inf_log_debug("nodes Size: %d", interpreter->nodes_size());
	inf_log_debug("input(0) name: %s", interpreter->GetInputName(0));
	inf_log_debug("number of outputs: %d", outputs.Size());
	inf_log_debug("[INFO] inputs[0] dims: [%d %d %d %d]", num_box, height, width, chn);

	for (int i = 0; i < num_box; ++i) {
		RunNetwork(m_rnet_model.get(), img, size, pnet_boxes[i], chn, m_type);

		if (m_type == Inf8U || m_type == Inf8S) {
			conf_data[1] = QuantConvert(conf_info, conf_data_ptr[1]);
			for (int j = 0; j < 4; j++)
				reg_data[j] = QuantConvert(reg_info, reg_data_ptr[j]);
		}

		FaceBox output_box{};
		FaceBox& input_box=pnet_boxes[i];

		if(conf_data[1]> m_rnet_threshold) {
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
		inf_log_debug("%d conf:%.2f [%.2f %.2f %.2f %.2f] reg:[%.2f %.2f %.2f %.2f]",
				i, conf_data[1], pnet_boxes[i].x0, pnet_boxes[i].y0, pnet_boxes[i].x1, pnet_boxes[i].y1,
				reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
	return 0;
}

int LiteMtcnn::RunRNet(const InfImage* img, std::vector<FaceBox>& pnet_boxes, std::vector<FaceBox>& output_boxes)
{
	//int batch = 1;
	int ret;

	if (m_type == Inf8U) {
		ret = TRunRNet<uint8_t, Inf8U>(img, pnet_boxes, output_boxes);
	} else if (m_type == Inf8S) {
		ret = TRunRNet<int8_t, Inf8S>(img, pnet_boxes, output_boxes);
	} else {
		inf_log_warn("Current RNET does not support floating point inference!");
		return -1;
	}

	return ret;
}


template<typename Tbuffer, InfDataType mtype>
int LiteMtcnn::TRunONet(const InfImage* img, std::vector<FaceBox>& rnet_boxes, std::vector<FaceBox>& output_boxes)
{

	const int height = m_input_dim[m_onet_index][1];
	const int width = m_input_dim[m_onet_index][2];
	const int chn = m_input_dim[m_onet_index][3];
	const Shape size{width, height};

	const int conf_index = m_output_prob_idx[m_onet_index];
	const int reg_index = m_output_reg_idx[m_onet_index];
	const int landmark_index = m_output_landmark_idx;

	const uint32_t batch = rnet_boxes.size();

	const Tbuffer* conf_data_ptr = m_onet_model->template typed_output_tensor<Tbuffer>(conf_index);
	const Tbuffer* reg_data_ptr = m_onet_model->template typed_output_tensor<Tbuffer>(reg_index);
	const Tbuffer* points_data_ptr = m_onet_model->template typed_output_tensor<Tbuffer>(landmark_index);
	const QuantInfo& conf_info = m_onet_info[conf_index];
	const QuantInfo& reg_info = m_onet_info[reg_index];
	const QuantInfo& points_info = m_onet_info[landmark_index];

	float conf_data[2]{};
	float reg_data[4]{};
	float points_data[10]{};

	for(uint32_t i=0; i<batch; i++) {
		RunNetwork(m_onet_model.get(), img, size, rnet_boxes[i], chn, mtype);

		if (mtype == Inf8U || mtype == Inf8S) {
			conf_data[1] = QuantConvert(conf_info, conf_data_ptr[1]);
			for (int j = 0; j < 4; j++)
				reg_data[j] = QuantConvert(reg_info, reg_data_ptr[j]);
			for (int j = 0; j < 10; j++)
				points_data[j] = QuantConvert(points_info, points_data_ptr[j]);
		}
		FaceBox output_box{};
		FaceBox& input_box=rnet_boxes[i];

		if(conf_data[1] > m_onet_threshold) {
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
		inf_log_debug("%d conf:%.2f [%.4f %.4f %.4f %.4f] reg:[%.4f %.4f %.4f %.4f]",
		i, conf_data[1], rnet_boxes[i].x0, rnet_boxes[i].y0, rnet_boxes[i].x1, rnet_boxes[i].y1,
		reg_data[1], reg_data[0], reg_data[3], reg_data[2]);
	}
	return 0;
}

int LiteMtcnn::RunONet(const InfImage* img, std::vector<FaceBox>& rnet_boxes, std::vector<FaceBox>& output_boxes)
{
	int ret;
	if (m_type == Inf8U) {
		ret = TRunONet<uint8_t, Inf8U>(img, rnet_boxes, output_boxes);
	} else if (m_type == Inf8S) {
		ret = TRunONet<int8_t, Inf8S>(img, rnet_boxes, output_boxes);
	} else {
		inf_log_warn("Current RNET does not support floating point inference!");
		return -1;
	}
	return ret;
}

int LiteMtcnn::FaceDetect(const InfImage* img, std::vector<FaceBox>& face_list)
{
	int img_h=img->h;
	int img_w=img->w;

	std::vector<ScaleWindow> win_list;
	std::vector<FaceBox> total_pnet_boxes;
	std::vector<FaceBox> total_rnet_boxes;
	std::vector<FaceBox> total_onet_boxes;

	CalPyramidList(img_h, img_w, m_min_size, m_factor, win_list);

#ifdef INF_MTCNN_DEBUG
	for (auto win : win_list) {
		printf("[(%d %d %.2f)]", win.h, win.w, win.scale);
	}
	printf("\n");
#endif

	if (m_verbose) TIC(start);

	for (auto& i : win_list) {
		std::vector<FaceBox> boxes;
		RunPNet(img, i, boxes);
		total_pnet_boxes.insert(total_pnet_boxes.end(), boxes.begin(), boxes.end());
	}

	inf_log_debug("stage one: %d total pre-process pnet_boxes", (int)total_pnet_boxes.size());

	std::vector<FaceBox> pnet_boxes;

	ProcessBoxes(total_pnet_boxes, img_h, img_w, pnet_boxes, m_inter_nms_th[1]);

	inf_log_debug("stage one->: %d total post-process pnet_boxes", (int)pnet_boxes.size());

	if (m_verbose) TOC("Lite PNet Inference", start);

	if (pnet_boxes.empty()) {
		return 0;
	}
	// RNet
	if (m_verbose) TIC(start);

	std::vector<FaceBox> rnet_boxes;

	RunRNet(img, pnet_boxes, total_rnet_boxes);

	ProcessBoxes(total_rnet_boxes, img_h, img_w, rnet_boxes, m_inter_nms_th[2]);

	if (m_verbose) TOC("Lite RNet Inference", start);

	inf_log_debug("stage two: %d total_rnet_boxes", (int)rnet_boxes.size());
	if (rnet_boxes.empty()) {
		return 0;
	}
	//ONet
	if (m_verbose) TIC(start);

	RunONet(img, rnet_boxes, total_onet_boxes);

	for (auto& box : total_onet_boxes) {
		int h = box.x1 - box.x0 + 1;
		int w = box.y1 - box.y0 + 1;

		for (int j = 0; j < 5; j++) {
			box.landmark.x[j] = box.x0 + w * box.landmark.x[j] - 1;
			box.landmark.y[j] = box.y0 + h * box.landmark.y[j] - 1;
		}
	}

	//Get Final Result
	if (m_verbose) TOC("Lite ONet Inference", start);

	RegressBoxes(total_onet_boxes);
	NmsBoxes(total_onet_boxes, m_nms_th, NMS_MIN, face_list);

	if (m_verbose)
		inf_log_notice("Stage one: %d=>%d, two: %d three: %d total_onet_boxes", (int)total_pnet_boxes.size(), (int)pnet_boxes.size(), (int)rnet_boxes.size(), (int)face_list.size());

	return 0;
}


int LiteMtcnn::FaceDetect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, std::vector<FaceBox>& face_list)
{
#define MIN_BOX_SIZE (12)
	std::vector<InfImage> img_list;
	MPI_IVA_OBJ_LIST_S local = *obj_list;

	if (!img || !obj_list) {
		inf_log_warn("input cannot be NULL pointer!");
		return 0;
	}

	// Crop image batches
	for (int i = 0; i < local.obj_num; i++) {
		auto& rect = local.obj[i].rect;
		// Clip on image edges
		rect.sx = Clamp(rect.sx, 0, rect.ex);
		rect.sy = Clamp(rect.sy, 0, rect.ey);
		rect.ex = Clamp(rect.ex, rect.sx, img->w);
		rect.ey = Clamp(rect.ey, rect.sy, img->h);
		int obj_w = rect.ex - rect.sx + 1;
		int obj_h = rect.ey - rect.sy + 1;

		InfImage wimg{obj_w, obj_h, img->c, nullptr, 1, img->dtype};
		if (obj_w < MIN_BOX_SIZE || obj_h < MIN_BOX_SIZE) {
			if (m_verbose)
				inf_log_warn("object [%d,%d,%d,%d] exceed image boundary [%dx%d] or too small!",
				obj_list->obj[i].rect.sx,
				obj_list->obj[i].rect.sy,
				obj_list->obj[i].rect.ex,
				obj_list->obj[i].rect.ey,
				img->w, img->h);
			// do nothing.
		} else {
			Inf_ImcropResize(img, rect.sx, rect.sy, rect.ex, rect.ey, &wimg, obj_w, obj_h);
		}
		img_list.push_back(wimg);
	}

	// heuristic method to define min face
	const bool dynamic_min_face = DYNAMIC_MIN_FACE;
	const int origin_min_face_size = m_min_size;
	int size_fraction_numerator;
	const int size_fraction_denominator = 100;

	for (uint32_t i = 0; i < img_list.size(); i++) {
		std::vector<FaceBox> face_list_local;
		auto& wimg = img_list[i];
		auto& rect = local.obj[i].rect;

		if (wimg.w < m_min_size || wimg.h < m_min_size)
			continue;

		if (dynamic_min_face) {
			float ratio = static_cast<float>(wimg.w) / wimg.h;
			if (ratio < 0.333f) size_fraction_numerator = 33;
			else if (ratio < 0.5f) size_fraction_numerator = 20;
			else size_fraction_numerator = 16;
			m_min_size = std::max((std::min(wimg.w, wimg.h) * size_fraction_numerator) / size_fraction_denominator, origin_min_face_size);
		}

		FaceDetect(&wimg, face_list_local);

		for (auto& face : face_list_local) {
			face.x0 += rect.sx;
			face.x1 += rect.sx;
			face.y0 += rect.sy;
			face.y1 += rect.sy;
		}

		face_list.insert(face_list.end(), face_list_local.begin(), face_list_local.end());
		Inf_Imrelease(&wimg);
	}

	m_min_size = origin_min_face_size;

	return 0;
}

int LiteMtcnn::Detect(const InfImage* img, InfDetList* result)
{
	std::vector<FaceBox> face_list;
	int ret = FaceDetect(img, face_list);
	TransformResult(face_list, result);
	return ret;
}

int LiteMtcnn::Detect(const InfImage* img, const MPI_IVA_OBJ_LIST_S* obj_list, InfDetList* result)
{
	std::vector<FaceBox> face_list;
	int ret = FaceDetect(img, obj_list, face_list);
	TransformResult(face_list, result);
	return ret;
}

#endif // USE_NCNN