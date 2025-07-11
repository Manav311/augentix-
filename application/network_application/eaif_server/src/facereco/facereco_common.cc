/*
  Copyright (C) 2017 Open Intelligent Machines Co.,Ltd
  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at
  http://www.apache.org/licenses/LICENSE-2.0
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#include <algorithm>
#include <vector>

#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "eaif_common.h"
#include "eaif_trc.h"

#include "facereco_common.h"
#include "mtcnn.h"

using namespace std;

const int EAIF_FR_CEIL_ONE = 1 << EAIF_FR_BIT;
const uint32_t EAIF_FR_CEIL_MASK = (uint32_t)(~0) << (EAIF_FR_BIT);

static int EAIF_FR_CEIL(int fp)
{
    int fpValueResult = 0;
    if ((fp & EAIF_FR_CEIL_MASK) == 0) {
        fpValueResult = fp;
    } else {
        fpValueResult = ((fp + EAIF_FR_CEIL_ONE) & EAIF_FR_CEIL_MASK);
    }
    return fpValueResult ;
}

template<typename T>
int SortFaceCompare(const FaceBox<T>& a, const FaceBox<T>&b)
{
	return a.score > b.score;  
}

template<typename T>
void NmsBoxes(std::vector<FaceBox<T>>& input, T threshold, int type, std::vector<FaceBox<T>>&output)
{
	std::sort(input.begin(),input.end(),
			SortFaceCompare<T>);

	int box_num=input.size();

	std::vector<int> merged(box_num,0);

	for(int i=0;i<box_num;i++)
	{ 
		if(merged[i])
			continue;

		output.push_back(input[i]);

		T h0=input[i].y1-input[i].y0+1;
		T w0=input[i].x1-input[i].x0+1;

		T area0=h0*w0;

		for(int j=i+1;j<box_num;j++)
		{
			if(merged[j]) 
				continue;

			T inner_x0=std::max(input[i].x0,input[j].x0);
			T inner_y0=std::max(input[i].y0,input[j].y0);

			T inner_x1=std::min(input[i].x1,input[j].x1);
			T inner_y1=std::min(input[i].y1,input[j].y1);

			T inner_h=inner_y1-inner_y0+1;
			T inner_w=inner_x1-inner_x0+1;


			if(inner_h<=0 || inner_w<=0)
				continue;

			T inner_area=inner_h*inner_w;

			T h1=input[j].y1-input[j].y0+1;
			T w1=input[j].x1-input[j].x0+1;

			T area1=h1*w1;

			T score;

			if(type == NMS_UNION)
				score=inner_area/(area0+area1-inner_area);
			else
				score=inner_area/std::min(area0,area1);

			if(score>threshold)
				merged[j]=1;
		}
	}
}

template
void NmsBoxes(std::vector<FaceBox<float>>&, float, int, std::vector<FaceBox<float>>&);

template
void NmsBoxes(std::vector<FaceBox<double>>&, double, int, std::vector<FaceBox<double>>&);

template<>
void NmsBoxes(std::vector<FaceBox<int>>& input, int threshold, int type, std::vector<FaceBox<int>>&output)
{
	std::sort(input.begin(),input.end(),
			[](const FaceBox<int>& a, const FaceBox<int>&b) {
			return a.score > b.score;  
			});

	int box_num=input.size();

	std::vector<int> merged(box_num,0);

	for(int i=0;i<box_num;i++)
	{ 
		if(merged[i])
			continue;

		output.push_back(input[i]);

		int h0=input[i].y1-input[i].y0+1;
		int w0=input[i].x1-input[i].x0+1;

		int area0=h0*w0;

		for(int j=i+1;j<box_num;j++)
		{
			if(merged[j]) 
				continue;

			int inner_x0=std::max(input[i].x0,input[j].x0);
			int inner_y0=std::max(input[i].y0,input[j].y0);

			int inner_x1=std::min(input[i].x1,input[j].x1);
			int inner_y1=std::min(input[i].y1,input[j].y1);

			int inner_h=inner_y1-inner_y0+1;
			int inner_w=inner_x1-inner_x0+1;


			if(inner_h<=0 || inner_w<=0)
				continue;

			int inner_area=inner_h*inner_w;

			int h1=input[j].y1-input[j].y0+1;
			int w1=input[j].x1-input[j].x0+1;

			int area1=h1*w1;

			int score;

			if(type == NMS_UNION)
				score=EAIF_FR_BIT_U8_DIVIDE(inner_area,(area0+area1-inner_area));
			else
				score=EAIF_FR_BIT_U8_DIVIDE(inner_area,std::min(area0,area1));

			if(score>threshold)
				merged[j]=1;
		}
	}
}

void RegressBoxes(std::vector<FaceBox<int>>& rects, const QuantInfo &info)
{
	for(unsigned int i=0;i<rects.size();i++)
	{
		FaceBox<int>& box=rects[i];

		int h=box.y1-box.y0+1;
		int w=box.x1-box.x0+1;

		box.x0=box.x0+EAIF_FR_DOWN(w*info.Convert(box.regress[0]));
		box.y0=box.y0+EAIF_FR_DOWN(h*info.Convert(box.regress[1]));
		box.x1=box.x1+EAIF_FR_DOWN(w*info.Convert(box.regress[2]));
		box.y1=box.y1+EAIF_FR_DOWN(h*info.Convert(box.regress[3]));
	}

}

void RegressBoxes(std::vector<FaceBox<float>>& rects)
{
	for(unsigned int i=0;i<rects.size();i++)
	{
		FaceBox<float>& box=rects[i];

		float h=box.y1-box.y0+1;
		float w=box.x1-box.x0+1;

		box.x0=box.x0+w*box.regress[0];
		box.y0=box.y0+h*box.regress[1];
		box.x1=box.x1+w*box.regress[2];
		box.y1=box.y1+h*box.regress[3];
	}

}

template<typename T>
void SquareBoxes(std::vector<FaceBox<T>>& rects)
{

	for(unsigned int i=0;i<rects.size();i++)
	{
		T h=rects[i].y1-rects[i].y0+1;
		T w=rects[i].x1-rects[i].x0+1;

		T l=std::max(h,w);

		rects[i].x0=rects[i].x0+((w-l) / 2);
		rects[i].y0=rects[i].y0+((h-l) / 2);
		rects[i].x1=rects[i].x0+l-1;
		rects[i].y1=rects[i].y0+l-1;
	}
}

template<>
void Padding(int img_h, int img_w, std::vector<FaceBox<int> >& rects)
{
	for(unsigned int i=0; i<rects.size();i++)
	{
		rects[i].px0=std::max(rects[i].x0,1);
		rects[i].py0=std::max(rects[i].y0,1);
		rects[i].px1=std::min(rects[i].x1,img_w);
		rects[i].py1=std::min(rects[i].y1,img_h);
	}
} 

template<>
void Padding(int img_h, int img_w, std::vector<FaceBox<float> >& rects)
{
	for(unsigned int i=0; i<rects.size();i++)
	{
		rects[i].px0=std::max(rects[i].x0,1.0f);
		rects[i].py0=std::max(rects[i].y0,1.0f);
		rects[i].px1=std::min(rects[i].x1,(float)img_w);
		rects[i].py1=std::min(rects[i].y1,(float)img_h);
	}
} 

void ProcessBoxes(std::vector<FaceBox<int>>& input, int img_h, int img_w, std::vector<FaceBox<int>>& rects, const QuantInfo& info, int nms_th)
{

	NmsBoxes(input, nms_th, NMS_UNION, rects); 

	RegressBoxes(rects, info);

	SquareBoxes(rects);

	Padding(img_h, img_w, rects);

} 

void ProcessBoxes(std::vector<FaceBox<float> >& input, int img_h, int img_w, std::vector<FaceBox<float> >& rects, float nms_th)
{

	NmsBoxes(input, nms_th, NMS_UNION, rects);  // nms_th = 0.7

	RegressBoxes(rects);

	SquareBoxes(rects);

	Padding(img_h,img_w,rects);

} 

void GenerateBoundingBox(const QuantInfo &info_conf, const uint8_t* confidence_data, int confidence_size,
		const QuantInfo &info_reg, const uint8_t* reg_data, float scale, float threshold, 
		int feature_h, int feature_w, std::vector<FaceBox<float>>&  output, bool transposed)
{

	int stride = 2;
	int cellSize = 12;

	int img_h= feature_h;
	int img_w = feature_w;

	int x_stride;
	int y_stride;

	int k = 0;
	for(int y=0; y<img_h; y++) {
		for(int x=0; x<img_w; x++, k++)
		{
			int line_size=img_w*2;
			uint8_t raw_score=confidence_data[line_size*y+2*x+1];
			float score = info_conf.Convertf(raw_score);

			if(score>= threshold)
			{

				x_stride = x * stride;
				y_stride = y * stride;

				int top_x = (x_stride+1)/scale;
				int top_y = (y_stride+1)/scale;
				int bottom_x = (x_stride+cellSize)/scale;
				int bottom_y = (y_stride+cellSize)/scale;

				FaceBox<float> box;

				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;

				box.score=score;

				int c_offset=(img_w*4)*y+4*x;

				if(transposed)
				{
					box.regress[1]=info_reg.Convertf(reg_data[c_offset]);
					box.regress[0]=info_reg.Convertf(reg_data[c_offset+1]); 
					box.regress[3]=info_reg.Convertf(reg_data[c_offset+2]);
					box.regress[2]=info_reg.Convertf(reg_data[c_offset+3]);
				}
				else {
					box.regress[0]=info_reg.Convertf(reg_data[c_offset]);
					box.regress[1]=info_reg.Convertf(reg_data[c_offset+1]); 
					box.regress[2]=info_reg.Convertf(reg_data[c_offset+2]);
					box.regress[3]=info_reg.Convertf(reg_data[c_offset+3]);
				}

				output.push_back(box);
			}

		}
	}

}

void GenerateBoundingBox(const QuantInfo &info_conf, const int8_t* confidence_data, int confidence_size,
		const QuantInfo &info_reg, const int8_t* reg_data, float scale, float threshold, 
		int feature_h, int feature_w, std::vector<FaceBox<float>>&  output, bool transposed)
{

	int stride = 2;
	int cellSize = 12;

	int img_h= feature_h;
	int img_w = feature_w;

	int x_stride;
	int y_stride;

	int k = 0;
	for(int y=0; y<img_h; y++) {
		for(int x=0; x<img_w; x++, k++)
		{
			int line_size=img_w*2;
			int8_t raw_score=confidence_data[line_size*y+2*x+1];
			float score = info_conf.Convertf(raw_score);

			if(score>= threshold)
			{

				x_stride = x * stride;
				y_stride = y * stride;

				int top_x = (x_stride+1)/scale;
				int top_y = (y_stride+1)/scale;
				int bottom_x = (x_stride+cellSize)/scale;
				int bottom_y = (y_stride+cellSize)/scale;

				FaceBox<float> box;

				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;

				box.score=score;

				int c_offset=(img_w*4)*y+4*x;

				if(transposed)
				{
					box.regress[1]=info_reg.Convertf(reg_data[c_offset]);
					box.regress[0]=info_reg.Convertf(reg_data[c_offset+1]); 
					box.regress[3]=info_reg.Convertf(reg_data[c_offset+2]);
					box.regress[2]=info_reg.Convertf(reg_data[c_offset+3]);
				}
				else {
					box.regress[0]=info_reg.Convertf(reg_data[c_offset]);
					box.regress[1]=info_reg.Convertf(reg_data[c_offset+1]); 
					box.regress[2]=info_reg.Convertf(reg_data[c_offset+2]);
					box.regress[3]=info_reg.Convertf(reg_data[c_offset+3]);
				}

				output.push_back(box);
			}

		}
	}

}

void GenerateBoundingBox(const float* confidence_data, int confidence_size,
		const float* reg_data, float scale, float threshold, 
		int feature_h, int feature_w, std::vector<FaceBox<float>>&  output, bool transposed)
{

	int stride = 2;
	int cellSize = 12;

	int img_h= feature_h;
	int img_w = feature_w;

	int x_stride;
	int y_stride;

	int line_size=img_w*2;

	for(int y=0; y<img_h; y++) {
		for(int x=0; x<img_w; x++)
		{
			float score=confidence_data[line_size*y+2*x+1];
			if(score>= threshold)
			{

				x_stride = x * stride;
				y_stride = y * stride;

				int top_x = (x_stride+1)/scale;
				int top_y = (y_stride+1)/scale;
				int bottom_x = (x_stride+cellSize)/scale;
				int bottom_y = (y_stride+cellSize)/scale;

				FaceBox<float> box;

				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;

				box.score=score;

				int c_offset=(img_w*4)*y+4*x;

				if(transposed)
				{
					box.regress[1]=reg_data[c_offset];
					box.regress[0]=reg_data[c_offset+1]; 
					box.regress[3]=reg_data[c_offset+2];
					box.regress[2]=reg_data[c_offset+3];
				}
				else {
					box.regress[0]=reg_data[c_offset];
					box.regress[1]=reg_data[c_offset+1]; 
					box.regress[2]=reg_data[c_offset+2];
					box.regress[3]=reg_data[c_offset+3];
				}

				output.push_back(box);
			}

		}
	}

}

void GenerateBoundingBox(const float* confidence_data, int confidence_size,
		const float* reg_data, double scale, double threshold, 
		int feature_h, int feature_w, std::vector<FaceBox<double>>&  output, bool transposed)
{

	int stride = 2;
	int cellSize = 12;

	int img_h= feature_h;
	int img_w = feature_w;

	int x_stride;
	int y_stride;

	int line_size=img_w*2;

	for(int y=0; y<img_h; y++) {
		for(int x=0; x<img_w; x++)
		{
			double score=confidence_data[line_size*y+2*x+1];
			if(score>= threshold)
			{

				x_stride = x * stride;
				y_stride = y * stride;

				int top_x = (x_stride+1)/scale;
				int top_y = (y_stride+1)/scale;
				int bottom_x = (x_stride+cellSize)/scale;
				int bottom_y = (y_stride+cellSize)/scale;

				FaceBox<double> box;

				box.x0 = top_x;
				box.y0 = top_y;
				box.x1 = bottom_x;
				box.y1 = bottom_y;

				box.score=score;

				int c_offset=(img_w*4)*y+4*x;

				if(transposed)
				{
					box.regress[1]=reg_data[c_offset];
					box.regress[0]=reg_data[c_offset+1]; 
					box.regress[3]=reg_data[c_offset+2];
					box.regress[2]=reg_data[c_offset+3];
				}
				else {
					box.regress[0]=reg_data[c_offset];
					box.regress[1]=reg_data[c_offset+1]; 
					box.regress[2]=reg_data[c_offset+2];
					box.regress[3]=reg_data[c_offset+3];
				}

				output.push_back(box);
			}

		}
	}

}

/*
void SetInputBuffer(std::vector<cv::Mat>& input_channels,
		float* input_data, const int height, const int width) 
{
	for (int i = 0; i < 3; ++i) {
		cv::Mat channel(height, width, CV_32FC1, input_data);
		input_channels.push_back(channel);
		input_data += width * height;
	}
}
*/

template<>
void  CalPyramidList(int height, int width, int min_size, float factor, std::vector<ScaleWindow<float>>& list)
{
	float min_side = std::min(height, width);
	float m = 12.0/min_size;
	const float max_face_size = 12; // * stride;

	min_side=min_side*m;
	float cur_scale= 1.0f;
	float scale;

	eaif_info_l("[h:%d, w:%d, min_size:%d, factor:%.2f min_side:%.2f, m:%.2f]\n",
		height, width, min_size, factor, min_side, m);
	while (min_side >= max_face_size)
	{
		scale=(m*cur_scale);
		cur_scale= (cur_scale *factor); 
		min_side = (min_side * factor);

		float hs = ceilf(height*scale);
		float ws = ceilf(width*scale);
		eaif_info_l(" win:[%.2f %.2f %.2f] cur:%.2f min_side:%.2f ?%.2f\n",
			hs, ws, scale, cur_scale, min_side, m*cur_scale);
		ScaleWindow<float> win;
		win.h=(int)hs;
		win.w=(int)ws;
		win.scale=scale;
		list.push_back(win);
	}
}

template<>
void  CalPyramidList(int height, int width, int min_size, float factor,  std::vector<ScaleWindow<int>>& list)
{

	int min_side = std::min(height, width);
	int m = EAIF_FR_BIT_DIVIDE(12, min_size);

	min_side=min_side*m; // 1up
	int cur_scale= 1 << EAIF_FR_BIT; // 1up
	int factor_ = factor * (1 << EAIF_FR_BIT); // 1up
	int scale;

	min_size <<= EAIF_FR_BIT; // 1up
	int min_th = (12 << EAIF_FR_BIT); // 1up
	eaif_info_l("[h:%d, w:%d, min_size:%d, factor:%2.f min_side:%d, m:%d]\n",
		height, width, min_size, factor, min_side, m);
	
	while (min_side >= min_th)
	{
		scale=EAIF_FR_DOWN(m*cur_scale);
		cur_scale= EAIF_FR_DOWN(cur_scale*factor_); 
		min_side = EAIF_FR_BIT_MUL(min_side,factor_);

		int hs = EAIF_FR_CEIL(height*scale);
		int ws = EAIF_FR_CEIL(width*scale);
		eaif_info_l(" win:[%d %d-%d %d]->[%.2f %.2f %.2f] cur:%.2f min_side:%.2f ?%d\n",
			height*scale, width*scale, hs, ws, EAIF_FR_BIT_2_FLOAT(hs), 
			EAIF_FR_BIT_2_FLOAT(ws), 
			EAIF_FR_BIT_2_FLOAT(scale), 
			EAIF_FR_BIT_2_FLOAT(cur_scale), 
			EAIF_FR_BIT_2_FLOAT(min_side), m*cur_scale);
		ScaleWindow<int> win;
		win.h=hs;
		win.w=ws;
		win.scale=scale;
		list.push_back(win);

	}
}

template<typename T>
void CalLandmark(std::vector<FaceBox<T>>& box_list)
{
	for(unsigned int i=0;i<box_list.size();i++)
	{
		FaceBox<T>& box=box_list[i];

		T w=box.x1-box.x0+1;
		T h=box.y1-box.y0+1;

		for(int j=0;j<5;j++)
		{
			box.landmark.x[j]=box.x0+w*box.landmark.x[j]-1;
			box.landmark.y[j]=box.y0+h*box.landmark.y[j]-1;
		}

	}
}

template<typename T>
void SetBoxBound(std::vector<FaceBox<T>>& box_list, int img_h, int img_w)
{
	for(unsigned int i=0; i<box_list.size();i++)
	{
		FaceBox<T>& box=box_list[i];

		box.x0=std::max((int)(box.x0),1);
		box.y0=std::max((int)(box.y0),1);
		box.x1=std::min((int)(box.x1),img_w-1);
		box.y1=std::min((int)(box.y1),img_h-1);

	}
}

void TransformResult(const std::vector<FaceBox<int> > &iface_list, std::vector<FaceBox<float> > &face_list,
					const QuantInfo& box_info, const QuantInfo& score_info, const QuantInfo& reg_info)
{
	face_list.clear();

	for (auto& face : face_list) {
		FaceBox<float> det;
		det = {
			box_info.Convertf(face.x0),
			box_info.Convertf(face.y0),
			box_info.Convertf(face.x1),
			box_info.Convertf(face.y1),
			score_info.Convertf(face.score)
		};
		det.landmark = {
			{
				reg_info.Convertf(face.landmark.x[0]),
				reg_info.Convertf(face.landmark.x[1]),
				reg_info.Convertf(face.landmark.x[2]),
				reg_info.Convertf(face.landmark.x[3]),
				reg_info.Convertf(face.landmark.x[4])
			},
			{
				reg_info.Convertf(face.landmark.y[0]),
				reg_info.Convertf(face.landmark.y[1]),
				reg_info.Convertf(face.landmark.y[2]),
				reg_info.Convertf(face.landmark.y[3]),
				reg_info.Convertf(face.landmark.y[4])
			},
		};
		face_list.emplace_back(std::move(det));
	}
}

void TransformResult(const std::vector<FaceBox<float> > &face_list, std::vector<Detection> &detections, float resize_ratio)
{
	for (auto& face : face_list) {
		Detection det;
		det.box = { face.x0 * resize_ratio, face.y0 * resize_ratio, face.x1 * resize_ratio, face.y1 * resize_ratio};
		det.confidence = face.score;
		det.cls.resize(2);
		det.prob.resize(2);
		det.cls[0] = 1;
		det.prob[1] = det.confidence;
		detections.emplace_back(std::move(det));
	}
}

void TransformResult(const std::vector<FaceBox<int> > &face_list, std::vector<Detection> &detections, const QuantInfo& info, float resize_ratio)
{
	for (auto& face : face_list) {
		Detection det;
		det.box = {
			EAIF_FR_BIT_2_FLOAT(face.x0) * resize_ratio,
			EAIF_FR_BIT_2_FLOAT(face.y0) * resize_ratio,
			EAIF_FR_BIT_2_FLOAT(face.x1) * resize_ratio,
			EAIF_FR_BIT_2_FLOAT(face.y1) * resize_ratio
		};
		det.cls.resize(2);
		det.prob.resize(2);
		det.confidence = info.Convertf(face.score);
		det.cls[0] = 1;
		det.prob[1] = det.confidence;
		detections.emplace_back(std::move(det));
	}
}