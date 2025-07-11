#ifndef TUTK_VIDEO_H_
#define TUTK_VIDEO_H_

#include "mpi_base_types.h"
#include "mpi_index.h"
#include "mpi_enc.h"
#include "mpi_dev.h"
#include "P2PCam/AVFRAMEINFO.h"
#include <string.h>
#include <sys/time.h>

#include "tutk_init.h"

int TUTK_initVideoSystem(int chn_idx);
int TUTK_initVideoStreamChn(int chn_idx, MPI_BCHN *bchn);
int TUTK_deInitVideoStreamChn(MPI_BCHN bchn);
int TUTK_deInitVideoSystem();
void *thread_VideoFrameData(void *arg);

#endif