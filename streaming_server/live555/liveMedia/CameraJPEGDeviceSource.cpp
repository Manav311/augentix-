/*
 *  Copyright (C) Peter Gaal
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#include "CameraJPEGDeviceSource.hh"
#include "BasicUsageEnvironment.hh"
#include "JPEGFrameParser.hh"

#include "mpi_dev.h"
#ifdef RTSP_SERVER_ENABLE_IVA
#include "avftr.h"
#include "avftr_conn.h"
#else
#endif

#include <GroupsockHelper.hh> // for "gettimeofday()"

#ifdef RECORD_V_SRC
#include <time.h>
char vfileName[128];
FILE *pfv;
#endif

extern RtspServerConf gServerconf;

CameraJPEGDeviceSource *CameraJPEGDeviceSource::createNew(UsageEnvironment &env, unsigned clientSessionId,
                                                          unsigned int chn_idx, int timerfdInterval)
{
	return new CameraJPEGDeviceSource(env, chn_idx, timerfdInterval);
}

CameraJPEGDeviceSource *CameraJPEGDeviceSource::createNew(UsageEnvironment &env, unsigned clientSessionId)
{
	return new CameraJPEGDeviceSource(env, 0, 30);
}

CameraJPEGDeviceSource ::CameraJPEGDeviceSource(UsageEnvironment &env, unsigned int chn_idx, int timerfdInterval)
        : JPEGVideoSource(env)
        , fEnv(env)
{
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

	printf("start chn_idx %d (MJPEG)\n", chn_idx);
	this->bchn_idx = MPI_createBitStreamChn(chn);
	if (!VALID_MPI_ENC_BCHN(this->bchn_idx)) {
		fprintf(stderr, "Failed to create bitstream channel %d.\n", chn_idx);
		init_fail = 1;
		return;
	}
	isCreateChn = 1;

	ret = MPI_ENC_getVencAttr(chn, &p_venc_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_ENC_getVencAttr %d.\n", chn_idx);
		init_fail = 1;
		return;
	}

	enc_type = p_venc_attr.type;
	this->chn_idx = chn;
	isIframe = 0;
	stream_params.seg_cnt = 0;
	stream_params.frame_id = 0;
	stream_params.seg[0].type = MPI_FRAME_TYPE_NUM;
	fTimerfdInterval = timerfdInterval;

	fJpegFrameParser = new JPEGFrameParser(); // can new in subsession ?
}

CameraJPEGDeviceSource::~CameraJPEGDeviceSource()
{
	if ((int)nextTask() != 0) {
		/*if timerfd exist, close it first to stop streamming*/
		envir().taskScheduler().unscheduleDelayedTask(nextTask());
	}
	
	
	delete fJpegFrameParser;

	INT32 ret = MPI_FAILURE;
	if ((&MJPEG_buf.params == NULL) || (MJPEG_buf.params.seg[0].type != MPI_FRAME_TYPE_I)) {
		/*Already release current bitstream*/
		fprintf(stderr, "params == NULL, don't need release\r\n");
		goto destroy_system;
	}
#ifdef HC1703_1723_1753_1783S
	ret = MPI_releaseBitStreamV2(this->bchn_idx, &MJPEG_buf.params);
#else
	ret = MPI_releaseBitStream(this->bchn_idx, &MJPEG_buf.params);
#endif
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to release bit stream!\n");
		handleClosure();
		return;
	}
destroy_system:
	if (isCreateChn == 1) {
		ret = MPI_destroyBitStreamChn(this->bchn_idx);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to exit bitstream system");
			return;
		}
	} else {
		fprintf(stderr, "has destroy chn for re-stream\r\n");
	}

	printf("Bit stream system exited.\n");
}

void CameraJPEGDeviceSource::getNextFrame(void *ptr)
{
	((CameraJPEGDeviceSource *)ptr)->getBitStream();
}

void CameraJPEGDeviceSource::doGetNextFrame()
{
	if (!isCurrentlyAwaitingData()) {
		return;
	}
	
	if (init_fail) {
		handleClosure();
		return;
	}

	if ((int)nextTask() == 0 /*have not created own timerfd*/) {
		nextTask() = envir().taskScheduler().scheduleDelayedTask(
			fTimerfdInterval, (TaskFunc *)CameraJPEGDeviceSource::getNextFrame, this);
		printf("%s %d video  timerfd: %d\n", __func__, __LINE__, *(int*)nextTask());
	}
}


void CameraJPEGDeviceSource::getBitStream()
{
	INT32 ret = MPI_FAILURE;

	int parser_ret;
	unsigned int jpeg_length;
	unsigned newFrameSize = 0;

#ifdef HC1703_1723_1753_1783S
	if (gServerconf.isBlocking /*MJPEG blockingmode*/) {
		ret = MPI_getBitStreamV2(this->bchn_idx, &MJPEG_buf.params, 10000 /*ms*/);
	} else {
		ret = MPI_getBitStreamV2(this->bchn_idx, &MJPEG_buf.params, 0 /*ms*/);
	}
#else
	ret = MPI_getBitStream(this->bchn_idx, &MJPEG_buf.params, 0 /*ms*/);
#endif
	if (ret != MPI_SUCCESS) {
		fFrameSize = 0;

		if (ret == -EAGAIN) {
			/*timerfd is little less then 1/FPS, so need to avoid log too much */
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

		//fprintf(stderr, "Failed to get stream param!, ret: %d\n", ret);
		if (isIframe == 0) { /*Before get the first MJPEG*/
			FramedSource::afterGetting(this);
		}
		return ;
	}
	if ((MJPEG_buf.params.frame_id == 0) && (MJPEG_buf.params.frame_id != fFrameID + 1)) {
		printf("get re-stream signal\r\n");
		if ((int)nextTask() != 0) {
			/*if timerfd exist, close it first*/
			envir().taskScheduler().unscheduleDelayedTask(nextTask());
		}
		envir().taskScheduler().triggerEvent(gServerconf.chnRestartTrigger, (void *)&this->chn_idx.chn);
#ifdef HC1703_1723_1753_1783S
		ret = MPI_releaseBitStreamV2(this->bchn_idx, &MJPEG_buf.params);
#else
		ret = MPI_releaseBitStream(this->bchn_idx, &MJPEG_buf.params);
#endif
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to release bit stream!\n");
			handleClosure();
			return;
		}

		ret = MPI_destroyBitStreamChn(this->bchn_idx);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to exit bitstream system");
			return;
		}
		isCreateChn = 0;
		return;
	}
	fFrameID = MJPEG_buf.params.frame_id;
	isIframe = 1;

	if (MJPEG_buf.params.seg[0].size < 0) {
		fprintf(stderr, "Failed to get segment\r\n");
		handleClosure();
		return;
	}

#ifdef RECORD_V_SRC
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	if (enc_type == MPI_VENC_TYPE_MJPEG) {
		sprintf(&vfileName[0], "/mnt/nfs/ethnfs/%d-%02d-%02d_%02d_%02d_%02d.jpg", tm.tm_year + 1900,
				tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	} else {
		fprintf(stderr, "unknown enc type:%d\n", enc_type);
		init_fail = 1;
		return;
	}
	pfv = fopen(vfileName, "w");
#endif

#ifdef RECORD_V_SRC
	printf("[%d]seg_cnt:%d, type:%d, size:%d\n", MJPEG_buf.params.frame_id, MJPEG_buf.params.seg_cnt,
			MJPEG_buf.params.seg[0].type, MJPEG_buf.params.seg[0].size);
	for (uint8_t i = 0; i < MJPEG_buf.params.seg_cnt; i++) {
		fwrite(MJPEG_buf.params.seg[i].uaddr, MJPEG_buf.params.seg[i].size, 1, pfv);
	}
#endif

#ifdef RECORD_V_SRC
	fclose(pfv);
#endif

	for (unsigned int i = 0; i < MJPEG_buf.params.seg_cnt; i++) {
		newFrameSize += MJPEG_buf.params.seg[i].size;
#ifdef RECORD_V_SRC
		fprintf(stderr, "seg[%d]uaddr:%#x size:%d \r\n", i, MJPEG_buf.params.seg[i].uaddr,
				MJPEG_buf.params.seg[i].size);
#endif
	}
#ifdef RECORD_V_SRC
	int seg_cnt = MJPEG_buf.params.seg_cnt;

	fprintf(stderr, "seg cnt: %d, size:%d, flag:%d, ", MJPEG_buf.params.seg_cnt, newFrameSize,
			MJPEG_buf.params.seg[seg_cnt - 1].frame_end);
#endif

	// Deliver the data here:
	if (newFrameSize > fMaxSize) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = newFrameSize - fMaxSize;
	} else {
		fFrameSize = newFrameSize;
	}

	parser_ret = fJpegFrameParser->parse(MJPEG_buf.params.seg[0].uaddr, newFrameSize);
	if (parser_ret == -1) {
		fprintf(stderr, "Failed to parse jpeg\n");
	}
	

	fJpegFrameParser->scandata(jpeg_length);
	fLastQFactor = fJpegFrameParser->qFactor();
	fLastWidth = fJpegFrameParser->width();
	fLastHeight = fJpegFrameParser->height();
	fType = fJpegFrameParser->type();

#ifdef HC1703_1723_1753_1783S
	fPresentationTime.tv_sec = MJPEG_buf.params.timestamp.tv_sec;
	fPresentationTime.tv_usec = MJPEG_buf.params.timestamp.tv_nsec / 1000;
#else
	gettimeofday(&fPresentationTime, NULL);
#endif
	fFrameSize = jpeg_length;
	int diff = newFrameSize - jpeg_length;

	for (uint8_t i = 0; i < MJPEG_buf.params.seg_cnt; i++) {
		if (i == 0) {
			memcpy(fTo, MJPEG_buf.params.seg[i].uaddr + diff, MJPEG_buf.params.seg[i].size - diff);
			fTo += MJPEG_buf.params.seg[i].size;
			fTo -= diff;
		} else {
			memcpy(fTo, MJPEG_buf.params.seg[i].uaddr, MJPEG_buf.params.seg[i].size);
			fTo += MJPEG_buf.params.seg[i].size;
		}
	}
#ifdef HC1703_1723_1753_1783S
	ret = MPI_releaseBitStreamV2(this->bchn_idx, &MJPEG_buf.params);
#else
	ret = MPI_releaseBitStream(this->bchn_idx, &MJPEG_buf.params);
#endif
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to release bit stream!\n");
		handleClosure();
		return;
	}

	FramedSource::afterGetting(this);
}

u_int8_t const *CameraJPEGDeviceSource::quantizationTables(u_int8_t &precision, u_int16_t &length)
{
	precision = fJpegFrameParser->precision();
	return fJpegFrameParser->quantizationTables(length);
}

u_int8_t CameraJPEGDeviceSource::type()
{
	return fType;
}
u_int8_t CameraJPEGDeviceSource::qFactor()
{
	return fLastQFactor;
}
u_int8_t CameraJPEGDeviceSource::width()
{
	return fLastWidth;
}
u_int8_t CameraJPEGDeviceSource::height()
{
	return fLastHeight;
}

void CameraJPEGDeviceSource::startCapture()
{
	// Arrange to get a new frame now:
	// Consider the capture as having occurred now:
	gettimeofday(&fLastCaptureTime, NULL);
}

void CameraJPEGDeviceSource::setParamsFromHeader()
{
	// Look for the "SOF0" marker (0xFF 0xC0), to get the frame
	// width and height:
	Boolean foundIt = False;
	for (int i = 0; i < JPEG_HEADER_SIZE - 8; ++i) {
		if (fJPEGHeader[i] == 0xFF && fJPEGHeader[i + 1] == 0xC0) {
			fLastHeight = (fJPEGHeader[i + 5] << 5) | (fJPEGHeader[i + 6] >> 3);
			fLastWidth = (fJPEGHeader[i + 7] << 5) | (fJPEGHeader[i + 8] >> 3);
			foundIt = True;
			break;
		}
	}
	if (!foundIt) {
		fprintf(stderr, "JPEGDeviceSource: Failed to find SOF0 marker in header!\n");
	}

	// The 'Q' factor is not in the header; do an ioctl() to get it:
	fLastQFactor = 70;
}

unsigned CameraJPEGDeviceSource::maxFrameSize() const
{
	return 1920 * 1080;
}
