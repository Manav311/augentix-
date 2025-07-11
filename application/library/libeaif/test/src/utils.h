#ifndef EAIF_TEST_UTILS_H_
#define EAIF_TEST_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CLAMP(x, low, high)                                          \
	({                                                           \
		__typeof__(x) __x = (x);                             \
		__typeof__(low) __low = (low);                       \
		__typeof__(high) __high = (high);                    \
		__x > __high ? __high : (__x < __low ? __low : __x); \
	})

typedef void *VideoCapPtr;
typedef void *CvMatPtr;

typedef struct {
	int width;
	int height;
	float fps;
	VideoCapPtr cap;
} CvVideo;

typedef struct {
	int w;
	int h;
	int c;
	int dtype;
	void *data;
	CvMatPtr mat;
} CvImage;

CvVideo CV_utils_OpenVideo(const char *video_file);
void CV_utils_GetFrame(CvVideo *vid, CvImage *img, int channel);
void CV_utils_ResizeFrame(CvImage *img, int w, int h);
int CV_utils_EnodeJpegImage(CvImage *img, uint8_t **jpeg);
void CV_utils_dumpRectJpeg(CvImage *img, int sx, int sy, int ex, int ey, const char *path);
void CV_utils_dumpJpegRaw(const uint8_t *jpeg, int size, const char *path);
void CV_utils_dumpJpeg(CvImage *img, const char *path);
void CV_utils_dumpRaw(CvImage *img, const char *path);
void CV_utils_destroyFrame(CvImage *img);
void CV_utils_closeVideo(CvVideo *vid);
void CV_utils_ReadImage(CvImage *img, const char *path);
void CV_utils_ReadImageGray(CvImage *img, const char *path);

#ifdef __cplusplus
}
#endif

#endif /* EAIF_TEST_UTILS_H_ */
