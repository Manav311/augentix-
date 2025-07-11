#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <syslog.h>
#include <pthread.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <time.h>

#include "tutk_define.h"
#include "tutk_sdmonit.h"

#include "log_define.h"

extern int gTime_;

#define SD_CARD_PATH SD_MOUNT_PATH "/record"
#define MAX_FILE_LEN 64
#define MAX_STR_LEN 256
#define MAX_MOUNT_INFO_LEN 1024
#define MAX_DATA_LEN 128 * 1024
#define SD_REMAIN_SIZE 100 /*Mega byte*/
#define RECORD_STOP_MIN_SIZE 20 /*Mega byte*/

int n_time_, gDelayInterval, gNotify;
static char out_of_mem = 0;
static char error_format = 0;
static char sd_inserted = 0;
static char bootUp_CheckSD = 0;
static char sd_formating = 0;
static char sd_broken = 0;
char sd_format = 0;
char sd_name[MAX_FILE_LEN] = { 0 };
char mount_info[MAX_MOUNT_INFO_LEN] = { 0 };

typedef enum {
	AG_SD_STATUS_READY,
	AG_SD_STATUS_UNKNOWN,
	AG_SD_STATUS_NONE,
	AG_SD_STATUS_NUM,
} AG_SD_STATUS;

typedef struct _record_ctrl_s {
	int curr_sd_status;
	int prev_sd_status;
	int sd_free;
} record_ctrl_s;

char gmount_info[MAX_MOUNT_INFO_LEN] = { 0 };
record_ctrl_s g_record_ctrl = { 0 };

extern bool gProgressRun;
/* TUTK P2P status*/
extern int gP2Pstate;

int TUTK_check_sdard_format(void)
{
	char tmp[16] = { 0 };
	FILE *fp;
	char type[8] = { 0 };

	fp = popen("blkid -i /dev/mmcblk0p1", "r");
	if (fp == NULL) {
		return -1;
	}

	while (fgets(tmp, sizeof(tmp), fp) != NULL) {
		tutkservice_log_debug("blkid --> %s\n", tmp);
		char *p = strstr(tmp, "TYPE=");
		if (p != NULL) {
			sscanf(p + strlen("TYPE="), "%s", type);
			tutkservice_log_debug("Type --> %s\n", type);
			break;
		}
	}

	if (strcmp(type, "\"vfat\"") == 0) {
		sd_format = SD_FORMAT_VFAT;
	} else if (strcmp(type, "\"ntfs\"") == 0) {
		sd_format = SD_FORMAT_NTFS;
	} else {
		sd_format = SD_FORMAT_UNKOWN;
	}

	tutkservice_log_info("sd_format = %d\n", sd_format);

	pclose(fp);

	return sd_format;
}

int TUTK_check_sdard_mount(void)
{
	char tmp[8] = { 0 };
	FILE *fp;
	int status = 0;

	fp = popen("cat /tmp/plog | tail -1 |  awk '{print $1}'", "r");
	if (fp == NULL) {
		return -1;
	}

	while (fgets(tmp, sizeof(tmp), fp) != NULL) {
		tutkservice_log_debug("status --> %s\n", tmp);
	}

	if (strcmp(tmp, "Mount") == 0) {
		status = 0x01; //Mount success
	} else if (strcmp(tmp, "Unmount") == 0) {
		status = 0x02; //UnMount success
	} else if (strcmp(tmp, "Failed") == 0) {
		status = 0x03; //Failed to Mount
	} else {
		status = 0x04; //Unknown status
	}

	tutkservice_log_info("sd_mount = %d\n", status);

	pclose(fp);

	return status;
}

int TUTK_check_sdard_usage()
{
	struct statfs sd_fs;

	if (statfs(SD_MOUNT_PATH, &sd_fs) != 0) {
		tutkservice_log_err("statfs failed!/n");
		return -1;
	}
	return (int)(((unsigned long long)sd_fs.f_bavail * (unsigned long long)sd_fs.f_bsize) >> 20);
}

int TUTK_check_sdcard_filesystem()
{
#define MAX_FILE_LEN 64
#define MAX_MOUNT_INFO_LEN 1024
	FILE *fp = fopen(LINUX_MOUNT_INFO, "rb");

	if (fp) {
		int len = fread(mount_info, 1, MAX_MOUNT_INFO_LEN, fp);
		fclose(fp);

		if (len >= MAX_MOUNT_INFO_LEN - 1) {
			goto error;
		}

		mount_info[len] = '\0';

		/* close file pointer after read */
		char *mmcblk_name_start = strstr(mount_info, LINUX_SD_DEV_FILE);
		if (!mmcblk_name_start) {
			tutkservice_log_err("failed to find device node\n");
			goto error;
		}

		char *mmcblk_name_end = strstr(mmcblk_name_start, SD_MOUNT_PATH);

		if (mmcblk_name_end) {
			int mmcblk_name_len = mmcblk_name_end - mmcblk_name_start;
			if (mmcblk_name_len >= MAX_FILE_LEN) {
				goto error;
			}
			strncpy(sd_name, mmcblk_name_start, mmcblk_name_len);
			sd_name[mmcblk_name_len - 1] = '\0';
		} else {
			// There are device nodes but no mount information. Generally,
			// the card format is incorrect and report abnormal.
			tutkservice_log_err("failed to find mount place\n");
			goto error;
		}

		if (access(sd_name, F_OK) != 0) {
			perror("invalid access node");
			goto error;
		}

		// If the mount information of the SD card is not at the end and
		// there is a ro mount behind it, there will be a problem.
		if (NULL != strstr(mmcblk_name_start, "ro,")) {
			tutkservice_log_err("invalid read-only mount\n");
			goto error;
		}
	} else {
		goto error;
	}

	return 0;

error:
	return -1;
}

int TUTK_get_sdcard_status()
{
	/*Check if boot up checking SD card status*/
	if ((bootUp_CheckSD == 0) && (gP2Pstate == 0)) {
		bootUp_CheckSD = 1;
		tutkservice_log_debug("Boot up check SD card !!!\n");
	}

	if ((access("/dev/mmcblk0p1", F_OK) != -1) && (access("/dev/mmcblk0", F_OK) != -1)) {
		sd_broken = 0; //reset
		//Card exist

		//if already check SD Card format, return result directly
		if (sd_format == SD_FORMAT_VFAT) {
			return AG_SD_STATUS_READY;
		} else if (error_format && (sd_format != SD_FORMAT_VFAT)) {
			return AG_SD_STATUS_UNKNOWN;
		}

		//Check SD card format
		if (TUTK_check_sdard_format() == SD_FORMAT_VFAT) {
			error_format = 0;
			return AG_SD_STATUS_READY;
		} else {
			tutkservice_log_err("Wrong SD card format !!!\n");
			if (gNotify && (error_format == 0)) {
				/* Send TUTK Notification */
				if (nebula_device_push_notification(NULL, 303) == 0) { //non Fat32 format
					error_format = 1;
				}
			}
			return AG_SD_STATUS_UNKNOWN;
		}

	} else if (access("/dev/mmcblk0", F_OK) != -1) {
		//There is a broken/damage card or unformatted card
		error_format = 0; //reset

		//if already check SD Card broken or not
		if (sd_broken) {
			return AG_SD_STATUS_UNKNOWN;
		}

		//Check SD Card mount status
		if (TUTK_check_sdard_mount() > 0x02) {
			tutkservice_log_err("SD card damage or unformat !!!\n");
			if (gNotify && (sd_broken == 0)) {
				/* Send TUTK Notification */
				if (nebula_device_push_notification(NULL, 304) == 0) { //SD card damage or unformat
					sd_broken = 1;
				}
			}
			return AG_SD_STATUS_UNKNOWN;
		}
	} else {
		//no card
		error_format = 0; //reset
		sd_broken = 0; //reset
		/* Check sdcard device node exists or not */
		if (0 != access(LINUX_SD_DEV_FILE, F_OK)) {
			if ((bootUp_CheckSD == 1) && gP2Pstate && ((access(DEFAULT_DEVICE_SETTING, F_OK) != -1))) {
				tutkservice_log_err("Boot up is without SD card !!!\n");
				bootUp_CheckSD = 2;
				if (gNotify) {
					nebula_device_push_notification(NULL, 305); //to express boot up without SD card
				}
			}
			return AG_SD_STATUS_NONE;
		}
	}

	/* remaining condition is sdcard errors */
	tutkservice_log_err("remaining condition is sdcard errors !!!\n");
	return AG_SD_STATUS_UNKNOWN;
}

void sighandler(int signo, siginfo_t *info, void *ctx)
{
	(void)(ctx);

	if (signo == SIGUSR1) {
		gTime_ = n_time_; //reset timer
		gDelayInterval = info->si_int;
		printf("img->delay_level = %d\n", gDelayInterval);
	} else if ((signo == SIGUSR2) && (info->si_int == 2588)) {
		gNotify = 1;
		printf("Got signal = %d\n", info->si_int);
	} else if ((signo == SIGUSR2) && (info->si_int == 2589)) {
		gNotify = 0;
		printf("Got signal = %d\n", info->si_int);
	}
}

void *thread_sd_monit(void *arg)
{
	(void)(arg);

	int free = 0;
	record_ctrl_s *rec_ctrl = &g_record_ctrl;

	struct sigaction act;
	act.sa_sigaction = sighandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;
	if (0 != sigaction(SIGUSR1, &act, NULL)) {
		perror("sigaction () failed installing SIGUSR1 handler");
	} else if (0 != sigaction(SIGUSR2, &act, NULL)) {
		perror("sigaction () failed installing SIGUSR2 handler");
	}

	while (gProgressRun) {
		if (gNotify && (gP2Pstate == 1) && (sd_formating == 0)) {
			//Check status
			rec_ctrl->prev_sd_status = rec_ctrl->curr_sd_status;
			rec_ctrl->curr_sd_status = TUTK_get_sdcard_status();

			if ((rec_ctrl->prev_sd_status != AG_SD_STATUS_READY) &&
			    (rec_ctrl->curr_sd_status == AG_SD_STATUS_READY)) {
				/* Send TUTK Notification */
				tutkservice_log_info("SD card insert !!!\n");
				sd_inserted = 1;
				nebula_device_push_notification(NULL, 301);
			} else if ((rec_ctrl->prev_sd_status != AG_SD_STATUS_NONE) &&
			           (rec_ctrl->curr_sd_status == AG_SD_STATUS_NONE)) {
				/* Send TUTK Notification */
				tutkservice_log_err("SD card remove !!!\n");

				nebula_device_push_notification(NULL, 302);
				int status = system("reboot -f");
				if (status < 0) {
					tutkservice_log_err("SD card remove fail, error: %s\n", strerror(errno));
				} else {
					tutkservice_log_err("SD card remove will be reboot...\n");
				}
			}

			if (rec_ctrl->curr_sd_status == AG_SD_STATUS_READY) {
				if ((free = TUTK_check_sdard_usage()) < 0) {
					free = 0;
					usleep(5000 * 1000);
					continue;
				}

				if (free < SD_REMAIN_SIZE) {
					if (sd_inserted) {
						tutkservice_log_err("Sd_inserted and Space is running out !!!\n");
						nebula_device_push_notification(NULL, 305);
						sd_inserted = 0;
					}

					if (free < RECORD_STOP_MIN_SIZE) {
						/* Notify sdcard no space */
						tutkservice_log_err("Space is running out !!!\n");
						if (out_of_mem) {
							nebula_device_push_notification(NULL, 305);
							out_of_mem = 0;
						}
					}
				} else {
					out_of_mem = 1;
					if (sd_inserted) {
						sd_inserted = 0;
					}
				}
			}

			if ((bootUp_CheckSD == 1) &&
			    ((access("/usrdata/active_setting/tutk/device_settings.txt", F_OK) != -1))) {
				bootUp_CheckSD = 2; //already checking boot up SD card status, no need to check again...
				tutkservice_log_info("bootUp_CheckSD is checked!!!\n");
			}
		} else {
			if ((rec_ctrl->curr_sd_status != AG_SD_STATUS_NONE) &&
			    (TUTK_get_sdcard_status() == AG_SD_STATUS_NONE)) {
				int status = system("reboot -f");
				if (status < 0) {
					tutkservice_log_err("SD card remove fail, error: %s\n", strerror(errno));
				} else {
					tutkservice_log_err("SD card remove will be reboot...\n");
				}
			}
		}
		usleep(1000 * 1000);
	}

	return NULL;
}

int TUTK_sd_monit_init()
{
	pthread_t sd_monit_thread;
	record_ctrl_s *rec_ctrl = &g_record_ctrl;

	rec_ctrl->curr_sd_status = AG_SD_STATUS_NONE;
	rec_ctrl->prev_sd_status = AG_SD_STATUS_NONE;

	//Get initial status
	//If there is any status changed, thread_sd_monit will report
	rec_ctrl->prev_sd_status = rec_ctrl->curr_sd_status;
	rec_ctrl->curr_sd_status = TUTK_get_sdcard_status();

	pthread_create(&sd_monit_thread, NULL, thread_sd_monit, NULL);
	pthread_detach(sd_monit_thread);

	return 0;
}