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
// A 'ServerMediaSubsession' object that creates new, unicast, "RTPSink"s
// on demand, from a DV video file.
// C++ header

#ifndef _DV_VIDEO_FILE_SERVER_MEDIA_SUBSESSION_HH
#define _DV_VIDEO_FILE_SERVER_MEDIA_SUBSESSION_HH

#ifndef _FILE_SERVER_MEDIA_SUBSESSION_HH
#include "FileServerMediaSubsession.hh"
#endif

class DVVideoFileServerMediaSubsession : public FileServerMediaSubsession {
    public:
	static DVVideoFileServerMediaSubsession *createNew(UsageEnvironment &env, char const *fileName,
	                                                   Boolean reuseFirstSource);

    private:
	DVVideoFileServerMediaSubsession(UsageEnvironment &env, char const *fileName, Boolean reuseFirstSource);
	// called only by createNew();
	virtual ~DVVideoFileServerMediaSubsession();

    private: // redefined virtual functions
	virtual char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource);
	virtual void seekStreamSource(FramedSource *inputSource, double &seekNPT, double streamDuration,
	                              u_int64_t &numBytes);
	virtual void setStreamSourceDuration(FramedSource *inputSource, double streamDuration, u_int64_t &numBytes);
	virtual FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
	virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
	                                  FramedSource *inputSource);
	virtual float duration() const;

    private:
	float fFileDuration; // in seconds
};

#endif
