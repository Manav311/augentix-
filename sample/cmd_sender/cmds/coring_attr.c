#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_CORING_ATTR (1 + 1 * MPI_ISO_LUT_ENTRY_NUM + 1 + 1)

static INT32 GET(CoringAttr)(CMD_DATA_S *opt)
{
	return MPI_getCoringAttr(opt->path_idx, opt->data);
}

static INT32 SET(CoringAttr)(const CMD_DATA_S *opt)
{
	return MPI_setCoringAttr(opt->path_idx, opt->data);
}

static void ARGS(CoringAttr)(void)
{
	printf("\t'--coring dev_idx path_idx mode coring_auto.abs_th[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] coring_manual.abs_th coring_slope'\n");
	printf("\t'--coring 0 0 0 20 20 15 10 10 10 10 10 10 10 10 20 1024'\n");
}

static void HELP(CoringAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--coring <MPI_PATH> [CORING_ATTR]'", "Set CORING attributes");
}

static void SHOW(CoringAttr)(const CMD_DATA_S *opt)
{
	MPI_CORING_ATTR_S *attr = (MPI_CORING_ATTR_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("coring_auto.abs_th[%d]=%d\n", i, attr->coring_auto.abs_th[i]);
	}
	printf("coring_manual.abs_th=%d\n", attr->coring_manual.abs_th);
	printf("coring_slope=%d\n", attr->coring_slope);
}

static int PARSE(CoringAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_CORING_ATTR_S *data = (MPI_CORING_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_CORING_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->coring_auto.abs_th[i] = atoi(argv[optind]);
			optind++;
		}

		data->coring_manual.abs_th = atoi(argv[optind]);
		optind++;
		data->coring_slope = atoi(argv[optind]);
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

static CMD_S coring_ops = MAKE_CMD("coring", MPI_CORING_ATTR_S, CoringAttr);

__attribute__((constructor)) void regCoringCmd(void)
{
	CMD_register(&coring_ops);
}