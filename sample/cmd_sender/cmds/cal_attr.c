#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_CAL_ATTR (4)

static INT32 GET(CalAttr)(CMD_DATA_S *opt)
{
	return MPI_getCalAttr(opt->path_idx, opt->data);
}

static INT32 SET(CalAttr)(const CMD_DATA_S *opt)
{
	return MPI_setCalAttr(opt->path_idx, opt->data);
}

static void ARGS(CalAttr)(void)
{
	printf("\t'--cal dev_idx path_idx cal_en dbc_en dcc_en lsc_en'\n");
	printf("\t'--cal 0 0 1 1 1 1'\n");
}

static void HELP(CalAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--cal <MPI_PATH> [CAL_ATTR]'", "Set CAL attributes");
}

static void SHOW(CalAttr)(const CMD_DATA_S *opt)
{
	MPI_CAL_ATTR_S *attr = (MPI_CAL_ATTR_S *)opt->data;
	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("cal_en=%d\n", attr->cal_en);
	printf("dbc_en=%d\n", attr->dbc_en);
	printf("dcc_en=%d\n", attr->dcc_en);
	printf("lsc_en=%d\n", attr->lsc_en);
}

static int PARSE(CalAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_CAL_ATTR_S *data = (MPI_CAL_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_CAL_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->cal_en = atoi(argv[optind]);
		optind++;
		data->dbc_en = atoi(argv[optind]);
		optind++;
		data->dcc_en = atoi(argv[optind]);
		optind++;
		data->lsc_en = atoi(argv[optind]);
		optind++;
	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S cal_ops = MAKE_CMD("cal", MPI_CAL_ATTR_S, CalAttr);

__attribute__((constructor)) void regCalCmd(void)
{
	CMD_register(&cal_ops);
}
