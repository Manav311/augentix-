#ifndef VFTR_DUMP_RTSP_SHM_H_
#define VFTR_DUMP_RTSP_SHM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <time.h>
#include "mpi_index.h"

int RSHM_init(const char *, MPI_WIN);
int RSHM_updateStatus(MPI_WIN idx, void *buf, uint32_t count, uint32_t flag, struct timespec ts);
void RSHM_exit(void);
const char *RSHM_getAppCbString(void);

#ifdef __cplusplus
}
#endif

#endif
