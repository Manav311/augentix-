#ifndef LITE_UTILS_H_
#define LITE_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "lite_classifier.h"

int GetDSize(int dtype);
int GetImageTypeChn(int dtype);
int GetImageType(int dtype, uint32_t chn);
int GetOutputType(const char *output_type_str);
void PrintResult(int output_dim, float *output_buf);

int ParseModelConfig(const char *model_config_path, LiteModelInfo *config);
float QuantConvert(uint8_t in, int zero, float scale);
float QuantConvertS(int8_t in, int zero, float scale);
void Sigmoid(float *output_addr, int num_classes);
void PostProcess(const float *output_addr, int num_classes, float conf_thresh, int topk, const LiteIntList *filter_cls,
                 const LiteIntList *filter_out_cls, LiteResult *result);
void ReleaseConfig(LiteModelInfo *config);
void ReleaseIntList(LiteIntList *list);
void ReleaseStrList(LiteStrList *list);

// image utils
unsigned char *LoadModelData(const char *model_path, int *model_data_len);

void ImcropResize(const LiteImage *src, int sx, int sy, int ex, int ey, LiteImage *dst, int w, int h);
void ImcropResizeAspectRatio(const LiteImage *src, int sx, int sy, int ex, int ey, LiteImage *dst, int w, int h);

void ImsaveBin(const char *name, const LiteImage *p_img);
void ImsavePgm(const char *name, const LiteImage *p_img);

#define SNAPSHOT_FORMAT "%s/%s/%05d_%.3f.pgm"

int SetupDebugTool(char *prefix);

#ifdef __cplusplus
}
#endif

#endif /* LITE_UTILS_H_ */
