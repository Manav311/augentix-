#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_WINFPS_ATTR (1)

static INT32 GET(WinFpsAttr)(CMD_DATA_S *opt)
{
	return MPI_DEV_getWindowAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinFpsAttr)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setWindowAttr(opt->win_idx, opt->data);
}

static void ARGS(WinFpsAttr)(void)
{
	printf("\t'--winfps dev_idx chn_idx win_idx fps'\n");
	printf("\t'--winfps 0 0 0 20'\n");
}

static void HELP(WinFpsAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winfps <MPI_WIN> [WIN_ATTR]'", "Set window fps");
}

static void SHOW(WinFpsAttr)(const CMD_DATA_S *opt)
{
	MPI_WIN_ATTR_S *attr = (MPI_WIN_ATTR_S *)opt->data;

	PRINT_WIN(opt->win_idx);
	printf("fps=%f\n", attr->fps);
}

static int PARSE(WinFpsAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_WIN_ATTR_S *data = (MPI_WIN_ATTR_S *)opt->data;
	int num = argc - optind;
	int ret;

	if (num == (NUM_WINFPS_ATTR + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		if ((ret = MPI_DEV_getWindowAttr(opt->win_idx, data))) {
			return ret;
		}

		data->fps = atof(argv[optind++]);

	} else if (num == 3) {
		opt->action = CMD_ACTION_GET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S winfps_ops = MAKE_CMD("winfps", MPI_WIN_ATTR_S, WinFpsAttr);

__attribute__((constructor)) void regWinFpsCmd(void)
{
	CMD_register(&winfps_ops);
}
