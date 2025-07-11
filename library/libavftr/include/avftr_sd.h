#ifndef AVFTR_SD_H_
#define AVFTR_SD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "aftr_sd.h"
#include "avftr_common.h"
#include "mpi_base_types.h"
#include <stdint.h>

/**
 * @brief Callback function type of audio sound detection alarm.
 */
typedef VOID (*AVFTR_SD_ALARM_CB)(void);

/**
 * @brief Structure of audio sound detection attributes.
 */
typedef struct {
	UINT8 en; /**< Enable status of sound detection*/
	UINT8 reg;
	MPI_DEV idx;
	AVFTR_SD_ALARM_CB cb; /**< Callback function when alarm triggered*/
	AFTR_SD_STATUS_S sd_res[AVFTR_AUDIO_RING_BUF_SIZE];
} AVFTR_SD_CTX_S;

int AVFTR_SD_getStat(const MPI_DEV idx, AVFTR_SD_CTX_S *aftr_sd_ctx);
int AVFTR_SD_getRes(const MPI_DEV idx, const char *raw_buffer, const int size_of_raw, const int buf_idx);

int AVFTR_SD_addInstance(const MPI_DEV idx);
int AVFTR_SD_deleteInstance(const MPI_DEV idx);

int AVFTR_SD_enable(const MPI_DEV idx);
int AVFTR_SD_disable(const MPI_DEV idx);

int AVFTR_SD_getParam(const MPI_DEV idx, AFTR_SD_PARAM_S *param);
int AVFTR_SD_setParam(const MPI_DEV idx, const AFTR_SD_PARAM_S *param);
int AVFTR_SD_writeParam(const MPI_DEV idx);

int AVFTR_SD_regCallback(const MPI_DEV idx, const AVFTR_SD_ALARM_CB alarm_cb_fptr);
int AVFTR_SD_resume(const MPI_DEV idx);
int AVFTR_SD_resetShm(const MPI_DEV idx);

#ifdef __cplusplus
}
#endif

#endif /* !AVFTR_SD_H_ */
