/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include "tange_cloud_proc.h"
#include "tange_cloud_video.h"
#include "tange_cloud_audio.h"
#include "tange_cloud_comm.h"
#if defined(TANGE_USE_AV_MAIN2)
#include "tange_cloud_cmd_utils.h"
#elif defined(TANGE_USE_AGT_MPI_STREAM)
#include "agtx_lpw.h"
#endif

#include "logfile.h"

#include "TgCloudApi.h"
#include "ec_const.h"

#include <sample_publisher.h>
#include <sample_stream.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

/**
 * Definition
*/
#define REG_INFO_SAVED "/usrdata/root/reg_info.saved"
#define GPIO_16_VALUE_FILE "/sys/class/gpio/gpio16/value"
#define STREAM_FPS 15
#define RECORD_TIME 10

/**
 * Variables
*/

/**
 * Static Variables
*/
static char *_uuid_ = NULL;
static pthread_t thread_video_frame_data_id = 0;
static pthread_t thread_audio_frame_data_id = 0;
static pthread_t thread_doorbell_event_id = 0;

/**
 * Static Function Prototype
*/
static inline void TGC_exeSystemCmd(char *excmd);
static inline int TGC_forkIndependentProc(char *prog, char **arg_list);
static int createStreamoutThread(void);
static int waitUntilThreadTerminate(void);
static void upload_log(const char *log_file);
static void printCommands(void);
static int createDoorbellEventThread(void);

// #include "mpi_enc.h"
extern MPI_BCHN g_bchn;

#if defined(TANGE_USE_AGT_MPI_STREAM)
lpw_handle gWifihd = 0;
struct timeval gNoSessionTime = { .tv_sec = 0, .tv_usec = 0 };
#endif

int Distribution_Wifi(void)
{
	int mode = 0;
	void *saved_reg_info = NULL;
	int saved_reg_info_len = 0;
	int ret = -1;

	mode = get_mode();

	if (access("/usrdata/root/registered.ini", F_OK) == 0) {
		mode = 2; // Already registered.
		set_mode(mode);
	}

	LogI("mode = %d \n", mode);
	if (mode == 0 || mode == 1) // Not registered, start QR code or AP mode pairing.
	{
		// Stop lpw_supplicant.
		TGC_exeSystemCmd("killall /usr/sbin/lpw_supplicant");
		// Wifi Switch to AP mode.
		ret = startSoftAP();
		if (ret != 0) {
			LogE("Start AP mode fail\n");
			return -1;
		}
		// Start wifi AP mode pairing.
		// Set up the network configuration in the set_wifi() callback.
		TciConfigWifi(GWM_AP);
	}

	if (mode != 2) { // not registered
		TGC_resetSetting();
	}

	TciStart2(mode == 2, (1 << 20), _uuid_);

	return 0;
}

#if defined(TANGE_USE_AGT_MPI_STREAM)
int startSoftAP()
{
	printf("----------0-0-Start SoftAP Init-0-0--------------\n");

	char APssid[32];
	sprintf(APssid, "AICAM_%s", _uuid_);
	int result = -1;
	int retry_count = 0;

	lpw_wifi_ap_config_t ap_conf = {
		.channel_num = 10,
		.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
		.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
		.ssid = APssid,
		.key = "12345678",
	};

	gWifihd = openLpwHandler();
	if (gWifihd == 0) {
		LogE("Failed to open LPW device.\n");
		closeLpwHandler(gWifihd);
		return -1;
	}

	while (retry_count < MAX_WIFI_RETRY_TIMES) {
		result = lpw_wifi_set_ap_mode(gWifihd, &ap_conf);
		if (result != 0) {
			retry_count++;
		} else {
			break;
		}
		sleep(1);
	}

	if (result != 0) {
		LogE("Run lpw_wifi_set_ap_mode Fail!\n");
		return -1;
	}

	_ok("Start wifi AP mode success, SSID = \"%s\" \n", APssid);
	printf("---------------0-0-SoftAP Done-0-0---------------\n");

	return 0;
}
#endif

/**
 * Functions
*/
int TGC_initProc(char *uuid)
{
	int ret = 0;
	_uuid_ = uuid;

#if defined(TANGE_USE_AV_MAIN2)
	// Initial and Run the Agentix necessary applications.
	if (TGC_initCcClient() < 0) {
		printf("TGC_initCcClient fail!\r\n");
		return -1;
	}
	AGTX_COLOR_CONF_S data;
	TGC_getCcConfig(AGTX_CMD_COLOR_CONF, &data);
	TGC_setCcConfig(AGTX_CMD_COLOR_CONF, &data, NULL);
#endif

#if defined(TANGE_USE_AGT_MPI_STREAM)
	LogI("!!! TANGE_USE_AGT_MPI_STREAM \r\n");
#elif defined(TANGE_USE_AV_MAIN2)
	LogI("!!! TANGE_USE_AV_MAIN2 \r\n");
#else
	LogE("Tange_clund enable video application fail \r\n");
	return -1;
#endif

	// Initialize video & audio process.
	TGC_initVideoSystem();
	TGC_initAudio();

	// Initial communication process.
	TGC_initComm(uuid);

#if defined(TANGE_USE_AGT_MPI_STREAM)
	Distribution_Wifi();
#elif defined(TANGE_USE_AV_MAIN2)
	TGC_startComm();
#endif

	return 0;
}

int TGC_deinitProc(void)
{
	// De-initial communication process.
	LogI("Communication de-intialization.\n");
	TGC_deinitComm();

	// De-initialize video & audio process.
	LogI("Audio de-intialization.\n");
	TGC_deinitAudio();
	LogI("Video de-intialization.\n");
	TGC_deinitVideoSystem();

#if defined(TANGE_USE_AV_MAIN2)
	// Exit the Agentix necessary applications.
	LogI("Exit CcClient.\n");
	TGC_exitCcClient();
#endif

	return 0;
}

int TGC_runProc(void)
{
	int ret = 0;
	char line[128] = { 0 };

	// Create video & audio process threads.
	ret = createStreamoutThread();
	if (ret == -1) {
		LogE("Failed to create the video streaming thread.\n");
		return ret;
	}
	if (ret == -2) {
		LogE("Failed to create the audio streaming thread.\n");
		return ret;
	}

#if defined(TANGE_USE_AGT_MPI_STREAM)
	ret = createDoorbellEventThread();
	if (ret < 0) {
		LogE("createDoorbellEventThread fail exit...!!\n");
		return ret;
	}
#endif

	ret = waitUntilThreadTerminate();
	if (ret == -1) {
		LogE("Failed to join the video streaming thread.\n");
		return ret;
	}
	if (ret == -2) {
		LogE("Failed to join the audio streaming thread.\n");
		return ret;
	}

	return 0;
}

void TGC_stopProc(void)
{
	TGC_stopVideoRun();
	TGC_stopAudioRun();
}

void *thread_DoorbellEvent(void *arg)
{
	LogI(" thread_DoorbellEvent Start !!!\n");

	char value;
	int return_value = -1;
	int ret = -1;
	int idx = -1;

	return_value = system("echo 16 > /sys/class/gpio/export");
	if (return_value == 0) {
		// LogI(" command  1 successfully \n");
	} else {
		LogI(" command  1 fail\n");
	}
	return_value = -1;
	return_value = system("echo \"out\" > /sys/class/gpio/gpio16/direction");
	if (return_value == 0) {
		// LogI(" command  2 successfully \n");
	} else {
		LogI(" command  2 fail\n");
	}
	return_value = -1;
	return_value = system("echo 1 > /sys/class/gpio/gpio16/value");
	if (return_value == 0) {
		// LogI(" command  3 successfully \n");
	} else {
		LogI(" command  3 fail\n");
	}

	EVENTPARAM dbevent;
	MPI_ECHN echn_idx;
	idx = 0;
	MPI_ECHN encoder_channel;
	CONF_BITSTREAM_PARAM_S bitstream;
	bitstream.enable = 1;
	bitstream.stream.enable = 0;
	bitstream.record.enable = 1;
	bitstream.record.frame_num = STREAM_FPS * RECORD_TIME;
	strcpy(bitstream.record.fname, "/tmp/record_file");
	bitstream.record.max_dumped_files = 1;

	while (1) {
		usleep(100);

		int fd = open(GPIO_16_VALUE_FILE, O_RDONLY);
		if (fd < 0) {
			LogE("Failed to open GPIO value file");
			return;
		}

		if (read(fd, &value, 1) != 1) {
			LogE("Failed to read from GPIO value file\n");
		} else {
			// LogI(" read gpio 16 success value='%c'\n",value);
			if (value == '0') {
				// LogI(" sent DoorbellEvent reset gpio16 = %c\n",value);
				memset(&dbevent, 0, sizeof(dbevent));

				dbevent.cbSize = sizeof(EVENTPARAM);
				dbevent.event = ECEVENT_DOORBELL;
				dbevent.tHappen = SA_time(NULL);
				dbevent.status = 1;
				dbevent.jpg_pic = NULL;
				dbevent.pic_len = 0;
				dbevent.evtp_flags = 1;
				// dbevent.evt_data = NULL;

				LogI(" Thread sent doorbell event command!!\n");
				ret = TciSetEventEx(&dbevent);
				if (ret == 0) {
					// LogI(" TciSetEventEx success !!");
				} else {
					LogE(" TciSetEventEx ret=%d !!\n", ret);
				}

				echn_idx = MPI_ENC_CHN(idx);
				LogI("Start get stream from channel %d\n", idx);
				ret = SAMPLE_startStreamPublisher(echn_idx, &bitstream, 0, 0);
				if (ret != MPI_SUCCESS) {
					LogE(" SAMPLE_startStreamPublisher fail !! ret = %d \n", ret);
				} else {
					// LogI(" startStreamPublisher success\n");
				}

				// LogI(" sleep 10S \n");
				sleep(RECORD_TIME);

				SAMPLE_shutdownStreamPublisher(echn_idx);
				return_value = system("echo 1 > /sys/class/gpio/gpio16/value");
				if (return_value == 0) {
					// LogI(" command  4 successfully \n");
				} else {
					LogE(" command  4 fail\n");
				}
				LogI(" Doorbell Event Record finish\n");
			}
		}
		close(fd);
	}

	pthread_exit(0);
	return;
}

static int createDoorbellEventThread(void)
{
	if (pthread_create(&thread_doorbell_event_id, NULL, &thread_DoorbellEvent, NULL) != 0) {
		LogE("Failed to create the doorbell event thread.\n");
		return -1;
	}
	return 0;
}

/**
 * Static Functions
*/
static int createStreamoutThread(void)
{
	if (pthread_create(&thread_video_frame_data_id, NULL, &thread_VideoFrameData, NULL) != 0) {
		return -1;
	}
	if (pthread_create(&thread_audio_frame_data_id, NULL, &thread_AudioFrameData, NULL) != 0) {
		return -2;
	}
	return 0;
}

static int waitUntilThreadTerminate(void)
{
	if (pthread_join(thread_video_frame_data_id, NULL) != 0) {
		return -1;
	}
	if (pthread_join(thread_audio_frame_data_id, NULL) != 0) {
		return -2;
	}
	return 0;
}

static inline void TGC_exeSystemCmd(char *excmd)
{
	FILE *pf = NULL;
	char buff[64];
	pf = popen(excmd, "r");
	if (pf == NULL) {
		fprintf(stderr, "Failed to execute command:\n %s\n", excmd);
		return;
	}
	while (fgets(buff, 64, pf) != NULL) {
		printf("%s", buff);
	}
	pclose(pf);
}

static inline int TGC_forkIndependentProc(char *prog, char **arg_list)
{
	pid_t child;

	if ((child = fork()) < 0) {
		/* parent: check if fork failed */
		//PR_ERR("fork error");
	} else if (child == 0) {
		/* 1st level child: fork again */
		if ((child = fork()) < 0) {
			//PR_ERR("fork error");
		} else if (child > 0) {
			/* 1st level child: terminate itself to make init process the parent of 2nd level child */
			exit(0);
		} else {
			/* 2nd level child: execute program and will become child of init once 1st level child exits */
			execvp(prog, arg_list);
			//PR_ERR("execvp error");
			exit(0);
		}
	}

	/* parent: wait for 1st level child ends */
	waitpid(child, NULL, 0);

	return child;
}

static void upload_log(const char *log_file)
{
	char *url = NULL;
	char uuid[40];
	sprintf(uuid, "xxxx_%s", _uuid_);
	int ret = TgQueryUploadReq(uuid, "model-XXXXX", &url);
	if (ret == 0 && url) {
		ret = TgPostLogFile(url, uuid, log_file, 8000);
		if (ret != 0)
			_err("TgPostLogFile: post log %s failed with error: %d\n", log_file, ret);
		else
			_info("TgPostLogFile: post log of %s ok\n", uuid);

		static char log_mem[] = "This is the log in memory";
		ret = TgPostLogMem(url, uuid, log_mem, sizeof(log_mem) - 1, "mem.log", 8000);
		if (ret != 0)
			_err("TgPostLogMem: Post mem failed with error: %d\n", ret);
		else
			_info("TgPostLogMem: posst log of %s ok\n", uuid);
		free(url);
	} else {
		if (ret == 0)
			printf("Should enable to upload log in the console website with uuid: %s\n", uuid);
		else
			_err("TgQueryUploadReq: ret=%d\n", ret);
	}
}

static void printCommands(void)
{
	printf("------------------------ COMMANDS LIST  -----------------------\n");
	printf("?              -       this command list\n");
	printf("snd            -       sound event\n");
	printf("bd             -       body detected\n");
	printf("spdup          -       speed up\n");
	printf("spdwn          -       speed down\n");
	printf("ring           -       doorbell/call/...\n");
	printf("ring0          -       cancel doorbell/call/...\n");
	printf("stay           -       sb. stays at the door\n");
	printf("pass           -       sb. passes the door\n");
	printf("snapshot       -       snapshot\n");
	printf("templo TEMP    -       temperature low\n");
	printf("temphi TEMP    -       temperature high\n");
	printf("     TEMP              temperature. example: 32.5C, or 78.4F\n");
	printf("humhi HUMIDITY   -     humidity high\n");
	printf("humlo HUMIDITY   -     humidity low\n");
	printf("     HUMIDITY          humidity. 0~100\n");
	printf("cry            -       sb. cries\n");
	printf("enable|disable {upload|bs}\n");
	printf("     upload            enable or disable uploading for backstore\n");
	printf("     bs                enable or disable backstore\n");
	printf("md             -       motion detected\n");
	printf("ai-XXX         -       send picture ai-XXX.jpg to ai server\n");
	printf("ai             -       list supported ai-XXX\n");
	printf("gps|gps0       -       start/stop report GPS info\n");
	printf("bat Q,L,C      -       report battery status\n");
	printf("qoe Q,C        -       set local battery state\n");
	printf("       Q       -       quality of electricity(0~100)\n");
	printf("       L       -       0|1: power low ?\n");
	printf("       C       -       0|1: is charging?\n");
	printf("pm allon|netdown|sleepable   -  set power mode\n");
	printf("gosleep        -       force to shutdown\n");
	printf("sdc err|ok|{state}     -       set SD card state\n");
	printf("sdc {total} {free}     -       set SD card capacity and free space\n");
	printf("rec|nrec       -       set recording/non-recording flag\n");
	printf("net wifi|sim|nosim  -  set connection-way of a 4G device\n");
	printf("setoff|park    -       setoff/parking for drive-recorder\n");
	printf("q              -       quit\n");
	printf("--------------------- END OF COMMANDS LIST  ------------------\n");
}
