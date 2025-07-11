#include "tutk_audio.h"
//#define _GNU_SOURCE

#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <math.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "P2PCam/AVFRAMEINFO.h"
#include "P2PCam/AVIOCTRLDEFs.h"
#include "tutk_define.h"

#include "tutk_audio.h"
#include "tutk_init.h"
#include "agtx_lpw.h"

#include "log_define.h"

const char g_device_name[] = "default";

//#define RECORD_TO_FILE_TEST
extern bool gProgressRun;
extern AV_Client gClientInfo[MAX_CLIENT_NUMBER];
extern int gOnlineNum;
extern lpw_handle gWifihd;
extern TutkConfigs gConfigs;

static audio_ctrl g_micphone_ctrl = {
	.channels = 1,
	.rate = 8000,
	.device = g_device_name,
	.params = NULL,
	.stream = SND_PCM_STREAM_CAPTURE,
	.format = SND_PCM_FORMAT_S16_LE,
	.frames = AUDIO_FRAME_SIZE,
	.play_bmp = 0,
	.volume = AUDIO_DEFAULT_INPUT_VOLUME,
};

static audio_ctrl g_speaker_ctrl = {
	.channels = 1,
	.rate = 8000,
	.device = g_device_name,
	.params = NULL,
	.stream = SND_PCM_STREAM_PLAYBACK,
	.format = SND_PCM_FORMAT_S16_LE,
	.frames = AUDIO_FRAME_SIZE,
	.play_bmp = 0,
	.volume = AUDIO_DEFAULT_OUTPUT_VOLUME,
};

static int aux_set_pcm_codec(codec_mode_t mode)
{
	const char *devicename = "default";
	snd_hwdep_t *hwdep;
	int err;
	int codec_mode = mode;

	tutkservice_log_info("codec_mode = %d\n", codec_mode);

	if ((err = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		tutkservice_log_err("hwdep interface open error: %s \n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_ENC_MODE, &codec_mode)) < 0) {
		tutkservice_log_err("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_ENC_MODE, &codec_mode)) < 0) {
		tutkservice_log_err("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	snd_hwdep_close(hwdep);

	return 0;
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
		tutkservice_log_err("Wrong volume value %d\n", volume);
		return -EINVAL;
	}

	/* open an empty mixer */
	err = snd_mixer_open(&handle, 0);
	if (err < 0) {
		tutkservice_log_err("snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		tutkservice_log_err("snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		tutkservice_log_err("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		tutkservice_log_err("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
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
				tutkservice_log_err("snd_mixer_selem_set_capture_dB_all: %s\n", snd_strerror(err));
				goto failed;
			}
		} else {
			err = snd_mixer_selem_set_capture_volume_all(elem, 0);
			if (err < 0) {
				tutkservice_log_err("snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
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

static int aux_pcm_init(unsigned int codec, audio_ctrl *p_audio_ctrl)
{
	int err = 0;

	/* Open PCM device for playback. */
	err = snd_pcm_open(&p_audio_ctrl->handle, p_audio_ctrl->device, p_audio_ctrl->stream, 0);
	if (err < 0) {
		tutkservice_log_err("unable to open pcm device: %s\n", snd_strerror(err));
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
		tutkservice_log_err("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->format);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(p_audio_ctrl->handle, p_audio_ctrl->params, p_audio_ctrl->channels);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_channels:%s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->rate, 0);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(p_audio_ctrl->handle, p_audio_ctrl->params, &p_audio_ctrl->frames,
	                                             0);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(p_audio_ctrl->handle, p_audio_ctrl->params);
	if (err < 0) {
		tutkservice_log_err("unable to set hw parameters: %s\n", snd_strerror(err));
		p_audio_ctrl->handle = NULL;
		return err;
	}

	aux_set_pcm_codec(codec);
	return err;
}

static void aux_pcm_close(audio_ctrl *p_audio_ctrl)
{
	int err = snd_pcm_drain(p_audio_ctrl->handle);
	if (err < 0) {
		tutkservice_log_err("snd_pcm_drain failed: %s\n", snd_strerror(err));
		goto exit;
	} else {
		snd_pcm_close(p_audio_ctrl->handle);
	}
exit:
	tutkservice_log_info("TUTK_deinitAudio done\n");
}

/* Callback of talkback mode, turn on or off the sound */
void TUTK_AudioPlayback(char *data, int size)
{
	audio_ctrl *p_audio_ctrl = &g_speaker_ctrl;

	if (p_audio_ctrl->handle == NULL) {
		return;
	}

	/* Set nonblocking mode */
	snd_pcm_nonblock(p_audio_ctrl->handle, 1);

	/* APP always send G711u */
	snd_pcm_uframes_t frames = size;
	snd_pcm_sframes_t rframes;
	int b_offset = 0;

	while (frames > 0) {
		rframes = snd_pcm_writei(p_audio_ctrl->handle, data + b_offset, AUDIO_PERIOD_SIZE);
		if (rframes == -EPIPE) {
			snd_pcm_prepare(p_audio_ctrl->handle);
		} else if (rframes == -EAGAIN) {
			//int t_sleep = ((rframes * 10000) / 8000) * 100;
			//usleep(t_sleep);
		} else if (rframes != AUDIO_PERIOD_SIZE) {
			//int t_sleep = ((rframes * 10000) / 8000) * 100;
			b_offset += rframes;
			frames -= rframes;
			//usleep(t_sleep);
		} else {
			b_offset = 0;
			frames = 0;
		}
	}
}

void TUTK_initAudio(void)
{
	pthread_mutex_init(&g_speaker_ctrl.lock, NULL);
	pthread_mutex_init(&g_micphone_ctrl.lock, NULL);

	tutkservice_log_info("TUTK_initAudio done\n");
	return;
}

void TUTK_deinitAudio(void)
{
	return;
}

//########################################################
//#Thread - Send live streaming
//########################################################
void *thread_AudioFrameData(void *arg)
{
	char buf[AUDIO_BUF_SIZE * 2];
	const int sleepTick = 1000000 / g_micphone_ctrl.rate;
	int frame_ = 512;
	FRAMEINFO_t frameInfo;

	AV_Client *client_info = (AV_Client *)(arg);
	int av_index = client_info->av_index;

	// *** set audio frame info here ***
	memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
	frameInfo.codec_id = AUDIO_CODEC;
	frameInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;

	tutkservice_log_info("thread_AudioFrameData start OK\n");
	tutkservice_log_debug("[Audio] is ENABLED!!\n", );

	aux_pcm_init(RAW, &g_micphone_ctrl);
	aux_set_audio_capture_gain(60);

	while (gProgressRun) {
		int ret = 0;
		int size = frame_ * 2;
		bool flowctrl = false;

		frameInfo.timestamp = GetTimeStampMs();

		if (client_info[av_index].av_index < 0 || client_info[av_index].enable_audio == 0) {
			usleep(100000);
			continue;
		}
		ret = snd_pcm_readi(g_micphone_ctrl.handle, buf, frame_);
		if (ret == -EPIPE) {
			/* EPIPE means overrun */
			snd_pcm_prepare(g_micphone_ctrl.handle);
			continue;
		} else if (ret == -EAGAIN) {
			continue;
		} else if (ret != frame_) {
		}

		if (flowctrl) {
			unsigned int resend_frame_count = 0;
			avServGetResendFrmCount(client_info[av_index].av_index, &resend_frame_count);
			if (resend_frame_count >= RESEND_FRAME_COUNT_THRESHOLD_TO_ENABLE_I_FRAME_ONLY) {
				usleep(sleepTick);
				continue;
			} else if (resend_frame_count <= RESEND_FRAME_COUNT_THRESHOLD_TO_DISABLE_I_FRAME_ONLY) {
				flowctrl = false;
			}
		}

		// send audio data to av-idx
		ret = avSendAudioData(client_info[av_index].av_index, buf, size, &frameInfo, sizeof(FRAMEINFO_t));

		if (ret == AV_ER_SESSION_CLOSE_BY_REMOTE) {
			tutkservice_log_err("thread_AudioFrameData: AV_ER_SESSION_CLOSE_BY_REMOTE\n");
			UnRegEditClientFromAudio(av_index);
		} else if (ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT) {
			tutkservice_log_err("thread_AudioFrameData: AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
			UnRegEditClientFromAudio(av_index);
		} else if (ret == IOTC_ER_INVALID_SID) {
			tutkservice_log_err("Session cant be used anymore\n");
			UnRegEditClientFromAudio(av_index);
		} else if (ret == AV_ER_EXCEED_MAX_SIZE) {
			flowctrl = true;
		} else if (ret < 0) {
			tutkservice_log_err("avSendAudioData error[%d]\n", ret);
			UnRegEditClientFromAudio(av_index);
		}

		usleep(sleepTick);
	}

	aux_pcm_close(&g_micphone_ctrl);

	tutkservice_log_info("[thread_AudioFrameData] exit\n");
	pthread_exit(0);
}

//########################################################
//#Thread - Receive audio
//########################################################

static void *ThreadReceiveAudio(void *arg)
{
	int sid = *((int *)arg);
	free(arg);
	AV_Client *p = &gClientInfo[sid];
	int av_index = p->av_index;

	aux_pcm_init(RAW, &g_speaker_ctrl);
	tutkservice_log_info("\n[%s] start SID[%d] avIndex[%d]\n", __func__, sid, av_index);

	if (av_index > -1) {
		char buf[AUDIO_BUF_SIZE];
		FRAMEINFO_t frame_info;
		frame_info.codec_id = AUDIO_CODEC;
		frame_info.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
		unsigned int frm_no = 0;
		int fps_count = 0;

		struct timeval tv1, tv2;
		gettimeofday(&tv1, NULL);

		/* adc measurement for low battery audio colume control */
		int adc_val = 0;
		int level_idx = 0;
		unsigned int interpolation_level = 0;
		if (gWifihd == (lpw_handle)NULL) {
			gWifihd = lpw_open();
		}
		adc_val = lpw_adc_get(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch);
		level_idx = sortAdcLevelIdx(adc_val);
		interpolation_level = getAdcLevelInterpolation(adc_val, level_idx);
		int mute_flag = 0;
		unsigned int iteration_count = 1;
		if (interpolation_level <= 10) {
			mute_flag = 1;
			TUTK_exeSystemCmd("amixer cset numid=1,iface=MIXER,name='Gain(Out) Playback Volume' 0");
			printf("Mute the volume for low battery level\n");
		} else {
			mute_flag = 0;
			TUTK_exeSystemCmd("amixer cset numid=1,iface=MIXER,name='Gain(Out) Playback Volume' 25");
			printf("Set the volume to 25 for normal battery level\n");
		}

		while (p->enable_speaker) {
			int ret = avRecvAudioData(av_index, buf, AUDIO_BUF_SIZE, (char *)&frame_info,
			                          sizeof(FRAMEINFO_t), &frm_no);
			if (ret == AV_ER_SESSION_CLOSE_BY_REMOTE) {
				tutkservice_log_err("[%s] AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d] avIndex[%d]\n",
				                    __func__, sid, av_index);
				break;
			} else if (ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT) {
				tutkservice_log_err("[%s] AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d] avIndex[%d]\n",
				                    __func__, sid, av_index);
				break;
			} else if (ret == IOTC_ER_INVALID_SID) {
				tutkservice_log_err("[%s] Session cant be used anymore SID[%d] avIndex[%d]\n", __func__,
				                    sid, av_index);
				break;
			} else if (ret == AV_ER_LOSED_THIS_FRAME) {
				tutkservice_log_err("[%s] Audio LOST[%d] SID[%d] avIndex[%d]", __func__, frm_no, sid,
				                    av_index);
				continue;
			} else if (ret == AV_ER_DATA_NOREADY) {
				usleep(5000);
				continue;
			} else if (ret < 0) {
				tutkservice_log_err("[%s] Other error[%d] SID[%d] avIndex[%d]!!!\n", __func__, ret, sid,
				                    av_index);
				break;
			} else {
				/* Measure the batterty level every 10 iteration when the speaker function is enable */
				if (mute_flag == 0 && iteration_count % 10 == 0) {
					adc_val = lpw_adc_get(gWifihd, gConfigs.pwr_mgmt.battery_adc_ch);
					level_idx = sortAdcLevelIdx(adc_val);
					interpolation_level = getAdcLevelInterpolation(adc_val, level_idx);

					if (interpolation_level <= 10) {
						mute_flag = 1;
						TUTK_exeSystemCmd(
						        "amixer cset numid=1,iface=MIXER,name='Gain(Out) Playback Volume' 0");
						printf("Mute the volume for battery level under 10\nSet the mute_fllag = 1, iteration_count = %d\n",
						       iteration_count);
					}
				}
				fps_count++;
				TUTK_AudioPlayback(buf, ret);
				iteration_count++;
			}

			gettimeofday(&tv2, NULL);
			long sec = tv2.tv_sec - tv1.tv_sec, usec = tv2.tv_usec - tv1.tv_usec;
			if (usec < 0) {
				sec--;
				usec += 1000000;
			}
			usec += (sec * 1000000);

			if (usec >= 1000000) {
				//tutkservice_log_debug("[Nebula_Device] Recv Spaeker Audio FPS = %d\n", fps_count);
				fps_count = 0;
				gettimeofday(&tv1, NULL);
			}
		}
	}

	aux_pcm_close(&g_speaker_ctrl);

	tutkservice_log_info("[%s] exit, SID[%d] avIndex[%d]\n", __func__, sid, av_index);
	pthread_exit(0);
}

//########################################################
//# Handle client command for speaker
//########################################################
int HandleSpeakerControl(int SID, int enable_speaker)
{
	AV_Client *p = &gClientInfo[SID];
	int ret = 0;

	p->enable_speaker = enable_speaker;
	printf("si");

	if (1 == enable_speaker) {
		tutkservice_log_info("Start Speaker\n");

		int *tmp_sid = (int *)malloc(sizeof(int));
		*tmp_sid = SID;
		pthread_t thread_id;
		if ((ret = pthread_create(&thread_id, NULL, &ThreadReceiveAudio, (void *)tmp_sid))) {
			free(tmp_sid);
			tutkservice_log_err("pthread_create ThreadReceiveAudio ret[%d]\n", ret);
			return -1;
		} else {
			pthread_detach(thread_id);
		}

	} else {
		tutkservice_log_info("Stop Speaker\n");
	}
	return 0;
}
