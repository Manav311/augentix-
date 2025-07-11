#ifndef TGC_AUDIO_H
#define TGC_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <sys/ioctl.h>

// audio
#include <alsa/asoundlib.h>
#include <pcm_interfaces.h>

void TGC_initTalkback(void);
void TGC_deinitTalkback(void);
void TGC_AudioTalkCallback(char *data, int size);
void TGC_initAudio(void);
void TGC_deinitAudio(void);
void TGC_muteAudio(bool mute);
void TGC_stopAudioRun(void);
void *thread_AudioFrameData(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* TGC_AUDIO_H */