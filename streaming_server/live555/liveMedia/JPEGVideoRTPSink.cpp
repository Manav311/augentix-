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
// RTP sink for JPEG video (RFC 2435)
// Implementation

#include "JPEGVideoRTPSink.hh"
#include "JPEGVideoSource.hh"

JPEGVideoRTPSink::JPEGVideoRTPSink(UsageEnvironment &env, Groupsock *RTPgs)
        : VideoRTPSink(env, RTPgs, 26, 90000, "JPEG")
{
}

JPEGVideoRTPSink::~JPEGVideoRTPSink()
{
}

JPEGVideoRTPSink *JPEGVideoRTPSink::createNew(UsageEnvironment &env, Groupsock *RTPgs)
{
	return new JPEGVideoRTPSink(env, RTPgs);
}

Boolean JPEGVideoRTPSink::sourceIsCompatibleWithUs(MediaSource &source)
{
	return source.isJPEGVideoSource();
}

Boolean JPEGVideoRTPSink::frameCanAppearAfterPacketStart(unsigned char const * /*frameStart*/,
                                                         unsigned /*numBytesInFrame*/) const
{
	// A packet can contain only one frame
	return False;
}

void JPEGVideoRTPSink::doSpecialFrameHandling(unsigned fragmentationOffset, unsigned char * /*frameStart*/,
                                              unsigned /*numBytesInFrame*/, struct timeval framePresentationTime,
                                              unsigned numRemainingBytes)
{
	// Our source is known to be a JPEGVideoSource
	JPEGVideoSource *source = (JPEGVideoSource *)fSource;
	if (source == NULL)
		return; // sanity check

	u_int8_t mainJPEGHeader[8]; // the special header
	u_int8_t const type = source->type();

	mainJPEGHeader[0] = 0; // Type-specific
	mainJPEGHeader[1] = fragmentationOffset >> 16;
	mainJPEGHeader[2] = fragmentationOffset >> 8;
	mainJPEGHeader[3] = fragmentationOffset;
	mainJPEGHeader[4] = type;
	mainJPEGHeader[5] = source->qFactor();
	mainJPEGHeader[6] = source->width();
	mainJPEGHeader[7] = source->height();
	setSpecialHeaderBytes(mainJPEGHeader, sizeof mainJPEGHeader);

	unsigned restartMarkerHeaderSize = 0; // by default
	if (type >= 64 && type <= 127) {
		// There is also a Restart Marker Header:
		restartMarkerHeaderSize = 4;
		u_int16_t const restartInterval = source->restartInterval(); // should be non-zero

		u_int8_t restartMarkerHeader[4];
		restartMarkerHeader[0] = restartInterval >> 8;
		restartMarkerHeader[1] = restartInterval & 0xFF;
		restartMarkerHeader[2] = restartMarkerHeader[3] = 0xFF; // F=L=1; Restart Count = 0x3FFF

		setSpecialHeaderBytes(restartMarkerHeader, restartMarkerHeaderSize,
		                      sizeof mainJPEGHeader /* start position */);
	}

	if (fragmentationOffset == 0 && source->qFactor() >= 128) {
		// There is also a Quantization Header:
		u_int8_t precision;
		u_int16_t length;
		u_int8_t const *quantizationTables = source->quantizationTables(precision, length);

		unsigned const quantizationHeaderSize = 4 + length;
		u_int8_t *quantizationHeader = new u_int8_t[quantizationHeaderSize];

		quantizationHeader[0] = 0; // MBZ
		quantizationHeader[1] = precision;
		quantizationHeader[2] = length >> 8;
		quantizationHeader[3] = length & 0xFF;
		if (quantizationTables != NULL) { // sanity check
			for (u_int16_t i = 0; i < length; ++i) {
				quantizationHeader[4 + i] = quantizationTables[i];
			}
		}

		setSpecialHeaderBytes(quantizationHeader, quantizationHeaderSize,
		                      sizeof mainJPEGHeader + restartMarkerHeaderSize /* start position */);
		delete[] quantizationHeader;
	}

	if (numRemainingBytes == 0) {
		// This packet contains the last (or only) fragment of the frame.
		// Set the RTP 'M' ('marker') bit:
		setMarkerBit();
	}

	// Also set the RTP timestamp:
	setTimestamp(framePresentationTime);
}

unsigned JPEGVideoRTPSink::specialHeaderSize() const
{
	// Our source is known to be a JPEGVideoSource
	JPEGVideoSource *source = (JPEGVideoSource *)fSource;
	if (source == NULL)
		return 0; // sanity check

	unsigned headerSize = 8; // by default

	u_int8_t const type = source->type();
	if (type >= 64 && type <= 127) {
		// There is also a Restart Marker Header:
		headerSize += 4;
	}

	if (curFragmentationOffset() == 0 && source->qFactor() >= 128) {
		// There is also a Quantization Header:
		u_int8_t dummy;
		u_int16_t quantizationTablesSize;
		(void)(source->quantizationTables(dummy, quantizationTablesSize));

		headerSize += 4 + quantizationTablesSize;
	}

	return headerSize;
}

char const *JPEGVideoRTPSink::auxSDPLine()
{
	/*MJPEG has no SDP Media attribute*/
	return NULL;
}

static unsigned const rtpHeaderSize = 12;
void JPEGVideoRTPSink::sendPacketIfNecessary()
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
		MultiFramedRTPSink::sendNext(this);
	}
}
