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
// RTP sink for H.264 or H.265 video
// Implementation

#include "H264or5VideoRTPSink.hh"
#include "H264or5VideoStreamFramer.hh"

#include <iostream>
using namespace std;

////////// H264or5Fragmenter definition //////////

// Because of the ideosyncracies of the H.264 RTP payload format, we implement
// "H264or5VideoRTPSink" using a separate "H264or5Fragmenter" class that
// delivers,
// to the "H264or5VideoRTPSink", only fragments that will fit within an outgoing
// RTP packet.  I.e., we implement fragmentation in this separate
// "H264or5Fragmenter"
// class, rather than in "H264or5VideoRTPSink".
// (Note: This class should be used only by "H264or5VideoRTPSink", or a
// subclass.)
#if 0
class H264or5Fragmenter : public FramedFilter {
    public:
	H264or5Fragmenter(int hNumber, UsageEnvironment &env, FramedSource *inputSource, unsigned inputBufferMax,
	                  unsigned maxOutputPacketSize);
	virtual ~H264or5Fragmenter();

	Boolean lastFragmentCompletedNALUnit() const
	{
		return fLastFragmentCompletedNALUnit;
	}

    private: // redefined virtual functions:
	virtual void doGetNextFrame();

    private:
	static void afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
	                              struct timeval presentationTime, unsigned durationInMicroseconds);
	void afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes, struct timeval presentationTime,
	                        unsigned durationInMicroseconds);

    public:
	unsigned fNumValidDataBytes;

    private:
	int fHNumber;
	unsigned fInputBufferSize;
	unsigned fMaxOutputPacketSize;
	unsigned char *fInputBuffer;
	unsigned fCurDataOffset;
	unsigned fSaveNumTruncatedBytes;
	Boolean fLastFragmentCompletedNALUnit;
};
#endif
////////// H264or5VideoRTPSink implementation //////////

H264or5VideoRTPSink::H264or5VideoRTPSink(int hNumber, UsageEnvironment &env, Groupsock *RTPgs,
                                         unsigned char rtpPayloadFormat, u_int8_t const *vps, unsigned vpsSize,
                                         u_int8_t const *sps, unsigned spsSize, u_int8_t const *pps, unsigned ppsSize)
        : VideoRTPSink(env, RTPgs, rtpPayloadFormat, 90000, hNumber == 264 ? "H264" : "H265")
        , fHNumber(hNumber)
        , fOurFragmenter(NULL)
        , fFmtpSDPLine(NULL)
{
	if (vps != NULL) {
		fVPSSize = vpsSize;
		fVPS = (u_int8_t *)vps;

	} else {
		fVPSSize = 0;
		fVPS = NULL;
	}
	if (sps != NULL) {
		fSPSSize = spsSize;
		fSPS = (u_int8_t *)sps;

	} else {
		fSPSSize = 0;
		fSPS = NULL;
	}
	if (pps != NULL) {
		fPPSSize = ppsSize;
		fPPS = (u_int8_t *)pps;
	} else {
		fPPSSize = 0;
		fPPS = NULL;
	}
}

H264or5VideoRTPSink::~H264or5VideoRTPSink()
{
	fSource = fOurFragmenter; // hack: in case "fSource" had gotten set to NULL
	        // before we were called
	delete[] fFmtpSDPLine;

	fVPS = NULL;
	fSPS = NULL;
	fPPS = NULL;

	stopPlaying(); // call this now, because we won't have our 'fragmenter' when
	        // the base class destructor calls it later.

	// Close our 'fragmenter' as well:
	Medium::close(fOurFragmenter);
	fSource = NULL; // for the base class destructor, which gets called next
}

Boolean H264or5VideoRTPSink::continuePlaying()
{
	// First, check whether we have a 'fragmenter' class set up yet.
	// If not, create it now:
	if (fOurFragmenter == NULL) {
		fOurFragmenter = new H264or5Fragmenter(fHNumber, envir(), fSource, OutPacketBuffer::maxSize,
		                                       ourMaxPacketSize() - 12 /*RTP hdr size*/);
	} else {
		fOurFragmenter->reassignInputSource(fSource);
	}
	fSource = fOurFragmenter;

	// Then call the parent class's implementation:
	return MultiFramedRTPSink::continuePlaying();
}

void H264or5VideoRTPSink::doSpecialFrameHandling(unsigned /*fragmentationOffset*/, unsigned char * /*frameStart*/,
                                                 unsigned /*numBytesInFrame*/, struct timeval framePresentationTime,
                                                 unsigned /*numRemainingBytes*/)
{
	// Set the RTP 'M' (marker) bit iff
	// 1/ The most recently delivered fragment was the end of (or the only
	// fragment of) an NAL unit, and
	// 2/ This NAL unit was the last NAL unit of an 'access unit' (i.e. video
	// frame).
	if (fOurFragmenter != NULL) {
		H264or5VideoStreamFramer *framerSource = (H264or5VideoStreamFramer *)(fOurFragmenter->inputSource());
		// This relies on our fragmenter's source being a
		// "H264or5VideoStreamFramer".
		if (((H264or5Fragmenter *)fOurFragmenter)->lastFragmentCompletedNALUnit() && framerSource != NULL &&
		    framerSource->pictureEndMarker()) {
			setMarkerBit();
			framerSource->pictureEndMarker() = False;
		}
	}

	setTimestamp(framePresentationTime);
}

static unsigned const rtpHeaderSize = 12;
void H264or5VideoRTPSink::sendPacketIfNecessary()
{
	printf("send in H264 or 5 VideoRTP\r\n");
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
		if (fOutBuf->haveOverflowData() /*for MJPEG*/ ||
		    (((H264or5Fragmenter *)fOurFragmenter)->fNumValidDataBytes != 1)) {
			MultiFramedRTPSink::sendNext(this);
		} else {
			nextTask() = envir().taskScheduler().scheduleDelayedTask(
			        0, (TaskFunc *)MultiFramedRTPSink::sendNext, this);
		}
	}
}

Boolean H264or5VideoRTPSink::frameCanAppearAfterPacketStart(unsigned char const * /*frameStart*/,
                                                            unsigned /*numBytesInFrame*/) const
{
	return False;
}

////////// H264or5Fragmenter implementation //////////

H264or5Fragmenter::H264or5Fragmenter(int hNumber, UsageEnvironment &env, FramedSource *inputSource,
                                     unsigned inputBufferMax, unsigned maxOutputPacketSize)
        : FramedFilter(env, inputSource)
        , fNumValidDataBytes(1)
        , fHNumber(hNumber)
        , fInputBufferSize(inputBufferMax + 1)
        , fMaxOutputPacketSize(maxOutputPacketSize)
        , fCurDataOffset(1)
        , fSaveNumTruncatedBytes(0)
        , fLastFragmentCompletedNALUnit(True)
{
	fInputBuffer = new unsigned char[fInputBufferSize];
#if DEBUG
	printf("H264or5Fragmenter: inputBufferMax = %d, maxOutputPacketSize = %d\n", inputBufferMax,
	       maxOutputPacketSize);
#endif
	fprintf(stderr, "create H264or5Fragmenter");
}

H264or5Fragmenter::~H264or5Fragmenter()
{
	delete[] fInputBuffer;
	detachInputSource(); // so that the subsequent ~FramedFilter() doesn't delete
	        // it
}

void H264or5Fragmenter::doGetNextFrame()
{
	if (fNumValidDataBytes == 1) {
		// We have no NAL unit data currently in the buffer.  Read a new one:
		fInputSource->getNextFrame(&fInputBuffer[1], fInputBufferSize - 1, afterGettingFrame, this,
		                           FramedSource::handleClosure, this);
	} else {
		// We have NAL unit data in the buffer.  There are three cases to consider:
		// 1. There is a new NAL unit in the buffer, and it's small enough to
		// deliver
		//    to the RTP sink (as is).
		// 2. There is a new NAL unit in the buffer, but it's too large to deliver
		// to
		//    the RTP sink in its entirety.  Deliver the first fragment of this
		//    data,
		//    as a FU packet, with one extra preceding header byte (for the "FU
		//    header").
		// 3. There is a NAL unit in the buffer, and we've already delivered some
		//    fragment(s) of this.  Deliver the next fragment of this data,
		//    as a FU packet, with two (H.264) or three (H.265) extra preceding
		//    header bytes
		//    (for the "NAL header" and the "FU header").

		if (fMaxSize < fMaxOutputPacketSize) { // shouldn't happen
			envir() << "H264or5Fragmenter::doGetNextFrame(): fMaxSize (" << fMaxSize
			        << ") is smaller than expected\n";
		} else {
			fMaxSize = fMaxOutputPacketSize;
		}

		fLastFragmentCompletedNALUnit = True; // by default
		if (fCurDataOffset == 1) { // case 1 or 2
			if (fNumValidDataBytes - 1 <= fMaxSize) { // case 1
				memcpy(fTo, &fInputBuffer[1], fNumValidDataBytes - 1);
				//printf("case 1\n");
				fFrameSize = fNumValidDataBytes - 1;
				fCurDataOffset = fNumValidDataBytes;
			} else { // case 2
				// We need to send the NAL unit data as FU packets.  Deliver the first
				// packet now.  Note that we add "NAL header" and "FU header" bytes to
				// the front
				// of the packet (overwriting the existing "NAL header").
				//memcpy(&fInputBuffer[1], fTo, fMaxSize);
				if (fHNumber == 264) {
					fInputBuffer[0] = (fInputBuffer[1] & 0xE0) | 28; // FU indicator
					fInputBuffer[1] = 0x80 | (fInputBuffer[1] & 0x1F); // FU header (with S bit)
				} else { // 265
					u_int8_t nal_unit_type = (fInputBuffer[1] & 0x7E) >> 1;
					fInputBuffer[0] = (fInputBuffer[1] & 0x81) |
					                  (49 << 1); // Payload header (1st byte)
					fInputBuffer[1] = fInputBuffer[2]; // Payload header (2nd byte)
					fInputBuffer[2] = 0x80 | nal_unit_type; // FU header (with S bit)
				}
				memcpy(fTo, fInputBuffer, fMaxSize);

				fFrameSize = fMaxSize;
				fCurDataOffset += fMaxSize - 1;
				fLastFragmentCompletedNALUnit = False;
			}
		} else { // case 3
			//memcpy(&fInputBuffer[1], fTo, fMaxSize);
			// We are sending this NAL unit data as FU packets.  We've already sent
			// the
			// first packet (fragment).  Now, send the next fragment.  Note that we
			// add
			// "NAL header" and "FU header" bytes to the front.  (We reuse these bytes
			// that
			// we already sent for the first fragment, but clear the S bit, and add
			// the E
			// bit if this is the last fragment.)
			unsigned numExtraHeaderBytes;
			if (fHNumber == 264) {
				fInputBuffer[fCurDataOffset - 2] = fInputBuffer[0]; // FU indicator
				fInputBuffer[fCurDataOffset - 1] = fInputBuffer[1] & ~0x80; // FU header (no S bit)
				numExtraHeaderBytes = 2;
			} else { // 265
				fInputBuffer[fCurDataOffset - 3] = fInputBuffer[0]; // Payload header (1st byte)
				fInputBuffer[fCurDataOffset - 2] = fInputBuffer[1]; // Payload header (2nd byte)
				fInputBuffer[fCurDataOffset - 1] = fInputBuffer[2] & ~0x80; // FU header (no S bit)
				numExtraHeaderBytes = 3;
			}
			unsigned numBytesToSend = numExtraHeaderBytes + (fNumValidDataBytes - fCurDataOffset);
			if (numBytesToSend > fMaxSize) {
				// We can't send all of the remaining data this time:
				numBytesToSend = fMaxSize;
				fLastFragmentCompletedNALUnit = False;
			} else {
				// This is the last fragment:
				fInputBuffer[fCurDataOffset - 1] |= 0x40; // set the E bit in the FU header
				fNumTruncatedBytes = fSaveNumTruncatedBytes;
			}

			memcpy(fTo, &fInputBuffer[fCurDataOffset - numExtraHeaderBytes], numBytesToSend);

			fFrameSize = numBytesToSend;
			fCurDataOffset += numBytesToSend - numExtraHeaderBytes;
		}

		if (fCurDataOffset >= fNumValidDataBytes) {
			// We're done with this data.  Reset the pointers for receiving new data:
			fNumValidDataBytes = fCurDataOffset = 1;
		}

		// Complete delivery to the client:
		FramedSource::afterGetting(this);
	}
}

void H264or5Fragmenter::afterGettingFrame(void *clientData, unsigned frameSize, unsigned numTruncatedBytes,
                                          struct timeval presentationTime, unsigned durationInMicroseconds)
{
	H264or5Fragmenter *fragmenter = (H264or5Fragmenter *)clientData;
	fragmenter->afterGettingFrame1(frameSize, numTruncatedBytes, presentationTime, durationInMicroseconds);
}

void H264or5Fragmenter::afterGettingFrame1(unsigned frameSize, unsigned numTruncatedBytes,
                                           struct timeval presentationTime, unsigned durationInMicroseconds)
{
	fNumValidDataBytes += frameSize;
	fSaveNumTruncatedBytes = numTruncatedBytes;
	fPresentationTime = presentationTime;
	fDurationInMicroseconds = durationInMicroseconds;

	// Deliver data to the client:
#if (0)
	if (fNumValidDataBytes > fMaxOutputPacketSize) {
		printf("frameSize:%d, fNumValid:%d, into case 2\n", frameSize, fNumValidDataBytes);
	} else {
		printf("frameSize:%d, fNumValid:%d, into case 1\n", frameSize, fNumValidDataBytes);
	}
#endif

	doGetNextFrame();
}
