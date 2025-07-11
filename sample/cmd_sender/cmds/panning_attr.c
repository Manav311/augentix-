#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_PANNING_ATTR (7)

static INT32 GET(WinPanning)(CMD_DATA_S *opt)
{
	return MPI_DEV_getPanningAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinPanning)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setPanningAttr(opt->win_idx, opt->data);
}

static void ARGS(WinPanning)(void)
{
	printf("\t'--winpanning dev_idx chn_idx win_idx enable center_offset.x center_offset.y radius hor_strength ver_strength ldc_ratio'\n");
	printf("\t'--winpanning 0 0 0 1 0 0 0 0 0 0'\n");
}

static void HELP(WinPanning)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winpanning <MPI_WIN> [PANNING_ATTR]'", "Set PANNING attributes");
}

static void SHOW(WinPanning)(const CMD_DATA_S *opt)
{
	MPI_PANNING_ATTR_S *attr = (MPI_PANNING_ATTR_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("enable=%d\n", attr->enable);
	printf("center_offset.x=%d\n", attr->center_offset.x);
	printf("center_offset.y=%d\n", attr->center_offset.y);
	printf("radius=%d\n", attr->radius);
	printf("hor_strength=%d\n", attr->hor_strength);
	printf("ver_strength=%d\n", attr->ver_strength);
	printf("ldc_ratio=%d\n", attr->ldc_ratio);
}

static int PARSE(WinPanning)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_PANNING_ATTR_S *data = (MPI_PANNING_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == NUM_PANNING_ATTR + 3) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		data->enable = atoi(argv[optind++]);

		data->center_offset.x = atoi(argv[optind++]);
		data->center_offset.y = atoi(argv[optind++]);

		data->radius = atoi(argv[optind++]);

		data->hor_strength = atoi(argv[optind++]);

		data->ver_strength = atoi(argv[optind++]);

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

static CMD_S winpanning_ops = MAKE_CMD("winpanning", MPI_PANNING_ATTR_S, WinPanning);

__attribute__((constructor)) void regWinPanningCmd(void)
{
	CMD_register(&winpanning_ops);
}