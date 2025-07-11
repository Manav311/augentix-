#ifndef LITE_CLASSIFIER_H_
#define LITE_CLASSIFIER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#ifdef USE_FAKE_MPI
#include "fake_mpi.h"
#else
#include "mpi_base_types.h"
#include "mpi_iva.h"
#endif

#define LITE_MODEL_CLASS_MAX (1000)

typedef enum { // map to cv dtype
	Lite8U = 0,
	Lite8UC1 = 0,
	Lite8S = 1,
	Lite8SC1 = 1,
	Lite32S = 4,
	Lite32SC1 = 4,
	Lite32F = 5,
	Lite32FC1 = 5,
	Lite64F = 6,
	Lite64FC1 = 6,
	Lite8UC3 = 16,
	Lite8SC3 = 17,
	Lite32FC3 = 21,
	Lite64FC3 = 22,
	Lite8UC4 = 24,
	Lite8SC4 = 25,
	Lite32SC4 = 28,
	Lite32FC4 = 29,
	LiteUnknownType = 7,
} LiteDataType;

#define LITE_STR_LEN (64)
typedef struct {
	int size;
	char **data;
} LiteStrList;

typedef struct {
	int size;
	int *data;
} LiteIntList;

typedef struct {
	int w;
	int h;
	int c;
	uint8_t *data;
	int buf_owner;
	LiteDataType dtype; // rgb HxWxC or Y
} LiteImage;

typedef enum { LiteLinear, LiteSigmoid, LiteSoftmax } LiteOutputTypeE;
typedef struct {
	int w;
	int h;
	int c;
	float conf_thresh;
	float norm_zeros[3];
	float norm_scales[3];
	int quant_zero;
	float quant_scale;
	LiteDataType dtype;
	char model_path[256];
	int num_thread;
	int topk;
	int resize_aspect_ratio;
	LiteOutputTypeE output_type;
	LiteIntList filter_cls;
	LiteIntList filter_out_cls;
	LiteStrList labels;
} LiteModelInfo;

typedef struct {
	const LiteModelInfo *info;
	void *model;
} LiteClassifierCtx;

typedef struct {
	int id;
	int cls_num;
	int prob_num;
	int *cls;
	float *prob;
} LiteResult;

typedef struct {
	int size;
	LiteResult *data;
} LiteResultList;

int Lite_InitModel(LiteClassifierCtx *ctx, const char *config);
int Lite_Invoke(LiteClassifierCtx *ctx, const LiteImage *img, const MPI_IVA_OBJ_LIST_S *obj_list,
                LiteResultList *result);
int Lite_ReleaseModel(LiteClassifierCtx *ctx);
int Lite_ReleaseResult(LiteResultList *result);
int Lite_Setup(LiteClassifierCtx *ctx, int verbose, int debug, int num_thread);
int Lite_Imread(const char *name, LiteImage *img, int c);

#ifdef __cplusplus
}
#endif

#endif
