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
  *Copyright(C),2015-2020, TUYA www.tuya.comm
  *FileName:    tuya_ipc_sd_demo
**********************************************************************************/

/*
 * Caution:
 *   Include mpi_base_types.h in the very first one.
 *   In order to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_stream_storage.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_dp_handler.h"

#include <sys/statfs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/prctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "errno.h"

/************************
Description: Using the TUYA SD card storage and playback function, 
the developers need to implement the relevant interface.
Note Interface names cannot be changed, and these interfaces are declared and used in SDK.
This demo file gives the management implementation of SD card operation for typical embedded devices in Linux system.
Developers can modify it according to their practical application.

VOID tuya_ipc_sd_format(VOID);
VOID tuya_ipc_sd_remount(VOID);
E_SD_STATUS tuya_ipc_sd_get_status(VOID);
VOID tuya_ipc_sd_get_capacity(UINT_T *p_total, UINT_T *p_used, UINT_T *p_free);
CHAR_T *tuya_ipc_get_sd_mount_path(VOID);

************************/
#define MAX_MMC_NAME_LEN 16
#define MAX_MOUNTS_INFO_LEN 1024

STATIC CHAR_T s_mounts_info[MAX_MOUNTS_INFO_LEN];
STATIC CHAR_T s_mmcblk_name[MAX_MMC_NAME_LEN] = { 0 };
int gformating = 0;
int gmount_failed = 0;
int gpre_record_time = 0;

VOID tuya_ipc_set_format_flag(int formating)
{
	gformating = formating;

	return;
}

//Implementation Interface of Formatting Operation
OPERATE_RET tuya_ipc_sd_format(VOID)
{
	int status = 0;
	char *ptr = NULL;
	char name[256] = { 0 };
	CHAR_T format_cmd[256] = { 0 };

	PR_NOTICE("sd format begin\n");

	/*umount sdcard, sleep 1 waiting for record stop*/
	snprintf(format_cmd, 256, "umount /mnt/sdcard; sleep 1;");

	status = system(format_cmd);
	if (status < 0 || (status >> 8)) {
		PR_ERR("[sd format] umount error: %s\n", strerror(errno));
		return -2001;
	}

	if (gmount_failed) {
		snprintf(format_cmd, 256, "dd if=/dev/zero of=%s bs=1M count=1 status=noxfer", LINUX_SD_DEV_FILE);
		status = system(format_cmd);
		if (status < 0 || (status >> 8)) {
			PR_ERR("[sd format] format error :%s : %s\n", LINUX_SD_DEV_FILE, strerror(errno));
			return -2001;
		}

		snprintf(format_cmd, 256, "echo -e \"o\nn\np\n1\n\n\nw\" | fdisk %s > /dev/null", LINUX_SD_DEV_FILE);
		status = system(format_cmd);
		if (status < 0 || (status >> 8)) {
			PR_ERR("[sd format] format error :%s : %s\n", LINUX_SD_DEV_FILE, strerror(errno));
			return -2001;
		}
	}

	/*format sdcard*/
	if (gmount_failed) {
		snprintf(format_cmd, 256, "%s %s;", FORMAT_CMD, LINUX_SD_DEV_FILE "p1");
	} else {
		snprintf(format_cmd, 256, "%s %s;", FORMAT_CMD, s_mmcblk_name);
	}

	status = system(format_cmd);
	if (status < 0 || (status >> 8)) {
		PR_ERR("[sd format] format error :%s : %s\n", s_mmcblk_name, strerror(errno));
		return -2001;
	}

	system("sync");

	/*mount sdcard*/
	if (gmount_failed) {
		strcpy(name, LINUX_SD_DEV_FILE "p1");
		snprintf(format_cmd, 256, "export ACTION=add;export MDEV=%s;/lib/mdev/automount.sh sdcard;"
		                          "unset ACTION;unset MDEV;",
		         &name[5]);
	} else {
		ptr = strstr(s_mmcblk_name, "/dev/");
		if (!ptr) {
			PR_ERR("[sd format] mount error\n");
			return -2001;
		}
		snprintf(format_cmd, 256, "export ACTION=add;export MDEV=%s;/lib/mdev/automount.sh sdcard;"
		                          "unset ACTION;unset MDEV;",
		         (char *)(ptr + 5));
	}

	status = system(format_cmd);
	if (status < 0 || (status >> 8)) {
		PR_ERR("[sd format] mount error: %s\n", strerror(errno));
		return -2001;
	}

	PR_NOTICE("sd format end\n");

	return 0;
}

//Implementation Interface for Remounting
VOID tuya_ipc_sd_remount(VOID)
{
	CHAR_T format_cmd[128] = { 0 };
	char buffer[512] = { 0 };
	E_SD_STATUS status = SD_STATUS_UNKNOWN;

	status = tuya_ipc_sd_get_status();
	if (SD_STATUS_NORMAL == status) {
		PR_DEBUG("sd don't need to remount!\n");
		return;
	}
	PR_NOTICE("remount_sd_card ..... \n");

	snprintf(format_cmd, 128, "export ACTION=add;/lib/mdev/automount.sh sdcard;unset ACTION");
	FILE *pp = popen(format_cmd, "r");
	if (NULL != pp) {
		fgets(buffer, sizeof(buffer), pp);
		PR_INFO("%s\n", buffer);
		pclose(pp);
	} else {
		PR_INFO("remount_sd_card failed\n");
	}
}

STREAM_STORAGE_WRITE_MODE_E tuya_ipc_sd_get_mode_config(VOID)
{
	TUYA_AG_CONF_S *conf = NULL;
	AGTX_LOCAL_RECORD_CONF_S *record = NULL;

	AG_Get_Conf(&conf);

	record = &conf->local_record.data;

	//PR_DEBUG("format %d enable %d mode %d / %d\n", gformating, record->enabled, record->mode,
	//         tuya_ipc_ss_get_write_mode());

	if (gformating) {
		return SS_WRITE_MODE_NONE;
	}

	if (record->enabled) {
		if (record->mode == AGTX_RECORD_MODE_event) {
			return SS_WRITE_MODE_EVENT;
		} else if (record->mode == AGTX_RECORD_MODE_continuous) {
			return SS_WRITE_MODE_ALL;
		} else {
			return SS_WRITE_MODE_EVENT;
		}
	} else {
		return SS_WRITE_MODE_NONE;
	}
}
int check_sdcard_filesystem()
{
	char tmp[256] = { 0 };
	char name[256] = { 0 };

	if (0 == access(LINUX_SD_DEV_FILE "p1", F_OK)) //Default node name information
	{
		strcpy(name, LINUX_SD_DEV_FILE "p1");
		sprintf(tmp, " blkid %s", name);

		FILE *fp = popen(tmp, "r");
		if (!fp) {
			/*sdcard not found.*/
			return 0;
		}

		while (fgets(tmp, sizeof(tmp), fp) != NULL) {
			if (NULL != strstr(tmp, "vfat")) {
				pclose(fp);
				return 0;
			}
		}

		pclose(fp);
	}

	strcpy(name, LINUX_SD_DEV_FILE);
	sprintf(tmp, "blkid %s", name);
	FILE *fp = popen(tmp, "r");
	if (!fp) {
		/*sdcard not found.*/
		return 0;
	}

	while (fgets(tmp, sizeof(tmp), fp) != NULL) {
		if (NULL != strstr(tmp, "vfat")) {
			pclose(fp);
			return 0;
		}
	}

	pclose(fp);

	/*Filesystem type is not FAT32*/
	return -1;
}
//Implementation Interface for Obtaining SD Card Status
E_SD_STATUS tuya_ipc_sd_get_status(VOID)
{
	CHAR_T search_for[64];

	FILE *fp = fopen(LINUX_SD_DEV_FILE, "rb");
	if (!fp) {
		return SD_STATUS_NOT_EXIST;
	}
	fclose(fp);

	//	strcpy(s_mmcblk_name, LINUX_SD_DEV_FILE);
	//	if (0 == access(LINUX_SD_DEV_FILE "p1", F_OK)) //Default node name information
	//	{
	//		strcat(s_mmcblk_name, "p1");
	//	}

	fp = fopen(LINUX_MOUNT_INFO_FILE, "rb");
	if (fp) {
		memset(s_mounts_info, 0, sizeof(s_mounts_info));
		fread(s_mounts_info, 1, MAX_MOUNTS_INFO_LEN, fp);
		fclose(fp);
		CHAR_T *mmcblk_name_start = strstr(s_mounts_info, LINUX_SD_DEV_FILE);
		sprintf(search_for, " %s", IPC_APP_SD_BASE_PATH);
		CHAR_T *mmcblk_name_end = strstr(s_mounts_info, search_for);
		if (mmcblk_name_start && mmcblk_name_end) {
			int mmcblk_name_len = mmcblk_name_end - mmcblk_name_start;
			if (mmcblk_name_len >= MAX_MMC_NAME_LEN) {
				PR_DEBUG("Unknown device name.\n");
				gmount_failed = 1;
				return SD_STATUS_ABNORMAL;
			}
			strncpy(s_mmcblk_name, mmcblk_name_start, mmcblk_name_len);
			s_mmcblk_name[mmcblk_name_len] = '\0';
		}
		//There are device nodes but no mount information. Generally, the card format is incorrect and report abnormal.
		else {
			if (check_sdcard_filesystem() < 0) {
				gmount_failed = 1;
				return SD_STATUS_ABNORMAL;
			} else {
				return SD_STATUS_NOT_EXIST;
			}
		}
		//If the mount information of the SD card is not at the end and there is a ro mount behind it, there will be a problem.
		if (NULL != strstr(mmcblk_name_start, "ro,")) {
			return SD_STATUS_ABNORMAL;
		}
		if (NULL == strstr(mmcblk_name_start, "vfat")) {
			return SD_STATUS_ABNORMAL;
		}
		if (access(s_mmcblk_name, 0)) {
			return SD_STATUS_ABNORMAL;
		}

		/*MAX pre-recordtime 10 sec*/
		if (tuya_ipc_ss_set_pre_record_time(gpre_record_time) != OPRT_OK) {
			PR_DEBUG("Set pre record time Fail.\n");
		}
		/*MAX pre-recordtime 600 sec*/
		tuya_ipc_ss_set_max_event_duration(600);

		gmount_failed = 0;

		return SD_STATUS_NORMAL;
	} else {
		return SD_STATUS_UNKNOWN;
	}
}

//SD card capacity acquisition interface, unit: KB
VOID tuya_ipc_sd_get_capacity(UINT_T *p_total, UINT_T *p_used, UINT_T *p_free)
{
	*p_total = 0;
	*p_used = 0;
	*p_free = 0;

	struct statfs sd_fs;
	if (statfs(IPC_APP_SD_BASE_PATH, &sd_fs) != 0) {
		PR_ERR("statfs failed!/n");
		return;
	}

	*p_total = (UINT_T)(((UINT64_T)sd_fs.f_blocks * (UINT64_T)sd_fs.f_bsize) >> 10);
	*p_used = (UINT_T)((((UINT64_T)sd_fs.f_blocks - (UINT64_T)sd_fs.f_bfree) * (UINT64_T)sd_fs.f_bsize) >> 10);
	*p_free = (UINT_T)(((UINT64_T)sd_fs.f_bavail * (UINT64_T)sd_fs.f_bsize) >> 10);
	//PR_DEBUG("sd capacity: total: %d KB, used %d KB, free %d KB\n", *p_total, *p_used, *p_free);
	return;
}

//get the path of mounting sdcard
CHAR_T *tuya_ipc_get_sd_mount_path(VOID)
{
	return IPC_APP_SD_BASE_PATH;
}

VOID TUYA_APP_Update_Sd_Parameter()
{
	TUYA_AG_CONF_S *conf = NULL;
	AGTX_LOCAL_RECORD_CONF_S *record = NULL;

	AG_Get_Conf(&conf);

	record = &conf->local_record.data;

	if (record->enabled) {
		if (record->mode == AGTX_RECORD_MODE_event) {
			tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_EVENT);
		} else if (record->mode == AGTX_RECORD_MODE_continuous) {
			tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
		} else {
			tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_EVENT);
		}
	} else {
		tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
	}

	gpre_record_time = record->pre_record_time;

	PR_NOTICE("pre %d enable %d mode %d\n", gpre_record_time, record->enabled, record->mode);
}

//The maximum number of events per day, exceeding this value, there will be an exception when playback and can not play.
//Too much setting of this value will affect the query efficiency
#define MAX_EVENT_NUM_PER_DAY (8640)
extern IPC_MEDIA_INFO_S s_media_info;

OPERATE_RET TUYA_APP_Init_Stream_Storage(IN CONST CHAR_T *p_sd_base_path)
{
	STATIC BOOL_T s_stream_storage_inited = FALSE;
	if (s_stream_storage_inited == TRUE) {
		PR_ERR("The Stream Storage Is Already Inited");
		return OPRT_OK;
	}

	PR_DEBUG("Init Stream_Storage SD:%s", p_sd_base_path);
	OPERATE_RET ret = tuya_ipc_ss_init((CHAR_T *)p_sd_base_path, &s_media_info, MAX_EVENT_NUM_PER_DAY, NULL);
	if (ret != OPRT_OK) {
		PR_ERR("Init Main Video Stream_Storage Fail. %d", ret);
		return OPRT_COM_ERROR;
	}

	TUYA_APP_Update_Sd_Parameter();

	return OPRT_OK;
}
