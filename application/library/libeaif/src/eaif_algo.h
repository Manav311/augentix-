#ifndef EAIF_ALGO_H_
#define EAIF_ALGO_H_

#include <pthread.h>
#include <string.h>

#include "cm_iva_eaif_resp.h"

#include "eaif.h"
#include "eaif_utils.h"

//#define EAIF_ALGO_DEBUG

#define EAIF_STR_DATA "data"
#define EAIF_STR_FMT "format"
#define EAIF_STR_META "meta"
#define EAIF_STR_TIME "time"
#define EAIF_STR_SHAPE "shape"

#define EAIF_STR_API_FMT "%s/predict/%s"

#define EAIF_STR_FMT_JPG "jpg"
#define EAIF_STR_FMT_RGB "rgb"
#define EAIF_STR_FMT_Y "y__"
#define EAIF_STR_FMT_MPI_JPG "mpi_jpg"
#define EAIF_STR_FMT_MPI_RGB "mpi_rgb"
#define EAIF_STR_FMT_MPI_Y "mpi_y__"

#define EAIF_TRY_SLEEP_TIME 2

#define min(x, y) ((x) > (y)) ? (y) : (x)
#define max(x, y) ((x) < (y)) ? (y) : (x)
#define clamp(a, l, h) (((a) < (l)) ? (l) : (((a) > (h)) ? (h) : (a)))

typedef struct {
	int numerator;
	int denominator;
} Fraction;

typedef union {
	struct {
		unsigned int append : 1;
		unsigned int wait_res : 1;
		unsigned int respond : 1;
		unsigned int recv : 1;
		unsigned int avail : 1;
		unsigned int reset : 1;
	};
	unsigned int val;
} EaifRequestState;

typedef struct {
	int frame_counter;
	int confid_counter;
	int infer_counter;
	int stage;
	EAIF_OBJ_ATTR_S basic;
} EaifObjAttrEx;

/* TODO: revisit the meaning of obj_exist_any* and obj_exist_classify_period */
typedef struct eaif_status_internal_s {
	EAIF_URL_REACHABLE_E server_reachable;
	UINT32 timestamp;
	UINT32 obj_cnt; /* number of object(s) (both positive and negative) */
	UINT32 obj_exist_any; /* if exist at least one positive object */
	UINT32 obj_exist_any_counter; /* how many frame that exist at least one positive object in a period of time */
	EaifObjAttrEx obj_attr_ex[MPI_IVA_MAX_OBJ_NUM];
} EaifStatusInternal;

typedef struct {
	const EAIF_PARAM_S *p;

	/**** face recongition ****/
	/* filter parameter */
	int min_fr_pixel;
	int min_fr_snp_pixel;
	Fraction min_fr_ratio;
	Fraction max_fr_ratio;
	int pos_period;

	/* update */
	int new_obj_prio; // priority of new object
	int neg_obj_prio; // priority of negative face (unknown)
	int pos_face_prio; // priority of positive face (face-id)
	int face_area_prio; // priority of face area (larger->higher)
	int inf_period_prio;
	int life_prio;
	Fraction face_prio_rate;

	int fr_stage1_wait;
	int fr_stage2_wait; // map to detection period ?

	bool face_preserve_prev;
	bool face_det_roi;
} EaifAlgo;

typedef struct eaif_info_s {
	EAIF_INFERENCE_MODE_E mode;
	int payload_size;
	int server_reachable;
	int detect_counter;
	EaifRequestState req_sta;
	MPI_SIZE_S src_resoln;
	EaifFixedPointSize scale_factor;
	MPI_IVA_OBJ_LIST_S obj_list; // append_req
	MPI_IVA_OBJ_LIST_S prev_obj_list;

	// local use for only
	// fr
	/* stage 0 - init stage, prepare for detect
	   stage 1 - wait for recognition
	   stage 2 - finish recognition
	*/
	int inf_fr_stage;
	int inf_fr_counter;
	int inf_fr_stage_id;

	int local_inf_fr_stage;
	MPI_IVA_OBJ_LIST_S local_list;
	EaifStatusInternal local_status;
	char local_url[EAIF_URL_CHAR_LEN];

	AGTX_IVA_EAIF_RESP_S agtx_resp;
	EaifRespond resp;
	EaifRequestData payload;
	const EaifAlgo *algo;
	EAIF_INF_COMMAND_E inf_cmd;
} EaifInfo;

struct eaif_algo_status_s {
	pthread_mutex_t lock; /* lock for algo */
	pthread_mutex_t inf_lock; /* lock for inference module */
	pthread_t tid_eaif;
	int running;
	MPI_WIN idx;
	EAIF_PARAM_S param;
	EaifInfo info;
	EaifAlgo algo;
	EaifStatusInternal status;
	InferenceModel model;

	// for jpeg snapshot
	MPI_ECHN snapshot_chn;
	MPI_BCHN bchn;

	/* call back from runEaif */
	int (*probe)(void *); /* probe url */
	int (*init_buffer)(void *); /* init buffer */
	int (*set_local_info)(void *); /* copy request to local buffer */
	int (*set_payload)(void *); /* setup request payload */
	int (*send_request)(void *); /* send request or run inference */
	//int (*set_payload)(const MPI_IVA_OBJ_LIST_S *, const EAIF_PARAM_S *, EaifInfo *);
	int (*debug)(void *);
	int (*decode_result)(void *); /* decode reesult from inference */
	int (*merge_result)(void *); /* merge result to eaif */
	int (*release_buffer)(void *);

	/* call back from activate */
	int (*init_algo)(void *); /* init algo parameter from usr param for each application */
	int (*init_module)(void *); /* init module for remote/inapp (check success before thread creation) */
	int (*exit_module)(void *); /* exit module for remote/inapp */

	/* call back from testrequest */
	int (*update)(void *, const MPI_IVA_OBJ_LIST_S *); /* update internal state */
};

/**
 * @brief		Get the object attribute pointer with target id.
 * @param[in]  objAttr_list 	List of object attribute
 * @param[in]  size				List size
 * @param[in]  id				Target id
 * @return Target object attribute pointer.
 * @retval EaifObjAttrEx*		Target object attribute
 * @retval NULL					Do not find the target
 */
static inline const EaifObjAttrEx *getObjAttrEx(const EaifObjAttrEx *objAttr_list, const int size, const int id)
{
	for (int i = 0; i < size && i < MPI_IVA_MAX_OBJ_NUM; i++) {
		if (objAttr_list[i].basic.id == id) {
			return &objAttr_list[i];
		}
	}
	return NULL;
}

void eaif_copyObjList(const MPI_IVA_OBJ_LIST_S *src, MPI_IVA_OBJ_LIST_S *dst);
void eaif_copyScaledObjList(const EaifFixedPointSize *scale_factor, const MPI_IVA_OBJ_LIST_S *src,
                            MPI_IVA_OBJ_LIST_S *dst);
int eaif_cpyInternalStatus(const EaifStatusInternal *src, EaifStatusInternal *dst);
int eaif_mergeInternalStatus(const EaifStatusInternal *src, EaifStatusInternal *dst);
int eaif_cpyRespStatus(const EaifStatusInternal *src, EAIF_STATUS_S *dst);
int eaif_cpyScaledDetectionStatus(const EaifFixedPointSize *scale_factor, const EaifStatusInternal *src,
                                  EaifStatusInternal *dst);
int eaif_cpyScaledFaceStatus(const EaifFixedPointSize *scale_factor, const EaifStatusInternal *src,
                             const MPI_IVA_OBJ_LIST_S *ol, EaifStatusInternal *dst);
int eaif_checkDetAppendable(const MPI_IVA_OBJ_LIST_S *ori_obj_list, const EaifAlgo *algo,
                            MPI_IVA_OBJ_LIST_S *fin_obj_list);
int eaif_checkAppendable(const MPI_IVA_OBJ_LIST_S *ori_obj_list, const EaifAlgo *algo, EaifStatusInternal *stats,
                         MPI_IVA_OBJ_LIST_S *fin_obj_list);
void eaif_updateObjAttr(const MPI_IVA_OBJ_LIST_S *inObj_list, const UINT16 obj_life_th, const EaifAlgo *algo,
                        EaifStatusInternal *status);
void eaif_updateFrObjAttr(const MPI_IVA_OBJ_LIST_S *ol, const EaifAlgo *algo, EaifInfo *info,
                          EaifStatusInternal *status);
int eaif_checkFrAppendable(const MPI_IVA_OBJ_LIST_S *src, const EaifAlgo *algo, struct eaif_status_internal_s *status,
                           EaifInfo *info);
void decodeDetection(EaifStatusInternal *status, const AGTX_IVA_EAIF_RESP_S *resp);

void decodeClassification(EaifStatusInternal *status, const AGTX_IVA_EAIF_RESP_S *resp);

void *runEaif(void *input);

int debugEmptyCb(void *ctx);

#endif /* EAIF_ALGO_H_ */
