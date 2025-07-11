#ifndef RTSP_APP_CB_H_
#define RTSP_APP_CB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include "mpi_iva.h"
#include "vftr_shd.h"

typedef int (*app_cb_t)(MPI_WIN odx, void *args, UINT32 count, UINT32 flag, struct timespec ts);

extern app_cb_t g_app_cb;

/* app callback functions */
int initHdCb(MPI_WIN);
int initMdCb(MPI_WIN);

/* utility function for rtsp shared memory */
int initOdCtx(MPI_WIN win_idx);
int updateBufIndex(MPI_WIN idx, UINT32 timestamp);
int refineObjList(MPI_IVA_OBJ_LIST_S *obj_list);
int updateOdStatus(MPI_WIN idx, MPI_IVA_OBJ_LIST_S *obj_list, int buf_index);
int updateShdStatus(MPI_WIN idx, VFTR_SHD_STATUS_S *status, int buf_index);
int setBufReady(MPI_WIN idx, int buf_idx);

#ifdef __cplusplus
}
#endif

#endif
