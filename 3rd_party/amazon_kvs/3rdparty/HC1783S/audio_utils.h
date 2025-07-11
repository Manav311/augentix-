#ifndef HC1783S_AUDIO_UTILS_H
#define HC1783S_AUDIO_UTILS_H

#include <stdbool.h>

#include "alsa/asoundlib.h"
#include "alsa/error.h"
#include "pcm_interfaces.h"

#define DEBUG_ENABLED 0
#define log_err(fmt, args...) printf("[ERROR] " fmt, ##args)
#define log_warn(fmt, args...) printf("[WARNING] " fmt, ##args)
#define log_notice(fmt, args...) printf("[NOTICE] " fmt, ##args)
#define log_info(fmt, args...) printf("[INFO] " fmt, ##args)

#if DEBUG_ENABLED
#define log_debug(fmt, args...) printf("[DEBUG] " fmt, ##args)
#else
#define log_debug(fmt, args...)
#endif

typedef struct pcm_sound {
    snd_pcm_t *handle;
    const char *device;
    snd_pcm_hw_params_t *params;
    unsigned int sampleRate;
    snd_pcm_uframes_t frames;
    snd_pcm_format_t format;
    unsigned int channels;
    unsigned int bytesPerFrame;
    unsigned int sampleSize;
    char *buffer;
    bool timestampEnabled;
} PcmSound;

int setGain(int gain);
PcmSound *PcmSound_new(snd_pcm_format_t format, unsigned int bytesPerFrame, int sampleRate,
                       int gain, bool enableTimestamp);
void PcmSound_dispose(PcmSound *self);
char *PcmSound_takeSample(PcmSound *self, unsigned int *pSampleSize);
bool PcmSound_reportTimestamp(PcmSound *self,
                              snd_htimestamp_t *pTimestamp, snd_htimestamp_t *pTriggerTimestamp,
                              snd_htimestamp_t *pAudioTimestamp, snd_pcm_uframes_t *pAvailable,
                              snd_pcm_sframes_t *pDelay);

#endif //HC1783S_AUDIO_UTILS_H
