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

// JPEG Media subsession
// C++ header

#include "OnDemandServerMediaSubsession.hh"
#include "FramedSource.hh"
#include "RTPSink.hh"

class JPEGMediaSubsession : public OnDemandServerMediaSubsession {
    public:
	JPEGMediaSubsession(UsageEnvironment &env, Boolean reuseFirstSource);
	~JPEGMediaSubsession();

    protected:
	virtual FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
	virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
	                                  FramedSource *inputSource);

    private:
	u_int64_t fFileSize; // if known
};
