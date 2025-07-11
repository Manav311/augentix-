#ifndef AVFTR_PD_H_
#define AVFTR_PD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "vftr_osc.h"
#include "mpi_base_types.h"

#include "avftr_common.h"
#include "video_od.h"

#define AVFTR_PD_PEDESTRIAN "PEDESTRIAN"
#define AVFTR_PD_UNKNOWN "UNKNOWN"

typedef VFTR_OSC_PARAM_S AVFTR_PD_PARAM_S;

typedef VFTR_OSC_STATUS_S AVFTR_PD_STATUS_S;

/**
 * @brief Callback function type of video pedestrian detection alarm.
 */
typedef VOID (*AVFTR_PD_ALARM_CB)(void);

/**
 * @brief Structure of video pedestrian detection attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of pedestrian detection */
	UINT8 reg;
	MPI_WIN idx; /**< Window index */
	AVFTR_PD_ALARM_CB cb; /**< Callback function when alarm triggered */
	AVFTR_PD_STATUS_S stat[AVFTR_VIDEO_RING_BUF_SIZE];
} AVFTR_PD_CTX_S;

int AVFTR_PD_addInstance(MPI_WIN idx);
int AVFTR_PD_deleteInstance(MPI_WIN idx);

int AVFTR_PD_getStat(MPI_WIN idx, const AVFTR_PD_CTX_S *vftr_pd_ctx);
int AVFTR_PD_getRes(MPI_WIN idx, VIDEO_FTR_OBJ_LIST_S *obj_list, int buf_idx);

int AVFTR_PD_enable(MPI_WIN idx);
int AVFTR_PD_disable(MPI_WIN idx);

int AVFTR_PD_getParam(MPI_WIN idx, AVFTR_PD_PARAM_S *param);
int AVFTR_PD_setParam(MPI_WIN idx, const AVFTR_PD_PARAM_S *param);
int AVFTR_PD_writeParam(MPI_WIN idx);

int AVFTR_PD_regCallback(MPI_WIN idx, const AVFTR_PD_ALARM_CB alarm_cb_fptr);

#ifdef __cplusplus
}
#endif

#endif /* !AVFTR_PD_H_ */
