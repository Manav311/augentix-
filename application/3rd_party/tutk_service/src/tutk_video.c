#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "AVAPIs.h"
#include <unistd.h>
#include <assert.h>

#include "mpi_enc.h"
#include "mpi_sys.h"

#include "tutk_video.h"
#include "tutk_define.h"

#include "log_define.h"

extern bool gProgressRun;
extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];
extern int gOnlineNum;
extern TutkConfigs gConfigs;

#define MAX_CHN_NUM (3)

int TUTK_initVideoSystem(int chn_idx)
{
	tutkservice_log_info("init video: %d\n", chn_idx);
	int ret = 0;

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		tutkservice_log_err("Initialize system failed.\n");
	} else {
		tutkservice_log_info("Initialize system successfully.\n");
	}

	ret = MPI_initBitStreamSystem();
	if (ret) {
		tutkservice_log_err("MPI bit-stream system initialization failed.\n");
	} else {
		tutkservice_log_info("MPI bit-stream system initialized.\n");
	}

	return ret;
}

int TUTK_initVideoStreamChn(int chn_idx, MPI_BCHN *bchn)
{
	MPI_ECHN echn_idx = MPI_ENC_CHN(chn_idx);

	*bchn = MPI_createBitStreamChn(echn_idx);
	if (bchn == NULL) {
		tutkservice_log_err("Failed to create bit stream channel.\n");
		return -1;
	} else {
		tutkservice_log_info("create bit stream channel %d\n", chn_idx);
		return 0;
	}
}

int TUTK_deInitVideoStreamChn(MPI_BCHN bchn)
{
	int ret = MPI_destroyBitStreamChn(bchn);
	if (ret == MPI_SUCCESS) {
		tutkservice_log_err("Destory BitStream Chn success \n");
	} else {
		tutkservice_log_err("Failed to Destory BitStream Chn \n");
	}
	return ret;
}

int TUTK_deInitVideoSystem()
{
	tutkservice_log_debug("Deinit video: ...\n");
	int err = 0;
	err = MPI_exitBitStreamSystem();
	if (err) {
		tutkservice_log_err("MPI bit-stream system de-initialization failed.\n");
		return 1;
	}
	tutkservice_log_info("MPI bit-stream system de-initialized.\n");

	err = MPI_SYS_exit();
	if (err) {
		tutkservice_log_err("MPI system de-initialization failed.\n");
		return 1;
	}
	return 0;
}

void *thread_VideoFrameData(void *arg)
{
	(void)(arg);

	int av_index = 0, enable_video = 0, ret = 0, lock_ret = 0;
	MPI_STREAM_PARAMS_V2_S stream_params;
	unsigned char send_iframe_only;
	FRAMEINFO_t frameInfo;
	MPI_BCHN bchn;

	TUTK_initVideoStreamChn(gConfigs.video_chn, &bchn);

	memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
	tutkservice_log_info("thread_VideoFrameData start OK\n");
	tutkservice_log_debug("[Video] is ENABLED!!\n");

	frameInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
	frameInfo.flags = IPC_FRAME_FLAG_IFRAME;

	while (gProgressRun) {
		frameInfo.timestamp = GetTimeStampMs();
		ret = MPI_getBitStreamV2(bchn, &stream_params, 1000 /*ms*/);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to get parameters of stream.non-blocking\n");
			continue;
		}
		if (stream_params.seg[0].type == MPI_FRAME_TYPE_SPS || stream_params.seg[0].type == MPI_FRAME_TYPE_I) {
			frameInfo.flags = IPC_FRAME_FLAG_IFRAME;
		} else {
			frameInfo.flags = IPC_FRAME_FLAG_PBFRAME;
		}

		// Send Video Frame to av-idx and know how many time it takes
		for (int i = 0; i < MAX_CLIENT_NUMBER; ++i) {
			//get reader lock
			lock_ret = pthread_rwlock_rdlock(&gClientInfo[i].lock);
			if (lock_ret) {
				tutkservice_log_err("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);
			}

			av_index = gClientInfo[i].av_index;
			enable_video = gClientInfo[i].enable_video;

			//release reader lock
			lock_ret = pthread_rwlock_unlock(&gClientInfo[i].lock);
			if (lock_ret) {
				tutkservice_log_err("Acquire SID %d rdlock error, ret = %d\n", i, lock_ret);
			}

			if (av_index < 0 || enable_video == 0) {
				continue;
			}
			send_iframe_only = gClientInfo[i].send_iframe_only;

			if (send_iframe_only) {
				if (frameInfo.flags == IPC_FRAME_FLAG_IFRAME) {
					unsigned int resend_frame_count = 0;
					avServGetResendFrmCount(av_index, &resend_frame_count);
					if (resend_frame_count >= RESEND_FRAME_COUNT_THRESHOLD_TO_ENABLE_I_FRAME_ONLY) {
						gClientInfo[i].send_iframe_only = 1;
					} else if (resend_frame_count <=
					           RESEND_FRAME_COUNT_THRESHOLD_TO_DISABLE_I_FRAME_ONLY) {
						gClientInfo[i].send_iframe_only = 0;
					}
				} else {
					continue;
				}
			}
			frameInfo.onlineNum = gOnlineNum;

			int frameDataBufSize[2] = { 0 };

			for (unsigned int k = 0; k < stream_params.seg_cnt; ++k) {
				frameDataBufSize[0] += stream_params.seg[k].size;
			}
			frameDataBufSize[1] = stream_params.seg[stream_params.seg_cnt - 1].uaddr +
			                      stream_params.seg[stream_params.seg_cnt - 1].size -
			                      stream_params.seg[0].uaddr;

			assert(frameDataBufSize[1] >= frameDataBufSize[0]);

			ret = avSendFrameData(av_index, (char *)stream_params.seg[0].uaddr, frameDataBufSize[1],
			                      (void *)&frameInfo, sizeof(FRAMEINFO_t));

			if (ret == AV_ER_EXCEED_MAX_SIZE) {
				continue;
			} else if (ret == AV_ER_SESSION_CLOSE_BY_REMOTE) {
				tutkservice_log_err("thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
				UnRegEditClientFromVideo(i);
				continue;
			} else if (ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT) {
				tutkservice_log_err("thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n",
				                    i);
				UnRegEditClientFromVideo(i);
				continue;
			} else if (ret == IOTC_ER_INVALID_SID) {
				tutkservice_log_err("thread_VideoFrameData Session cant be used anymore\n");
				UnRegEditClientFromVideo(i);
				continue;
			} else if (ret == AV_ER_DASA_CLEAN_BUFFER) {
				tutkservice_log_err("thread_VideoFrameData av_index[%d] need to do clean buffer\n",
				                    av_index);
				gClientInfo[i].do_clean_buffer = 1;
				gClientInfo[i].do_clean_buffer_done = 0;
				continue;
			} else if (ret < 0) {
				tutkservice_log_err("thread_VideoFrameData error[%d]\n", ret);
				UnRegEditClientFromVideo(i);
			} else {
				/* normal case do nothing */
				//avServSetResendSize(av_index, 1024);
			}
		}
		ret = MPI_releaseBitStreamV2(bchn, &stream_params);
		if (ret != MPI_SUCCESS) {
			tutkservice_log_err("TUTK_getCurrentVideoFrame ret=%d\n", ret);
			continue;
		}
	}

	TUTK_deInitVideoStreamChn(bchn);

	tutkservice_log_info("[thread_VideoFrameData] exit \n");
	pthread_exit(0);
}

