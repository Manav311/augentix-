#include "inc/log_define.h"
#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // for write
#include <pthread.h> // for threading, link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>
#include <sys/file.h>
#include <pthread.h>

#include "mpi_base_types.h"

#include "agtx_iva.h"
#include "agtx_video.h"
#include "agtx_video_layout_conf.h"

#include "agtx_audio.h"
#include "agtx_cmd.h"
#include "agtx_osd.h"
#include "agtx_color_conf.h"

#include "log_define.h"
#include "core.h"
#include "nodes.h"

int g_run_flag = 0;
bool g_no_iva_flag = false;
bool g_no_image_preference_flag = false;
bool g_no_video_control_flag = false;

#define AVMAIN2_RDY_FILE "/tmp/av_main2_ready"

static int createReadyFile(void)
{
	avmain2_log_debug("create ready file !");
	int ret = 0;
	ret = open(AVMAIN2_RDY_FILE, O_CREAT | O_EXCL);
	if (ret == -EEXIST) {
		avmain2_log_err("av_main2 already exist");
		return ret;
	}
	return 0;
}

static int destroyReadyFile(void)
{
	avmain2_log_debug("destroy ready file !");
	int ret = 0;
	ret = remove(AVMAIN2_RDY_FILE);

	return ret;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}

	g_run_flag = 0;
}

void help(void)
{
	printf("[Avmain2 usage:]\n");
	printf("-h --help\n");
	printf("-v --no_video\n");
	printf("-f --no_iva\n");
	printf("-m --no_image_pref\n");
	return;
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	int c;
	int ret = 0;
	const char *optstring = "hfmv";
	struct option opts[] = { { "help", 0, NULL, 'h' },
		                 { "no_iva", 0, NULL, 'f' },
		                 { "no_image_pref", 0, NULL, 'm' },
		                 { "no_video", 0, NULL, 'v' } };
	while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'f':
			g_no_iva_flag = true;
			break;
		case 'm':
			g_no_image_preference_flag = true;
			break;
		case 'v':
			g_no_video_control_flag = true;
			break;
		default:
			help();
			exit(1);
		}
	}

	avmain2_log_info("Flag [no iva, no img pref, no video]:(%d, %d, %d)", g_no_iva_flag, g_no_image_preference_flag,
	                 g_no_video_control_flag);

	if (g_no_iva_flag && g_no_image_preference_flag && g_no_video_control_flag) {
		help();
		exit(1);
	}

	g_run_flag = 1;

	if (0 != CORE_init()) {
		avmain2_log_err("failed to init core");
		return -EINVAL;
	}

	ret = createReadyFile();
	if (ret == -EEXIST) {
		avmain2_log_err("av_main2 already exist");
		exit(1);
	} else if (ret != 0) {
		avmain2_log_err("failed to create av_main2 ready file, ret:%d", ret);
		exit(1);
	}

	while (g_run_flag) {
		sleep(2);
	}

	ret = destroyReadyFile();
	if (ret != 0) {
		avmain2_log_err("failed to destroy av_main2 ready file, ret:%d", ret);
	}

	CORE_exit();

	return 0;
}