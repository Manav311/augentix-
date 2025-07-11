#include <assert.h>
#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "inf_image.h"
#include "inf_types.h"

#include "inf_log.h"
#include "inf_utils.h"

#ifdef USE_STB
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
#endif // USE_STB

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
	inf_log_warn("Unknown datatype %d!", e_type);
	return sizeof(uint8_t);
}

int GetImageTypeChn(int dtype)
{
	return (((dtype >> 3) & 0x3) + 1);
}

int GetImageType(int dtype, uint32_t chn)
{
	if (chn == 2 || chn > 3) {
		inf_log_warn("Unknown arg (dtype: %d, chn: %d). Replaced as Inf32FC3!", dtype, chn);
		return Inf32FC3;
	}

	return (dtype & 0x7) | ((chn - 1) << 3);
}

static inline int Min(int a, int b)
{
	return (a > b) ? b : a;
}

static inline float Lerp(float a, float b, float w)
{
	return w * b + (1.f - w) * a;
}

// https://stackoverflow.com/questions/12037028/yuv-to-rgb-for-libvpx-webm
#define BT601 // Y range [0-255]
#ifdef Y709
static const float RGB2GRAY[3] = {0.183, 0.614, 0.062};
static const float RGB2GRAY_CONST = 16;
#endif // Y709

// https://github.com/opencv/opencv/blob/4.x/modules/imgproc/src/opencl/color_rgb.cl#L84
#ifdef BT601
static const float RGB2GRAY[3] = {0.114f, 0.587f, 0.299f};
static const float RGB2GRAY_CONST = 0;
#endif

static inline void GetPixelU(const uint8_t *src, uint32_t w, uint32_t h, uint32_t x, uint32_t y, int channel,
                             uint8_t *dst)
{
	int idx = (y * w + x) * channel;
	memcpy(dst, &src[idx], sizeof(uint8_t) * channel);
}

#ifdef USE_NCNN
static inline void PutDataU(uint8_t *data, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t c,
                            float value)
{
	data[y * width + x + height * width * c] = value;
}
static inline void PutDataS(int8_t *data, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t c,
                            float value)
{
    data[y * width + x + height * width * c] = value - 128;
}
static inline signed char float2int8(float v)
{
	int int32 = static_cast<int>(round(v));
	if (int32 > 127) return 127;
	if (int32 < -127) return -127;
	return (signed char)int32;
}
static inline void NormalizedAndPutDataS(int8_t *data, uint32_t width, uint32_t height, uint32_t x, uint32_t y, uint32_t c, float value,
										 const float* norm_zeros, const float* norm_scales,
										 const float int8_scale)
{
//    const float mean = 122.2f;
//    const float scale = 0.014798382f;
//    const float int8_scale_ = 60.501507f;
	const int8_t target_c = y / height + x / height / width + c; // (y * width + x + height * width * c) / (height * width)
	value = (value - norm_zeros[target_c]) * norm_scales[target_c] * int8_scale;
	data[y * width + x + height * width * c] = float2int8(value);
}
#else
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
#endif


template<typename Tbuffer, typename TputOp>
static void ResizeBilinear8X(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c,
                             Tbuffer *dst, size_t dst_w, size_t dst_h, size_t dst_c, TputOp op)
{
	const float scaleY = (float)(src_h - 1) / (dst_h - 1);
	const float scaleX = (float)(src_w - 1) / (dst_w - 1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);
	int is_togray = (src_c == 3) && (dst_c == 1);

	for (uint32_t y = 0; y < dst_h; ++y) {
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x) {
			// Real-valued and discrete width coordinates in input image.
			const float ix = (float)x * scaleX;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = Min(x0 + 1, (uint32_t)src_w - 1u);
			const uint32_t y1 = Min(y0 + 1, (uint32_t)src_h - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					op(dst, dst_c, dst_w, x, y, c, l);
				}
			}
			else if (is_togray) {
				float l = 0;
				for (uint32_t c = 0; c < src_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					l += RGB2GRAY[c] * Lerp(ly0, ly1, yw);
				}
				op(dst, 1, dst_w, x, y, 0, l + RGB2GRAY_CONST);
			}
			else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					op(dst, dst_c, dst_w, x, y, c, l);
				}
			}
		}
	}
}

void Inf_Imresize(const InfImage *src, int w, int h, InfImage *dst)
{
	if (src == NULL || src->data == NULL) {
		inf_log_err("Input args should not be NULL!");
		return;
	}

	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src Input channel not supported! (%d)", src->c);
		return;
	}

	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_err("Input channel(%d) not equal to dest channel(%d)!", src->c, dst->c);
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c ;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
	}

	dst->w = w;
	dst->h = h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);

	if ((dst->dtype & 0b111) == Inf8U) {
		ResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c,
						 (uint8_t *)dst->data, w, h, dst->c, PutDataU);
	} else if ((dst->dtype & 0b111) == Inf8S) {
		ResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c,
						 (int8_t *)dst->data, w, h, dst->c, PutDataS);
	} else {
		inf_log_warn("%s does not support floating point image resize!", __func__);
		assert(0);
	}

	return;
}

#ifdef USE_NCNN
template<typename Tbuffer, typename TputOp>
static void ResizeBilinearNorm8X(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c,
								 Tbuffer *dst, size_t dst_w, size_t dst_h, size_t dst_c, TputOp op,
								 const float* norm_zeros, const float* norm_scales,
								 const float int8_scale)
{
	const float scaleY = (float)(src_h - 1) / (dst_h - 1);
	const float scaleX = (float)(src_w - 1) / (dst_w - 1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);
	int is_togray = (src_c == 3) && (dst_c == 1);

	for (uint32_t y = 0; y < dst_h; ++y) {
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x) {
			// Real-valued and discrete width coordinates in input image.
			const float ix = (float)x * scaleX;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = Min(x0 + 1, (uint32_t)src_w - 1u);
			const uint32_t y1 = Min(y0 + 1, (uint32_t)src_h - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					op(dst, dst_w, dst_h, x, y, c, l, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
				}
			}
			else if (is_togray) {
				float l = 0;
				for (uint32_t c = 0; c < src_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					l += RGB2GRAY[c] * Lerp(ly0, ly1, yw);
				}
				op(dst, dst_w, dst_h, x, y, 0, l + RGB2GRAY_CONST, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
			}
			else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					op(dst, dst_w, dst_h, x, y, c, l, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
				}
			}
		}
	}
}

/**
 * @brief Resize source image to destination image and normalization.
 * @details Inf_ImresizeNorm resize the source image to desired dimension
 *          and normalize.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[out] dst          destination image
 * @param[in] w             resize width
 * @param[in] h             resize height
 * @param[in] norm_zeros    normalization means
 * @param[in] norm_scales   normalization stds
 * @param[in] int8_scale    quantization parameter of ncnn
 * @retval execution result
 * @see Inf_ImcropResizeAspectRatio()
 */

void Inf_ImresizeNorm(const InfImage *src, int w, int h, InfImage *dst, const float* norm_zeros, const float* norm_scales, const float int8_scale)
{
	if (src == NULL || src->data == NULL) {
		inf_log_err("Input args should not be NULL!");
		return;
	}

	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src Input channel not supported! (%d)", src->c);
		return;
	}

	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_err("Input channel(%d) not equal to dest channel(%d)!", src->c, dst->c);
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c ;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
	}

	dst->w = w;
	dst->h = h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);
    if ((dst->dtype & 0b111) == Inf8U || (dst->dtype & 0b111) == Inf8S) {
		ResizeBilinearNorm8X((const uint8_t *)src->data, src->w, src->h, src->c,
		                     (int8_t *)dst->data, w, h, dst->c, NormalizedAndPutDataS, norm_zeros, norm_scales, int8_scale);
	} else {
		inf_log_warn("%s does not support floating point image resize!", __func__);
		assert(0);
	}

	return;
}
#endif //USE_NCNN

template<typename Tbuffer, typename TputOp>
static void CropResizeBilinear8X(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy, int ex,
                                 int ey, Tbuffer *dst, size_t dst_w, size_t dst_h, size_t dst_c, TputOp op)
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
	int is_togray = (src_c == 3) && (dst_c == 1);

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
					op(dst, dst_c, dst_w, x, y, c, l);
				}
			}
			else if (is_togray) {
				float l = 0;
				for (uint32_t c = 0; c < src_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					l += RGB2GRAY[c] * Lerp(ly0, ly1, yw);
				}
				op(dst, 1, dst_w, x, y, 0, l + RGB2GRAY_CONST);
			}
			else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					op(dst, dst_c, dst_w, x, y, c, l);
				}
			}
		}
	}
}

/**
 * @brief Resize the cropped area to destination image.
 * @details Inf_ImcropResize first crop the image,
 *          zero padding on the edge based on asepect ratio and
 *          resize to desired dimension.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[out] dst          destination image
 * @param[in] w             resize width
 * @param[in] h             resize height
 * @retval execution result
 * @see Inf_ImcropResizeAspectRatio()
 */
void Inf_ImcropResize(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h)
{
	if (src == NULL || src->data == NULL) {
		inf_log_err("Input src is NULL!");
		return;
	}

	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src input channel not supported! (%d)", src->c);
		return;
	}

	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_warn("Input channel not equal to dest channel!");
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c ;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
	}

	dst->w = w;
	dst->h = h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);
	if ((dst->dtype & 0b111) == Inf8U) {
		CropResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
		                     (uint8_t *)dst->data, w, h, dst->c, PutDataU);
	} else if ((dst->dtype & 0b111) == Inf8S) {
		CropResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
		                     (int8_t *)dst->data, w, h, dst->c, PutDataS);
	} else {
		assert(0);
	}

	return;
}

#ifdef USE_NCNN
template<typename Tbuffer, typename TputOp>
static void CropResizeBilinearNorm8X(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy, int ex,
								 int ey, Tbuffer *dst, size_t dst_w, size_t dst_h, size_t dst_c, TputOp op,
								 const float* norm_zeros, const float* norm_scales,
								 const float int8_scale)
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
	int is_togray = (src_c == 3) && (dst_c == 1);

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
					op(dst, dst_w, dst_h, x, y, c, l, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
				}
			}
			else if (is_togray) {
				float l = 0;
				for (uint32_t c = 0; c < src_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					l += RGB2GRAY[c] * Lerp(ly0, ly1, yw);
				}
				op(dst, dst_w, dst_h, x, y, 0, l + RGB2GRAY_CONST, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
			}
			else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					op(dst, dst_w, dst_h, x, y, c, l, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
				}
			}
		}
	}
}

/**
 * @brief Resize the cropped area to destination image and normalization.
 * @details Inf_ImcropResizeNorm first crop the image,
 *          zero padding on the edge based on asepect ratio,
 *          resize to desired dimension and normalize.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[out] dst          destination image
 * @param[in] w             resize width
 * @param[in] h             resize height
 * @param[in] norm_zeros    normalization means
 * @param[in] norm_scales   normalization stds
 * @param[in] int8_scale    quantization parameter of ncnn
 * @retval execution result
 * @see Inf_ImcropResizeAspectRatio()
 */
void Inf_ImcropResizeNorm(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h,
					  const float* norm_zeros, const float* norm_scales, const float int8_scale)
{
	if (src == NULL || src->data == NULL) {
		inf_log_err("Input src is NULL!");
		return;
	}

	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src input channel not supported! (%d)", src->c);
		return;
	}

	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_warn("Input channel not equal to dest channel!");
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c ;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
	}

	dst->w = w;
	dst->h = h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);
	if ((dst->dtype & 0b111) == Inf8U || (dst->dtype & 0b111) == Inf8S) {
		CropResizeBilinearNorm8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
						 	 (int8_t *)dst->data, w, h, dst->c, NormalizedAndPutDataS, norm_zeros, norm_scales, int8_scale);
	} else {
		assert(0);
	}

	return;
}
#endif //USE_NCNN

#ifdef USE_NCNN
#define PADDING_VALUE (-100)
#else
#define PADDING_VALUE (-128)
#endif

template<typename Tbuffer, typename TputOp>
static void CropPadResizeBilinear8X(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy,
                                    int ex, int ey, int ptop, int pbottom, int pleft, int pright, Tbuffer *dst,
                                    size_t dst_w, size_t dst_h, size_t dst_c, int pad_value, TputOp op)
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

	const float pad_h = ey - sy + 1 + ptop + pbottom; // height of cropped & padded img
	const float pad_w = ex - sx + 1 + pleft + pright; // width of cropped & padded img
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
	int is_togray = (src_c == 3) && (dst_c == 1);
	const int dst_size = dst_w * sizeof(uint8_t) * dst_c * dst_h;

	memset(dst, pad_value, dst_size);

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
					op(dst, dst_c, dst_w, dst_px, dst_py, c, l);
				}
			}
			else if (is_togray) {
				float l = 0;
				for (uint32_t c = 0; c < src_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					l += RGB2GRAY[c] * Lerp(ly0, ly1, yw);
				}
				op(dst, 1, dst_w, dst_px, dst_py, 0, l + RGB2GRAY_CONST);
			}
			else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					op(dst, dst_c, dst_w, dst_px, dst_py, c, l);
				}
			}
		}
	}
}

/**
 * @brief Resize the cropped area with aspect ratio.
 * @details Inf_ImcropResizeAspectRatio first crop the image,
 *          zero padding on the edge based on asepect ratio and
 *          resize to desired dimension.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[out] dst          destination image
 * @param[in] w             resize width
 * @param[in] h             resize height
 * @retval execution result
 */
void Inf_ImcropResizeAspectRatio(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h)
{
	if (src == NULL || src->data == NULL) {
		inf_log_err("Input src is NULL!");
		return;
	}

	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src input channel not supported! (%d)", src->c);
		return;
	}

	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_err("Input channel not equal to dst channel!");
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
	}


	dst->w = w;
	dst->h = h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);
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
		if ((dst->dtype & 0b111) == Inf8U) {
			CropResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
			                     (uint8_t *)dst->data, w, h, dst->c, PutDataU);
		} else {
			CropResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
			                     (int8_t *)dst->data, w, h, dst->c, PutDataS);
		}
	} else {
		if ((dst->dtype & 0b111) == Inf8U) {
			CropPadResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
									ptop, pbottom, pleft, pright, (uint8_t *)dst->data, w, h, dst->c, 0, PutDataU);
		} else {
			CropPadResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
									ptop, pbottom, pleft, pright, (int8_t *)dst->data, w, h, dst->c, PADDING_VALUE, PutDataS);
		}
	}
	return;
}

/**
 * @brief image crop, pad, resize within one function
 * @details Inf_ImcropPadResize first crop the image,
 *          zero padding on the edge and resize to desired dimension.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[in] ptop          top region padding value relative to source image
 * @param[in] pbottom       pbottom region padding value relative tfInf_ImcropResizeo source image
 * @param[in] pleft         left region padding value relative to source image
 * @param[in] pright        right region padding value relative to source image
 * @param[out] dst          destination image
 * @param[in] dst_w         resize width
 * @param[in] dst_h         resize
 * @retval execution result
 */
void Inf_ImcropPadResize(const InfImage *src, int sx, int sy, int ex, int ey,
						 int ptop, int pbottom, int pleft, int pright, InfImage *dst, int dst_w, int dst_h)
{
	if (src->data == NULL) {
		inf_log_err("Input src is NULL!");
		return;
	}
	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src input channel not supported! (%d)", src->c);
		return;
	}
	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_warn("Input channel not equal to dst channel!");
			return;
		}
	}
	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != dst_w || dst->h != dst_h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * dst_w * dst_h * dst->c);
			}
		} else {
			if (dst->w != dst_w || dst->h != dst_h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * dst_w * dst_h * dst->c);
	}

	dst->w = dst_w;
	dst->h = dst_h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);

	if ((dst->dtype & 0b111) == Inf8U) {
		CropPadResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
								ptop, pbottom, pleft, pright, (uint8_t *)dst->data, dst_w, dst_h, dst->c, 0, PutDataU);
	} else if ((dst->dtype & 0b111) == Inf8S){
		CropPadResizeBilinear8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
								ptop, pbottom, pleft, pright, (int8_t *)dst->data, dst_w, dst_h, dst->c, PADDING_VALUE, PutDataS);
    } else {
		inf_log_err("%s does not support dtype %d.", __func__, dst->dtype);
		return;
	}
}

#ifdef USE_NCNN
template<typename Tbuffer, typename TputOp>
static void CropPadResizeBilinearNorm8X(const uint8_t *src, size_t src_w, size_t src_h, size_t src_c, int sx, int sy,
									int ex, int ey, int ptop, int pbottom, int pleft, int pright, Tbuffer *dst,
									size_t dst_w, size_t dst_h, size_t dst_c, int pad_value, TputOp op,
									const float* norm_zeros, const float* norm_scales,
									const float int8_scale)
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

	const float pad_h = ey - sy + 1 + ptop + pbottom; // height of cropped & padded img
	const float pad_w = ex - sx + 1 + pleft + pright; // width of cropped & padded img
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
	int is_togray = (src_c == 3) && (dst_c == 1);
	const int dst_size = dst_w * sizeof(uint8_t) * dst_c * dst_h;

	memset(dst, pad_value, dst_size);

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
					op(dst, dst_w, dst_h, dst_px, dst_py, c, l, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
				}
			}
			else if (is_togray) {
				float l = 0;
				for (uint32_t c = 0; c < src_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					l += RGB2GRAY[c] * Lerp(ly0, ly1, yw);
				}
				op(dst, dst_w, dst_h, dst_px, dst_py, 0, l + RGB2GRAY_CONST, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
			}
			else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					op(dst, dst_w, dst_h, dst_px, dst_py, c, l, norm_zeros, norm_scales, int8_scale); // NHWC(TFLite) to NCHW(NCNN)
				}
			}
		}
	}
}

/**
 * @brief Resize the cropped area with aspect ratio with normalization.
 * @details Inf_ImcropResizeNormAspectRatio first crop the image,
 *          zero padding on the edge based on asepect ratio,
 *          resize to desired dimension and normalize.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[out] dst          destination image
 * @param[in] w             resize width
 * @param[in] h             resize height
 * @param[in] norm_zeros    normalization means
 * @param[in] norm_scales   normalization stds
 * @param[in] int8_scale    quantization parameter of ncnn
 * @retval execution result
 */
void Inf_ImcropResizeNormAspectRatio(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h,
								 const float* norm_zeros, const float* norm_scales, const float int8_scale)
{
	if (src == NULL || src->data == NULL) {
		inf_log_err("Input src is NULL!");
		return;
	}

	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src input channel not supported! (%d)", src->c);
		return;
	}

	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_err("Input channel not equal to dst channel!");
			return;
		}
	}

	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != w || dst->h != h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
			}
		} else {
			if (dst->w != w || dst->h != h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * w * h * dst->c);
	}


	dst->w = w;
	dst->h = h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);
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
		CropResizeBilinearNorm8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
							     (int8_t *)dst->data, w, h, dst->c, NormalizedAndPutDataS, norm_zeros, norm_scales, int8_scale);
	} else {
		CropPadResizeBilinearNorm8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
								    ptop, pbottom, pleft, pright, (int8_t *)dst->data, w, h, dst->c, PADDING_VALUE, NormalizedAndPutDataS, norm_zeros, norm_scales, int8_scale);
	}
	return;
}

/**
 * @brief image crop, pad, resize, normalization within one function
 * @details Inf_ImcropPadResizeNorm first crop the image,
 *          zero padding on the edge,resize to desired dimension and normalize.
 *          If destination image is not buffer owner, it has to provide
 *          valid data pointer, current dtype and channel number.
 *          If destination image has null data pointer, this function
 *          copies the source datatype/ channel to destination image.
 * @param[in] src           source image to be cropped, padded, resized
 * @param[in] sx            top left point x-axis of cropping
 * @param[in] sy            top left point y-axis of cropping
 * @param[in] ex            bottom right point x-axis of cropping
 * @param[in] ey            bottom right point point y-axis of cropping
 * @param[in] ptop          top region padding value relative to source image
 * @param[in] pbottom       pbottom region padding value relative Inf_ImcropResizeo source image
 * @param[in] pleft         left region padding value relative to source image
 * @param[in] pright        right region padding value relative to source image
 * @param[out] dst          destination image
 * @param[in] dst_w         resize width
 * @param[in] dst_h         resize height
 * @param[in] norm_zeros    normalization means
 * @param[in] norm_scales   normalization stds
 * @param[in] int8_scale    quantization parameter of ncnn
 *
 * @retval execution result
 */
void Inf_ImcropPadResizeNorm(const InfImage *src, int sx, int sy, int ex, int ey,
						 int ptop, int pbottom, int pleft, int pright, InfImage *dst, int dst_w, int dst_h,
						 const float* norm_zeros, const float* norm_scales, const float int8_scale)
{
	if (src->data == NULL) {
		inf_log_err("Input src is NULL!");
		return;
	}
	if (src->c == 1 || src->c == 3) {
	} else {
		inf_log_err("Src input channel not supported! (%d)", src->c);
		return;
	}
	if (dst->data != nullptr && src->c != dst->c) {
		if ((src->c == 1 && dst->c == 3) ||
			(src->c == 3 && dst->c == 1)) {
		} else {
			inf_log_warn("Input channel not equal to dst channel!");
			return;
		}
	}
	if (dst->data != NULL) {
		if (dst->buf_owner) {
			if (dst->w != dst_w || dst->h != dst_h) {
				free(dst->data);
				dst->data = (uint8_t *)malloc(sizeof(uint8_t) * dst_w * dst_h * dst->c);
			}
		} else {
			if (dst->w != dst_w || dst->h != dst_h) {
				inf_log_warn("Cannot resize/chn dst const image!");
				return;
			}
		}
	} else { // dst.data == NULL
		dst->c = (dst->c == 0) ? src->c : dst->c;
		dst->buf_owner = 1;
		dst->data = (uint8_t *)malloc(sizeof(uint8_t) * dst_w * dst_h * dst->c);
	}

	dst->w = dst_w;
	dst->h = dst_h;
	dst->dtype = (InfDataType)GetImageType(dst->dtype, dst->c);

	if ((dst->dtype & 0b111) == Inf8U || (dst->dtype & 0b111) == Inf8S) {
	CropPadResizeBilinearNorm8X((const uint8_t *)src->data, src->w, src->h, src->c, sx, sy, ex, ey,
							ptop, pbottom, pleft, pright, (int8_t *)dst->data, dst_w, dst_h, dst->c, PADDING_VALUE, NormalizedAndPutDataS, norm_zeros, norm_scales, int8_scale);
	} else {
		inf_log_err("%s does not support dtype %d.", __func__, dst->dtype);
		return;
	}
}
#endif //USE_NCNN


static int imreadPgm(const char *img_name, InfImage *p_img)
{
	FILE *fp = fopen(img_name, "rb");
	logThenRetIf(!fp, "Cannot open pgm file!", -ENOENT);

	char fmt[32]{};
	int bitdepth = 0;
	int w = 0, h = 0;

	if (fscanf(fp, "%31s\n", fmt) !=1 ||
		fscanf(fp, "%d %d\n", &w, &h) != 2 ||
		fscanf(fp, "%d\n", &bitdepth) != 1) {
		logThenRetIf(1, "pgm header format is invalid!", -1);
	}

	logThenRetIf(strcmp(fmt,"P5"), "pgm format not supported!", -1);
	logThenRetIf(w <1 || w > 0xffff || h <1 || h > 0xffff, "pgm invalid dimension!", -1);
	logThenRetIf(bitdepth != 255, "pgm bitdepth not supported!", -1);

	p_img->w = w;
	p_img->h = h;
	p_img->c = 1;
	p_img->dtype = Inf8U;
	p_img->buf_owner = 1;

	size_t expected_size = p_img->w * p_img->h;

	p_img->data = (uint8_t *)malloc(expected_size);
	size_t size = fread(p_img->data, 1, expected_size, fp);

	int ret = 0;
	if (size != expected_size) {
		p_img->w = 0;
		p_img->h = 0;
		p_img->c = 0;
		p_img->buf_owner = 0;
		free(p_img->data);
		p_img->data = nullptr;
		ret = -EINVAL;
	}
	fclose(fp);
	return ret;
}

static int imreadPpm(const char *img_name, InfImage *p_img)
{
	FILE *fp = fopen(img_name, "rb");
	logThenRetIf(!fp, "Cannot open ppm file!", -ENOENT);
	char fmt[32]{};
	int bitdepth = 0;
	int w = 0, h = 0;

	if (fscanf(fp, "%31s\n", fmt) !=1 ||
		fscanf(fp, "%d %d\n", &w, &h) != 2 ||
		fscanf(fp, "%d\n", &bitdepth) != 1) {
		logThenRetIf(1, "pgm header format is invalid!", -1);
	}

	logThenRetIf(strcmp(fmt,"P6"), "ppm format not supported. Only P6 magic number is supported!", -1);
	logThenRetIf(w <1 || w > 0xffff || h <1 || h > 0xffff, "ppm invalid dimension!", -1);
	logThenRetIf(bitdepth != 255, "ppm bitdepth not supported!", -1);

	p_img->w = w;
	p_img->h = h;
	p_img->c = 3;
	p_img->dtype = Inf8UC3;
	p_img->buf_owner = 1;

	size_t expected_size = p_img->w * p_img->h * 3;

	p_img->data = (uint8_t *)malloc(expected_size);
	size_t size = fread(p_img->data, 1, expected_size, fp);

	int ret = 0;
	if (size != expected_size) {
		p_img->w = 0;
		p_img->h = 0;
		p_img->c = 0;
		p_img->buf_owner = 0;
		free(p_img->data);
		p_img->data = nullptr;
		ret = -EINVAL;
	}
	fclose(fp);
	return ret;
}

/**@brief image read using stb and read pgm image
 **@details for stb enabled, imread can support
 **@         ppm(P6), pgm(P5), jpeg, png, bmp image fmt.
 **@param[in] img_name      input image name.
 **@param[in/out] img       output image.
 **@param[in] channel       desire output channel number (0 means image original channel).
 **@retval -EFAULT          input args is invalid
 **@retval -EINVAL          image file data is invalid
 **@retval -ENOENT          cannot open image file
 **@retval 0                success.
 **/
extern "C" int Inf_Imread(const char *img_name, InfImage *img, int channel)
{
	retIfNull(img_name && img);

	if (img->data) {
		inf_log_err("dest addr should be null");
		return -EFAULT;
	}

	int ret = 0;

	if (strstr(img_name, "pgm")) {
		ret = imreadPgm(img_name, img);
	}
	else if (strstr(img_name, "ppm")) {
		ret = imreadPpm(img_name, img);
	}
	else {
#ifdef USE_STB
		if (!(channel == 0 || channel == 1 || channel == 3)) {
			inf_log_err("Invalid input channel!");
			ret = -EINVAL;
		}
		else {
			img->data = stbi_load(img_name, &img->w, &img->h, &img->c, channel);
			channel = (channel) ? channel : img->c;
			img->buf_owner = 1;
			img->dtype = (InfDataType)GetImageType(Inf8U, channel);
			if (img->data == nullptr) {
				inf_log_warn("Cannot load image from %s", img_name);
				ret = -EINVAL;
			}
		}
#else
		inf_log_warn("Does not support STB in this build, Currently only support PPM/ PGM read!");
#endif
	}
	return ret;
}

static int imwritePgm(const char *img_name, const InfImage *p_img)
{
	size_t dsize = GetDSize(p_img->dtype);
	int chn_no = GetImageTypeChn(p_img->dtype);
	InfDataType dtype = (InfDataType)(p_img->dtype & 0b111);

	if (dsize != sizeof(uint8_t) && chn_no != 1) {
		inf_log_err("Datatype (%s) or channel no (%d) not supported!", GetDTypeString(p_img->dtype),
			chn_no);
		return -EINVAL;
	}
	FILE *fp = fopen(img_name, "wb");
	logThenRetIf(!fp, "Cannot open file for pgm saving!", -ENOENT);

	fprintf(fp, "P5\n");
	fprintf(fp, "%d %d\n", p_img->w, p_img->h);
	fprintf(fp, "255\n");
	if (dtype == Inf8U) {
		fwrite(p_img->data, 1, p_img->w * p_img->h, fp);
	} else if (dtype == Inf8S) {
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
	return 0;
}

static int imwritePpm(const char *img_name, const InfImage *p_img)
{
	size_t dsize = GetDSize(p_img->dtype);
	int chn_no = GetImageTypeChn(p_img->dtype);
	InfDataType dtype = (InfDataType)(p_img->dtype & 0b111);

	if (dsize != sizeof(uint8_t) || chn_no != 3) {
		inf_log_err("Datatype (%s) or channel no (%d) not supported!", GetDTypeString(p_img->dtype),
			chn_no);
		return -EINVAL;
	}

	FILE *fp = fopen(img_name, "wb");
	logThenRetIf(!fp, "Cannot open file for ppm saving!", -ENOENT);
	fprintf(fp, "P6\n");
	fprintf(fp, "%d %d\n", p_img->w, p_img->h);
	fprintf(fp, "255\n");
	if (dtype == Inf8U) {
		fwrite(p_img->data, 1, p_img->w * p_img->h * chn_no, fp);
	} else if (dtype == Inf8S) {
		uint8_t *img = (uint8_t *)malloc(p_img->w * p_img->h * chn_no);
		for (int i = 0; i < p_img->h; i++) {
			int row = i * p_img->w * chn_no;
			for (int j = 0; j < p_img->w; j++) {
				int idx = j * chn_no;
				img[row + idx + 0] = p_img->data[row + idx + 0] + 128;
				img[row + idx + 1] = p_img->data[row + idx + 1] + 128;
				img[row + idx + 2] = p_img->data[row + idx + 2] + 128;
			}
		}
		fwrite(img, 1, p_img->w * p_img->h * chn_no, fp);
		free(img);
	}
	fclose(fp);
	return 0;
}


/**@brief Create empty image
 **@param[in] w         width
 **@param[in] h         height
 **@param[in] c         channel
 **@param[in] dtype         image data type
 **@retval InfImage         fail if data attr is 0
 **@retval InfImage         success if data attr is allocated
 **/
extern "C" InfImage Inf_ImcreateEmpty(int w, int h, int c, InfDataType dtype)
{
	InfImage img = {};
	if (!(w > 0 && h > 0 && w < 0xffff && h < 0xffff && (c == 1 || c == 3)))
		return img;

	img.data = (uint8_t*)calloc(1, w * h *c * GetDSize(dtype));
	img.w = w;
	img.h = h;
	img.c = c;
	img.buf_owner = 1;
	img.dtype = (InfDataType)GetImageType(dtype, c);

	return img;
}

/**@brief free InfImage
 **@param[in] p_img         image data to be freed
 **@retval -EINVAL          input image is not buffer owner, cannot free the buffer.
 **@retval 0                success.
 **/
extern "C" int Inf_Imrelease(InfImage *p_img)
{
	logThenRetIf(!p_img|| !p_img->data, "", 0);
	logThenRetIf(p_img->buf_owner!=1, "Cannot release image which is not buffer owner!", -EINVAL);

	free(p_img->data);
	p_img->data = nullptr;
	p_img->w = 0;
	p_img->h = 0;
	p_img->c = 0;
	p_img->buf_owner = 1;
	p_img->dtype = Inf8U;

	return 0;
}

/**@brief Decode jpeg buffer
 **@param[in] buf           jpeg buffer data to be decoded
 **@param[in] buf_size      jpeg buffer Size
 **@param[out] p_img        decoded dst image
 **@retval -EFAULT          input pointer cannot be null.
 **@retval -EINVAL          cannot decode image.
 **@retval 0                success.
 **/
extern "C" int Inf_ImdecodeJpeg(const uint8_t *buf, size_t buf_size, InfImage *p_img, int channel)
{
#ifdef USE_STB
	logThenRetIf(!buf || !p_img , "Input data cannot be null!", -EFAULT);
	logThenRetIf(p_img->data && p_img->buf_owner , "Input image data should be null or buf_owner!", -EFAULT);

	if (p_img->data) {
		free(p_img->data);
		p_img->data = nullptr;
	}
	p_img->data = stbi_load_from_memory(buf, buf_size, &p_img->w, &p_img->h, &p_img->c, channel);

	if (!p_img->data) {
		return -EINVAL;
	}
	p_img->c = channel;
	p_img->buf_owner = 1;
	if (channel == 0) { // jpeg always have 3 channel.
		p_img->dtype = Inf8UC3;
	}
	else if (channel == 1) p_img->dtype = Inf8UC1;
	else if (channel == 3) p_img->dtype = Inf8UC3;

#else
	inf_log_err("STB is not enabled in this built, cannot decode image!");
	return -EINVAL;
#endif
	return 0;
}

/**@brief image save using stb and read pgm image
 **@param[in] img_name      input image name.
 **@param[in] p_img         image data to be saved.
 **@retval -EINVAL          fail to write image.
 **@retval -EFAULT          input pointers are null.
 **@retval -ENOENT          cannot open file.
 **@retval 0                success.
 **/
extern "C" int Inf_Imwrite(const char *img_name, const InfImage *p_img)
{
	retIfNull(img_name && p_img && p_img->data);

	int ret = 0;

	if (strstr(img_name, "pgm")) {
		ret = imwritePgm(img_name, p_img);
	}
	else if (strstr(img_name, "ppm")) {
		ret = imwritePpm(img_name, p_img);
	}
	else if (strstr(img_name, "jpg") ||
			strstr(img_name, "jpeg")) {
#ifdef USE_STB
		int quality = 95;
		// return 0 for failure, success for non-zero
		ret = stbi_write_jpg(img_name, p_img->w, p_img->h, p_img->c, p_img->data, quality);
		ret = (ret == 0) ? -EINVAL : 0;
#else // USE_STB
		inf_log_err("STB is not enabled in this built, cannot write jpeg image!");
		ret = -EINVAL;
#endif // USE_STB
	}
	else {
		inf_log_err("Not supported image format %s.", img_name);
		ret = -EINVAL;
	}
	return ret;
}
