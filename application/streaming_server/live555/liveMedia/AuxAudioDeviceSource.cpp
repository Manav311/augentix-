#include "AuxAudioDeviceSource.hh"
#include <GroupsockHelper.hh>
#include <sys/ioctl.h>
#include <cstdarg>
//#include <fcntl.h>

#include <alsa/error.h>
#include <alsa/asoundlib.h>

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <fcntl.h>

#define REQIDR
//#define RECORD_A_SRC
#ifdef RECORD_A_SRC
#include <time.h>
char afileName[128];
FILE *pfa;
#endif

int set_gain(int gain)
{
	int err;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name[] = { "Gain(In)", "Gain(Out)" };

	/* open an empty mixer */
	err = snd_mixer_open(&handle, 0);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}

	/* allocat an invalid snd_ mixer_selem_id_t */
	snd_mixer_selem_id_alloca(&sid);

	/* set capture gain ( "Gain(In)" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name[0]);

	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_capture_dB_all(elem, gain * 100, 0);
		if (err < 0) {
			fprintf(stderr, "snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

DeviceSource *AuxAudioDeviceSource::createNew(UsageEnvironment &env, unsigned int chn_idx, unsigned int codecType,
                                              unsigned int bitPerSample, unsigned int freq, unsigned int gain)
{
	return new AuxAudioDeviceSource(env, chn_idx, codecType, bitPerSample, freq, gain);
}

int agtx_pcm_init(snd_pcm_t **pcm_handle, const char *device, snd_pcm_stream_t stream, snd_pcm_format_t format,
                  snd_pcm_uframes_t frame, unsigned int rate, unsigned int channels, unsigned int gain)
{
	int ret;
	snd_pcm_hw_params_t *params;

	/* return error if already initialized */
	ret = snd_pcm_open(pcm_handle, device, stream, SND_PCM_NONBLOCK);
	if (ret < 0) {
		fprintf(stderr, "fail pcm open\n");
		*pcm_handle = NULL;
		return -1;
	}

	/* configure alsa devicei, including layout, format, channels, sample rate, and periords */
	ret = snd_pcm_hw_params_malloc(&params);
	if (ret != 0) {
		fprintf(stderr, "Failed to Pcm hw param malloc\n");
		return -1;
	}

	ret = snd_pcm_hw_params_any(*pcm_handle, params);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_access(*pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_format(*pcm_handle, params, format);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_channels(*pcm_handle, params, channels);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params_set_rate_near(*pcm_handle, params, &rate, 0);
	if (ret != 0) {
		goto handle_error;
	}

	/* Thit will only follow /etc/asound.conf */
	ret = snd_pcm_hw_params_set_period_size_near(*pcm_handle, params, &frame, 0);
	if (ret != 0) {
		goto handle_error;
	}

	ret = snd_pcm_hw_params(*pcm_handle, params);
	if (ret != 0) {
		goto handle_error;
	}

	ret = set_gain(gain);
	if (ret != 0) {
		goto handle_error;
	}

	snd_pcm_hw_params_free(params);
	return 0;

handle_error:
	snd_pcm_hw_params_free(params);
	perror("configure alsa device failture");
	snd_pcm_close(*pcm_handle);
	*pcm_handle = NULL;
	return -1;
}

static const char *g_device[] = { "default", "hw:0,0" }; /* sound device */
AuxAudioDeviceSource::AuxAudioDeviceSource(UsageEnvironment &env, unsigned int chn_idx, unsigned int codecType,
                                           unsigned int bitPerSample, unsigned int freq, unsigned int gain)
        : DeviceSource(env, DeviceParameters())
{
	if (codecType == WA_PCMA) {
		fCodecType = (unsigned int)SND_PCM_FORMAT_A_LAW;
	} else if (codecType == WA_PCMU) {
		fCodecType = (unsigned int)SND_PCM_FORMAT_MU_LAW;
	}
	fSamplingBits = bitPerSample;
	fFrequency = freq;
	fGain = gain;

	int channel = 1;
	int dev_id = 0;

	int ret;
	init_fail = 0;

	fFrame = (fFrequency == 48000) ? 4096 : 1024;
	ret = agtx_pcm_init((snd_pcm_t **)&fCaptureHandle, g_device[dev_id], SND_PCM_STREAM_CAPTURE,
	                    (snd_pcm_format_t)fCodecType, (snd_pcm_uframes_t)fFrame, fFrequency, channel, fGain);
	if (ret != 0) {
		fprintf(stderr, "Failed init ALSA\n");
		init_fail = 1;
	}

	ret = snd_pcm_nonblock((snd_pcm_t *)fCaptureHandle, 1);
	if (ret != 0) {
		fprintf(stderr, "Failed set ALSA nonblock\n");
		init_fail = 1;
	}

	fPresentationTime.tv_sec = 0;
	isAudioframe = 0;
	fChn_idx = chn_idx;

	fTimerfdInterval = (10000 * fFrame) / fFrequency * 99; /*1 sec / (samplerate / frame size means 1s return timees) * 0.99*/

#ifdef RECORD_A_SRC
	time_t t = time(NULL);
	struct tm tm = *localtime(&t);
	sprintf(&afileName[0], "/mnt/nfs/ethnfs/%d-%02d-%02d_%02d_%02d_%02d.pcm", tm.tm_year + 1900, tm.tm_mon + 1,
	        tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	pfa = fopen(afileName, "w");
#endif
}

AuxAudioDeviceSource::~AuxAudioDeviceSource()
{
	// TODO Auto-generated destructor stub
	printf("PCM Bit stream system exited.\n");
	int ret = -1;

	ret = snd_pcm_drop((snd_pcm_t *)fCaptureHandle);
	if (ret < 0) {
		fprintf(stderr, "Failed snd_pcm_drop, %d\n", ret);
	}

	ret = snd_pcm_close((snd_pcm_t *)fCaptureHandle);
	if (ret < 0) {
		fprintf(stderr, "Failed snd_pcm_close, %d\n", ret);
	}
#ifdef RECORD_A_SRC
	fclose(pfa);
#endif
}

void AuxAudioDeviceSource::getBitStream()
{
	int ret;
	ret = snd_pcm_readi((snd_pcm_t *)fCaptureHandle, fTo, fFrame);
	if (ret == -EPIPE) {
		snd_pcm_prepare((snd_pcm_t *)fCaptureHandle);
		fprintf(stderr, "snd -EPIPE\n");
	} else if (ret == -EAGAIN) {
		fprintf(stderr, "snd -EAGAIN\n");
	}

	gettimeofdayMonotonic(&fPresentationTime, NULL);

	if (ret <= 0) {
		fFrameSize = 0;
		fNumTruncatedBytes = 0;
		if (isAudioframe == 0) { /*before get the first PCM*/
			FramedSource::afterGetting(this);
		}
		return;
	}


	isAudioframe = 1;
	fFrameSize = 0;
	if ((unsigned int)ret > fMaxSize) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = ret - fMaxSize;
	} else {
		fFrameSize = ret;
	}
#ifdef RECORD_A_SRC
	fwrite(fTo, fFrameSize, 1, pfa);
#endif

	// Deliver the data here:
	FramedSource::afterGetting(this);
}

void AuxAudioDeviceSource::getNextFrame(void *ptr)
{
	((AuxAudioDeviceSource *)ptr)->getBitStream();
}

void AuxAudioDeviceSource::doGetNextFrame()
{
	if (!isCurrentlyAwaitingData())
		return; // we're not ready for the data yet

	if (init_fail) {
		handleClosure();
		return;
	}

	if ((int)nextTask() == 0 /*have not created own timerfd*/) {
		nextTask() = envir().taskScheduler().scheduleDelayedTask(
			fTimerfdInterval /*1/2 of 30 fps*/, (TaskFunc *)AuxAudioDeviceSource::getNextFrame, this);
		printf("%s %d audio timerfd: %d\n", __func__, __LINE__, *(int*)nextTask());
	}
	return;
}

unsigned AuxAudioDeviceSource::maxFrameSize() const
{
	return 128 * 1024; // 128k
}
