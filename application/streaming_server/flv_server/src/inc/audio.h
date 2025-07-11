#ifndef AUDIO_H_
#define AUDIO_H_

#include <alsa/asoundlib.h>
#include <alsa/hwdep.h>
#include <alsa/error.h>
#include <pcm_interfaces.h>

#include "aac.h"

int agtxPcmInit(snd_pcm_t **pcm_handle, const char *device, snd_pcm_stream_t stream, snd_pcm_format_t format,
                snd_pcm_uframes_t frame, unsigned int rate, unsigned int channels);

int agtxPcmInitNoTs(snd_pcm_t **pcm_handle, const char *device, snd_pcm_stream_t stream, snd_pcm_format_t format,
                    snd_pcm_uframes_t frame, unsigned int rate, unsigned int channels);

int agtxPcmUninit(snd_pcm_t *p_capture);

int agtxSetGain(int gain);

void agtxAudiogettimestamp(snd_pcm_t *handle, snd_htimestamp_t *timestamp, snd_htimestamp_t *trigger_timestamp,
                           snd_htimestamp_t *audio_timestamp, snd_pcm_uframes_t *avail, snd_pcm_sframes_t *delay);

long long timestamp2ns(snd_htimestamp_t t);

#endif