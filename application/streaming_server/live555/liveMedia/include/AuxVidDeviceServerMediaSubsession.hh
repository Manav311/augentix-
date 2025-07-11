#ifndef _AUXVID_DEVICE_SERVER_MEDIA_SUBSESSION_HH
#define _AUXVID_DEVICE_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif
class AuxVidDeviceSource;
class DeviceServerMediaSubsession : public OnDemandServerMediaSubsession {
    public:
	static DeviceServerMediaSubsession *createNew(UsageEnvironment &env, Boolean reuseFirstSource,
	                                              unsigned int channel);

	// Used to implement "getAuxSDPLine()":
	void checkForAuxSDPLine1();
	void afterPlayingDummy1(); /*not get SDP from source class, don't need to implement*/

    protected: // we're a virtual base class
	DeviceServerMediaSubsession(UsageEnvironment &env, Boolean reuseFirstSource, unsigned int channel);
	virtual ~DeviceServerMediaSubsession();

	void setDoneFlag()
	{
		fDoneFlag = ~0;
	}

    protected: // redefined virtual functions
	virtual char const *getAuxSDPLine(RTPSink *rtpSink, FramedSource *inputSource);
	virtual FramedSource *createNewStreamSource(unsigned clientSessionId, unsigned &estBitrate);
	virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
	                                  FramedSource *inputSource);

    private:
	char *fAuxSDPLine;
	char fDoneFlag; // used when setting up "fAuxSDPLine"
	unsigned int fchannel; // which video channel
	RTPSink *fDummyRTPSink; // ditto
};

#endif
