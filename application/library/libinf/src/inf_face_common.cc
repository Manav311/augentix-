#include "inf_face_internal.h"

#include <algorithm>
#include <vector>
#include <cmath>

#include "inf_log.h"
#include "inf_types.h"

#include "inf_utils.h"

static int SortFaceCompare(const FaceBox& a, const FaceBox&b)
{
	return a.score > b.score;
}

void NmsBoxes(std::vector<FaceBox>& input, float threshold, int type, std::vector<FaceBox>& output)
{
	std::sort(input.begin(),input.end(), SortFaceCompare);

	int box_num=input.size();

	std::vector<int> merged(box_num,0);

	for (int i=0; i < box_num; i++) {
		if (merged[i])
			continue;

		output.push_back(input[i]);

		float h0=input[i].y1-input[i].y0+1;
		float w0=input[i].x1-input[i].x0+1;

		float area0=h0*w0;

		for (int j=i+1; j < box_num; j++) {
			if (merged[j])
				continue;

			float inner_x0=std::max(input[i].x0,input[j].x0);
			float inner_y0=std::max(input[i].y0,input[j].y0);

			float inner_x1=std::min(input[i].x1,input[j].x1);
			float inner_y1=std::min(input[i].y1,input[j].y1);

			float inner_h=inner_y1-inner_y0+1;
			float inner_w=inner_x1-inner_x0+1;

			if (inner_h<=0 || inner_w<=0)
				continue;

			float inner_area=inner_h*inner_w;

			float h1=input[j].y1-input[j].y0+1;
			float w1=input[j].x1-input[j].x0+1;

			float area1=h1*w1;

			float score;

			if (type == NMS_UNION)
				score=inner_area/(area0+area1-inner_area);
			else
				score=inner_area/std::min(area0,area1);

			if (score>threshold)
				merged[j]=1;
		}
	}
}

void RegressBoxes(std::vector<FaceBox>& rects)
{
	for (unsigned int i=0; i < rects.size(); i++) {
		FaceBox& box=rects[i];

		float h=box.y1-box.y0+1;
		float w=box.x1-box.x0+1;

		box.x0=box.x0+w*box.regress[0];
		box.y0=box.y0+h*box.regress[1];
		box.x1=box.x1+w*box.regress[2];
		box.y1=box.y1+h*box.regress[3];
	}

}

void SquareBoxes(std::vector<FaceBox> &rects)
{
	for (unsigned int i=0; i < rects.size(); i++) {
		float h=rects[i].y1-rects[i].y0+1;
		float w=rects[i].x1-rects[i].x0+1;

		float l=std::max(h,w);

		rects[i].x0=rects[i].x0+((w-l) / 2);
		rects[i].y0=rects[i].y0+((h-l) / 2);
		rects[i].x1=rects[i].x0+l-1;
		rects[i].y1=rects[i].y0+l-1;
	}
}

void Padding(int img_h, int img_w, std::vector<FaceBox> &rects)
{
	for (unsigned int i=0; i < rects.size(); i++) {
		rects[i].px0=std::max(rects[i].x0,1.0f);
		rects[i].py0=std::max(rects[i].y0,1.0f);
		rects[i].px1=std::min(rects[i].x1,(float)img_w);
		rects[i].py1=std::min(rects[i].y1,(float)img_h);
	}
}

void ProcessBoxes(std::vector<FaceBox> &input, int img_h, int img_w, std::vector<FaceBox> &rects, float nms_th)
{
	NmsBoxes(input, nms_th, NMS_UNION, rects);  // nms_th = 0.7

	RegressBoxes(rects);

	SquareBoxes(rects);

	Padding(img_h,img_w,rects);
}

void GenerateBoundingBox(const QuantInfo &info_conf, const uint8_t *confidence_data, int confidence_size,
                         const QuantInfo &info_reg, const uint8_t *reg_data, float scale, float threshold,
                         int feature_h, int feature_w, std::vector<FaceBox> &output, bool transposed)
{
	int stride = 2;
	int cellSize = 12;

	int img_h= feature_h;
	int img_w = feature_w;

	int x_stride;
	int y_stride;

	int k = 0;
	for (int y=0; y<img_h; y++) {
		for (int x=0; x<img_w; x++, k++)
		{
			int line_size=img_w*2;
			uint8_t raw_score=confidence_data[line_size*y+2*x+1];
			float score = QuantConvert(info_conf, raw_score);

			if (score>= threshold) {

				x_stride = x * stride;
				y_stride = y * stride;

				int top_x = (x_stride+1)/scale;
				int top_y = (y_stride+1)/scale;
				int bottom_x = (x_stride+cellSize)/scale;
				int bottom_y = (y_stride+cellSize)/scale;

				FaceBox box;

				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;

				box.score=score;

				int c_offset=(img_w*4)*y+4*x;

				if(transposed)
				{
					box.regress[1]=QuantConvert(info_reg, reg_data[c_offset]);
					box.regress[0]=QuantConvert(info_reg, reg_data[c_offset+1]);
					box.regress[3]=QuantConvert(info_reg, reg_data[c_offset+2]);
					box.regress[2]=QuantConvert(info_reg, reg_data[c_offset+3]);
				}
				else {
					box.regress[0]=QuantConvert(info_reg, reg_data[c_offset]);
					box.regress[1]=QuantConvert(info_reg, reg_data[c_offset+1]);
					box.regress[2]=QuantConvert(info_reg, reg_data[c_offset+2]);
					box.regress[3]=QuantConvert(info_reg, reg_data[c_offset+3]);
				}

				output.push_back(box);
			}

		}
	}
}

void GenerateBoundingBox(const QuantInfo &info_conf, const int8_t *confidence_data, int confidence_size,
                         const QuantInfo &info_reg, const int8_t *reg_data, float scale, float threshold,
                         int feature_h, int feature_w, std::vector<FaceBox> &output, bool transposed)
{
	int stride = 2;
	int cellSize = 12;

	int img_h= feature_h;
	int img_w = feature_w;

	int x_stride;
	int y_stride;

	int k = 0;
	for (int y=0; y<img_h; y++) {
		for (int x=0; x<img_w; x++, k++) {
			int line_size=img_w*2;
			int8_t raw_score=confidence_data[line_size*y+2*x+1];
			float score = QuantConvert(info_conf, raw_score);

			if(score>= threshold)
			{

				x_stride = x * stride;
				y_stride = y * stride;

				int top_x = (x_stride+1)/scale;
				int top_y = (y_stride+1)/scale;
				int bottom_x = (x_stride+cellSize)/scale;
				int bottom_y = (y_stride+cellSize)/scale;

				FaceBox box;

				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;

				box.score=score;

				int c_offset=(img_w*4)*y+4*x;

				if(transposed)
				{
					box.regress[1]=QuantConvert(info_reg, reg_data[c_offset]);
					box.regress[0]=QuantConvert(info_reg, reg_data[c_offset+1]);
					box.regress[3]=QuantConvert(info_reg, reg_data[c_offset+2]);
					box.regress[2]=QuantConvert(info_reg, reg_data[c_offset+3]);
				}
				else {
					box.regress[0]=QuantConvert(info_reg, reg_data[c_offset]);
					box.regress[1]=QuantConvert(info_reg, reg_data[c_offset+1]);
					box.regress[2]=QuantConvert(info_reg, reg_data[c_offset+2]);
					box.regress[3]=QuantConvert(info_reg, reg_data[c_offset+3]);
				}

				output.push_back(box);
			}

		}
	}
}


void CalPyramidList(int height, int width, int min_size, float factor, std::vector<ScaleWindow> &list)
{
	float min_side = std::min(height, width);
	float m = 12.0/min_size;
	const float max_face_size = 12; // * stride;

	min_side=min_side*m;
	float cur_scale= 1.0f;
	float scale;

	inf_log_debug("[h:%d, w:%d, min_size:%d, factor:%.2f min_side:%.2f, m:%.2f]",
		height, width, min_size, factor, min_side, m);
	while (min_side >= max_face_size)
	{
		scale=(m*cur_scale);
		cur_scale= (cur_scale *factor);
		min_side = (min_side * factor);

		float hs = ceilf(height*scale);
		float ws = ceilf(width*scale);
		inf_log_debug(" win:[%.2f %.2f %.2f] cur:%.2f min_side:%.2f ?%.2f",
			hs, ws, scale, cur_scale, min_side, m*cur_scale);
		ScaleWindow win;
		win.h=(int)hs;
		win.w=(int)ws;
		win.scale=scale;
		list.push_back(win);
	}
}

void TransformResult(const std::vector<FaceBox>& face_list, InfDetList* detections,
                     float resize_ratio, int cls_num)
{
	uint32_t size = face_list.size();
	if (!size)
		return;

	if (cls_num > 2 || cls_num < 1) {
		inf_log_err("Wrong cls number (%d) for face detection result!", cls_num);
		return;
	}

	detections->size = size;
	detections->data = (InfDetResult*)malloc(sizeof(InfDetResult) * size);

	for (uint32_t i = 0; i < size; i++) {
		const auto& face = face_list[i];
		auto& det = detections->data[i];
		det.id = i;
		det.rect.sx = face.x0 * resize_ratio;
		det.rect.sy = face.y0 * resize_ratio;
		det.rect.ex = face.x1 * resize_ratio;
		det.rect.ey = face.y1 * resize_ratio;
		det.confidence = face.score;
		det.cls_num = cls_num;
		det.prob_num = cls_num;
		det.cls = (int*) malloc(sizeof(int) * cls_num);
		det.prob = (float*) malloc(sizeof(float) * cls_num);

		if (cls_num==1) {
			det.cls[0] = 0;
			det.prob[0] = det.confidence;
		} else {
			det.cls[0] = 1;
			det.cls[1] = 0;
			det.prob[1] = det.confidence;
			det.prob[0] = 1-det.confidence;
		}
	}
}

template<typename Tbuffer>
void GenerateBoundingBoxScrfd(const QuantInfo& qconf, const Tbuffer* pconf, const QuantInfo& qreg,
                         const Tbuffer* preg, const ScrfdPostProcessConfig& conf, std::vector<FaceBox> &output)
{
	const float conf_thresh = conf.conf_thresh;
	const int number_output = conf.number_output;
	const int num_anchors = conf.num_anchors;
	const int feature_stride = conf.feature_stride;
	const Shape& input = conf.input;

	int mesh_height = input.h / feature_stride;
	int mesh_width = input.w / feature_stride;
#if 1
	int mesh_size = mesh_height * mesh_width;

	if (mesh_size * num_anchors != number_output) {
		inf_log_err("Config is not correct!");
		return;
	}
#endif
	int conf_offset = 0;
	int reg_offset = 0;
	for (int y = 0; y < mesh_height; y++) {
		const int y_stride = y * feature_stride;

		for (int x = 0; x < mesh_width; x++) {
			const int x_stride = x * feature_stride;

			for (int k = 0; k < num_anchors; k++) {
				const float score = QuantConvert(qconf, pconf[conf_offset]);
				const Tbuffer* _preg = &preg[reg_offset];
				conf_offset++;
				reg_offset+=4;

				if (score < conf_thresh)
					continue;
				FaceBox box;
				box.score = score;
				box.x0 = x_stride;
				box.y0 = y_stride;
				box.x1 = x_stride;
				box.y1 = y_stride;
				box.regress[0] = QuantConvert(qreg, _preg[0]) * feature_stride;
				box.regress[1] = QuantConvert(qreg, _preg[1]) * feature_stride;
				box.regress[2] = QuantConvert(qreg, _preg[2]) * feature_stride;
				box.regress[3] = QuantConvert(qreg, _preg[3]) * feature_stride;

				output.push_back(box);
			}
		}
	}
}

template<>
void GenerateBoundingBoxScrfd<float>(const QuantInfo& qconf, const float* pconf, const QuantInfo& qreg,
                         const float* preg, const ScrfdPostProcessConfig& conf, std::vector<FaceBox> &output)
{
	const float conf_thresh = conf.conf_thresh;
	const int number_output = conf.number_output;
	const int num_anchors = conf.num_anchors;
	const int feature_stride = conf.feature_stride;
	const Shape& input = conf.input;

	int mesh_height = input.h / feature_stride;
	int mesh_width = input.w / feature_stride;

	int mesh_size = mesh_height * mesh_width;

	if (mesh_size * num_anchors != number_output) {
		inf_log_err("Config is not correct!");
		return;
	}

	int offset = 0;
	for (int y = 0; y < mesh_height; y++) {
		const int y_stride = y * feature_stride;

		for (int x = 0; x < mesh_width; x++) {
			const int x_stride = x * feature_stride;

			for (int k = 0; k < num_anchors; k++) {
				const int conf_ind = offset;
				const int reg_ind = offset * 4;
				const float score = pconf[conf_ind];
				offset++;

				if (score < conf_thresh)
					continue;
				FaceBox box;
				box.x0 = x_stride;
				box.y0 = y_stride;
				box.x1 = x_stride;
				box.y1 = y_stride;
				box.regress[0] = preg[reg_ind + 0] * feature_stride;
				box.regress[1] = preg[reg_ind + 1] * feature_stride;
				box.regress[2] = preg[reg_ind + 2] * feature_stride;
				box.regress[3] = preg[reg_ind + 3] * feature_stride;

				output.push_back(box);
			}
		}
	}
}


void Distance2Bbox(std::vector<FaceBox>& rects)
{
	for(size_t i=0; i < rects.size(); i++)
	{
		FaceBox& box=rects[i];

		box.x0 = box.x0-box.regress[0];
		box.y0 = box.y0-box.regress[1];
		box.x1 = box.x1+box.regress[2];
		box.y1 = box.y1+box.regress[3];
	}
}

#ifdef USE_NCNN
void GenerateBoundingBoxScrfd(const float* pconf, const float* preg,
							  const ScrfdPostProcessConfig& conf, std::vector<FaceBox> &output)
{
	const float conf_thresh = conf.conf_thresh;
	const int number_output = conf.number_output;
	const int num_anchors = conf.num_anchors;
	const int feature_stride = conf.feature_stride;
	const Shape& input = conf.input;

	int mesh_height = input.h / feature_stride;
	int mesh_width = input.w / feature_stride;

	int mesh_size = mesh_height * mesh_width;

	if (mesh_size * num_anchors != number_output) {
		inf_log_err("Config is not correct!");
		return;
	}

	int offset = 0;
	for (int y = 0; y < mesh_height; y++) {
		const int y_stride = y * feature_stride;

		for (int x = 0; x < mesh_width; x++) {
			const int x_stride = x * feature_stride;

			for (int k = 0; k < num_anchors; k++) {
				const int conf_ind = offset;
				const int reg_ind = offset * 4;
				const float score = pconf[conf_ind];
				offset++;

				if (score < conf_thresh)
					continue;
				FaceBox box;
				box.x0 = x_stride;
				box.y0 = y_stride;
				box.x1 = x_stride;
				box.y1 = y_stride;
				box.regress[0] = preg[reg_ind + 0] * feature_stride;
				box.regress[1] = preg[reg_ind + 1] * feature_stride;
				box.regress[2] = preg[reg_ind + 2] * feature_stride;
				box.regress[3] = preg[reg_ind + 3] * feature_stride;

				output.push_back(box);
			}
		}
	}
}

int ScrfdPostProcess(const float* pconf, const float* preg,
	const ScrfdPostProcessConfig& conf,
	std::vector<FaceBox>& face_list)
{
	GenerateBoundingBoxScrfd(pconf, preg, conf, face_list);

	Distance2Bbox(face_list);

	/* perform nms from final stage instead inside the loop */

	return 0;
}
#endif // USE_NCNN


template<typename Tbuffer>
int ScrfdPostProcess(const Tbuffer* pconf, const Tbuffer* preg,
	const QuantInfo& qconf, const QuantInfo& qreg,
	const ScrfdPostProcessConfig& conf,
	std::vector<FaceBox>& face_list)
{
	/* postprocessing step
	1. inf -> output -> unquant
	vector<bbox> boxes;
	for i in prediction(scores, bbox)
		if scores[i] > conf_thresh:
			2. bbox_pre * stride
			3. generate anchor-center meshgrid x,y
			3.a (input_height // stride) X (input_width // stride)
			3.b array number is multiplied by num_anchor of 3.a
			4. distance2bbox(anchor_center, prediction box)
			4.a x1 = anchor-center_i_x - box_0
			4.b y1 = anchor-center_i_y - box_1
			4.c x2 = anchor-center_i_x + box_2
			4.d y2 = anchor-center_i_y + box_3
			boxes.push_back([x1,y1,x2,y2,score])
	5. nms(boxes)
	5.a sort scores
	5.b for each box
	5.c     calc iou score
	5.d     if iou score < thresh: keep
	           -> else filter.
	return
	*/

	GenerateBoundingBoxScrfd(qconf, pconf, qreg, preg, conf, face_list);

	Distance2Bbox(face_list);

	/* perform nms from final stage instead inside the loop */

	return 0;
}

/* instantiate function template */
template
int ScrfdPostProcess(const float*, const float*,
	const QuantInfo&, const QuantInfo&, const ScrfdPostProcessConfig&,
	std::vector<FaceBox>&);

template
int ScrfdPostProcess(const uint8_t*, const uint8_t*,
	const QuantInfo&, const QuantInfo&, const ScrfdPostProcessConfig&,
	std::vector<FaceBox>&);

template
int ScrfdPostProcess(const int8_t*, const int8_t*,
	const QuantInfo&, const QuantInfo&, const ScrfdPostProcessConfig&,
	std::vector<FaceBox>&);
