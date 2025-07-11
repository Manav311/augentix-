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
// RTP sink for H.265 video
// Implementation

#include "H265VideoRTPSink.hh"
#include "Base64.hh"
#include "BitVector.hh"
#include "H264VideoRTPSource.hh" // for "parseSPropParameterSets()"
#include "H265VideoStreamFramer.hh"
#include <malloc.h>

////////// H265VideoRTPSink implementation //////////

H265VideoRTPSink::H265VideoRTPSink(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                   u_int8_t const *vps, unsigned vpsSize, u_int8_t const *sps, unsigned spsSize,
                                   u_int8_t const *pps, unsigned ppsSize, unsigned profileSpace, unsigned profileId,
                                   unsigned tierFlag, unsigned levelId, char const *interopConstraintsStr)
        : H264or5VideoRTPSink(265, env, RTPgs, rtpPayloadFormat, vps, vpsSize, sps, spsSize, pps, ppsSize)
        , fProfileSpace(profileSpace)
        , fProfileId(profileId)
        , fTierFlag(tierFlag)
        , fLevelId(levelId)
        , fInteropConstraintsStr(strDup(interopConstraintsStr))
{
}

H265VideoRTPSink::~H265VideoRTPSink()
{
	delete[] fInteropConstraintsStr;
}

H265VideoRTPSink *H265VideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat)
{
	return new H265VideoRTPSink(env, RTPgs, rtpPayloadFormat);
}

H265VideoRTPSink *H265VideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                              u_int8_t const *vps, unsigned vpsSize, u_int8_t const *sps,
                                              unsigned spsSize, u_int8_t const *pps, unsigned ppsSize,
                                              unsigned profileSpace, unsigned profileId, unsigned tierFlag,
                                              unsigned levelId, char const *interopConstraintsStr)
{
	return new H265VideoRTPSink(env, RTPgs, rtpPayloadFormat, vps, vpsSize, sps, spsSize, pps, ppsSize,
	                            profileSpace, profileId, tierFlag, levelId, interopConstraintsStr);
}

H265VideoRTPSink *H265VideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                              char const *sPropVPSStr, char const *sPropSPSStr, char const *sPropPPSStr,
                                              unsigned profileSpace, unsigned profileId, unsigned tierFlag,
                                              unsigned levelId, char const *interopConstraintsStr)
{
	u_int8_t *vps = NULL;
	unsigned vpsSize = 0;
	u_int8_t *sps = NULL;
	unsigned spsSize = 0;
	u_int8_t *pps = NULL;
	unsigned ppsSize = 0;

	// Parse each 'sProp' string, extracting and then classifying the NAL unit(s)
	// from each one.
	// We're 'liberal in what we accept'; it's OK if the strings don't contain the
	// NAL unit type
	// implied by their names (or if one or more of the strings encode multiple
	// NAL units).
	SPropRecord *sPropRecords[3];
	unsigned numSPropRecords[3];
	sPropRecords[0] = parseSPropParameterSets(sPropVPSStr, numSPropRecords[0]);
	sPropRecords[1] = parseSPropParameterSets(sPropSPSStr, numSPropRecords[1]);
	sPropRecords[2] = parseSPropParameterSets(sPropPPSStr, numSPropRecords[2]);

	for (unsigned j = 0; j < 3; ++j) {
		SPropRecord *records = sPropRecords[j];
		unsigned numRecords = numSPropRecords[j];

		for (unsigned i = 0; i < numRecords; ++i) {
			if (records[i].sPropLength == 0)
				continue; // bad data
			u_int8_t nal_unit_type = ((records[i].sPropBytes[0]) & 0x7E) >> 1;
			if (nal_unit_type == 32 /*VPS*/) {
				vps = records[i].sPropBytes;
				vpsSize = records[i].sPropLength;
			} else if (nal_unit_type == 33 /*SPS*/) {
				sps = records[i].sPropBytes;
				spsSize = records[i].sPropLength;
			} else if (nal_unit_type == 34 /*PPS*/) {
				pps = records[i].sPropBytes;
				ppsSize = records[i].sPropLength;
			}
		}
	}

	H265VideoRTPSink *result = new H265VideoRTPSink(env, RTPgs, rtpPayloadFormat, vps, vpsSize, sps, spsSize, pps,
	                                                ppsSize, profileSpace, profileId, tierFlag, levelId,
	                                                interopConstraintsStr);
	delete[] sPropRecords[0];
	delete[] sPropRecords[1];
	delete[] sPropRecords[2];

	return result;
}

Boolean H265VideoRTPSink::sourceIsCompatibleWithUs(MediaSource &source)
{
	// Our source must be an appropriate framer:
	return source.isH265VideoStreamFramer();
}

char const *H265VideoRTPSink::auxSDPLine()
{
	// Generate a new "a=fmtp:" line each time, using our VPS, SPS and PPS (if we
	// have them),
	// otherwise parameters from our framer source (in case they've changed since
	// the last time that
	// we were called):
	H264or5VideoStreamFramer *framerSource = NULL;
	u_int8_t *vps = fVPS;
	unsigned vpsSize = fVPSSize;
	u_int8_t *sps = fSPS;
	unsigned spsSize = fSPSSize;
	u_int8_t *pps = fPPS;
	unsigned ppsSize = fPPSSize;
	if (vps == NULL || sps == NULL || pps == NULL || fInteropConstraintsStr == NULL) {
		/*fSPS & fPPS has given when sink created, so don't need to get them from input source*/
		// We need to get VPS, SPS and PPS from our framer source:
		if (fOurFragmenter == NULL)
			return NULL; // we don't yet have a fragmenter (and therefore not a
			        // source)
		framerSource = (H264or5VideoStreamFramer *)(fOurFragmenter->inputSource());
		if (framerSource == NULL)
			return NULL; // we don't yet have a source

		framerSource->getVPSandSPSandPPS(vps, vpsSize, sps, spsSize, pps, ppsSize);
		if (vpsSize == 0 || spsSize == 0 || ppsSize == 0)
			return NULL; // our source isn't ready

		// Extract from our upstream framer's 'profile_tier_level' bytes
		// several parameters that we'll put in this line:
		u_int8_t const *profileTierLevelHeaderBytes = framerSource->profileTierLevelHeaderBytes();
		fProfileSpace = profileTierLevelHeaderBytes[0] >> 6; // general_profile_space
		fProfileId = profileTierLevelHeaderBytes[0] & 0x1F; // general_profile_idc
		fTierFlag = (profileTierLevelHeaderBytes[0] >> 5) & 0x1; // general_tier_flag
		fLevelId = profileTierLevelHeaderBytes[11]; // general_level_idc
		u_int8_t const *interop_constraints = &profileTierLevelHeaderBytes[5];
		char buf[100];
		sprintf(buf, "%02X%02X%02X%02X%02X%02X", interop_constraints[0], interop_constraints[1],
		        interop_constraints[2], interop_constraints[3], interop_constraints[4], interop_constraints[5]);
		delete[] fInteropConstraintsStr;
		fInteropConstraintsStr = strDup(buf);
	}

	// Set up the "a=fmtp:" SDP line for this stream.
	char *sprop_vps = base64Encode((char *)vps, vpsSize);
	char *sprop_sps = base64Encode((char *)sps, spsSize);
	char *sprop_pps = base64Encode((char *)pps, ppsSize);

	char const *fmtpFmt = "a=fmtp:%d profile-space=%u"
	                      ";profile-id=%u"
	                      ";tier-flag=%u"
	                      ";level-id=%u"
	                      ";interop-constraints=%s"
	                      ";sprop-vps=%s"
	                      ";sprop-sps=%s"
	                      ";sprop-pps=%s\r\n";
	unsigned fmtpFmtSize =
	        strlen(fmtpFmt) + 3 /* max num chars: rtpPayloadType */ + 20 /* max num chars: profile_space */
	        + 20 /* max num chars: profile_id */
	        + 20 /* max num chars: tier_flag */
	        + 20 /* max num chars: level_id */
	        + strlen(fInteropConstraintsStr) + strlen(sprop_vps) + strlen(sprop_sps) + strlen(sprop_pps);
	char *fmtp = new char[fmtpFmtSize];
	sprintf(fmtp, fmtpFmt, rtpPayloadType(), fProfileSpace, fProfileId, fTierFlag, fLevelId, fInteropConstraintsStr,
	        sprop_vps, sprop_sps, sprop_pps);

	delete[] sprop_vps;
	delete[] sprop_sps;
	delete[] sprop_pps;

	delete[] fFmtpSDPLine;
	fFmtpSDPLine = fmtp;
	return fFmtpSDPLine;
}

static unsigned const rtpHeaderSize = 12;
void H265VideoRTPSink::sendPacketIfNecessary()
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
