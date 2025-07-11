#ifndef AVFTR_FLD_H_
#define AVFTR_FLD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "vftr_fld.h"
#include "mpi_base_types.h"

#include "avftr_common.h"
#include "video_od.h"

typedef struct {
	VFTR_FLD_PARAM_S fld_param;
} AVFTR_FLD_PARAM_S;

typedef struct {
	VFTR_FLD_STATUS_S fld_status;
	MPI_IVA_OBJ_LIST_S obj_list;
} AVFTR_FLD_STATUS_S;

typedef VOID (*AVFTR_FLD_ALARM_CB)(MPI_WIN idx, VFTR_FLD_RESULT_E event, const AVFTR_FLD_PARAM_S *param);

typedef struct {
	UINT8 en;
	UINT8 reg;
	MPI_WIN idx;
	AVFTR_FLD_ALARM_CB cb;
	AVFTR_FLD_STATUS_S fld_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_FLD_CTX_S;

int AVFTR_FLD_getStat(const MPI_WIN idx, AVFTR_FLD_CTX_S *vftr_fld_ctx);
int AVFTR_FLD_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx);
int AVFTR_FLD_transRes(AVFTR_FLD_CTX_S *vftr_fld_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                       const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                       const MPI_RECT_S *dst_roi, char *str, const int buf_idx);

int AVFTR_FLD_addInstance(const MPI_WIN idx);
int AVFTR_FLD_deleteInstance(const MPI_WIN idx);

int AVFTR_FLD_enable(const MPI_WIN idx);
int AVFTR_FLD_disable(const MPI_WIN idx);

int AVFTR_FLD_getParam(const MPI_WIN idx, AVFTR_FLD_PARAM_S *param);
int AVFTR_FLD_setParam(const MPI_WIN idx, const AVFTR_FLD_PARAM_S *param);
int AVFTR_FLD_writeParam(const MPI_WIN idx);

int AVFTR_FLD_regCallback(const MPI_WIN idx, const AVFTR_FLD_ALARM_CB alarm_cb_fptr);

#ifdef __cplusplus
}
#endif

#endif /* AVFTR_FLD_H_ */