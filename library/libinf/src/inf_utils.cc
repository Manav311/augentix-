#include <cassert>
#include <cmath>
#include <cstring>
#include <cstdint>
#include <cstdlib>
#include <sys/stat.h>

#include <algorithm>

#include "inf_log.h"
#include "inf_utils.h"

const char *GetInfTypeStr(InfRunType inference_type)
{
	assert(inference_type < InfRunNum);
	static const char typestr[InfRunNum][16] = {
		"classify",
		"detect",
		"faceencode",
		"facereco"
	};
	return typestr[inference_type];
}

void Sigmoid(float *output_addr, int num_classes)
{
	for (int i = 0; i < num_classes; ++i)
		output_addr[i] = 1 / (1 + exp(-output_addr[i]));
}

void Softmax(float *output_addr, int num_classes)
{
	int i;
	float sum = EPSILON; // avoid zero sum */
	float maximum = -999999.0f;
	for (i = 0; i < num_classes; ++i) {
		if (maximum < output_addr[i]) {
			maximum = output_addr[i];
		}
	}

	std::vector<float> buffer(num_classes, 0);

	for (i = 0; i < num_classes; ++i) {
		buffer[i] = exp(output_addr[i] - maximum);
		sum += buffer[i];
	}

	for (i = 0; i < num_classes; ++i) {
		output_addr[i] = buffer[i] / sum;
	}
}

int GetOutputType(const char *output_type_str)
{
	if (!strcmp(output_type_str, "linear"))
		return InfLinear;
	else if (!strcmp(output_type_str, "sigmoid"))
		return InfSigmoid;
	else if (!strcmp(output_type_str, "softmax"))
		return InfSoftmax;
	else {
		inf_log_warn("Invalid output_type_str %s. Replaced as InfLinear.", output_type_str);
		return InfLinear;
	}
}

static inline int IsInFilterCls(int cls, const InfIntList *filter_cls)
{
	for (int i = 0; i < filter_cls->size; ++i) {
		if (cls == filter_cls->data[i])
			return 1;
	}
	return 0;
}

static inline int IsInFilterOutCls(int cls, const InfIntList *filter_out_cls)
{
	for (int i = 0; i < filter_out_cls->size; ++i) {
		if (cls == filter_out_cls->data[i])
			return 1;
	}
	return 0;
}

static inline int IsPutIntoVec(int cls, const InfIntList *filter_cls, const InfIntList *filter_out_cls)
{
	int filter_cls_size = filter_cls->size;
	int filter_out_cls_size = filter_out_cls->size;
	if (!filter_cls_size) {
		if (!filter_out_cls_size) {
			return 1;
		} else {
			if (IsInFilterOutCls(cls, filter_out_cls))
				return 0;
			return 1;
		}
	}
	return IsInFilterCls(cls, filter_cls);
}

void PrintResult(int output_dim, float *output_buf)
{
	char msg[1024] = {};
	int size = 0;

	size = sprintf(msg, "model ouput result: ");
	for (int k = 0; k < output_dim; k++)
		size += sprintf(&msg[size], "%.4f,", output_buf[k]);

	inf_log_notice("%s", msg);
}

void PostProcess(const float *output_addr, int num_classes, float conf_thresh, int topk, const InfIntList *filter_cls,
                 const InfIntList *filter_out_cls, InfResult *result)
{
	if (topk == 0)
		topk = num_classes;

	result->prob_num = num_classes;
	result->prob = (float *)malloc(num_classes * sizeof(float));
	result->cls = (int *)malloc(num_classes * sizeof(int));

	int *cls = (int *)malloc(num_classes * sizeof(int));

	int output_class = 0;

	for (int i = 0; i < num_classes; ++i)
		cls[i] = i;

	if (num_classes == 1) {
		if (output_addr[0] > conf_thresh) {
			result->cls[0] = 0;
			result->prob[0] = output_addr[0];
			output_class++;
		}
	} else {
		//sort_helper = (const void *)output_addr;
		// Sorthelper sort_helper{output_addr};

		// qsort(cls, sizeof(int), num_classes, sort_helper);
		//qsort(cls, sizeof(int), num_classes, cmp);

		std::sort(cls, cls + num_classes,
		          [&](const int& cls1, const int& cls2) {
			          return output_addr[cls1] > output_addr[cls2];
		          });

		for (int i = 0; i < topk; ++i) {
			int cls_value = cls[i];

			if(IsInFilterOutCls(cls_value,filter_out_cls)){
				topk++;
				topk = std::min(topk, num_classes);
			}

			if (output_addr[cls_value] > conf_thresh) {
				if (IsPutIntoVec(cls_value, filter_cls, filter_out_cls)) {
					result->cls[output_class] = cls_value;
					result->prob[output_class] = output_addr[cls_value];
					output_class++;
				}
			} else {
				break;
			}
		}
	}

	result->cls_num = output_class;
	free(cls);
}

void ReleaseIntList(InfIntList *list)
{
	if (list->size) {
		free(list->data);
		list->size = 0;
		list->data = 0;
	}
}

void ReleaseFloatList(InfFloatList *list)
{
	if (list->size) {
		free(list->data);
		list->size = 0;
		list->data = 0;
	}
}

void ReleaseStrList(InfStrList *list)
{
	if (list->size) {
		for (int i = 0; i < list->size; i++) {
			free(list->data[i]);
		}
		free(list->data);
		list->size = 0;
		list->data = 0;
	}
}

void ReleaseDetResult(InfDetResult *det)
{
	if (det->cls) {
		free(det->cls);
		det->cls = nullptr;
		det->cls_num = 0;
	}
	if (det->prob) {
		free(det->prob);
		det->prob = nullptr;
		det->prob_num = 0;
	}
	det->confidence = 0;
}

void ReleaseConfig(InfModelInfo* config)
{
	ReleaseStrList(&config->labels);
	ReleaseStrList(&config->model_paths);
	ReleaseIntList(&config->filter_cls);
	ReleaseIntList(&config->filter_out_cls);
	ReleaseFloatList(&config->nms_internal_thresh);
	ReleaseFloatList(&config->conf_thresh);
}

void ImsaveBin(const char *name, const InfImage *p_img)
{
	FILE *fp = fopen(name, "wb");
	fwrite(&p_img->dtype, 1, 4, fp);
	fwrite(&p_img->w, 1, 4, fp);
	fwrite(&p_img->h, 1, 4, fp);
	fwrite(&p_img->c, 1, 4, fp);
	fwrite(p_img->data, 1, p_img->w * p_img->h * p_img->c * GetDSize(p_img->dtype), fp);
	fclose(fp);
	return;
}

unsigned char *LoadModelData(const char *model_path, int *model_data_len)
{
	FILE *fp = fopen(model_path, "rb");
	if (!fp) {
		inf_log_warn("Cannot open %s!", model_path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	*model_data_len = ftell(fp);
	rewind(fp);
	auto model_data = new uint8_t[*model_data_len];
	int ret = fread(model_data, 1, *model_data_len, fp);
	if (ret != *model_data_len) {
		inf_log_warn("fread error, size does not match!");
		delete[] model_data;
		model_data = 0;
		*model_data_len = 0;
	}
	fclose(fp);
	return model_data;
}

int SetupDebugTool(char *prefix)
{
	char *pre = getenv("INF_CAP_PREFIX");
	char pos_path[256] = {};
	char neg_path[256] = {};
	char det_path[256] = {};

	if (pre) {
		struct stat st = { 0 };
		strncpy(prefix, pre, 256);
		if (stat(prefix, &st) == -1) {
			mkdir(prefix, 0700);
			strncpy(pos_path, pre, 256);
			strncpy(neg_path, pre, 256);
			strncpy(det_path, pre, 256);
			strcat(pos_path, "/pos");
			strcat(neg_path, "/neg");
			strcat(det_path, "/det");
			mkdir(pos_path, 0700);
			mkdir(neg_path, 0700);
			mkdir(det_path, 0700);
		}
		printf("[INFO] INF_CAP_PREFIX is detected: \"%s\"!", prefix);
		return 1;
	}

	return 0;
}

void VecL2Norm(const std::vector<float>& src, std::vector<float>& dst)
{
	float denominator = 0.0;

	for (auto& v : src)
		denominator += v * v;

	if (denominator) {
		denominator = sqrt(denominator);

		for (size_t i = 0; i < src.size(); i++)
			dst[i] = src[i] / denominator;
	} else {
		for (size_t i = 0; i < src.size(); i++)
			dst[i] = 0;
	}
}

float NormVecDistL2Norm(const std::vector<float>& v0, const std::vector<float>& v1)
{
	std::vector<float> nv0(v0.size(),0), nv1(v1.size(),0);
	VecL2Norm(v0, nv0);
	VecL2Norm(v1, nv1);
	return VecDistL2Norm(nv0, nv1);
}

float VecDistL2Norm(const std::vector<float>& v0, const std::vector<float>& v1)
{
	float sum = 0.0f;
	float ele = 0.0f;
	for (size_t i = 0; i < v0.size(); ++i) {
		ele = v0[i] - v1[i];
		sum += ele * ele;
	}
	return sqrt(sum);
}

float VecDistL1Norm(const std::vector<float>& v0, const std::vector<float>& v1)
{
	float sum = 0.0f;
	for (size_t i = 0; i < v0.size(); ++i)
		sum += abs(v0[i] - v1[i]);
	return sum;
}

float VecCosineSimilarity(const std::vector<float>& v0, const std::vector<float>& v1)
{
	float deno_v0 = 0.0f, deno_v1 = 0.0f, numerator = 0.0f;
	for (size_t i = 0; i < v0.size(); ++i) {
		numerator += v0[i] * v1[i];
		deno_v0 += v0[i] * v0[i];
		deno_v1 += v1[i] * v1[i];
	}
	if (deno_v0 == 0.0f || deno_v1 == 0.0f)
		return 0.0f;
	float result = numerator / (sqrt(deno_v0) * sqrt(deno_v1));
	// reject negative value
	return result > 0 ? result : 0;
}

void Grouping(const MPI_IVA_OBJ_LIST_S* obj_list, MPI_RECT_POINT_S& roi)
{
	roi = obj_list->obj[0].rect;
	for (int i = 1; i < obj_list->obj_num; i++) {
		const MPI_RECT_POINT_S& rect = obj_list->obj[i].rect;
		roi.sx = std::min(rect.sx, roi.sx);
		roi.sy = std::min(rect.sy, roi.sy);
		roi.ex = std::max(rect.ex, roi.ex);
		roi.ey = std::max(rect.sy, roi.ey);
	}
}

#if 0
void PadAndRescale(int pad, const Shape& img, const Shape& dst, const MPI_RECT_POINT_S& roi,
							MPI_RECT_POINT_S& dst_roi)
{
	//const float dst_ratio = (float)dst.w / dst.h;
	int dst_roi_w = 0;
	int dst_roi_h = 0;
	int max_w = 0;
	int max_h = 0;

	// 0. find max resoln under aspect ratio
	int img_w = img.w;
	int img_h = img.h;

	if ((img_w * dst.h) > (img_h * dst.w)) {
		max_h = img_h;
		max_w = (img_h * dst.w) / dst.h;
	} else {
		max_w = img_w;
		max_h = (img_w * dst.h) / dst.w;
	}

	// 1. map to correct aspect ratio after padding
	int src_roi_cx = (roi.sx + roi.ex + 1) / 2;
	int src_roi_cy = (roi.sy + roi.ey + 1) / 2;
	int src_roi_w = roi.ex - roi.sx + 1 + pad * 2;
	int src_roi_h = roi.ey - roi.sy + 1 + pad * 2;

	/* Take network input As minimum input size */
	if (src_roi_h < dst.h || src_roi_w < dst.w) {
		dst_roi_w = dst.w;
		dst_roi_h = dst.h;

	/* Map aspect ratio */
	} else if ((src_roi_w * dst.h) > (src_roi_h * dst.w)) {
		dst_roi_w = src_roi_w;
		dst_roi_h = (src_roi_w * dst.h) / dst.w;
	} else {
		dst_roi_h = src_roi_h;
		dst_roi_w = (src_roi_h * dst.w) / dst.h;
	}

	/* Keep max Size */
	if (dst_roi_w > max_w || dst_roi_h > max_h) {
		dst_roi_w = max_w;
		dst_roi_h = max_h;
	}

	dst_roi.sx = src_roi_cx - dst_roi_w / 2;
	dst_roi.ex = src_roi_cx + dst_roi_w / 2;
	dst_roi.sy = src_roi_cy - dst_roi_h / 2;
	dst_roi.ey = src_roi_cy + dst_roi_h / 2;

	// 2. take offset by the image boundary
	if (dst_roi.sx < 0) {
		dst_roi.ex -= dst_roi.sx;
		dst_roi.sx = 0;
	}
	if (dst_roi.sy < 0) {
		dst_roi.ey -= dst_roi.sy;
		dst_roi.sy = 0;
	}
	if (dst_roi.ex >= img.w) {
		dst_roi.sx -= (dst_roi.ex - img.w);
		dst_roi.ex = img.w-1;
	}
	if (dst_roi.ey >= img.h) {
		dst_roi.sy -= (dst_roi.ey - img.h);
		dst_roi.ey = img.h-1;
	}
}
#else
void PadAndRescale(int pad, const Shape& img, const Shape& dst, const MPI_RECT_POINT_S& roi,
							MPI_RECT_POINT_S& dst_roi)
{
	//const float dst_ratio = (float)dst.w / dst.h;
	int dst_roi_w = 0;
	int dst_roi_h = 0;
	int max_w = 0;
	int max_h = 0;

	// 0. find max resoln under aspect ratio
	int img_w = img.w;
	int img_h = img.h;

	if ((img_w * dst.h) > (img_h * dst.w)) {
		max_w = img_w;
		max_h = (img_w * dst.h) / dst.w;
	} else {
		max_h = img_h;
		max_w = (img_h * dst.w) / dst.h;
	}

	// 1. map to correct aspect ratio after padding
	int src_roi_cx = (roi.sx + roi.ex + 1) / 2;
	int src_roi_cy = (roi.sy + roi.ey + 1) / 2;
	int src_roi_w = roi.ex - roi.sx + 1 + pad * 2;
	int src_roi_h = roi.ey - roi.sy + 1 + pad * 2;

	/* Take network input As minimum input Size */
	if ((src_roi_h < dst.h) && (src_roi_w < dst.w)) {
		dst_roi_w = dst.w;
		dst_roi_h = dst.h;

	/* Map aspect ratio */
	} else if ((src_roi_w * dst.h) > (src_roi_h * dst.w)) {
		dst_roi_w = src_roi_w;
		dst_roi_h = (src_roi_w * dst.h) / dst.w;
	} else {
		dst_roi_h = src_roi_h;
		dst_roi_w = (src_roi_h * dst.w) / dst.h;
	}

	/* Keep max Size */
	if (dst_roi_w > img_w || dst_roi_h > img_h) {
		dst_roi.sx = (img_w / 2) - max_w / 2;
		dst_roi.ex = dst_roi.sx + max_w;
		dst_roi.sy = (img_h / 2) - max_h / 2;
		dst_roi.ey = dst_roi.sy + max_h;
		return;
	}

	dst_roi.sx = src_roi_cx - dst_roi_w / 2;
	dst_roi.ex = src_roi_cx + dst_roi_w / 2;
	dst_roi.sy = src_roi_cy - dst_roi_h / 2;
	dst_roi.ey = src_roi_cy + dst_roi_h / 2;

	// 2. take offset by the image boundary
	if (dst_roi.sx < 0) {
		dst_roi.ex -= dst_roi.sx;
		dst_roi.sx = 0;
	}
	if (dst_roi.sy < 0) {
		dst_roi.ey -= dst_roi.sy;
		dst_roi.sy = 0;
	}
	if (dst_roi.ex >= img.w) {
		dst_roi.sx -= (dst_roi.ex - img.w);
		dst_roi.ex = img.w-1;
	}
	if (dst_roi.ey >= img.h) {
		dst_roi.sy -= (dst_roi.ey - img.h);
		dst_roi.ey = img.h-1;
	}
}
#endif

void RescaleFaceBox(int net_h, int net_w, MPI_RECT_POINT_S& roi)
{
	int roi_w = roi.ex - roi.sx + 1;
	int roi_h = roi.ey - roi.sy + 1;
	int roi_cx = (roi.sx + roi.ex + 1) / 2;
	int roi_cy = (roi.sy + roi.ey + 1) / 2;

	if ((net_w * roi_h ) > (net_h * roi_w ))
		roi_w = roi_h * net_w / net_h;
	else
		roi_h = roi_w * net_h / net_w;

	roi.sx = roi_cx - roi_w/ 2;
	roi.ex = roi.sx + roi_w - 1;
	roi.sy = roi_cy - roi_h/ 2;
	roi.ey = roi.sy + roi_h - 1;

	return;
}

void GetPadInfo(const InfImage* img, Pads& pad, MPI_RECT_POINT_S& roi)
{
	if (roi.sx < 0) {
		pad.left = -roi.sx;
		roi.sx = 0;
	}
	if (roi.sy < 0) {
		pad.top = -roi.sy;
		roi.sy = 0;
	}
	if (roi.ex > img->w - 1) {
		pad.right = roi.ex - img->w + 1;
		roi.ex = img->w - 1;
	}
	if (roi.ey > img->h - 1) {
		pad.bot = roi.ey - img->h + 1;
		roi.ey = img->h - 1;
	}
}

void CopyDetResult(const InfDetResult* src, InfDetResult** dst)
{
	if (*dst)
		ReleaseDetResult(*dst);
	*dst = new InfDetResult;
	**dst = *src;
	(*dst)->cls = new int[src->cls_num];
	(*dst)->prob = new float[src->prob_num];
	memcpy((*dst)->cls, src->cls, sizeof(int) * src->cls_num);
	memcpy((*dst)->prob, src->prob, sizeof(float) * src->prob_num);
}
