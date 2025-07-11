#include "ado_ctrl.h"

static int adc_setVolume(const char* selem_name, int volume)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";

	/* open an empty mixer */
	err = snd_mixer_open(&handle, 0);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		fprintf(stderr, "snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}

	/* allocat an invalid snd_ mixer_selem_id_t */
	snd_mixer_selem_id_alloca(&sid);

	/* set gain */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_capture_volume_all(elem, volume);
		if (err < 0) {
			fprintf(stderr, "snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

static int adc_ADOI_setVolume(unsigned int volume) {
	/* 0~100 convert to 0~25 */
	return adc_setVolume("Gain(In)", (unsigned int)(volume >> 2));
}

static struct ado_callback cb[] = {
	{
		.id = ACODEC_ADC,
		.set_volume = adc_ADOI_setVolume,
		.get_db_gain = NULL,
	},
};

int regAdoiCallback_adc(void) {
	return ADOI_regCallback(&cb[0]);
}

int regAdooCallback_adc(void) {
	return -1;
}
