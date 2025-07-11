#include <stdio.h>
#include <assert.h>
#include <iostream>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "utils.h"

using namespace std;

static vector<uint8_t> buf;

CvVideo CV_utils_OpenVideo(const char *video_file)
{
	cv::VideoCapture *cap = new cv::VideoCapture;
	cap->open(video_file, cv::CAP_ANY);
	int ret = cap->isOpened();
	if (!ret) {
		fprintf(stderr, "[ERROR] Cannot open %s!\n", video_file);
		exit(0);
	}
	CvVideo vid;
	vid.cap = (VideoCapPtr) cap;
	vid.width = (int)cap->get(cv::CAP_PROP_FRAME_WIDTH);
	vid.height = (int)cap->get(cv::CAP_PROP_FRAME_HEIGHT);
	vid.fps = (float)cap->get(cv::CAP_PROP_FPS );
	return vid;
}

void CV_utils_GetFrame(CvVideo *vid, CvImage *img, int channel)
{
	if (img->mat)
		delete (cv::Mat*)img->mat;
	img->mat = 0;
	img->data = 0;
	img->mat = (void*) new cv::Mat;
	bool ret = ((cv::VideoCapture*)vid->cap)->read(*(cv::Mat*)img->mat);
	if (ret) { // success
		img->h = ((cv::Mat*)img->mat)->rows;
		img->w = ((cv::Mat*)img->mat)->cols;
		img->c = ((cv::Mat*)img->mat)->channels();
		img->dtype = ((cv::Mat*)img->mat)->type();
		img->data = (void*)((cv::Mat*)img->mat)->data;
		if (channel == 1) {
			cv::Mat *gray_mat = new cv::Mat;
			cv::cvtColor(*(cv::Mat*)img->mat, *gray_mat, cv::COLOR_BGR2GRAY);
			img->h = gray_mat->rows;
			img->w = gray_mat->cols;
			img->c = gray_mat->channels();
			img->dtype = gray_mat->type();
			img->data = (void*)(gray_mat->data);
			delete (cv::Mat*)img->mat;
			img->mat = (void*)gray_mat;
		} else {
			cv::Mat *rgb_mat = new cv::Mat;
			cv::cvtColor(*(cv::Mat*)img->mat, *rgb_mat, cv::COLOR_BGR2RGB);
			img->h = rgb_mat->rows;
			img->w = rgb_mat->cols;
			img->c = rgb_mat->channels();
			img->dtype = rgb_mat->type();
			img->data = (void*)(rgb_mat->data);
			delete (cv::Mat*)img->mat;
			img->mat = (void*)rgb_mat;
		}
	} else { // false
		delete (cv::Mat*)img->mat;
		img->mat = 0;
		return;
	}
}

void CV_utils_ResizeFrame(CvImage *img, int w, int h)
{
	if (w == 1.0 && h == 1.0)
		return;
	cv::Mat *dst = new cv::Mat;
	cv::resize(*(cv::Mat*)(img->mat), *dst, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
	img->h = dst->rows;
	img->w = dst->cols;
	img->c = dst->channels();
	img->dtype = dst->type();
	img->data = (void*)(dst->data);
	delete (cv::Mat*)(img->mat);
	img->mat = (void*)dst;
}

int CV_utils_EnodeJpegImage(CvImage *img, uint8_t **jpeg)
{
	cv::imencode(".jpg", *(cv::Mat*)img->mat, buf);
	*jpeg = buf.data();
	return buf.size();
}

void CV_utils_ReadImage(CvImage *img, const char *path)
{
	if (!img->mat)
		img->mat = new cv::Mat;
	cv::Mat bgr;
	cv::Mat *rgb = (cv::Mat*)img->mat;
	bgr = cv::imread(path);

	if (!bgr.data) {
		fprintf(stderr, "cannot read %s\n", path);
		assert(0);
	}

	cv::cvtColor(bgr, *rgb, cv::COLOR_BGR2RGB);

	img->h = rgb->rows;
	img->w = rgb->cols;
	img->c = rgb->channels();
	img->dtype = rgb->type();
	img->data = (void*)(rgb->data);
}

void CV_utils_ReadImageGray(CvImage *img, const char *path)
{
	if (!img->mat)
		img->mat = new cv::Mat;
	cv::Mat *mat = (cv::Mat*)img->mat;
	*mat = cv::imread(path, cv::IMREAD_GRAYSCALE);
	if (!mat->data) {
		fprintf(stderr, "cannot read %s\n", path);
		assert(0);
	}
	img->h = mat->rows;
	img->w = mat->cols;
	img->c = mat->channels();
	img->dtype = mat->type();
	img->data = (void*)(mat->data);
}


void CV_utils_destroyFrame(CvImage *img)
{
	if (img->mat)
		delete (cv::Mat*)img->mat;
	img->mat = 0;
	img->data = 0;
}

void CV_utils_closeVideo(CvVideo *vid)
{
	((cv::VideoCapture*)vid->cap)->release();
	delete (cv::VideoCapture*)vid->cap;
	vid->cap = 0;
}

void CV_utils_dumpJpegRaw(const uint8_t *jpeg, int size, const char *path)
{
	FILE *fp = fopen(path, "wb");
	fwrite(jpeg, 1, size, fp);
	fclose(fp);
}

void CV_utils_dumpRectJpeg(CvImage *img, int sx, int sy, int ex, int ey, const char *path)
{
	cv::Mat &src = *(cv::Mat*)img->mat;
	sx = CLAMP(sx, 0, src.cols-2);
	ex = CLAMP(ex, sx, src.cols-1);
	sy = CLAMP(sy, 0, src.rows-2);
	ey = CLAMP(ey, sy, src.rows-1);

	cv::Mat rect = src(cv::Range(sy, ey), cv::Range(sx, ex));
	cv::imwrite(path, rect);
}

void CV_utils_dumpJpeg(const CvImage *img, const char *path)
{
	cv::imwrite(path, *(cv::Mat*)img->mat);
}

void CV_utils_dumpRaw(const CvImage *img, const char *path)
{
	FILE *fp = fopen(path, "wb");
	int size = img->w * img->h * img->c;
	fwrite(&img->w, 1, 4, fp);
	fwrite(&img->h, 1, 4, fp);
	fwrite(&img->c, 1, 4, fp);
	fwrite(&img->dtype, 1, 4, fp);
	fwrite(img->data, 1, size, fp);
	fclose(fp);
}
