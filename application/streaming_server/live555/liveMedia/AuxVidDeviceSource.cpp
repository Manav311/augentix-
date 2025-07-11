/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 **********/
// "liveMedia"
// Copyright (c) 1996-2014 Live Networks, Inc.  All rights reserved.
// A template for a MediaSource encapsulating an audio/video input device
//
// NOTE: Sections of this code labeled "%%% TO BE WRITTEN %%%" are incomplete,
// and need to be written by the programmer
// (depending on the features of the particular device).
// Implementation

#include "AuxVidDeviceSource.hh"
#include "MediaSink.hh"
#include "BasicUsageEnvironment.hh"
#include <GroupsockHelper.hh> // for "gettimeofday()"
#include <malloc.h>
#include "mpi_dev.h"
#ifdef RTSP_SERVER_ENABLE_IVA
#include "avftr.h"
#include "avftr_conn.h"
#else
#endif
#include "auxdebug.hh"

#define NHU_LAYER_ID 0
#define NHU_TEMPORAL_ID 0
#ifdef RTSP_SERVER_ENABLE_IVA
extern int g_avftr_conn;
extern int g_avftr_dst_win;
extern MPI_WIN g_avftr_src_win;
extern AVFTR_CTX_S *avftr_res_shm_client;
#else
#endif
extern RtspServerConf gServerconf;

#define REQIDR
//#define RECORD_V_SRC
#ifdef RECORD_V_SRC
#include <time.h>
char vfileName[128];
FILE *pfv;
#endif

DeviceSource *AuxVidDeviceSource::createNew(UsageEnvironment &env, UINT32 chn_idx, int timerfdInterval)
{
	return new AuxVidDeviceSource(env, chn_idx, timerfdInterval);
}

AuxVidDeviceSource::AuxVidDeviceSource(UsageEnvironment &env, UINT32 chn_idx, int timerfdInterval)
        : DeviceSource(env, DeviceParameters())
        , fWriteTrigger(0)
        , fSplitFileSize(0)
        , fFirstRelease(1)
        , fTimerfdInterval(timerfdInterval)
{
	bzero(fUuid, SEI_UUID_SIZE);
	char uuid[SEI_UUID_SIZE] = { 0x0F, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
		                     0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
	memcpy(fUuid, &uuid[0], SEI_UUID_SIZE);

	INT32 ret = MPI_FAILURE;
	MPI_VENC_ATTR_S p_venc_attr;
	MPI_ECHN chn;
	chn.chn = chn_idx;
	init_fail = 0;

	ret = MPI_initBitStreamSystem();
	if (ret != MPI_SUCCESS) {
		perror("Bitstream system initialisation failed");
		init_fail = 1;
		return;
	}

	printf("start chn_idx %d\n", chn_idx);
	this->bchn_idx = MPI_createBitStreamChn(chn);
	if (!VALID_MPI_ENC_BCHN(this->bchn_idx)) {
		fprintf(stderr, "Failed to create bitstream channel %d.\n", chn_idx);
		init_fail = 1;
		return;
	}
	this->isCreateChn = 1;
#ifdef REQIDR
	/* req Iframe first*/
	ret = MPI_ENC_requestIdr(chn);
	printf("request IDR\n");
	if (ret != MPI_SUCCESS) {
		printf("Failed to MPI_ENC_requestIdr %d.\n", chn_idx);
		init_fail = 1;
		return;
	}
#endif
	ret = MPI_ENC_getVencAttr(chn, &p_venc_attr);
	if (ret != MPI_SUCCESS) {
		printf("Failed to MPI_ENC_getVencAttr %d.\n", chn_idx);
		init_fail = 1;
		return;
	}

	enc_type = p_venc_attr.type;
	this->chn_idx = chn;
#ifdef RTSP_SERVER_ENABLE_IVA
	/* FIXME select window idx */
	this->win_idx = g_avftr_dst_win;
#else
	this->win_idx = 0;
#endif
	isIframe = 0;
	stream_params.seg_cnt = 0;
	stream_params.frame_id = 0;
	stream_params.seg[0].type = MPI_FRAME_TYPE_NUM;


#ifdef RECORD_V_SRC
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if (enc_type == MPI_VENC_TYPE_H264) {
		sprintf(&vfileName[0], "/mnt/nfs/ethnfs/%d-%02d-%02d_%02d_%02d_%02d.264", tm.tm_year + 1900,
		        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else if (enc_type == MPI_VENC_TYPE_H265) {
		sprintf(&vfileName[0], "/mnt/nfs/ethnfs/%d-%02d-%02d_%02d_%02d_%02d.265", tm.tm_year + 1900,
		        tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else {
		fprintf(stderr, "unknown enc type:%d\n", enc_type);
		init_fail = 1;
		return;
	}
	pfv = fopen(vfileName, "w");
#endif
}

AuxVidDeviceSource::~AuxVidDeviceSource()
{
	if ((int)nextTask() != 0) {
		/*if timerfd exist, close it first to stop streamming*/
		envir().taskScheduler().unscheduleDelayedTask(nextTask());
	}
	
	// TODO Auto-generated destructor stub
	INT32 ret = MPI_FAILURE;
	// ret = MPI_exitBitStreamSystem();
	// if (ret != MPI_SUCCESS) {
	//   printf("Failed to exit bitstream system");
	//   return;
	// }
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S *param = &((VIDEO_STREAM_DATA *)fTo)->params;
#else
	MPI_STREAM_PARAMS_S *param = &((VIDEO_STREAM_DATA *)fTo)->params;
#endif

	if ((NULL != &((VIDEO_STREAM_DATA *)fTo)->params) && (isIframe == 1)) {
#ifdef HC1703_1723_1753_1783S
		ret = MPI_releaseBitStreamV2(this->bchn_idx, param);
#else
		ret = MPI_releaseBitStream(this->bchn_idx, param);
#endif
		if (ret != MPI_SUCCESS) {
			printf("Failed to release bit stream!\n");
			handleClosure();
			return;
		}
	} else {
		fprintf(stderr, "params == NULL, don't need release\r\n");
	}

	if (this->isCreateChn == 1) {
		ret = MPI_destroyBitStreamChn(this->bchn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Failed to exit bitstream system");
			return;
		}
		this->isCreateChn = 0;
	} else {
		fprintf(stderr, "has destroy chn for re-stream\r\n");
	}

#ifdef RECORD_V_SRC
	fclose(pfv);
#endif
	printf("Bit stream system exited.\n");
}

unsigned AuxVidDeviceSource::maxFrameSize() const
{
	// By default, this source has no maximum frame size.
	// return maximun size for streamParser.cpp
	// streamParser.cpp have a ring buffer,
	// if we don't give maximun size,the fMaxSize sometimes give a smaller size
	// than frame size
	// Modify BANK_SIZE to change MAX BUFFER in streamParser.cpp
	return 512 * 1024; // 512k
}

uint32_t timestamp = 0;
#ifdef RTSP_SERVER_ENABLE_IVA
int AuxVidDeviceSource::setH265SEIdata()
{
	unsigned int startSEI = 0;
	unsigned int sei_info_size = 0;
	unsigned int sei_nal_size = 0;

	if (AVFTR_getVideoStat(fIvaIdx, &avftr_res_shm_client->vftr)) {
		bzero(fTmpSEIStr, MAX_SEI_SIZE);
		bzero(fSeiNalu, MAX_SEI_SIZE);
		//start code
		startSEI = 0;
		fSeiNalu[startSEI++] = 0x00;
		fSeiNalu[startSEI++] = 0x00;
		fSeiNalu[startSEI++] = 0x00;
		fSeiNalu[startSEI++] = 0x01;
		// SEI (NALU header)
		/*if SEI before VPS:0X4E, after I(P) frame: 0x50, but iCatch NVR can only decode SEI before VPS*/
		fSeiNalu[startSEI++] = 0x4E;
		fSeiNalu[startSEI++] = 0x01;
		// SEI :Type unregistered
		fSeiNalu[startSEI++] = 0x05;
		sei_nal_size = 0;
		//fetch the xmlsei data
		sei_info_size = AVFTR_tranVideoResV2(fIvaIdx, fIdx, &fSrcRect, &fDstRect, &fSrcRoi, &fDstRoi,
		                                     &avftr_res_shm_client->vftr, timestamp, fTmpSEIStr);
		if (sei_info_size < 0) {
			/*h265 SEI data invalid*/
			fprintf(stderr, "[%s, %d]sei_info_size: %d, src(%d, %d, %d) win(%d, %d, %d, %d), dst(%d, %d, %d)  win(%d, %d, %d, %d)ts:%d\n", __func__, __LINE__, sei_info_size, 
			fIvaIdx.dev, fIvaIdx.chn, fIvaIdx.win, fSrcRect.x, fSrcRect.y, fSrcRect.width, fSrcRect.height,
			fIdx.dev, fIdx.chn, fIdx.win, fDstRect.x, fDstRect.y, fDstRect.width, fDstRect.height,
			timestamp);
			return sei_nal_size;
		}

		sei_nal_size = sei_info_size + SEI_UUID_SIZE;
		while (sei_nal_size >= 255) {
			fSeiNalu[startSEI++] = 255;
			sei_nal_size -= 255;
		}
		fSeiNalu[startSEI++] = sei_nal_size;
		//UUID
		memcpy(&fSeiNalu[startSEI], fUuid, SEI_UUID_SIZE);
		startSEI += SEI_UUID_SIZE;
		//copy the tmpSEIStr as payload
		strncpy(&fSeiNalu[startSEI], fTmpSEIStr, sei_info_size);
		startSEI += sei_info_size;
		//Set RBSP trailing bits
		fSeiNalu[startSEI++] = 0x80;
		sei_nal_size = startSEI;
		//dump_mem(sei_nalu,32);
		if (sei_nal_size == 0) {
			fprintf(stderr, "Error CreateH264SEINALU \n");
		} else {
			((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len = sei_nal_size;
			((VIDEO_STREAM_DATA *)fTo)->p_sei_nalu = (unsigned char *)&fSeiNalu[0];
		}
	}
	return sei_nal_size;
}

int AuxVidDeviceSource::setH264SEIdata()
{
	unsigned int startSEI = 0;
	unsigned int sei_info_size = 0;
	unsigned int sei_nal_size = 0;

	if (AVFTR_getVideoStat(fIvaIdx, &avftr_res_shm_client->vftr)) {
		bzero(fTmpSEIStr, MAX_SEI_SIZE);
		bzero(fSeiNalu, MAX_SEI_SIZE);
		//start code
		startSEI = 0;
		fSeiNalu[startSEI++] = 0x00;
		fSeiNalu[startSEI++] = 0x00;
		fSeiNalu[startSEI++] = 0x00;
		fSeiNalu[startSEI++] = 0x01;
		// SEI
		fSeiNalu[startSEI++] = 0x06;
		// SEI , unregistered
		fSeiNalu[startSEI++] = 0x05;
		sei_nal_size = 0;
		//fetch the xmlsei data
		sei_info_size = AVFTR_tranVideoResV2(fIvaIdx, fIdx, &fSrcRect, &fDstRect, &fSrcRoi, &fDstRoi,
		                                     &avftr_res_shm_client->vftr, timestamp, fTmpSEIStr);
		if (sei_info_size < 0) {
			/*h265 SEI data invalid*/
			fprintf(stderr, "[%s, %d]sei_info_size: %d, src(%d, %d, %d) win(%d, %d, %d, %d), dst(%d, %d, %d)  win(%d, %d, %d, %d)ts:%d\n", __func__, __LINE__, sei_info_size, 
			fIvaIdx.dev, fIvaIdx.chn, fIvaIdx.win, fSrcRect.x, fSrcRect.y, fSrcRect.width, fSrcRect.height,
			fIdx.dev, fIdx.chn, fIdx.win, fDstRect.x, fDstRect.y, fDstRect.width, fDstRect.height,
			timestamp);
			return sei_nal_size;
		}

		sei_nal_size = sei_info_size + SEI_UUID_SIZE;

		while (sei_nal_size >= 255) {
			fSeiNalu[startSEI++] = 255;
			sei_nal_size -= 255;
		}
		fSeiNalu[startSEI++] = sei_nal_size;
		//UUID
		memcpy(&fSeiNalu[startSEI], fUuid, SEI_UUID_SIZE);
		startSEI += SEI_UUID_SIZE;
		//copy the fTmpSEIStr as payload
		strncpy(&fSeiNalu[startSEI], fTmpSEIStr, sei_info_size);
		startSEI += sei_info_size;
		//Set RBSP trailing bits
		fSeiNalu[startSEI++] = 0x80;
		sei_nal_size = startSEI;
		//fprintf(stderr,">>>>SEIinfosize:seiPayload = %d: %s \n",sei_info_size,tmpSEIStr);
		//fprintf(stderr,">>>>SEInaluSize = %d:\n",sei_nal_size);
		//dump_mem(sei_nalu,32);
		if (sei_nal_size == 0) {
			fprintf(stderr, "Error CreateH264SEINALU \n");
		} else {
			((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len = sei_nal_size;
			((VIDEO_STREAM_DATA *)fTo)->p_sei_nalu = (unsigned char *)&fSeiNalu[0];
		}
	}

	return sei_nal_size;
}

static int getWinLayout(MPI_WIN idx, MPI_RECT_S *rect)
{
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn;
	uint8_t i;

	chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	if (MPI_DEV_getChnLayout(chn, &layout_attr) < 0) {
		printf("Cannot get channel layout for chn:%d\n", chn.chn);
		return -1;
	}
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			break;
		}
	}
	if (i == layout_attr.window_num) {
		printf("Window %d does not exist in channel %d\n", idx.win, idx.chn);
		return -1;
	}

	rect->x = layout_attr.window[i].x;
	rect->y = layout_attr.window[i].y;
	rect->width = layout_attr.window[i].width;
	rect->height = layout_attr.window[i].height;
	return 0;
}
#else
#endif

void AuxVidDeviceSource::getNextFrame(void *ptr)
{
	((AuxVidDeviceSource *)ptr)->getBitStream();
}

void AuxVidDeviceSource::getBitStream()
{
	INT32 ret = MPI_FAILURE;
	UINT32 i = 0;
	INT32 newFrameSize = 0;
	INT32 writtenFrameSize = 0;
	INT32 newSegSize = 0;
	
	if (fFirstRelease == 1) {
		fFirstRelease = 0;
	} else {
#ifdef HC1703_1723_1753_1783S
		ret = MPI_releaseBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#else
		ret = MPI_releaseBitStream(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#endif
		if (ret != MPI_SUCCESS) {
			printf("Failed to release bit stream!\n");
			handleClosure();
			return;
		}
	}

get_source:
#ifdef HC1703_1723_1753_1783S
	if (isIframe == 0 || gServerconf.isBlocking) {
		/*blocking code or force to get first I frame*/
		ret = MPI_getBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params, 10000 /*ms*/);
	} else {
		ret = MPI_getBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params, 10 /*ms*/);
	}
#else
	ret = MPI_getBitStream(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params, 10000 /*ms*/);
#endif
	if (ret != MPI_SUCCESS) {
		fFrameSize = 0;
		fNumTruncatedBytes = 0;

		if (ret == -EAGAIN) {
			/*timerfd is little less then 1/FPS, need to avoid log too much */
			//fprintf(stderr, "%s:%d: EAGAIN \r\n", __func__, __LINE__);
		} else if (ret == -EFAULT) {
			fprintf(stderr, "%s:%d: EFAULT \r\n", __func__, __LINE__);
		} else if (ret == -ETIMEDOUT) {
			//fprintf(stderr, "%s:%d: ETIMEDOUT  \r\n", __func__, __LINE__);
		} else if (ret == -EINTR) {
			fprintf(stderr, "%s:%d: EINTR \r\n", __func__, __LINE__);
		} else if (ret == -ENODATA) {
			fprintf(stderr, "%s:%d: ENODATA: enc stop chn[%lu] \r\n", __func__, __LINE__,
			        (unsigned long)this->bchn_idx.value);
			if ((int)nextTask() != 0) {
				/*if timerfd exist, close it first*/
				envir().taskScheduler().unscheduleDelayedTask(nextTask());
			}
			envir().taskScheduler().triggerEvent(gServerconf.chnRestartTrigger, (void *)&this->chn_idx.chn);
			return;
		} else {
			fprintf(stderr, "%s:%d unknown get bitstream failed: %d\r\n", __func__, __LINE__, ret);
		}

		if (isIframe == 0) {
			fprintf(stderr, "need to force to get first IDR frame\n");
			goto get_source;			
		}
		//fprintf(stderr, "Failed to get stream param at %lu\r\n", (unsigned long)this->bchn_idx.value);
		
		gettimeofdayMonotonic(&fPresentationTime, NULL);

#ifdef HC1703_1723_1753_1783S
		//fprintf(stderr, "%s:%d release err frame\r\n", __func__, __LINE__);
		ret = MPI_releaseBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#else
		ret = MPI_releaseBitStream(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#endif
		if (ret != MPI_SUCCESS) {
			printf("Failed to release bit stream!\n");
			handleClosure();
			return;
		}

		return;
	}

	/*check re-stream status*/
	if ((((VIDEO_STREAM_DATA *)fTo)->params.frame_id == 0) && /*has delay 10 s, maybe id not 0*/
	    ((VIDEO_STREAM_DATA *)fTo)->params.frame_id != fFrameID + 1) {
		printf("get re-stream signal\r\n");
		if ((int)nextTask() != 0) {
			/*if timerfd exist, close it first*/
			envir().taskScheduler().unscheduleDelayedTask(nextTask());
		}
		envir().taskScheduler().triggerEvent(gServerconf.chnRestartTrigger, (void *)(&this->chn_idx.chn));
		/*release bitstream*/
#ifdef HC1703_1723_1753_1783S
		ret = MPI_releaseBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#else
		ret = MPI_releaseBitStream(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#endif
		if (ret != MPI_SUCCESS) {
			printf("Failed to release bit stream!\n");
			handleClosure();
			return;
		}
		isIframe = 0;
		ret = MPI_destroyBitStreamChn(this->bchn_idx);
		if (ret != MPI_SUCCESS) {
			printf("Failed to exit bitstream system");
			return;
		}
		isCreateChn = 0;

		return;
	}

	fFrameID = (unsigned long)((VIDEO_STREAM_DATA *)fTo)->params.frame_id;
#ifdef HC1703_1723_1753_1783S
	MPI_STREAM_PARAMS_V2_S *param = (MPI_STREAM_PARAMS_V2_S *)fTo;
#else
	MPI_STREAM_PARAMS_S *param = (MPI_STREAM_PARAMS_S *)fTo;
#endif

	if (param->seg[0].type == MPI_FRAME_TYPE_I) {
		fprintf(stderr, "seg[0] IDR\r\n,segcnt: %d,list first 16 char:", param->seg_cnt);
		for (int i = 0; i < 16; i++) {
			printf("%0x ", param->seg[0].uaddr[i]);
		}
		printf("\r\n");
	}

	if (!isIframe) {
		if (param->seg[0].type == MPI_FRAME_TYPE_SPS) {
			isIframe = 1;
			fprintf(stderr, "find first I, segcnt:%d, ", param->seg_cnt);
			for (int i = 0; i < (int)param->seg_cnt; i++) {
				printf("type:%d, size:%d\n", param->seg[i].type, param->seg[i].size);
			}
#ifdef RTSP_SERVER_ENABLE_IVA
			if (g_avftr_conn > 0) {
				/*get channel win*/
				fIvaIdx = g_avftr_src_win;
				fIdx = MPI_VIDEO_WIN(0, (uint8_t)chn_idx.chn, (uint8_t)win_idx);

				/*get channel layout*/
				ret = getWinLayout(fIvaIdx, &fSrcRect);
				if (ret != 0) {
					printf("Failed to get src layout!\n");
					handleClosure();
					return;
				}
				ret = getWinLayout(fIdx, &fDstRect);
				if (ret != 0) {
					printf("Failed to get dst layout!\n");
					handleClosure();
					return;
				}

				/*get channel roi*/
				ret = MPI_DEV_getWindowRoi(fIvaIdx, &fSrcRoi);
				if (ret != 0) {
					printf("Failed to get src roi!\n");
					handleClosure();
					return;
				}
				ret = MPI_DEV_getWindowRoi(fIdx, &fDstRoi);
				if (ret != 0) {
					printf("Failed to get dst roi!\n");
					handleClosure();
					return;
				}
			}
#else
#endif
		} else {
#ifdef HC1703_1723_1753_1783S
			ret = MPI_releaseBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#else
			ret = MPI_releaseBitStream(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#endif
			if (ret != MPI_SUCCESS) {
				printf("Failed to release bit stream!\n");
				handleClosure();
				return;
			}
			fprintf(stderr, "not find first I\n");
			goto get_source;
		}
	}

	if (param->seg_cnt < 0 || param->seg_cnt > MPI_ENC_MAX_FRAME_SEG_CNT) {
		fprintf(stderr, "[%s, %d] seg cnt: %d\n",  __func__, __LINE__, param->seg_cnt);
		/*not to parse, and wait for nex timerfs event*/
		return;
	}

#ifdef RECORD_V_SRC
	for (uint8_t i = 0; i < param->seg_cnt; i++) {
		fwrite(param->seg[i].uaddr, param->seg[i].size, 1, pfv);
	}
#endif

	gettimeofdayMonotonic(&fPresentationTime, NULL);

	timestamp = param->jiffies;
	fFrameSize = 0;
	/* iterate the segments to get the frame size */
	newFrameSize = param->seg[0].size;

	if (newFrameSize < 0) {
#ifdef HC1703_1723_1753_1783S
		ret = MPI_releaseBitStreamV2(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#else
		ret = MPI_releaseBitStream(this->bchn_idx, &((VIDEO_STREAM_DATA *)fTo)->params);
#endif
		if (ret != MPI_SUCCESS) {
			printf("Failed to release bit stream!\n");
			handleClosure();
			return;
		}
		handleClosure();
		return;
	}
#ifdef RTSP_SERVER_ENABLE_IVA
	if (g_avftr_conn > 0) {
		int sei_val = 0;
		if (enc_type == MPI_VENC_TYPE_H264) {
			sei_val = setH264SEIdata();
			writtenFrameSize += sei_val;
		} else if (enc_type == MPI_VENC_TYPE_H265) {
			if (g_avftr_conn > 1) {
				sei_val = setH265SEIdata();
				writtenFrameSize += sei_val;
			} else {
				sei_val = 0;
			}
		} else {
			printf("not a h254 or h265 Enc_type!\n");
		}

		if (sei_val < 0) {
			fprintf(stderr, "ENC type:%d sei ret: %d, skip this sei frame\n", enc_type, sei_val);
			sei_val = 0;  /*set sei_nalu_len = 0*/
		}

		if (sei_val == 0) {
			((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len = 0;
			((VIDEO_STREAM_DATA *)fTo)->p_sei_nalu = NULL;
		}
	} else {
		((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len = 0;
		((VIDEO_STREAM_DATA *)fTo)->p_sei_nalu = NULL;
	}
#else
	((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len = 0;
	((VIDEO_STREAM_DATA *)fTo)->p_sei_nalu = NULL;
#endif

	if (DUMP_FLAG == 1) {
		if (fWriteTrigger > 0) {
			if (((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len > 0) {
				fwrite(((VIDEO_STREAM_DATA *)fTo)->p_sei_nalu, ((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len, 1,
				       xh264DumpFile);
			}

			if (SPLIT_FILE_SIZE > 0) {
				fSplitFileSize = fSplitFileSize + newSegSize; // update the dump size
				if ((unsigned int)SPLIT_FILE_SIZE <= fSplitFileSize) {
					// do split
					dumpH264Stream2File(2); // SPLIT: stop current dump
				}
			}
		}
	}

	if (DUMP_FLAG != 0) {
		for (i = 0; i < param->seg_cnt; ++i, writtenFrameSize += newSegSize) {
			u_int8_t *newSegAddr = (u_int8_t *)(param->seg[i].uaddr);
			newSegSize = param->seg[i].size;

			/*1 Start Dump with I frame*/
			if ((param->seg[i].type == MPI_FRAME_TYPE_SPS) && (DUMP_FLAG == 1) && (fWriteTrigger == 0)) {
				fWriteTrigger = 1;
				fSplitFileSize = 0;
			}

			/*2 save to the dumpfile descriptor*/
			if (DUMP_FLAG == 1) {
				if (fWriteTrigger > 0) {
					fwrite(newSegAddr, newSegSize, 1, xh264DumpFile);
					if (SPLIT_FILE_SIZE > 0) {
						fSplitFileSize = fSplitFileSize + newSegSize; // update the dump size
						if ((unsigned int)SPLIT_FILE_SIZE <= fSplitFileSize) {
							// do split
							dumpH264Stream2File(2); // SPLIT: stop current dump
						}
					}
				}
			} /* 3 if DUMP_FLAG == 2 got interrupt to pause the dump till all pframes are saved */
			else if (DUMP_FLAG == 2) { // save all pframes till we reach next i frame
				if (param->seg[i].type == MPI_FRAME_TYPE_P) {
					fprintf(stderr, ">P<");
					fwrite(newSegAddr, newSegSize, 1, xh264DumpFile);
					if (SPLIT_FILE_SIZE > 0) {
						fSplitFileSize = fSplitFileSize + newSegSize;
					}
				} else {
					fprintf(stderr, ">SPS-PPS-I<");
					DUMP_FLAG = 3;
					fWriteTrigger = 0;
					if (SPLIT_FILE_SIZE > 0) {
						dumpH264Stream2File(3); // SPLIT: close file  descriptor
						fSplitFileSize = 0;
						dumpH264Stream2File(1); // SPLIT: create new file
					}
				}
			}
		}
	}

	for (unsigned int i = 0; i < param->seg_cnt; i++) {
		fFrameSize += param->seg[i].size;
		/*not count SEI*/
		/*cnt frame total size*/
		if ((param->seg[i].size < 12 || param->seg[i].size > OutPacketBuffer::maxSize) && (fFrameSize < 12 || fFrameSize > OutPacketBuffer::maxSize)/*totally frame size abnormal*/) {
			fprintf(stderr, "[%s %d] seg[%d]size: %d, type:%d, frame size: %d, uaddr: %0x %0x %0x %0x %0x\n",
			__func__, __LINE__, i, param->seg[i].size, param->seg[i].type, fFrameSize,
			param->seg[i].uaddr[0],
			param->seg[i].uaddr[1],
			param->seg[i].uaddr[2],
			param->seg[i].uaddr[3],
			param->seg[i].uaddr[4]);
			/*not to parse, and wait for nex timerfs event*/
			return;
		}
	}

	if (((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len != 0) {
		fFrameSize += ((VIDEO_STREAM_DATA *)fTo)->sei_nalu_len;
	}

	FramedSource::afterGetting(this);
}

void AuxVidDeviceSource::doGetNextFrame()
{
	if (!isCurrentlyAwaitingData())
		return; // we're not ready for the data yet


	if (init_fail) {
		handleClosure();
		return;
	}

	if ((int)nextTask() == 0 /*have not created own timerfd*/) {
		nextTask() = envir().taskScheduler().scheduleDelayedTask(
			fTimerfdInterval, (TaskFunc *)AuxVidDeviceSource::getNextFrame, this);
		printf("%s %d video  timerfd: %d, interval: %d\n", __func__, __LINE__, *(int*)nextTask(), fTimerfdInterval);
	}
}

