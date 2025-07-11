#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_WIN_ATTR (2)

static INT32 GET(WinMirrorFlip)(CMD_DATA_S *opt)
{
	return MPI_DEV_getWindowAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinMirrorFlip)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setWindowAttr(opt->win_idx, opt->data);
}

static void ARGS(WinMirrorFlip)(void)
{
	printf("\t'--win dev_idx chn_idx win_idx mirr_en flip_en'\n");
	printf("\t'--win 0 0 0 1 1'\n");
}

static void HELP(WinMirrorFlip)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winmirrorflip <MPI_WIN> [WIN_ATTR]'", "Set window mirror and flip");
}

static void SHOW(WinMirrorFlip)(const CMD_DATA_S *opt)
{
	MPI_WIN_ATTR_S *attr = (MPI_WIN_ATTR_S *)opt->data;

	PRINT_WIN(opt->win_idx);
	printf("mirror=%d, flip=%d\n", attr->mirr_en, attr->flip_en);
}

static int PARSE(WinMirrorFlip)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_WIN_ATTR_S *data = (MPI_WIN_ATTR_S *)opt->data;
	int num = argc - optind;
	int ret = 0;

	if (num == (NUM_WIN_ATTR + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		if ((ret = MPI_DEV_getWindowAttr(opt->win_idx, data))) {
			return ret;
		}

		data->mirr_en = atoi(argv[optind++]);
		data->flip_en = atoi(argv[optind++]);

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

static CMD_S winmirrorflip_ops = MAKE_CMD("winmirrorflip", MPI_WIN_ATTR_S, WinMirrorFlip);

__attribute__((constructor)) void regWinMirrorFlipCmd(void)
{
	CMD_register(&winmirrorflip_ops);
}