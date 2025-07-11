/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <stdio.h>
#include "logfile.h"

/**
 * Variables
 */
static char _cwd[128] = { 0 };

/**
 * Functions
 */
void _printUsage()
{
	printf("Usage: demo {-u UUID} [-g] [-v N]");
	printf("\n");
	printf("-g\treport gps position\n");
	printf("-v N\tlog level\n");
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

	TGC_stopProc();
}

/**
 * Main function
 */
int main(int argc, char *argv[])
{
	int ret;
	int opt;
	char short_opts[64] = "u:v:g";
	const char *option;
	struct option longopts[] = { { "only-ai", no_argument, 0, 1 }, { NULL, 0, 0, 0 } };
	int opt_idx = 0;
	int log_to_console = 0;
	int log_lvl = -1;
	char *_uuid_ = NULL;

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

	while ((opt = getopt_long(argc, argv, short_opts, longopts, &opt_idx)) >= 0) {
		switch (opt) {
		case 'u':
			_uuid_ = optarg;
			break;
		case 'g':
			log_to_console = atoi(optarg);
			break;
		case 'v':
			log_lvl = atoi(optarg);
			break;
		default:
			break;
		}
	}

	// getcwd(_cwd, sizeof(_cwd));
	// Tange cloud loading local configuration process
	sprintf(_cwd, "/usrdata/root");
	TGC_loadConfig(_cwd);

	// "UUID" is necessary to Tange Cloud SDK.
	if (!_uuid_) {
		_uuid_ = (char *)TGC_getSetting("uuid");
		if (!_uuid_) {
			printUsage();
			return 0;
		}
	}

	// Get information from the configuration.
	// It should be run after "TGC_loadConfig".
	if (log_lvl == (-1)) {
		if ((option = (char *)TGC_getSetting("loglevel")))
			log_lvl = atoi(option);
	}

	if (log_lvl >= 0) {
		LogfSetLevel(log_lvl);
		TciSetLogLevel(log_lvl);
	}

	if (log_to_console) {
		static char log_path[] = "/tmp/icam365.log";
		TciSetLogOption(log_to_console, log_path, 512 << 10);
	}

	// Process Initial
	printf("Process Initial\n");
	if (TGC_initProc(_uuid_) != 0) {
		goto init_fail;
	}

	// Process Run
	printf("Process Run\n");
	TGC_runProc();

init_fail:
	// Process de-initial
	printf("Process de-initial\n");
	TGC_deinitProc();

	LogI("Reboot Now !!\n");
	system("reboot");

	return 0;
}
