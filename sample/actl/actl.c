#include <stdio.h>
#include <stddef.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>
#include <alsa/error.h>

int main(int argc, char **argv)
{
	int i;
	snd_ctl_t *handle = NULL;
	snd_ctl_elem_info_t *info;
	snd_ctl_elem_id_t *id;
	snd_ctl_elem_value_t *control;
	const char *card = "default";
	char buf[128];

	snd_ctl_elem_info_alloca(&info);
	snd_ctl_elem_id_alloca(&id);
	snd_ctl_elem_value_alloca(&control);

	if (((argc % 2) != 1) || argc <= 2) {
		printf("Usage: actl <[control name] [control value]> ...\n");
		exit(0);
	}

	/* open an empty mixer */
	if (snd_ctl_open(&handle, card, 0) < 0) {
		printf("[ACTL] snd_ctl_open failed\n");
		return 1;
	}

	for (i = 1; i < argc; i++) {
		sprintf(buf, "name=%s", argv[i++]);
		if (snd_ctl_ascii_elem_id_parse(id, buf) < 0) {
			printf("[ACTL] snd_ctl_ascii_elem_id_parse %s failed\n", buf);
			continue;
		}
		snd_ctl_elem_info_set_id(info, id);
		if (snd_ctl_elem_info(handle, info) < 0) {
			printf("[ACTL] snd_ctl_elem_info failed\n");
			continue;
		}
		snd_ctl_elem_value_set_id(control, id);
		if (snd_ctl_elem_read(handle, control) < 0) {
			printf("[ACTL] snd_ctl_elem_read failed\n");
			continue;
		}
		if (snd_ctl_ascii_value_parse(handle, control, info, argv[i]) < 0) {
			printf("[ACTL] snd_ctl_ascii_value_parse %s failed\n", argv[i]);
			continue;
		}
		if (snd_ctl_elem_write(handle, control) < 0) {
			printf("[ACTL] snd_ctl_elem_write %s: %s failed\n", buf, argv[i]);
			continue;
		}
	}

	snd_ctl_close(handle);
	return 0;
}
