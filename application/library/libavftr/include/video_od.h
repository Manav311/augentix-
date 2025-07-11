#ifndef VIDEO_OD_H_
#define VIDEO_OD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mpi_iva.h"

#include "avftr_common.h"

/**
 * @brief Callback function type of video object detection alarm.
 */
typedef VOID (*VIDEO_FTR_OD_ALARM_CB)(void);

#define VFTR_OBJ_CAT_LEN (256)
#define VFTR_OBJ_CONF_LEN (32)
typedef struct {
	UINT8 en_shake_det; /**< Enable status of shaking detection*/
	UINT8 en_crop_outside_obj; /**< Enable status of crop outside part of objects*/
	MPI_IVA_OD_PARAM_S od_param; /**< Parameters of object detection library */
	MPI_IVA_OD_MOTOR_PARAM_S od_motor_param; /**< Parameters of object detection motor library */
} VIDEO_FTR_OD_PARAM_S;

typedef struct {
	char cat[VFTR_OBJ_CAT_LEN];
	char conf[VFTR_OBJ_CONF_LEN];
	MPI_RECT_POINT_S shake_rect;
	int shaking;
} VIDEO_FTR_OBJ_ATTR_S;

typedef struct {
	MPI_IVA_OBJ_LIST_S basic_list;
	VIDEO_FTR_OBJ_ATTR_S obj_attr[MPI_IVA_MAX_OBJ_NUM];
} VIDEO_FTR_OBJ_LIST_S;

/**
 * @brief Structure of video object detection attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of object detection*/
	UINT8 en_implicit; /**< Enable status of implicit object detection*/
	UINT8 en_shake_det; /**< Enable status of shaking detection*/
	UINT8 en_crop_outside_obj; /**< Enable status of crop outside part of objects*/
	MPI_WIN idx; /**< window index */
	VIDEO_FTR_OD_ALARM_CB cb; /**< Callback function when alarm triggered*/
	VIDEO_FTR_OBJ_LIST_S ol[AVFTR_VIDEO_RING_BUF_SIZE]; /**< Object detection result */
	MPI_RECT_POINT_S bdry; /**< Object boundary */
} VIDEO_OD_CTX_S;

int VIDEO_FTR_getOdStat(MPI_WIN idx, VIDEO_OD_CTX_S *vftr_od_ctx);
int VIDEO_FTR_getOdRes(MPI_WIN idx, VIDEO_FTR_OBJ_LIST_S *raw_ol, int buf_idx);
int VIDEO_FTR_transOdRes(VIDEO_OD_CTX_S *vftr_od_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                         const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str,
                         int buf_idx);
int VIDEO_FTR_enableOd(MPI_WIN idx);
int VIDEO_FTR_disableOd(MPI_WIN idx);
int VIDEO_FTR_enableOd_implicit(MPI_WIN idx); // TODO: Move to avftr internal interface.
int VIDEO_FTR_disableOd_implicit(MPI_WIN idx); // TODO: Move to avftr internal interface.
int VIDEO_FTR_getOdParam(MPI_WIN idx, VIDEO_FTR_OD_PARAM_S *param);
int VIDEO_FTR_setOdParam(MPI_WIN idx, const VIDEO_FTR_OD_PARAM_S *param);
int VIDEO_FTR_regOdCallback(MPI_WIN idx, const VIDEO_FTR_OD_ALARM_CB alarm_cb_fptr);
int VIDEO_FTR_getObjList(MPI_WIN idx, UINT32 timestamp, VIDEO_FTR_OBJ_LIST_S *obj_list);

/* deprecated */
int VIDEO_FTR_setOdShakedetStat(MPI_WIN idx, int en_shake_det);
int VIDEO_FTR_getOdShakedetStat(MPI_WIN idx, int *en_shake_det);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_OD_H_ */
