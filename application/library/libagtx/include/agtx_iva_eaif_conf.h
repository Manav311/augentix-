#ifndef AGTX_IVA_EAIF_CONF_H_
#define AGTX_IVA_EAIF_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_IVA_EAIF_INF_CMD_NONE,
	AGTX_IVA_EAIF_INF_CMD_FACE_REGISTER,
	AGTX_IVA_EAIF_INF_CMD_FACE_LOAD,
	AGTX_IVA_EAIF_INF_CMD_FACE_SAVE,
	AGTX_IVA_EAIF_INF_CMD_FACE_DELETE,
	AGTX_IVA_EAIF_INF_CMD_FACE_RESET
} AGTX_IVA_EAIF_INF_CMD_E;

typedef enum {
	AGTX_IVA_EAIF_DATA_FMT_JPEG,
	AGTX_IVA_EAIF_DATA_FMT_Y,
	AGTX_IVA_EAIF_DATA_FMT_YUV,
	AGTX_IVA_EAIF_DATA_FMT_RGB,
	AGTX_IVA_EAIF_DATA_FMT_MPI_JPEG,
	AGTX_IVA_EAIF_DATA_FMT_MPI_Y,
	AGTX_IVA_EAIF_DATA_FMT_MPI_YUV,
	AGTX_IVA_EAIF_DATA_FMT_MPI_RGB
} AGTX_IVA_EAIF_DATA_FMT_E;

typedef enum {
	AGTX_IVA_EAIF_API_METHOD_FACEDET,
	AGTX_IVA_EAIF_API_METHOD_FACERECO,
	AGTX_IVA_EAIF_API_METHOD_DETECT,
	AGTX_IVA_EAIF_API_METHOD_CLASSIFY,
	AGTX_IVA_EAIF_API_METHOD_CLASSIFY_CV,
	AGTX_IVA_EAIF_API_METHOD_HUMAN_CLASSIFY
} AGTX_IVA_EAIF_API_METHOD_E;

#define MAX_AGTX_IVA_EAIF_CONF_S_CLASSIFY_CV_MODEL_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_CLASSIFY_MODEL_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_DETECT_MODEL_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_FACE_DETECT_MODEL_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_FACE_NAME_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_FACE_RECO_MODEL_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_HUMAN_CLASSIFY_MODEL_SIZE 257
#define MAX_AGTX_IVA_EAIF_CONF_S_URL_SIZE 257

typedef struct {
	AGTX_IVA_EAIF_API_METHOD_E api;
	AGTX_UINT8 classify_cv_model
	        [MAX_AGTX_IVA_EAIF_CONF_S_CLASSIFY_CV_MODEL_SIZE]; /* classify cv model name from host server */
	AGTX_UINT8
	        classify_model[MAX_AGTX_IVA_EAIF_CONF_S_CLASSIFY_MODEL_SIZE]; /* classify model name from host server */
	AGTX_IVA_EAIF_DATA_FMT_E data_fmt;
	AGTX_UINT8 detect_model[MAX_AGTX_IVA_EAIF_CONF_S_DETECT_MODEL_SIZE]; /* detect model name from host server */
	AGTX_INT32 detection_period; /* Period for object detection (# frames) */
	AGTX_INT32 enabled;
	AGTX_UINT8 face_detect_model
	        [MAX_AGTX_IVA_EAIF_CONF_S_FACE_DETECT_MODEL_SIZE]; /* face detect model name from host server */
	AGTX_UINT8 face_name[MAX_AGTX_IVA_EAIF_CONF_S_FACE_NAME_SIZE];
	AGTX_UINT8
	        face_reco_model[MAX_AGTX_IVA_EAIF_CONF_S_FACE_RECO_MODEL_SIZE]; /* face reco model name from host server */
	AGTX_INT32 facereco_roi_ex;
	AGTX_INT32 facereco_roi_ey;
	AGTX_INT32 facereco_roi_sx;
	AGTX_INT32 facereco_roi_sy;
	AGTX_UINT8 human_classify_model
	        [MAX_AGTX_IVA_EAIF_CONF_S_HUMAN_CLASSIFY_MODEL_SIZE]; /* human classification model name from host server */
	AGTX_INT32 identification_period; /* Period for identification detection (# frames) */
	AGTX_IVA_EAIF_INF_CMD_E inf_cmd; /* inference command */
	AGTX_INT32 inf_with_obj_list; /* Enable object list based model inference (for detection only) */
	AGTX_INT32 min_size; /* Minimum face size (pixel) for identification */
	AGTX_INT32 neg_classify_period; /* Period to send request for an object with negative result(# frames) */
	AGTX_INT32
	        obj_exist_classify_period; /* Period to send request for any positive object exists on the screen (# frames) */
	AGTX_INT32 obj_life_th;
	AGTX_INT32 pos_classify_period; /* Period to send request for an object with positive result(# frames) */
	AGTX_INT32
	        pos_stop_count_th; /* Threshold to stop sending request when one object accumulated same positive result */
	AGTX_INT32 snapshot_height; /* snapshot height (classification only) */
	AGTX_INT32 snapshot_width; /* snapshot width (classification only) */
	AGTX_INT32 target_idx;
	AGTX_UINT8 url[MAX_AGTX_IVA_EAIF_CONF_S_URL_SIZE];
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_EAIF_CONF_S;


#endif /* AGTX_IVA_EAIF_CONF_H_ */
