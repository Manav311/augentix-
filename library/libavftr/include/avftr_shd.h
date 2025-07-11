#ifndef AVFTR_SHD_H_
#define AVFTR_SHD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "mpi_dev.h"
#include "mpi_base_types.h"
#include "vftr_shd.h"

#include "avftr_common.h"

typedef VFTR_SHD_PARAM_S AVFTR_SHD_PARAM_S;
typedef VFTR_SHD_STATUS_S AVFTR_SHD_STATUS_S;
typedef VFTR_SHD_LONGTERM_LIST_S AVFTR_SHD_LONGTERM_LIST_S;

typedef struct {
	UINT8 en;
	UINT8 reg;
	MPI_WIN idx;
	AVFTR_SHD_STATUS_S shd_res[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_SHD_CTX_S;

int AVFTR_SHD_addInstance(const MPI_WIN idx);
int AVFTR_SHD_deleteInstance(const MPI_WIN idx);

int AVFTR_SHD_getStat(const MPI_WIN idx, const AVFTR_SHD_CTX_S *vftr_shd_ctx);
int AVFTR_SHD_detectShake(const MPI_WIN idx, const MPI_IVA_OBJ_LIST_S *obj_list, AVFTR_SHD_STATUS_S *status);

int AVFTR_SHD_enable(const MPI_WIN idx);
int AVFTR_SHD_disable(const MPI_WIN idx);

int AVFTR_SHD_setParam(const MPI_WIN idx, const AVFTR_SHD_PARAM_S *param);
int AVFTR_SHD_getParam(const MPI_WIN idx, AVFTR_SHD_PARAM_S *param);
int AVFTR_SHD_writeParam(const MPI_WIN idx);

int AVFTR_SHD_setUsrList(const MPI_WIN idx, const AVFTR_SHD_LONGTERM_LIST_S *lt_list);
int AVFTR_SHD_getUsrList(const MPI_WIN idx, AVFTR_SHD_LONGTERM_LIST_S *lt_list);
int AVFTR_SHD_writeUsrList(const MPI_WIN idx);

#ifdef __cplusplus
}
#endif

#endif /* AVFTR_SHD_H_ */
