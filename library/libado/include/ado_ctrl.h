#ifndef ADO_CTRL_H_
#define ADO_CTRL_H_

#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <pcm_interfaces.h>
#include "ado_cb.h"

#define FRAME_SIZE 256
#define SAMPLE_SIZE_IN_BYTE 2

#ifndef ADOI_REG_CALLBACK
#define ADOI_REG_CALLBACK()
#endif

#ifndef ADOO_REG_CALLBACK
#define ADOO_REG_CALLBACK()
#endif

/* Debug */
int ADO_setViolation(int enable);

/* Capture */
int ADOI_initSystem(void);
int ADOI_startSystem(void);
void ADOI_stopSystem(void);
void ADOI_closeSystem(void);
int ADOI_setVolume(unsigned int volume);
unsigned int ADOI_getVolume(void);
int ADOI_setCodecMode(codec_mode_t mode);
int ADOI_getCodecMode(codec_mode_t *mode);
int ADOI_setFormat(snd_pcm_format_t format);
snd_pcm_format_t ADOI_getFormat(void);
int ADOI_setChannels(unsigned int channels);
unsigned int ADOI_getChannels(void);
int ADOI_setRate(unsigned int rate);
unsigned int ADOI_getRate(void);
int ADOI_setPeriodSize(snd_pcm_uframes_t frame);
snd_pcm_uframes_t ADOI_getPeriodSize(void);
int ADOI_getBitStream(char *buf);
int ADOI_getDbGain(int volume_before, int volume_after, float *db_gain);
int ADOI_regCallback(struct ado_callback *cb);

/* Playback */
int ADOO_initSystem(void);
int ADOO_startSystem(void);
void ADOO_stopSystem(void);
void ADOO_closeSystem(void);
int ADOO_setVolume(unsigned int volume);
unsigned int ADOO_getVolume(void);
int ADOO_setCodecMode(codec_mode_t mode);
int ADOO_getCodecMode(codec_mode_t *mode);
int ADOO_setFormat(snd_pcm_format_t format);
snd_pcm_format_t ADOO_getFormat(void);
int ADOO_setChannels(unsigned int channels);
unsigned int ADOO_getChannels(void);
int ADOO_setRate(unsigned int rate);
unsigned int ADOO_getRate(void);
int ADOO_setPeriodSize(snd_pcm_uframes_t frame);
snd_pcm_uframes_t ADOO_getPeriodSize(void);
int ADOO_setBitStream(char *buf, int size_in_byte, int nonblock);
int ADOO_getDbGain(int volume_before, int volume_after, float *db_gain);
int ADOO_regCallback(struct ado_callback *cb);

/* very start audio library for autotest */
int ADO_recordFileStart(char *file);
int ADO_recordFileStop(void);
int ADO_playFile(char *file);

#endif /* ADO_CTRL_H_ */
