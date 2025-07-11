#ifndef AVFTR_MD_H_
#define AVFTR_MD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "vftr_md.h"
#include "mpi_index.h"
#include "avftr_common.h"
#include "video_od.h"

/**
 * @brief Define of time stamp unit and MD alarm buffer max time
 */
#define AVFTR_MD_ALARM_BUF_MAX 10 /* MD alarm buffer max time (second) */
#define AVFTR_MD_ALRAM_BUF_DEFAULT 0

/**
 * @brief Callback function type of video motion detection alarm.
 */
typedef VOID (*AVFTR_MD_ALARM_CB)(UINT8 alarm);

/**
 * @brief Struct of video motion detection parameters.
 */
typedef struct {
	UINT8 en_skip_shake; /**< Enable to ignore shaking object. */
	UINT8 en_skip_pd; /**< Enable to skip objects based on Pedestrian detection. */
	UINT32 duration; /**< Alarm buffer duration */
	VFTR_MD_PARAM_S md_param; /**< Parameters of motion detection library */
} AVFTR_MD_PARAM_S;

/**
 * @brief Structure of video motion detection attributes.
 */
typedef struct {
	UINT8 en; /**< Flag to represent MD is enabled on WIN(idx). */
	UINT8 reg; /**< Flag to represent MD is registered on WIN(idx) */
	UINT8 total_alarm; /* FIXME: Add into MD lib */
	MPI_WIN idx;
	AVFTR_MD_ALARM_CB cb; /**< Callback function when alarm triggered */
	VFTR_MD_STATUS_S md_res[AVFTR_VIDEO_RING_BUF_SIZE]; /**< Motion detection result list */
} AVFTR_MD_CTX_S;

int AVFTR_MD_getStat(MPI_WIN idx, AVFTR_MD_CTX_S *vftr_md_ctx);
int AVFTR_MD_getRes(MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, int buf_idx);
int AVFTR_MD_transRes(AVFTR_MD_CTX_S *vftr_md_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_MD_addInstance(MPI_WIN idx);
int AVFTR_MD_deleteInstance(MPI_WIN idx);

int AVFTR_MD_enable(MPI_WIN idx);
int AVFTR_MD_disable(MPI_WIN idx);

int AVFTR_MD_getParam(MPI_WIN idx, AVFTR_MD_PARAM_S *param);
int AVFTR_MD_setParam(MPI_WIN idx, const AVFTR_MD_PARAM_S *param);
int AVFTR_MD_writeParam(MPI_WIN idx);

int AVFTR_MD_regCallback(MPI_WIN idx, const AVFTR_MD_ALARM_CB alarm_cb_fptr);

#ifdef __cplusplus
}
#endif

#endif /* !AVFTR_MD_H_ */
