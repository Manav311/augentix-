#define _GNU_SOURCE
#include "ado_ctrl.h"
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <alsa/asoundlib.h>
#include <alsa/pcm.h>
#include <alsa/hwdep.h>
#include <alsa/error.h>
#include <pcm_interfaces.h>
#include "acodec.h"

#define ADO_THREAD_RECORD_NAME "ado_record"
pthread_t g_threadRecord;

struct ado_attr {
	snd_pcm_t *handle;
	snd_pcm_uframes_t frames;
	snd_pcm_hw_params_t *params;
	snd_pcm_format_t format;
	codec_mode_t codec;
	int volume;
	int violation_en;
	unsigned int channels;
	unsigned int rate;
	unsigned int bytes_per_sample;
	snd_pcm_stream_t stream;
	struct ado_callback_list *cb_list;
};

static char *g_ado_device = "default";
static struct ado_attr *g_adoin_attr = NULL;
static struct ado_attr *g_adoout_attr = NULL;

static void ADO_initAttr(struct ado_attr *attr)
{
	attr->format = SND_PCM_FORMAT_S16_LE;
	attr->channels = 1;
	attr->rate = 8000;
	attr->frames = FRAME_SIZE;
	attr->codec = RAW;
	attr->volume = 0;
	attr->violation_en = 0;
	attr->bytes_per_sample = 2;
	attr->cb_list = NULL;
}

static int ADO_initParams(struct ado_attr *attr)
{
	int ret = -1;
	snd_pcm_t *handle;
	snd_pcm_hw_params_t *params;
	snd_pcm_uframes_t frames = attr->frames;
	snd_pcm_format_t format = attr->format;
	unsigned int channels = attr->channels;
	unsigned int rate = attr->rate;

	/* Open PCM capture device. */
	ret = snd_pcm_open(&attr->handle, g_ado_device, attr->stream, 0);
	if (ret < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(ret));
		attr->handle = NULL;
		return ret;
	}
	handle = attr->handle;

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&attr->params);
	params = attr->params;

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);

	/* Interleaved mode */
	ret = snd_pcm_hw_params_set_access(attr->handle, attr->params,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (ret < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_access: %s\n", snd_strerror(ret));
		return ret;
	}

	/* Set format */
	ret = snd_pcm_hw_params_set_format(handle, params, format);
	if (ret < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_format: %s\n", snd_strerror(ret));
		goto error;
	}

	/* Set channels number */
	ret = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (ret < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_channels: %s\n", snd_strerror(ret));
		goto error;
	}

	/* Set sampling rate */
	ret = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	if (ret < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(ret));
		goto error;
	}

	/* Set period size */
	ret = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, 0);
	if (ret < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(ret));
		goto error;
	}

	/* Write the parameters to the driver */
	ret = snd_pcm_hw_params(handle, params);
	if (ret < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(ret));
		goto error;
	}

error:
	return ret;
}

static int ADO_regCallback(struct ado_attr *attr)
{
	int ret = 0;
	const char *codec_path_prefix = "/sys/module/hc18xx_";
	char codecs[] = CODECS;
	char *codecs_ptr = codecs;
	char *codec;
	char codec_path[32];

	codec = strtok_r(codecs_ptr, " ", &codecs_ptr);
	while (codec != NULL) {
		/* 1. Check audio codec driver */
		sprintf(codec_path, "%s%s", codec_path_prefix, codec);
		if (access(codec_path, F_OK) != -1) {
			/* 2. register callback by audio codec */
			if (attr == g_adoin_attr) {
				/* Add new codec capture callback here */
				if (strcmp(codec, "ak4637") == 0) {
					ret = regAdoiCallback_ak4637();
				} else if (strcmp(codec, "cjc8990") == 0) {
					ret = regAdoiCallback_cjc8990();
				} else if (strcmp(codec, "rt5660") == 0) {
					ret = regAdoiCallback_rt5660();
				} else if (strcmp(codec, "wm8731") == 0) {
					ret = regAdoiCallback_wm8731();
				} else if (strcmp(codec, "adc") == 0) {
					ret = regAdoiCallback_adc();
				} else if (strcmp(codec, "dummy") == 0) {
					ret = regAdoiCallback_dummy();
				} else {
					ret = -1;
				}
			} else if (attr == g_adoout_attr) {
				/* Add new codec playback callback here */
				if (strcmp(codec, "ak4637") == 0) {
					ret = regAdooCallback_ak4637();
				} else if (strcmp(codec, "cjc8990") == 0) {
					ret = regAdooCallback_cjc8990();
				} else if (strcmp(codec, "rt5660") == 0) {
					ret = regAdooCallback_rt5660();
				} else if (strcmp(codec, "wm8731") == 0) {
					ret = regAdooCallback_wm8731();
				} else if (strcmp(codec, "dummy") == 0) {
					ret = regAdooCallback_dummy();
				} else {
					ret = -1;
				}
			} else {
				assert(0);
			}

			if (ret == 0) {
				printf("Audio codec %s callback registered\n", codec);
			} else {
				printf("Audio codec %s callback register failed: %d\n", codec, ret);
			}
		}
		codec = strtok_r(codecs_ptr, " ", &codecs_ptr);
	}
	return ret;
}

static int _ADO_startSystem(struct ado_attr *attr)
{
	return ADO_initParams(attr);
}

static int _ADO_setVolume(struct ado_attr *attr, unsigned int volume)
{
	int ret = -1;
	struct ado_callback_list *cb_ptr;

	if (volume > 100) {
		return -EINVAL;
	}

	cb_ptr = attr->cb_list;
	while (cb_ptr != NULL) {
		if (cb_ptr->cb != NULL && cb_ptr->cb->set_volume != NULL) {
			ret = cb_ptr->cb->set_volume(volume);
		}
		cb_ptr = cb_ptr->next;
	}

	attr->volume = volume;

	return ret;
}

static int _ADO_getDbGain(struct ado_attr *attr, int volume_before, int volume_after, float *db_gain)
{
	struct ado_callback_list *cb_ptr;

	*db_gain = 0;

	if ((volume_before < 0 || volume_before > 100)
		|| (volume_after < 0 || volume_after > 100)) {
		return -EINVAL;
	}

	cb_ptr = attr->cb_list;
	while (cb_ptr != NULL) {
		if (cb_ptr->cb != NULL && cb_ptr->cb->get_db_gain != NULL) {
			return cb_ptr->cb->get_db_gain(volume_before, volume_after, db_gain);
		}
		cb_ptr = cb_ptr->next;
	}

	return -ENOENT;
}

static inline unsigned int _ADO_getVolume(struct ado_attr *attr)
{
	return attr->volume;
}

static inline void _ADO_setRate(struct ado_attr *attr, unsigned int rate)
{
	attr->rate = rate;
}

static inline unsigned int _ADO_getRate(struct ado_attr *attr)
{
	return attr->rate;
}

static inline void _ADO_setChannels(struct ado_attr *attr, unsigned int channels)
{
	attr->channels = channels;
}

static inline unsigned int _ADO_getChannels(struct ado_attr *attr)
{
	return attr->channels;
}

static inline void _ADO_setFormat(struct ado_attr *attr, snd_pcm_format_t format)
{
	attr->format = format;
	if ((format == SND_PCM_FORMAT_S16_LE) || (format == SND_PCM_FORMAT_S16_BE)) {
		attr->bytes_per_sample = 2;
	} else if ((format == SND_PCM_FORMAT_A_LAW) || (format == SND_PCM_FORMAT_MU_LAW)){
		attr->bytes_per_sample = 1;
	}
}

static inline snd_pcm_format_t _ADO_getFormat(struct ado_attr *attr)
{
	return attr->format;
}

static inline void _ADO_setPeriodSize(struct ado_attr *attr, snd_pcm_uframes_t frames)
{
	attr->frames = frames;
}

static inline snd_pcm_uframes_t _ADO_getPeriodSize(struct ado_attr *attr)
{
	return attr->frames;
}

static int _ADO_regCallback(struct ado_attr *attr, struct ado_callback *cb)
{
	struct ado_callback_list *cb_ptr = NULL;
	struct ado_callback_list *cb_ptr_pre = NULL;

	if (cb == NULL) {
		return -EINVAL;
	}

	if (attr->cb_list == NULL) {
		attr->cb_list = malloc(sizeof(struct ado_callback_list));
		attr->cb_list->next = NULL;
		attr->cb_list->cb = cb;
	} else {
		cb_ptr = attr->cb_list;

		while (cb_ptr != NULL) {
			cb_ptr_pre = cb_ptr;
			cb_ptr = cb_ptr->next;
		}

		cb_ptr = malloc(sizeof(struct ado_callback_list));
		if (cb_ptr_pre != NULL) {
			cb_ptr_pre->next = cb_ptr;
		}

		cb_ptr->next = NULL;
		cb_ptr->cb = cb;
	}

	return 0;
}

/**
 * @Set audio system dram violation enable/disable.
 * @returns 0 - success, others - failure.
 */
int ADO_setViolation(int enable)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep = NULL;
	int ret = -1;

	if ((ret = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		printf("hwdep interface open error: %s \n",snd_strerror(ret));
		return ret;
	}

	if (0 == enable) {
		ret = snd_hwdep_ioctl(hwdep, HC_IOCTL_VIOLATION_DIS, NULL);
		if (ret < 0) {
			printf("hwdep ioctl error: %s\n", snd_strerror(ret));
		}
	} else {
		ret = snd_hwdep_ioctl(hwdep, HC_IOCTL_VIOLATION_EN, NULL);
		if (ret < 0) {
			printf("hwdep ioctl error: %s\n", snd_strerror(ret));
		}
	}

	snd_hwdep_close(hwdep);

	return ret;
}

/**
 * @Init audio input system.
 * @returns 0 - success, others - failure.
 */
int ADOI_initSystem(void)
{
	if (g_adoin_attr != NULL) {
		return -EBUSY;
	}

	g_adoin_attr = (struct ado_attr *)malloc(sizeof(struct ado_attr));
	g_adoin_attr->stream = SND_PCM_STREAM_CAPTURE;

	ADO_initAttr(g_adoin_attr);
	ADO_regCallback(g_adoin_attr);

	return 0;
}

/**
 * @Start audio input system.
 * @returns 0 - success, others - failure.
 */
int ADOI_startSystem(void)
{
	return _ADO_startSystem(g_adoin_attr);
}

/**
 * @Stop audio input system.
 * @returns 0 - success, others - failure.
 */
void ADOI_stopSystem(void)
{
	if (g_adoin_attr != NULL) {
		snd_pcm_drop(g_adoin_attr->handle);
		snd_pcm_close(g_adoin_attr->handle);
	}
}

/**
 * @Close audio input system.
 * @returns 0 - success, others - failure.
 */
void ADOI_closeSystem(void)
{
	struct ado_callback_list *cb_ptr;
	struct ado_callback_list *cb_ptr_pre;

	if (g_adoin_attr != NULL) {
		cb_ptr = g_adoin_attr->cb_list;
		while (cb_ptr != NULL) {
			cb_ptr_pre = cb_ptr;
			cb_ptr = cb_ptr_pre->next;
			free(cb_ptr_pre);
		}

		g_adoin_attr->handle = NULL;
		g_adoin_attr->params = NULL;
		free(g_adoin_attr);
		g_adoin_attr = NULL;
	}
}

/**
 * @Set audio input gain.
 * @param[in] gain - gain(dB)
 * @returns 0 - success, others - failure.
 */
int ADOI_setVolume(unsigned int volume)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_setVolume(g_adoin_attr, volume);
}

/**
 * @Set audio input gain.
 * @param[out] gain - gain(dB)
 * @returns 0 - success, others - failure.
 */
unsigned int ADOI_getVolume(void){
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getVolume(g_adoin_attr);
}

/**
 * @Set audio input codec mode.
 * @param[in] mode - codec mode
 * @returns 0 - success, others - failure.
 */
int ADOI_setCodecMode(codec_mode_t mode)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep = NULL;
	int ret = -1;

	if ((ret = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		printf("hwdep interface open error: %s \n",snd_strerror(ret));
		return ret;
	}

	if ((ret = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_ENC_MODE, &mode)) < 0) {
		printf("hwdep ioctl error: %s \n",snd_strerror(ret));
	}

	snd_hwdep_close(hwdep);

	return ret;
}

/**
 * @Get audio input codec mode.
 * @param[out] mode - codec mode
 * @returns 0 - success, others - failure.
 */
int ADOI_getCodecMode(codec_mode_t *mode)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep = NULL;
	int ret = -1;

	if ((ret = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		printf("hwdep interface open error: %s \n",snd_strerror(ret));
		return ret;
	}

	if ((ret = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_ENC_MODE, &mode)) < 0) {
		printf("hwdep ioctl error: %s \n",snd_strerror(ret));
	}

	snd_hwdep_close(hwdep);

	return ret;
}

/**
 * @Set audio input format.
 * @param[in] format - format.
 * @returns 0 - success, others - failure.
 */
int ADOI_setFormat(snd_pcm_format_t format)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setFormat(g_adoin_attr, format);
	return 0;
}

/**
 * @Get audio input format.
 * @returns 0 - success, others - failure.
 */
snd_pcm_format_t ADOI_getFormat(void)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getFormat(g_adoin_attr);
}

/**
 * @Set audio input channel number.
 * @param[in] channels - channel number.
 * @returns 0 - success, others - failure.
 */
int ADOI_setChannels(unsigned int channels)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setChannels(g_adoin_attr, channels);
	return 0;
}

/**
 * @Get audio input channel number.
 * @returns channel number.
 */
unsigned int ADOI_getChannels(void)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getChannels(g_adoin_attr);
}

/**
 * @Set audio input sampling rate.
 * @param[in][out] rate - sampling rate.
 * @returns 0 - success, others - failure.
 */
int ADOI_setRate(unsigned int rate)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setRate(g_adoin_attr, rate);
	return 0;
}

/**
 * @Get audio input sampling rate.
 * @returns 0 - success, others - failure.
 */
unsigned int ADOI_getRate(void)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getRate(g_adoin_attr);
}

/**
 * @Set audio input period size in frame.
 * @param[in][out] frame - period size.
 * @returns period size.
 */
int ADOI_setPeriodSize(snd_pcm_uframes_t frames)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setPeriodSize(g_adoin_attr, frames);
	return 0;
}

/**
 * @Get audio input period size in frame.
 * @returns 0 - success, others - failure.
 */
snd_pcm_uframes_t ADOI_getPeriodSize(void)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getPeriodSize(g_adoin_attr);
}

/**
 * @Get audio input bitstream.
 * @param[out] buf - buffer for bitstream.
 * @returns > 0 - success, others - failure.
 */
int ADOI_getBitStream(char *buf)
{
	int ret = -1;
	snd_pcm_t *handle = NULL;
	snd_pcm_uframes_t frames = 0;
	unsigned int channels = 0;
	unsigned int bytes_per_sample = 0;

	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	handle = g_adoin_attr->handle;
	frames = g_adoin_attr->frames;
	channels = g_adoin_attr->channels;
	bytes_per_sample = g_adoin_attr->bytes_per_sample;

	if (handle == NULL) {
		return -EINVAL;
	}

	ret = snd_pcm_readi(handle, buf, frames);
	if (ret == -EPIPE) {
		/* EPIPE means overrun */
		snd_pcm_prepare(handle);
	}

	if (ret < 0) {
		return ret;
	}

	return ret * channels * bytes_per_sample;
}

/**
 * @Get volume_after - volume_before Db difference.
 * @returns INT_MIN - failure, others - Db difference.
 */
int ADOI_getDbGain(int volume_before, int volume_after, float *db_gain)
{
	return _ADO_getDbGain(g_adoin_attr, volume_before, volume_after, db_gain);
}

/**
 * @Register audio input callback functions.
 * @returns 0 - success, others - failure.
 */
int ADOI_regCallback(struct ado_callback *cb)
{
	if (g_adoin_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_regCallback(g_adoin_attr, cb);
}

/**
 * @Init audio output system.
 * @returns 0 - success, others - failure.
 */
int ADOO_initSystem(void)
{
	if (g_adoout_attr != NULL) {
		return -EBUSY;
	}

	g_adoout_attr = (struct ado_attr *)malloc(sizeof(struct ado_attr));
	g_adoout_attr->stream = SND_PCM_STREAM_PLAYBACK;

	ADO_initAttr(g_adoout_attr);
	ADO_regCallback(g_adoout_attr);

	return 0;
}

/**
 * @Start audio output system.
 * @returns 0 - success, others - failure.
 */
int ADOO_startSystem(void)
{
	return _ADO_startSystem(g_adoout_attr);
}

/**
 * @Stop audio output system.
 * @returns 0 - success, others - failure.
 */
void ADOO_stopSystem(void)
{
	if (g_adoout_attr != NULL) {
		snd_pcm_drain(g_adoout_attr->handle);
		snd_pcm_close(g_adoout_attr->handle);
	}
}

/**
 * @Close audio output system.
 * @returns 0 - success, others - failure.
 */
void ADOO_closeSystem(void)
{
	struct ado_callback_list *cb_ptr;
	struct ado_callback_list *cb_ptr_pre;

	if (g_adoout_attr != NULL) {
		cb_ptr = g_adoout_attr->cb_list;
		while (cb_ptr != NULL) {
			cb_ptr_pre = cb_ptr;
			cb_ptr = cb_ptr_pre->next;
			free(cb_ptr_pre);
		}

		g_adoout_attr->handle = NULL;
		g_adoout_attr->params = NULL;
		free(g_adoout_attr);
		g_adoout_attr = NULL;
	}
}

/**
 * @Set audio output volume.
 * @returns 0 - success, others - failure.
 */
int ADOO_setVolume(unsigned int volume)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_setVolume(g_adoout_attr, volume);
}

/**
 * @Get audio output volume.
 * @returns 0 - success, others - failure.
 */
unsigned int ADOO_getVolume(void)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getVolume(g_adoout_attr);
}

/**
 * @Set audio output codec mode.
 * @param[in] mode - codec mode
 * @returns 0 - success, others - failure.
 */
int ADOO_setCodecMode(codec_mode_t mode)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep = NULL;
	int ret = -1;

	if ((ret = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		printf("hwdep interface open error: %s \n",snd_strerror(ret));
		return ret;
	}

	if ((ret = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_DEC_MODE, &mode)) < 0) {
		printf("hwdep ioctl error: %s \n",snd_strerror(ret));
	}

	snd_hwdep_close(hwdep);

	return ret;
}

/**
 * @Get audio output codec mode.
 * @param[out] mode - codec mode
 * @returns 0 - success, others - failure.
 */
int ADOO_getCodecMode(codec_mode_t *mode)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep;
	int ret = -1;

	if ((ret = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		printf("hwdep interface open error: %s \n",snd_strerror(ret));
		return ret;
	}

	if ((ret = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_DEC_MODE, &mode)) < 0) {
		printf("hwdep ioctl error: %s \n",snd_strerror(ret));
	}

	snd_hwdep_close(hwdep);

	return ret;
}

/**
 * @Set audio output format.
 * @returns 0 - success, others - failure.
 */
int ADOO_setFormat(snd_pcm_format_t format)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setFormat(g_adoout_attr, format);
	return 0;
}

/**
 * @Get audio output format.
 * @returns 0 - success, others - failure.
 */
snd_pcm_format_t ADOO_getFormat(void)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getFormat(g_adoout_attr);
}

/**
 * @Set audio output channel number.
 * @returns 0 - success, others - failure.
 */
int ADOO_setChannels(unsigned int channels)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setChannels(g_adoout_attr, channels);
	return 0;
}

/**
 * @Get audio output channel number.
 * @returns 0 - success, others - failure.
 */
unsigned int ADOO_getChannels()
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getChannels(g_adoout_attr);
}

/**
 * @Set audio output sampling rate.
 * @returns 0 - success, others - failure.
 */
int ADOO_setRate(unsigned int rate)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setRate(g_adoout_attr, rate);
	return 0;
}

/**
 * @Get audio output sampling rate.
 * @returns 0 - success, others - failure.
 */
unsigned int ADOO_getRate()
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getRate(g_adoout_attr);
}

/**
 * @Set audio output period size in frame.
 * @param[in][out] frame - period size.
 * @returns period size.
 */
int ADOO_setPeriodSize(snd_pcm_uframes_t frames)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	_ADO_setPeriodSize(g_adoout_attr, frames);
	return 0;
}

/**
 * @Get audio output period size in frame.
 * @returns 0 - success, others - failure.
 */
snd_pcm_uframes_t ADOO_getPeriodSize(void)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_getPeriodSize(g_adoout_attr);
}

/**
 * @Write audio output bitstream.
 * @returns > 0 - frames size, < 0 - failure.
 */
int ADOO_setBitStream(char *buf, int size_in_byte, int nonblock)
{
	snd_pcm_t *handle = NULL;
	snd_pcm_uframes_t frames = 0;
	unsigned int channels = 0;
	unsigned int bytes_per_sample = 0;

	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	handle = g_adoout_attr->handle;
	frames = g_adoout_attr->frames;
	channels = g_adoout_attr->channels;
	bytes_per_sample = g_adoout_attr->bytes_per_sample;

	if (handle == NULL) {
		return -EINVAL;
	}

	snd_pcm_nonblock(handle, nonblock);

	frames = snd_pcm_writei(handle, buf, (size_in_byte / bytes_per_sample / channels));
	if ((int)frames == -EPIPE) {
		snd_pcm_prepare(handle);
	}

	return (int)frames;
}

/**
 * @Get volume_after - volume_before Db difference.
 * @returns INT_MIN - failure, others - Db difference.
 */
int ADOO_getDbGain(int volume_before, int volume_after, float *db_gain)
{
	return _ADO_getDbGain(g_adoout_attr, volume_before, volume_after, db_gain);
}

/**
 * @Register audio output callback functions.
 * @returns 0 - success, others - failure.
 */
int ADOO_regCallback(struct ado_callback *cb)
{
	if (g_adoout_attr == NULL) {
		return -EINVAL;
	}

	return _ADO_regCallback(g_adoout_attr, cb);
}

static void _ADO_recordThreadCleanUpFd(void *arg)
{
	int fd = *(int *)arg;

	close(fd);
	ADOI_stopSystem();
	ADOI_closeSystem();
}

static void _ADO_recordThreadCleanUpBuf(void *arg)
{
	char *buf = arg;

	free(buf);
}

static void _ADO_recordThreadCleanUpShm(void *arg)
{
	int shmid = *(int *)arg;

	shmctl(shmid, IPC_RMID, NULL);
}

static void * _ADO_recordThread(void *arg)
{
	int fd;
	int ret = -1;
	char *buf;
	int buf_size = 0;
	int data_size = 0;
	char *file = arg;
	int *stop_flag = NULL;
	int stop = 0;
	int shmid;
	key_t key;

	ret = ADOI_initSystem();
	if (ret < 0) {
		printf("%s: init system failed\n", __func__);
		return NULL;
	}

	ret = ADOI_startSystem();
	if (ret < 0) {
		printf("%s: start system failed\n", __func__);
		return NULL;
	}

	fd = open(file, O_CREAT | O_RDWR | O_TRUNC);
#define BUFFER_SIZE(attr) (attr->frames * attr->channels * SAMPLE_SIZE_IN_BYTE)
	buf_size = BUFFER_SIZE(g_adoin_attr);
	buf = (char *)malloc(buf_size);

	key = ftok("/system/", 10);
	shmid = shmget(key, sizeof(int), IPC_CREAT | IPC_EXCL);
	if (shmid < 0) {
		shmid = shmget(key, sizeof(int), 0);
	}
	stop_flag = shmat(shmid, NULL, 0);
	stop_flag = 0;
	shmdt(stop_flag);

	pthread_cleanup_push(_ADO_recordThreadCleanUpShm, (void *)&shmid);
	pthread_cleanup_push(_ADO_recordThreadCleanUpBuf, (void *)buf);
	pthread_cleanup_push(_ADO_recordThreadCleanUpFd, (void *)&fd);

	do {
		pthread_testcancel();
		data_size = ADOI_getBitStream(buf);
		write(fd, buf, data_size);
		stop_flag = shmat(shmid, NULL, 0);
		stop = *stop_flag;
		shmdt(stop_flag);
	} while (stop == 0);

	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);
	pthread_cleanup_pop(1);

	return NULL;
}

/**
 * @Create a thread continuing record bitstream to <file>.
 * @param[in] file: the file with path to record to.
 * @returns 0 - success, others - failure.
 */
int ADO_recordFileStart(char *file)
{
	if ((pthread_create(&g_threadRecord, NULL, _ADO_recordThread, (void *)file)) < 0) {
		return -1;
	}
	pthread_setname_np(g_threadRecord, ADO_THREAD_RECORD_NAME);

	return pthread_detach(g_threadRecord);
}

/**
 * @Stop the record thread.
 * @returns 0 - success, others - failure.
 */
int ADO_recordFileStop(void)
{
	int ret;
	int shmid;
	key_t key;
	int *stop_flag = NULL;

	key = ftok("/system/", 10);
	shmid = shmget(key, sizeof(int), 0);
	if (shmid < 0) {
		return -1;
	}
	stop_flag = shmat(shmid, NULL, 0);
	*stop_flag = 1;
	ret = shmdt(stop_flag);
	if (ret < 0) {
		return -1;
	}
	return 0;
}

/**
 * @Play <file>.
 * @param[in] file: the file with path to play.
 * @returns 0 - success, others - failure.
 */
int ADO_playFile(char *file)
{
	int fd;
	int ret = -1;
	char *buf;
	int buf_size = 0;
	int data_size = 0;

	ret = ADOO_initSystem();
	if (ret < 0) {
		printf("%s: init system failed\n", __func__);
		return ret;
	}

	ret = ADOO_startSystem();
	if (ret < 0) {
		printf("%s: start system failed\n", __func__);
		return ret;
	}

	fd = open(file, O_CREAT | O_RDWR);
#define BUFFER_SIZE(attr) (attr->frames * attr->channels * SAMPLE_SIZE_IN_BYTE)
	buf_size = BUFFER_SIZE(g_adoout_attr);
	buf = (char *)malloc(buf_size);
	while ((data_size = read(fd, buf, buf_size)) > 0) {
		ret = ADOO_setBitStream(buf, buf_size, 0);
		if (ret < 0) {
			break;
		}
	}

	close(fd);
	ADOO_stopSystem();
	ADOO_closeSystem();
	free(buf);

	return ret;
}
