#ifndef AVFTR_TD_H_
#define AVFTR_TD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vftr_td.h"

#include <stdint.h>

#include "avftr_common.h"
#include "mpi_base_types.h"

/**
 * @brief Callback function type of video tamper detection alarm.
 */
typedef VOID (*AVFTR_TD_ALARM_CB)(void);

typedef struct {
	VFTR_TD_PARAM_S td_param;
} AVFTR_TD_PARAM_S;

/**
 * @brief Structure of video tamper detection attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of tamper detection*/
	UINT8 reg; /**< Flag for instance registration */
	UINT8 resource_registered; /* Flag for resource registration */
	MPI_WIN idx; /**< window index */
	AVFTR_TD_ALARM_CB cb; /**< Callback function when alarm triggered*/
	VFTR_TD_STATUS_S td_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_TD_CTX_S;

int AVFTR_TD_getStat(const MPI_WIN idx, AVFTR_TD_CTX_S *vftr_td_ctx);
int AVFTR_TD_getRes(const MPI_WIN idx, const int buf_idx);
int AVFTR_TD_transRes(AVFTR_TD_CTX_S *vftr_td_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_TD_addInstance(const MPI_WIN idx);
int AVFTR_TD_deleteInstance(const MPI_WIN idx);

int AVFTR_TD_enable(const MPI_WIN idx);
int AVFTR_TD_disable(const MPI_WIN idx);

int AVFTR_TD_getParam(const MPI_WIN idx, AVFTR_TD_PARAM_S *param);
int AVFTR_TD_setParam(const MPI_WIN idx, const AVFTR_TD_PARAM_S *param);
int AVFTR_TD_writeParam(const MPI_WIN idx);

int AVFTR_TD_regCallback(const MPI_WIN idx, const AVFTR_TD_ALARM_CB alarm_cb_fptr);
int AVFTR_TD_reset(const MPI_WIN idx);
int AVFTR_TD_suppress(const MPI_WIN idx);
int AVFTR_TD_resume(const MPI_WIN idx);
int AVFTR_TD_resetShm(const MPI_WIN idx);

int AVFTR_TD_regMpiInfo(const MPI_WIN idx);
int AVFTR_TD_releaseMpiInfo(const MPI_WIN idx);
int AVFTR_TD_updateMpiInfo(const MPI_WIN idx);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_TD_H_ */
