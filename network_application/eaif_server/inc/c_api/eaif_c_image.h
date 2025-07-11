#ifndef EAIF_C_IMAGE_H_
#define EAIF_C_IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eaif_image EAIF_Image;

EAIF_Image *eaif_createImage(void);
EAIF_Image *eaif_createImageDim(int w, int h);
EAIF_Image *eaif_createImageDtype(int w, int h, int c, int dtype);
EAIF_Image *eaif_createImageBuffer(int w, int h, int c, int dtype, unsigned char *data);
void eaif_destroyImage(EAIF_Image **img);
void eaif_imageGetInfo(EAIF_Image *img, int *h, int *w, int *c, unsigned char **data);

EAIF_Image *eaif_ImreadPtr(const char *filename);
EAIF_Image *eaif_ImreadShape(const char *filename, int w, int h, int channel);
void eaif_Imread(const char *filename, EAIF_Image *img);
void eaif_Imdecode(const unsigned char *buf, size_t buffer_size, int imtype, EAIF_Image *img, int channel);
void eaif_Imresize(const EAIF_Image *src, EAIF_Image *dst, int w, int h);
void eaif_ImresizeNorm(const EAIF_Image *src, EAIF_Image *dst, int w, int h, const float zeros[3],
                       const float scales[3]);

#ifdef __cplusplus
}
#endif

#endif /* EAIF_C_IMAGE_H_ */
