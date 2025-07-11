#ifndef AVFTR_LD_H_
#define AVFTR_LD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "vftr_ld.h"
#include "mpi_base_types.h"

#include "avftr_common.h"

typedef MPI_RECT_POINT_S AVFTR_RECT_POINT_S;

/**
 * @brief Callback function type of video light detection alarm.
 */
typedef VOID (*AVFTR_LD_ALARM_CB)(void);

/**
 * @brief Structure of video light detection results.
 */
typedef struct {
	VFTR_LD_STATUS_S status;
	AVFTR_RECT_POINT_S roi;
} AVFTR_LD_STATUS_S;

/**
 * @brief Structure of video light detection attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of tamper detection*/
	UINT8 reg;
	MPI_WIN idx;
	AVFTR_LD_ALARM_CB cb; /**< Callback function when alarm triggered*/
	AVFTR_LD_STATUS_S ld_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_LD_CTX_S;

int AVFTR_LD_getStat(const MPI_WIN idx, AVFTR_LD_CTX_S *vftr_ld_ctx);
int AVFTR_LD_getRes(const MPI_WIN idx, const int buf_idx);
int AVFTR_LD_transRes(AVFTR_LD_CTX_S *vftr_ld_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_LD_addInstance(const MPI_WIN idx);
int AVFTR_LD_deleteInstance(const MPI_WIN idx);

int AVFTR_LD_enable(const MPI_WIN idx);
int AVFTR_LD_disable(const MPI_WIN idx);

int AVFTR_LD_getParam(const MPI_WIN idx, VFTR_LD_PARAM_S *param);
int AVFTR_LD_setParam(const MPI_WIN idx, const VFTR_LD_PARAM_S *param);
int AVFTR_LD_writeParam(const MPI_WIN idx);

int AVFTR_LD_regCallback(const MPI_WIN idx, const AVFTR_LD_ALARM_CB alarm_cb_fptr);
// int AVFTR_LD_reset(const MPI_WIN idx);
// int AVFTR_LD_suppress(const MPI_WIN idx);
int AVFTR_LD_resume(const MPI_WIN idx);
int AVFTR_LD_resetShm(const MPI_WIN idx);

int AVFTR_LD_regMpiInfo(const MPI_WIN idx, const MPI_RECT_POINT_S *roi);
int AVFTR_LD_releaseMpiInfo(const MPI_WIN idx);
int AVFTR_LD_updateMpiInfo(const MPI_WIN idx);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_LD_H_ */
