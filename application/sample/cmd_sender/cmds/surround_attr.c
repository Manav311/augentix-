#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_SURROUND_ATTR (7)

static INT32 GET(WinSurround)(CMD_DATA_S *opt)
{
	return MPI_DEV_getSurroundAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinSurround)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setSurroundAttr(opt->win_idx, opt->data);
}

static void ARGS(WinSurround)(void)
{
	printf("\t'--winsurr dev_idx chn_idx win_idx enable center_offset.x center_offset.y rotate min_radius max_radius ldc_ratio'\n");
	printf("\t'--winsurr 0 0 0 1 0 0 0 0 0 0'\n");
}

static void HELP(WinSurround)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winsurr <MPI_WIN> [SURROUND_ATTR]'", "Set Surround attributes");
}

static char *getRotateName(MPI_ROTATE_TYPE_E rotate)
{
	switch (rotate) {
	case MPI_ROTATE_0:
		return "MPI_ROTATE_0";
	case MPI_ROTATE_90:
		return "MPI_ROTATE_90";
	case MPI_ROTATE_180:
		return "MPI_ROTATE_180";
	case MPI_ROTATE_270:
		return "MPI_ROTATE_270";
	default:
		break;
	}
	return NULL;
}

static void SHOW(WinSurround)(const CMD_DATA_S *opt)
{
	MPI_SURROUND_ATTR_S *attr = (MPI_SURROUND_ATTR_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("enable=%d\n", attr->enable);
	printf("center_offset.x=%d\n", attr->center_offset.x);
	printf("center_offset.y=%d\n", attr->center_offset.y);
	printf("rotate=%d (%s)\n", attr->rotate, getRotateName(attr->rotate));
	printf("min_radius=%d\n", attr->min_radius);
	printf("max_radius=%d\n", attr->max_radius);
	printf("ldc_ratio=%d\n", attr->ldc_ratio);
}

static int PARSE(WinSurround)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_SURROUND_ATTR_S *data = (MPI_SURROUND_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == NUM_SURROUND_ATTR + 3) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		data->enable = atoi(argv[optind++]);

		data->center_offset.x = atoi(argv[optind++]);
		data->center_offset.y = atoi(argv[optind++]);

		data->rotate = atoi(argv[optind++]);

		data->min_radius = atoi(argv[optind++]);

		data->max_radius = atoi(argv[optind++]);

		data->ldc_ratio = atoi(argv[optind]);
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

static CMD_S winsurround_ops = MAKE_CMD("winsurr", MPI_SURROUND_ATTR_S, WinSurround);

__attribute__((constructor)) void regWinSurroundCmd(void)
{
	CMD_register(&winsurround_ops);
}