/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/*
 * Caution: Include mpi_base_types.h first to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_api.h"
#include "tuya_utils.h"
#include "ty_media_aac_codec.h"
#include "tuya_ipc_echo_show.h"
#include "tuya_ipc_chromecast.h"
#include "tuya_ipc_media_demo.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_cloud_storage.h"
#include "tuya_ring_buffer.h"
#include "tuya_ipc_dp_handler.h"

#include "mpi_enc.h"
#include "mpi_dev.h"
#include "avftr_conn.h"
#include "avftr.h"
#include "agtx_types.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/statfs.h>

// audio
#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <alsa/hwdep.h>
#include <pcm_interfaces.h>
#include <sys/ioctl.h>
#include <sys/time.h>
//Posix monotonic time .
#include <time.h>

#define MAX_AUDIO_FRAME 640
#define AUDIO_BUFFER_SIZE (MAX_AUDIO_FRAME * 2)

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
IPC_MEDIA_INFO_S s_media_info = { { 0 } };
#pragma GCC diagnostic pop
snd_pcm_uframes_t gframes = MAX_AUDIO_FRAME;
snd_pcm_t *gcapture_handle = NULL;

int g_echoshow = 0;
int g_chromecast = 0;

//Posix Monotonic clock
static int gettimeofdayMonotonic(struct timeval *t1, int *tz __attribute__((unused)))
{
	struct timespec tp;
	int ret;
	clockid_t clk_id;
	clk_id = CLOCK_MONOTONIC;
	ret = clock_gettime(clk_id, &tp);
	//t1->tv_sec = tp.tv_sec;
	//t1->tv_sec = 0xfffffed3 + tp.tv_sec;
	t1->tv_sec = tp.tv_sec; // + 0x83AA7E80; //(1970 epoch -> 1900 epoch)
	t1->tv_usec = (tp.tv_nsec) / 1000; //nano to milli
	return ret;
}

BOOL_T IPC_APP_Quarry_Stream_Status(CHANNEL_E chn)
{
	return s_media_info.channel_enable[chn];
}

/* Set audio and video properties */
VOID IPC_APP_Set_Media_Info(VOID)
{
	memset(&s_media_info, 0, sizeof(IPC_MEDIA_INFO_S));

	/* main stream(HD), video configuration*/
	/* NOTE
    FIRST:If the main stream supports multiple video stream configurations, set each item to the upper limit of the allowed configuration.
    SECOND:E_CHANNEL_VIDEO_MAIN must exist.It is the data source of SDK.
    please close the E_CHANNEL_VIDEO_SUB for only one stream*/
	s_media_info.channel_enable[E_CHANNEL_VIDEO_MAIN] = TRUE; /* Whether to enable local HD video streaming */
	s_media_info.video_fps[E_CHANNEL_VIDEO_MAIN] = 30; /* FPS */
	s_media_info.video_gop[E_CHANNEL_VIDEO_MAIN] = 30; /* GOP */
	s_media_info.video_bitrate[E_CHANNEL_VIDEO_MAIN] = TUYA_VIDEO_BITRATE_1_5M; /* Rate limit */
	s_media_info.video_width[E_CHANNEL_VIDEO_MAIN] = 1920; /* Single frame resolution of width*/
	s_media_info.video_height[E_CHANNEL_VIDEO_MAIN] = 1080; /* Single frame resolution of height */
	s_media_info.video_freq[E_CHANNEL_VIDEO_MAIN] = 90000; /* Clock frequency */
	s_media_info.video_codec[E_CHANNEL_VIDEO_MAIN] = TUYA_CODEC_VIDEO_H265; /* Encoding format */

	/* substream(HD), video configuration */
	/* Please note that if the substream supports multiple video stream configurations, please set each item to the upper limit of the allowed configuration. */
	s_media_info.channel_enable[E_CHANNEL_VIDEO_SUB] = TRUE; /* Whether to enable local SD video stream */
	s_media_info.video_fps[E_CHANNEL_VIDEO_SUB] = 30; /* FPS */
	s_media_info.video_gop[E_CHANNEL_VIDEO_SUB] = 30; /* GOP */
	s_media_info.video_bitrate[E_CHANNEL_VIDEO_SUB] = TUYA_VIDEO_BITRATE_1_5M; /* Rate limit */
	s_media_info.video_width[E_CHANNEL_VIDEO_SUB] = 1920; /* Single frame resolution of width*/
	s_media_info.video_height[E_CHANNEL_VIDEO_SUB] = 1080; /* Single frame resolution of height */
	s_media_info.video_freq[E_CHANNEL_VIDEO_SUB] = 90000; /* Clock frequency */
	s_media_info.video_codec[E_CHANNEL_VIDEO_SUB] = TUYA_CODEC_VIDEO_H265; /* Encoding format */

	/* Audio stream configuration.
    Note: The internal P2P preview, cloud storage, and local storage of the SDK are all use E_CHANNEL_AUDIO data. */
#if (TUYA_DISABLE_AUDIO == 0)
	s_media_info.channel_enable[E_CHANNEL_AUDIO] = TRUE; /* Whether to enable local sound collection */
#else
	s_media_info.channel_enable[E_CHANNEL_AUDIO] = FALSE; /* Whether to enable local sound collection */
#endif
	s_media_info.audio_codec[E_CHANNEL_AUDIO] = TUYA_CODEC_AUDIO_PCM; /* Encoding format */
	s_media_info.audio_sample[E_CHANNEL_AUDIO] = TUYA_AUDIO_SAMPLE_8K; /* Sampling Rate */
	s_media_info.audio_databits[E_CHANNEL_AUDIO] = TUYA_AUDIO_DATABITS_16; /* Bit width */
	s_media_info.audio_channel[E_CHANNEL_AUDIO] = TUYA_AUDIO_CHANNEL_MONO; /* channel */
	s_media_info.audio_fps[E_CHANNEL_AUDIO] = 20; /* Fragments per second */

	if (enable_chromecast == 1) {
		s_media_info.channel_enable[E_CHANNEL_AUDIO_2RD] = TRUE; /* Whether to enable local sound collection */
		s_media_info.audio_codec[E_CHANNEL_AUDIO_2RD] = TUYA_CODEC_AUDIO_AAC_ADTS; /* Encoding format */
		s_media_info.audio_sample[E_CHANNEL_AUDIO_2RD] = TUYA_AUDIO_SAMPLE_8K; /* Sampling Rate */
		s_media_info.audio_databits[E_CHANNEL_AUDIO_2RD] = TUYA_AUDIO_DATABITS_16; /* Bit width */
		s_media_info.audio_channel[E_CHANNEL_AUDIO_2RD] = TUYA_AUDIO_CHANNEL_MONO; /* channel */
		s_media_info.audio_fps[E_CHANNEL_AUDIO_2RD] = 8; /* Fragments per second */
	}
	PR_DEBUG("channel_enable:%d %d %d", s_media_info.channel_enable[0], s_media_info.channel_enable[1],
	         s_media_info.channel_enable[2]);

	PR_DEBUG("fps:%u", s_media_info.video_fps[E_CHANNEL_VIDEO_MAIN]);
	PR_DEBUG("gop:%u", s_media_info.video_gop[E_CHANNEL_VIDEO_MAIN]);
	PR_DEBUG("bitrate:%u kbps", s_media_info.video_bitrate[E_CHANNEL_VIDEO_MAIN]);
	PR_DEBUG("video_main_width:%u", s_media_info.video_width[E_CHANNEL_VIDEO_MAIN]);
	PR_DEBUG("video_main_height:%u", s_media_info.video_height[E_CHANNEL_VIDEO_MAIN]);
	PR_DEBUG("video_freq:%u", s_media_info.video_freq[E_CHANNEL_VIDEO_MAIN]);
	PR_DEBUG("video_codec:%d", s_media_info.video_codec[E_CHANNEL_VIDEO_MAIN]);

	PR_DEBUG("audio_codec:%d", s_media_info.audio_codec[E_CHANNEL_AUDIO]);
	PR_DEBUG("audio_sample:%d", s_media_info.audio_sample[E_CHANNEL_AUDIO]);
	PR_DEBUG("audio_databits:%d", s_media_info.audio_databits[E_CHANNEL_AUDIO]);
	PR_DEBUG("audio_channel:%d", s_media_info.audio_channel[E_CHANNEL_AUDIO]);
}

int aux_set_audio_gain(int gain)
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
		PR_ERR("snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		PR_ERR("snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		PR_ERR("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		PR_ERR("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}

	/* allocat an invalid snd_ mixer_selem_id_t */
	snd_mixer_selem_id_alloca(&sid);

	/* set capture gain ( "Gain(In)" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name[0]);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_capture_volume_all(elem, gain);
		if (err < 0) {
			PR_ERR("snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

int aux_set_pcm_codec(codec_mode_t mode)
{
	// const char *devicename = "hw:0,0";
	const char *devicename = "default";
	snd_hwdep_t *hwdep;
	int err;
	int codec_mode = mode;

	PR_INFO("codec_mode = %d\n", codec_mode);

	if ((err = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		PR_INFO("hwdep interface open error: %s \n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_ENC_MODE, &codec_mode)) < 0) {
		PR_INFO("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_ENC_MODE, &codec_mode)) < 0) {
		PR_INFO("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	snd_hwdep_close(hwdep);

	return 0;
}

void aux_pcm_init(unsigned int codec __attribute__((unused)), int audio_gain __attribute__((unused)))
{
	int err = 0;
	unsigned int channels = 1;
	unsigned int rate = 8000;
	static char *device = "default";
	snd_pcm_hw_params_t *params;
	snd_pcm_stream_t stream = SND_PCM_STREAM_CAPTURE;
	snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

	/* Open PCM device for recording (capture). */
	err = snd_pcm_open(&gcapture_handle, device, stream, 0); // SND_PCM_NONBLOCK);//0);//SND_PCM_NONBLOCK);
	if (err < 0) {
		PR_ERR("unable to open pcm device: %s\n", snd_strerror(err));
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_malloc(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(gcapture_handle, params);

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(gcapture_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(gcapture_handle, params, format);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(gcapture_handle, params, channels);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_channels:%s\n", snd_strerror(err));
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(gcapture_handle, params, &rate, 0);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(gcapture_handle, params, &gframes, 0);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(gcapture_handle, params);
	if (err < 0) {
		PR_ERR("unable to set hw parameters: %s\n", snd_strerror(err));
	}

	//aux_set_pcm_codec(codec);

	/*gain control by init script*/
	//	err = aux_set_audio_gain(audio_gain);
	//	if (err < 0) {
	//		PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
	//		// exit(EXIT_FAILURE);
	//	}

	snd_pcm_hw_params_get_period_size(params, &gframes, 0);
}

/* This is for demo only. Should be replace with real PCM/AAC/G711 encoder output */
void *thread_live_audio(void *arg)
{
	AGTX_UNUSED(arg);
	int err = 0;
	int size = AUDIO_BUFFER_SIZE;
	uint64_t pts = 0;
	ty_media_aac_handle_s hdl;
	char audioBuf[AUDIO_BUFFER_SIZE];
	MEDIA_FRAME_S pcm_frame = { 0 };
	pcm_frame.type = E_AUDIO_FRAME;

	//aux_pcm_init(MU_LAW, 0); //toggle comment for using MU-LAW
	aux_pcm_init(RAW, 0);

	/* init: pcm to ulaw_pcm */
	//ulaw_pcm16_tableinit();

	if (enable_chromecast == 1) {
		//if ( IS_LITTLE_ENDIAN() != 1) {
		if ( is_little_endian() != 1) {
			PR_ERR("unsupported Endian\n");
		}
		if(ty_media_aac_encoder_init(&hdl, 1, 8000, 8000 * 16) < 0) {
			PR_ERR("AAC: encoder init error\n");
		}
	}

	struct timeval dbgTime;
	char outBuf[1024] = {0}; int outlen = sizeof(outBuf);
	int last_pts = 0;
	int pts_correction = 0;
	int sample_correction = 0;
	while (1) {
		err = snd_pcm_readi(gcapture_handle, audioBuf, MAX_AUDIO_FRAME);
		if (err == -EPIPE) {
			/* EPIPE means overrun */
			PR_ERR("overrun occurred\n");
			snd_pcm_prepare(gcapture_handle);
			continue;
		} else if (err == -EAGAIN) {
			continue;
		} else if (err != MAX_AUDIO_FRAME) {
			PR_ERR("short read, read %d frames\n", err);
		}
		/* Get pts */
		gettimeofday(&dbgTime,NULL);
		pts = ((uint64_t)(dbgTime.tv_sec))*1000000 + (dbgTime.tv_usec);

		/* Get size in byte */
		size = err * 2;

		/* Fill 1st audio buffer */
		pcm_frame.size = size;
		pcm_frame.pts = pts;
		pcm_frame.p_buf = (BYTE_T *)audioBuf;
		TUYA_APP_Put_Frame(E_CHANNEL_AUDIO, &pcm_frame);

		/* Fill 2nd audio buffer (ChromeCast) */
		if (enable_chromecast == 1) {
			if (g_chromecast != 0) { //aac encode if chromecast session is active
				outlen = sizeof(outBuf);
				sample_correction += size;
				if (last_pts == 0)
					last_pts = pts;
				if(ty_media_aac_encoder_data(&hdl, audioBuf, size, outBuf, &outlen) == 0) {
					sample_correction -= 2048;
					tuya_ipc_ring_buffer_append_data(E_CHANNEL_AUDIO_2RD, (BYTE_T *)outBuf, outlen, E_AUDIO_FRAME, last_pts + (pts_correction * 125));
					pts_correction = 1024 - sample_correction / 2;
					last_pts = pts;
				}
			} else {
				last_pts = 0;
				pts_correction = 0;
				sample_correction = 0;
				hdl.pcmLen = 0;
			}
		}
	}

	ty_media_aac_encoder_uninit(&hdl);
	pthread_exit(0);
}

/* This is for demo only. Should be replace with real H264 encoder output */
int read_one_frame_from_demo_video_file(unsigned char *pVideoBuf, unsigned int offset, unsigned int BufSize,
                                        unsigned int *IskeyFrame, unsigned int *FramLen, unsigned int *Frame_start)
{
	int pos = 0;
	int bNeedCal = 0;
	unsigned char NalType = 0;
	int idx = 0;
	if (BufSize <= 5) {
		PR_INFO("bufSize is too small\n");
		return -1;
	}
	for (pos = 0; (unsigned int)pos <= BufSize - 5; pos++) {
		if (pVideoBuf[pos] == 0x00 && pVideoBuf[pos + 1] == 0x00 && pVideoBuf[pos + 2] == 0x00 &&
		    pVideoBuf[pos + 3] == 0x01) {
			NalType = pVideoBuf[pos + 4] & 0x1f;
			if (NalType == 0x7) {
				if (bNeedCal == 1) {
					*FramLen = pos - idx;
					return 0;
				}

				*IskeyFrame = 1;
				*Frame_start = offset + pos;
				bNeedCal = 1;
				idx = pos;
			} else if (NalType == 0x1) {
				if (bNeedCal) {
					*FramLen = pos - idx;
					return 0;
				}
				*Frame_start = offset + pos;
				*IskeyFrame = 0;
				idx = pos;
				bNeedCal = 1;
			}
		}
	}

	return 0;
}
#define PLAY_FILE 0
#if PLAY_FILE
void *thread_live_video(void *arg)
{
	AGTX_UNUSED(arg);
	char raw_fullpath[128] = { 0 };
	//char info_fullpath[128] = { 0 };
	unsigned int FrameLen = 0, Frame_start = 0;
	unsigned int offset = 0;
	unsigned int IsKeyFrame = 0;
	unsigned char *pVideoBuf = NULL;
	//Augentix: replace a valid file in the below file
	sprintf(raw_fullpath, "path/to/demo_video.264");
	//sprintf(raw_fullpath, "/mnt/sdcard/zerocp/100.264");

	PR_DEBUG("start live video using %s", raw_fullpath);

	FILE *streamBin_fp = fopen(raw_fullpath, "rb");
	if ((streamBin_fp == NULL)) {
		PR_INFO("can't read live video file %s\n", raw_fullpath);
		pthread_exit(0);
	}
	fseek(streamBin_fp, 0, SEEK_END);
	UINT_T file_size = ftell(streamBin_fp);
	fseek(streamBin_fp, 0, SEEK_SET);
	pVideoBuf = malloc(file_size);
	fread(pVideoBuf, 1, file_size, streamBin_fp);

	MEDIA_FRAME_S h264_frame = { 0 };
	while (1) {
		offset = Frame_start + FrameLen;
		if (offset >= file_size) {
			offset = 0;
		}
		read_one_frame_from_demo_video_file(pVideoBuf + offset, offset, file_size - offset, &IsKeyFrame,
		                                    &FrameLen, &Frame_start);
		//Note: For I frame of H264, SPS/PPS/SEI/IDR should be combined within one frame, and the NALU separator should NOT be deleted.
		if (IsKeyFrame == 1) {
			h264_frame.type = E_VIDEO_I_FRAME;
			h264_frame.size = FrameLen;
		} else {
			h264_frame.type = E_VIDEO_PB_FRAME;
			h264_frame.size = FrameLen;
		}
		h264_frame.p_buf = pVideoBuf + Frame_start;
		h264_frame.pts = 0;

		/* Send HD video data to the SDK */
		TUYA_APP_Put_Frame(E_CHANNEL_VIDEO_MAIN, &h264_frame);
		/* Send SD video data to the SDK */
		TUYA_APP_Put_Frame(E_CHANNEL_VIDEO_SUB, &h264_frame);

		int frameRate = 30;
		int sleepTick = 1000000 / frameRate;
		usleep(sleepTick);
	}

	pthread_exit(0);
}
#else /* PLAY_FILE */
#define MAX_FRAME_SIZE (200000)
#define MAX_SEI_SIZE (4096)
#define SEI_UUID_SIZE (16)
#define NALU_TYPE_SUFFIX_SEI (40)
#define NHU_LAYER_ID (0)
#define NHU_TEMPORAL_ID (0)

extern STREAM_TRHEAD_STATUS_E g_tread_live_status[E_CHANNEL_VIDEO_MAX];

extern AVFTR_CTX_S *avftr_res_shm_client;

char sei_nalu[MAX_SEI_SIZE] = { 0 };
char tmpSEIStr[MAX_SEI_SIZE] = { 0 };

const char uuid[SEI_UUID_SIZE] = { 0x0F, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	                           0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };

unsigned int setH264SEIdata(unsigned char *fTo, MPI_WIN idx, unsigned int timestamp)
{
	MPI_WIN iva_idx = MPI_VIDEO_WIN(idx.dev, 0, 0);
	//VFTR_MSG_HEADER_S cmd;
	//cmd.res.dev_idx = iva_idx.dev;
	//cmd.res.chn_idx = iva_idx.chn;
	//cmd.res.win_idx = iva_idx.win;

	unsigned int startSEI, sei_nal_size, sei_info_size;

	if (AVFTR_getVideoStat(iva_idx, &avftr_res_shm_client->vftr)) {
		//start code
		startSEI = 0;
		sei_nalu[startSEI++] = 0x00;
		sei_nalu[startSEI++] = 0x00;
		sei_nalu[startSEI++] = 0x00;
		sei_nalu[startSEI++] = 0x01;
		// SEI
		sei_nalu[startSEI++] = 0x06;
		// SEI , unregistered
		sei_nalu[startSEI++] = 0x05;
		sei_nal_size = 0;

		//fetch the xmlsei data
		//cmd.res.timestamp = timestamp;
		//if (idx.value == iva_idx.value) {
		//	VIDEO_FTR_clientGetRes(avftrUnxSktClientFD, &cmd);
		//}
		sei_info_size = AVFTR_tranVideoRes(iva_idx, idx, &avftr_res_shm_client->vftr, timestamp, tmpSEIStr);

		sei_nal_size = sei_info_size + SEI_UUID_SIZE;
		while (sei_nal_size >= 255) {
			sei_nalu[startSEI++] = 255;
			sei_nal_size -= 255;
		}

		sei_nalu[startSEI++] = sei_nal_size;
		//UUID
		memcpy(&sei_nalu[startSEI], uuid, SEI_UUID_SIZE);
		startSEI += SEI_UUID_SIZE;

		//copy the tmpSEIStr as payload
		strncpy(&sei_nalu[startSEI], tmpSEIStr, sei_info_size);
		startSEI += sei_info_size;
		//Set RBSP trailing bits
		sei_nalu[startSEI++] = 0x80;
		sei_nal_size = startSEI;

		if (sei_nal_size == 0) {
			PR_INFO("Error CreateH264SEINALU \n");
		} else {
			memcpy(fTo, sei_nalu, sei_nal_size);
		}
		//fFrameSize += sei_nal_size;
	} else {
		//start code
		startSEI = 0;
#if 1
		sei_nalu[startSEI++] = 0x00;
		sei_nalu[startSEI++] = 0x00;
		sei_nalu[startSEI++] = 0x00;
		sei_nalu[startSEI++] = 0x01;
		// SEI
		sei_nalu[startSEI++] = 0x06;
		// SEI , unregistered
		sei_nalu[startSEI++] = 0x05;
		sei_nal_size = 0;

		//fetch the xmlsei data
		//cmd.res.timestamp = timestamp;
		sei_info_size = 16;
		sei_nal_size = sei_info_size + SEI_UUID_SIZE;
		while (sei_nal_size >= 255) {
			sei_nalu[startSEI++] = 255;
			sei_nal_size -= 255;
		}

		sei_nalu[startSEI++] = sei_nal_size;
		//UUID
		memcpy(&sei_nalu[startSEI], uuid, SEI_UUID_SIZE);
		startSEI += SEI_UUID_SIZE;

		//copy the tmpSEIStr as payload
		strncpy(&sei_nalu[startSEI], "0123456789ABCDEF", sei_info_size);
		startSEI += sei_info_size;
		//Set RBSP trailing bits
		sei_nalu[startSEI++] = 0x80;
		sei_nal_size = startSEI;

		if (sei_nal_size == 0) {
			PR_INFO("Error CreateH264SEINALU \n");
		} else {
			memcpy(fTo, sei_nalu, sei_nal_size);
		}
#endif
	}
	return sei_nal_size;
}

static pthread_mutex_t s_video_sub_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t s_video_sub_cond = PTHREAD_COND_INITIALIZER;
static int s_video_sub_en = 1;

void IPC_APP_Live_Video_Sub_En(int enable)
{
	pthread_mutex_lock(&s_video_sub_lock);
	s_video_sub_en = enable;
	pthread_cond_signal(&s_video_sub_cond);
	pthread_mutex_unlock(&s_video_sub_lock);
	return;
}

#define STORE_CHN 0
void *thread_live_video(void *arg)
{
	STREAM_INFO *info = (STREAM_INFO *)arg;
	int i, newFrameSize, ret;
	int writtenFrameSize, newSegSize, seiSize;
	unsigned char *fTo = malloc(MAX_FRAME_SIZE);
	CHANNEL_E chn_type = info->chn_type;
	MPI_WIN panorama_idx = {{0}};
	TUYA_AG_CONF_S *conf = NULL;
	AGTX_LAYOUT_CONF_S *video_layout = &conf->layout.data;
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;
	AGTX_LAYOUT_PARAM_S *layout = NULL;

	AG_Get_Conf(&conf);
	video_layout = &conf->layout.data;
	ptz = &conf->ptz.data;
	layout = &video_layout->video_layout[info->chn_idx.chn];

#if STORE_CHN
	char store_file[128] = "/mnt/sdcard/video_FHD264_MAIN";
	sprintf(store_file, "%s%d", store_file, info->stream_idx.chn);
	FILE *fp = NULL;
	int store_fcnt = 100;
	int fame_cnt = 0;
#endif
	if (fTo == NULL) {
		PR_INFO("Allocate frame buf for video stream %d failed.\n", info->stream_idx.value);
		goto err_exit;
	}
	PR_DEBUG("Alloc stream temp frame buffer %p done", fTo);

#if STORE_CHN
	fp = fopen(store_file, "wb");
	if (fp == NULL) {
		PR_INFO("Cannot Open %s, all frames will be discarded!\n", store_file);
	}
#endif
	MPI_BCHN bchn = MPI_createBitStreamChn(info->stream_idx);
	if (!VALID_MPI_ENC_BCHN(bchn)) {
		PR_INFO("Failed to create bit stream channel %u.\n", info->stream_idx.value);
		goto err_createBs;
	}
	PR_DEBUG("Create bitstream %u done", info->stream_idx.value);

	MEDIA_FRAME_S h264_frame = { 0 };
	MPI_STREAM_PARAMS_S stream_params;

	g_tread_live_status[chn_type] = STREAM_RUNNING;

	int retry_cnt = 10;
	int start_seg_idx = 0;
	struct timeval tmp_pts;
	while (1) {
		if (g_tread_live_status[chn_type] == STREAM_CLOSING) {
			PR_INFO("Close stream 0\n");
			goto err_proc;
		}
		if (chn_type == E_CHANNEL_VIDEO_SUB) {
			pthread_mutex_lock(&s_video_sub_lock);
			while (!s_video_sub_en) { /* We're paused */
				pthread_cond_wait(&s_video_sub_cond, &s_video_sub_lock); /* Wait for play signal */
			}
			pthread_mutex_unlock(&s_video_sub_lock);
		}

		ret = MPI_getBitStream(bchn, &stream_params, -1 /*ms*/);
		if (ret != MPI_SUCCESS) {
			if (ret == -ETIMEDOUT || ret == -EAGAIN) {
				PR_INFO("MPI_getBitStream return -ETIMEDOUT\n");
				continue;
			} else if (ret == -EAGAIN) {
				PR_INFO("MPI_getBitStream return -EAGAIN\n");
				continue;
			} else if (ret == -ENODATA) {
				PR_INFO("MPI_getBitStream return -ENODATA\n");
				continue;
			}

			PR_INFO("Failed to get parameters of stream %u.\n", info->stream_idx.value);
			retry_cnt--;
			if (retry_cnt == 0) {
				goto err_proc;
			} else {
				continue;
			}
		}
		retry_cnt = 10;

		//PR_DEBUG("Get bitstream %u done", info->stream_idx.value);
		writtenFrameSize = 0;
		newFrameSize = 0;
		for (i = 0; (UINT32)i < stream_params.seg_cnt; ++i) {
			newFrameSize += stream_params.seg[i].size;
		}

		if (newFrameSize < 0 || newFrameSize > MAX_FRAME_SIZE) {
			PR_INFO("Incorrect frame size %d on stream %d\n", newFrameSize, info->stream_idx.value);
			MPI_releaseBitStream(bchn, &stream_params);
			continue;
			//goto err_proc;
		}
		start_seg_idx = 0;
		if (stream_params.seg[0].type == MPI_FRAME_TYPE_SPS) {
			for (i = 0; i < 2; ++i) { // SPS/PPS
				u_int8_t *newSegAddr = (u_int8_t *)(stream_params.seg[i].uaddr);
				newSegSize = stream_params.seg[i].size;
				memcpy(fTo + writtenFrameSize, newSegAddr, newSegSize);
				writtenFrameSize += newSegSize;
			}
			start_seg_idx = 2;
		}

		/*when only zoom view, don't send SEI*/
		if (ptz->enabled && layout->window_num == 1 && layout->video_strm_idx == 1) {
			//do nothing
		} else {
			panorama_idx = MPI_VIDEO_WIN(0, info->chn_idx.chn, layout->window_array[0].window_idx);
			seiSize = setH264SEIdata(fTo+writtenFrameSize, panorama_idx, stream_params.timestamp);
			writtenFrameSize += seiSize;
		}

		for (i = start_seg_idx; (UINT32)i < stream_params.seg_cnt; ++i) {
			u_int8_t *newSegAddr = (u_int8_t *)(stream_params.seg[i].uaddr);
			newSegSize = stream_params.seg[i].size;
			uint32_t tmpFrameSize = writtenFrameSize + newSegSize;
			if (tmpFrameSize > MAX_FRAME_SIZE) {
				PR_INFO("Frame size %u is larger than limitation %u\n", tmpFrameSize, MAX_FRAME_SIZE);
				memcpy(fTo + writtenFrameSize, newSegAddr, tmpFrameSize - MAX_FRAME_SIZE);
				break;
			} else {
				memcpy(fTo + writtenFrameSize, newSegAddr, newSegSize);
			}
			writtenFrameSize += newSegSize;
		}

		MPI_releaseBitStream(bchn, &stream_params);
		//PR_DEBUG("Release bitstream %u done\n", info->stream_idx.value);
		if (stream_params.seg[0].type == MPI_FRAME_TYPE_SPS) {
			h264_frame.type = E_VIDEO_I_FRAME;
			//PR_DEBUG("seiSize = %d\n", seiSize);
		} else {
			h264_frame.type = E_VIDEO_PB_FRAME;
		}

		h264_frame.size = writtenFrameSize;
		h264_frame.p_buf = fTo;
        //gettimeofday(&tmp_pts, NULL);
        if (enable_chromecast == 1) {
            gettimeofday(&tmp_pts, NULL);
        } else {
		gettimeofdayMonotonic(&tmp_pts, NULL);
	}
		h264_frame.pts = (((uint64_t)(tmp_pts.tv_sec))*1000000 + (tmp_pts.tv_usec));
		//h264_frame.timestamp = stream_params.timestamp;
        //
		//PR_DEBUG("size = %d, buf_addr = %p, timesp = %llu.",
		//		h264_frame.size, h264_frame.p_buf, h264_frame.timestamp);
		/* Send HD video data to the SDK */
		TUYA_APP_Put_Frame(chn_type, &h264_frame);
		if (!s_video_sub_en) {
			TUYA_APP_Put_Frame(E_CHANNEL_VIDEO_SUB, &h264_frame);
		}
#if STORE_CHN
		if (fp != NULL && (fame_cnt < store_fcnt)) {
			fwrite(fTo, sizeof(char), writtenFrameSize, fp);
		}

		fame_cnt++;
#endif
		/* Send SD video data to the SDK */
		//TUYA_APP_Put_Frame(E_CHANNEL_VIDEO_SUB, &h264_frame);
	}

err_proc:
	PR_INFO("Closing BS %u...\n", info->stream_idx.value);
#if STORE_CHN
	fclose(fp);
#endif
	MPI_destroyBitStreamChn(bchn);
	usleep(1000 * 1000);
err_createBs:
	free(fTo);
err_exit:
	free(info);
	g_tread_live_status[chn_type] = STREAM_STOP;
	pthread_exit(0);
}

#endif /* PLAY_FILE */
/*
---------------------------------------------------------------------------------
code related RingBuffer
---------------------------------------------------------------------------------
*/
OPERATE_RET TUYA_APP_Init_Ring_Buffer(VOID)
{
	STATIC BOOL_T s_ring_buffer_inited = FALSE;
	if (s_ring_buffer_inited == TRUE) {
		PR_DEBUG("The Ring Buffer Is Already Inited");
		return OPRT_OK;
	}

	CHANNEL_E channel;
	OPERATE_RET ret;
	for (channel = E_CHANNEL_VIDEO_MAIN; channel < E_CHANNEL_MAX; channel++) {
		PR_DEBUG("init ring buffer Channel:%d Enable:%d", channel, s_media_info.channel_enable[channel]);
		if (s_media_info.channel_enable[channel] == TRUE) {
			if (channel == E_CHANNEL_AUDIO) {
				PR_DEBUG("audio_sample %d, audio_databits %d, audio_fps %d",
				         s_media_info.audio_sample[E_CHANNEL_AUDIO],
				         s_media_info.audio_databits[E_CHANNEL_AUDIO],
				         s_media_info.audio_fps[E_CHANNEL_AUDIO]);
				ret = tuya_ipc_ring_buffer_init(
				        channel, s_media_info.audio_sample[E_CHANNEL_AUDIO] *
				                         s_media_info.audio_databits[E_CHANNEL_AUDIO] / 1024,
				        s_media_info.audio_fps[E_CHANNEL_AUDIO], 0, NULL);
			} else if ((channel == E_CHANNEL_AUDIO_2RD) && (enable_chromecast == 1)) {
				PR_DEBUG("audio_sample %d, audio_databits %d, audio_fps %d",
				         s_media_info.audio_sample[E_CHANNEL_AUDIO_2RD],
				         s_media_info.audio_databits[E_CHANNEL_AUDIO_2RD],
				         s_media_info.audio_fps[E_CHANNEL_AUDIO_2RD]);
				ret = tuya_ipc_ring_buffer_init(
				        channel, s_media_info.audio_sample[E_CHANNEL_AUDIO_2RD] *
				                         s_media_info.audio_databits[E_CHANNEL_AUDIO_2RD] / 1024,
				        s_media_info.audio_fps[E_CHANNEL_AUDIO_2RD], 0, NULL);
            } else {
				PR_DEBUG("video_bitrate %d, video_fps %d", s_media_info.video_bitrate[channel],
				         s_media_info.video_fps[channel]);
				ret = tuya_ipc_ring_buffer_init(channel, s_media_info.video_bitrate[channel],
				                                s_media_info.video_fps[channel], 0, NULL);
			}
			if (ret != 0) {
				PR_ERR("init ring buffer fails. %d %d", channel, ret);
				return OPRT_MALLOC_FAILED;
			}
			PR_DEBUG("init ring buffer success. channel:%d", channel);
		}
	}

	s_ring_buffer_inited = TRUE;

	return OPRT_OK;
}

OPERATE_RET TUYA_APP_Put_Frame(IN CONST CHANNEL_E channel, IN CONST MEDIA_FRAME_S *p_frame)
{
	OPERATE_RET ret =
	        tuya_ipc_ring_buffer_append_data(channel, p_frame->p_buf, p_frame->size, p_frame->type, p_frame->pts);

	if (ret != OPRT_OK) {
		PR_ERR("Put Frame Fail.%d Channel:%d type:%d size:%u pts:%llu ts:%llu", ret, channel, p_frame->type,
		       p_frame->size, p_frame->pts, p_frame->timestamp);
	}
	return ret;
}

OPERATE_RET TUYA_APP_Get_Frame_Backwards(IN CONST CHANNEL_E channel, IN CONST USER_INDEX_E user_index,
                                         IN CONST UINT_T backward_frame_num, INOUT MEDIA_FRAME_S *p_frame)
{
	if (p_frame == NULL) {
		PR_ERR("input is null");
		return OPRT_INVALID_PARM;
	}

	Ring_Buffer_Node_S *node;
	if (channel == E_CHANNEL_VIDEO_MAIN || channel == E_CHANNEL_VIDEO_SUB)
		node = tuya_ipc_ring_buffer_get_pre_video_frame(channel, user_index, backward_frame_num);
	else
		node = tuya_ipc_ring_buffer_get_pre_audio_frame(channel, user_index, backward_frame_num);
	if (node != NULL) {
		p_frame->p_buf = node->rawData;
		p_frame->size = node->size;
		p_frame->timestamp = node->timestamp;
		p_frame->type = node->type;
		p_frame->pts = node->pts;
		return OPRT_OK;
	} else {
		PR_ERR("Fail to re-locate for user %d backward %d frames, channel %d", user_index, backward_frame_num,
		       channel);
		return OPRT_COM_ERROR;
	}
}

OPERATE_RET TUYA_APP_Get_Frame(IN CONST CHANNEL_E channel, IN CONST USER_INDEX_E user_index, IN CONST BOOL_T isRetry,
                               IN CONST BOOL_T ifBlock, INOUT MEDIA_FRAME_S *p_frame)
{
	if (p_frame == NULL) {
		PR_ERR("input is null");
		return OPRT_INVALID_PARM;
	}
	PR_DEBUG("Get Frame Called. channel:%d user:%d retry:%d", channel, user_index, isRetry);

	Ring_Buffer_Node_S *node = NULL;
	while (node == NULL) {
		if (channel == E_CHANNEL_VIDEO_MAIN || channel == E_CHANNEL_VIDEO_SUB) {
			node = tuya_ipc_ring_buffer_get_video_frame(channel, user_index, isRetry);
		} else if (channel == E_CHANNEL_AUDIO) {
			node = tuya_ipc_ring_buffer_get_audio_frame(channel, user_index, isRetry);
		}
		if (NULL == node) {
			if (ifBlock) {
				usleep(10 * 1000);
			} else {
				return OPRT_NO_MORE_DATA;
			}
		}
	}
	p_frame->p_buf = node->rawData;
	p_frame->size = node->size;
	p_frame->timestamp = node->timestamp;
	p_frame->type = node->type;
	p_frame->pts = node->pts;

	PR_DEBUG("Get Frame Success. channel:%d user:%d retry:%d size:%u ts:%llu type:%d pts:%llu\n", channel, user_index,
	         isRetry, p_frame->size, p_frame->timestamp, p_frame->type, p_frame->pts);
	return OPRT_OK;
}

/*
---------------------------------------------------------------------------------
---------------------------------------------------------------------------------
*/

/*
---------------------------------------------------------------------------------
code related EchoShow
---------------------------------------------------------------------------------
*/

INT_T TUYA_APP_Echoshow_Start(PVOID_T context __attribute__((unused)), PVOID_T priv_data __attribute__((unused)))
{
	PR_INFO("echoshow start...\n");
	g_echoshow = -1;
	IPC_APP_set_bitrate(1, 200000);
	return 0;
}

INT_T TUYA_APP_Echoshow_Stop(PVOID_T context __attribute__((unused)), PVOID_T priv_data __attribute__((unused)))
{
	PR_INFO("echoshow stop...\n");
	g_echoshow = 0;
	if(!g_echoshow && !g_chromecast) {
		IPC_APP_set_bitrate(1, 500000);
	}
	return 0;
}

INT_T TUYA_APP_Chromecast_Start(PVOID_T context __attribute__((unused)), PVOID_T priv_data __attribute__((unused)))
{
	PR_INFO("chromecast start...\n");
	g_chromecast = -1;
	IPC_APP_set_bitrate(1, 200000);
	return 0;
}

INT_T TUYA_APP_Chromecast_Stop(PVOID_T context __attribute__((unused)), PVOID_T priv_data __attribute__((unused)))
{
	PR_INFO("chromecast stop...\n");
	g_chromecast = 0;
	if(!g_echoshow && !g_chromecast) {
		IPC_APP_set_bitrate(1, 500000);
	}
	return 0;
}

OPERATE_RET TUYA_APP_Enable_EchoShow_Chromecast(VOID)
{
	STATIC BOOL_T s_echoshow_inited = FALSE;
	if (s_echoshow_inited == TRUE) {
		PR_DEBUG("The EchoShow Is Already Inited");
		return OPRT_OK;
	}

	PR_DEBUG("Init EchoShow");
#if 0
    if (enable_echoShow == 1) {
        TUYA_ECHOSHOW_PARAM_S es_param = { 0 };

        es_param.pminfo = &s_media_info;
        es_param.cbk.pcontext = NULL;
        es_param.cbk.start = TUYA_APP_Echoshow_Start;
        es_param.cbk.stop = TUYA_APP_Echoshow_Stop;
        /*Channel settings according to requirements*/
        es_param.vchannel = E_CHANNEL_VIDEO_SUB;
        es_param.mode = TUYA_ECHOSHOW_MODE_ECHOSHOW;

	//tuya_ipc_echoshow_init(&es_param);
    }
    if (enable_chromecast == 1) {
            TUYA_CHROMECAST_PARAM_S param = { 0 };

            param.pminfo = &s_media_info;
            /*Channel settings according to requirements*/
            param.audio_channel = E_CHANNEL_AUDIO_2RD;
            param.video_channel = E_CHANNEL_VIDEO_SUB;
            param.src = TUYA_STREAM_SOURCE_RINGBUFFER;
            param.mode = TUYA_STREAM_TRANSMIT_MODE_ASYNC;
            param.cbk.pcontext = NULL;
            param.cbk.start = TUYA_APP_Chromecast_Start;
            param.cbk.stop = TUYA_APP_Chromecast_Stop;
            param.cbk.get_frame = NULL;

	    //tuya_ipc_chromecast_init(&param);
    }
#endif
	if (enable_chromecast || enable_echoShow) {
		s_echoshow_inited = TRUE;
	}

	return OPRT_OK;
}
/*
---------------------------------------------------------------------------------

---------------------------------------------------------------------------------
*/
