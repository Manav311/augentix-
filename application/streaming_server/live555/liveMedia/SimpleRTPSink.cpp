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
// A simple RTP sink that packs frames into each outgoing
//     packet, without any fragmentation or special headers.
// Implementation

#include "SimpleRTPSink.hh"

SimpleRTPSink::SimpleRTPSink(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                             unsigned rtpTimestampFrequency, char const *sdpMediaTypeString,
                             char const *rtpPayloadFormatName, unsigned numChannels,
                             Boolean allowMultipleFramesPerPacket, Boolean doNormalMBitRule, int interval)
        : MultiFramedRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, rtpPayloadFormatName, numChannels)
        , fAllowMultipleFramesPerPacket(allowMultipleFramesPerPacket)
        , fSetMBitOnNextPacket(False)
		, fInterval(interval)
{
	fSDPMediaTypeString = strDup(sdpMediaTypeString == NULL ? "unknown" : sdpMediaTypeString);
	fSetMBitOnLastFrames = doNormalMBitRule && strcmp(fSDPMediaTypeString, "audio") != 0;
}

SimpleRTPSink::~SimpleRTPSink()
{
	delete[](char *) fSDPMediaTypeString;
}

SimpleRTPSink *SimpleRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
                                        unsigned rtpTimestampFrequency, char const *sdpMediaTypeString,
                                        char const *rtpPayloadFormatName, unsigned numChannels,
                                        Boolean allowMultipleFramesPerPacket, Boolean doNormalMBitRule, int interval)
{
	return new SimpleRTPSink(env, RTPgs, rtpPayloadFormat, rtpTimestampFrequency, sdpMediaTypeString,
	                         rtpPayloadFormatName, numChannels, allowMultipleFramesPerPacket, doNormalMBitRule, interval);
}

void SimpleRTPSink::doSpecialFrameHandling(unsigned fragmentationOffset, unsigned char *frameStart,
                                           unsigned numBytesInFrame, struct timeval framePresentationTime,
                                           unsigned numRemainingBytes)
{
	if (numRemainingBytes == 0) {
		// This packet contains the last (or only) fragment of the frame.
		// Set the RTP 'M' ('marker') bit, if appropriate:
		if (fSetMBitOnLastFrames)
			setMarkerBit();
	}
	if (fSetMBitOnNextPacket) {
		// An external object has asked for the 'M' bit to be set on the next
		// packet:
		setMarkerBit();
		fSetMBitOnNextPacket = False;
	}

	// Important: Also call our base class's doSpecialFrameHandling(),
	// to set the packet's timestamp:
	MultiFramedRTPSink::doSpecialFrameHandling(fragmentationOffset, frameStart, numBytesInFrame,
	                                           framePresentationTime, numRemainingBytes);
}

Boolean SimpleRTPSink::frameCanAppearAfterPacketStart(unsigned char const * /*frameStart*/,
                                                      unsigned /*numBytesInFrame*/) const
{
	return fAllowMultipleFramesPerPacket;
}

char const *SimpleRTPSink::sdpMediaType() const
{
	return fSDPMediaTypeString;
}


static unsigned const rtpHeaderSize = 12;
void SimpleRTPSink::sendPacketIfNecessary() {

	if (fNumFramesUsedSoFar > 0) {
// Send the packet:
#ifdef TEST_LOSS
		if ((our_random() % 10) != 0) // simulate 10% packet loss #####
#endif
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
		/*don't re-schedule here, just send until packFrame() and find sour in empty*/
		MultiFramedRTPSink::sendNext(this);
	}
}

