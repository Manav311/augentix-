#ifndef AVFTR_BM_H_
#define AVFTR_BM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define BM_SHOW_VFTR_FGD_REPORT

#include "vftr_fgd.h"
#include "mpi_base_types.h"

#include "avftr_common.h"
#include "video_od.h"

typedef enum {
	AVFTR_BM_EVENT_ABSENT = 0,
	AVFTR_BM_EVENT_ACTIVE,
	AVFTR_BM_EVENT_AWAKE,
	AVFTR_BM_EVENT_SLEEP,
	AVFTR_BM_EVENT_BOUNDARY,
	AVFTR_BM_EVENT_ENTERING,
	AVFTR_BM_EVENT_LEAVING
} AVFTR_BM_EVENT_E;

typedef struct {
	UINT32 duration_active;
	UINT32 duration_awake;
	UINT32 duration_sleep;
	AVFTR_BM_EVENT_E current_event;
	MPI_RECT_POINT_S roi;
	VFTR_FGD_STATUS_S fgd_stat;
} AVFTR_BM_STATUS_S;

typedef struct {
	VFTR_FGD_PARAM_S fgd_param;
} AVFTR_BM_PARAM_S;

typedef struct {
	UINT8 en;
	UINT8 reg;
	UINT8 resource_registered;
	MPI_WIN idx;
	AVFTR_BM_STATUS_S bm_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_BM_CTX_S;

typedef VFTR_FGD_DATA_CTRL_E AVFTR_BM_DATA_CTRL_E;

int AVFTR_BM_getStat(MPI_WIN idx, const AVFTR_BM_CTX_S *vftr_bm_ctx);
int AVFTR_BM_getRes(MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, int buf_idx);
int AVFTR_BM_transRes(AVFTR_BM_CTX_S *vftr_bm_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                      const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str,
                      int buf_idx);
int AVFTR_BM_addInstance(MPI_WIN idx);
int AVFTR_BM_deleteInstance(MPI_WIN idx);
int AVFTR_BM_enable(MPI_WIN idx);
int AVFTR_BM_disable(MPI_WIN idx);
int AVFTR_BM_getParam(MPI_WIN idx, AVFTR_BM_PARAM_S *param);
int AVFTR_BM_setParam(MPI_WIN idx, const AVFTR_BM_PARAM_S *param);
int AVFTR_BM_writeParam(MPI_WIN idx);
int AVFTR_BM_resetData(MPI_WIN idx);
int AVFTR_BM_updateMpiInfo(MPI_WIN idx);
int AVFTR_BM_regMpiInfo(MPI_WIN idx);
int AVFTR_BM_releaseMpiInfo(MPI_WIN idx);


static inline int VIDEO_FTR_suppressBm(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

static inline int VIDEO_FTR_resumeBm(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

int VIDEO_FTR_resetBmShm(MPI_WIN idx);
int VIDEO_FTR_ctrlBmData(MPI_WIN idx, const char *data_path, AVFTR_BM_DATA_CTRL_E ctrl);

#ifdef __cplusplus
}
#endif

#endif /* AVFTR_BM_H_ */