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
// MPEG-1 or MPEG-2 Video RTP Sources
// C++ header

#ifndef _MPEG_1OR2_VIDEO_RTP_SOURCE_HH
#define _MPEG_1OR2_VIDEO_RTP_SOURCE_HH

#ifndef _MULTI_FRAMED_RTP_SOURCE_HH
#include "MultiFramedRTPSource.hh"
#endif

class MPEG1or2VideoRTPSource : public MultiFramedRTPSource {
    public:
	static MPEG1or2VideoRTPSource *createNew(UsageEnvironment &env, Groupsock *RTPgs,
	                                         unsigned char rtpPayloadFormat = 32,
	                                         unsigned rtpPayloadFrequency = 90000);

    protected:
	virtual ~MPEG1or2VideoRTPSource();

    private:
	MPEG1or2VideoRTPSource(UsageEnvironment &env, Groupsock *RTPgs, unsigned char rtpPayloadFormat,
	                       unsigned rtpTimestampFrequency);
	// called only by createNew()

    private:
	// redefined virtual functions:
	virtual Boolean processSpecialHeader(BufferedPacket *packet, unsigned &resultSpecialHeaderSize);
	virtual Boolean packetIsUsableInJitterCalculation(unsigned char *packet, unsigned packetSize);
	virtual char const *MIMEtype() const;
};

#endif
