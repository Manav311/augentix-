#ifndef VIDEO_PFM_H_
#define VIDEO_PFM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vftr_pfm.h"

#include <stdint.h>

#include "avftr_common.h"
#include "mpi_base_types.h"
#include "video_od.h"

#define VIDEO_PFM_SCHEDULE_MAX_NUM 10
#define VIDEO_PFM_SEC_PER_DAY 86400
#define VIDEO_PFM_REGIS_INTERVAL_MAX 3600

/**
 * @brief Callback function type of video pet feeding monitoring alarm.
 */
typedef VOID (*AVFTR_PFM_ALARM_CB)(void);

/**
 * @brief Structure of video pet feeding monitoring attributes.
 */
typedef struct {
	VFTR_PFM_STATUS_S data;
	MPI_RECT_POINT_S roi;
} AVFTR_PFM_STATUS_S;

/**
 * @brief Callback function type of video pet feeding monitoring alarm.
 */
typedef struct {
	UINT8 time_num;
	UINT32 times[VIDEO_PFM_SCHEDULE_MAX_NUM]; /**< Absolute time(s) per day [0-86399]*/
	UINT32 regisBg_feed_interval; /**< Time interval(s) between background registration and feeding*/
} AVFTR_PFM_SCHEDULE_S;

/**
 * @brief Callback function type of video pet feeding monitoring alarm.
 */
typedef struct {
	VFTR_PFM_PARAM_S pfm_param;
	AVFTR_PFM_SCHEDULE_S schedule;
} AVFTR_PFM_PARAM_S;

/**
 * @brief Structure of video pet feeding monitoring context.
 */
typedef struct {
	UINT8 en; /**< Enable Pet Feeding Monitoring*/
	UINT8 reg; /**< Flag for instance registration */
	UINT8 resource_registered; /* Flag for resource registration */
	MPI_WIN idx; /**< Window index */
	AVFTR_PFM_STATUS_S stat[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_PFM_CTX_S;

int AVFTR_PFM_getStat(const MPI_WIN idx, AVFTR_PFM_CTX_S *vftr_pfm_ctx);
int AVFTR_PFM_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx);
int AVFTR_PFM_transRes(AVFTR_PFM_CTX_S *vftr_pfm_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                       const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                       const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_PFM_addInstance(const MPI_WIN idx);
int AVFTR_PFM_deleteInstance(const MPI_WIN idx);

int AVFTR_PFM_enable(const MPI_WIN idx);
int AVFTR_PFM_disable(const MPI_WIN idx);

int AVFTR_PFM_getParam(const MPI_WIN idx, AVFTR_PFM_PARAM_S *param);
int AVFTR_PFM_setParam(const MPI_WIN idx, const AVFTR_PFM_PARAM_S *param);
int AVFTR_PFM_writeParam(const MPI_WIN idx);

int AVFTR_PFM_regMpiInfo(const MPI_WIN idx);
int AVFTR_PFM_releaseMpiInfo(const MPI_WIN idx);
int AVFTR_PFM_updateMpiInfo(const MPI_WIN idx);

#ifdef __cplusplus
}
#endif

#endif /* !VIDEO_PFM_H_ */
