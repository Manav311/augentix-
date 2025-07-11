#include <stdlib.h>
#include <stdbool.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <limits.h>
#include <getopt.h>

#include "utlist.h"
#include "utstring.h"
#include "mpi_sys.h"
#include "mpi_enc.h"
#include "av_lib.h"
#include "mp4_log_define.h"

static volatile bool g_shutdown_requested = false;

static void sigintHandler(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else {
		perror("Unexpected signal!\n");
		exit(1);
	}

	g_shutdown_requested = true;
}

static bool shouldShutdown(void *context)
{
	(void)context;
	return g_shutdown_requested;
}

static bool shouldStopExport(void *context, unsigned int frame_number, float timestamp)
{
	(void)context;
	(void)frame_number;
	(void)timestamp;
	// mp4_log_info("Frame#%u at %.3f seconds.\n", frame_number, timestamp);
	return g_shutdown_requested;
}

typedef struct encoder_item {
	MPI_ECHN encoder;
	UT_string *output_path;
	struct encoder_item *next;
} EncoderItem;

typedef struct app_config {
	int audio_gain;
	unsigned int audio_sample_rate;
	bool variable_rate;
	float audio_tape_seconds;
	float preload_seconds;
	float audio_delay_seconds;
	EncoderItem *encoder_list;
} AppConfig;

typedef struct av_recorder_item {
	AvAvRecorder *recorder;
	UT_string *output_path;
	float start_point;
	AppConfig *config;
	pthread_t task;
	struct av_recorder_item *next;
} AvRecorderItem;

typedef struct {
	AvAudioTape *tape;
	AppConfig *config;
} RunTapeParams;

static void *runTapeRecording(void *arg)
{
	RunTapeParams *params = arg;
	AV_AudioTape_runRecording(params->tape, params->config->audio_tape_seconds, shouldShutdown, NULL);

	return NULL;
}

static void *exportFile(void *arg)
{
	AvRecorderItem *item = arg;
	mp4_log_info("Start export A/V to file: %s\n", utstring_body(item->output_path));
	AV_AvRecorder_exportToFile(item->recorder, utstring_body(item->output_path), item->start_point,
	                           item->config->variable_rate, shouldStopExport, NULL);

	return NULL;
}

static float current_time()
{
	struct timespec cur_time;
	clock_gettime(CLOCK_MONOTONIC, &cur_time);
	return cur_time.tv_nsec / 1000000000.0 + cur_time.tv_sec;
}

static void printUsage(char *program_name, AppConfig *config)
{
	printf("Usage: %s [OPTION]... [enc_index output_path]...\n", program_name);
	printf("Example: %s 0 /mnt/sdcard/enc0.mp4\n", program_name);
	printf("         %s 0 /mnt/sdcard/enc0.mp4 1 /mnt/nfs/ethnfs/enc1.mp4\n", program_name);
	printf("\n");
	printf("Options:\n");
	printf("  -h Display the usage and exit.\n");
	printf("  -g audio_gain           Specify audio gain. (default: %d)\n", config->audio_gain);
	printf("  -a audio_sample_rate    Specify PCM sample rate. (default: %d)\n", config->audio_sample_rate);
	printf("   (should be one of 96000, 88200, 64000, 48000, 44100, 32000, 24000, 22050, 16000, 12000, 11025, 8000)\n");
	printf("  -t audio_tape_length    Audio tape recording length in seconds. (default: %.3f)\n",
	       config->audio_tape_seconds);
	printf("  -p preload_seconds      Export mp4 with preload video/audio data. (default: %.3f)\n",
	       config->preload_seconds);
	printf("  -d audio_delay_seconds  Compensate ALSA audio delay in seconds. (default: %.3f)\n",
	       config->audio_delay_seconds);
	printf("                          (If you don't understand what this is, don't touch it)");
	printf("  -C                      Video sample use fixed frame rate (dts). (default: variable frame rate)\n");
}

static int parseCommandArgs(int argc, char *argv[], AppConfig *config)
{
	int c;
	char *end;
	while ((c = getopt(argc, argv, "+g:t:p:a:d:Ch")) != -1) {
		errno = 0;
		switch (c) {
		case 'h':
			printUsage(argv[0], config);
			return -1;
		case 'g':
			config->audio_gain = strtol(optarg, &end, 10);
			if (errno || end == optarg) {
				mp4_log_err("Invalid value for -g: %s %s\n", optarg, errno ? strerror(errno) : "");
				return 1;
			}
			break;
		case 't':
			config->audio_tape_seconds = strtof(optarg, &end);
			if (errno || end == optarg) {
				mp4_log_err("Invalid value for -t: %s %s\n", optarg, errno ? strerror(errno) : "");
				return 1;
			}
			break;
		case 'p':
			config->preload_seconds = strtof(optarg, &end);
			if (errno || end == optarg) {
				mp4_log_err("Invalid value for -p: %s %s\n", optarg, errno ? strerror(errno) : "");
				return 1;
			}
			break;
		case 'd':
			config->audio_delay_seconds = strtof(optarg, &end);
			if (errno || end == optarg) {
				mp4_log_err("Invalid value for -d: %s %s\n", optarg, errno ? strerror(errno) : "");
				return 1;
			}
			break;
		case 'a':
			config->audio_sample_rate = strtol(optarg, &end, 10);
			if (errno || end == optarg) {
				mp4_log_err("Invalid value for -a: %s %s\n", optarg, errno ? strerror(errno) : "");
				return 1;
			}
			break;
		case 'C':
			config->variable_rate = false;
			break;
		case ':':
		case '?':
			return 1;
		}
	}

	if ((argc - optind) % 2) {
		mp4_log_err("numbers of argument MUST be even.\n");
		return 2;
	}

	for (int i = optind; i < argc; i += 2) {
		char *enc_arg = argv[i];
		errno = 0;
		int encoder_idx = strtol(enc_arg, &end, 10);
		if (errno || end == enc_arg) {
			mp4_log_err("%s is NOT a valid encoder index.\n", enc_arg);
			return 3;
		}

		EncoderItem *item;
		LL_FOREACH(config->encoder_list, item)
		{
			if (item->encoder.chn == encoder_idx) {
				mp4_log_err("encoder index %d is specify more than once!\n", encoder_idx);
				return 4;
			}
		}

		char *path_arg = argv[i + 1];
		item = malloc(sizeof(*item));
		item->encoder = MPI_ENC_CHN(encoder_idx);
		utstring_new(item->output_path);
		utstring_printf(item->output_path, "%s", path_arg);
		LL_APPEND(config->encoder_list, item);
	}

	return 0;
}

static void dumpConfig(AppConfig *config)
{
	EncoderItem *item;
	int size;
	printf("AppConfig {\n");
	printf("  audio_gain=%d,\n", config->audio_gain);
	printf("  audio_sample_rate=%u,\n", config->audio_sample_rate);
	printf("  variable_rate=%d,\n", config->variable_rate);
	printf("  audio_tape_seconds=%.3f,\n", config->audio_tape_seconds);
	printf("  preload_seconds=%.3f,\n", config->preload_seconds);
	printf("  audio_delay_seconds=%.3f,\n", config->audio_delay_seconds);
	LL_COUNT(config->encoder_list, item, size);
	printf("  encoder_size=%d\n", size);
	printf("}\n");
}

int main(int argc, char *argv[])
{
	int ret;
	EncoderItem *item, *tmp;
	AppConfig config = { .audio_gain = 25,
		             .audio_sample_rate = 48000,
		             .audio_tape_seconds = 10,
		             .preload_seconds = 0,
		             .audio_delay_seconds = 0,
		             .variable_rate = true,
		             .encoder_list = NULL };
	if (parseCommandArgs(argc, argv, &config)) {
		return -1;
	}

	dumpConfig(&config);

	int encoder_count;
	LL_COUNT(config.encoder_list, item, encoder_count);
	if (encoder_count == 0) {
		mp4_log_info("Specify no encoder, exit.\n");
		return 0;
	}

	/** Set signal handler. */
	if (signal(SIGINT, sigintHandler) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(2);
	}

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		mp4_log_err("Initialize MPP system FAILED, err=%d\n", ret);
		return 1;
	}

	ret = MPI_initBitStreamSystem();
	if (ret != MPI_SUCCESS) {
		mp4_log_err("Initialize BitStream system FAILED, err=%d\n", ret);
		goto EXIT_MPP;
	}

	AvAudioTape *tape = AV_createAudioTape(config.audio_sample_rate, config.audio_gain, config.audio_delay_seconds);
	if (!AV_AudioTape_isReady(tape)) {
		mp4_log_err("PCM sound is NOT ready!\n");
		goto EXIT_BITSTREAM_SYSTEM;
	}

	pthread_t tape_task;
	RunTapeParams tape_task_params = { .tape = tape, .config = &config };
	ret = pthread_create(&tape_task, NULL, runTapeRecording, &tape_task_params);
	if (ret) {
		mp4_log_err("Create audio tape recording thread FAILED, err=%d\n", ret);
		goto DESTRUCT_AUDIO_TAPE;
	}

	while (!AV_AudioTape_isRecording(tape)) {
		usleep(1000);
	}

	AvRecorderItem *recorder_list = NULL;
	LL_FOREACH(config.encoder_list, item)
	{
		AvRecorderItem *recorder_item = malloc(sizeof(*recorder_item));
		recorder_item->recorder = AV_createAvRecorder(item->encoder);
		mp4_log_info("Activating AV recorder for encoder %u...\n", item->encoder.chn);
		if (!AV_AvRecorder_activateWithAudio(recorder_item->recorder, tape)) {
			AV_AvRecorder_dispose(recorder_item->recorder);
			free(recorder_item);
			mp4_log_warn("Activate AV recorder for encoder %u FAILED!\n", item->encoder.chn);
			continue;
		}

		utstring_new(recorder_item->output_path);
		utstring_printf(recorder_item->output_path, "%s", utstring_body(item->output_path));
		if (utstring_find(item->output_path, -4, ".mp4", 4) == -1) {
			utstring_printf(recorder_item->output_path, ".mp4");
		}
		recorder_item->config = &config;
		LL_APPEND(recorder_list, recorder_item);
	}

	LL_FOREACH_SAFE(config.encoder_list, item, tmp)
	{
		utstring_free(item->output_path);
		LL_DELETE(config.encoder_list, item);
		free(item);
	}

	printf("Press [ENTER] to start export mp4 files...\n");
	char line[LINE_MAX];
	fgets(line, sizeof(line), stdin);
	const float start_point = current_time() - config.preload_seconds;
	AvRecorderItem *recorder_item, *recorder_tmp;
	LL_FOREACH(recorder_list, recorder_item)
	{
		recorder_item->start_point = start_point;
		ret = pthread_create(&recorder_item->task, NULL, exportFile, recorder_item);
		if (ret) {
			mp4_log_notice("Create exporting thread for %s FAILED, err=%d\n",
			               utstring_body(recorder_item->output_path), ret);
			recorder_item->task = pthread_self();
		}
	}

	LL_FOREACH_SAFE(recorder_list, recorder_item, recorder_tmp)
	{
		if (!pthread_equal(recorder_item->task, pthread_self())) {
			pthread_join(recorder_item->task, NULL);
		}
		LL_DELETE(recorder_list, recorder_item);
		utstring_free(recorder_item->output_path);
		AV_AvRecorder_deactivate(recorder_item->recorder);
		AV_AvRecorder_dispose(recorder_item->recorder);
	}

	/* Since all AV recorders are stopped */
	g_shutdown_requested = true;
	pthread_join(tape_task, NULL);

DESTRUCT_AUDIO_TAPE:
	AV_AudioTape_dispose(tape);

EXIT_BITSTREAM_SYSTEM:
	MPI_exitBitStreamSystem();

EXIT_MPP:
	MPI_SYS_exit();
	return 0;
}
