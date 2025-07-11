#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_SHP_ATTR (13)

static INT32 GET(ShpAttr)(CMD_DATA_S *opt)
{
	return MPI_getShpAttr(opt->path_idx, opt->data);
}

static INT32 SET(ShpAttr)(const CMD_DATA_S *opt)
{
	return MPI_setShpAttr(opt->path_idx, opt->data);
}

static void ARGS(ShpAttr)(void)
{
	printf("\t'--shp dev_idx path_idx mode shp_auto.sharpness[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] shp_manual.sharpness'\n");
	printf("\t'--shp 0 0 0 255 100 60 0 0 0 0 0 0 0 0 128'\n");
}

static void HELP(ShpAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--shp <MPI_PATH> [SHP_ATTR]'", "Set SHP attributes");
}

static void SHOW(ShpAttr)(const CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_S *attr = (MPI_SHP_ATTR_S *)opt->data;
	int i;

	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("shp_auto.sharpness[%d]=%d\n", i, attr->shp_auto.sharpness[i]);
	}
	printf("shp_manual.sharpness=%d\n", attr->shp_manual.sharpness);
}

static int PARSE(ShpAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_S *data = (MPI_SHP_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_SHP_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->shp_auto.sharpness[i] = atoi(argv[optind]);
			optind++;
		}

		data->shp_manual.sharpness = atoi(argv[optind]);
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

static CMD_S shp_ops = MAKE_CMD("shp", MPI_SHP_ATTR_S, ShpAttr);

__attribute__((constructor)) void regShpCmd(void)
{
	CMD_register(&shp_ops);
}
