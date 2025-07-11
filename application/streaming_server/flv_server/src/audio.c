
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "audio.h"
#include "log_define.h"

int agtxPcmInit(snd_pcm_t **pcm_handle, const char *device, snd_pcm_stream_t stream, snd_pcm_format_t format,
                snd_pcm_uframes_t frame, unsigned int rate, unsigned int channels)
{
	int err = 0;
	int ret;
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	/* return error if already initialized */
	err = snd_pcm_open(pcm_handle, device, stream, SND_PCM_NONBLOCK);
	if (err < 0) {
		*pcm_handle = NULL;
		flv_server_log_err("failed to open pcm, %s", snd_strerror(err));
		return err;
	}
	/* configure alsa devicei, including layout, format, channels, sample rate, and periords */
	snd_pcm_hw_params_malloc(&params);
	snd_pcm_hw_params_any(*pcm_handle, params);
	if (snd_pcm_hw_params_set_access(*pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
		goto no_support;
	}
	if (snd_pcm_hw_params_set_format(*pcm_handle, params, format) < 0) {
		goto no_support;
	}
	if (snd_pcm_hw_params_set_channels(*pcm_handle, params, channels) < 0) {
		goto no_support;
	}
	if (snd_pcm_hw_params_set_rate_near(*pcm_handle, params, &rate, 0) < 0) {
		goto no_support;
	}
	if (snd_pcm_hw_params_set_period_size_near(*pcm_handle, params, &frame, 0) < 0) {
		goto no_support;
	}
	/* apply settings to hardware */
	if (snd_pcm_hw_params(*pcm_handle, params) < 0) {
		goto no_support;
	}
	snd_pcm_hw_params_free(params);
	if (snd_pcm_nonblock(*pcm_handle, 1) < 0) {
		flv_server_log_err("failed to set ALSA pcm nonblock");
	}
	/*Enable tstamp in sw_params*/
	snd_pcm_sw_params_alloca(&swparams);
	ret = snd_pcm_sw_params_current(*pcm_handle, swparams);
	if (ret)
		goto no_support;
	ret = snd_pcm_sw_params_set_tstamp_mode(*pcm_handle, swparams, SND_PCM_TSTAMP_ENABLE);
	if (ret)
		goto no_support;
	ret = snd_pcm_sw_params_set_tstamp_type(*pcm_handle, swparams, SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY);
	if (ret)
		goto no_support;
	snd_pcm_sw_params(*pcm_handle, swparams);
	if (ret < 0)
		goto no_support;
	return 0;
no_support:
	flv_server_log_err("configure alsa device failure:%s", snd_strerror(err));
	snd_pcm_close(*pcm_handle);
	*pcm_handle = NULL;
	return -EACCES;
}
int agtxPcmUninit(snd_pcm_t *p_capture)
{
	int ret;
	if (p_capture == NULL) {
		flv_server_log_err("pcm hd null, don't need drop");
		return -EACCES;
	}
	ret = snd_pcm_drop(p_capture);
	if (ret < 0) {
		flv_server_log_err("Failed snd_pcm_drop, %s", snd_strerror(ret));
	}
	ret = snd_pcm_close(p_capture);
	if (ret < 0) {
		flv_server_log_err("Failed snd_pcm_close, %s", snd_strerror(ret));
	}
	return 0;
}
int agtxPcmInitNoTs(snd_pcm_t **pcm_handle, const char *device, snd_pcm_stream_t stream, snd_pcm_format_t format,
                    snd_pcm_uframes_t frame, unsigned int rate, unsigned int channels)
{
	int err = 0;
	snd_pcm_hw_params_t *params;
	/* Open PCM device for recording (capture). */
	err = snd_pcm_open(pcm_handle, device, stream, 0);
	if (err < 0) {
		flv_server_log_err("unable to open pcm device: %s", snd_strerror(err));
		return err;
	}
	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);
	/* Fill it in with default values. */
	snd_pcm_hw_params_any(*pcm_handle, params);
	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(*pcm_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		flv_server_log_err("snd_pcm_hw_params_set_access: %s", snd_strerror(err));
		return err;
	}
	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(*pcm_handle, params, format);
	if (err < 0) {
		flv_server_log_err("snd_pcm_hw_params_set_format: %s", snd_strerror(err));
		return err;
	}
	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(*pcm_handle, params, channels);
	if (err < 0) {
		flv_server_log_err("snd_pcm_hw_params_set_channels: %s", snd_strerror(err));
		return err;
	}
	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(*pcm_handle, params, &rate, 0);
	if (err < 0) {
		flv_server_log_err("snd_pcm_hw_params_set_rate_near: %s", snd_strerror(err));
		return err;
	}
	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(*pcm_handle, params, &frame, 0);
	if (err < 0) {
		flv_server_log_err("snd_pcm_hw_params_set_period_size_near: %s", snd_strerror(err));
		return err;
	}
	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(*pcm_handle, params);
	if (err < 0) {
		flv_server_log_err("unable to set hw parameters: %s", snd_strerror(err));
		return err;
	}
	return err;
}
int agtxSetGain(int gain)
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
		flv_server_log_err("snd_mixer_open failed: %s", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		flv_server_log_err("snd_mixer_attach failed: %s", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		flv_server_log_err("snd_mixer_selem_register failed: %s", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		flv_server_log_err("snd_mixer_selem_register failed: %s", snd_strerror(err));
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
			flv_server_log_err("snd_mixer_selem_set_capture_volume_all: %s", snd_strerror(err));
			goto failed;
		}
	}
	/* set playback gain ( "Gain(Out)" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name[1]);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_playback_volume_all(elem, gain);
		if (err < 0) {
			flv_server_log_err("snd_mixer_selem_set_playback_volume_all: %s", snd_strerror(err));
			goto failed;
		}
	}
	snd_mixer_close(handle);
	return 0;
failed:
	snd_mixer_close(handle);
	return err;
}
long long timestamp2ns(snd_htimestamp_t t)
{
	long long nsec;
	nsec = t.tv_sec * 1000000000;
	nsec += t.tv_nsec;
	return nsec;
}
void agtxAudiogettimestamp(snd_pcm_t *handle, snd_htimestamp_t *timestamp, snd_htimestamp_t *trigger_timestamp,
                           snd_htimestamp_t *audio_timestamp, snd_pcm_uframes_t *avail, snd_pcm_sframes_t *delay)
{
	int err;
	snd_pcm_status_t *status;
	snd_pcm_status_alloca(&status);
	if ((err = snd_pcm_status(handle, status)) < 0) {
		flv_server_log_err("Stream status error: %s\n", snd_strerror(err));
	}
	snd_pcm_status_get_trigger_htstamp(status, trigger_timestamp);
	snd_pcm_status_get_htstamp(status, timestamp);
	snd_pcm_status_get_audio_htstamp(status, audio_timestamp);
	*avail = snd_pcm_status_get_avail(status);
	*delay = snd_pcm_status_get_delay(status);
	snd_pcm_status_free(status);
}