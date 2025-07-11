/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tz_update.c - library for update timezone
 * Copyright (C) 2018-2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 * * Brief: library for update timezone
 * *
 * * Author: Henry Liu <henry.liu@augentix.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <sys/inotify.h>

#define TZ_FILE_PATH "/etc/TZ"
#define DST_ENABLE_PATH "/usrdata/active_setting/timeMode.conf"

pthread_t threadUpdateTimeZone;
int isDst = 0;

static void getTz(void)
{
	FILE *fp_tz;
	char buffer[128];

	fp_tz = fopen(TZ_FILE_PATH, "r");
	if (fp_tz) {
		fgets(buffer, 128, fp_tz);
		fclose(fp_tz);
		setenv("TZ", buffer, 1);
		tzset();
	}
}

int getDst(void)
{
	FILE *fp_DST;
	char buffer[128];

	fp_DST = fopen(DST_ENABLE_PATH, "rb");
	if(fp_DST) {
		while(fgets(buffer, 128, fp_DST)!=NULL) {
			if( sscanf(buffer, "DST_enabled=%s", buffer)!= 0 ) {
				isDst = atoi(buffer);
			}
		}
	}
	fclose(fp_DST);
	return isDst;
}

int setDst(int enabled)
{
	char buffer[128];
	sprintf(buffer, "/system/bin/setDST_en.sh setDST %d", enabled);
	if (system(buffer) != 0) {
		return -1;
	}
	return 0;
}

int setTimeinfo(int DSTenabled, char *TZ)
{
	char buffer[128];
	sprintf(buffer, "echo -n \"%s\" > /etc/TZ", TZ);
	system(buffer);
	setenv("TZ", TZ, 1);
	tzset();

	if (setDst(DSTenabled) != 0){
		return -1;
	}
	return 0;
}

static void cleanupTimeZone(int *desc)
{
	int fd = desc[0];
	int wd = desc[1];

	inotify_rm_watch(fd, wd);
	close(fd);
}

static int updateTimeZone(void)
{
	char buffer[128];
	int fd = -1;
	int wd = -1;
	fd_set readfds;
	int ret = 0;
	int desc[2];

	getTz();

	fd = inotify_init();
	assert(fd != -1);

	wd = inotify_add_watch(fd, TZ_FILE_PATH, IN_CLOSE_WRITE);
	assert(wd != -1);

	desc[0] = fd;
	desc[1] = wd;
	pthread_cleanup_push(cleanupTimeZone, desc);

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		ret = select(fd + 1, &readfds, 0, 0, NULL);
		if (ret == -1) {
			continue;
		}
		ret = read(fd, buffer, 128);
		if (ret <= 0) {
			continue;
		}
		getTz();
	}

	pthread_cleanup_pop(1);
	return 0;
}

int enableTzUpdate(void)
{
	return pthread_create(&threadUpdateTimeZone, NULL, (void *)updateTimeZone, NULL);
}

int disableTzUpdate(void)
{
	int ret = 0;

	ret = pthread_cancel(threadUpdateTimeZone);
	if (ret < 0) {
		return ret;
	}

	ret = pthread_join(threadUpdateTimeZone, NULL);
	return ret;
}
