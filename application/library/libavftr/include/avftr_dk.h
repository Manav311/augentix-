#ifndef AVFTR_DK_H_
#define AVFTR_DK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_base_types.h"
#include "mpi_index.h"
#include "vftr_dk.h"
#include "avftr_common.h"
#include "video_od.h"

#define AVFTR_DK_SHOW_OD_REPORT

typedef struct {
	VFTR_DK_PARAM_S dk_param;
} AVFTR_DK_PARAM_S;

typedef struct {
	VFTR_DK_STATUS_S dk_status;
	MPI_IVA_OBJ_LIST_S obj_list;
	MPI_RECT_POINT_S roi;
} AVFTR_DK_STATUS_S;

typedef VOID (*AVFTR_DK_ALARM_CB)(MPI_WIN idx, VFTR_DK_DET_RESULT_E event, const AVFTR_DK_PARAM_S *param);

typedef struct {
	UINT8 en;
	UINT8 reg;
	MPI_WIN idx;
	AVFTR_DK_STATUS_S dk_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_DK_CTX_S;

int AVFTR_DK_getStat(const MPI_WIN idx, AVFTR_DK_CTX_S *vftr_dk_ctx);
int AVFTR_DK_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx);
int AVFTR_DK_transRes(AVFTR_DK_CTX_S *vftr_dk_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_DK_addInstance(const MPI_WIN idx);
int AVFTR_DK_deleteInstance(const MPI_WIN idx);

int AVFTR_DK_enable(const MPI_WIN idx);
int AVFTR_DK_disable(const MPI_WIN idx);

int AVFTR_DK_getParam(const MPI_WIN idx, AVFTR_DK_PARAM_S *param);
int AVFTR_DK_setParam(const MPI_WIN idx, const AVFTR_DK_PARAM_S *param);
int AVFTR_DK_writeParam(const MPI_WIN idx);

#ifdef __cplusplus
}
#endif

#endif /* AVFTR_DK_H_ */
