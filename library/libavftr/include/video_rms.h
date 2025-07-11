#ifndef VIDEO_RMS_H_
#define VIDEO_RMS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "mpi_iva_rms.h"

#include "avftr_common.h"

/**
 * @brief Callback function type of video regional motion sensor alarm.
 */
typedef VOID (*VIDEO_FTR_RMS_ALARM_CB)(void);

typedef MPI_IVA_RMS_PARAM_S VIDEO_FTR_RMS_PARAM_S;

/**
 * @brief Structure of video regional motion sensor attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of regional motion sensor*/
	MPI_WIN idx; /**< window index */
	VIDEO_FTR_RMS_ALARM_CB cb; /**< Callback function when alarm triggered*/
	MPI_RECT_POINT_S roi[MPI_IVA_RMS_MAX_REG_NUM];
	MPI_IVA_RMS_REG_LIST_S reg_list[AVFTR_VIDEO_RING_BUF_SIZE];
} VIDEO_RMS_CTX_S;

int VIDEO_FTR_getRmsStat(MPI_WIN idx, VIDEO_RMS_CTX_S *vftr_rms_ctx);
int VIDEO_FTR_getRmsRes(MPI_WIN idx, int buf_idx);
int VIDEO_FTR_transRmsRes(VIDEO_RMS_CTX_S *vftr_rms_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                          const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str, int buf_idx);
int VIDEO_FTR_enableRms(MPI_WIN idx);
int VIDEO_FTR_disableRms(MPI_WIN idx);
int VIDEO_FTR_getRmsParam(MPI_WIN idx, VIDEO_FTR_RMS_PARAM_S *param);
int VIDEO_FTR_setRmsParam(MPI_WIN idx, const VIDEO_FTR_RMS_PARAM_S *param);
int VIDEO_FTR_regRmsCallback(MPI_WIN idx, const VIDEO_FTR_RMS_ALARM_CB alarm_cb_fptr);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_RMS_H_ */
