#include <tuple>


//#ifndef USE_OPENCV
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
//#endif /* USE_OPENCV */

#include "eaif_common.h"
#include "eaif_image.h"
#include "eaif_trc.h"

static inline float Lerp(float a, float b, float w)
{
	return w * b + (1.f - w) * a;
}

static inline void PutDataF(float* data,
					uint32_t channel,
					uint32_t width,
					uint32_t x,
					uint32_t y,
					uint32_t c,
					float value)
{
	data[(channel * ((y * width) + x)) + c] = value;
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

static inline std::tuple<uint8_t, uint8_t, uint8_t> GetPixelAs3Channels(const uint8_t* data, uint32_t w, uint32_t h, uint32_t x, uint32_t y)
{
	uint32_t idx = (y * w + x) * 3;
	return std::make_tuple(data[idx], data[idx+1], data[idx+2]);
}

static inline void GetPixelU(const uint8_t *src, uint32_t w, uint32_t h, uint32_t x, uint32_t y, int channel,
                             uint8_t *dst)
{
	int idx = (y * w + x) * channel;
	memcpy(dst, &src[idx], sizeof(uint8_t) * channel);
}

namespace eaif {
namespace image {

#ifdef USE_OPENCV

template<>
cv::Mat Imread<cv::Mat>(const char* filename)
{
	cv::Mat img = cv::imread(filename, cv::IMREAD_COLOR);
	if (img.data == nullptr) {
		eaif_warn("Cannot read file %s\n", filename);
	}
	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	return img;
}

template<>
cv::Mat Imread(const char* img_file, int w, int h, int channel)
{
	cv::Mat dst;
	cv::Mat img;
	if (channel == 3) {
		img = cv::imread(img_file, cv::IMREAD_COLOR);
		cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	} else if (channel == 1)
		img = cv::imread(img_file, cv::IMREAD_GRAYSCALE);
	else // try to get raw info from image
		img = cv::imread(img_file, cv::IMREAD_UNCHANGED);

	cv::resize(img, dst, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
	return dst;
}

template<>
void Imread(const char* img_file, cv::Mat& img)
{
	img = cv::imread(img_file, cv::IMREAD_COLOR);
	cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
}

template<>
void Imread(const char* img_file, cv::Mat& img, int w, int h)
{
	cv::Mat _img = cv::imread(img_file, cv::IMREAD_COLOR);
	cv::cvtColor(_img, _img, cv::COLOR_BGR2RGB);
	cv::resize(_img, img, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
}

template<>
void Imdecode(const uint8_t* buf, size_t buffer_size, int imtype, cv::Mat& img, int channel)
{
	//https://stackoverflow.com/questions/4271489/how-to-use-cvimdecode-if-the-contents-of-an-image-file-are-in-a-char-array
	std::vector<uint8_t> bufmat(buf, buf + buffer_size);

	if (channel == 3) {
		img = cv::imdecode(
		cv::Mat(1, buffer_size, CV_8UC1, const_cast<uint8_t*>(buf)), //bufmat,
		cv::IMREAD_COLOR);
		cv::cvtColor(img, img, cv::COLOR_BGR2RGB);
	} else if (channel == 1) {
		img = cv::imdecode(
		cv::Mat(1, buffer_size, CV_8UC1, const_cast<uint8_t*>(buf)), //bufmat,
		cv::IMREAD_GRAYSCALE);
	} else { // try to get raw info from image (may not be rgb)
		img = cv::imdecode(
		cv::Mat(1, buffer_size, CV_8UC1, const_cast<uint8_t*>(buf)), //bufmat,
		cv::IMREAD_UNCHANGED);
	}

	return;
}


template<>
void Imresize(const cv::Mat& src, cv::Mat& dst, int w, int h)
{
	cv::resize(src, dst, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
}

template<>
void Imwrite(const char* name, cv::Mat& img)  
{
	cv::imwrite(name, img);
	return;
}

template<>
cv::Mat Imcrop(const cv::Mat& img, int sx, int sy, int ex, int ey)
{
	return img(cv::Range(sy, ey), cv::Range(sx, ex));
}

template<>
int ImcopyMakeBorder(cv::Mat& src, cv::Mat& dst, int pad_top, int pad_bottom, int pad_left, int pad_right)
{
	/* constant for border */
	if (!pad_top && !pad_bottom && !pad_left && !pad_right) {
		dst = src.clone();
		return 0;
	}
	cv::copyMakeBorder(src, dst, pad_top, pad_bottom, pad_left, pad_right, cv::BORDER_CONSTANT, cv::Scalar(0));
	return 0;
}

#if 1
// testing
inline void cv_convertTo(const cv::Mat& src, cv::Mat& dst, const float zero[3], const float scale[3])
{
	int h = src.rows;
	int w = src.cols;
	const uint8_t *sdata = src.data;
	int chn_size = w * h;
	float* ddata = reinterpret_cast<float*>(dst.data);
	for (int i = 0; i < chn_size; i++) {
		ddata[0] = scale[0] * (sdata[0] - zero[0]);
		ddata[1] = scale[1] * (sdata[1] - zero[1]);
		ddata[2] = scale[2] * (sdata[2] - zero[2]);
		sdata += 3;
		ddata += 3;
	}
}

inline void cv_convertToCHW(const cv::Mat& src, cv::Mat& dst, const float zero[3], const float scale[3])
{
	int h = src.rows;
	int w = src.cols;
	const uint8_t *sdata = src.data;
	float* ddata = reinterpret_cast<float*>(dst.data);
	int d_chn_size = h * w;
	int d_chn_size2 = d_chn_size + d_chn_size;

	for (int i = 0; i < d_chn_size; i++) {
		*ddata = scale[0] * (sdata[0] - zero[0]);
		*(ddata + d_chn_size) = scale[1] * (sdata[1] - zero[1]);
		*(ddata + d_chn_size2) = scale[2] * (sdata[2] - zero[2]);
		sdata += 3;
		ddata ++;
	}
}

#endif

template<>
void ImresizeNorm(const cv::Mat& src, cv::Mat& dst, int w, int h, const float zero[3], const float scale[3])
{
	cv::Mat tmp;
	cv::resize(src, tmp, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
	//tmp.convertTo(dst, CV_32F, scale[0], (float)-zero[0]/127.5f);
	cv_convertTo(tmp, dst, zero, scale);
}

template<>
void ImcropResizeNorm(const cv::Mat &img, int sx, int sy, int ex, int ey,
						cv::Mat &dst, int w, int h, const float zero[3], const float scale[3])
{
	cv::Mat crop = img(cv::Range(sy, ey), cv::Range(sx, ex));
	cv::Mat tmp;
	cv::resize(crop, tmp, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
	cv_convertTo(tmp, dst, zero, scale);
}

template<>
void ImcropResize(const cv::Mat &img, int sx, int sy, int ex, int ey
					cv::Mat &dst, int w, int h);
{
	cv::Mat crop = img(cv::Range(sy, ey), cv::Range(sx, ex));
	cv::resize(crop, dst, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);

}

template<>
void ImresizeNormFlip(const cv::Mat& src, cv::Mat& dst, int w, int h, const float zero[3], const float scale[3], int flip)
{
//#define STR(x) #x
//#pragma message("do not use " STR(imresizeNormFlip) "!")
	// cv:: out = scale * (in) + offset vs
	// tf:: out = (in - offset) * scale
	// TBD
	cv::Mat tmp, c_hw;
	cv::resize(src, tmp, cv::Size(w, h), 0, 0, cv::INTER_LINEAR);
	cv_convertToCHW(tmp, dst, zero, scale);
}

template<>
void ImresizeNormAspectRatio(const cv::Mat& src, cv::Mat& dst, int w, int h, const float zero[3], const float scale[3])
{
	TRC_NOT_IMPL();
	return;
}

template<>
void ImbroadcastChannel(cv::Mat& src)
{
	cv::cvtColor(src, src, cv::COLOR_GRAY2RGB);
}

template<>
void ImcvtGray(const cv::Mat& img, cv::Mat &dst)
{
	cv::cvtColor(img, dst, cv::COLOR_RGB2GRAY);
}

template<>
void ImsaveBmp(const char *name, const cv::Mat& img)
{
	cv::imwrite(name, img);
}
#endif

/********************** STB image *****************************/

//int Image::img_idx = 0;

inline void ImageCopy(const Image& src, Image& dst)
{
	dst.w = src.w;
	dst.h = src.h;
	dst.const_ptr = 0;
	dst.c = src.c;
	dst.dtype = src.dtype;
	int dsize = GetDSize(src.dtype);
	int image_size = dsize * src.w * src.h * src.c;
	dst.data = malloc(image_size);
	memcpy(dst.data, src.data, image_size);
}

template<>
Image Imread<Image>(const char* filename)
{
	Image img;
	img.data = stbi_load(filename, &img.w, &img.h, &img.c, IMG_DESIRED_CHANNEL);
	if (img.data == nullptr) {
		eaif_warn("Cannot read file %s\n", filename);
	}
	return img;
}

template<>
Image Imread(const char* img_file, int w, int h, int channel)
{
	Image src;
	Image dst;
	dst.w = w;
	dst.h = h;
	src.data = stbi_load(img_file, &src.w, &src.h, &src.c, channel);
	dst.c = src.c;
	stbir_resize_uint8((const uint8_t*)src.data , src.w , src.h , 0,
						(uint8_t*)dst.data, w, h, 0, src.c);

	if (channel == 1) dst.dtype = Eaif8UC1;
	else if (channel == 3) dst.dtype = Eaif8UC3;
	else if (channel == 4) dst.dtype = Eaif8UC4;
	else dst.dtype = EaifUnknownType;

	return dst;
}


template<>
void Imread(const char* img_file, Image& img)
{
	assert(img.data == nullptr);

	img.data = stbi_load(img_file, &img.w, &img.h, &img.c, IMG_DESIRED_CHANNEL);
	img.const_ptr = 0;

	if (img.data == nullptr) {
		eaif_warn("Cannot load image from %s\n", img_file);
		throw std::runtime_error(std::string(stbi_failure_reason()));
	}
}

template<>
void Imdecode(const uint8_t* buf, size_t buffer_size, int imtype, Image& img, int channel)
{
	// unsigned char *stbi_load_from_memory(stbi_uc const *buffer, int len, int *x, int *y, int *comp, int req_comp)
	// force decode image channel number
	assert(img.data == nullptr);
	img.data = stbi_load_from_memory(buf, buffer_size, &img.w, &img.h, &img.c, channel);
	img.c = channel;
	img.const_ptr = 0;

	if (channel == 1) img.dtype = Eaif8UC1;
	else if (channel == 3) img.dtype = Eaif8UC3;
	else if (channel == 4) img.dtype = Eaif8UC4;
	else img.dtype = EaifUnknownType;
	return;
}

static void ResizeBilinear8U(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
								uint8_t* dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	const float scaleY = (float)(src_h-1) / (dst_h-1);
	const float scaleX = (float)(src_w-1) / (dst_w-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y)
	{
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x)
		{
			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)src_w - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)src_h - 1u);

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

static void CropResizeBilinear8U(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
								int sx, int sy, int ex, int ey,
								uint8_t* dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w-1);
	ey = Clamp(ey, 1, (int)src_h-1);
	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	const float crop_h = ey - sy + 1;
	const float crop_w = ex - sx + 1;
	const float scaleY = (float)(crop_h-1) / (dst_h-1);
	const float scaleX = (float)(crop_w-1) / (dst_w-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y)
	{
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x)
		{
			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)ey - 1u);

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

static void CropPadResizeBilinear8U(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
								int sx, int sy, int ex, int ey, int ptop, int pbottom, int pleft, int pright,
								uint8_t* dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w-1);
	ey = Clamp(ey, 1, (int)src_h-1);
	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	//const float crop_h = ey - sy + 1;
	//const float crop_w = ex - sx + 1;

	const float pad_h = ey - sy + 1 + ptop + pbottom;
	const float pad_w = ex - sx + 1 + pleft + pright;
	const float scaleY = (float)(pad_h-1) / (dst_h-1);
	const float scaleX = (float)(pad_w-1) / (dst_w-1);

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

	for (uint32_t y = 0; y < dst_box_h; ++y)
	{
		// pad top
		const uint32_t dst_py = y + dst_sy;

		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_box_w; ++x)
		{
			// pad left
			uint32_t dst_px = x + dst_sx;

			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)ey - 1u);

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

static void ResizeBilinear8S(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
							 int8_t* dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	const float scaleY = (float)(src_h-1) / (dst_h-1);
	const float scaleX = (float)(src_w-1) / (dst_w-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y)
	{
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x)
		{
			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)src_w - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)src_h - 1u);

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

static void CropResizeBilinear8S(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
									int sx, int sy, int ex, int ey,
									int8_t* dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w-1);
	ey = Clamp(ey, 1, (int)src_h-1);
	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	const float crop_h = ey - sy + 1;
	const float crop_w = ex - sx + 1;
	const float scaleY = (float)(crop_h-1) / (dst_h-1);
	const float scaleX = (float)(crop_w-1) / (dst_w-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y)
	{
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x)
		{
			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)ey - 1u);

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

static void CropPadResizeBilinear8S(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
								int sx, int sy, int ex, int ey, int ptop, int pbottom, int pleft, int pright,
								int8_t* dst, size_t dst_w, size_t dst_h, size_t dst_c)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w-1);
	ey = Clamp(ey, 1, (int)src_h-1);
	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	//const float crop_h = ey - sy + 1;
	//const float crop_w = ex - sx + 1;

	const float pad_h = ey - sy + 1 + ptop + pbottom;
	const float pad_w = ex - sx + 1 + pleft + pright;
	const float scaleY = (float)(pad_h-1) / (dst_h-1);
	const float scaleX = (float)(pad_w-1) / (dst_w-1);

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

	for (uint32_t y = 0; y < dst_box_h; ++y)
	{
		// pad top
		const uint32_t dst_py = y + dst_sy;

		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_box_w; ++x)
		{
			// pad left
			uint32_t dst_px = x + dst_sx;

			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)ey - 1u);

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

template<>
void Imresize(const Image& src, Image& dst, int w, int h)
{
	assert(src.data != nullptr);
	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h) {
				free(dst.data);
				dst.w = w;
				dst.h = h;
				dst.c = src.c;
				dst.data = (uint8_t*)malloc(sizeof(uint8_t) * w * h * src.c);
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot resize dst const image!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.w = w;
		dst.h = h;
		dst.c = src.c;
		dst.dtype = src.dtype;
		dst.data = (uint8_t*)malloc(sizeof(uint8_t) * w * h * src.c);
	}
	if ((dst.dtype & 0b111) == Eaif8U) {
		ResizeBilinear8U((const uint8_t*)src.data , src.w , src.h, src.c,
						(uint8_t*)dst.data, w, h, dst.c);
	} else if ((dst.dtype & 0b111) == Eaif8S) {
		ResizeBilinear8S((const uint8_t*)src.data , src.w , src.h, src.c,
						(int8_t*)dst.data, w, h, dst.c);
	}
	return;
}

template<>
void Imread(const char* img_file, Image& img, int w, int h)
{
	Image src;
	//if (img.data != nullptr)
	//	free(img.data)
	src.data = stbi_load(img_file, &src.w, &src.h, &src.c, IMG_DESIRED_CHANNEL);
	Imresize(src, img, w, h);
	//stbir_resize_uint8((const uint8_t*)src.data , src.w , src.h , 0,
	//                    (uint8_t*)img.data, w, h, 0, src.c);
}

template<>
void Imwrite(const char* filename, Image& img)
{
	int quality = 80;
	stbi_write_jpg(filename, img.w, img.h, img.c, img.data, quality);
	return;
}

template<>
Image Imcrop(const Image& img, int sx, int sy, int ex, int ey)
{
	const int iw = img.w;
	const int ih = img.h;
	const int ic = img.c;

	ex = Clamp(ex, 1, iw-1);
	ey = Clamp(ey, 1, ih-1);

	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	const size_t dw = ex - sx + 1;
	const size_t dh = ey - sy + 1;

	Image dst(dh, dw, ic, img.dtype);

	// TODO. make it compatible to 16F datatype
	const size_t dsize = GetDSize(img.dtype);

	const size_t copy_size = dw * ic * dsize;
	const size_t src_increment = iw * ic * dsize;
	const size_t dst_increment = dw * ic * dsize;

	size_t src_offset = sy * iw * ic * dsize + sx * ic * dsize;
	size_t dst_offset = 0;

	uint8_t* src_addr = (uint8_t*) img.data;
	uint8_t* dst_addr = (uint8_t*) dst.data;
	for (size_t h = 0; h < dh; ++h) {
		//printf("src[w:%d h:%d] h:%d/%d dst:%d src:%d cpsize:%d\n", iw, img.h, h,dh-1,dst_offset,src_offset, copy_size);
		memcpy(&dst_addr[dst_offset], &src_addr[src_offset], copy_size);
		src_offset += src_increment;
		dst_offset += dst_increment;
	}

	return dst;
}

template<>
int ImcopyMakeBorder(Image& src, Image& dst, int pad_top, int pad_bottom, int pad_left, int pad_right)
{
	/* constant for border */
	assert(src.data != nullptr);
	const int sw = src.w;
	const int sh = src.h;
	const int sc = src.c;
	const int dw = sw + pad_left + pad_right;
	const int dh = sh + pad_top + pad_right;
	const int flag_same_image = src.data == dst.data;
	const size_t dsize = GetDSize(src.dtype);
	const int src_increment = sw * sc * dsize;
	const int dst_increment = dw * sc * dsize;

	uint8_t* src_addr = (uint8_t*) src.data;

	size_t dst_size = dw * dh * sc * dsize;
	size_t src_offset, dst_offset;
	size_t copy_row_size = sw * sc * dsize;

	if (dst.data == nullptr) {
		dst.data = (uint8_t*) calloc(dst_size, 1);
	} else if (!dst.const_ptr && (dst.w != dw || dst.h != dh)){
		if (!flag_same_image)
			free(dst.data);
		dst.data = (uint8_t*) calloc(dst_size, 1);
	} else if (dst.w != dw || dst.h != dh) {
		eaif_err("Cannot make border as dst image is const ptr!\n");
		return -1;
	} else { // no padding
		return 0;
	}

	dst.w = dw;
	dst.h = dh;
	dst.c = sc;

	src_offset = 0;
	dst_offset = pad_top * dw * sc * dsize + pad_left * sc * dsize;
	
	uint8_t* dst_addr = (uint8_t*) dst.data;
	for (int h = 0; h < sh ; ++h) {
		//DEB("srcdim:[%d %d] to [%d %d] %d/%d src:%d dst:%d cpysize:%d same:%d", sh, sw, dh, dw, h, sh, src_offset, dst_offset, copy_row_size, flag_same_image);
		memcpy(&dst_addr[dst_offset], &src_addr[src_offset], copy_row_size);
		dst_offset += dst_increment;
		src_offset += src_increment;
	}
	if (flag_same_image)
		free(src_addr);

	return 0;
}


static void ResizeBilinearAndNormalize(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
									float* dst, size_t dst_w, size_t dst_h, size_t dst_c,
									const float zero[3], const float scale[3], int flip = 0)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	const float scaleY = (float)(src_h-1) / (dst_h-1);
	const float scaleX = (float)(src_w-1) / (dst_w-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	if (dst_c == 1)
		flip = 0;

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	if (flip) {
		for (uint32_t y = 0; y < dst_h; ++y)
		{
			// Corresponding real-valued height coordinate in input image.
			const float iy = y * scaleY;

			// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
			const float fiy = floorf(iy);
			uint32_t y0 = (uint32_t)fiy;

			// Interpolation weight (range [0,1])
			const float yw = iy - fiy;

			for (uint32_t x = 0; x < dst_w; ++x)
			{
				// Real-valued and discrete width coordinates in input image.
				const float ix = static_cast<float>(x) * scaleX;
				const float fix = floorf(ix);
				uint32_t x0 = (uint32_t)fix;
				// Interpolation weight (range [0,1]).
				const float xw = ix - fix;

				// Discrete width/height coordinates of texels below and to the right of (x0, y0).
				const uint32_t x1 = std::min(x0 + 1, (uint32_t)src_w - 1u);
				const uint32_t y1 = std::min(y0 + 1, (uint32_t)src_h - 1u);

				GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
				GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
				GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
				GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

				if (is_broadcast) {
					const float ly0 = Lerp(float(rgb_x0y0[0]), float(rgb_x1y0[0]), xw);
					const float ly1 = Lerp(float(rgb_x0y1[0]), float(rgb_x1y1[0]), xw);
					const float l   = Lerp(ly0, ly1, yw);
					for (uint32_t c = 0; c < dst_c; ++c)
						PutDataF(dst, dst_c, dst_w, dst_w - x - 1, y, c, ((l - zero[c]) * scale[c]));
				} else {
					for (uint32_t c = 0; c < dst_c; ++c) {
						const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
						const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
						const float l = Lerp(ly0, ly1, yw);
						PutDataF(dst, dst_c, dst_w, dst_w - x - 1, y, c, ((l - zero[c]) * scale[c]));
					}
				}
			}
		}
	} else {
		for (uint32_t y = 0; y < dst_h; ++y)
		{
			// Corresponding real-valued height coordinate in input image.
			const float iy = y * scaleY;

			// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
			const float fiy = floorf(iy);
			uint32_t y0 = (uint32_t)fiy;

			// Interpolation weight (range [0,1])
			const float yw = iy - fiy;

			for (uint32_t x = 0; x < dst_w; ++x)
			{
				// Real-valued and discrete width coordinates in input image.
				const float ix = static_cast<float>(x) * scaleX;
				const float fix = floorf(ix);
				uint32_t x0 = (uint32_t)fix;
				// Interpolation weight (range [0,1]).
				const float xw = ix - fix;

				// Discrete width/height coordinates of texels below and to the right of (x0, y0).
				const uint32_t x1 = std::min(x0 + 1, (uint32_t)src_w - 1u);
				const uint32_t y1 = std::min(y0 + 1, (uint32_t)src_h - 1u);

				GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
				GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
				GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
				GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

				if (is_broadcast) {
					const float ly0 = Lerp(float(rgb_x0y0[0]), float(rgb_x1y0[0]), xw);
					const float ly1 = Lerp(float(rgb_x0y1[0]), float(rgb_x1y1[0]), xw);
					const float l   = Lerp(ly0, ly1, yw);
					for (uint32_t c = 0; c < dst_c; ++c)
						PutDataF(dst, dst_c, dst_w, x, y, c, ((l - zero[c]) * scale[c]));
				} else {
					for (uint32_t c = 0; c < dst_c; ++c)
					{
						const float ly0 = Lerp(float(rgb_x0y0[c]), float(rgb_x1y0[c]), xw);
						const float ly1 = Lerp(float(rgb_x0y1[c]), float(rgb_x1y1[c]), xw);
						const float l   = Lerp(ly0, ly1, yw);
						PutDataF(dst, dst_c, dst_w, x, y, c, ((l - zero[c]) * scale[c]));
					}
				}
			}
		}
	}
}

static void ResizeBilinearAndNormalizeAR(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
											 float* dst, size_t dst_w, size_t dst_h, size_t dst_c,
											 const float zero[3], const float scale[3], int flip = 0)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	const float aspect_ratio = (float)(src_w-1) / (src_h-1);
	const float fit_w = (src_w * dst_h) > (dst_w * src_h) ? 1 : ((src_w * dst_h) == (dst_w * src_h)) ? 2 : 0;
	float rescaleX, rescaleY;
	uint32_t start_x, start_y, end_x, end_y;

	if (fit_w==1) {
		rescaleX = dst_w;
		rescaleY = dst_w / aspect_ratio;
		start_x = 0;
		start_y = (dst_h - (int)(rescaleY + 0.5))* 0.5;
		end_x = dst_w;
		end_y = start_y + int(rescaleY + 0.5);
	} else if (fit_w == 0) {
		rescaleY = dst_h;
		rescaleX = dst_h * aspect_ratio;
		start_y = 0;
		start_x = (dst_w - (int)(rescaleX + 0.5)) * 0.5;
		end_y = dst_h;
		end_x = start_x + (int)(rescaleX + 0.5);
	} else { // (fit_w == 2)
		ResizeBilinearAndNormalize(src, src_w, src_h, src_c,
			dst, dst_w, dst_h, dst_c, zero, scale, flip);
		return;
	}

	const float scaleY = (float)(src_h-1) / (rescaleY-1);
	const float scaleX = (float)(src_w-1) / (rescaleX-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	const int dst_size = dst_w * sizeof(float) * 3 * dst_h;

	memset(dst, 0, dst_size);

	if (!flip) {
		for (uint32_t y = start_y; y < end_y; ++y)
		{
			// Corresponding real-valued height coordinate in input image.
			float iy = (y - start_y) * scaleY;

			// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
			float fiy = floorf(iy);
			uint32_t y0 = (uint32_t)fiy;

			// Interpolation weight (range [0,1])
			float yw = iy - fiy;

			for (uint32_t x = start_x; x < end_x; ++x)
			{
				// Real-valued and discrete width coordinates in input image.
				const float ix = static_cast<float>(x - start_x) * scaleX;
				const float fix = floorf(ix);
				uint32_t x0 = (uint32_t)fix;
				// Interpolation weight (range [0,1]).
				const float xw = ix - fix;

				// Discrete width/height coordinates of texels below and to the right of (x0, y0).
				const uint32_t x1 = std::min(x0 + 1, (uint32_t)src_w - 1u);
				const uint32_t y1 = std::min(y0 + 1, (uint32_t)src_h - 1u);

				GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
				GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
				GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
				GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

				for (uint32_t c = 0; c < dst_c; ++c)
				{
					const float ly0 = Lerp(float(rgb_x0y0[c]), float(rgb_x1y0[c]), xw);
					const float ly1 = Lerp(float(rgb_x0y1[c]), float(rgb_x1y1[c]), xw);
					const float l   = Lerp(ly0, ly1, yw);
					PutDataF(dst, dst_c, dst_w, x, y, c, ((l - zero[c]) * scale[c]));
				}
			}
		}
	} else {
		for (uint32_t y = start_y; y < end_y; ++y)
		{
			// Corresponding real-valued height coordinate in input image.
			float iy = (y - start_y) * scaleY;

			// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
			float fiy = floorf(iy);
			uint32_t y0 = (uint32_t)fiy;

			// Interpolation weight (range [0,1])
			float yw = iy - fiy;

			for (uint32_t x = start_x; x < end_x; ++x)
			{
				// Real-valued and discrete width coordinates in input image.
				const float ix = static_cast<float>(x - start_x) * scaleX;
				const float fix = floorf(ix);
				uint32_t x0 = (uint32_t)fix;
				// Interpolation weight (range [0,1]).
				const float xw = ix - fix;

				// Discrete width/height coordinates of texels below and to the right of (x0, y0).
				const uint32_t x1 = std::min(x0 + 1, (uint32_t)src_w - 1u);
				const uint32_t y1 = std::min(y0 + 1, (uint32_t)src_h - 1u);

				GetPixelU(src, src_w, src_h, x0, y0, dst_c, rgb_x0y0);
				GetPixelU(src, src_w, src_h, x1, y0, dst_c, rgb_x1y0);
				GetPixelU(src, src_w, src_h, x0, y1, dst_c, rgb_x0y1);
				GetPixelU(src, src_w, src_h, x1, y1, dst_c, rgb_x1y1);

				for (uint32_t c = 0; c < dst_c; ++c)
				{
					const float ly0 = Lerp(float(rgb_x0y0[c]), float(rgb_x1y0[c]), xw);
					const float ly1 = Lerp(float(rgb_x0y1[c]), float(rgb_x1y1[c]), xw);
					const float l   = Lerp(ly0, ly1, yw);
					PutDataF(dst, dst_c, dst_w, dst_w - x - 1, y, c, ((l - zero[c]) * scale[c]));
				}
			}
		}
	}
}

static void CropResizeBilinearAndNormalize(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
	int sx, int sy, int ex, int ey, float* dst, size_t dst_w, size_t dst_h, size_t dst_c,
	const float zero[3], const float scale[3], int flip = 0)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong

	ex = Clamp(ex, 1, (int)src_w-1);
	ey = Clamp(ey, 1, (int)src_h-1);
	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	const float crop_h = ey - sy + 1;
	const float crop_w = ex - sx + 1;
	const float scaleY = (float)(crop_h-1) / (dst_h-1);
	const float scaleX = (float)(crop_w-1) / (dst_w-1);

	uint8_t rgb_x0y0[3] = {};
	uint8_t rgb_x1y0[3] = {};
	uint8_t rgb_x0y1[3] = {};
	uint8_t rgb_x1y1[3] = {};

	int is_broadcast = (src_c == 1) && (dst_c == 3);

	for (uint32_t y = 0; y < dst_h; ++y)
	{
		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_w; ++x)
		{
			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)ey - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					PutDataF(dst, dst_c, dst_w, x, y, c, ((l - zero[c]) * scale[c]));
				}
			} else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					PutDataF(dst, dst_c, dst_w, x, y, c, ((l - zero[c]) * scale[c]));
				}
			}
		}
	}
}

static void CropPadResizeBilinear32F(const uint8_t* src, size_t src_w, size_t src_h, size_t src_c,
	int sx, int sy, int ex, int ey, int ptop, int pbottom, int pleft, int pright,
	float* dst, size_t dst_w, size_t dst_h, size_t dst_c,
	const float zero[3], const float scale[3], int flip = 0)
{
	// How much to scale pixel coordinates in the output image to get the corresponding pixel coordinates
	// in the input image.
	// https://stackoverflow.com/questions/43598373/opencv-resize-result-is-wrong
	ex = Clamp(ex, 1, (int)src_w-1);
	ey = Clamp(ey, 1, (int)src_h-1);
	sx = Clamp(sx, 0, ex-1);
	sy = Clamp(sy, 0, ey-1);

	//const float crop_h = ey - sy + 1;
	//const float crop_w = ex - sx + 1;

	const float pad_h = ey - sy + 1 + ptop + pbottom;
	const float pad_w = ex - sx + 1 + pleft + pright;
	const float scaleY = (float)(pad_h-1) / (dst_h-1);
	const float scaleX = (float)(pad_w-1) / (dst_w-1);

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

	const int dst_size = dst_w * sizeof(float) * dst_c * dst_h;

	memset(dst, 0, dst_size);

	for (uint32_t y = 0; y < dst_box_h; ++y)
	{
		// pad top
		const uint32_t dst_py = y + dst_sy;

		// Corresponding real-valued height coordinate in input image.
		const float iy = y * scaleY + sy;

		// Discrete height coordinate of top-left texel (in the 2x2 texel area used for interpolation).
		const float fiy = floorf(iy);
		uint32_t y0 = (uint32_t)fiy;

		// Interpolation weight (range [0,1])
		const float yw = iy - fiy;

		for (uint32_t x = 0; x < dst_box_w; ++x)
		{
			// pad left
			uint32_t dst_px = x + dst_sx;

			// Real-valued and discrete width coordinates in input image.
			const float ix = static_cast<float>(x) * scaleX + sx;
			const float fix = floorf(ix);
			uint32_t x0 = (uint32_t)fix;
			// Interpolation weight (range [0,1]).
			const float xw = ix - fix;

			// Discrete width/height coordinates of texels below and to the right of (x0, y0).
			const uint32_t x1 = std::min(x0 + 1, (uint32_t)ex - 1u);
			const uint32_t y1 = std::min(y0 + 1, (uint32_t)ey - 1u);

			GetPixelU(src, src_w, src_h, x0, y0, src_c, rgb_x0y0);
			GetPixelU(src, src_w, src_h, x1, y0, src_c, rgb_x1y0);
			GetPixelU(src, src_w, src_h, x0, y1, src_c, rgb_x0y1);
			GetPixelU(src, src_w, src_h, x1, y1, src_c, rgb_x1y1);

			if (is_broadcast) {
				const float ly0 = Lerp((float)rgb_x0y0[0], (float)rgb_x1y0[0], xw);
				const float ly1 = Lerp((float)rgb_x0y1[0], (float)rgb_x1y1[0], xw);
				const float l = Lerp(ly0, ly1, yw);
				for (uint32_t c = 0; c < dst_c; ++c) {
					PutDataF(dst, dst_c, dst_w, dst_px, dst_py, c, ((l - zero[c]) * scale[c]));
				}
			} else {
				for (uint32_t c = 0; c < dst_c; ++c) {
					const float ly0 = Lerp((float)rgb_x0y0[c], (float)rgb_x1y0[c], xw);
					const float ly1 = Lerp((float)rgb_x0y1[c], (float)rgb_x1y1[c], xw);
					const float l = Lerp(ly0, ly1, yw);
					PutDataF(dst, dst_c, dst_w, dst_px, dst_py, c, ((l - zero[c]) * scale[c]));
				}
			}
		}
	}
}

// TODO. Add single channel
template<>
void ImresizeNorm(const Image& src, Image& dst, int w, int h, const float zero[3], const float scale[3])
{
	assert(src.data != nullptr);
	assert(src.c == 3 || src.c == 1);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h || dst.c != src.c) {
				free(dst.data);
				dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
				dst.dtype = eaif_GetImageType(Eaif32F, src.c);
				dst.w = w;
				dst.h = h;
				dst.c = src.c;
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot modify const dst image dimension!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.w = w;
		dst.h = h;
		dst.c = src.c;
		dst.dtype = eaif_GetImageType(Eaif32F, src.c);
		dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
	}
	ResizeBilinearAndNormalize((const uint8_t*)src.data, src.w , src.h, src.c,
						(float*)dst.data, w, h, dst.c, zero, scale);

}


template<>
void ImresizeNormFlip(const Image& src, Image& dst, int w, int h, const float zero[3], const float scale[3], int flip)
{
	assert(src.data != nullptr);
	assert(src.c == 3 || src.c == 1);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h || dst.c != src.c) {
				free(dst.data);
				dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
				dst.dtype = eaif_GetImageType(Eaif32F, src.c);
				dst.w = w;
				dst.h = h;
				dst.c = src.c;
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot modify const dst image dimension!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.w = w;
		dst.h = h;
		dst.c = src.c;
		dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
		dst.dtype = eaif_GetImageType(Eaif32F, src.c);
	}
	ResizeBilinearAndNormalize((const uint8_t*)src.data , src.w , src.h, src.c,
						(float*)dst.data, w, h, dst.c, zero, scale, flip);
}

template<>
void ImcropResizeNorm(const Image &src, int sx, int sy, int ex, int ey,
						Image &dst, int w, int h, const float zero[3], const float scale[3])
{
	assert(src.data != nullptr);
	assert(src.c == 3 || src.c == 1);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h || dst.c != src.c) {
				free(dst.data);
				dst.c = src.c;
				dst.w = w;
				dst.h = h;
				dst.dtype = eaif_GetImageType(Eaif32F, src.c);
				dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot modify const dst image dimension!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.c = src.c;
		dst.w = w;
		dst.h = h;
		dst.dtype = eaif_GetImageType(Eaif32F, dst.c);
		dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
	}
	CropResizeBilinearAndNormalize((const uint8_t*)src.data, src.w , src.h, src.c,
						sx, sy, ex, ey,	(float*)dst.data, w, h, dst.c, zero, scale);
}

template<>
void ImcropResize(const Image &src, int sx, int sy, int ex, int ey,
					Image &dst, int w, int h)
{
	assert(src.data != nullptr);
	assert(src.c == 3 || src.c == 1);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h) {
				free(dst.data);
				dst.w = w;
				dst.h = h;
				dst.c = src.c;
				dst.dtype = eaif_GetImageType(Eaif8U, src.c);
				dst.data = (uint8_t*)malloc(sizeof(uint8_t) * w * h * src.c);
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot resize/chn dst const image!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.w = w;
		dst.h = h;
		dst.c = src.c;
		dst.dtype = eaif_GetImageType(Eaif8U, src.c);
		dst.data = (uint8_t*)malloc(sizeof(uint8_t) * w * h * src.c);
	}
	if ((dst.dtype & 0b111) == Eaif8U) {
		CropResizeBilinear8U((const uint8_t*)src.data , src.w , src.h, src.c,
						sx, sy, ex, ey, (uint8_t*)dst.data, w, h, dst.c);
	} else {
		CropResizeBilinear8S((const uint8_t*)src.data , src.w , src.h, src.c,
						sx, sy, ex, ey, (int8_t*)dst.data, w, h, dst.c);
	}
	return;
}

// TODO adopt different channel
template<>
void ImresizeNormAspectRatio(const Image& src, Image& dst, int w, int h, const float zero[3], const float scale[3])
{
	assert(src.c == 3);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h) {
				free(dst.data);
				dst.w = w;
				dst.h = h;
				dst.c = src.c;
				dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
				dst.dtype = Eaif32FC3;
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot resize dst const image!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.w = w;
		dst.h = h;
		dst.c = src.c;
		dst.data = (float*)malloc(sizeof(float) * w * h * dst.c);
		dst.dtype = eaif_GetImageType(Eaif32F, src.c);
	}
	ResizeBilinearAndNormalizeAR((const uint8_t*)src.data , src.w , src.h, src.c,
						(float*)dst.data, w, h, dst.c, zero, scale);
}

template<>
void ImcropPadResize(const Image &src, int sx, int sy, int ex, int ey,
					int ptop, int pbottom, int pleft, int pright,
					Image &dst, int w, int h)
{
	assert(src.data != nullptr);
	assert(src.c == 3 || src.c == 1);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h) {
				free(dst.data);
				dst.w = w;
				dst.h = h;
				dst.c = src.c;
				dst.dtype = eaif_GetImageType(Eaif8U, src.c);
				dst.data = (uint8_t*)malloc(sizeof(uint8_t) * w * h * src.c);
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot resize/chn dst const image!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.w = w;
		dst.h = h;
		dst.c = src.c;
		dst.dtype = eaif_GetImageType(Eaif8U, src.c);
		dst.data = (uint8_t*)malloc(sizeof(uint8_t) * w * h * src.c);
	}
	if ((dst.dtype & 0b111) == Eaif8U) {
		CropPadResizeBilinear8U((const uint8_t*)src.data , src.w , src.h, src.c,
						sx, sy, ex, ey, ptop, pbottom, pleft, pright, (uint8_t*)dst.data, w, h, dst.c);
	} else {
		CropPadResizeBilinear8S((const uint8_t*)src.data , src.w , src.h, src.c,
						sx, sy, ex, ey, ptop, pbottom, pleft, pright, (int8_t*)dst.data, w, h, dst.c);
	}
	return;
}


template<>
void ImcropPadResizeNorm(const Image &src, int sx, int sy, int ex, int ey,
					int ptop, int pbottom, int pleft, int pright,
					Image &dst, int w, int h, const float zero[3], const float scale[3])
{
	assert(src.data != nullptr);
	assert(src.c == 3 || src.c == 1);

	if (dst.data != nullptr) {
		if (!dst.const_ptr) {
			if (dst.w != w || dst.h != h || dst.c != src.c) {
				free(dst.data);
				dst.c = src.c;
				dst.w = w;
				dst.h = h;
				dst.dtype = eaif_GetImageType(Eaif32F, src.c);
				dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
			}
		} else {
			if (dst.w != w || dst.h != h) {
				eaif_err("Cannot modify const dst image dimension!\n");
				return;
			}
		}
	} else { // dst.data == nullptr
		dst.c = src.c;
		dst.w = w;
		dst.h = h;
		dst.dtype = eaif_GetImageType(Eaif32F, dst.c);
		dst.data = (float*)malloc(sizeof(float) * w * h * src.c);
	}

	CropPadResizeBilinear32F((const uint8_t*)src.data, src.w , src.h, src.c,
						sx, sy, ex, ey,	ptop, pbottom, pleft, pright,
						(float*)dst.data, w, h, dst.c, zero, scale);
	return;
}

template<>
void ImbroadcastChannel(Image& src)
{
	assert(src.data != nullptr);
	assert(!src.const_ptr);
	assert(src.c == 1);

	const int w = src.w;
	const int h = src.h;
	// FIXME support other dtype
	const size_t data_size = src.dtype == Eaif32FC3 ? sizeof(float) : sizeof(char);
	const size_t data_size2 = data_size + data_size;
	const size_t dst_size = src.w * src.h * 3 * data_size;
	const int c = 3;

	uint8_t *dst_data = (uint8_t*)malloc(dst_size);
	uint8_t *dst_addr = dst_data, *src_addr = (uint8_t*)src.data ;
	int src_offset, dst_offset, src_index_offset, dst_index_offset;
	src.c = c;

	for (int y = 0; y < h; ++y) {
		src_offset = y * w;
		dst_offset = y * w * c;
		for (int x = 0; x < w; ++x) {
			src_index_offset = (src_offset + x) * data_size;
			dst_index_offset = (dst_offset + x * c) * data_size;
			src_addr = (uint8_t*)(src.data) + src_index_offset;
			dst_addr = (uint8_t*)(dst_data) + dst_index_offset;
			memcpy(dst_addr, src_addr, data_size);
			memcpy(dst_addr + data_size, src_addr, data_size);
			memcpy(dst_addr + data_size2, src_addr, data_size);
		}
	}
	free(src.data);
	src.dtype = EAIF_DTYPE_TO_C3(src.dtype);
	src.data = dst_data;
}

template <>
void ImcvtGray(const Image &img, Image &dst)
{
	assert(img.data != nullptr);
	eaif_check(!dst.const_ptr);

	if (dst.data) {
		free(dst.data);
		dst.data = nullptr;
	}

	if(img.c == 1) {
		ImageCopy(img, dst);
		return;
	}

	dst.data = stbi__convert_format_preserve((const unsigned char *)img.data, img.c, 1, img.w, img.h);
	dst.c = 1;
	dst.w = img.w;
	dst.h = img.h;
	dst.dtype = EAIF_DTYPE_TO_C1(img.dtype);
}
//#endif

void ImsaveBin(const char* name, void* p_img, int size)
{
	FILE *fp = fopen(name, "wb");
	fwrite(p_img, 1, size, fp);
	fclose(fp);
	return;
}

void ImsaveBin(const char* name, const void* p_img)
{
	FILE *fp = fopen(name, "wb");
	const Image* p = (const Image*) p_img;
	fwrite(&p->dtype, 1, 4, fp);
	fwrite(&p->w, 1, 4, fp);
	fwrite(&p->h, 1, 4, fp);
	fwrite(&p->c, 1, 4, fp);
	fwrite(p->data, 1, p->w * p->h * p->c * GetDSize(p->dtype), fp);
	fclose(fp);
	return;
}

template<>
void ImsaveBmp(const char *name, const Image& img)
{
// TODO implement bmp
#if 0
	eaif_check(EAIF_DTYPE(img.dtype) < 2);

	struct header_{
		char os[2]; // "BM"
		int32_t file_size;
		char reserved[4];
		int32_t start_byte;
	};

	struct dib_header_info_ { //BITMAPINFOHEADER
		int32_t size; // 40
		int32_t width;
		int32_t height;
		int16_t color_planes; // 1
		int16_t bits_per_pixel; // 24
		int32_t compress_method; //0 for BI RGB
		int32_t img_data_size; //
		int32_t hori_resoln; // 3780 ... from pc ex.
		int32_t vert_resoln; // 3780 ... from pc ex.
		int32_t num_color_palette; // 0 no palette
		int32_t num_important_color; //0 ignore
	};

	struct bmp_format {
		header_ header;
		dib_header_info_ dib;
	};

	FILE *fp = fopen(name, "wb");
	assert(fp);
	struct bmp_format bmp = {
		{ {'B','M'}, 14+40, {}, 54 },
		{ 40, 0, 0, 1, 24, 0, 0, 3780, 3780, 0, 0}
	};

	bmp.dib.width = img.w;
	bmp.dib.height = img.h;
	const char dummp = '\0';

	int pad = (img.w * img.c) & 3;
	int linesize = img.w * img.c;
	int linesize_with_pad = linesize + pad;

	bmp.dib.img_data_size = img.h * linesize_with_pad;
	bmp.header.file_size += bmp.dib.img_data_size;
	fwrite(bmp.header.os, 1, 2, fp);
	fwrite(&bmp.header.file_size, 1, 4, fp);
	fwrite(bmp.header.reserved, 1, 4, fp);
	fwrite(&bmp.header.start_byte, 1, 4, fp);
	fwrite(&bmp.dib, 1, 40, fp);
	for (int i = img.w * img.h * img.c - 1 - linesize; i >= 0; i-=linesize) { // down to top
		fwrite((uint8_t*)img.data+i, 1, linesize, fp);
		for (int j = 0; j < pad; j++)
			fwrite(&dummp, 1, 1, fp);
	}
	fclose(fp);
#endif
	ImsaveBin(name, &img);
}

} // utils
} // eaif
