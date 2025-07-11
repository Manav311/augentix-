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
// RTP sink for H.264 video (RFC 3984)
// Implementation

#include "H264VideoRTPSink.hh"
#include "Base64.hh"
#include "H264VideoRTPSource.hh" // for "parseSPropParameterSets()"
#include "H264VideoStreamFramer.hh"
#include <unistd.h>

////////// H264VideoRTPSink implementation //////////

H264VideoRTPSink::H264VideoRTPSink(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                   u_int8_t const *sps, unsigned spsSize, u_int8_t const *pps, unsigned ppsSize,
                                   unsigned profile_level_id)
        : H264or5VideoRTPSink(264, env, RTPgs, rtpPayloadFormat, NULL, 0, sps, spsSize, pps, ppsSize)
        , fProfileLevelId(profile_level_id)
{
}

H264VideoRTPSink::~H264VideoRTPSink()
{
}

H264VideoRTPSink *H264VideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat)
{
	return new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

H264VideoRTPSink *H264VideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                              u_int8_t const *sps, unsigned spsSize, u_int8_t const *pps,
                                              unsigned ppsSize, unsigned profile_level_id)
{
	return new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat, sps, spsSize, pps, ppsSize, profile_level_id);
}

H264VideoRTPSink *H264VideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                              char const *sPropParameterSetsStr, unsigned profile_level_id)
{
	u_int8_t *sps = NULL;
	unsigned spsSize = 0;
	u_int8_t *pps = NULL;
	unsigned ppsSize = 0;

	unsigned numSPropRecords;
	SPropRecord *sPropRecords = parseSPropParameterSets(sPropParameterSetsStr, numSPropRecords);
	for (unsigned i = 0; i < numSPropRecords; ++i) {
		if (sPropRecords[i].sPropLength == 0)
			continue; // bad data
		u_int8_t nal_unit_type = (sPropRecords[i].sPropBytes[0]) & 0x1F;
		if (nal_unit_type == 7 /*SPS*/) {
			sps = sPropRecords[i].sPropBytes;
			spsSize = sPropRecords[i].sPropLength;
		} else if (nal_unit_type == 8 /*PPS*/) {
			pps = sPropRecords[i].sPropBytes;
			ppsSize = sPropRecords[i].sPropLength;
		}
	}

	H264VideoRTPSink *result =
	        new H264VideoRTPSink(env, RTPgs, rtpPayloadFormat, sps, spsSize, pps, ppsSize, profile_level_id);
	delete[] sPropRecords;

	return result;
}

Boolean H264VideoRTPSink::sourceIsCompatibleWithUs(MediaSource &source)
{
	// Our source must be an appropriate framer:
	return source.isH264VideoStreamFramer();
}

char const *H264VideoRTPSink::auxSDPLine()
{
	// Generate a new "a=fmtp:" line each time, using our SPS and PPS (if we have
	// them),
	// otherwise parameters from our framer source (in case they've changed since
	// the last time that
	// we were called):
	H264or5VideoStreamFramer *framerSource = NULL;
	u_int8_t *vpsDummy = NULL;
	unsigned vpsDummySize = 0;
	u_int8_t *sps = fSPS;
	unsigned spsSize = fSPSSize;
	u_int8_t *pps = fPPS;
	unsigned ppsSize = fPPSSize;
	if (sps == NULL || pps == NULL) { /*fSPS & fPPS has given when sink created, so don't need to get them from input source*/
		// We need to get SPS and PPS from our framer source:
		if (fOurFragmenter == NULL)
			return NULL; // we don't yet have a fragmenter (and therefore not a
			        // source)
		framerSource = (H264or5VideoStreamFramer *)(fOurFragmenter->inputSource());
		if (framerSource == NULL)
			return NULL; // we don't yet have a source

		framerSource->getVPSandSPSandPPS(vpsDummy, vpsDummySize, sps, spsSize, pps, ppsSize);
		if (spsSize == 0 || ppsSize == 0)
			return NULL; // our source isn't ready

		fProfileLevelId = framerSource->profileLevelId();
	}

	// Set up the "a=fmtp:" SDP line for this stream:
	char *sps_base64 = base64Encode((char *)sps, spsSize);
	char *pps_base64 = base64Encode((char *)pps, ppsSize);

	char const *fmtpFmt = "a=fmtp:%d packetization-mode=1"
	                      ";profile-level-id=%06X"
	                      ";sprop-parameter-sets=%s,%s\r\n";
	unsigned fmtpFmtSize = strlen(fmtpFmt) + 3 /* max char len */
	                       + 6 /* 3 bytes in hex */
	                       + strlen(sps_base64) + strlen(pps_base64);
	char *fmtp = new char[fmtpFmtSize];
	sprintf(fmtp, fmtpFmt, rtpPayloadType(), fProfileLevelId, sps_base64, pps_base64);

	delete[] sps_base64;
	delete[] pps_base64;

	delete[] fFmtpSDPLine;
	fFmtpSDPLine = fmtp;
	return fFmtpSDPLine;
}

static unsigned const rtpHeaderSize = 12;
void H264VideoRTPSink::sendPacketIfNecessary()
{
	if (fNumFramesUsedSoFar > 0) {
		if (!fRTPInterface.sendPacket(fOutBuf->packet(), fOutBuf->curPacketSize())) {
			// if failure handler has been specified, call it
			if (fOnSendErrorFunc != NULL)
				(*fOnSendErrorFunc)(fOnSendErrorData);
		}
		++fPacketCount;
		fTotalOctetCount += fOutBuf->curPacketSize();
		fOctetCount +=
		        fOutBuf->curPacketSize() - rtpHeaderSize - fSpecialHeaderSize - fTotalFrameSpecificHeaderSizes;

		++fSeqNo; // for next time
	}

	if (fOutBuf->haveOverflowData() && fOutBuf->totalBytesAvailable() > fOutBuf->totalBufferSize() / 2) {
		// Efficiency hack: Reset the packet start pointer to just in front of
		// the overflow data (allowing for the RTP header and special headers),
		// so that we probably don't have to "memmove()" the overflow data
		// into place when building the next packet:
		unsigned newPacketStart =
		        fOutBuf->curPacketSize() - (rtpHeaderSize + fSpecialHeaderSize + frameSpecificHeaderSize());
		fOutBuf->adjustPacketStart(newPacketStart);
	} else {
		// Normal case: Reset the packet start pointer back to the start:
		fOutBuf->resetPacketStart();
	}
	fOutBuf->resetOffset();
	fNumFramesUsedSoFar = 0;

	if (fNoFramesLeft) {
		// We're done:
		onSourceClosure();
	} else {
		// We have more frames left to send.  Figure out when the next frame
		// is due to start playing, then make sure that we wait this long before
		// sending the next packet.
		/*don't re-schedule here, just send until packFrame() and find sour in empty*/
		MultiFramedRTPSink::sendNext(this);
	}
}
