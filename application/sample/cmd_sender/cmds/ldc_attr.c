#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_LDC_ATTR (5)

static INT32 GET(WinLdc)(CMD_DATA_S *opt)
{
	return MPI_DEV_getLdcAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinLdc)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setLdcAttr(opt->win_idx, opt->data);
}

static void ARGS(WinLdc)(void)
{
	printf("\t'--winldc dev_idx chn_idx win_idx enable view_type center_offset.x center_offset.y ratio'\n");
	printf("\t'--winldc 0 0 0 1 0 0 0 0'\n");
}

static void HELP(WinLdc)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winldc <MPI_WIN> [LDC_ATTR]'", "Set LDC attributes");
}

static void SHOW(WinLdc)(const CMD_DATA_S *opt)
{
	MPI_LDC_ATTR_S *attr = (MPI_LDC_ATTR_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("enable=%d\n", attr->enable);
	printf("view_type=%d\n", attr->view_type);
	printf("center_offset.x=%d\n", attr->center_offset.x);
	printf("center_offset.y=%d\n", attr->center_offset.y);
	printf("ratio=%d\n", attr->ratio);
}

static int PARSE(WinLdc)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_LDC_ATTR_S *data = (MPI_LDC_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_LDC_ATTR + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		data->enable = atoi(argv[optind]);
		optind++;

		data->view_type = atoi(argv[optind]);
		optind++;

		data->center_offset.x = atoi(argv[optind]);
		optind++;

		data->center_offset.y = atoi(argv[optind]);
		optind++;

		data->ratio = atoi(argv[optind]);
		optind++;
	} else if (num == 3) {
		opt->action = CMD_ACTION_GET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S winldc_ops = MAKE_CMD("winldc", MPI_LDC_ATTR_S, WinLdc);

__attribute__((constructor)) void regWinLdcCmd(void)
{
	CMD_register(&winldc_ops);
}
