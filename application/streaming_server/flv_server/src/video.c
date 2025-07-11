#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "log_define.h"
#include "video.h"

#define NANO 1000000L
unsigned int timespec_to_ms(const TIMESPEC_S *ts)
{
	return (ts->tv_sec * 1000) + (ts->tv_nsec / NANO);
}

int checkFrameNalus(const MediaSrcInfo *pSrc_info, VideoStreamData *pStreamData)
{
	if (!((pStreamData->params.seg[0].type == MPI_FRAME_TYPE_SPS) ||
	      (pStreamData->params.seg[0].type == MPI_FRAME_TYPE_P))) {
		flv_server_log_err("unknown seg type: %d\n", pStreamData->params.seg[0].type);
		return -EINVAL;
	}

	/*Both H264/265 Need to have 0x00 00 00 01 header*/
	if (!((pStreamData->params.seg[0].uaddr[0] == 0x00) && (pStreamData->params.seg[0].uaddr[1] == 0x00) &&
	      (pStreamData->params.seg[0].uaddr[2] == 0x00) && (pStreamData->params.seg[0].uaddr[3] == 0x01))) {
		flv_server_log_err("invalid NALU HEAD %0x %0x %0x %0x %0x", pStreamData->params.seg[0].uaddr[0],
		                   pStreamData->params.seg[0].uaddr[1], pStreamData->params.seg[0].uaddr[2],
		                   pStreamData->params.seg[0].uaddr[3], pStreamData->params.seg[0].uaddr[4]);
		return -EINVAL;
	}

	if (pSrc_info->video_codec == MPI_VENC_TYPE_H264) {
		if ((pStreamData->params.seg[0].type == MPI_FRAME_TYPE_SPS) &&
		    !((pStreamData->params.seg[0].uaddr[4] == 0x67 /*H264 SPS*/) ||
		      (pStreamData->params.seg[1].uaddr[4] == 0x68) /*H264 PPS*/ ||
		      (pStreamData->params.seg[2].uaddr[4] == 0x65 /*H264 IDR*/))) {
			flv_server_log_err("[H264-sps]invalid NALU %0x %0x %0x %0x %0x",
			                   pStreamData->params.seg[0].uaddr[0], pStreamData->params.seg[0].uaddr[1],
			                   pStreamData->params.seg[0].uaddr[2], pStreamData->params.seg[0].uaddr[3],
			                   pStreamData->params.seg[0].uaddr[4]);
			return -EINVAL;
		}

		if ((pStreamData->params.seg[0].type == MPI_FRAME_TYPE_P) &&
		    !(pStreamData->params.seg[0].uaddr[4] == 0x41 /*H264 P frame*/)) {
			flv_server_log_err("[H264-P]invalid NALU %0x %0x %0x %0x %0x.",
			                   pStreamData->params.seg[0].uaddr[0], pStreamData->params.seg[0].uaddr[1],
			                   pStreamData->params.seg[0].uaddr[2], pStreamData->params.seg[0].uaddr[3],
			                   pStreamData->params.seg[0].uaddr[4]);
			return -EINVAL;
		}
	}

	if (pSrc_info->video_codec == MPI_VENC_TYPE_H265) {
		if ((pStreamData->params.seg[0].type == MPI_FRAME_TYPE_SPS) &&
		    (pStreamData->params.seg[0].uaddr[4] == 0x40 /*H265 SPS*/)) {
			flv_server_log_err("[H265-SPS]invalid NALU %0x %0x %0x %0x %0x.",
			                   pStreamData->params.seg[0].uaddr[0], pStreamData->params.seg[0].uaddr[1],
			                   pStreamData->params.seg[0].uaddr[2], pStreamData->params.seg[0].uaddr[3],
			                   pStreamData->params.seg[0].uaddr[4]);
			return -EINVAL;
		}

		if ((pStreamData->params.seg[0].type == MPI_FRAME_TYPE_I) &&
		    (pStreamData->params.seg[0].uaddr[4] == 0x26 /*H265 IDR*/)) {
			flv_server_log_err("[H265-IDR]invalid NALU %0x %0x %0x %0x %0x.",
			                   pStreamData->params.seg[0].uaddr[0], pStreamData->params.seg[0].uaddr[1],
			                   pStreamData->params.seg[0].uaddr[2], pStreamData->params.seg[0].uaddr[3],
			                   pStreamData->params.seg[0].uaddr[4]);
			return -EINVAL;
		}

		if ((pStreamData->params.seg[0].type == MPI_FRAME_TYPE_P) &&
		    !(pStreamData->params.seg[0].uaddr[4] == 0x02 /*H265 P frame*/)) {
			flv_server_log_err("[H265-P]invalid NALU %0x %0x %0x %0x %0x.",
			                   pStreamData->params.seg[0].uaddr[0], pStreamData->params.seg[0].uaddr[1],
			                   pStreamData->params.seg[0].uaddr[2], pStreamData->params.seg[0].uaddr[3],
			                   pStreamData->params.seg[0].uaddr[4]);
			return -EINVAL;
		}
	}

	return 0;
}

int initStream(uint8_t chn_num, MPI_BCHN *pBchn)
{
	if (chn_num > MPI_MAX_ENC_CHN_NUM) { /*mpi get chn num*/
		flv_server_log_err("Invalid chn num\n");
		return -1;
	}

	int ret = MPI_FAILURE;
	MPI_ECHN chn_idx;

	chn_idx = MPI_ENC_CHN(chn_num);
	*pBchn = MPI_createBitStreamChn(chn_idx);
	flv_server_log_debug("bchn.value = %08X", pBchn->value);
	if (!VALID_MPI_ENC_BCHN(*pBchn)) {
		flv_server_log_err("MPI_createBitStreamChn failed.");
		return -EACCES;
	}

	/*check codec*/
	MPI_VENC_ATTR_S attr;
	ret = MPI_ENC_getVencAttr(chn_idx, &attr);
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to MPI_ENC_getVencAttr. %d", ret);
		return -EACCES;
	}
	if (attr.type != MPI_VENC_TYPE_H264 && attr.type != MPI_VENC_TYPE_H265) {
		flv_server_log_err("Codec should be H264 or H265. %d", attr.type);
		return -ENOEXEC;
	}

	ret = MPI_ENC_requestIdr(chn_idx);
	flv_server_log_debug("request IDR\n");
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to MPI_ENC_requestIdr. ret: %d", ret);
	}

	return 0;
}

int uninitStream(MPI_BCHN *pBchn)
{
	flv_server_log_debug("uninitStream");
	INT32 ret = MPI_FAILURE;
	ret = MPI_destroyBitStreamChn(*pBchn);
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to destroy bitstream channel, %d", ret);
		return -EACCES;
	}

	return 0;
}

#define FLV_JIFFIES "FLV_JIFFIES"
int readFrame(MPI_BCHN *pBchn, VideoStreamData *pStreamData)
{
	INT32 ret = MPI_FAILURE;
	ret = MPI_getBitStreamV2(*pBchn, &(pStreamData->params), 10000 /*ms*/);
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to get stream param!");
		if (ret == -EAGAIN) {
			flv_server_log_err("MPI_getBitStream EAGAIN");
		} else if (ret == -EFAULT) {
			flv_server_log_err("MPI_getBitStream EFAULT");
		} else if (ret == -ETIMEDOUT) {
			flv_server_log_err("MPI_getBitStream ETIMEDOUT");
		} else if (ret == -EINTR) {
			flv_server_log_err("MPI_getBitStream EINTR");
		} else if (ret == -ENODATA) {
			flv_server_log_err("MPI_getBitStream ENODATA: enc stop chn[%lu]", (unsigned long)pBchn->value);
		} else {
			flv_server_log_err("MPI_getBitStream unknown get bitstream failed: %d", ret);
		}
		return -EIO;
	}
	const char *p = secure_getenv(FLV_JIFFIES);
	if (p == NULL) {
	} else if (atoi(p) == 1) {
		flv_server_log_debug("timestamp: %u\r\n", timespec_to_ms(&pStreamData->params.timestamp));
	}

	return 0;
}

int releaseFrame(MPI_BCHN *pBchn, VideoStreamData *pStreamData)
{
	INT32 ret = MPI_FAILURE;
	ret = MPI_releaseBitStreamV2(*pBchn, &(pStreamData->params));
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to release bit stream! %d", ret);
		return -1;
	}
	return 0;
}
