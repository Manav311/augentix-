#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h> // for write
#include "getopt.h"
#include "errno.h"
#include "signal.h"

#include "core.h"
#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_enc.h"

#include "log_define.h"
#include "handlers.h"
#include "nodes.h"

int g_run_flag = 0;
bool g_no_iva_flag = false;
bool g_no_image_preference_flag = false;
bool g_no_video_control_flag = false;

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

int main(void)
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
	g_run_flag = 1;

	if (MPI_SYS_init() != MPI_SUCCESS) {
		avmain2_log_err("Initialize system failed.");
		return -ENXIO;
	}
	HANDLERS_allReadDb(TMP_ACTIVE_DB);

	NODE_initIva();
	NODE_startIva();

	avmain2_log_info("system start");

	while (g_run_flag) {
		sleep(2);
	}

	NODE_stopIva();
	NODE_exitIva();

	avmain2_log_info("exit iva");

	if (MPI_SYS_exit() != MPI_SUCCESS) {
		avmain2_log_err("Exit system failed.");
		return -ENXIO;
	}

	return 0;
}