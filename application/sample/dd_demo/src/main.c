/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 * dd (decoder & display) demo is sample code of the integration of ffmpeg decoder and augentix display.
 *
 */
#include <signal.h>
#include <stdio.h>
#include <pthread.h>

#include "dd_proc.h"

/**
 * Variables
 */
// static pthread_t thread_video_frame_data_id = 0;
// // static pthread_t thread_audio_frame_data_id = 0;

/**
 * Functions
 */
void _printUsage()
{
	printf("Usage: dd_demo {-i LINK/FILE} [-f DISP_INPUT_FMT]");
	printf("\n");
}
#define printUsage()           \
	do {                   \
		_printUsage(); \
		return 1;      \
	} while (0)

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

	stopProc();
}

/**
 * Static Functions
*/

/**
 * Main function
 */
int main(int argc, char *argv[])
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
	printf("**** AGT demo: build at %s %s ****\n\n", __DATE__, __TIME__);

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <TCP/IP address>\n", argv[0]);
		return -1;
	}

	// Process Initial
	printf("Process Initial\n");
	if (initProc(argv[1]) != 0) {
		fprintf(stderr, "initProc fail!\n");
		goto err_proc;
	}

	// Process Run
	printf("Process Run\n");
	if (runProc() != 0) {
		fprintf(stderr, "runProc fail!\n");
		return -1;
	}

err_proc:
	// Process de-initial
	printf("Process de-initial\n");
	if (deinitProc() != 0) {
		fprintf(stderr, "runProc fail!\n");
		return -1;
	}

	return 0;
}
