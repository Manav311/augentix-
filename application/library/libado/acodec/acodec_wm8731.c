#include "ado_ctrl.h"

extern int digital_1db[100];
extern int digital_1_5db[100];

static int ACODECI_getGain(unsigned int volume)
{
	int vol_mute = 0;
	int vol_offset = -22;
	int *vol_table = digital_1_5db;

	if (volume > 100) {
		printf("Wrong volume value %d\n",volume);
		return -EINVAL;
	}

	if (volume == 0) {
		return vol_mute;
	}

	if (vol_table[volume - 1] + vol_offset <= 0) {
		return 1;
	}

	return vol_table[volume - 1] + vol_offset;
}

static int ACODECI_setVolume(unsigned int volume)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Capture";
	int vol_mixer = 0;

	if (volume > 100) {
		printf("Wrong volume value %d\n",volume);
		return -EINVAL;
	}

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
		vol_mixer = ACODECI_getGain(volume);
		err = snd_mixer_selem_set_capture_volume_all(elem, vol_mixer);
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

static int ACODECI_getDbGain(unsigned int volume_before, unsigned int volume_after, float *db_gain)
{
	int gain_before = 0;
	int gain_after = 0;

	if ((volume_before > 100) || (volume_after > 100)) {
		printf("Wrong volume value %d, %d\n", volume_before, volume_after);
		return -EINVAL;
	}

	/* Get gain */
	gain_before = ACODECI_getGain(volume_before);
	gain_after = ACODECI_getGain(volume_after);

	/* Convert the diff to Db */
	*db_gain = (float)(gain_after - gain_before) * 1.5;

	return 0;
}

static int ACODECO_getGain(unsigned int volume)
{
	int vol_mute = 0;
	int vol_offset = 54;
	int *vol_table = digital_1db;

	if (volume > 100) {
		printf("Wrong volume value %d\n",volume);
		assert(0);
		return -EINVAL;
	}

	if (volume == 0) {
		return vol_mute;
	}

	return vol_table[volume - 1] + vol_offset;
}

static int ACODECO_setVolume(unsigned int volume)
{
	int err = 0;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Master";
	int vol_mixer;

	if (volume > 100) {
		printf("Wrong volume value %d\n",volume);
		assert(0);
		return -EINVAL;
	}

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
		vol_mixer = ACODECO_getGain(volume);
		err = snd_mixer_selem_set_playback_volume_all(elem, vol_mixer);
		if (err < 0) {
			fprintf(stderr, "snd_mixer_selem_set_playback_volume_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

static int ACODECO_getDbGain(unsigned int volume_before, unsigned int volume_after, float *db_gain)
{
	int gain_before = 0;
	int gain_after = 0;

	if ((volume_before > 100) || (volume_after > 100)) {
		printf("Wrong volume value %d, %d\n", volume_before, volume_after);
		assert(0);
		return -EINVAL;
	}

	/* Get gain */
	gain_before = ACODECO_getGain(volume_before);
	gain_after = ACODECO_getGain(volume_after);

	/* Convert the diff to Db */
	*db_gain = (float)(gain_after - gain_before);

	return 0;
}

static struct ado_callback cb[] = {
	{
		.id = ACODEC_WM8731,
		.set_volume = ACODECI_setVolume,
		.get_db_gain = ACODECI_getDbGain,
	},
	{
		.id = ACODEC_WM8731,
		.set_volume = ACODECO_setVolume,
		.get_db_gain = ACODECO_getDbGain,
	},
};

int regAdoiCallback_wm8731(void)
{
	return ADOI_regCallback(&cb[0]);
}

int regAdooCallback_wm8731(void)
{
	return ADOO_regCallback(&cb[1]);
}
