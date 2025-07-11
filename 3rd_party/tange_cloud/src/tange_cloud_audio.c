/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <math.h>
#include "tange_cloud_audio.h"

#include "logfile.h"
#include "ec_const.h"

/**
 * Definition
*/
typedef struct {
	/* ALSA releated parameters */
	unsigned int channels;
	unsigned int rate;
	const char *device;
	snd_pcm_hw_params_t *params;
	snd_pcm_stream_t stream;
	snd_pcm_format_t format;
	snd_pcm_uframes_t frames;
	snd_pcm_t *handle;

	/* other */
	int volume;
	unsigned int encode_type;
	unsigned int sampling;
} audio_ctrl;
/* Audio global variable */
#define AUDIO_DEFAULT_OUTPUT_VOLUME (90)
#define AUDIO_DEFAULT_INPUT_VOLUME (60)

/**
 * Static Variables
*/
static bool g_runAudio = false;
static bool g_muteAudio = true;
static const char g_device_name[] = "default";
static audio_ctrl g_micphone_ctrl = {
	.channels = 1,
	.rate = 8000,
	.device = g_device_name,
	.params = NULL,
	.stream = SND_PCM_STREAM_CAPTURE,
	.format = SND_PCM_FORMAT_A_LAW,
	.frames = 0,
	.volume = AUDIO_DEFAULT_INPUT_VOLUME,
	// customer capability.
	.encode_type = (unsigned int)TCMEDIA_AUDIO_ALAW,
	.sampling = ((AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO),
};

static audio_ctrl g_speaker_ctrl = {
	.channels = 1,
	.rate = 8000,
	.device = g_device_name,
	.params = NULL,
	.stream = SND_PCM_STREAM_PLAYBACK,
	.format = SND_PCM_FORMAT_A_LAW,
	.frames = 0,
	.volume = AUDIO_DEFAULT_OUTPUT_VOLUME,
};
static bool enable_talkback = true;

/**
 * Static Function Prototype
*/
static int aux_set_audio_capture_gain(int volume);
static int aux_pcm_init(audio_ctrl *p_audio_ctrl);
static int aux_set_pcm_codec(codec_mode_t mode);
static void aux_pcm_close(audio_ctrl *p_audio_ctrl);

/**
 * Functions
*/
/* Callback of talkback mode, turn on or off the sound */
void TGC_initTalkback(void)
{
	aux_pcm_init(&g_speaker_ctrl);
	aux_set_audio_capture_gain(g_speaker_ctrl.volume);
	enable_talkback = true;
	return;
}

void TGC_deinitTalkback(void)
{
	enable_talkback = false;
	aux_pcm_close(&g_speaker_ctrl);
	return;
}

void TGC_AudioTalkCallback(char *data, int size)
{
	snd_pcm_uframes_t frames = (snd_pcm_uframes_t)size;
	snd_pcm_sframes_t rframes = 0;

	if (!enable_talkback) {
		return;
	}

	if (g_speaker_ctrl.handle == NULL) {
		return;
	}
	/* Set nonblocking mode */
	snd_pcm_nonblock(g_speaker_ctrl.handle, 1);

	/* APP always send G711a */
	rframes = snd_pcm_writei(g_speaker_ctrl.handle, data, frames);
	if (rframes != frames) {
		snd_pcm_prepare(g_speaker_ctrl.handle);
	}
}

void TGC_initAudio(void)
{
	aux_pcm_init(&g_micphone_ctrl);
	aux_set_audio_capture_gain(g_micphone_ctrl.volume);
	g_runAudio = true;
	return;
}

void TGC_deinitAudio(void)
{
	aux_pcm_close(&g_micphone_ctrl);
	return;
}

void TGC_stopAudioRun(void)
{
	g_runAudio = false;
	return;
}

void TGC_muteAudio(bool mute)
{
	g_muteAudio = mute;
}

void *thread_AudioFrameData(void *arg)
{
	char buf[1024] = { 0 };
	int ret = 0;

	while (g_runAudio) {
		if (g_muteAudio) {
			usleep(100000);
			continue;
		}
		ret = snd_pcm_readi(g_micphone_ctrl.handle, buf, g_micphone_ctrl.frames);
		if (ret == -EPIPE) {
			/* EPIPE means overrun */
			snd_pcm_prepare(g_micphone_ctrl.handle);
			continue;
		} else if (ret == -EAGAIN) {
			continue;
		} else if (ret != g_micphone_ctrl.frames) {
		}
		ret = TGC_sendFrame(0, 0, (TCMEDIA)g_micphone_ctrl.encode_type, buf, g_micphone_ctrl.frames, GetTimeStampMs(),
		                    g_micphone_ctrl.sampling);
	}
	pthread_exit(0);
}

/**
 * Static Function
*/
static int aux_pcm_init(audio_ctrl *p_audio_ctrl)
{
	int err = 0;

	/* Open PCM device for playback. */
	err = snd_pcm_open(&p_audio_ctrl->handle, p_audio_ctrl->device, p_audio_ctrl->stream, 0);
	if (err < 0) {
		LogE("unable to open pcm device: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_malloc(&(p_audio_ctrl->params));

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(p_audio_ctrl->handle, p_audio_ctrl->params);

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(p_audio_ctrl->handle, p_audio_ctrl->params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		LogE("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->format);
	if (err < 0) {
		LogE("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->channels);
	if (err < 0) {
		LogE("snd_pcm_hw_params_set_channels:%s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->rate, 0);
	if (err < 0) {
		LogE("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->frames,
	                                             0);
	if (err < 0) {
		LogE("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(p_audio_ctrl->handle, p_audio_ctrl->params);
	if (err < 0) {
		LogE("unable to set hw parameters: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}
	return err;
}

static void aux_pcm_close(audio_ctrl *p_audio_ctrl)
{
	if (p_audio_ctrl == (audio_ctrl *)&g_speaker_ctrl)
		snd_pcm_drain(p_audio_ctrl->handle);
	else
		snd_pcm_drop(p_audio_ctrl->handle);
	snd_pcm_close(p_audio_ctrl->handle);
}

static int aux_set_audio_capture_gain(int volume)
{
	int err;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Digital Input";
	long db;

	if (volume < 0 || volume > 100) {
		LogE("Wrong volume value %d\n", volume);
		return -EINVAL;
	}

	/* open an empty mixer */
	err = snd_mixer_open(&handle, 0);
	if (err < 0) {
		LogE("snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		LogE("snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		LogE("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		LogE("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}

	/* allocat an invalid snd_ mixer_selem_id_t */
	snd_mixer_selem_id_alloca(&sid);

	/* set capture gain ( "Digital Input" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		if (volume > 0) {
			db = -(long)(log2(((double)100 / volume)) * 10 * 100);
			err = snd_mixer_selem_set_capture_dB_all(elem, db, 0);
			if (err < 0) {
				LogE("snd_mixer_selem_set_capture_dB_all: %s\n", snd_strerror(err));
				goto failed;
			}
		} else {
			err = snd_mixer_selem_set_capture_volume_all(elem, 0);
			if (err < 0) {
				LogE("snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
				goto failed;
			}
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

static int aux_set_pcm_codec(codec_mode_t mode)
{
	snd_hwdep_t *hwdep;
	int err;
	int codec_mode = mode;

	LogI("codec_mode = %d\n", codec_mode);

	if ((err = snd_hwdep_open(&hwdep, g_device_name, O_RDWR)) < 0) {
		LogE("hwdep interface open error: %s \n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_ENC_MODE, &codec_mode)) < 0) {
		LogE("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_ENC_MODE, &codec_mode)) < 0) {
		LogE("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	snd_hwdep_close(hwdep);

	return 0;
}
