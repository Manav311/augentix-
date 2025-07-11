/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/*********************************************************************************
  *Copyright(C),2015-2020, TUYA company www.tuya.comm
  *FileName: tuya_ipc_system_control_demo.c
  *
  * File description
  * The demo shows how the SDK uses callback to achieve system control, such as
  * 1. Setting local ID
  * 2. Restart System and Restart Process
  * 3. OTA upgrade
  * 4. Sound and LED prompts.
  *
**********************************************************************************/

/*
 * Caution:
 *   Include mpi_base_types.h in the very first one.
 *   In order to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"
#include "agtx_types.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_media.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_cloud_types.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_system_control_demo.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_dp_handler.h"
#include "tuya_ipc_dp_utils.h"
#include "tuya_ipc_p2p.h"
#include "ledevt.h"

#include <string.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <sys/reboot.h>
#include <signal.h>
#include <pthread.h>

// audio
#include <alsa/asoundlib.h>
#include <alsa/error.h>
#include <alsa/hwdep.h>
#include <pcm_interfaces.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/resource.h>

extern IPC_MGR_INFO_S s_mgr_info;
snd_pcm_t *ghandle = NULL;

#define cmd_sysupd "/usr/sbin/sysupd"
#define cmd_sysupd_flag "touch /tmp/SystemUpgradeFlag"
#define cmd_vsterm "/system/bin/vsterm"
#define cmd_md5sum "md5sum /tmp/update.swu | awk '{print $1 > \"/tmp/md5sum\"}'"
#define SW_VER_PATH "/etc/sw-version"
#define md5sum_PATH "/tmp/md5sum"
#define SIREN_FRAME_SIZE 320
#define SIREN_PATH "/system/factory_default/siren.ul"
#define AUDIO_PERIOD_SIZE 320
#define RD_VALUE_SECS _IOR('a', 'S', long int *)
#define RD_VALUE_USEC _IOR('a', 'U', long int *)

typedef struct _siren_control_s {
	int enable; /*enable siren*/
	int force_siren;
	int siren_stop;
	int speaker_on;
	int siren_on;
	int alarm_on;
	int siren_play_time; /*seconds*/
	int siren_timeout_time;
	int end_play;
	int exit;
	int fd;
	int voice_resume_time;
	int siren_volume;
	int voice_volume;
	char *audiobuf;
} siren_control_s;

#if 1
char gsilence_buf[AUDIO_PERIOD_SIZE * 2] = { 0 };
#endif
siren_control_s gsiren_control = { 0 };
pthread_mutex_t gsiren_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t gsiren_thread;

#if 0
int gettimeofdayMonotonic(struct timeval *t1, int *tz)
{
	int fd, ret = 0;
	long int second, usecond;

	fd = open("/dev/etx_device", O_RDWR);
	if (fd < 0) {
		PR_ERR("Cannot open /dev/etx_device...\n");
	} else {
		t1->tv_sec = ioctl(fd, RD_VALUE_SECS, &second);
		t1->tv_usec = ioctl(fd, RD_VALUE_USEC, &usecond);
		t1->tv_sec = second;
		t1->tv_usec = usecond;

		//PR_INFO("%ld|%ld\n",second,usecond);
		ret = close(fd);
	}
	return ret;
}
#else
int gettimeofdayMonotonic(struct timeval *t1, int *tz __attribute__((unused)))
{
	struct timespec tp;
	int ret;
	clockid_t clk_id;
	clk_id = CLOCK_MONOTONIC;
	ret = clock_gettime(clk_id, &tp);
	//t1->tv_sec = tp.tv_sec;
	//t1->tv_sec = 0xfffffed3 + tp.tv_sec;
	t1->tv_sec = tp.tv_sec; // + 0x83AA7E80; //(1970 epoch -> 1900 epoch)
	t1->tv_usec = (tp.tv_nsec) / 1000; //nano to milli
	return ret;
}
#endif

/*
Callback when the user clicks on the APP to remove the device
*/
VOID IPC_APP_Reset_System_CB(GW_RESET_TYPE_E type __attribute__((unused)))
{
	IPC_APP_Notify_LED_Sound_Status_CB(IPC_RESET_SUCCESS);

	/* reboot system */
	sync();
	reboot(RB_AUTOBOOT);
}

VOID IPC_APP_Restart_Process_CB(VOID)
{
	/* According to FAE, this is not used */
}

/* Get sw version */
VOID get_sw_version(char **sw_version)
{
	FILE *sw_ver;
	sw_ver = fopen(SW_VER_PATH, "r");
	if (sw_ver) {
		fscanf(sw_ver, "%s\n", *sw_version);
	}
	fclose(sw_ver);
}

/* OTA */
#define OTA_UPDATED_PCTG 80 /* Status percentage when OTA update finished */
#define OTA_NICENESS 0
//Callback after downloading OTA files
VOID __IPC_APP_upgrade_notify_cb(IN CONST FW_UG_S *fw, IN CONST INT_T download_result, IN PVOID_T pri_data)
{
	char dw_md5sum[128];
	FILE *p_upgrade_fd = (FILE *)pri_data;
	fclose(p_upgrade_fd);

	PR_DEBUG("Upgrade Finish");
	PR_DEBUG("download_result:%d fw_url:%s", download_result, fw->fw_url);

	if (download_result == 0) {
		/* The developer needs to implement the operation of OTA upgrade,
        when the OTA file has been downloaded successfully to the specified path. [ p_mgr_info->upgrade_file_path ]*/
		system(cmd_md5sum);
		FILE *md5sum;
		md5sum = fopen(md5sum_PATH, "r");
		if (md5sum) {
			fscanf(md5sum, "%s\n", dw_md5sum);
		}
		fclose(md5sum);
		PR_DEBUG("fw_md5:%s", fw->fw_md5);
		if (strcmp(dw_md5sum, fw->fw_md5) == 0) {
			PR_DEBUG("Upgrade Verification Finish");
			/* Make sure system update is running at niceness 0 */
			if(setpriority(PRIO_PROCESS, 0, OTA_NICENESS) != OTA_NICENESS) {
				PR_ERR("Unable to change niceness");
				goto end_ota;
			}
			system(cmd_sysupd);
		} else {
			PR_DEBUG("Upgrade Verification Fail; System reboot..");
		}
	}
	tuya_ipc_upgrade_progress_report(OTA_UPDATED_PCTG);
end_ota:
	setLEDInform("OTA", 0);
	/* reboot system */
	sync();
	reboot(RB_AUTOBOOT);
}

//To collect OTA files in fragments and write them to local files
OPERATE_RET __IPC_APP_get_file_data_cb(IN CONST FW_UG_S *fw, IN CONST UINT_T total_len, IN CONST UINT_T offset,
                                       IN CONST BYTE_T *data __attribute__((unused)), IN CONST UINT_T len,
                                       OUT UINT_T *remain_len __attribute__((unused)),
                                       IN PVOID_T pri_data __attribute__((unused)))
{
	PR_DEBUG("Rev File Data");
	PR_DEBUG("total_len:%d  fw_url:%s", total_len, fw->fw_url);
	PR_DEBUG("Offset:%d Len:%d", offset, len);

	//report UPGRADE process, NOT only download percent, consider flash-write time
	//APP will report overtime fail, if uprgade process is not updated within 60 seconds

	int download_percent = (offset * 70) / (total_len + 1);
	int report_percent = download_percent / 2; // as an example, download 100% = 50%  upgrade work finished
	tuya_ipc_upgrade_progress_report(report_percent);

	if (offset == total_len) // finished downloading
	{
		//start write OTA file to flash by parts
		/* only for example: 
        FILE *p_upgrade_fd = (FILE *)pri_data;
        fwrite(data, 1, len, p_upgrade_fd);
        *remain_len = 0;
        */
		// finish 1st part
		report_percent += 10;
		tuya_ipc_upgrade_progress_report(report_percent);
		// finish 2nd part
		sleep(5);
		report_percent += 10;
		tuya_ipc_upgrade_progress_report(report_percent);
		// finish all parts, set to 90% for example
		report_percent = 90;
		tuya_ipc_upgrade_progress_report(report_percent);
	}

	//APP will report "uprage success" after reboot and new FW version is reported inside SDK automaticlly

	return OPRT_OK;
}

VOID IPC_APP_Upgrade_Inform_cb(IN CONST FW_UG_S *fw)
{
	PR_DEBUG("Rev Upgrade Info");
	PR_DEBUG("fw->fw_url:%s", fw->fw_url);
	PR_DEBUG("fw->fw_md5:%s", fw->fw_md5);
	PR_DEBUG("fw->sw_ver:%s", fw->sw_ver);
	PR_DEBUG("fw->file_size:%u", fw->file_size);

	setLEDInform("OTA", 1);
	system(cmd_sysupd_flag);
	FILE *p_upgrade_fd = fopen(s_mgr_info.upgrade_file_path, "w+b");
	PR_DEBUG("Set SS write mode to SS_WRITE_MODE_NONE");
	tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
	//PR_DEBUG("Quit transfer");
	//tuya_ipc_tranfser_quit();
	PR_DEBUG("Invoke vsterm");
	system(cmd_vsterm);
	PR_DEBUG("Initiate OTA");
	tuya_ipc_upgrade_sdk(fw, __IPC_APP_get_file_data_cb, __IPC_APP_upgrade_notify_cb, p_upgrade_fd);
}

/* Developers need to implement the corresponding prompt sound playback and LED prompts,
   you can refer to the SDK attached files, using TUYA audio files. */
VOID IPC_APP_Notify_LED_Sound_Status_CB(IPC_APP_NOTIFY_EVENT_E notify_event)
{
	return; //Have error. remove this function first.
	/* IPC_BOOTUP_FINISH,
       IPC_START_WIFI_CFG,
       IPC_REV_WIFI_CFG,
       IPC_CONNECTING_WIFI,
       IPC_MQTT_ONLINE,
       IPC_RESET_SUCCESS,
     */
	switch (notify_event) {
	case IPC_BOOTUP_FINISH:
		/* Startup success */
		PR_INFO("curr event: %d (IPC_BOOTUP_FINISH)\n", notify_event);
		break;
	case IPC_START_WIFI_CFG:
		/* Start configuring the network */
		PR_INFO("curr event: %d (IPC_START_WIFI_CFG)\n", notify_event);
		setLEDInform("Wifi_Pairing", 1);
		break;
	case IPC_REV_WIFI_CFG:
		/* Receive network configuration information */
		PR_INFO("curr event: %d (IPC_REV_WIFI_CFG)\n", notify_event);
		break;
	case IPC_CONNECTING_WIFI:
		/* Start Connecting WIFI */
		PR_INFO("curr event: %d (IPC_CONNECTING_WIFI)\n", notify_event);
		setLEDInform("Wifi_Connecting", 1);
		break;
	case IPC_MQTT_ONLINE:
		/* MQTT on-line */
		PR_INFO("curr event: %d (IPC_MQTT_ONLINE)\n", notify_event);
		setLEDInform("Wifi_Connected", 1);
		break;
	case IPC_STREAM_READY:
		PR_INFO("curr event: %d (IPC_STREAM_READY)\n", notify_event);
		break;
	case IPC_WEAK_WIFI_TRUE:
		PR_INFO("curr event: %d (IPC_WEAK_WIFI_TRUE)\n", notify_event);
		setLEDInform("Low_Signal", 1);
		break;
	case IPC_WEAK_WIFI_FALSE:
		PR_INFO("curr event: %d (IPC_WEAK_WIFI_FALSE)\n", notify_event);
		setLEDInform("Low_Signal", 0);
		break;
	case IPC_LIVE_VIDEO_START:
		PR_INFO("curr event: %d (IPC_LIVE_VIDEO_START)\n", notify_event);
		setLEDInform("Live_view", 1);
		break;
	case IPC_LIVE_VIDEO_STOP:
		PR_INFO("curr event: %d (IPC_LIVE_VIDEO_STOP)\n", notify_event);
		setLEDInform("Live_view", 0);
		break;
	case IPC_RESET_SUCCESS:
		/* Reset completed */
		PR_INFO("curr event: %d (IPC_RESET_SUCCESS)\n", notify_event);
		break;
	default:
		break;
	}
}

VOID IPC_APP_Notify_Siren(int alarm)
{
	siren_control_s *siren_ctrl = &gsiren_control;
	struct timeval t1 = { 0 };

	PR_INFO("IPC_APP_Notify_Siren alarm %d\n", alarm);

	if (!siren_ctrl->enable) {
		return;
	}

	if (alarm) {
		gettimeofdayMonotonic(&t1, NULL);
		pthread_mutex_lock(&gsiren_mutex);
		if (siren_ctrl->end_play) {
			siren_ctrl->siren_timeout_time = t1.tv_sec + siren_ctrl->siren_play_time;
			siren_ctrl->end_play = 0;
			siren_ctrl->siren_stop = 0;
		}
		siren_ctrl->alarm_on = 1; /*unuse*/
		siren_ctrl->siren_on = siren_ctrl->speaker_on ? 0 : 1;
		IPC_APP_Report_siren_switch(siren_ctrl->siren_on & !siren_ctrl->siren_stop);
		pthread_mutex_unlock(&gsiren_mutex);
	} else {
		pthread_mutex_lock(&gsiren_mutex);
		siren_ctrl->alarm_on = 0; /*unuse*/
		//		siren_ctrl->siren_timeout_time = 0xFFFFFFFF;
		pthread_mutex_unlock(&gsiren_mutex);
	}
}

int aux_set_audio_pb_gain(int volume)
{
	int err;
	snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Speaker";
	long db = -(long)(log2(((double)100 / volume)) * 10 * 100);

	if (volume < 0 || volume > 100) {
		PR_ERR("Wrong volume value %d\n", volume);
		assert(0);
		return -EINVAL;
	}

	/* open an empty mixer */
	err = snd_mixer_open(&handle, 0);
	if (err < 0) {
		PR_ERR("snd_mixer_open failed: %s\n", snd_strerror(err));
		return err;
	}
	/* attach an HCTL specified with the CTL device name to an opened mixer */
	err = snd_mixer_attach(handle, card);
	if (err < 0) {
		PR_ERR("snd_mixer_attach failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* register mixer simple element class */
	err = snd_mixer_selem_register(handle, NULL, NULL);
	if (err < 0) {
		PR_ERR("snd_mixer_selem_register failed: %s\n", snd_strerror(err));
		goto failed;
	}
	/* load a mixer elements */
	err = snd_mixer_load(handle);
	if (err < 0) {
		PR_ERR("snd_mixer_load failed: %s\n", snd_strerror(err));
		goto failed;
	}

	/* allocat an invalid snd_ mixer_selem_id_t */
	snd_mixer_selem_id_alloca(&sid);

	/* set playback gain ( "Speaker" ) */
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	elem = snd_mixer_find_selem(handle, sid);
	if (elem) {
		err = snd_mixer_selem_set_playback_dB_all(elem, db, 0);
		if (err < 0) {
			fprintf(stderr, "snd_mixer_selem_set_playback_dB_all: %s\n", snd_strerror(err));
			goto failed;
		}
	}

	snd_mixer_close(handle);
	return 0;

failed:
	snd_mixer_close(handle);
	return err;
}

int aux_set_pcm_pb_codec(codec_mode_t mode)
{
	// const char *devicename = "hw:0,0";
	const char *devicename = "default";
	snd_hwdep_t *hwdep;
	int err;
	int codec_mode = mode;

	PR_INFO("codec_mode = %d\n", codec_mode);

	if ((err = snd_hwdep_open(&hwdep, devicename, O_RDWR)) < 0) {
		PR_INFO("hwdep interface open error: %s \n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_SET_DEC_MODE, &codec_mode)) < 0) {
		PR_INFO("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	if ((err = snd_hwdep_ioctl(hwdep, HC_IOCTL_GET_DEC_MODE, &codec_mode)) < 0) {
		PR_INFO("hwdep ioctl error: %s \n", snd_strerror(err));
	}

	snd_hwdep_close(hwdep);

	return 0;
}

void aux_pcm_pb_init(unsigned int codec __attribute__((unused)), int audio_gain __attribute__((unused)))
{
	int err = 0;
	snd_pcm_uframes_t frames = AUDIO_PERIOD_SIZE;
	unsigned int channels = 1;
	unsigned int rate = 8000;
	static char *device = "default";
	snd_pcm_hw_params_t *params;
	snd_pcm_stream_t stream = SND_PCM_STREAM_PLAYBACK;
	snd_pcm_format_t format = SND_PCM_FORMAT_MU_LAW;
	//	snd_async_handler_t *pcm_callback;

	/* Open PCM device for playback. */
	err = snd_pcm_open(&ghandle, device, stream, 0);
	if (err < 0) {
		PR_ERR("unable to open pcm device: %s\n", snd_strerror(err));
		ghandle = NULL;
	}

	/* Allocate a hardware parameters object. */
	snd_pcm_hw_params_malloc(&params);

	/* Fill it in with default values. */
	snd_pcm_hw_params_any(ghandle, params);

	/* Set the desired hardware parameters. */
	/* Interleaved mode */
	err = snd_pcm_hw_params_set_access(ghandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_access: %s\n", snd_strerror(err));
		ghandle = NULL;
	}

	/* Signed 16-bit little-endian format */
	err = snd_pcm_hw_params_set_format(ghandle, params, format);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_format: %s\n", snd_strerror(err));
		ghandle = NULL;
	}

	/* Two channels (stereo) */
	err = snd_pcm_hw_params_set_channels(ghandle, params, channels);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_channels:%s\n", snd_strerror(err));
		ghandle = NULL;
	}

	/* 8000 bits/second sampling rate */
	err = snd_pcm_hw_params_set_rate_near(ghandle, params, &rate, 0);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_rate_near: %s\n", snd_strerror(err));
		ghandle = NULL;
	}

	/* Set period size to 1024 frames. */
	err = snd_pcm_hw_params_set_period_size_near(ghandle, params, &frames, 0);
	if (err < 0) {
		PR_ERR("snd_pcm_hw_params_set_period_size_near: %s\n", snd_strerror(err));
		ghandle = NULL;
	}

	/* Write the parameters to the driver */
	err = snd_pcm_hw_params(ghandle, params);
	if (err < 0) {
		PR_ERR("unable to set hw parameters: %s\n", snd_strerror(err));
		ghandle = NULL;
	}

	//aux_set_pcm_pb_codec(codec);

	/*gain control by init script*/
	// err = aux_set_audio_pb_gain(audio_gain);
	// if (err < 0) {
	// 	PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
	// 	// exit(EXIT_FAILURE);
	// }

#if 0
	/*Fill silence data to ring buffer to avoid underrun, but increase latency*/
	if (snd_pcm_format_set_silence(SND_PCM_FORMAT_MU_LAW, gsilence_buf, AUDIO_PERIOD_SIZE * 2) < 0) {
		PR_ERR("silence error\n");
		ghandle = NULL;
	}
#endif
#if 0
	for (i = 0; i < 3; i++) {
		frames = snd_pcm_writei(ghandle, silence_buf, AUDIO_PERIOD_SIZE);
	}
#endif
}

VOID *thread_siren_proc(VOID *arg)
{
	AGTX_UNUSED(arg);
	int size = 0;
	int file_size = 0;
	int offset = 0;
	char *audioBuf = NULL;
	snd_pcm_uframes_t frames;
	siren_control_s *siren_ctrl = &gsiren_control;
	struct timeval t1 = { 0 };
	int fd = 0;

	//	pthread_detach(pthread_self());

	siren_ctrl->force_siren = 0;
	siren_ctrl->siren_stop = 0;
	siren_ctrl->alarm_on = 0;
	siren_ctrl->siren_on = 0;
	siren_ctrl->end_play = 1;
	siren_ctrl->siren_timeout_time = 0xFFFFFFFF;
	siren_ctrl->speaker_on = 0;

	aux_pcm_pb_init(MU_LAW, siren_ctrl->siren_volume);

	if (ghandle == NULL) {
		PR_ERR("audio playback initial failed\n");
		pthread_exit(0);
	}

	siren_ctrl->fd = open(SIREN_PATH, O_RDONLY);
	fd = siren_ctrl->fd;
	if (fd < 0) {
		PR_ERR("\n\topen siren file failed\n\t");
		pthread_exit(0);
	}
	file_size = lseek(fd, 0, SEEK_END);
	PR_NOTICE("siren file size %d\n", file_size);
	siren_ctrl->audiobuf = malloc(file_size);
	if (!siren_ctrl->audiobuf) {
		PR_ERR("No memory to allocate siren\n\t");
	}
	audioBuf = siren_ctrl->audiobuf;
	lseek(fd, 0, SEEK_SET);
	size = read(fd, audioBuf, file_size);
	if (file_size != size) {
		PR_ERR("read siren file failed %d\n", size);
		pthread_exit(0);
	}

	while (1) {
		if (siren_ctrl->exit) {
			PR_INFO("close siren\n");
			snd_pcm_drain(ghandle);
			snd_pcm_close(ghandle);
			break;
		}

		gettimeofdayMonotonic(&t1, NULL);

		/*end of play time*/
		if (!siren_ctrl->end_play && (siren_ctrl->siren_timeout_time <= t1.tv_sec)) {
			//			PR_INFO("end of play time %d / %d\n", (int)siren_ctrl->siren_timeout_time, (int)t1.tv_sec);
			pthread_mutex_lock(&gsiren_mutex);
			siren_ctrl->end_play = 1;
			pthread_mutex_unlock(&gsiren_mutex);
		}

		/*end of play time, disable siren*/
		if ((siren_ctrl->siren_on && siren_ctrl->end_play) && !siren_ctrl->force_siren) {
			pthread_mutex_lock(&gsiren_mutex);
			siren_ctrl->siren_on = 0;
			IPC_APP_Report_siren_switch(siren_ctrl->siren_on & !siren_ctrl->siren_stop);
			pthread_mutex_unlock(&gsiren_mutex);
		}

		/*speaker enable*/
		if (siren_ctrl->siren_on && siren_ctrl->speaker_on) {
			PR_NOTICE("speaker enable\n");
			pthread_mutex_lock(&gsiren_mutex);
			siren_ctrl->siren_on = 0;
			IPC_APP_Report_siren_switch(siren_ctrl->siren_on & !siren_ctrl->siren_stop);
			pthread_mutex_unlock(&gsiren_mutex);
		}
		//		PR_INFO("siren_ctrl->siren_on %d sec %d\n",siren_ctrl->siren_on,t1.tv_sec);
		if (siren_ctrl->siren_on && !siren_ctrl->siren_stop) {
			/* Set blocking mode */
			snd_pcm_nonblock(ghandle, 0);

			if ((offset + SIREN_FRAME_SIZE) >= file_size) {
				size = file_size - offset;
			} else {
				size = SIREN_FRAME_SIZE;
			}

			frames = snd_pcm_writei(ghandle, audioBuf + offset, size);
			if ((int)frames == -EPIPE) {
				snd_pcm_prepare(ghandle);
				frames = snd_pcm_writei(ghandle, audioBuf + offset, size);
			}

			if ((offset + SIREN_FRAME_SIZE) >= file_size) {
				offset = 0;
			} else {
				offset += SIREN_FRAME_SIZE;
			}
		} else {
			usleep(30000);
		}
	}

	if (siren_ctrl->audiobuf) {
		free(siren_ctrl->audiobuf);
	}

	pthread_exit(0);
}

VOID resume_siren()
{
	siren_control_s *siren_ctrl = &gsiren_control;

	PR_DEBUG("resume_siren %d / %d\n", siren_ctrl->end_play, siren_ctrl->force_siren);
	pthread_mutex_lock(&gsiren_mutex);
	if (!siren_ctrl->end_play || siren_ctrl->force_siren) {
		siren_ctrl->siren_on = 1;
		IPC_APP_Report_siren_switch(siren_ctrl->siren_on & !siren_ctrl->siren_stop);
	}
	pthread_mutex_unlock(&gsiren_mutex);
}

/* Callback of talkback mode,turn on or off speaker hardware*/
VOID TUYA_APP_Enable_Speaker_CB(BOOL_T enabled)
{
	int cnt = 0;
	int err = 0;
	siren_control_s *siren_ctrl = &gsiren_control;

	PR_NOTICE("enable speaker %d \n", enabled);
	if (enabled) {
		siren_ctrl->speaker_on = 1;
		while (siren_ctrl->siren_on) {
			cnt++;
			if (cnt > 100) {
				PR_ERR("Enable_Speaker timeout failed %d / %d\n", siren_ctrl->siren_on,
				       siren_ctrl->end_play);
				break;
			}
			usleep(20000);
		}
		err = aux_set_audio_pb_gain(siren_ctrl->voice_volume);
		if (err < 0) {
			PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
			// exit(EXIT_FAILURE);
		}
	} else {
		if (siren_ctrl->speaker_on) {
			/* Waiting for n seconds to resume siren */
			signal(SIGALRM, resume_siren);
			alarm(siren_ctrl->voice_resume_time);
		}
		siren_ctrl->speaker_on = 0;
		err = aux_set_audio_pb_gain(siren_ctrl->siren_volume);
		if (err < 0) {
			PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
			// exit(EXIT_FAILURE);
		}
	}
	/* Developers need to turn on or off speaker hardware operations.
    If IPC hardware features do not need to be explicitly turned on, the function can be left blank. */
}

#define PB_RING_BUFFER_SIZE (3200)
#define PB_RING_BUFFER_THRESHOLD (2048)
static void *pbuf[PB_RING_BUFFER_SIZE];
static unsigned int pbuf_size = 0;
static unsigned int pflush = 0;
/* Callback of talkback mode,turn on or off the sound */
VOID TUYA_APP_Rev_Audio_CB(IN CONST MEDIA_FRAME_S *p_audio_frame,
                           TUYA_AUDIO_SAMPLE_E audio_sample __attribute__((unused)),
                           TUYA_AUDIO_DATABITS_E audio_databits __attribute__((unused)),
                           TUYA_AUDIO_CHANNEL_E audio_channel __attribute__((unused)))
{
	snd_pcm_uframes_t frames;
	siren_control_s *siren_ctrl = &gsiren_control;

	//	PR_INFO("TODO: rev audio cb len:%u sample:%d db:%d channel:%d\n", p_audio_frame->size, audio_sample,
	//	       audio_databits, audio_channel);

	/*if siren not disable, don't output audio*/
	if (!siren_ctrl->siren_on && siren_ctrl->speaker_on) {
		if (pbuf_size + p_audio_frame->size > PB_RING_BUFFER_SIZE) {
			printf("[%s] buffer full\n", __func__);
		}
		memcpy(pbuf, p_audio_frame->p_buf, p_audio_frame->size);
		pbuf_size += p_audio_frame->size;

		if (pbuf_size > PB_RING_BUFFER_THRESHOLD) {
			pflush = 1;
		}

		if (pflush == 1) {
			snd_pcm_uframes_t flushed_frames = 0;

			/* Set nonblocking mode */
			snd_pcm_nonblock(ghandle, 1);

			while (flushed_frames < pbuf_size) {
				/*APP always send G711u*/
				//frames = snd_pcm_writei(ghandle, p_audio_frame->p_buf, p_audio_frame->size);
				frames = snd_pcm_writei(ghandle, &pbuf[flushed_frames], AUDIO_PERIOD_SIZE);
				if ((int)frames == -EPIPE) {
					snd_pcm_prepare(ghandle);
					/* *Workaround* : Fill silent buffer to reduce EPIPE rate*/
					//snd_pcm_writei(ghandle, gsilence_buf, AUDIO_PERIOD_SIZE);
					//frames = snd_pcm_writei(ghandle, p_audio_frame->p_buf, p_audio_frame->size);
					printf("[%s] EPIPE\n", __func__);
				} else if ((int)frames == -EAGAIN) {
					//frames = snd_pcm_writei(ghandle, p_audio_frame->p_buf, p_audio_frame->size);
					printf("[%s] EAGAIN\n", __func__);
				} else if ((int)frames == AUDIO_PERIOD_SIZE) {
					flushed_frames += frames;
				} else {
					int i;
					flushed_frames += frames;
					for (i = 0; i < PB_RING_BUFFER_SIZE - (int)flushed_frames; i++) {
						pbuf[i] = pbuf[i + flushed_frames];
					}
					break;
				}
			}
			pbuf_size -= flushed_frames;
		}
	}
}

OPERATE_RET IPC_APP_Sync_Utc_Time(VOID)
{
	TIME_T time_utc;
	INT_T time_zone;
	int hr;
	int min = 0;
	BOOL_T pIsDls = false;
	char cmdstr[36];
	char buf[36];
	struct tm now_time;
	PR_DEBUG("Get Server Time ");
	OPERATE_RET ret = tuya_ipc_get_service_time_force(&time_utc, &time_zone);

	if (ret != OPRT_OK) {
		return ret;
	}
	//The API returns OK, indicating that UTC time has been successfully obtained.
	//If it return not OK, the time has not been fetched.

	PR_DEBUG("Get Server Time Success: %u %d ", time_utc, time_zone);

	ret = tuya_ipc_check_in_dls(time_utc, &pIsDls);
	if (ret != OPRT_OK) {
		return ret;
	}
	PR_DEBUG("DST: %d", pIsDls);

	hr = time_zone / 3600;
	min = time_zone / 60 - hr * 60;
	if (min < 0) {
		min = -min;
	}

	if (hr > 0) {
		if (pIsDls) {
			sprintf(cmdstr, "GMT-%d:%dDST", hr, min);
		} else {
			sprintf(cmdstr, "GMT-%d:%d", hr, min);
		}
	} else {
		hr = -hr;
		if (pIsDls) {
			sprintf(cmdstr, "GMT%d:%dDST", hr, min);
		} else {
			sprintf(cmdstr, "GMT%d:%d", hr, min);
		}
	}

	sprintf(buf, "/system/script/DSTConf.sh set %s", cmdstr);
	system(buf);

	localtime_r((const time_t *)(&time_utc), &now_time);
	sprintf(buf, "/system/script/time.sh %d.%d.%d-%d:%d:%d", now_time.tm_year + 1900, now_time.tm_mon + 1,
	        now_time.tm_mday, now_time.tm_hour, now_time.tm_min, now_time.tm_sec);
	ret = system(buf);

	if (ret != 0) {
		PR_DEBUG("Set System Time Fail");
		return ret;
	}

	return OPRT_OK;
}

VOID IPC_APP_Show_OSD_Time(VOID)
{
	struct tm localTime;
	tuya_ipc_get_tm_with_timezone_dls(&localTime);
	PR_DEBUG("show OSD [%04d-%02d-%02d %02d:%02d:%02d]", localTime.tm_year, localTime.tm_mon, localTime.tm_mday,
	         localTime.tm_hour, localTime.tm_min, localTime.tm_sec);
}

BOOL_T TUYA_APP_Get_Siren_Status()
{
	siren_control_s *siren_ctrl = &gsiren_control;

	return (BOOL_T)siren_ctrl->siren_on & !siren_ctrl->siren_stop;
}

VOID TUYA_APP_Force_Siren_On(BOOL_T on)
{
	siren_control_s *siren_ctrl = &gsiren_control;

	if (siren_ctrl->speaker_on) {
		PR_NOTICE("Speaker enabled, siren can't output\n");
		return;
	}

	PR_DEBUG("TUYA_APP_Force_Siren_On %d\n", on);

	pthread_mutex_lock(&gsiren_mutex);
	if (on) {
		/*switch to force mode*/
		siren_ctrl->siren_stop = 0;
		siren_ctrl->force_siren = 1;
		siren_ctrl->siren_on = 1;
	} else {
		/*stop output siren, if end play, siren_on will be closed in siren thread.*/
		siren_ctrl->siren_stop = 1;
		siren_ctrl->force_siren = 0;
	}
	pthread_mutex_unlock(&gsiren_mutex);
}

VOID TUYA_APP_Update_Siren_Parameter()
{
	int err = 0;
	siren_control_s *siren_ctrl = &gsiren_control;
	TUYA_AG_CONF_S *conf = NULL;
	AGTX_SIREN_CONF_S *siren = NULL;

	AG_Get_Conf(&conf);
	siren = &conf->siren.data;

	siren_ctrl->enable = siren->enabled;
	siren_ctrl->siren_play_time = siren->duration;
	siren_ctrl->siren_volume = siren->volume;

	if (siren_ctrl->speaker_on) {
		err = aux_set_audio_pb_gain(siren_ctrl->voice_volume);
		if (err < 0) {
			PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
		}
	} else {
		err = aux_set_audio_pb_gain(siren_ctrl->siren_volume);
		if (err < 0) {
			PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
			// exit(EXIT_FAILURE);
		}
	}

	PR_NOTICE("siren hold time %d volume %d\n", siren_ctrl->siren_play_time, siren_ctrl->siren_volume);
}

VOID TUYA_APP_Update_Voice_Parameter()
{
	int err = 0;
	siren_control_s *siren_ctrl = &gsiren_control;
	TUYA_AG_CONF_S *conf = NULL;
	AGTX_VOICE_CONF_S *voice = NULL;

	AG_Get_Conf(&conf);

	voice = &conf->voice.data;

	siren_ctrl->voice_resume_time = voice->hold_time;
	siren_ctrl->voice_volume = voice->volume;

	if (siren_ctrl->speaker_on) {
		err = aux_set_audio_pb_gain(siren_ctrl->voice_volume);
		if (err < 0) {
			PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
		}
	} else {
		err = aux_set_audio_pb_gain(siren_ctrl->siren_volume);
		if (err < 0) {
			PR_ERR("Audio: set_gain failed: %s\n", snd_strerror(err));
			// exit(EXIT_FAILURE);
		}
	}

	PR_NOTICE("voice hold time %d volume %d\n", siren_ctrl->voice_resume_time, siren_ctrl->voice_volume);
}

VOID TUYA_APP_Siren_init()
{
	char *th_name_siren = "tuya_siren";

	TUYA_APP_Update_Siren_Parameter();
	TUYA_APP_Update_Voice_Parameter();

	pthread_create(&gsiren_thread, NULL, thread_siren_proc, th_name_siren);
	//	pthread_detach(gsiren_thread);
}

VOID TUYA_APP_Siren_deinit()
{
	int ret = 0;
	siren_control_s *siren_ctrl = &gsiren_control;

	PR_INFO("TUYA_APP_Siren_deinit\n");
	siren_ctrl->exit = 1;
	ret = pthread_join(gsiren_thread, NULL);
	PR_INFO("TUYA_APP_Siren_deinit ret %d\n", ret);
}
