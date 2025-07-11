#include <cerrno>
#include "audio_utils.h"
#include "mp4_log_define.h"


PcmSound::PcmSound(unsigned int sample_rate, int gain, bool enable_timestamp)
        : _handle(nullptr), _params(nullptr), _sample_rate(sample_rate)
        , _timestamp_enabled(enable_timestamp)
{
	/* Open PCM device for recording (capture). */
	int err = snd_pcm_open(&_handle, _device, SND_PCM_STREAM_CAPTURE, 0);
	if (err) {
		mp4_log_err("Unable to open pcm device: %s\n", snd_strerror(err));
		return;
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_malloc(&_params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(_handle, _params);

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(_handle, _params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err) {
		mp4_log_err("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		return;
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(_handle, _params, _format);
	if (err) {
		mp4_log_err("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		return;
	}

	/* One channel */
	err = snd_pcm_hw_params_set_channels(_handle, _params, _channels);
	if (err) {
		mp4_log_err("snd_pcm_hw_params_set_channels: %s\n", snd_strerror(err));
		return;
	}

	/* 44100 bits/second sampling rate */
	mp4_log_notice("PCM sample rate: %u\n", _sample_rate);
	err = snd_pcm_hw_params_set_rate_near(_handle, _params, &_sample_rate, nullptr);
	if (err) {
		mp4_log_err("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		return;
	}
	mp4_log_notice("PCM sample update to %u\n", _sample_rate);

	/* Set period size to 1024 frames. */
	mp4_log_notice("PCM frames: %lu\n", _frames);
	snd_pcm_uframes_t frames = _frames;
	err = snd_pcm_hw_params_set_period_size_near(_handle, _params, &frames, nullptr);
	if (err) {
		mp4_log_err("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		return;
	}
	mp4_log_notice("PCM frames update to %lu\n", frames);

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(_handle, _params);
	if (err) {
		mp4_log_err("Unable to set hw parameters: %s\n", snd_strerror(err));
		return;
	}

	/* Set gain */
	err = setGain(gain);
	if (err) {
		mp4_log_err("Set mixer gain failed: %s\n", snd_strerror(err));
		return;
	}

	if (enable_timestamp) {
		snd_pcm_sw_params_t *swparams;
		snd_pcm_sw_params_alloca(&swparams);

		err = snd_pcm_sw_params_current(_handle, swparams);
		if (err) {
			mp4_log_err("snd_pcm_sw_params_current: %s\n", snd_strerror(err));
			return;
		}

		/* enable tstamp */
		err = snd_pcm_sw_params_set_tstamp_mode(_handle, swparams, SND_PCM_TSTAMP_ENABLE);
		if (err < 0) {
			mp4_log_err("Unable to set tstamp mode : %s\n", snd_strerror(err));
			return;
		}

		err = snd_pcm_sw_params_set_tstamp_type(_handle, swparams, SND_PCM_TSTAMP_TYPE_MONOTONIC);
		if (err < 0) {
			mp4_log_err("Unable to set tstamp type : %s\n", snd_strerror(err));
			return;
		}

		/* write the sw parameters */
		err = snd_pcm_sw_params(_handle, swparams);
		if (err < 0) {
			mp4_log_err("Unable to set swparams_p : %s\n", snd_strerror(err));
			return;
		}

		snd_pcm_tstamp_type_t tstamp_type;
		snd_pcm_sw_params_get_tstamp_type(swparams, &tstamp_type);
		mp4_log_info("tstamp_type = %d\n", static_cast<int>(tstamp_type));
	}

	_ready = true;
}

int PcmSound::setGain(int gain)
{
	snd_mixer_t *handle = nullptr;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name[] = { "Gain(In)", "Gain(Out)" };

	/* open an empty mixer */
	int err = snd_mixer_open(&handle, 0);
	if (err < 0) {
		mp4_log_err("snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		mp4_log_err("snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, nullptr, nullptr);
	if (err < 0) {
		mp4_log_err("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		mp4_log_err("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
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
			mp4_log_err("snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
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
			mp4_log_err("snd_mixer_selem_set_playback_volume_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}
	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

PcmSound::~PcmSound()
{
	snd_pcm_drain(_handle);
	snd_pcm_close(_handle);
}

long PcmSound::takeSamples(void *buffer, unsigned int frames)
{
	int err = snd_pcm_readi(_handle, buffer, frames);
	if (err == EPIPE) {
		mp4_log_err("Overrun occurred: %s\n", snd_strerror(err));
		snd_pcm_prepare(_handle);
		return err;
	}
	if (err < 0) {
		mp4_log_err("Error from read: %s\n", snd_strerror(err));
		return err;
	}
	mp4_log_debug("snd_pcm_readi() => %d\n", err);
	return err;
}

bool PcmSound::reportTimestamp(snd_htimestamp_t& timestamp, snd_htimestamp_t& trigger_timestamp,
                               snd_htimestamp_t& audio_timestamp, snd_pcm_uframes_t& available,
                               snd_pcm_sframes_t& delay)
{
	int err;
	snd_pcm_status_t *status;

	snd_pcm_status_alloca(&status);
	if ((err = snd_pcm_status(_handle, status)) < 0) {
		mp4_log_err("Stream status error: %s\n", snd_strerror(err));
		return false;
	}
	snd_pcm_status_get_trigger_htstamp(status, &trigger_timestamp);
	snd_pcm_status_get_htstamp(status, &timestamp);
	snd_pcm_status_get_audio_htstamp(status, &audio_timestamp);
	available = snd_pcm_status_get_avail(status);
	delay = snd_pcm_status_get_delay(status);
	return true;
}
