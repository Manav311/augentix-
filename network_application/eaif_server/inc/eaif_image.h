#ifndef EAIF_IMAGE_H_
#define EAIF_IMAGE_H_

/* This image library utils treat all image as uint8_t type */
#include "eaif_common.h"
#include "eaif_trc.h"

#ifdef USE_OPENCV
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#endif /* USE_OPENCV */

namespace eaif
{
namespace image
{
//#ifndef USE_OPENCV
class Image {
    public:
	union {
		int w;
		int cols;
	};
	union {
		int h;
		int rows;
	};
	//static int img_idx;
	//int idx;
	int c;
	int dtype;
	void *data;
	int const_ptr;

	Image()
	        : w(0)
	        , h(0)
	        , c(0)
	        , dtype(Eaif8UC3)
	        , data(nullptr)
	        , const_ptr(0){
		        //idx = img_idx++;
		        //eaif_trc("Img create:%d\n", idx);
	        };
	Image(int row, int col, int ch)
	        : w(col)
	        , h(row)
	        , c(ch)
	        , dtype(Eaif8UC3)
	        , data(nullptr)
	        , const_ptr(0)
	{
		data = (uint8_t *)malloc(w * h * c * sizeof(uint8_t));
		//idx = img_idx++;
		//eaif_trc("Img create:%d\n", idx);
	};
	Image(int row, int col, int ch, int type)
	        : w(col)
	        , h(row)
	        , c(ch)
	        , dtype(type)
	        , data(nullptr)
	        , const_ptr(0)
	{
		int size = GetDSize(dtype);
		data = (uint8_t *)malloc(w * h * c * size);
		//idx = img_idx++;
		//eaif_trc("Img create:%d\n", idx);
		return;
	};
	Image(int row, int col, void *dptr)
	        : w(col)
	        , h(row)
	        , c(3)
	        , dtype(Eaif8UC3)
	        , data(dptr)
	        , const_ptr(1)
	{
		assert(dptr != nullptr);
		//idx = img_idx++;
		//eaif_trc("Img create:%d\n", idx);
	};
	Image(int row, int col, int type, void *dptr)
	        : w(col)
	        , h(row)
	        , c(eaif_GetImageTypeChn(type))
	        , dtype(type)
	        , data(dptr)
	        , const_ptr(1)
	{
		assert(dptr != nullptr);
		//idx = img_idx++;
		//eaif_trc("Img create:%d\n", idx);
	};
	Image(int row, int col, int ch, int type, void *dptr)
	        : w(col)
	        , h(row)
	        , c(ch)
	        , dtype(type)
	        , data(dptr)
	        , const_ptr(1)
	{
		assert(dptr != nullptr);
		//idx = img_idx++;
		//eaif_trc("Img create:%d\n", idx);
	};
	~Image()
	{
		if (data != nullptr && const_ptr != 1) {
			free(data);
			data = nullptr;
			//eaif_trc("Img destroy freed:%d\n", idx);
		} else {
			//eaif_trc("Img destroy non-freed:%d\n", idx);
		}
	}
	int channels(void) const
	{
		return c;
	}
	int type(void) const
	{
		return dtype;
	}
	void set(int row, int col, int type, void *dptr)
	{
		assert(data == nullptr);
		w = col;
		h = row;
		c = eaif_GetImageTypeChn(type);
		dtype = type;
		data = dptr;
		const_ptr = 1;
	}
};

//#endif

template <typename Timage> Timage Imread(const char *filename);

template <typename Timage> Timage Imread(const char *img_file, int w, int h, int channel = IMG_DESIRED_CHANNEL);

template <typename Timage> void Imread(const char *img_file, Timage &img);

template <typename Timage> void Imread(const char *img_file, Timage &img, int w, int h);

template <typename Timage>
void Imdecode(const uint8_t *buf, size_t buffer_size, int imtype, Timage &img, int channel = IMG_DESIRED_CHANNEL);

template <typename Timage> void Imresize(const Timage &src, Timage &dst, int w, int h);

template <typename Timage> void Imwrite(const char *name, Timage &img); // accept jpg only currently

template <typename Timage> Timage Imcrop(const Timage &img, int sx, int sy, int ex, int ey);

template <typename Timage>
int ImcopyMakeBorder(Timage &src, Timage &dst, int pad_top, int pad_bottom, int pad_left, int pad_right);

template <typename Timage>
void ImresizeNorm(const Timage &src, Timage &dst, int w, int h, const float zero[3], const float scale[3]);
template <typename Timage>
void ImresizeNormFlip(const Timage &src, Timage &dst, int w, int h, const float zero[3], const float scale[3],
                      int flip = 0);
template <typename Timage>
void ImresizeNormAspectRatio(const Timage &src, Timage &dst, int w, int h, const float zero[3], const float scale[3]);

template <typename Timage>
void ImcropResizeNorm(const Timage &src, int sx, int sy, int ex, int ey, Timage &dst, int w, int h, const float zero[3],
                      const float scale[3]);

template <typename Timage>
void ImcropResize(const Timage &src, int sx, int sy, int ex, int ey, Timage &dst, int w, int h);

template <typename Timage>
void ImcropPadResize(const Timage &src, int sx, int sy, int ex, int ey, int pad_top, int pad_bottom, int pad_left,
                     int pad_right, Timage &dst, int w, int h);

template <typename Timage>
void ImcropPadResizeNorm(const Timage &src, int sx, int sy, int ex, int ey, int pad_top, int pad_bottom, int pad_left,
                         int pad_right, Timage &dst, int w, int h, const float zero[3], const float scale[3]);

template <typename Timage> void ImbroadcastChannel(Timage &src);

template <typename Timage> void ImcvtGray(const Timage &img, Timage &dst);

template <typename Timage> void ImsaveBmp(const char *name, const Timage &img);

void ImsaveBin(const char *name, void *p_img, int size);

} // namespace image
} // namespace eaif

#ifdef USE_OPENCV
using WImage = cv::Mat;
#else
using WImage = eaif::image::Image;
#endif /* USE_OPENCV */

#endif /* EAIF_IMAGE_H_ */
