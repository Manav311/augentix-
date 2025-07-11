#include "audio_utils.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>


PcmSound *PcmSound_new(snd_pcm_format_t format, unsigned int bytesPerFrame, int sampleRate,
                       int gain, bool enableTimestamp)
{
    PcmSound *self = NULL;

    if (!(self = malloc(sizeof(*self)))) {
        log_err("OOM when construct PcmSound service!\n");
        return NULL;
    }

    memset(self, 0, sizeof(*self));
    self->device = "default";
    self->sampleRate = sampleRate;
    self->frames = 1024;
    self->format = format;
    self->channels = 1;
    self->bytesPerFrame = bytesPerFrame;  /* this SHOULD match with _format */

    int /* Open PCM device for recording (capture). */
    err = snd_pcm_open(&self->handle, self->device, SND_PCM_STREAM_CAPTURE, 0);
    if (err) {
        log_err("Unable to open pcm device: %s\n", snd_strerror(err));
        goto initialize_failed;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_malloc(&self->params);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(self->handle, self->params);

    /* Set the desired hardware parameters. */
    /* Interleaved mode */
    err = snd_pcm_hw_params_set_access(self->handle, self->params,
                                       SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err) {
        log_err("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
        goto initialize_failed;
    }

    /* Signed 16-bit little-endian format */
    err = snd_pcm_hw_params_set_format(self->handle, self->params, self->format);
    if (err) {
        log_err("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
        goto initialize_failed;
    }

    /* One channel */
    err = snd_pcm_hw_params_set_channels(self->handle, self->params, self->channels);
    if (err) {
        log_err("snd_pcm_hw_params_set_channels: %s\n", snd_strerror(err));
        goto initialize_failed;
    }

    /* 44100 bits/second sampling rate */
    log_debug("PCM sample rate: %u\n", _sample_rate);
    err = snd_pcm_hw_params_set_rate_near(self->handle, self->params, &self->sampleRate, NULL);
    if (err) {
        log_err("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
        goto initialize_failed;
    }
    log_debug("PCM sample update to %u\n", _sample_rate);

    /* Set period size to 1024 frames. */
    log_debug("PCM frames: %lu\n", _frames);
    err = snd_pcm_hw_params_set_period_size_near(self->handle, self->params, &self->frames, NULL);
    if (err) {
        log_err("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
        goto initialize_failed;
    }
    log_debug("PCM frames update to %lu\n", _frames);
    log_info("near period size=%lu\n", self->frames);
    self->frames = 1024;

    /* Write the parameters to the driver */
    err = snd_pcm_hw_params(self->handle, self->params);
    if (err) {
        log_err("Unable to set hw parameters: %s\n", snd_strerror(err));
        goto initialize_failed;
    }

    /* Set gain */
    err = setGain(gain);
    if (err) {
        log_err("Set mixer gain failed: %s\n", snd_strerror(err));
        goto initialize_failed;
    }

    if (enableTimestamp) {
        snd_pcm_sw_params_t *swparams;
        snd_pcm_sw_params_alloca(&swparams);

        err = snd_pcm_sw_params_current(self->handle, swparams);
        if (err) {
            log_err("snd_pcm_sw_params_current: %s\n", snd_strerror(err));
            goto initialize_failed;
        }

        /* enable tstamp */
        err = snd_pcm_sw_params_set_tstamp_mode(self->handle, swparams, SND_PCM_TSTAMP_ENABLE);
        if (err < 0) {
            log_err("Unable to set tstamp mode : %s\n", snd_strerror(err));
            goto initialize_failed;
        }

        err = snd_pcm_sw_params_set_tstamp_type(self->handle, swparams, SND_PCM_TSTAMP_TYPE_MONOTONIC);
        if (err < 0) {
            log_err("Unable to set tstamp type : %s\n", snd_strerror(err));
            goto initialize_failed;
        }

        /* write the sw parameters */
        err = snd_pcm_sw_params(self->handle, swparams);
        if (err < 0) {
            log_err("Unable to set swparams_p : %s\n", snd_strerror(err));
            goto initialize_failed;
        }

        snd_pcm_tstamp_type_t tstamp_type;
        snd_pcm_sw_params_get_tstamp_type(swparams, &tstamp_type);
        log_info("tstamp_type = %d\n", (int) tstamp_type);
    }

    self->sampleSize = self->frames * self->bytesPerFrame * self->channels;
    self->buffer = malloc(self->sampleSize);
    if (!self->buffer) {
        log_err("OOM unable to allocate buffer for PcmSound!\n");
        goto initialize_failed;
    }

    return self;

initialize_failed:
    free(self);
    return NULL;
}

void PcmSound_dispose(PcmSound *self)
{
    snd_pcm_drain(self->handle);
    snd_pcm_close(self->handle);
    free(self->buffer);
}

int setGain(int gain)
{
    snd_mixer_t *handle = NULL;
    snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selemNames[] = { "Gain(In)", "Gain(Out)" };

    /* open an empty mixer */
    int err = snd_mixer_open(&handle, 0);
    if (err < 0) {
        log_err("snd_mixer_open failed: %s\n", snd_strerror(err));
        return err;
    }
    /* attach an HCTL specified with the CTL device name to an opened mixer */
    err = snd_mixer_attach(handle, card);
    if (err < 0) {
        log_err("snd_mixer_attach failed: %s\n", snd_strerror(err));
        goto failed;
    }
    /* register mixer simple element class */
    err = snd_mixer_selem_register(handle, NULL, NULL);
    if (err < 0) {
        log_err("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
        goto failed;
    }
    /* load a mixer elements */
    err = snd_mixer_load(handle);
    if (err < 0) {
        log_err("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
        goto failed;
    }

    /* allocat an invalid snd_ mixer_selem_id_t */
    snd_mixer_selem_id_alloca(&sid);

    /* set capture gain ( "Gain(In)" ) */
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selemNames[0]);
    elem = snd_mixer_find_selem(handle, sid);
    if (elem) {
        err = snd_mixer_selem_set_capture_volume_all(elem, gain);
        if (err < 0) {
            log_err("snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
            goto failed;
        }
    }

    /* set playback gain ( "Gain(Out)" ) */
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selemNames[1]);
    elem = snd_mixer_find_selem(handle, sid);
    if (elem) {
        err = snd_mixer_selem_set_playback_volume_all(elem, gain);
        if (err < 0) {
            log_err("snd_mixer_selem_set_playback_volume_all: %s\n", snd_strerror(err));
            goto failed;
        }
    }
    snd_mixer_close(handle);
    return 0;

failed:
    snd_mixer_close(handle);
    return err;
}

char *PcmSound_takeSample(PcmSound *self, unsigned int *pSampleSize)
{
    int err = snd_pcm_readi(self->handle, self->buffer, self->frames);
    if (err == -EPIPE) {
        log_err("Overrun occurred: %s\n", snd_strerror(err));
        snd_pcm_prepare(self->handle);
        return NULL;
    } else if (err < 0) {
        log_err("Error from read: %s\n", snd_strerror(err));
        return NULL;
    }
    log_debug("snd_pcm_readi() => %d\n", err);
    *pSampleSize = err * self->bytesPerFrame * self->channels;
    return self->buffer;
}

bool PcmSound_reportTimestamp(PcmSound *self,
                              snd_htimestamp_t *pTimestamp, snd_htimestamp_t *pTriggerTimestamp,
                              snd_htimestamp_t *pAudioTimestamp, snd_pcm_uframes_t *pAvailable,
                              snd_pcm_sframes_t *pDelay)
{
    int err;
    snd_pcm_status_t *status;
    if (!pTimestamp && !pTriggerTimestamp && !pAudioTimestamp && !pAvailable && !pDelay) {
        return false;
    }

    snd_pcm_status_alloca(&status);
    if ((err = snd_pcm_status(self->handle, status)) < 0) {
        log_err("Stream status error: %s\n", snd_strerror(err));
        return false;
    }
    if (pTriggerTimestamp) {
        snd_pcm_status_get_trigger_htstamp(status, pTriggerTimestamp);
    }
    if (pTimestamp) {
        snd_pcm_status_get_htstamp(status, pTimestamp);
    }
    if (pAudioTimestamp) {
        snd_pcm_status_get_audio_htstamp(status, pAudioTimestamp);
    }
    if (pAvailable) {
        *pAvailable = snd_pcm_status_get_avail(status);
    }
    if (pDelay) {
        *pDelay = snd_pcm_status_get_delay(status);
    }
    return true;
}
