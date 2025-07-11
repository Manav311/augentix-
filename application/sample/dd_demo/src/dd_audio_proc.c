/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <stdbool.h>
#include <pthread.h>
#include <math.h>
#include <sys/ioctl.h>

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswresample/swresample.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>

// audio
#include <alsa/asoundlib.h>
#include <pcm_interfaces.h>

#include "dd_audio_proc.h"
#include "dd_video_proc.h"

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
} audio_ctrl;
/* Audio global variable */
#define AUDIO_DEFAULT_OUTPUT_VOLUME (90)
#define AUDIO_DEFAULT_INPUT_VOLUME (60)

/**
 * Static Variables
*/
static bool g_runAudio = false;
static bool g_muteAudio = false;
static const char g_device_name[] = "default";
static audio_ctrl g_speaker_ctrl = {
	.channels = 1,
	.rate = 8000,
	.device = g_device_name,
	.params = NULL,
	.stream = SND_PCM_STREAM_PLAYBACK,
	.format = SND_PCM_FORMAT_U8,
	.frames = 0,
	.volume = AUDIO_DEFAULT_OUTPUT_VOLUME,
};
// static bool enable_talkback = true;
static int audio_stream_index = -1;
static AVFormatContext *format_ctx = NULL;
static AVCodecContext *codec_ctx = NULL;
static AVCodec *codec = NULL;
static SwrContext *swr_ctx = NULL;

/**
 * Static Function Prototype
*/
static int aux_set_audio_capture_gain(int volume);
static int aux_pcm_init(audio_ctrl *p_audio_ctrl);
static void aux_pcm_close(audio_ctrl *p_audio_ctrl);

/**
 * Functions
*/
int audio_process_initial(void *arg)
{
	const char *input_url = (const char *)arg;

	// Do global initialization of network libraries.
	avformat_network_init();

	// open input stream
	if (avformat_open_input(&format_ctx, input_url, NULL, NULL) != 0) {
		fprintf(stderr, "Could not open input stream.\n");
		return -1;
	}

	// Find flow information
	if (avformat_find_stream_info(format_ctx, NULL) < 0) {
		fprintf(stderr, "Could not find stream information.\n");
		return -1;
	}

	// Find video stream
	for (unsigned int i = 0; i < format_ctx->nb_streams; i++) {
		if (format_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			audio_stream_index = i;
			break;
		}
	}
	if (audio_stream_index == -1) {
		fprintf(stderr, "Could not find audio stream.\n");
		return -1;
	}

	// Get decoder
	codec = avcodec_find_decoder(format_ctx->streams[audio_stream_index]->codecpar->codec_id);
	if (!codec) {
		fprintf(stderr, "Unsupported codec.\n");
		return -1;
	}
	g_runAudio = true;
	return 0;
}

void audio_process_deinitial(void)
{
	aux_pcm_close(&g_speaker_ctrl);
	return;
}

int startAudioRun(void)
{
	codec_ctx = avcodec_alloc_context3(codec);
	if (avcodec_parameters_to_context(codec_ctx, format_ctx->streams[audio_stream_index]->codecpar) < 0) {
		fprintf(stderr, "Could not copy codec context.\n");
		return -1;
	}

	if (avcodec_open2(codec_ctx, codec, NULL) < 0) {
		fprintf(stderr, "Could not open codec.\n");
		return -1;
	}

	// 檢查並設置通道佈局
	if (codec_ctx->channel_layout == 0) {
		codec_ctx->channel_layout = av_get_default_channel_layout(codec_ctx->channels);
	}

	printf("codec_ctx->channel_layout 0x%llx\r\n", codec_ctx->channel_layout);
	printf("codec_ctx->sample_rate 0x%x\r\n", codec_ctx->sample_rate);
	printf("codec_ctx->sample_fmt 0x%x\r\n", codec_ctx->sample_fmt);
	printf("codec_ctx->channels 0x%x\r\n", codec_ctx->channels);

	swr_ctx = swr_alloc();
	av_opt_set_int(swr_ctx, "in_channel_layout", codec_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "out_channel_layout", codec_ctx->channel_layout, 0);
	av_opt_set_int(swr_ctx, "in_sample_rate", codec_ctx->sample_rate, 0);
	av_opt_set_int(swr_ctx, "out_sample_rate", g_speaker_ctrl.rate, 0);
	av_opt_set_sample_fmt(swr_ctx, "in_sample_fmt", codec_ctx->sample_fmt, 0);
	av_opt_set_sample_fmt(swr_ctx, "out_sample_fmt", AV_SAMPLE_FMT_U8, 0);
	if (swr_init(swr_ctx) < 0) {
		fprintf(stderr, "SwrContext initial fail\n");
		return -1;
	}

	// 打開 ALSA PCM 設備
	// AVCodecParameters *codec_params = format_ctx->streams[audio_stream_index]->codecpar;
	g_speaker_ctrl.channels = codec_ctx->channels;
	g_speaker_ctrl.rate = codec_ctx->sample_rate;
	aux_pcm_init(&g_speaker_ctrl);
	aux_set_audio_capture_gain(g_speaker_ctrl.volume);

	return 0;
}

void stopAudioRun(void)
{
	g_runAudio = false;
	return;
}

void muteAudioRun(void)
{
	g_muteAudio = true;
	return;
}

void *thread_AudioFrameData(void *arg)
{
	(void)(arg);
	AVPacket packet;
	AVFrame *frame = av_frame_alloc();
	uint8_t *output_buffer = (uint8_t *)av_malloc(192000);
	snd_pcm_sframes_t rframes = 0;

	// 讀取音頻幀並播放
	while (g_runAudio && ((av_read_frame(format_ctx, &packet)) >= 0)) {
		if (g_muteAudio) {
			av_packet_unref(&packet);
			usleep(100000);
			continue;
		}
		if (packet.stream_index == audio_stream_index) {
			if (avcodec_send_packet(codec_ctx, &packet) == 0) {
				while (avcodec_receive_frame(codec_ctx, frame) == 0) {
					int output_buffer_size = av_samples_get_buffer_size(
					        NULL, codec_ctx->channels, frame->nb_samples, AV_SAMPLE_FMT_U8, 1);
					swr_convert(swr_ctx, &output_buffer, output_buffer_size,
					            (const uint8_t **)frame->data, frame->nb_samples);
					rframes =
					        snd_pcm_writei(g_speaker_ctrl.handle, output_buffer, frame->nb_samples);
					if (rframes != frame->nb_samples) {
						snd_pcm_prepare(g_speaker_ctrl.handle);
					}
				}
			}
		}
		av_packet_unref(&packet);
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
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(err));
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
		fprintf(stderr, "snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->format);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->channels);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_channels: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->rate, 0);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->frames,
	                                             0);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(p_audio_ctrl->handle, p_audio_ctrl->params);
	if (err < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(err));
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
		fprintf(stderr, "Wrong volume value %d\n", volume);
		return -EINVAL;
	}

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

	/* set capture gain ( "Digital Input" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		if (volume > 0) {
			db = -(long)(log2(((double)100 / volume)) * 10 * 100);
			err = snd_mixer_selem_set_capture_dB_all(elem, db, 0);
			if (err < 0) {
				fprintf(stderr, "snd_mixer_selem_set_capture_dB_all: %s\n", snd_strerror(err));
				goto failed;
			}
		} else {
			err = snd_mixer_selem_set_capture_volume_all(elem, 0);
			if (err < 0) {
				fprintf(stderr, "snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
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

// static int aux_set_pcm_codec(codec_mode_t mode)
// {
// 	snd_hwdep_t *hwdep;
// 	int err;
// 	int codec_mode = mode;

// 	fprintf(stderr, "codec_mode = %d\n", codec_mode);

// 	if ((err = snd_hwdep_open(&hwdep, g_device_name, O_RDWR)) < 0) {
// 		fprintf(stderr, "hwdep interface open error: %s\n", snd_strerror(err));
// 		return -1;
// 	}

// 	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_ENC_MODE, &codec_mode)) < 0) {
// 		fprintf(stderr, "hwdep ioctl error: %s\n", snd_strerror(err));
// 	}

// 	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_ENC_MODE, &codec_mode)) < 0) {
// 		fprintf(stderr, "hwdep ioctl error: %s\n", snd_strerror(err));
// 	}

// 	snd_hwdep_close(hwdep);

// 	return 0;
// }
