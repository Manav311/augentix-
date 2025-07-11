#ifndef INF_TYPES_H_
#define INF_TYPES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include "mpi_base_types.h"
#include "mpi_iva.h"

#define INF_MODEL_CLASS_MAX (1000)
#define INF_STR_LEN (64)
#define INF_MODEL_PATH_LEN (255)
#define INF_MODEL_NAME_LEN (16)

#define INF_CLASSIFY_STR "classify"
#define INF_DETECT_STR "detect"
#define INF_FACEENCODE_STR "faceencode"
#define INF_FACERECO_STR "facereco"

typedef MPI_IVA_OBJ_LIST_S InfObjList;

/**@brief data/ image type
  *@details  bit         00       000
  *              channel-no  datatype
 **/
typedef enum { // map to cv dtype
	Inf8U = 0,
	Inf8UC1 = 0,
	Inf8S = 1,
	Inf8SC1 = 1,
	Inf32S = 4,
	Inf32SC1 = 4,
	Inf32F = 5,
	Inf32FC1 = 5,
	Inf64F = 6,
	Inf64FC1 = 6,
	Inf8UC3 = 16,
	Inf8SC3 = 17,
	Inf32FC3 = 21,
	Inf64FC3 = 22,
	Inf8UC4 = 24,
	Inf8SC4 = 25,
	Inf32SC4 = 28,
	Inf32FC4 = 29,
	InfUnknownType = 7,
} InfDataType;

/**@brief str list
 **/
typedef struct {
	int size;
	char **data;
} InfStrList;

/**@brief int list
 **/
typedef struct {
	int size;
	int *data;
} InfIntList;

/**@brief float list
 **/
typedef struct {
	int size;
	float *data;
} InfFloatList;

/**@brief classify model output type
 **/
typedef enum { InfLinear, InfSigmoid, InfSoftmax } InfOutputTypeE;

/**@brief model type
 **/
typedef enum { InfRunClassify, InfRunDetect, InfRunFaceEncode, InfRunFaceReco, InfRunNum } InfRunType;

/**@brief quantization info
 **/
typedef struct {
	int zero;
	float scale;
} QuantInfo;

/**@brief classification result
 **/
typedef struct {
	int id;
	int cls_num;
	int prob_num;
	int *cls;
	float *prob;
} InfResult;

/**@brief classification list
 **/
typedef struct {
	int size;
	InfResult *data;
} InfResultList;

/**@brief detection result
 **/
typedef struct {
	int id;
	int cls_num;
	int prob_num;
	float confidence;
	int *cls;
	float *prob;
	MPI_RECT_POINT_S rect;
} InfDetResult;

/**@brief detection list
 **/
typedef struct {
	int size;
	InfDetResult *data;
} InfDetList;

/**@brief image structure
 **/
typedef struct {
	int w; /**< width */
	int h; /**< height */
	int c; /**< number of channel */
	uint8_t *data; /**< data buffer */
	int buf_owner; /**< flag to show if image holds the buffer */
	InfDataType dtype;
} InfImage;

typedef struct {
	/* models common properties */
	char config_path[INF_MODEL_PATH_LEN]; /**< configure file path */
	char model_name[INF_MODEL_NAME_LEN]; /**< model name */
	int w; /**< model input width */
	int h; /**< model input height */
	int c; /**< model input channel */
	InfFloatList conf_thresh; /**< confidence thresh list */
	float norm_zeros[3]; /**< image normalization zero offset */
	float norm_scales[3]; /**< image normalization scales */
	float input_int8_scale;
	QuantInfo quant;
	InfRunType inference_type;
	InfDataType dtype;
	InfStrList model_paths; /**< User can specify 1 to multiple model files */
	int topk; /**< model classification topk output selection */
	int resize_aspect_ratio; /**< force aspect ratio resize input to model */
	int out_dim; /**< number of output dimension */

	/* detection model properties */
	float iou_thresh; /**< iou threshold for detection */
	InfFloatList nms_internal_thresh;

	/* face detection model parameters */
	float window_scale_factor;
	int window_min_size;

	/* face detect scrfd */
	int feature_output_pairs;
	int num_anchors_per_feature;
	int use_kps;
	int feature_stride[5];

	/* face encode parameter */
	int align_margin;

	/* model common postprocessing parameter */
	InfOutputTypeE output_type;
	InfIntList filter_cls;
	InfIntList filter_out_cls;
	InfStrList labels;

	int num_threads;
	int verbose;
	int debug;
} InfModelInfo;

struct inf_model {
	const InfModelInfo *info;
	void *model;
};

#ifdef __cplusplus
}
#endif

#endif /* INF_TYPES_H */
