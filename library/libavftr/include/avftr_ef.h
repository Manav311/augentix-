#ifndef AVFTR_EF_H_
#define AVFTR_EF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "vftr_ef.h"
#include "mpi_base_types.h"

#include "video_od.h"

/**
 * @brief Callback function type of video electronic fence alarm.
 */
typedef VOID (*AVFTR_EF_ALARM_CB)(void);

/**
 * @brief Structure of video electronic fence attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of electronic fence*/
	UINT8 reg; /**< Register ef on idx*/
	MPI_WIN idx;
	AVFTR_EF_ALARM_CB cb; /**< Callback function when alarm triggered*/
	VFTR_EF_STATUS_S ef_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_EF_CTX_S;

/**
 * @brief Alias of VFTR_EF_PARAM_S.
 */
typedef VFTR_EF_VL_ATTR_S AVFTR_EF_VL_ATTR_S;

int AVFTR_EF_getStat(MPI_WIN idx, AVFTR_EF_CTX_S *vftr_ef_ctx);
int AVFTR_EF_getRes(MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, int buf_idx);
int AVFTR_EF_transRes(AVFTR_EF_CTX_S *vftr_ef_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                      const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str,
                      const int buf_idx);

int AVFTR_EF_addInstance(MPI_WIN idx);
int AVFTR_EF_deleteInstance(MPI_WIN idx);

int AVFTR_EF_enable(MPI_WIN idx);
int AVFTR_EF_disable(MPI_WIN idx);

int AVFTR_EF_getParam(MPI_WIN idx, VFTR_EF_PARAM_S *param);
int AVFTR_EF_setParam(MPI_WIN idx, const VFTR_EF_PARAM_S *param);
int AVFTR_EF_writeParam(MPI_WIN idx);

int AVFTR_EF_checkParam(MPI_WIN idx);
int AVFTR_EF_regCallback(MPI_WIN idx, const AVFTR_EF_ALARM_CB alarm_cb_fptr);

int AVFTR_EF_addVl(MPI_WIN idx, AVFTR_EF_VL_ATTR_S *fence);
int AVFTR_EF_rmVl(MPI_WIN idx, INT32 fence_id);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_EF_H_ */
