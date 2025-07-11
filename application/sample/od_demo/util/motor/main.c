/**
 * @file main.c
 * @brief Application binds motor and GMV OD algorithm.
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <fcntl.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "mpi_dev.h"
#include "mpi_iva.h"
#include "mpi_sys.h"

#include "log.h"
#include "motor.h"

// Store the original config
static struct termios *stdin_config = NULL;
static Controller g_ctx = {
	.curr_state = MOTOR_IDLE,
	.delay = 7,
	.tty_node = "/dev/ttyUSB0",
	.baud = 2400,
};

static int setStdinAttributes(void)
{
	struct termios tty;

	if (tcgetattr(STDIN_FILENO, &tty) != 0) {
		return -errno;
	}

	if (fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK)) {
		return -errno;
	}

	if (stdin_config == NULL) {
		stdin_config = malloc(sizeof(*stdin_config));
		*stdin_config = tty;
	}

	tty.c_cflag |= CLOCAL;
	tty.c_iflag &= ~IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN] = 1;
	tty.c_cc[VTIME] = 0;

	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &tty) != 0) {
		return -errno;
	}

	return 0;
}

static int resetStdinAttributes(void)
{
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, stdin_config) != 0) {
		return -errno;
	}

	free(stdin_config);
	stdin_config = NULL;

	return 0;
}

void loop(void)
{
	UINT32 timestamp;
	int ret;
	enum motor_state cmd;
	char c;

	ret = setStdinAttributes();
	if (ret) {
		log_err("Set stdin attributes failure. %s", strerror(-ret));
	}

	while (g_ctx.curr_state != MOTOR_EXIT) {
		ret = MPI_DEV_waitWin(g_ctx.idx, &timestamp, 0);
		if (ret) {
			break;
		}

		// Non-blocking read, disabled signal chars from stdin.
		// See setStdinAttributes() for details.
		ret = read(STDIN_FILENO, &c, 1);
		if (ret == 0) {
			log_err("Error: read() 0 bytes: %s", strerror(errno));
		} else if (ret == -1) {
			if (errno == EAGAIN) {
				c = '\0';
			} else if (errno == EINTR) {
				break;
			} else {
				log_err("Unexpected error when read(): %d, %s", errno, strerror(errno));
				break;
			}
		}

		if (c == KEY_CTRL_C || c == KEY_CTRL_D) {
			c = KEY_ESC;
		}

		// Perform state transition
		// See fsm.c for transition graph and details implementation
		cmd = Motor_parseCmd(&g_ctx, c);
		state_map[g_ctx.curr_state](&g_ctx, cmd);
	}

	ret = resetStdinAttributes();
	if (ret) {
		log_err("Set stdin attributes failure. %s", strerror(-ret));
	}
}

void describe(void)
{
	printf("How to use:\n"
	       "  When the program is running, user can control the motor via WASD.\n"
	       "   - W / S: Start tilt movement, camera directs to up / down.\n"
	       "   - A / D: Start pan movement, camera directs to left / right.\n"
	       "   - SPACE: Stop motor.\n"
	       "   - ESC  : Stop motor. If motor is still, exit program.\n"
	       "\n");
}

void help(void)
{
	printf("Usage: motor [options]...\n"
	       "Description:\n"
	       "  GL602 control module for OD demonstration.\n"
	       "\n"
	       "Options:\n"
	       "  -c <chn_idx>     Specify which video channel to use. (Default: 0).\n"
	       "  -w <win_idx>     Specify which video window to use. (Default: 0).\n"
	       "  -d <delay>       Delay sending motor stop signal to MPP system.\n"
	       "                   Unit: number of frames. (Default: 7).\n"
	       "  -h --help        Display help message.\n"
	       "  --baud <rate>    Transmission baudrate between device and GL602 (Default: 2400).\n"
	       "  --node <FILE>    Specify device node (Default: /dev/ttyUSB0).\n"
	       "\n"
	       "Example:\n"
	       "  $ mpi_stream -d /system/mpp/case_config/case_config_1001_FHD &\n"
	       "  $ od_demo -i /system/mpp/od_config/od_conf.json &\n"
	       "  $ testOnDemandRTSPServer 0 &\n"
	       "  $ motor\n");
}

int main(int argc, char **argv)
{
	int baud;
	int ret;
	int opt;

	const struct option long_opts[] = { { "baud", required_argument, 0, 'b' },
		                            { "node", required_argument, 0, 'n' },
		                            { "help", no_argument, 0, 'h' },
		                            { 0, 0, 0, 0 } };

	while ((opt = getopt_long(argc, argv, "c:d:w:h", long_opts, NULL)) != -1) {
		switch (opt) {
		case 'c':
			g_ctx.idx.chn = atoi(optarg);
			break;

		case 'd':
			g_ctx.delay = atoi(optarg);
			break;

		case 'b':
			g_ctx.baud = atoi(optarg);
			break;

		case 'n':
			g_ctx.tty_node = optarg;
			break;

		case 'w':
			g_ctx.idx.win = atoi(optarg);
			break;

		case 'h':
			help();
			return EXIT_SUCCESS;

		default:
			return EXIT_FAILURE;
		}
	}

	// Open logger. Currently we use syslog as our logging system.
	openlog(NULL, 0, LOG_LOCAL7);

	if ((baud = toBaudrate(g_ctx.baud)) <= 0) {
		log_err("Expected baudrate (%d) is not supported!", baud);
		return EXIT_FAILURE;
	}

	ret = Motor_init(&g_ctx.motor, g_ctx.tty_node, baud);
	printf("Operating mode: %s\n", (ret) ? "Manual" : "Motor");

	describe();

	// First, initialize MPI system to access video pipeline.
	ret = MPI_SYS_init();
	if (ret) {
		log_err("Failed to initialize MPI system! err: %d", ret);
		return EXIT_FAILURE;
	}

	// Then, modify OD attribute by read-modify-write steps.
	ret = MPI_IVA_getObjParam(g_ctx.idx, &g_ctx.param);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get OD param. err: %d", ret);
		goto err;
	}

	ret = MPI_IVA_getObjMotorParam(g_ctx.idx, &g_ctx.motor_param);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get OD motor param. err: %d", ret);
		goto err;
	}

	loop();

err:

	Motor_exit(&g_ctx.motor);

	MPI_SYS_exit();

	// Close logger.
	closelog();

	return 0;
}
