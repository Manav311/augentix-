#ifndef _AUXAUDIO_DEVICE_SERVER_MEDIA_SUBSESSION_HH
#define _AUXAUDIO_DEVICE_SERVER_MEDIA_SUBSESSION_HH

#ifndef _ON_DEMAND_SERVER_MEDIA_SUBSESSION_HH
#include "OnDemandServerMediaSubsession.hh"
#endif

/*typedef enum {
        WA_PCM = 0x01,
        WA_PCMA = 0x06,
        WA_PCMU = 0x07,
        WA_IMA_ADPCM = 0x11,
        WA_UNKNOWN
} WAV_AUDIO_FORMAT; */

class AuxAudioDeviceSource;
class AudioDeviceServerMediaSubsession : public OnDemandServerMediaSubsession {
    public:
	static AudioDeviceServerMediaSubsession *createNew(UsageEnvironment &env, Boolean reuseFirstSource,
	                                                   unsigned int channel, unsigned char audioformat,
	                                                   unsigned char BitsPerSample, unsigned frequency,
	                                                   unsigned char gain);

	// Used to implement "getAuxSDPLine()":

    protected: // we're a virtual base class
	AudioDeviceServerMediaSubsession(UsageEnvironment &env, Boolean reuseFirstSource, unsigned int channel,
	                                 unsigned char audioformat, unsigned char BitsPerSample, unsigned frequency,
	                                 unsigned char gain);
	virtual ~AudioDeviceServerMediaSubsession();

    protected: // redefined virtual functions
	virtual FramedSource *createNewStreamSource(unsigned int clientSessionId, unsigned int &estBitrate);
	virtual RTPSink *createNewRTPSink(Groupsock *rtpGroupsock, unsigned char rtpPayloadTypeIfDynamic,
	                                  FramedSource *inputSource);
	Boolean fConvertToULaw;
	// The following parameters of the input stream are set after
	// "createNewStreamSource" is called:
	unsigned char fAudioFormat;
	unsigned char fBitsPerSample;
	unsigned fSamplingFrequency;
	unsigned fNumChannels;
	unsigned char fGain;
	float fFileDuration;

    private:
	unsigned int fchannel; //which audio channel
	RTPSink *fDummyRTPSink; // ditto
};

#endif
