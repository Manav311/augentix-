#ifndef AVFTR_AROI_H_
#define AVFTR_AROI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "vftr_aroi.h"
#include "mpi_base_types.h"

#include "avftr_common.h"
#include "video_od.h"

/**
 * @brief Callback function type of video automatic region of interest alarm.
 */
typedef VOID (*AVFTR_AROI_ALARM_CB)(MPI_WIN idx, const VFTR_AROI_STATUS_S *status,
                                    const VIDEO_FTR_OBJ_LIST_S *obj_list);

typedef struct {
	UINT8 en_skip_shake; /**< Enable to ignore shaking object*/
	VFTR_AROI_PARAM_S aroi_param; /**< Parameters of aroi library */
} AVFTR_AROI_PARAM_S;

typedef enum {
	AVFTR_MOTION_NONE = 0,
	AVFTR_MOTION_LEFT = 0b01,
	AVFTR_MOTION_RIGHT = 0b10,
	AVFTR_MOTION_BOTH = 0b11
} AVFTR_MOTOIN_DIR_E;

/**
 * @brief Structure of video automatic region of interest attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of automatic region of interest*/
	UINT8 reg;
	UINT8 en_skip_shake; /**< Enable to ignore shaking object*/
	MPI_WIN idx;
	AVFTR_MOTOIN_DIR_E motion_reg;
	AVFTR_MOTOIN_DIR_E motion_dir;
	AVFTR_AROI_ALARM_CB cb; /**< Callback function when alarm triggered*/
	VFTR_AROI_STATUS_S aroi_res[AVFTR_VIDEO_RING_BUF_SIZE];
	MPI_RECT_POINT_S tar[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_AROI_CTX_S;

int AVFTR_AROI_getStat(const MPI_WIN idx, AVFTR_AROI_CTX_S *vftr_aroi_ctx);
int AVFTR_AROI_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx);
int AVFTR_AROI_transRes(AVFTR_AROI_CTX_S *vftr_aroi_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                        const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                        const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_AROI_addInstance(const MPI_WIN idx);
int AVFTR_AROI_deleteInstance(const MPI_WIN idx);

int AVFTR_AROI_enable(const MPI_WIN idx);
int AVFTR_AROI_disable(const MPI_WIN idx);

int AVFTR_AROI_getParam(const MPI_WIN idx, AVFTR_AROI_PARAM_S *param);
int AVFTR_AROI_setParam(const MPI_WIN idx, const AVFTR_AROI_PARAM_S *param);
int AVFTR_AROI_writeParam(const MPI_WIN idx);

int AVFTR_AROI_regCallback(const MPI_WIN idx, const AVFTR_AROI_ALARM_CB alarm_cb_fptr);

int AVFTR_AROI_updateMotionEvt(MPI_WIN idx, const VFTR_AROI_STATUS_S *status, const VIDEO_FTR_OBJ_LIST_S *obj_list,
                               const MPI_SIZE_S *res);

#ifdef __cplusplus
}
#endif

#endif /* !AVFTR_AROI_H_ */
