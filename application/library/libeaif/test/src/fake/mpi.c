#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_sys.h"
#include "mpi_enc.h"

int getMpiSize(const MPI_WIN idx, MPI_SIZE_S *res)
{
	return 0;
}

INT32 MPI_DEV_getWinFrame(MPI_WIN idx, MPI_VIDEO_FRAME_INFO_S *frame_info, INT32 time_ms)
{
	return 0;
}

INT32 MPI_DEV_releaseWinFrame(MPI_WIN idx, MPI_VIDEO_FRAME_INFO_S *frame_info)
{
	return 0;
}

INT32 MPI_DEV_queryChnState(MPI_CHN idx, MPI_CHN_STAT_S *stat)
{
	return 0;
}

INT32 MPI_DEV_getChnAttr(MPI_CHN idx, MPI_CHN_ATTR_S *p_chn_attr)
{
	return 0;
}

MPI_BCHN MPI_createBitStreamChn(MPI_ECHN chn_idx)
{
	return (MPI_BCHN){};
}
INT32 MPI_destroyBitStreamChn(MPI_BCHN chn_idx)
{
	return 0;
}

INT32 MPI_initBitStreamSystem(void)
{
	return 0;
}
INT32 MPI_exitBitStreamSystem(void)
{
	return 0;
}

INT32 _MPI_SYS_init(const char *p_ver_str)
{
	return 0;
}
INT32 _MPI_SYS_exit()
{
	return 0;
}

INT32 MPI_ENC_getChnFrame(MPI_ECHN idx, MPI_STREAM_PARAMS_S *stream_params, INT32 time_ms)
{
	return 0;
}
INT32 MPI_ENC_releaseChnFrame(MPI_ECHN idx, MPI_STREAM_PARAMS_S *stream_params)
{
	return 0;
}

INT32 MPI_DEV_getChnLayout(MPI_CHN idx, MPI_CHN_LAYOUT_S *layout)
{
	return 0;
}
