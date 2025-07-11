#ifndef _AUDIO_PROC_H_
#define _AUDIO_PROC_H_

#ifdef __cplusplus
extern "C" {
#endif

int audio_process_initial(void *arg);
void audio_process_deinitial(void);
int startAudioRun(void);
void stopAudioRun(void);
void muteAudioRun(void);
void *thread_AudioFrameData(void *arg);

#ifdef __cplusplus
}
#endif

#endif
