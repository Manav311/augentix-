/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include "mpi_base_types.h"

#include "tuya_service.h"
#include "tuya_ipc_define.h"
#include "tuya_cloud_base_defs.h"
#include "tuya_cloud_wifi_defs.h"
#include "tuya_ipc_dp_utils.h"
#include "tuya_ipc_dp_handler.h"
#include "tuya_ipc_cloud_storage.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_system_control_demo.h"
#include "tuya_ipc_media_demo.h"
#include "tuya_ipc_motion_detect_demo.h"
#include "tuya_utils.h"

#include "mpi_sys.h"
#include "mpi_enc.h"
#include "avftr_conn.h"
#include "avftr.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdio.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <syslog.h>
#include "ledevt.h"

/*augentix connection mode : for m=0, three possible modes
 *    qr/sniff (default)
 *    qr code only,
 *    sniff mode only
 */
unsigned int AGTX_TUYA_PAIR_QR_MODE = 0;
unsigned int AGTX_TUYA_PAIR_SNIFF_MODE = 0;
unsigned int enable_echoShow = 0;
unsigned int enable_chromecast = 0;

unsigned int eventDetectCntr = 0;

CHAR_T g_ipc_app_pid[TUYA_ID_PID_SIZE + 1] = { 0 };
CHAR_T g_ipc_app_uuid[TUYA_ID_UUID_SIZE + 1] = { 0 };
CHAR_T g_ipc_app_authkey[TUYA_ID_AUTHKEY_SIZE + 1] = { 0 };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
IPC_MGR_INFO_S s_mgr_info = { { 0 } };
#pragma GCC diagnostic pop
STATIC INT_T s_mqtt_status = 0;
STATIC INT_T s_timesync_status = 0;
int g_verbose = 1;
STREAM_TRHEAD_STATUS_E g_tread_live_status[E_CHANNEL_VIDEO_MAX] = {[0 ...(E_CHANNEL_VIDEO_MAX - 1)] = STREAM_STOP };

// Vftr Client
//int vftrUnxSktClientFD; //Unix Socket FD
//int vftrResShmClientFD; //Shared memory for result FD
//VIDEO_FTR_IVA_CTX_S *vftr_res_shm_client;
int avftrUnxSktClientFD; //Unix Socket FD
int avftrResShmClientFD; //Shared memory for result FD
AVFTR_CTX_S *avftr_res_shm_client;

/**
 * @brief Callback for on network status change
 * @param[in] stat	TBD
 * @return None
 */
static void tuyaNetStatChangeCb(BYTE_T stat)
{
	PR_DEBUG("Net status: %d\n", stat);
	switch (stat) {
	case STAT_MQTT_ONLINE:
		s_mqtt_status = 1;
		IPC_APP_Notify_LED_Sound_Status_CB(IPC_MQTT_ONLINE);
		setLEDInform("Disconnected", 0);
		break;
	case STAT_MQTT_OFFLINE:
		setLEDInform("Disconnected", 1);
		break;
	case STAT_REG_FAIL:
		setLEDInform("Critical_Error", 1);
		break;
	case STAT_OFFLINE:
		break;
	default:
		break;
	}
}

/**
 * @brief Initialize TUYA IPC SDK
 * @param[in] init_mode	Mode to be initialized
 * @param[in] token		Token acquired from app
 * @return OPERATE_RET
 */
OPERATE_RET initTuyaSdk(WIFI_INIT_MODE_E init_mode, const char *token)
{
	char *sw_ver = calloc(sizeof(char), 20);
	PR_DEBUG("SDK Version: %s\n", tuya_ipc_get_sdk_info());
	memset(&s_mgr_info, 0, sizeof(IPC_MGR_INFO_S));
	strcpy(s_mgr_info.storage_path, IPC_APP_STORAGE_PATH);
	strcpy(s_mgr_info.upgrade_file_path, IPC_APP_UPGRADE_FILE);
	strcpy(s_mgr_info.sd_base_path, IPC_APP_SD_BASE_PATH);
	strcpy(s_mgr_info.product_key, g_ipc_app_pid);
	strcpy(s_mgr_info.uuid, g_ipc_app_uuid);
	strcpy(s_mgr_info.auth_key, g_ipc_app_authkey);
	get_sw_version(&sw_ver);

	if (sw_ver != 0) {
		strcpy(s_mgr_info.dev_sw_version, sw_ver);
	} else {
		strcpy(s_mgr_info.dev_sw_version, IPC_APP_VERSION);
	}
	free(sw_ver);
	sw_ver = NULL;
	s_mgr_info.max_p2p_user = 5; //TUYA P2P supports 5 users at the same time, including live preview and playback

	PR_DEBUG("Init Value.product_key %s", s_mgr_info.product_key);
	PR_DEBUG("Init Value.uuid %s", s_mgr_info.uuid);
	PR_DEBUG("Init Value.auth_key %s", s_mgr_info.auth_key);
	PR_DEBUG("Init Value.p2p_id %s", s_mgr_info.p2p_id);
	PR_DEBUG("Init Value.dev_sw_version %s", s_mgr_info.dev_sw_version);
	PR_DEBUG("Init Value.max_p2p_user %u", s_mgr_info.max_p2p_user);

	IPC_APP_Set_Media_Info();
	TUYA_APP_Init_Ring_Buffer();

	IPC_APP_Notify_LED_Sound_Status_CB(IPC_BOOTUP_FINISH);

	TUYA_IPC_ENV_VAR_S env;

	memset(&env, 0, sizeof(TUYA_IPC_ENV_VAR_S));

	strcpy(env.storage_path, s_mgr_info.storage_path);
	strcpy(env.product_key, s_mgr_info.product_key);
	strcpy(env.uuid, s_mgr_info.uuid);
	strcpy(env.auth_key, s_mgr_info.auth_key);
	strcpy(env.dev_sw_version, s_mgr_info.dev_sw_version);
	strcpy(env.dev_serial_num, "tuya_ipc");
	env.dev_obj_dp_cb = IPC_APP_handle_dp_cmd_objs;
	env.dev_dp_query_cb = IPC_APP_handle_dp_query_objs;
	env.status_changed_cb = tuyaNetStatChangeCb;
	env.gw_ug_cb = IPC_APP_Upgrade_Inform_cb;
	env.gw_rst_cb = IPC_APP_Reset_System_CB;
	env.gw_restart_cb = IPC_APP_Restart_Process_CB;
	env.mem_save_mode = FALSE;
	tuya_ipc_init_sdk(&env);
	tuya_ipc_start_sdk(init_mode, token);
	tuya_ipc_set_log_attr(g_verbose, NULL);
	return OPRT_OK;
}

/**
 * @brief Load TUYA IDs
 *
 * The priorities of loading is:
 * 1. ApplyId file in TUYA_ID_SEARCH_PATH_1 (usually from SD card)
 * 2. ApplyId file in TUYA_ID_SEARCH_PATH_2 (usually /root)
 * 3. Flash memory
 *
 * @param None
 * @return
 * - 0 on success
 * - 1 on failure:
 */
int loadTuyaId(void)
{
	char temp[256];
	char *key[TUYA_ID_KEY_COUNT] = { TUYA_ID_PID_KEY, TUYA_ID_UUID_KEY, TUYA_ID_AUTHKEY_KEY };
	char *value[TUYA_ID_KEY_COUNT] = { g_ipc_app_pid, g_ipc_app_uuid, g_ipc_app_authkey };
	int size[TUYA_ID_KEY_COUNT] = { TUYA_ID_PID_SIZE, TUYA_ID_UUID_SIZE, TUYA_ID_AUTHKEY_SIZE };
	char *dir_list[] = { TUYA_ID_SEARCH_PATH_2, TUYA_ID_SEARCH_PATH_1, NULL };
	char **dir = dir_list;
	int is_user_mode = 0;
	FILE *fp = NULL;

	/*
	 * By default, load from non-volatile memory
	 */
	for (int i = 0; i < TUYA_ID_KEY_COUNT; i++) {
		sprintf(temp, "fw_printenv -n %s", key[i]);
		fp = popen(temp, "r");
		if (!fp) {
			continue;
		}

		fgets(value[i], size[i] + 1, fp);
		value[i][size[i]] = 0;
		pclose(fp);
		fp = NULL;
	}

	/*
	 * Get current access mode
	 */
	sprintf(temp, CMD_MODE);
	fp = popen(temp, "r");
	if (fp) {
		fgets(temp, 256, fp);
		pclose(fp);
		fp = NULL;
		if (strncmp(temp, "user", 4) == 0) {
			is_user_mode = 1;
		}
	}

	/*
	 * Now, override previous result from file
	 * Note that ID overwrite should not work in user mode
	 */
	if (!is_user_mode) {
		while (*dir != NULL) {
			sprintf(temp, "%s/%s", *dir, TUYA_ID_FILE);
			FILE *fp = fopen(temp, "r");
			if (!fp) {
				dir++;
				continue;
			}

			while (!feof(fp)) {
				fgets(temp, 256, fp);
				char *k = strtok(temp, "=");
				char *v = strtok(NULL, "=");
				for (int i = 0; i < TUYA_ID_KEY_COUNT; i++) {
					if ((strcmp(k, key[i]) == 0) && (v != NULL)) {
						strncpy(value[i], v, size[i]);
						value[i][size[i]] = 0;
					}
				}
			}

			dir++;
			fclose(fp);
		}
	}

	/*
	 * Check IDs, only check string length
	 */
	for (int i = 0; i < TUYA_ID_KEY_COUNT; i++) {
		int len = strlen(value[i]);
		if (len != size[i]) {
			PR_ERR("Invalid %s = %s. (expecting %d chars but got %d)\n", key[i], value[i], size[i], len);
			return 1;
		}
	}

	return 0;
}

/**
 * @brief Show help messages
 * @param[in] app_name	Application name
 * @return None
 */
void showHelp(char *app_name)
{
	/*
	 * Help messages should not go to syslogd.
	 * just use printf().
	 */
	printf("Usage: %s [options]\n\n", basename(app_name));
	printf("Options:\n");
	printf("  -m 0\tStart with WiFi and QR paring mode.\n");
	printf("  -m 0 -s\tStart with WiFi paring mode.\n");
	printf("  -m 0 -q\tStart with QR paring mode.\n");
	printf("  -m 1\tStart with SoftAP paring mode.\n");
	printf("  -G 1\tEnable Google ChromeCast\n");
	printf("  -E 1\tEnable Echo Show\n");
	printf("  -h\tShow help.\n");

	return;
}

static void handle_sig_int(int signo)
{
	if (signo == SIGINT) {
		PR_DEBUG("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		PR_DEBUG("Caught SIGTERM!\n");
	} else {
		PR_ERR("Unexpected signal!\n");
		exit(1);
	}

	/*exit siren thread*/
#if (TUYA_DISABLE_AUDIO == 0)
	TUYA_APP_Siren_deinit();
#endif

	exit(0);
}

void sync_utc_time(void)
{
	INT_T err;
	/* Synchronize time with app */
	do {
		err = IPC_APP_Sync_Utc_Time();
		sleep(1);
	} while (err < 0);

	s_timesync_status = 1;
	PR_DEBUG("Time synchronized.\n");
}

/**
 * @brief Main function
 * @param[in] argc	Argument count
 * @param[in] argv	Argument values
 * @return
 * - 0 on success
 * - 1 on failure
 */
int main(int argc, char *argv[])
{
    struct sigaction sa;
	INT_T err;
	INT_T opt;
	WIFI_INIT_MODE_E mode = WIFI_INIT_AUTO;
	pthread_t time_sync_th;

	/*capture signal*/
	if (signal(SIGINT, handle_sig_int) == SIG_ERR) {
		PR_ERR("Cannot handle SIGINT!\n");
		return -1;
	}
    //signal(SIGPIPE, SIG_IGN);
    sa.sa_handler = SIG_IGN;
    sa.sa_flags = 0;
    if (sigaction(SIGPIPE, &sa, 0) == -1) {
        PR_ERR("Recv: SIGPIPE\n");
    }

	/* Parse command line arguments */
	/* m = Mode,
     * For m = 0, possible second arg are,
     *         s = snif mode
     *         q = qr-code
     *         if s,q not defined default run snif/qr-code modes
     *     m = 1, AP Mode
     *     m = 2, debug mode
     */
    while ((opt = getopt(argc, argv, "?m:s:q:v:E:G:h")) != -1) {
	    switch (opt) {
	    case 'm':
		    mode = atoi(optarg);
		    if (mode == 0) {
			    AGTX_TUYA_PAIR_QR_MODE = 1;
			    AGTX_TUYA_PAIR_SNIFF_MODE = 1;
		    }
		    break;
	    case 's':
		    if (mode == 0) {
			    AGTX_TUYA_PAIR_QR_MODE = 0;
			    break;
		    }
	    case 'q':
		    if (mode == 0) {
			    AGTX_TUYA_PAIR_SNIFF_MODE = 0;
			    break;
		    }
		    break;
	    case 'v':
		    g_verbose = atoi(optarg);
		    break;
	    case 'E':
		    if (atoi(optarg) == 1) {
			    //if (mode == 1) {
			    enable_echoShow = 1;
		    }
		    break;
	    case 'G':
		    if (atoi(optarg) == 1) {
			    //if (mode == 1) {
			    enable_chromecast = 1;
		    }
		    break;
	    case 'h':
	    default:
		    showHelp(argv[0]);
		    return -1;
	    }
    }

	/* Load PID, UUID and AUTHKEY */
	err = loadTuyaId();
	if (err) {
		return 1;
	}

	/* Create directory to store settings */
	struct stat st = { 0 };
	if (stat(IPC_APP_STORAGE_PATH, &st) == -1) {
		mkdir(IPC_APP_STORAGE_PATH, ACCESSPERMS);
	}

	/* Initialize SDK */
	initTuyaSdk(mode, "");
	PR_DEBUG("Tuya IPC SDK initialized.\n");

	/* Wait until online */
	while (s_mqtt_status != 1) {
		sleep(1);
	}
	PR_DEBUG("MQTT link is up.\n");

	TUYA_APP_Enable_P2PTransfer(s_mgr_info.max_p2p_user);
	PR_DEBUG("P2P initialized.\n");

	pthread_create(&time_sync_th, NULL, (void *)sync_utc_time, NULL);

	if (AG_Init_CClient() < 0) {
		printf("Failed to connect ccserver, reboot.\n");
		reboot(RB_AUTOBOOT);
	}
	PR_DEBUG("CC Server connected.\n");

	/* Start video server and register to it */
	int cnt = 30;
	char *av_main_argv[] = { "av_main2", NULL };
	forkIndependentProc("/system/bin/av_main2", av_main_argv);
	while (access(AVMAIN_RDY_FILE, F_OK)) {
		usleep(500000);

		/* Workaround solution as av_main is abnormally initialized
		 * Reboot system after 15 seconds if av_main no response */
		if (--cnt < 0) {
			printf("Failed to initialize av_main, reboot.\n");
			reboot(RB_AUTOBOOT);
		}
	}
	PR_DEBUG("AV Main initialized.\n");

	err = MPI_SYS_init();
	if (err) {
		PR_ERR("MPI system initialization failed.\n");
		return 1;
	}
	PR_DEBUG("MPI system initialized.\n");

	err = MPI_initBitStreamSystem();
	if (err) {
		PR_ERR("MPI bit-stream system initialization failed.\n");
		return 1;
	}
	PR_DEBUG("MPI bit-stream system initialized.\n");

	while (AVFTR_initClient(&avftrResShmClientFD, &avftrUnxSktClientFD, &avftr_res_shm_client)) {
		sleep(1);
	}
	PR_DEBUG("VFTR server connected.\n");

	/* Wait for time sync finish */
	if (pthread_join(time_sync_th, NULL)) {
		PR_ERR("Failed to join time_sync_th!\n");

	} else {
	    PR_ERR("Join time_sync_th.\n");
	}

	/* Load settings from cc server */
	loadPtzSetting();
	loadDataPointBasicOsd();
	loadDataPointIpcObjectOutline();
	loadDataPointIpcAutoSiren();
	loadVideoLayoutSetting();

	/* Start data point handling thread */
	pthread_t thread_data_point;
	pthread_create(&thread_data_point, NULL, threadDataPoint, NULL);
	pthread_detach(thread_data_point);

	/* Start local storage (Must be done after time synchronized) */
	TUYA_APP_Init_Stream_Storage(s_mgr_info.sd_base_path);
	PR_DEBUG("Local storage initialized.\n");

	/* Start main video stream collecting thread */
	pthread_t h264_output_thread_0;
	pthread_t h264_output_thread_1;
	if (IPC_APP_Quarry_Stream_Status(E_CHANNEL_VIDEO_MAIN) == TRUE) {
		STREAM_INFO *stream0 = malloc(sizeof(STREAM_INFO));
		if (stream0 == NULL) {
			PR_ERR("Stream V0 info buffer allocation failed.\n");
			return -1;
		}
		stream0->chn_type = E_CHANNEL_VIDEO_MAIN;
		stream0->chn_idx = MPI_VIDEO_WIN(0, 0, 0);
		stream0->stream_idx = MPI_ENC_CHN(0);
		pthread_create(&h264_output_thread_0, NULL, thread_live_video, (void *)stream0);
		//pthread_detach(h264_output_thread_0);
	}
	PR_DEBUG("Stream V0 initialized.\n");

	/* Start sub video stream collecting thread */
	if (IPC_APP_Quarry_Stream_Status(E_CHANNEL_VIDEO_SUB) == TRUE) {
		STREAM_INFO *stream1 = malloc(sizeof(STREAM_INFO));
		if (stream1 == NULL) {
			PR_ERR("Stream V1 info buffer allocation failed.\n");
			return -1;
		}
		stream1->chn_type = E_CHANNEL_VIDEO_SUB;
		stream1->chn_idx = MPI_VIDEO_WIN(0, 1, 0);
		stream1->stream_idx = MPI_ENC_CHN(1);
		pthread_create(&h264_output_thread_1, NULL, thread_live_video, (void *)stream1);
		//pthread_detach(h264_output_thread_1);
	}
	PR_DEBUG("Stream V1 initialized.\n");

	/* Start audio stream collecting thread */
#if (TUYA_DISABLE_AUDIO == 0)
	pthread_t pcm_output_thread;
	char *th_name_audio = "tuya_audio";
	pthread_create(&pcm_output_thread, NULL, thread_live_audio, th_name_audio);
	pthread_detach(pcm_output_thread);
	PR_DEBUG("Stream A0 initialized.\n");
#endif

	/* Start siren thread */
#if (TUYA_DISABLE_AUDIO == 0)
	TUYA_APP_Siren_init();
	PR_DEBUG("Siren initialized.\n");
#endif

	/* Start motion detection thread */
	TUYA_APP_Update_Md_Parameter();
	pthread_t motion_detect_thread;
	char *th_name_md = "tuya_md";
	pthread_create(&motion_detect_thread, NULL, thread_md_proc, th_name_md);
	pthread_detach(motion_detect_thread);
	if (pthread_setname_np(motion_detect_thread, th_name_md) != 0) {
		PR_DEBUG("Cannot set motion detect thread name.\n");
	}
	PR_DEBUG("Motion detection initialized.\n");

	/* Set up DP and upload settings and status */
	IPC_APP_upload_all_status();
	PR_DEBUG("Data points uploaded.\n");

	/* Start cloud storage */
	/* TUYA_APP_Enable_CloudStorage(); */
	/* PR_DEBUG("Cloud storage initialized.\n"); */

	/* Start Echo Show and Chromecast */
    if ( (enable_echoShow == 1) || (enable_chromecast == 1)) {
        TUYA_APP_Enable_EchoShow_Chromecast();
    }

    IPC_APP_Notify_LED_Sound_Status_CB(IPC_STREAM_READY);

    /* FIXME: Wait until termination request */
    while (1) {
	    if (0) {
		    g_tread_live_status[E_CHANNEL_VIDEO_MAIN] = STREAM_CLOSING;
		    g_tread_live_status[E_CHANNEL_VIDEO_SUB] = STREAM_CLOSING;

		    pthread_join(h264_output_thread_0, NULL);
		    pthread_join(h264_output_thread_1, NULL);
		    break;
	    }
	    usleep(1000 * 1000);
    }

	/* Unregister from video server and stop it */
	err = AVFTR_exitClient(&avftrResShmClientFD, &avftrUnxSktClientFD, &avftr_res_shm_client);
	if (err) {
		PR_ERR("Disconnecting from VFTR server failed.\n");
		return 1;
	}
	PR_DEBUG("VFTR server dis-connected.\n");


	err = MPI_exitBitStreamSystem();
	if (err) {
		PR_ERR("MPI bit-stream system de-initialization failed.\n");
		return 1;
	}
	PR_DEBUG("MPI bit-stream system de-initialized.\n");

	err = MPI_SYS_exit();
	if (err) {
		PR_ERR("MPI system de-initialization failed.\n");
		return 1;
	}
	PR_DEBUG("MPI system de-initialized.\n");

	AG_Exit_CClient();
	PR_DEBUG("CC Server dis-connected.\n");

	system("killall -s TERM av_main2");
	PR_DEBUG("AV Main terminated.\n");

	/* End program */
	return 0;
}
