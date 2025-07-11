#include "AuxAudioDeviceServerMediaSubsession.hh"
#include "AuxAudioDeviceSource.hh"
#include "SimpleRTPSink.hh"
#include "DeviceSource.hh"
#include "WAVAudioFileServerMediaSubsession.hh"
#include "uLawAudioFilter.hh"
#include "WAVAudioFileSource.hh"
#include "BasicUsageEnvironment.hh"

#include <cstdarg>
#include <alsa/error.h>
#include <alsa/asoundlib.h>
#include <pcm_interfaces.h>

extern RtspServerConf gServerconf;

AudioDeviceServerMediaSubsession *
AudioDeviceServerMediaSubsession::createNew(UsageEnvironment &env, Boolean reuseFirstSource, unsigned int channel,
                                            unsigned char audioformat, unsigned char BitsPerSample, unsigned frequency,
                                            unsigned char gain)
{
	return new AudioDeviceServerMediaSubsession(env, reuseFirstSource, channel, audioformat, BitsPerSample,
	                                            frequency, gain);
}

AudioDeviceServerMediaSubsession::AudioDeviceServerMediaSubsession(UsageEnvironment &env, Boolean reuseFirstSource,
                                                                   unsigned int channel, unsigned char audioformat,
                                                                   unsigned char BitsPerSample, unsigned frequency,
                                                                   unsigned char gain)
        : OnDemandServerMediaSubsession(env, reuseFirstSource)
        , fDummyRTPSink(NULL)
{
	fchannel = channel;
	if (audioformat == SND_PCM_FORMAT_A_LAW) {
		fAudioFormat = WA_PCMA;
	} else if (audioformat == SND_PCM_FORMAT_MU_LAW) {
		fAudioFormat = WA_PCMU;
	} else {
		fAudioFormat = WA_UNKNOWN;
	}

	fNumChannels = 1;
	fBitsPerSample = BitsPerSample;
	fSamplingFrequency = frequency;
	fGain = gain;
}

AudioDeviceServerMediaSubsession::~AudioDeviceServerMediaSubsession()
{
	//TODO Close Alsa Audio
}
FramedSource *AudioDeviceServerMediaSubsession::createNewStreamSource(unsigned int /*clientSessionId*/,
                                                                      unsigned int &estBitrate)
{
	FramedSource *resultSource = NULL;

	DeviceSource *wavSource = AuxAudioDeviceSource::createNew(envir(), fchannel, fAudioFormat, fBitsPerSample,
	                                                          fSamplingFrequency, fGain);
	if (wavSource == NULL)
		return NULL;

	//get Attributes from audio source //FIXME use a better way to assign
	unsigned bitsPerSecond = fSamplingFrequency * fBitsPerSample * fNumChannels;

	// Add in any filter necessary to transform the data prior to streaming:
	resultSource = wavSource;
	if (fAudioFormat == WA_PCM) {
		if (fBitsPerSample == 16) {
			// Note that samples in the WAV audio file are in little-endian order.
			if (fConvertToULaw) {
				// Add a filter that converts from raw 16-bit PCM audio to 8-bit u-law audio:
				resultSource =
				        uLawFromPCMAudioSource::createNew(envir(), wavSource, 1 /*little-endian*/);
				bitsPerSecond /= 2;
			} else {
				// Add a filter that converts from little-endian to network (big-endian) order:
				resultSource = EndianSwap16::createNew(envir(), wavSource);
			}
		} else if (fBitsPerSample == 20 || fBitsPerSample == 24) {
			// Add a filter that converts from little-endian to network (big-endian) order:
			resultSource = EndianSwap24::createNew(envir(), wavSource);
		}
	}
	estBitrate = (bitsPerSecond + 500) / 1000; // kbps
	if (resultSource != NULL)
		return resultSource;
	else { // error occured:
		Medium::close(resultSource);
		return NULL;
	}
}

RTPSink *AudioDeviceServerMediaSubsession::createNewRTPSink(Groupsock *rtpGroupsock,
                                                            unsigned char rtpPayloadTypeIfDynamic,
                                                            FramedSource * /*inputSource*/)
{
	char const *mimeType;
	unsigned char payloadFormatCode = rtpPayloadTypeIfDynamic; //97
	if (fAudioFormat == WA_PCM) {
		if (fBitsPerSample == 16) {
			if (fConvertToULaw) {
				mimeType = "PCMU";
				if (fSamplingFrequency == 8000 && fNumChannels == 1) {
					payloadFormatCode = 0; // a static RTP payload type
				}
			} else {
				mimeType = "L16";
				if (fSamplingFrequency == 44100 && fNumChannels == 2) {
					payloadFormatCode = 10; // a static RTP payload type
				} else if (fSamplingFrequency == 44100 && fNumChannels == 1) {
					payloadFormatCode = 11; // a static RTP payload type
				}
			}
		} else if (fBitsPerSample == 20) {
			mimeType = "L20";
		} else if (fBitsPerSample == 24) {
			mimeType = "L24";
		} else { // fBitsPerSample == 8 (we assume that fBitsPerSample == 4 is only for WA_IMA_ADPCM)
			mimeType = "L8";
		}
	} else if (fAudioFormat == WA_PCMU) {
		mimeType = "PCMU";
		if (fSamplingFrequency == 8000 && fNumChannels == 1) {
			payloadFormatCode = 0; // a static RTP payload type
		}
	} else if (fAudioFormat == WA_PCMA) {
		mimeType = "PCMA";
		if (fSamplingFrequency == 8000 && fNumChannels == 1) {
			payloadFormatCode = 8; // a static RTP payload type
		}
	} else if (fAudioFormat == WA_IMA_ADPCM) {
		mimeType = "DVI4";
		// Use a static payload type, if one is defined:
		if (fNumChannels == 1) {
			if (fSamplingFrequency == 8000) {
				payloadFormatCode = 5; // a static RTP payload type
			} else if (fSamplingFrequency == 16000) {
				payloadFormatCode = 6; // a static RTP payload type
			} else if (fSamplingFrequency == 11025) {
				payloadFormatCode = 16; // a static RTP payload type
			} else if (fSamplingFrequency == 22050) {
				payloadFormatCode = 17; // a static RTP payload type
			}
		}
	} else {
		//mimeType = "AAL2-G.726-16";
		mimeType = "G726-16";
		//payloadFormatCode = 97; //G726 payload type is dynamic in range 96-127
	}
	int delay_interval = fSamplingFrequency == 48000 ? 85333 /*1000000 / (48000 / 4096)*/ : 128000 /*1000000 / (8000 / 1024)*/;
	return SimpleRTPSink::createNew(envir(), rtpGroupsock, payloadFormatCode, fSamplingFrequency, "audio", mimeType,
						fNumChannels, true, true, delay_interval - 2000);
}