#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_ROI (4)

static INT32 GET(WinRoi)(CMD_DATA_S *opt)
{
	return MPI_DEV_getWindowRoi(opt->win_idx, opt->data);
}

static INT32 SET(WinRoi)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setWindowRoi(opt->win_idx, opt->data);
}

static void ARGS(WinRoi)(void)
{
	printf("\t'--winroi dev_idx chn_idx win_idx x y width height'\n");
	printf("\t'--winroi 0 0 0 0 0 1024 1024'\n");
}

static void HELP(WinRoi)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winroi <MPI_WIN> [ROI_ATTR]'", "Set Window ROI Attribute");
}

static void SHOW(WinRoi)(const CMD_DATA_S *opt)
{
	MPI_RECT_S *winroi = (MPI_RECT_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("roi{x=%d, y=%d, w=%d, h=%d}\n", winroi->x, winroi->y, winroi->width, winroi->height);
}

static int PARSE(WinRoi)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_RECT_S *roi = (MPI_RECT_S *)opt->data;
	MPI_CHN_LAYOUT_S attr;
	int num = argc - optind;
	int ret = 0;

	if (num == (NUM_ROI + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		roi->x = atoi(argv[optind]);
		optind++;
		roi->y = atoi(argv[optind]);
		optind++;
		roi->width = atoi(argv[optind]);
		optind++;
		roi->height = atoi(argv[optind]);
		optind++;
	} else if (num == 3) {
		opt->action = CMD_ACTION_GET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		ret = MPI_DEV_getChnLayout(MPI_VIDEO_CHN(opt->win_idx.dev, opt->win_idx.chn), &attr);
		if (ret != 0) {
			return ret;
		}

		if (opt->win_idx.win >= attr.window_num) {
			return -EACCES;
		}
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S winroi_ops = MAKE_CMD("winroi", MPI_RECT_S, WinRoi);

__attribute__((constructor)) void regWinRoiCmd(void)
{
	CMD_register(&winroi_ops);
}
