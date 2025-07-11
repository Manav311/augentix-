#ifndef TUTK_AUDIO_H
#define TUTK_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

// audio
#include <pcm_interfaces.h>
#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <alsa/hwdep.h>

/* Audio global variable */
/* The size of audio frame sent by APP is 320 bytes and the size
 * of PCM frame is 2 bytes, so we define AUDIO_PERIOD_SIZE = 320/2 = 160 */
#define AUDIO_FRAME_SIZE (640)
#define PCM_FRAME_SIZE_IN_BYTE (2)
#define AUDIO_PERIOD_SIZE (AUDIO_FRAME_SIZE / PCM_FRAME_SIZE_IN_BYTE)
#define AUDIO_DEFAULT_OUTPUT_VOLUME (90)
#define AUDIO_DEFAULT_INPUT_VOLUME (60)

void TUTK_initAudio(void);
void TUTK_deinitAudio(void);
void *thread_AudioFrameData(void *arg);
void TUTK_AudioPlayback(char *data, int size);
//void TUTK_SirenOnOff(char *data, int size);
//void *thread_SirenAudio(void *arg);

int HandleSpeakerControl(int SID, int enable_speaker);

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
	pthread_mutex_t lock;
	uint32_t play_bmp;
	int volume;
} audio_ctrl;

#ifdef __cplusplus
}
#endif

#endif /* TUTK_AUDIO_H */