/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include "mpi_enc.h"
#include "mpi_sys.h"
#include "tange_cloud_video.h"
#include "tange_cloud_comm.h"
#include "ec_const.h"
#include "logfile.h"

/**
 * Definition
*/
enum { ENUM_VIDEO_ENC_CHN_0 = 0, // Channel 0 = 2304x1296
       ENUM_VIDEO_ENC_CHN_1, // Channel 1 = VGA
       ENUM_VIDEO_ENC_CHN_MAX,
};

/**
 * Variables
*/

/**
 * Static Variables
*/
static bool g_runVideo = false;
static MPI_BCHN g_bchn[ENUM_VIDEO_ENC_CHN_MAX] = { 0 }; // 0HD+0SD
static TCMEDIA media_type[ENUM_VIDEO_ENC_CHN_MAX] = { TCMEDIA_VIDEO_MAX }; // 0HD+0SD
static int g_video_chn = ENUM_VIDEO_ENC_CHN_0;
static int g_p2p_chn = 0;
/**
 * Static Function Prototype
*/
static int TGC_initVideoStreamChn(int chn_idx, MPI_BCHN *bchn, TCMEDIA *type);
static int TGC_deinitVideoStreamChn(MPI_BCHN bchn);

/**
 * Functions
*/
int TGC_initVideoSystem(void)
{
	int ret = 0;

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		LogE("Initialize system failed.\n");
	} else {
		LogI("Initialize system successfully.\n");
	}

	ret = MPI_initBitStreamSystem();
	if (ret) {
		LogE("MPI bit-stream system initialization failed.\n");
	} else {
		LogI("MPI bit-stream system initialized.\n");
	}

	return ret;
}

int TGC_deinitVideoSystem(void)
{
	LogI("Deinit video: ...\n");
	int err = 0;
	void *ret = NULL;

	err = MPI_exitBitStreamSystem();
	if (err) {
		LogE("MPI bit-stream system de-initialization failed.\n");
		return 1;
	}
	LogI("MPI bit-stream system de-initialized.\n");

	err = MPI_SYS_exit();
	if (err) {
		LogE("MPI system de-initialization failed.\n");
		return 1;
	}
	LogI("MPI_SYS_exit.\n");
	return 0;
}

void TGC_stopVideoRun(void)
{
	g_runVideo = false;
}

int TGC_switchVideoSystem(int stream, const char *qstr)
{
	int ret = 0;
	if (!strcmp(qstr, "HD")) {
		g_video_chn = ENUM_VIDEO_ENC_CHN_0;
	} else {
		g_video_chn = ENUM_VIDEO_ENC_CHN_1;
	}
	g_p2p_chn = stream;
	return ret;
}

void *thread_VideoFrameData(void *arg)
{
	(void)(arg);

	int tmp_stream = 0;
	int tmp_p2p_stream = 0;
	int ret = 0;
	MPI_STREAM_PARAMS_S stream_params;
	int i = 0;

	g_runVideo = true;
	for (i = 0; i < ENUM_VIDEO_ENC_CHN_MAX; i++) {
		ret = TGC_initVideoStreamChn(i, &(g_bchn[i]), &(media_type[i]));
		if (media_type[i] >= TCMEDIA_VIDEO_MAX) {
			LogE("TCMEDIA[%d] %d is not supported.\n", i, media_type[i]);
		}
	}

	resetIdleTime();
	while (g_runVideo && (media_type[tmp_stream] < TCMEDIA_VIDEO_MAX)) {
		tmp_stream = g_video_chn;
		tmp_p2p_stream = g_p2p_chn;

		ret = MPI_getBitStream(g_bchn[tmp_stream], &stream_params, 10000 /*ms*/);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to get parameters of stream.non-blocking\n");
			continue;
		}

		int size = 0;
		unsigned int i = 0;
		for (i = 0; i < stream_params.seg_cnt; ++i) {
			size += stream_params.seg[i].size;
		}

		/**
		 * Do frame send here
		*/
		TGC_sendFrame(0, tmp_p2p_stream, media_type[tmp_stream], (char *)stream_params.seg[0].uaddr, size,
		              GetTimeStampMs(), (stream_params.seg[0].type == MPI_FRAME_TYPE_SPS));

		ret = MPI_releaseBitStream(g_bchn[tmp_stream], &stream_params);

		if (ret != MPI_SUCCESS) {
			continue;
		}
	}

	for (i = 0; i < ENUM_VIDEO_ENC_CHN_MAX; i++) {
		TGC_deinitVideoStreamChn(g_bchn[i]);
	}
	pthread_exit(0);
	return NULL;
}

/**
 * Static Function
*/
static int TGC_initVideoStreamChn(int chn_idx, MPI_BCHN *bchn, TCMEDIA *type)
{
	int ret = MPI_SUCCESS;
	MPI_VENC_ATTR_S venc_attr;

	MPI_ECHN echn_idx = MPI_ENC_CHN(chn_idx);

	ret = MPI_ENC_getVencAttr(echn_idx, &venc_attr);
	if (ret == MPI_SUCCESS) {
		if (venc_attr.type == MPI_VENC_TYPE_H264) {
			*type = TCMEDIA_VIDEO_H264;
		} else if (venc_attr.type == MPI_VENC_TYPE_H265) {
			*type = TCMEDIA_VIDEO_H265;
		} else if (venc_attr.type == MPI_VENC_TYPE_JPEG) {
			*type = TCMEDIA_VIDEO_JPEG;
		} else if (venc_attr.type == MPI_VENC_TYPE_MJPEG) {
			*type = TCMEDIA_VIDEO_MJPEG;
		} else {
			*type = TCMEDIA_VIDEO_MAX;
		}
	}

	if (bchn == NULL) {
		LogE("Input NULL MPI_BCHN pointer.\n");
		return (ret = -1);
	} else {
		*bchn = MPI_createBitStreamChn(echn_idx);
		if (bchn == NULL) {
			LogE("Failed to create bit stream channel.\n");
			return (ret = -1);
		} else {
			LogI("create bit stream channel %d\n", chn_idx);
			return ret;
		}
	}
}

static int TGC_deinitVideoStreamChn(MPI_BCHN bchn)
{
	int ret = MPI_destroyBitStreamChn(bchn);
	if (ret == MPI_SUCCESS) {
		LogI("Destory BitStream Chn success \n");
	} else {
		LogE("Failed to Destory BitStream Chn \n");
	}
	return ret;
}
