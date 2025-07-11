#include "eaif_image.h"
#include "eaif_c_image.h"

using namespace eaif::image;

EAIF_Image *eaif_createImage(void)
{
	return (EAIF_Image*)(new WImage);
}

EAIF_Image *eaif_createImageDim(int w, int h)
{
	return (EAIF_Image*)(new WImage(w, h, 3));
}

EAIF_Image *eaif_createImageDtype(int w, int h, int c, int dtype)
{
	return (EAIF_Image*)(new WImage(w, h, c, dtype));
}

EAIF_Image *eaif_createImageBuffer(int w, int h, int c, int dtype, unsigned char *data)
{
	return (EAIF_Image*)(new WImage(w, h, c, dtype, data));
}

void eaif_destroyImage(EAIF_Image **img)
{
	if (!img)
		return;

	delete (WImage*)(*img);
	*img = nullptr;
}

void eaif_imageGetInfo(EAIF_Image *img, int *h, int *w, int *c, unsigned char **data)
{
	WImage *dimg = (WImage*)img;
	*h = dimg->h;
	*w = dimg->w;
	*c = dimg->channels();
	*data = (unsigned char*)dimg->data;
}

EAIF_Image *eaif_ImreadPtr(const char *filename)
{
	WImage *img = new WImage;
	Imread(filename, *img);
	return (EAIF_Image *)img;
}

EAIF_Image *eaif_ImreadShape(const char *filename, int w, int h, int channel)
{
	WImage *img = new WImage;
	*img = Imread<WImage>(filename, w, h, channel);
	return (EAIF_Image *)img;	
}

void eaif_Imread(const char *filename, EAIF_Image *img)
{
	Imread(filename, *(WImage*)img);
	return;
}

void eaif_Imdecode(const unsigned char *buf, size_t buffer_size, int imtype, EAIF_Image *img, int channel)
{
	Imdecode(buf, buffer_size, imtype, *(WImage*)img, channel);
	return;
}

void eaif_Imresize(const EAIF_Image *src, EAIF_Image *dst, int w, int h)
{
	Imresize(*(WImage*)src, *(WImage*)dst, w, h);
	return;
}

void eaif_ImresizeNorm(const EAIF_Image *src, EAIF_Image *dst, int w, int h, const float zeros[3], const float scales[3])
{
	ImresizeNorm(*(WImage*)src, *(WImage*)dst, w, h, zeros, scales);
	return;
}
