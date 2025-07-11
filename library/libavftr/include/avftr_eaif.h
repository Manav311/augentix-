#ifndef AVFTR_EAIF_H_
#define AVFTR_EAIF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_base_types.h"
#include "mpi_index.h"
#include "mpi_iva.h"

#include "video_od.h"
#include "eaif.h"

typedef EAIF_PARAM_S AVFTR_EAIF_PARAM_S;

/**
 * @brief Callback function type of video edge ai assisted feature alarm.
 */
typedef VOID (*AVFTR_EAIF_ALARM_CB)(MPI_WIN idx, void *args);

/**
 * @brief Structure of video edge ai assisted feature attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of edge assisted ai feature*/
	UINT8 reg;
	MPI_WIN idx; /**< Window index */
	AVFTR_EAIF_ALARM_CB cb; /**< Callback function when alarm triggered*/
	EAIF_STATUS_S stat[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_EAIF_CTX_S;

int AVFTR_EAIF_getStat(const MPI_WIN idx, AVFTR_EAIF_CTX_S *vftr_eaif_ctx);
int AVFTR_EAIF_getRes(const MPI_WIN idx, VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx);
int AVFTR_EAIF_transRes(AVFTR_EAIF_CTX_S *vftr_eaif_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                        const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                        const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_EAIF_addInstance(const MPI_WIN idx);
int AVFTR_EAIF_deleteInstance(const MPI_WIN idx);

int AVFTR_EAIF_enable(const MPI_WIN idx);
int AVFTR_EAIF_disable(const MPI_WIN idx);

int AVFTR_EAIF_enableV2(const MPI_WIN idx);
int AVFTR_EAIF_disableV2(const MPI_WIN idx);

int AVFTR_EAIF_getParam(const MPI_WIN idx, AVFTR_EAIF_PARAM_S *param);
int AVFTR_EAIF_setParam(const MPI_WIN idx, const AVFTR_EAIF_PARAM_S *param);

int AVFTR_EAIF_regCallback(MPI_WIN idx, const AVFTR_EAIF_ALARM_CB alarm_cb_fptr);

int AVFTR_EAIF_checkParam(const AVFTR_EAIF_PARAM_S *param);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_EAIF_H_ */
