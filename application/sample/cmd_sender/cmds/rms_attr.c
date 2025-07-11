#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_iva_rms.h"

#include "cmd_util.h"

#define NUM_RMS_PARAM (3)

static INT32 GET(RmsAttr)(CMD_DATA_S *opt)
{
	return MPI_IVA_getRegMotSensParam(opt->win_idx, opt->data);
}

static INT32 SET(RmsAttr)(const CMD_DATA_S *opt)
{
	return MPI_IVA_setRegMotSensParam(opt->win_idx, opt->data);
}

static void ARGS(RmsAttr)(void)
{
	printf("\t'--rms dev_idx chn_idx win_idx sen split_x split_y'\n");
	printf("\t'--rms 0 0 0 252 10 5'\n");
}

static void HELP(RmsAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--rms <MPI_WIN> [RMS_ATTR]'", "Set RMS parameters");
}

static void SHOW(RmsAttr)(const CMD_DATA_S *opt)
{
	MPI_IVA_RMS_PARAM_S *attr = (MPI_IVA_RMS_PARAM_S *)opt->data;

	PRINT_WIN(opt->win_idx);
	printf("sen=%d\n", attr->sen);
	printf("split_x = %d\n", attr->split_x);
	printf("split_y = %d\n", attr->split_y);
}

static int PARSE(RmsAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_IVA_RMS_PARAM_S *data = (MPI_IVA_RMS_PARAM_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_RMS_PARAM + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		data->sen = atoi(argv[optind]);
		optind++;
		data->split_x = atoi(argv[optind]);
		optind++;
		data->split_y = atoi(argv[optind]);
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

static CMD_S rms_ops = MAKE_CMD("rms", MPI_IVA_RMS_PARAM_S, RmsAttr);

__attribute__((constructor)) void regRmsCmd(void)
{
	CMD_register(&rms_ops);
}
