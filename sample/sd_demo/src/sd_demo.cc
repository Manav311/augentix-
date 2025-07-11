#include <cstdio>
#include <cstring>
#include <csignal>
#include <cstdarg>
#include <vector>
#include <pthread.h>
#include <unistd.h>
#include <ctime>

#include "mpi_index.h"
#include "mpi_iva.h"
#include "log.h"
#include "json.h"
#include "pcm_interfaces.h"
#include "alsa/error.h"
#include "alsa/asoundlib.h"
#include "aftr_sd.h"
#include "aftr_cd.h"

#ifndef max
#define max(x, y) ((x) > (y) ? (x) : (y))
#endif

static sig_atomic_t g_run_flag = 1;

static snd_pcm_t *pcm_handle;
static const char *pcm_handle_id = "default";
static char *buffer;
static int buffer_size;

#define SD_RUN_USLEEP 50000
#define WAV_HEADER_BYTES 44

#define SD_TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#define SD_TOC(str, start)                                                                               \
	do {                                                                                             \
		struct timespec end;                                                                     \
		uint64_t delta_us;                                                                       \
		float delta_s;                                                                           \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                \
		delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; \
		delta_s = (float)delta_us / 1000000;                                                     \
		printf("%s Elapsed time: %.8f (s).\n", str, delta_s);                                    \
	} while (0)

typedef struct {
	snd_pcm_stream_t stream_type;
	snd_pcm_format_t format;
	unsigned int channels;
	unsigned int sample_rate;
	unsigned int read_frames;
	snd_pcm_uframes_t period_size;
} Pcm_Param;

typedef struct {
	UINT16 notification_interval;
	UINT8 sample_stride;
	UINT8 num_of_vote;
	INT8 volume_gain;
	bool lsd_use;
	bool cd_use;
} SD_AppApi;

typedef struct {
	std::vector<char*> ring_buffer;
	pthread_mutex_t mutex;
} RunDetectionArgs;

struct timespec g_detect_time;
time_t g_time_start;
time_t g_time_now;
Pcm_Param g_pcm_param;
AFTR_SD_PARAM_S g_lsd_param;
AFTR_CD_PARAM_S g_cd_param;
SD_AppApi* g_sd_api = nullptr;
AFTR_CD_INSTANCE_S *g_cd_instance;
char g_audio_fname[256] = { 0 };

static void handleSigInt(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_run_flag = 0;
	}
}

void help()
{
	printf("Usage: sd_demo -i [options] ...\n"
		   "Options:\n"
		   "  -i <file>        SD config file\n"
		   "  -f <FORMAT>      FORMAT: S16_LE(default)/S16_BE/a-law/mu-law\n"
		   "  -c <arg>         channel count (default: 1)\n"
		   "  -r <arg>         sample rate (default: 8000)\n"
		   "  -p <arg>         period size (default: 1024)\n"
		   "  -s <arg>         frames read from alsa (default: 4096)\n"
		   "  -n <arg>         notification interval (default: 0)\n"
		   "  -h               help\n"
		   "\n"
		   "Example:\n"
		   "  $ sd_demo -i /system/mpp/sd_config/lsd_conf.json\n"
		   "  $ sd_demo -i /system/mpp/sd_config/cd_conf.json\n");
}

int parseConfig(const char *file_name, AFTR_SD_PARAM_S *lsd_param, AFTR_CD_PARAM_S *cd_param, Pcm_Param *pcm_param)
{
	static const char *agtx_iva_audio_cd_window_map[] = { "HAMMING", "HANNING" };

	json_object *root = NULL;
	json_object *child = NULL;
	int i;
	const char *str;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	/* LSD param */
	if (json_object_object_get_ex(root, "volume", &child)) {
		lsd_param->volume = json_object_get_int(child);
	} else {
		json_object_object_add(root, "volume", json_object_new_int(lsd_param->volume));
	}

	if (json_object_object_get_ex(root, "duration", &child)) {
		lsd_param->duration = json_object_get_int(child);
		g_sd_api->lsd_use = true;
	} else {
		json_object_object_add(root, "duration", json_object_new_int(lsd_param->duration));
	}

	if (json_object_object_get_ex(root, "suppression", &child)) {
		lsd_param->suppression = json_object_get_int(child);
		g_sd_api->lsd_use = true;
	} else {
		json_object_object_add(root, "suppression", json_object_new_int(lsd_param->suppression));
	}

	/* CD param*/
	cd_param->sample_rate = pcm_param->sample_rate;

	if (json_object_object_get_ex(root, "time", &child)) {
		g_sd_api->cd_use = true;
		cd_param->time = json_object_get_int(child);
	} else {
		json_object_object_add(root, "time", json_object_new_int(cd_param->time));
	}

	if (json_object_object_get_ex(root, "volume", &child)) {
		cd_param->volume = json_object_get_int(child);
	} else {
		json_object_object_add(root, "volume", json_object_new_int(cd_param->volume));
	}

	if (json_object_object_get_ex(root, "window_function", &child)) {
		str = json_object_get_string(child);
		for (i = 0; (UINT32)i < sizeof(agtx_iva_audio_cd_window_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_audio_cd_window_map[i], str) == 0) {
				g_sd_api->cd_use = true;
				cd_param->m_param.window_function = (AFTR_CD_WINDOW_FUNCTION_E)i;
				break;
			}
		}
	}

	if (json_object_object_get_ex(root, "window_size", &child)) {
		cd_param->m_param.window_size = json_object_get_int(child);
		g_sd_api->cd_use = true;
	} else {
		json_object_object_add(root, "window_size", json_object_new_int(cd_param->m_param.window_size));
	}

	if (json_object_object_get_ex(root, "stride", &child)) {
		cd_param->m_param.stride = json_object_get_int(child);
		g_sd_api->cd_use = true;
	} else {
		json_object_object_add(root, "stride", json_object_new_int(cd_param->m_param.stride));
	}

	if (json_object_object_get_ex(root, "sensitivity", &child)) {
		cd_param->sensitivity = json_object_get_int(child);
		g_sd_api->cd_use = true;
	} else {
		json_object_object_add(root, "sensitivity", json_object_new_int(cd_param->sensitivity));
	}

	if (json_object_object_get_ex(root, "notification_interval", &child)) {
		g_sd_api->notification_interval = json_object_get_int(child);
		g_sd_api->cd_use = true;
	} else {
		json_object_object_add(root, "notification_interval",
							   json_object_new_int(g_sd_api->notification_interval));
	}

	printf("%s\n", json_object_to_json_string(root));
	json_object_put(root);

	return EXIT_SUCCESS;
}

/**
 * @brief Volume up input audio data by 8-bit integer from +1 dB to +40 dB
 */
void gainVolume(char *raw_buffer, int read_frames)
{
	size_t size = read_frames;
	int16_t* int16_data = reinterpret_cast<int16_t*>(raw_buffer);
	const int volume_gain_bits = 8;
	const static int32_t numerator_list[41] = {
		  256,  287,   322,   362,   406,   455,   511,   573,   643,   722,
		  810,  908,  1019,  1144,  1283,  1440,  1615,  1812,  2033,  2282,
		 2560, 2872,  3223,  3616,  4057,  4552,  5108,  5731,  6430,  7215,
		 8095, 9083, 10192, 11435, 12830, 14396, 16153, 18123, 20335, 22816,
		25600
	};
	const int32_t numerator = numerator_list[g_sd_api->volume_gain];
	int32_t upper_bound = 32767;
	int32_t lower_bound = -32768;
	for (size_t i = 0; i < size; i++) {
		int32_t result = ((int32_t)int16_data[i] * numerator + (1 << (volume_gain_bits - 1))) >> volume_gain_bits;
		if (result >= upper_bound) {
			int16_data[i] = static_cast<int16_t>(upper_bound);
		} else if (result <= lower_bound) {
			int16_data[i] = static_cast<int16_t>(lower_bound);
		} else {
			int16_data[i] = static_cast<int16_t>(result);
		}
	}
	return;
}

void *runDetect(void *inputs)
{
	RunDetectionArgs* detection_args = (RunDetectionArgs*)inputs;
	std::vector<char*> &ring_buffer = detection_args->ring_buffer;
	std::vector<UINT8> result;
	AFTR_CD_STATUS_S *cd_status = &g_cd_instance->status;
	const size_t num_of_buffer = 5;
	const size_t num_of_vote = g_sd_api->num_of_vote;
	const size_t sample_stride = g_sd_api->sample_stride;
	char* cd_buffer = (char *)malloc(buffer_size);
	UINT16 notification_interval = g_sd_api->notification_interval;
	int ret;
	int first_notify = 0;
	int time_diff = notification_interval;

	while (1) {
		if (g_run_flag == 0) {
			log_info("Stop detecting cry.");
			break;
		}
		if (ring_buffer.size() < num_of_buffer) {
			usleep(SD_RUN_USLEEP);
			continue;
		} else {
			if (ring_buffer.size() > 10 && g_audio_fname[0] == '\0') {
				size_t clear_size = ring_buffer.size() - 5;
				log_warn("Too slow to handle input data, clear %ds old data, which might affects detection result.", clear_size);
				pthread_mutex_lock(&detection_args->mutex);
				for (size_t i = 0; i < clear_size; i++) {
					free(ring_buffer[0]);
					ring_buffer.erase(ring_buffer.begin());
				}
				pthread_mutex_unlock(&detection_args->mutex);
			}
			pthread_mutex_lock(&detection_args->mutex);
			// copy ring_buffer data to cd_buffer
			for (size_t i = 0; i < num_of_buffer; i++) {
				memcpy(cd_buffer + i * buffer_size / num_of_buffer, ring_buffer[i], buffer_size / num_of_buffer);
			}
			pthread_mutex_unlock(&detection_args->mutex);
			
			// pop out first element
			pthread_mutex_lock(&detection_args->mutex);
			for (size_t i = 0; i < sample_stride; i++) {
				free(ring_buffer[0]);
				ring_buffer.erase(ring_buffer.begin());
			}
			pthread_mutex_unlock(&detection_args->mutex);
		}
		// Detect baby cry sound
		// SD_TIC(g_detect_time);
		ret = AFTR_CD_detect(g_cd_instance, cd_buffer, buffer_size, cd_status);

		if (ret != 0) {
			log_err("Cry detect failed. err: %d", ret);
			break;
		}
		// SD_TOC("[CD detect once]", g_detect_time);

		result.push_back(cd_status->alarm);
		if (result.size() > num_of_vote){
			result.erase(result.begin());
		}

		g_time_now = std::time(0);
		if (first_notify) {
			time_diff = difftime(g_time_now, g_time_start);
		}

		// Show CD alarm
		if (time_diff >= notification_interval && result.size() == num_of_vote) {
			UINT8 sum = 0;
			for (size_t i = 0; i < result.size(); i++) {
				sum += result[i];
			}
			// printf("sum: %d\n", sum);
			if (sum >= (num_of_vote + 1) / 2) {
				log_info("Cry detected!");
				first_notify = 1;
				g_time_start = std::time(0);
			}
		}
	}

	// free buffer
	free(cd_buffer);

	pthread_mutex_lock(&detection_args->mutex);
	for (char* ptr : ring_buffer) {
		free(ptr);
	}
	ring_buffer.clear();
	pthread_mutex_unlock(&detection_args->mutex);

	pthread_exit(NULL);
}

int runLsdDetection(const int read_frames, const AFTR_SD_PARAM_S *lsd_param)
{
	int ret;

	AFTR_SD_INSTANCE_S *sd_instance;
	AFTR_SD_STATUS_S sd_status;

	// Create SD instance
	sd_instance = AFTR_SD_newInstance();
	if (!sd_instance) {
		log_err("Failed to create SD instance.");
		return EXIT_FAILURE;
	}

	// Set SD parameters
	ret = AFTR_SD_setParam(sd_instance, lsd_param);
	if (ret != 0) {
		log_err("Set SD parameters failed. err: %d", ret);
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

		// Detect IVA sound application
		ret = AFTR_SD_detect(sd_instance, buffer, buffer_size, &sd_status);
		if (ret != 0) {
			log_err("Sound detect failed. err: %d", ret);
			break;
		}

		// Show SD alarm
		if (sd_status.alarm) {
			log_info("loud sound detected!");
		}
	}

	// Delete SD instance
	ret = AFTR_SD_deleteInstance(&sd_instance);
	if (ret != 0) {
		log_err("Delete SD instance failed. err: %d", ret);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int runCdDetection(AFTR_CD_PARAM_S *cd_param)
{
	RunDetectionArgs detection_args;
	pthread_t cd_thread;
	int read_frames = 8192;
	int ret;
	FILE *file = NULL;
	size_t frames_read = 0;

	// Create CD instance
	g_cd_instance = AFTR_CD_newInstance();
	if (!g_cd_instance) {
		log_err("Failed to create CD instance.");
		return EXIT_FAILURE;
	}

	// Set CD parameters
	ret = AFTR_CD_setParam(g_cd_instance, cd_param);
	if (ret != 0) {
		log_err("Set CD parameters failed. err: %d", ret);
		return EXIT_FAILURE;
	}

	if (g_audio_fname[0] != '\0') {
		file = fopen(g_audio_fname, "rb");
		if (file == NULL) {
			log_err("Failed to open file");
			return EXIT_FAILURE;
		}
		char *ext = strrchr(g_audio_fname, '.');
		if (ext != NULL && strcmp(ext, ".wav") == 0) {
			if (fseek(file, WAV_HEADER_BYTES, SEEK_SET) != 0) {
				log_err("Failed to ignore header in %s file", ext);
				fclose(file);
				return EXIT_FAILURE;
			}
		}
	}

	pthread_mutex_init(&detection_args.mutex, NULL);
	g_time_start = std::time(0);

	// Create cd thread to calculate data
	if ((ret = pthread_create(&cd_thread, NULL, runDetect, (void *) &detection_args)) != 0) {
		log_err("Cannot create CD thread. err: %d", ret);
		return EXIT_FAILURE;
	}

	while (g_run_flag) {
		char* buffer = (char *)malloc(buffer_size / 5);
		if (buffer == NULL) {
			log_err("Failed to init ring_buffer");
			break;
		}

		if (file != NULL) {
			// Read from files
			if ((frames_read = fread(buffer, 2, 8192, file)) <= 0) {
				fclose(file);
				break;
			}
		} else {
			// Read interleaved frames from PCM
			ret = snd_pcm_readi(pcm_handle, buffer, (snd_pcm_uframes_t)read_frames);
			if (ret == -EPIPE) {
				log_warn("overrun occurred: %s", snd_strerror(ret));
				snd_pcm_prepare(pcm_handle);
				continue;
			} else if (ret < 0) {
				log_err("error from read: %s", snd_strerror(ret));
				break;
			} else if (ret != read_frames) {
				log_warn("short read (expected %d, read %d)", read_frames, ret);
				snd_pcm_prepare(pcm_handle);
				continue;
			}
		}
		if (g_sd_api->volume_gain != 0) {
			gainVolume(buffer, read_frames);
		}
		pthread_mutex_lock(&detection_args.mutex);
		detection_args.ring_buffer.push_back(buffer);
		pthread_mutex_unlock(&detection_args.mutex);
	}
	pthread_join(cd_thread, NULL);
	pthread_mutex_destroy(&detection_args.mutex);

	// Delete CD instance
	ret = AFTR_CD_deleteInstance(&g_cd_instance);
	if (ret != 0) {
		log_err("Delete CD instance failed. err: %d", ret);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int initPcm(Pcm_Param *pcm_param)
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

static int getGainInDb(const char *selem_name, long *db)
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
		err = snd_mixer_selem_get_capture_dB(elem, SND_MIXER_SCHN_UNKNOWN, db);
		if (err < 0) {
			fprintf(stderr, "snd_mixer_selem_get_capture_dB: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

int main(int argc, char **argv)
{
	int c;
	int ret;
	int bytes_per_frame = 2;
	char config_fname[256] = { 0 };

	Pcm_Param *pcm_param = &g_pcm_param;

	pcm_param->stream_type = SND_PCM_STREAM_CAPTURE; // capture audio stream
	pcm_param->channels = 1; // one channel
	pcm_param->format = SND_PCM_FORMAT_S16_LE; // Signed 16-bit little-endian format
	pcm_param->sample_rate = 8000; // 8000 bits per second
	pcm_param->read_frames = 4096; // buffer size
	pcm_param->period_size = 1024; // period size

	AFTR_SD_PARAM_S *lsd_param = &g_lsd_param;
	AFTR_CD_PARAM_S *cd_param = &g_cd_param;
	AFTR_CD_MODEL_PARAM_S *cd_m_param = &cd_param->m_param;

	lsd_param->volume = 65;
	lsd_param->duration = 1;
	lsd_param->suppression = 2;

	cd_param->sample_rate = 8000;
	cd_param->time = 5120;
	cd_param->volume = 60;
	cd_param->sensitivity = 127;
	cd_m_param->window_function = AFTR_CD_MODEL_WINDOW_HANN;
	cd_m_param->window_size = 256;
	cd_m_param->stride = 64;

	g_sd_api = new SD_AppApi();

	g_sd_api->notification_interval = 0;
	g_sd_api->sample_stride = 1;
	g_sd_api->num_of_vote = 5;
	g_sd_api->volume_gain = 0;
	g_sd_api->lsd_use = false;
	g_sd_api->cd_use = false;

	if (argc < 2) {
		help();
		return EXIT_SUCCESS;
	}

	while ((c = getopt(argc, argv, "i:c:f:p:s:r:n:h:t:v:g:w:")) != -1) {
		switch (c) {
		case 'i':
			sprintf(config_fname, optarg);
			log_info("Load config file: %s", config_fname);
			break;
		case 'c':
			pcm_param->channels = atoi(optarg);
			break;
		case 'f':
			if (!strcasecmp(optarg, "S16_LE")) {
				pcm_param->format = SND_PCM_FORMAT_S16_LE;
				bytes_per_frame = 2;
			} else if (!strcasecmp(optarg, "s16_be")) {
				pcm_param->format = SND_PCM_FORMAT_S16_BE;
				bytes_per_frame = 2;
			} else if (!strcasecmp(optarg, "a-law")) {
				pcm_param->format = SND_PCM_FORMAT_A_LAW;
				bytes_per_frame = 1;
			} else if (!strcasecmp(optarg, "mu-law")) {
				pcm_param->format = SND_PCM_FORMAT_MU_LAW;
				bytes_per_frame = 1;
			}
			break;
		case 'p':
			pcm_param->period_size = atoi(optarg);
			break;
		case 's':
			pcm_param->read_frames = atoi(optarg);
			break;
		case 'r':
			pcm_param->sample_rate = atoi(optarg);
			break;
		case 'n':
			g_sd_api->notification_interval = atoi(optarg);
			break;
		case 't':
			g_sd_api->sample_stride = atoi(optarg);
			break;
		case 'v':
			g_sd_api->num_of_vote = atoi(optarg);
			break;
		case 'g':
			g_sd_api->volume_gain = atoi(optarg);
			break;
		case 'w':
			sprintf(g_audio_fname, optarg);
			log_info("Load audio file: %s", g_audio_fname);
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
	ret = initPcm(pcm_param);
	if (ret == EXIT_FAILURE) {
		log_err("Initialize PCM failed.");
		goto error;
	}

	long gain_in_db;
	ret = getGainInDb("Gain(In)", &gain_in_db);
	if (ret != 0) {
		log_err("Unknown Gain(In) parameter. err: %d", ret);
		goto error;
	}
	gain_in_db = gain_in_db / 100;
	printf("Gain(In): %ld db\n", gain_in_db);

	// Parse SD parameters from config file
	parseConfig(config_fname, lsd_param, cd_param, pcm_param);

	// Check parameters
	if (g_sd_api->lsd_use) {
		ret = AFTR_SD_checkParam(lsd_param);
		if (ret != 0) {
			log_err("Invalid SD parameters. err: %d", ret);
			goto error;
		}
	} else if (g_sd_api->cd_use) {
		ret = AFTR_CD_checkParam(cd_param);
		if (ret != 0) {
			log_err("Invalid CD parameters. err: %d\n", ret);
			goto error;
		}
		if (pcm_param->read_frames != cd_param->time / 1024 * 8192) {
			log_err("Pcm Param read frames %u is not %u.", pcm_param->read_frames,
					cd_param->time / 1024 * 8192);
			goto error;
		}
	}

	// Init buffer size
	buffer_size = pcm_param->read_frames * bytes_per_frame * pcm_param->channels;
	printf("buffer_size: %d\n", buffer_size);
	// Adjust cd_param->volume threshold based on gain_in_db and volume_gain
	cd_param->volume += gain_in_db + g_sd_api->volume_gain;

	// Run detection
	if (g_sd_api->lsd_use) {
		ret = runLsdDetection(pcm_param->read_frames, lsd_param);
		if (ret == EXIT_FAILURE) {
			log_err("Run LSD detection failed.");
			goto error;
		}
	} else if (g_sd_api->cd_use) {
		ret = runCdDetection(cd_param);
		if (ret == EXIT_FAILURE) {
			log_err("Run CD detection failed.");
			goto error;
		}
	}

error:

	free(buffer);
	delete g_sd_api;
	g_sd_api = nullptr;
	snd_pcm_drain(pcm_handle);
	snd_pcm_drop(pcm_handle);
	snd_pcm_close(pcm_handle);
	closelog();

	return EXIT_SUCCESS;
}
