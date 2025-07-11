#include "mpi_index.h"
#include "AuxVidDeviceServerMediaSubsession.hh"
#include "BasicUsageEnvironment.hh"
#include "AuxVidDeviceSource.hh"
#include "DeviceSource.hh"
#include "H264VideoRTPSink.hh"
#include "H264VideoStreamFramer.hh"
#include "H265VideoRTPSink.hh"
#include "H265VideoStreamFramer.hh"
#include "CameraJPEGDeviceSource.hh"
#include "JPEGVideoRTPSink.hh"
#include <sys/types.h>

extern RtspServerConf gServerconf;

DeviceServerMediaSubsession *DeviceServerMediaSubsession::createNew(UsageEnvironment &env, Boolean reuseFirstSource,
                                                                    unsigned int channel)
{
	return new DeviceServerMediaSubsession(env, reuseFirstSource, channel);
}
DeviceServerMediaSubsession::DeviceServerMediaSubsession(UsageEnvironment &env, Boolean reuseFirstSource,
                                                         unsigned int channel)
        : OnDemandServerMediaSubsession(env, reuseFirstSource)
        , fAuxSDPLine(NULL)
        , fDoneFlag(0)
        , fDummyRTPSink(NULL)
{
	fchannel = channel;
	if (MPI_SYS_init() != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_SYS_init!");
	}
}

DeviceServerMediaSubsession::~DeviceServerMediaSubsession()
{
	delete[] fAuxSDPLine;
	MPI_SYS_exit();
}

FramedSource *DeviceServerMediaSubsession::createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate)
{
	int ret = 0;
	MPI_ECHN chn_idx;
	chn_idx.chn = fchannel;
	MPI_VENC_ATTR_S p_venc_attr;
	ret = MPI_ENC_getVencAttr(chn_idx, &p_venc_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_ENC_getVencAttr %d.\n", chn_idx.chn);
		return NULL;
	}

	int timerfdInterval = 33333;

	if (p_venc_attr.type == MPI_VENC_TYPE_H265) {
		timerfdInterval = ((1000000 / (p_venc_attr.h265.rc.frm_rate_o)) * 3 ) / 5;
		if (gServerconf.isBlocking) {
			timerfdInterval = (1000000 / (p_venc_attr.h265.rc.frm_rate_o)) / 2;
		}
		DeviceSource *source = AuxVidDeviceSource::createNew(envir(), fchannel, timerfdInterval);
		return H265VideoStreamFramer::createNew(envir(), source);
	} else if (p_venc_attr.type == MPI_VENC_TYPE_H264) {
		timerfdInterval = ((1000000 / (p_venc_attr.h264.rc.frm_rate_o)) * 3 ) / 5;
		if (gServerconf.isBlocking) {
			timerfdInterval = (1000000 / (p_venc_attr.h264.rc.frm_rate_o)) / 2;
		}
		DeviceSource *source = AuxVidDeviceSource::createNew(envir(), fchannel, timerfdInterval);
		return H264VideoStreamFramer::createNew(envir(), source);
	} else if (p_venc_attr.type == MPI_VENC_TYPE_MJPEG) {
		timerfdInterval = ((1000000 / (p_venc_attr.mjpeg.rc.frm_rate_o)) * 3 ) / 5;
		if (gServerconf.isBlocking) {
			timerfdInterval = (1000000 / (p_venc_attr.mjpeg.rc.frm_rate_o)) / 2;
		}
		return CameraJPEGDeviceSource::createNew(envir(), clientSessionId, fchannel, timerfdInterval);
	}

	fprintf(stderr, "%s : unknown encode type %d\n", __func__, p_venc_attr.type);
	return NULL;
}

RTPSink *DeviceServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
                                                       FramedSource * /*inputSource*/)
{
	int ret = 0;
	RTPSink* sink = NULL;
	unsigned vps_size;
	unsigned sps_size;
	unsigned pps_size;
	u_int8_t sps_remove_emulation[128];

	MPI_BCHN bchn;
	MPI_STREAM_PARAMS_V2_S param;

	MPI_ECHN chn_idx;
	chn_idx.chn = fchannel;
	MPI_VENC_ATTR_S p_venc_attr;
	ret = MPI_ENC_getVencAttr(chn_idx, &p_venc_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_ENC_getVencAttr %d.\n", chn_idx.chn);
		return NULL;
	}


	if (p_venc_attr.type == MPI_VENC_TYPE_H264 || p_venc_attr.type == MPI_VENC_TYPE_H265) {
		/*create RTP sink with SPS/PPS/VPS ready, so sink don't need to get them from source:getbitstream()*/
		ret = MPI_initBitStreamSystem();
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "failed to init bitstream system:%d\n", ret);
			goto failed;
		}

		bchn = MPI_createBitStreamChn(chn_idx);
		if (!VALID_MPI_ENC_BCHN(bchn)) {
			fprintf(stderr, "Failed to create bitstream channel %d.\n", chn_idx.chn);
			goto failed;
		}
		ret = MPI_ENC_requestIdr(chn_idx);
get_source:
		ret = MPI_getBitStreamV2(bchn, &param, 10000 /*ms*/);

		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to get stream param at %lu, %d\r\n", (unsigned long)chn_idx.value, ret);
			goto get_source;
		}

		if (param.seg[0].type != MPI_FRAME_TYPE_SPS) {
			ret = MPI_releaseBitStreamV2(bchn, &param);
			printf("not find sps\n");
			goto get_source;
		}

		if (p_venc_attr.type == MPI_VENC_TYPE_H264) {
			sps_size = param.seg[0].size - 4;
			pps_size =  param.seg[1].size - 4;
			
			removeH264or5EmulationBytes(sps_remove_emulation, 128, (u_int8_t*)param.seg[0].uaddr + 4, sps_size);

			printf("create H264 rtp sink with SPS\n");
			sink  = H264VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic, 
            (u_int8_t*)param.seg[0].uaddr + 4, sps_size /*sps*/,
            (u_int8_t*)param.seg[1].uaddr + 4, pps_size/*pps*/, 
           (sps_remove_emulation[1] << 16) | (sps_remove_emulation[2] << 8) | sps_remove_emulation[3]/*profile level id*/);
		}

		if (p_venc_attr.type == MPI_VENC_TYPE_H265) {
			vps_size = 24;
			if (param.seg[0].size != 75) {
				sps_size = param.seg[0].size - (4 + 24 + 4 + 4 + 7);
			} else {
				sps_size = 32;
			}
			pps_size = 7;

			/*these all include in VPS, check format:https://www.jianshu.com/p/1eb281a612bb*/
			removeH264or5EmulationBytes(sps_remove_emulation, 128, (u_int8_t*)param.seg[0].uaddr + 4 + vps_size + 4, sps_size);
			u_int8_t profileTierLevelHeaderBytes[12];
			memcpy(profileTierLevelHeaderBytes, &sps_remove_emulation[3], 12);

			u_int8_t profile_space = profileTierLevelHeaderBytes[0] >> 6;
			u_int8_t tier_flag = (profileTierLevelHeaderBytes[0] >> 5) & 0x01;
			u_int8_t level_id = profileTierLevelHeaderBytes[11];
			u_int8_t const *interop_constraints = &profileTierLevelHeaderBytes[3];
			char buf[32];
			snprintf(buf, 32, "%02X%02X%02X%02X%02X%02X", interop_constraints[0], interop_constraints[1],
					interop_constraints[2], interop_constraints[3], interop_constraints[4], interop_constraints[5]);

			printf("create H265 rtp sink with SPS\n");
			sink = H265VideoRTPSink::createNew(envir(), rtpGroupsock, rtpPayloadTypeIfDynamic,
                                              (u_int8_t*)param.seg[0].uaddr + 4, vps_size, 
                                              (u_int8_t*)param.seg[0].uaddr + 4 + 24 + 4,  sps_size, 
                                              (u_int8_t*)param.seg[0].uaddr + sps_size + 36, pps_size,
                                              profile_space, p_venc_attr.h265.profile, tier_flag,
                                              level_id, &buf[0]);
		}

		ret = MPI_releaseBitStreamV2(bchn, &param);
		ret = MPI_destroyBitStreamChn(bchn);

		return sink;
	}

	if (p_venc_attr.type == MPI_VENC_TYPE_MJPEG) { /*MJPEG format no need auxSDP*/
		return JPEGVideoRTPSink::createNew(envir(), rtpGroupsock);
	}
failed:
	fprintf(stderr, "%s : unknown encode type %d\n", __func__, p_venc_attr.type);
	return NULL;
}

static void checkForAuxSDPLine(void *clientData)
{
	DeviceServerMediaSubsession *subsess = (DeviceServerMediaSubsession *)clientData;
	subsess->checkForAuxSDPLine1();
}

void DeviceServerMediaSubsession::checkForAuxSDPLine1()
{
	char const *dasl;

	if (fAuxSDPLine != NULL) {
		// Signal the event loop that we're done:
		setDoneFlag();
	} else if (fDummyRTPSink != NULL && (dasl = fDummyRTPSink->auxSDPLine()) != NULL) {
		fAuxSDPLine = strDup(dasl);
		fDummyRTPSink = NULL;

		// Signal the event loop that we're done:
		setDoneFlag();
	} else if (!fDoneFlag) {
		// try again after a brief delay:
		int uSecsToDelay = 100000; // 100 ms
		if ((int)nextTask() == 0 /*have not created own timerfd*/) {
			nextTask() =
		        envir().taskScheduler().scheduleDelayedTask(uSecsToDelay, (TaskFunc *)checkForAuxSDPLine, this);
		}
		
	}
}
#if 0
/*no need to generate SDP by open source class, so don't need these handler*/
static void afterPlayingDummy(void *clientData)
{
	DeviceServerMediaSubsession *subsess = (DeviceServerMediaSubsession *)clientData;
	subsess->afterPlayingDummy1();
}

void DeviceServerMediaSubsession::afterPlayingDummy1()
{
	// Unschedule any pending 'checking' task:
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	// Signal the event loop that we're done:
	setDoneFlag();
}
#endif

char const *DeviceServerMediaSubsession::getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource)
{
	int ret;
	MPI_ECHN chn_idx;
	chn_idx.chn = fchannel;
	MPI_VENC_ATTR_S p_venc_attr;
	ret = MPI_ENC_getVencAttr(chn_idx, &p_venc_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Failed to MPI_ENC_getVencAttr %d.\n", chn_idx.chn);
	}

	if (p_venc_attr.type == MPI_VENC_TYPE_MJPEG) {
		fDummyRTPSink = rtpSink;
		/*SDP info is given when create sink, so auxSDPLine can generate by sink with out open source*/
		return fDummyRTPSink->auxSDPLine();
	} else {
		if (fAuxSDPLine != NULL)
			return fAuxSDPLine; // it's already been set up (for a previous client)

		if (fDummyRTPSink == NULL) { // we're not already setting it up for another, concurrent stream
			// Note: For H264 video files, the 'config' information ("profile-level-id"
			// and "sprop-parameter-sets") isn't known
			// until we start reading the file.  This means that "rtpSink"s
			// "auxSDPLine()" will be NULL initially,
			// and we need to start reading data from our file until this changes.
			fDummyRTPSink = rtpSink;
#if 0
			/*sink generate RTP not from source class*/
			fDummyRTPSink->startPlaying(*inputSource, afterPlayingDummy, this);
#endif
			// Start reading the file:
			// Check whether the sink's 'auxSDPLine()' is ready:
			checkForAuxSDPLine(this);
		}

		envir().taskScheduler().doEventLoop(&fDoneFlag);

		return fAuxSDPLine;
	}
}
