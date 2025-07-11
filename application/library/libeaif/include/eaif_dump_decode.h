#ifndef EAIF_DUMP_DECODE_H_
#define EAIF_DUMP_DECODE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "eaif_dump_define.h"

#include <unistd.h>
#include <sys/types.h>

#include "mpi_enc.h"
typedef pid_t PID_T;

int EAIF_showDump(const VOID *buf, size_t count, UINT32 flag, TIMESPEC_S timestamp, PID_T tid);

#ifdef __cplusplus
}
#endif

#endif
