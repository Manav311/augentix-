#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_WB_INFO (18)

static INT32 GET(WbAttr)(CMD_DATA_S *opt)
{
	return MPI_queryWhiteBalanceInfo(opt->path_idx, opt->data);
}

static INT32 SET(WbAttr)(const CMD_DATA_S *opt __attribute__((unused)))
{
	return -ENOSYS;
}

static void ARGS(WbAttr)(void)
{
	printf("\t'--wb dev_idx path_idx'\n");
	printf("\t'--wb 0 0'\n");
}

static void HELP(WbAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--wb <MPI_PATH>'", "Query internal white balance status information");
}

static void SHOW(WbAttr)(const CMD_DATA_S *opt)
{
	MPI_WHITE_BALANCE_INFO_S *attr = (MPI_WHITE_BALANCE_INFO_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);

	for (i = 0; i < MPI_AWB_CHN_NUM; ++i) {
		printf("gain0[%d] = %d\n", i, attr->gain0[i]);
	}

	for (i = 0; i < MPI_AWB_CHN_NUM; ++i) {
		printf("gain1[%d] = %d\n", i, attr->gain1[i]);
	}

	for (i = 0; i < MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM; ++i) {
		printf("matrix[%d] = %d\n", i, attr->matrix[i]);
	}

	printf("color_temp = %d\n", attr->color_temp);
}

static int PARSE(WbAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;

	if (num != 2) {
		return -EINVAL;
	}

	opt->action = CMD_ACTION_GET;
	opt->path_idx.dev = atoi(argv[optind]);
	optind++;
	opt->path_idx.path = atoi(argv[optind]);
	optind++;

	return 0;
}

static CMD_S wb_ops = MAKE_CMD("wb", MPI_WHITE_BALANCE_INFO_S, WbAttr);

__attribute__((constructor)) void regWbCmd(void)
{
	CMD_register(&wb_ops);
}
