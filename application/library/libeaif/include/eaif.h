#ifndef EAIF_H_
#define EAIF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_base_types.h"
#include "mpi_index.h"
#include "mpi_iva.h"

#define EAIF_INFERENCE_INAPP_STR "inapp"
#define EAIF_RUN_USLEEP (10000)
#define EAIF_CHAR_LEN (32)
#define EAIF_MODEL_LEN (256)
#define EAIF_GENERAL_STR_LEN (256)
#define EAIF_URL_CHAR_LEN (256)
#define EAIF_TIMEOUT_UNIT (8L)
#define EAIF_MIN_OBJ_LIFE_TH (0)
#define EAIF_MAX_OBJ_LIFE_TH (MPI_IVA_OD_MAX_LIFE)
#define EAIF_MAX_CLS_PER_OBJ (5)
#define EAIF_RECT_INIT (-9999)

/**
 * @brief Enum for EAIF face utils
 */
typedef enum {
	EAIF_INF_NONE,
	EAIF_INF_FACE_REGISTER,
	EAIF_INF_FACE_LOAD,
	EAIF_INF_FACE_SAVE,
	EAIF_INF_FACE_DELETE,
	EAIF_INF_FACE_RESET,
} EAIF_INF_COMMAND_E;

/**
 * @brief Enum for EAIF face cmd
	// face_dir (file_dir)
	   |-face.bin (face_db)
	   |-faces/name0.img
	   |-faces/name1.img
	   |-...
 */
typedef struct {
	EAIF_INF_COMMAND_E cmd;
	MPI_RECT_POINT_S roi;
	char dir[EAIF_GENERAL_STR_LEN];
	char face_db[EAIF_CHAR_LEN];
	char face_name[EAIF_CHAR_LEN];
} EAIF_INF_UTILS_S;

/**
 * @brief Enum for EAIF Restful service selection
 */
typedef enum {
	EAIF_API_FACEDET = 0,
	EAIF_API_FACERECO,
	EAIF_API_DETECT,
	EAIF_API_CLASSIFY,
	EAIF_API_CLASSIFY_CV,
	EAIF_API_HUMAN_CLASSIFY,
	EAIF_API_MODEL_NUM
} EAIF_API_METHOD_E;

/**
 * @brief Enum for EAIF data format
 * @details JPEG format is only available on remote mode
 *          EAIF_DATA_MPI_<fmt> differs to EAIF_DATA_<fmt>
 *          for remote mode only
 */
typedef enum {
	EAIF_DATA_JPEG,
	EAIF_DATA_Y,
	EAIF_DATA_YUV, /**< Not supported currently */
	EAIF_DATA_RGB,
	EAIF_DATA_MPI_JPEG,
	EAIF_DATA_MPI_Y,
	EAIF_DATA_MPI_YUV, /**< Not supported currently */
	EAIF_DATA_MPI_RGB,
	EAIF_DATA_FMT_NUM,
} EAIF_DATA_FMT_E;

/**
 * @brief Enum for EAIF server connection status
 */
typedef enum {
	EAIF_URL_REACHABLE,
	EAIF_URL_NONREACHABLE,
} EAIF_URL_REACHABLE_E;

/**
 * @brief Struct for edge-assisted AI framework parameter.
 */
typedef struct {
	UINT16 obj_life_th; /**< input object life threshold */
	MPI_WIN target_idx; /**< snapshot window index */
	EAIF_API_METHOD_E api; /**< request api method */
	EAIF_DATA_FMT_E data_fmt; /**< request data format */
	EAIF_INF_UTILS_S inf_utils; /**< inference utils cmd */
	char url[EAIF_URL_CHAR_LEN]; /**< request address */
	UINT32 snapshot_width; /**< request snapshot size (only for raw data, 0 for default window size) */
	UINT32 snapshot_height; /**< request snapshot size (only for raw data, 0 for default window size) */
	UINT32 pos_stop_count_th; /**< threshold to stop sending request for positive objects */
	UINT32 pos_classify_period; /**< period for positive object classification */
	UINT32 neg_classify_period; /**< period for negative object classification */
	UINT32 detection_period; /** period for detection */
	UINT32 identification_period; /** period of identification */
	UINT32 min_size; /** minimum face size to be identified (pixel) */
	UINT32 inf_with_obj_list; /** inference based on obj_list (flag for detection only)*/
	UINT32 obj_exist_classify_period; /**< period for object classification after any positive object exists*/
	union {
		struct {
			char face_detect_model[EAIF_MODEL_LEN]; /**< face recognition model name */
			char face_reco_model[EAIF_MODEL_LEN]; /**< face recognition model name */
			char detect_model[EAIF_MODEL_LEN]; /**< object detection model name */
			char classify_model[EAIF_MODEL_LEN]; /**< classification model name */
			char classify_cv_model[EAIF_MODEL_LEN]; /**< classification cv model name */
			char human_classify_model[EAIF_MODEL_LEN]; /**< human classification model name */
		};
		char api_models[EAIF_API_MODEL_NUM][EAIF_MODEL_LEN];
	};
} EAIF_PARAM_S;

/**
 * @brief Struct for EAIF object attributes.
 */
typedef struct {
	INT32 id; /**< object index */
	INT32 label_num; /**< results number */
	MPI_RECT_POINT_S rect; /**< object location */
	char category[EAIF_MAX_CLS_PER_OBJ][EAIF_CHAR_LEN]; /**< category array */
	char prob[EAIF_MAX_CLS_PER_OBJ][EAIF_CHAR_LEN]; /**< prob array */
} EAIF_OBJ_ATTR_S;

/**
 * @brief Struct for EAIF status.
 */
typedef struct {
	EAIF_URL_REACHABLE_E server_reachable; /**< EAIF-serv status */
	UINT32 timestamp; /**< result timestamp */
	UINT32 obj_cnt; /**< number of objects */
	EAIF_OBJ_ATTR_S obj_attr[MPI_IVA_MAX_OBJ_NUM]; /**< results of objects */
} EAIF_STATUS_S;

typedef struct eaif_algo_status_s EAIF_ALGO_STATUS_S;

typedef struct {
	EAIF_ALGO_STATUS_S *algo_status;
} EAIF_INSTANCE_S;

EAIF_INSTANCE_S *EAIF_newInstance(MPI_WIN idx);
int EAIF_deleteInstance(EAIF_INSTANCE_S **instance);
int EAIF_activate(EAIF_INSTANCE_S *instance);
int EAIF_deactivate(EAIF_INSTANCE_S *instance);
int EAIF_reset(EAIF_INSTANCE_S *instance);
int EAIF_checkParam(const EAIF_PARAM_S *param);
int EAIF_setParam(EAIF_INSTANCE_S *instance, const EAIF_PARAM_S *param);
int EAIF_getParam(EAIF_INSTANCE_S *instance, EAIF_PARAM_S *param);
int EAIF_testRequest(EAIF_INSTANCE_S *instance, const MPI_IVA_OBJ_LIST_S *obj_list, EAIF_STATUS_S *status);
int EAIF_testRequestV2(EAIF_INSTANCE_S *instance, const MPI_IVA_OBJ_LIST_S *obj_list, EAIF_STATUS_S *status);
int EAIF_applyFaceUtils(EAIF_INSTANCE_S *instance, const EAIF_PARAM_S *param);

#ifdef __cplusplus
}
#endif

#endif /* EAIF_H_ */