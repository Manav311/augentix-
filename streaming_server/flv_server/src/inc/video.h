#ifndef VIDEO_H_
#define VIDEO_H_

#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_osd.h"
#include "mpi_iva.h"
#include "mpi_limits.h"
#include "agtx_types.h"
#include <sys/time.h>

#include "flv_muxer.h"

int initStream(uint8_t chn_num, MPI_BCHN *pBchn);

int uninitStream(MPI_BCHN *pBchn);

int readFrame(MPI_BCHN *pBchn, VideoStreamData *pStreamData);

int releaseFrame(MPI_BCHN *pBchn, VideoStreamData *pStreamData);

int checkFrameNalus(const MediaSrcInfo *pSrc_info, VideoStreamData *pStreamData);

unsigned int timespec_to_ms(const TIMESPEC_S *ts);

#endif
