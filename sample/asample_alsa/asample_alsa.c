#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <pcm_interfaces.h>

#define SAMPLE_SIZE_IN_BYTE 2

static char *device = "default"; /* playback device */
static snd_pcm_t *handle;
static int fd;
static char *buffer;

static void handleSigInt(int signo __attribute__((unused)))
{
	if (handle) {
		snd_pcm_abort(handle);
	}
	if (buffer) {
		free(buffer);
	}
	if (fd) {
		close(fd);
	}
	exit(0);
}

int set_gain(int gain)
{
	int err;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name[] = { "Gain(In)", "Gain(Out)" };

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

	/* set capture gain ( "Gain(In)" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name[0]);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_capture_volume_all(elem, gain);
		if (err < 0) {
			fprintf(stderr, "snd_mixer_selem_set_capture_volume_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	/* set playback gain ( "Gain(Out)" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name[1]);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_playback_volume_all(elem, gain);
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

void help()
{
	printf("Usage: asample [OPTION]... -<i/o> <FILE>\n");
	printf("\n");
	printf("-h,          help\n");
	printf("-c <#>       channel number (default: 1)\n");
	printf("-d <#>       duration # in second (capture only)\n");
	printf("-f <FORMAT>  FORMAT: S16_LE(default)/S16_BE/a-law/mu-law\n");
	printf("-r <#>       sample rate (default: 8000)\n");
	printf("-p <#>       period size (default: 256)\n");
	printf("-s <#>       buffer size read/write from/to alsa (default: 256)\n");
	printf("-g <#>       gain (default: 0)\n");
	printf("-t           print timestamp\n");
	printf("-T           tstamp type: 0-gettimeofday, 1-monotonic, 2-monotonic_raw\n");
	printf("-i           playback\n");
	printf("-o           capture\n");
	printf("\n\n");
}

long long timestamp2ns(snd_htimestamp_t t)
{
	long long nsec;

	nsec = t.tv_sec * 1000000000;
	nsec += t.tv_nsec;

	return nsec;
}

void gettimestamp(snd_pcm_t *handle, snd_htimestamp_t *timestamp,
		  snd_htimestamp_t *trigger_timestamp,
		  snd_htimestamp_t *audio_timestamp,
		  snd_pcm_uframes_t *avail, snd_pcm_sframes_t *delay)
{
	int err;
	snd_pcm_status_t *status;

	snd_pcm_status_alloca(&status);
	if ((err = snd_pcm_status(handle, status)) < 0) {
		printf("Stream status error: %s\n", snd_strerror(err));
		exit(0);
	}
	snd_pcm_status_get_trigger_htstamp(status, trigger_timestamp);
	snd_pcm_status_get_htstamp(status, timestamp);
	snd_pcm_status_get_audio_htstamp(status, audio_timestamp);
	*avail = snd_pcm_status_get_avail(status);
	*delay = snd_pcm_status_get_delay(status);
}

int main(int argc, char **argv)
{
	char *file = NULL;
	snd_pcm_uframes_t frames;
	snd_pcm_sframes_t rframes;
	snd_pcm_sframes_t wframes;
	snd_pcm_sframes_t delay;
	snd_pcm_uframes_t avail;
	snd_pcm_hw_params_t *params;
	snd_pcm_sw_params_t *swparams;
	snd_htimestamp_t tstamp;
	snd_htimestamp_t trigger_tstamp;
	snd_htimestamp_t audio_tstamp;
	long loops;
	int size;
	int dsize = 0;
	int codec = 0;
	int codec_div = 1;
	int gain = 25;
	int err;
	int c;
	unsigned int buf_sz = 256;
	unsigned int format, duration, channels, rate, stream;
	int bytes_per_sample = 2;
	int tstamp_en = 0;
	snd_pcm_tstamp_type_t tstamp_type = SND_PCM_TSTAMP_TYPE_GETTIMEOFDAY;

	format = SND_PCM_FORMAT_S16_LE;
	duration = 5;
	channels = 1;
	rate = 8000;
	frames = 256;
	stream = SND_PCM_STREAM_CAPTURE;

	if (argc < 2) {
		help();
		return EXIT_SUCCESS;
	}

	while ((c = getopt(argc, argv, "c:d:f:p:g:hi:o:r:C:s:T:t")) != -1) {
		switch (c) {
		case 'T':
			tstamp_type = atoi(optarg);
			break;
		case 'c':
			channels = atoi(optarg);
			break;
		case 'd':
			duration = atoi(optarg);
			break;
		case 'h':
			help();
			return EXIT_SUCCESS;
		case 'f':
			if (!strcasecmp(optarg, "S16_LE")) {
				format = SND_PCM_FORMAT_S16_LE;
				bytes_per_sample = 2;
			} else if (!strcasecmp(optarg, "s16_be")) {
				format = SND_PCM_FORMAT_S16_BE;
				bytes_per_sample = 2;
			} else if (!strcasecmp(optarg, "a-law")) {
				format = SND_PCM_FORMAT_A_LAW;
				bytes_per_sample = 1;
			} else if (!strcasecmp(optarg, "mu-law")) {
				format = SND_PCM_FORMAT_MU_LAW;
				bytes_per_sample = 1;
			}
			break;
		case 'p':
			frames = atoi(optarg);
			break;
		case 's':
			buf_sz = atoi(optarg);
			break;
		case 'r':
			rate = atoi(optarg);
			break;
		case 'o':
			file = optarg;
			stream = SND_PCM_STREAM_CAPTURE;
			break;
		case 'i':
			file = optarg;
			stream = SND_PCM_STREAM_PLAYBACK;
			break;
		case 'C':
			codec = atoi(optarg);
			switch(codec) {
			case G726_4_LE:
			case G726_4_BE:
				codec_div = 4;
				break;
			case G726_2_LE:
			case G726_2_BE:
				codec_div = 8;
				break;
			case A_LAW:
			case MU_LAW:
				codec_div = 2;
				break;
			case RAW:
			default:
				codec_div = 1;
				break;
			}
			break;
		case 'g':
			gain = atoi(optarg);
			break;
		case 't':
			tstamp_en = 1;
			break;
		default:
			break;
		}
	}

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		return -1;
	}

	if (!file) {
		printf("Please specify a input and output file.\n");
		return EXIT_FAILURE;
	}

	/* Open PCM device for recording (capture). */
	err = snd_pcm_open(&handle, device, stream, 0);
	if (err < 0) {
		fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_alloca(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(handle, params);

	/* Set the desired hardware parameters. */

	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(handle, params,
			SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(handle, params, format);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(handle, params, channels);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_channels: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, 0);
	if (err < 0) {
		fprintf(stderr, "snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(handle, params);
	if (err < 0) {
		fprintf(stderr, "unable to set hw parameters: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	if (tstamp_en) {
		snd_pcm_sw_params_alloca(&swparams);

		err = snd_pcm_sw_params_current(handle, swparams);
		if (err < 0) {
			fprintf(stderr, "snd_pcm_sw_params_current: %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}

		/* enable tstamp */
		err = snd_pcm_sw_params_set_tstamp_mode(handle, swparams, SND_PCM_TSTAMP_ENABLE);
		if (err < 0) {
			printf("Unable to set tstamp mode : %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}

		err = snd_pcm_sw_params_set_tstamp_type(handle, swparams, tstamp_type);
		if (err < 0) {
			printf("Unable to set tstamp type : %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}

		/* write the sw parameters */
		err = snd_pcm_sw_params(handle, swparams);
		if (err < 0) {
			printf("Unable to set swparams_p : %s\n", snd_strerror(err));
			exit(EXIT_FAILURE);
		}

		snd_pcm_sw_params_get_tstamp_type(swparams, &tstamp_type);
		printf("tstamp_type = %d\n", (int)tstamp_type);
	}

	/* Set gain */
	err = set_gain(gain);
	if (err < 0) {
		fprintf(stderr, "set_gain failed: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	if (stream == SND_PCM_STREAM_CAPTURE) {
		printf("CAPTURE: %s\n", file);
	} else {
		printf("PLAYBACK: %s\n", file);
	}
	printf("rate: %u, channels: %u, format: %u\n"
	       "codec: %d(%d), duration: %u, frames: %d\n",
	       rate, channels, format, codec, codec_div, duration, (int)frames);

	if (stream == SND_PCM_STREAM_CAPTURE) {
		fd = open(file, O_CREAT | O_RDWR | O_TRUNC);

		/* Use a buffer large enough to hold one period */
		size = buf_sz * bytes_per_sample * channels;
		buffer = (char *) malloc(size);

		/* loop for <duration> seconds */
		loops = duration * rate / buf_sz / codec_div;
		printf("buf_sz = %u, size = %d, loops = %ld\n", buf_sz, size, loops);
		wframes = buf_sz;
		while (loops > 0) {
			loops--;
retry_readi:
			err = snd_pcm_readi(handle, buffer, wframes);
			if (err == -EPIPE) {
				/* EPIPE means xrun */
				fprintf(stderr, "overrun occurred: %s\n", snd_strerror(err));
				snd_pcm_prepare(handle);
				goto retry_readi;
			} else if (err < 0) {
				fprintf(stderr, "error from read: %s\n", snd_strerror(err));
			} else if (err != (int)wframes) {
				fprintf(stderr, "short read (expected %d, read %d)\n", (int)wframes, err);
			}
			err = write(fd, buffer, size);
			if (err != size)
				fprintf(stderr, "short write: wrote %d bytes\n", err);
			if (tstamp_en) {
				struct timeval tv;
				gettimeofday(&tv, NULL);
				gettimestamp(handle, &tstamp, &trigger_tstamp, &audio_tstamp, &avail, &delay);
				printf("tv = %li_%li\nts = %li_%li\n", tv.tv_sec, tv.tv_usec, tstamp.tv_sec, tstamp.tv_nsec/1000);
				printf("tstamp = %lli, t_tstamp = %lli, a_tstamp = %lli, av = %d, dl = %d\n",
					timestamp2ns(tstamp), timestamp2ns(trigger_tstamp), timestamp2ns(audio_tstamp), (int)avail, (int)delay);
			}
		}
		free(buffer);
		close(fd);
		snd_pcm_drain(handle);
		snd_pcm_close(handle);
	} else { /* SND_PCM_STREAM_PLAYBACK */
		fd = open(file, O_RDONLY);
		if (fd > 0) {
			size = buf_sz * bytes_per_sample * channels;
			buffer = (char *)malloc(size);
			wframes = buf_sz;
			while ((dsize = read(fd, buffer, size)) > 0) {
retry_writei:
				rframes = snd_pcm_writei(handle, buffer, wframes);
				if (rframes == -EPIPE) {
					/* EPIPE means xrun */
					fprintf(stderr, "Underrun occrred: %s\n", snd_strerror(err));
					snd_pcm_prepare(handle);
					goto retry_writei;
				} else if (rframes < 0) {
					fprintf(stderr, "error from write: %s\n", snd_strerror(err));
				} else if (rframes != wframes) {
					fprintf(stderr, "Short write (expected %d, wrote %d)\n", (int)wframes, (int)rframes);
				}
				if (tstamp_en) {
					gettimestamp(handle, &tstamp, &trigger_tstamp, &audio_tstamp, &avail, &delay);
					printf("tstamp = %lli, t_tstamp = %lli, a_tstamp = %lli, av = %d, dl = %d\n",
							timestamp2ns(tstamp), timestamp2ns(trigger_tstamp), timestamp2ns(audio_tstamp), (int)avail, (int)delay);
				}
			}
			free(buffer);
			close(fd);
			snd_pcm_drop(handle);
			snd_pcm_close(handle);
		} else {
			fprintf(stderr, "Open file %s failed: %s\n", file, strerror(fd));
		}
	}

	return 0;
}
