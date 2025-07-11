/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <stdio.h>
#include <errno.h>

#include <pthread.h>

#include "dd_video_proc.h"
#include "dd_audio_proc.h"
#include "dd_proc.h"

/**
 * Variables
 */
static pthread_t thread_video_frame_data_id = 0;
static pthread_t thread_audio_frame_data_id = 0;

/**
 * Functions
 */

/**
 * Static Functions
*/
static int createStreamoutThread(void)
{
	int ret = 0;

	if ((ret = pthread_create(&thread_video_frame_data_id, NULL, &thread_VideoFrameData, NULL))) {
		fprintf(stderr, "pthread_create ret=%d\n", ret);
		return -1;
	}
	if ((ret = pthread_create(&thread_audio_frame_data_id, NULL, &thread_AudioFrameData, NULL))) {
		fprintf(stderr, "pthread_create ret=%d\n", ret);
		return -1;
	}
	return ret;
}

static int waitUntilThreadTerminate(void)
{
	int ret = 0;

	if ((ret = pthread_join(thread_video_frame_data_id, NULL))) {
		fprintf(stderr, "pthread_join ret=%d\n", ret);
		return -1;
	}
	if ((ret = pthread_join(thread_audio_frame_data_id, NULL))) {
		fprintf(stderr, "pthread_join ret=%d\n", ret);
		return -1;
	}
	return 0;
}

int initProc(void *input_url)
{
	if (video_process_initial(input_url)) {
		perror("video_process_initial fail!!\n");
		return -1;
	}

	if (audio_process_initial(input_url)) {
		perror("audio_process_initial fail!!\n");
		return -1;
	}

	return startProc();
}

int deinitProc(void)
{
	video_process_deinitial();
	audio_process_deinitial();
	return 0;
}

int startProc(void)
{
	startVideoRun();
	return startAudioRun();
}

void stopProc(void)
{
	stopVideoRun();
	stopAudioRun();
}

int runProc(void)
{
	// Create video & audio process threads.
	if (createStreamoutThread()) {
		perror("exit...!!\n");
		return -1;
	}

	if (waitUntilThreadTerminate()) {
		perror("waitUntilThreadTerminate Fail!!\n");
	}
	return 0;
}
