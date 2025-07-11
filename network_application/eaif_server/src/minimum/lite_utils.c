#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/stat.h>

#include "lite_utils.h"
#include "lite_trc.h"

#ifdef USE_STB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

static inline int Clamp(int a, int low, int high)
{
	if (a < low)
		return low;
	if (a > high)
		return high;
	return a;
}

static inline int Min(int a, int b)
{
	return (a > b) ? b : a;
}

static inline float Lerp(float a, float b, float w)
{
	return w * b + (1.f - w) * a;
}

static inline void GetPixelU(const uint8_t *src, uint32_t w, uint32_t h, uint32_t x, uint32_t y, int channel,
                             uint8_t *dst)
{
	int idx = (y * w + x) * channel;
	memcpy(dst, &src[idx], sizeof(uint8_t) * channel);
}

static inline void PutDataU(uint8_t *data, uint32_t channel, uint32_t width, uint32_t x, uint32_t y, uint32_t c,
                            float value)
{
	data[(channel * ((y * width) + x)) + c] = value;
}

static inline void PutDataS(int8_t *data, uint32_t channel, uint32_t width, uint32_t x, uint32_t y, uint32_t c,
                            float value)
{
	data[(channel * ((y * width) + x)) + c] = value - 128;
}

int GetDSize(int dtype)
{
	int e_type = dtype & 0b111;
	if (e_type <= 1)
		return sizeof(uint8_t);
	else if (e_type <= 3)
		return sizeof(uint16_t);
	else if (e_type <= 5)
		return sizeof(uint32_t);
	else if (e_type <= 6)
		return sizeof(uint64_t);
	lite_warn("Unknown datatype %d!\n", e_type);
	return sizeof(uint8_t);
}

int GetImageTypeChn(int dtype)
{
	return (((dtype >> 3) & 0x3) + 1);
}

int GetImageType(int dtype, uint32_t chn)
{
	if (chn == 2 || chn > 3) {
		lite_warn("Unknown chn value %d!\n", chn);
		return Lite32FC3;
	}
	return (dtype & 0x7) | ((chn - 1) << 3);
}

float QuantConvert(uint8_t in, int zero, float scale)
{
	return ((int)in - zero) * scale;
}

float QuantConvertS(int8_t in, int zero, float scale)
{
	return ((int)in - zero) * scale;
}

void Sigmoid(float *output_addr, int num_classes)
{
	for (int i = 0; i < num_classes; ++i)
		output_addr[i] = 1 / (1 + exp(-output_addr[i]));
}

int GetOutputType(const char *output_type_str)
{
	if (!strcmp(output_type_str, "linear"))
		return LiteLinear;
	else if (!strcmp(output_type_str, "sigmoid"))
		return LiteSigmoid;
	else {
		lite_warn("invalid output_type_str %s\n", output_type_str);
		return LiteLinear;
	}
}

static const void *sort_helper = NULL;

static inline int IsInFilterCls(int cls, const LiteIntList *filter_cls)
{
	for (size_t i = 0; i < filter_cls->size; ++i) {
		if (cls == filter_cls->data[i])
			return 1;
	}
	return 0;
}

static inline int IsInFilterOutCls(int cls, const LiteIntList *filter_out_cls)
{
	for (size_t i = 0; i < filter_out_cls->size; ++i) {
		if (cls == filter_out_cls->data[i])
			return 1;
	}
	return 0;
}

static inline int IsPutIntoVec(int cls, const LiteIntList *filter_cls, const LiteIntList *filter_out_cls)
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

int cmp(const void *a, const void *b)
{
	int a_idx = *(const int *)a;
	int b_idx = *(const int *)b;
	const float *output = (float *)sort_helper;
	return output[a_idx] < output[b_idx];
}

void PrintResult(int output_dim, float *output_buf)
{
	char msg[1024] = {};
	int size = 0;
	size = sprintf(msg, "model ouput result: ");
	for (int k = 0; k < output_dim; k++)
		size += sprintf(&msg[size], "%.4f,", output_buf[k]);
	lite_info_h("%s\n", msg);
}

void PostProcess(const float *output_addr, int num_classes, float conf_thresh, int topk, const LiteIntList *filter_cls,
                 const LiteIntList *filter_out_cls, LiteResult *result)
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
		sort_helper = (const void *)output_addr;
		qsort(cls, sizeof(int), num_classes, cmp);
		//std::sort(cls.begin(), cls.end(), [&](const int &cls1, const int &cls2) { return output_addr[cls1] > output_addr[cls2]; });

		for (int i = 0; i < topk; ++i) {
			int cls_value = cls[i];
			if (output_addr[cls_value] > conf_thresh) {
				if (IsPutIntoVec(cls_value, filter_cls, filter_out_cls)) {
					result->cls[output_class] = cls_value;
					result->prob[cls_value] = output_addr[cls_value];
					output_class++;
				}
			} else {
				break;
			}
		}
	}

	result->cls_num = output_class;
	if (output_class < num_classes)
		memmove(result->cls, result->cls, sizeof(int) * output_class);
	free(cls);
}

int ParseConfigParam(char *str, LiteModelInfo *conf)
{
	int hit = 1;
	char *tok = NULL;
	if (!strcmp(str, "output_type")) {
		tok = strtok(NULL, "=\n");
		conf->output_type = GetOutputType(tok);
	} else if (!strcmp(str, "width")) {
		tok = strtok(NULL, "=\n");
		conf->w = atoi(tok);
	} else if (!strcmp(str, "height")) {
		tok = strtok(NULL, "=\n");
		conf->h = atoi(tok);
	} else if (!strcmp(str, "channels")) {
		tok = strtok(NULL, "=\n");
		conf->c = atoi(tok);
	} else if (!strcmp(str, "topk")) {
		tok = strtok(NULL, "=\n");
		conf->topk = atoi(tok);
	} else if (!strcmp(str, "model_path")) {
		tok = strtok(NULL, "=\n");
		strcpy(conf->model_path, tok);
	} else if (!strcmp(str, "resize_aspect_ratio")) {
		tok = strtok(NULL, "=\n");
		conf->resize_aspect_ratio = atoi(tok);
	} else if (!strcmp(str, "num_classes")) {
		tok = strtok(NULL, "=\n");
		conf->labels.size = atoi(tok);

		conf->labels.data = (char **)malloc(sizeof(char *) * conf->labels.size);
		for (int i = 0; i < conf->labels.size; i++) {
			conf->labels.data[i] = (char *)malloc(sizeof(char) * LITE_STR_LEN);
		}
	} else if (!strcmp(str, "labels")) {
		tok = strtok(NULL, "=\n");
		if (!conf->labels.size)
			return -1;

		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok != NULL) {
			strcpy(conf->labels.data[i], tok);
			tok = strtok(NULL, ",");
			i++;
		}
		if (i != conf->labels.size) {
			return -1;
		}

	} else if (!strcmp(str, "zeros")) {
		tok = strtok(NULL, "=\n");
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok != NULL && i < 3) {
			conf->norm_zeros[i] = atof(tok);
			tok = strtok(NULL, ",");
			i++;
		};
	} else if (!strcmp(str, "stds")) {
		tok = strtok(NULL, "=\n");
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		while (tok != NULL && i < 3) {
			conf->norm_scales[i] = atof(tok);
			tok = strtok(NULL, ",");
			i++;
		};
	} else if (!strcmp(str, "conf_thresh")) {
		tok = strtok(NULL, "=\n");
		conf->conf_thresh = atof(tok);
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
	} else if (!strcmp(str, "filter_cls")) {
		int tmp;
		tok = strtok(NULL, "=\n");
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		conf->filter_cls.data = (int *)malloc(sizeof(int) * LITE_MODEL_CLASS_MAX);
		while (tok != NULL) {
			tmp = atoi(tok);
			if (tmp == -1 && i > LITE_MODEL_CLASS_MAX) {
				conf->filter_cls.size = 0;
				free(conf->filter_cls.data);
				conf->filter_cls.data = 0;
				return 1;
			}
			conf->filter_cls.data[i] = atoi(tok);
			tok = strtok(NULL, ",");
			i++;
		}
		memmove(conf->filter_cls.data, conf->filter_cls.data, sizeof(int) * i);
		conf->filter_cls.size = i;

	} else if (!strcmp(str, "filter_out_cls")) {
		int tmp;
		tok = strtok(NULL, "=\n");
		if (tok != NULL)
			tok = strtok(tok, ",");
		else
			return 0;
		int i = 0;
		conf->filter_out_cls.data = (int *)malloc(sizeof(int) * LITE_MODEL_CLASS_MAX);
		while (tok != NULL) {
			tmp = atoi(tok);
			if (tmp == -1 && i > LITE_MODEL_CLASS_MAX) {
				conf->filter_out_cls.size = 0;
				free(conf->filter_out_cls.data);
				conf->filter_out_cls.data = 0;
				return 1;
			}
			conf->filter_out_cls.data[i] = atoi(tok);
			tok = strtok(NULL, ",");
			i++;
		}
		memmove(conf->filter_out_cls.data, conf->filter_out_cls.data, sizeof(int) * i);
		conf->filter_out_cls.size = i;
	} else {
		hit = 0;
	}
	return hit;
}

int ParseParam(char *str, LiteModelInfo *config)
{
	int hit = 0;
	char *tok = strtok(str, "=");

	while (tok != NULL) {
		hit = 0;
		if (!strncmp(tok, "#", 1)) {
			hit = 1;
			break;
		} else if (!strncmp(tok, "\n", strlen("\n"))) {
			hit = 1;
			break;
		}
		hit = ParseConfigParam(tok, config);
		if (hit == 1)
			goto next;
		else if (hit == 0) {
			printf("Unknown parameter: %s\n", tok);
			hit = 1;
			break;
		} else if (hit < 0) {
			if (!strncmp(tok, "\n", strlen("\n"))) {
				hit = 1;
				break;
			} else {
				printf("Unknown parameter: %s\n", tok);
				break;
			}
		}
	next:
		tok = strtok(NULL, "=");
	}
	return hit;
}
int ParseModelConfig(const char *model_config_path, LiteModelInfo *config)
{
	FILE *fp = fopen(model_config_path, "r");
	if (!fp) {
		lite_err("Cannot open config file");
		return -1;
	}
	char str[256];
	while (fgets(str, 256, fp) != NULL) {
		int ret = ParseParam(str, config);
		/* Stop parsing when a unknown parameter found */
		if (!ret) {
			printf("Parsing parameter file failed.\n");
			break;
		}
	}
	fclose(fp);
	return 0;
}

void ReleaseIntList(LiteIntList *list)
{
	if (list->size) {
		free(list->data);
		list->size = 0;
		list->data = 0;
	}
}

void ReleaseStrList(LiteStrList *list)
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

void ReleaseConfig(LiteModelInfo *config)
{
	ReleaseStrList(&config->labels);
	ReleaseIntList(&config->filter_cls);
	ReleaseIntList(&config->filter_out_cls);
}

static void CropResizeBilinear8U(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy, int ex,
                                 int ey, uint8_t *dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	ex = Clamp(ex, 1, (int)src_w - 1);
	ey = Clamp(ey, 1, (int)src_h - 1);
	sx = Clamp(sx, 0, ex - 1);
	sy = Clamp(sy, 0, ey - 1);

	const float crop_h = ey - sy + 1;
	const float crop_w = ex - sx + 1;
	const float scaleY = (float)(crop_h - 1) / (dst_h - 1);
	const float scaleX = (float)(crop_w - 1) / (dst_w - 1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y) {
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x) {
			// Real-valued and discrete width coordinates in input image.
			const float ix = (float)x * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = Min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = Min(y0 + 1, (uint32_t)ey - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					PutDataU(dst, dst_c, dst_w, x, y, c, l);
				}
			} else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					PutDataU(dst, dst_c, dst_w, x, y, c, l);
				}
			}
		}
	}
}

static void CropResizeBilinear8S(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy, int ex,
                                 int ey, int8_t *dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	ex = Clamp(ex, 1, (int)src_w - 1);
	ey = Clamp(ey, 1, (int)src_h - 1);
	sx = Clamp(sx, 0, ex - 1);
	sy = Clamp(sy, 0, ey - 1);

	const float crop_h = ey - sy + 1;
	const float crop_w = ex - sx + 1;
	const float scaleY = (float)(crop_h - 1) / (dst_h - 1);
	const float scaleX = (float)(crop_w - 1) / (dst_w - 1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y) {
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x) {
			// Real-valued and discrete width coordinates in input image.
			const float ix = (float)x * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = Min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = Min(y0 + 1, (uint32_t)ey - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					PutDataS(dst, dst_c, dst_w, x, y, c, l);
				}
			} else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					PutDataS(dst, dst_c, dst_w, x, y, c, l);
				}
			}
		}
	}
}

void ImcropResize(const LiteImage *src, int sx, int sy, int ex, int ey, LiteImage *dst, int w, int h)
{
	if (src->data == NULL) {
		lite_warn("Input src is NULL!\n");
		return;
	}
	if (src->c == 1 || src->c == 3) {
	} else {
		lite_warn("Src Input channel not supported! (%d)\n", src->c);
		return;
	}
	if (src->c != dst->c) {
		if (src->c == 1 && dst->c == 3) {
		} else {
			lite_warn("Input channel not equal to dst channel!\n");
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * src->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				lite_warn("Cannot resize/chn dst const image!\n");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * src->c);
	}

	dst->w = w;
	dst->h = h;
	dst->dtype = GetImageType(dst->dtype, dst->c);
	if ((dst->dtype & 0b111) == Lite8U) {
		CropResizeBilinear8U((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
		                     (uint8_t *)dst->data, w, h, dst->c);
	} else {
		CropResizeBilinear8S((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
		                     (int8_t *)dst->data, w, h, dst->c);
	}

	return;
}

static void CropPadResizeBilinear8U(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy,
                                    int ex, int ey, int ptop, int pbottom, int pleft, int pright, uint8_t *dst,
                                    size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w - 1);
	ey = Clamp(ey, 1, (int)src_h - 1);
	sx = Clamp(sx, 0, ex - 1);
	sy = Clamp(sy, 0, ey - 1);

	//const float crop_h = ey - sy + 1;
	//const float crop_w = ex - sx + 1;

	const float pad_h = ey - sy + 1 + ptop + pbottom;
	const float pad_w = ex - sx + 1 + pleft + pright;
	const float scaleY = (float)(pad_h - 1) / (dst_h - 1);
	const float scaleX = (float)(pad_w - 1) / (dst_w - 1);

	const uint32_t src_box_w = ex - sx + 1;
	const uint32_t src_box_h = ey - sy + 1;
	const uint32_t dst_box_w = ((src_box_w - 1) / scaleX) + 1;
	const uint32_t dst_box_h = ((src_box_h - 1) / scaleY) + 1;

	const float dst_sy = ptop / scaleY;
	const float dst_sx = pleft / scaleX;

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	const int dst_size = dst_w * sizeof(uint8_t) * dst_c * dst_h;

	memset(dst, 0, dst_size);

	for (uint32_t y = 0; y < dst_box_h; ++y) {
		// pad top
		const uint32_t dst_py = y + dst_sy;

		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_box_w; ++x) {
			// pad left
			uint32_t dst_px = x + dst_sx;

			// Real-valued and discrete width coordinates in input image.
			const float ix = (float)(x)*scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = Min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = Min(y0 + 1, (uint32_t)ey - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					PutDataU(dst, dst_c, dst_w, dst_px, dst_py, c, l);
				}
			} else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					PutDataU(dst, dst_c, dst_w, dst_px, dst_py, c, l);
				}
			}
		}
	}
}

static void CropPadResizeBilinear8S(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy,
                                    int ex, int ey, int ptop, int pbottom, int pleft, int pright, int8_t *dst,
                                    size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w - 1);
	ey = Clamp(ey, 1, (int)src_h - 1);
	sx = Clamp(sx, 0, ex - 1);
	sy = Clamp(sy, 0, ey - 1);

	//const float crop_h = ey - sy + 1;
	//const float crop_w = ex - sx + 1;
#define PADDING_VALUE (-128)

	const float pad_h = ey - sy + 1 + ptop + pbottom;
	const float pad_w = ex - sx + 1 + pleft + pright;
	const float scaleY = (float)(pad_h - 1) / (dst_h - 1);
	const float scaleX = (float)(pad_w - 1) / (dst_w - 1);

	const uint32_t src_box_w = ex - sx + 1;
	const uint32_t src_box_h = ey - sy + 1;
	const uint32_t dst_box_w = ((src_box_w - 1) / scaleX) + 1;
	const uint32_t dst_box_h = ((src_box_h - 1) / scaleY) + 1;

	const float dst_sy = ptop / scaleY;
	const float dst_sx = pleft / scaleX;

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	const int dst_size = dst_w * sizeof(uint8_t) * dst_c * dst_h;

	memset(dst, PADDING_VALUE, dst_size);

	for (uint32_t y = 0; y < dst_box_h; ++y) {
		// pad top
		const uint32_t dst_py = y + dst_sy;

		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_box_w; ++x) {
			// pad left
			uint32_t dst_px = x + dst_sx;

			// Real-valued and discrete width coordinates in input image.
			const float ix = (float)(x)*scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = Min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = Min(y0 + 1, (uint32_t)ey - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					PutDataS(dst, dst_c, dst_w, dst_px, dst_py, c, l);
				}
			} else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					PutDataS(dst, dst_c, dst_w, dst_px, dst_py, c, l);
				}
			}
		}
	}
}

void ImcropResizeAspectRatio(const LiteImage *src, int sx, int sy, int ex, int ey, LiteImage *dst, int w, int h)
{
	if (src->data == NULL) {
		lite_warn("Input src is NULL!\n");
		return;
	}
	if (src->c == 1 || src->c == 3) {
	} else {
		lite_warn("Src Input channel not supported! (%d)\n", src->c);
		return;
	}
	if (src->c != dst->c) {
		if (src->c == 1 && dst->c == 3) {
		} else {
			lite_warn("Input channel not equal to dst channel!\n");
			return;
		}
	}
	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * src->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				lite_warn("Cannot resize/chn dst const image!\n");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * src->c);
	}

	dst->w = w;
	dst->h = h;
	dst->dtype = GetImageType(dst->dtype, dst->c);

	int ptop = 0, pbottom = 0, pleft = 0, pright = 0;
	int padding = 0;

	int sw = ex - sx + 1;
	int sh = ey - sy + 1;

	if (sw > sh) {
		padding = (sw - sh + 1) / 2;
		ptop = padding;
		pbottom = padding;
	} else if (sw < sh) {
		padding = (sh - sw + 1) / 2;
		pleft = padding;
		pright = padding;
	}

	if (padding == 0) {
		if ((dst->dtype & 0b111) == Lite8U) {
			CropResizeBilinear8U((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
			                     (uint8_t *)dst->data, w, h, dst->c);
		} else {
			CropResizeBilinear8S((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
			                     (int8_t *)dst->data, w, h, dst->c);
		}
	} else {
		if ((dst->dtype & 0b111) == Lite8U) {
			CropPadResizeBilinear8U((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
			                        ptop, pbottom, pleft, pright, (uint8_t *)dst->data, w, h, dst->c);
		} else {
			CropPadResizeBilinear8S((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
			                        ptop, pbottom, pleft, pright, (int8_t *)dst->data, w, h, dst->c);
		}
	}
	return;
}

void ImsaveBin(const char *name, const LiteImage *p_img)
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

int Lite_Imread(const char *name, LiteImage *img, int c)
{
#ifdef USE_STB
	if (img.data) {
		lite_warn("dst addr should be null");
		return 0;
	}

	img->data = stbi_load(name, &img->w, &img->h, &img->c, c);
	img->buf_owner = 1;

	if (img->data == nullptr) {
		lite_warn("Cannot load image from %s\n", name);
	}
#else
	lite_warn("Does not support STB in this build!\n");
#endif
	return 0;
}

unsigned char *LoadModelData(const char *model_path, int *model_data_len)
{
	FILE *fp = fopen(model_path, "rb");
	if (!fp) {
		lite_warn("Cannot open %s!\n", model_path);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	*model_data_len = ftell(fp);
	rewind(fp);
	uint8_t *model_data = malloc(sizeof(uint8_t) * *model_data_len);
	int ret = fread(model_data, 1, *model_data_len, fp);
	if (ret != *model_data_len) {
		lite_warn("fread error, size does not match!\n");
		free(model_data);
		model_data = 0;
		*model_data_len = 0;
	}
	fclose(fp);
	return model_data;
}

void ImsavePgm(const char *name, const LiteImage *p_img)
{
	FILE *fp = fopen(name, "wb");
	fprintf(fp, "P5\n");
	fprintf(fp, "%d %d\n", p_img->w, p_img->h);
	fprintf(fp, "255\n");
	if (p_img->dtype == Lite8U) {
		fwrite(p_img->data, 1, p_img->w * p_img->h, fp);
	} else if (p_img->dtype == Lite8S) {
		uint8_t *img = (uint8_t *)malloc(p_img->w * p_img->h);
		for (int i = 0; i < p_img->h; i++) {
			int row = i * p_img->w;
			for (int j = 0; j < p_img->w; j++) {
				img[row + j] = p_img->data[row + j] + 128;
			}
		}
		fwrite(img, 1, p_img->w * p_img->h, fp);
		free(img);
	}
	fclose(fp);
	return;
}

int SetupDebugTool(char *prefix)
{
	char *pre = getenv("LITE_CAP_PREFIX");
	char pos_path[256] = {};
	char neg_path[256] = {};
	if (pre) {
		struct stat st = { 0 };
		strncpy(prefix, pre, 256);
		if (stat(prefix, &st) == -1) {
			mkdir(prefix, 0700);
			strncpy(pos_path, pre, 256);
			strncpy(neg_path, pre, 256);
			strcat(pos_path, "/pos");
			strcat(neg_path, "/neg");
			mkdir(pos_path, 0700);
			mkdir(neg_path, 0700);
		}
		printf("[INFO] LITE_CAP_PREFIX is detected: \"%s\"!\n", prefix);
		return 1;
	}
	return 0;
}