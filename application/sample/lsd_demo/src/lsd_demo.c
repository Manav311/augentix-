#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>

#include "log.h"
#include "json.h"
#include "alsa/error.h"
#include "alsa/asoundlib.h"
#include "pcm_interfaces.h"
#include "aftr_sd.h"

static sig_atomic_t g_run_flag = 1;

static snd_pcm_t *pcm_handle;
static char *pcm_handle_id = "default";
static char *buffer;

typedef struct {
	unsigned int stream_type;
	unsigned int format;
	unsigned int channels;
	unsigned int sample_rate;
	unsigned int read_frames;
	snd_pcm_uframes_t period_size;
} PCM_PARAM;

static void handleSigInt(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_run_flag = 0;
	}
}

void help()
{
	printf("Usage: lsd_demo -i [options] ...\n"
	       "Options:\n"
	       "  -i <file>        LSD config file\n"
	       "  -f <FORMAT>      FORMAT: S16_LE(default)/S16_BE/a-law/mu-law\n"
	       "  -c <arg>         channel count (default: 1)\n"
	       "  -r <arg>         sample rate (default: 8000)\n"
	       "  -p <arg>         period size (default: 1024)\n"
	       "  -s <arg>         frames read from alsa (default: 4096)\n"
	       "  -h               help\n"
	       "\n"
	       "Example:\n"
	       "  $ lsd_demo -i /system/mpp/lsd_config/lsd_conf.json\n");
}

int parseConfig(const char *file_name, AFTR_SD_PARAM_S *sd_param)
{
	json_object *root = NULL;
	json_object *child = NULL;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	if (json_object_object_get_ex(root, "volume", &child)) {
		sd_param->volume = json_object_get_int(child);
	} else {
		json_object_object_add(root, "volume", json_object_new_int(sd_param->volume));
	}

	if (json_object_object_get_ex(root, "duration", &child)) {
		sd_param->duration = json_object_get_int(child);
	} else {
		json_object_object_add(root, "duration", json_object_new_int(sd_param->duration));
	}

	if (json_object_object_get_ex(root, "suppression", &child)) {
		sd_param->suppression = json_object_get_int(child);
	} else {
		json_object_object_add(root, "suppression", json_object_new_int(sd_param->suppression));
	}

	printf("%s\n", json_object_to_json_string(root));
	json_object_put(root);

	return EXIT_SUCCESS;
}

int runLsdDetection(const int read_frames, const int buffer_size, const AFTR_SD_PARAM_S *lsd_param)
{
	int ret;

	AFTR_SD_INSTANCE_S *lsd_instance;
	AFTR_SD_STATUS_S lsd_status;

	// Create LSD instance
	lsd_instance = AFTR_SD_newInstance();
	if (!lsd_instance) {
		log_err("Failed to create LSD instance.");
		return EXIT_FAILURE;
	}

	// Set LSD parameters
	ret = AFTR_SD_setParam(lsd_instance, lsd_param);
	if (ret != 0) {
		log_err("Set LSD parameters failed. err: %d", ret);
		return EXIT_FAILURE;
	}

	while (g_run_flag) {
		// Read interleaved frames from PCM
		ret = snd_pcm_readi(pcm_handle, buffer, (snd_pcm_uframes_t)read_frames);
		if (ret == -EPIPE) {
			log_err("overrun occurred: %s", snd_strerror(ret));
			snd_pcm_prepare(pcm_handle);
			continue;
		} else if (ret < 0) {
			log_err("error from read: %s", snd_strerror(ret));
			break;
		} else if (ret != read_frames) {
			log_err("short read (expected %d, read %d)", read_frames, ret);
			snd_pcm_prepare(pcm_handle);
			continue;
		}

		// Detect loud sound
		ret = AFTR_SD_detect(lsd_instance, buffer, buffer_size, &lsd_status);
		if (ret != 0) {
			log_err("Detect loud sound failed. err: %d", ret);
			break;
		}

		// Show LSD alarm
		if (lsd_status.alarm) {
			log_info("loud sound detected!");
		}
	}

	// Delete LSD instance
	ret = AFTR_SD_deleteInstance(&lsd_instance);
	if (ret != 0) {
		log_err("Delete LSD instance failed. err: %d", ret);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int initPcm(PCM_PARAM *pcm_param)
{
	int err;
	snd_pcm_hw_params_t *hw_params;

	// Open a PCM.
	err = snd_pcm_open(&pcm_handle, pcm_handle_id, pcm_param->stream_type, 0);
	if (err < 0) {
		log_err("unable to open pcm: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}

	// Allocate a hardware parameters object.
	snd_pcm_hw_params_alloca(&hw_params);

	// Fill it in with default values.
	snd_pcm_hw_params_any(pcm_handle, hw_params);

	// Set the desired hardware parameters.
	err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		log_err("snd_pcm_hw_params_set_access: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}
	err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, pcm_param->format);
	if (err < 0) {
		log_err("snd_pcm_hw_params_set_format: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}
	err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, pcm_param->channels);
	if (err < 0) {
		log_err("snd_pcm_hw_params_set_channels: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}
	err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &pcm_param->sample_rate, 0);
	if (err < 0) {
		log_err("snd_pcm_hw_params_set_rate_near: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}
	err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &pcm_param->period_size, 0);
	if (err < 0) {
		log_err("snd_pcm_hw_params_set_period_size_near: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}

	// Write the parameters to the driver
	err = snd_pcm_hw_params(pcm_handle, hw_params);
	if (err < 0) {
		log_err("unable to set hw parameters: %s", snd_strerror(err));
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
	int c;
	int ret;
	int buffer_size;
	int bytes_per_frame = 2;
	char config_fname[256] = { 0 };

	PCM_PARAM pcm_param = {
		.stream_type = SND_PCM_STREAM_CAPTURE, // capture audio stream
		.channels = 1, // one channel
		.format = SND_PCM_FORMAT_S16_LE, // Signed 16-bit little-endian format
		.sample_rate = 8000, // 8000 bits per second
		.read_frames = 4096, // buffer size
		.period_size = 1024 // period size
	};

	AFTR_SD_PARAM_S lsd_param = { .volume = 65, .duration = 1, .suppression = 2 };

	if (argc < 2) {
		help();
		return EXIT_SUCCESS;
	}

	while ((c = getopt(argc, argv, "i:c:f:p:s:r:h")) != -1) {
		switch (c) {
		case 'i':
			sprintf(config_fname, optarg);
			log_info("Load config file: %s", config_fname);
			break;
		case 'c':
			pcm_param.channels = atoi(optarg);
			break;
		case 'f':
			if (!strcasecmp(optarg, "S16_LE")) {
				pcm_param.format = SND_PCM_FORMAT_S16_LE;
				bytes_per_frame = 2;
			} else if (!strcasecmp(optarg, "s16_be")) {
				pcm_param.format = SND_PCM_FORMAT_S16_BE;
				bytes_per_frame = 2;
			} else if (!strcasecmp(optarg, "a-law")) {
				pcm_param.format = SND_PCM_FORMAT_A_LAW;
				bytes_per_frame = 1;
			} else if (!strcasecmp(optarg, "mu-law")) {
				pcm_param.format = SND_PCM_FORMAT_MU_LAW;
				bytes_per_frame = 1;
			}
			break;
		case 'p':
			pcm_param.period_size = atoi(optarg);
			break;
		case 's':
			pcm_param.read_frames = atoi(optarg);
			break;
		case 'r':
			pcm_param.sample_rate = atoi(optarg);
			break;
		case 'h':
			help();
			return EXIT_SUCCESS;
		default:
			help();
			return EXIT_FAILURE;
		}
	}

	openlog(NULL, 0, LOG_LOCAL7);

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		log_err("Cannot handle SIGINT! (%s)", strerror(errno));
		return EXIT_FAILURE;
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		log_err("Cannot handle SIGTERM! (%s)", strerror(errno));
		return EXIT_FAILURE;
	}

	// Initialize PCM for capturing audio stream by ALSA lib
	ret = initPcm(&pcm_param);
	if (ret == EXIT_FAILURE) {
		log_err("Initialize PCM failed.");
		goto error;
	}

	// Parse LSD parameters from config file
	parseConfig(config_fname, &lsd_param);

	// Check LSD parameters
	ret = AFTR_SD_checkParam(&lsd_param);
	if (ret != 0) {
		log_err("Invalid LSD parameters. err: %d", ret);
		goto error;
	}

	// Run LSD detection
	buffer_size = pcm_param.read_frames * bytes_per_frame * pcm_param.channels;
	buffer = (char *)malloc(buffer_size); // allocate a buffer large enough to hold one period

	ret = runLsdDetection(pcm_param.read_frames, buffer_size, &lsd_param);
	if (ret == EXIT_FAILURE) {
		log_err("Run LSD detection failed.");
		goto error;
	}

error:

	free(buffer);
	snd_pcm_drain(pcm_handle);
	snd_pcm_drop(pcm_handle);
	snd_pcm_close(pcm_handle);
	closelog();

	return EXIT_SUCCESS;
}
