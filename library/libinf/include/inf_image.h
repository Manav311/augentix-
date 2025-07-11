#ifndef INF_IMAGE_H_
#define INF_IMAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "inf_types.h"


void Inf_Imresize(const InfImage *src, int w, int h, InfImage *dst);
void Inf_ImcropPadResize(const InfImage *src, int sx, int sy, int ex, int ey, int ptop, int pbottom, int pleft,
                         int pright, InfImage *dst, int dst_w, int dst_h);
void Inf_ImcropResize(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h);
void Inf_ImcropResizeAspectRatio(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h);

#ifdef USE_NCNN
void Inf_ImresizeNorm(const InfImage *src, int w, int h, InfImage *dst,
                      const float* norm_zeros, const float* norm_scales, const float int8_scale);
void Inf_ImcropPadResizeNorm(const InfImage *src, int sx, int sy, int ex, int ey, int ptop, int pbottom, int pleft,
                             int pright, InfImage *dst, int dst_w, int dst_h,
                             const float* norm_zeros, const float* norm_scales, const float int8_scale);
void Inf_ImcropResizeNorm(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h,
                          const float* norm_zeros, const float* norm_scales, const float int8_scale);
void Inf_ImcropResizeNormAspectRatio(const InfImage *src, int sx, int sy, int ex, int ey, InfImage *dst, int w, int h,
                                     const float* norm_zeros, const float* norm_scales, const float int8_scale);
#endif

int Inf_Imread(const char *img_name, InfImage *p_img, int channel);
int Inf_ImdecodeJpeg(const uint8_t *buf, size_t buf_size, InfImage *p_img, int channel);
int Inf_Imwrite(const char *img_name, const InfImage *p_img);
int Inf_Imrelease(InfImage *p_img);
InfImage Inf_ImcreateEmpty(int w, int h, int c, InfDataType dtype);

#ifdef __cplusplus
}
#endif

#endif /* INF_IMAGE_H_ */