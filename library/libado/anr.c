#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "pcm_interfaces.h"
#include <alsa/asoundlib.h>
#include <alsa/hwdep.h>
#include <sys/ioctl.h>

int anr_en(int en)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep;
	int err;

	err = snd_hwdep_open(&hwdep, devicename, O_RDWR);
	if (err < 0) {
		return err;
	}

	err = snd_hwdep_ioctl(hwdep, AGTX_AUDIO_IOCTL_SET_ANR_ENABLE, &en);
	if (err < 0) {
		return err;
	}

	snd_hwdep_close(hwdep);

	return 0;
}

int anr_get_en(int *en)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep;
	int err;

	err = snd_hwdep_open(&hwdep, devicename, O_RDWR);
	if (err < 0) {
		return err;
	}

	err = snd_hwdep_ioctl(hwdep, AGTX_AUDIO_IOCTL_GET_ANR_ENABLE, en);
	if (err < 0) {
		return err;
	}

	snd_hwdep_close(hwdep);

	return 0;
}

int anr_set_params(AnrParams params)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep;
	int err;

	err = snd_hwdep_open(&hwdep, devicename, O_RDWR);
	if (err < 0) {
		return err;
	}

	err = snd_hwdep_ioctl(hwdep, AGTX_AUDIO_IOCTL_SET_ANR_PARAMS, &params);
	if (err < 0) {
		return err;
	}

	snd_hwdep_close(hwdep);

	return 0;
}

int anr_get_params(AnrParams *params)
{
	const char *devicename = "hw:0,0";
	snd_hwdep_t *hwdep;
	int err;

	err = snd_hwdep_open(&hwdep, devicename, O_RDWR);
	if (err < 0) {
		return err;
	}

	err = snd_hwdep_ioctl(hwdep, AGTX_AUDIO_IOCTL_GET_ANR_PARAMS, params);
	if (err < 0) {
		return err;
	}

	snd_hwdep_close(hwdep);

	return 0;
}
