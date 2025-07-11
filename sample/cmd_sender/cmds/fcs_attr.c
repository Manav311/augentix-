#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_FCS_ATTR (1 + 3 * MPI_ISO_LUT_ENTRY_NUM + 3)

static INT32 GET(FcsAttr)(CMD_DATA_S *opt)
{
	return MPI_getFcsAttr(opt->path_idx, opt->data);
}

static INT32 SET(FcsAttr)(const CMD_DATA_S *opt)
{
	return MPI_setFcsAttr(opt->path_idx, opt->data);
}

static void ARGS(FcsAttr)(void)
{
	printf("\t'--fcs dev_idx path_idx mode fcs_auto.strength[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] fcs_auto.threshold[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] fcs_auto.offset[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]\n");
	printf("\t'      fcs_manual.strength fcs_manual.threshold fcs_manual.offset\n");
	printf("\t'--fcs 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0\n");
}

static void HELP(FcsAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--fcs <MPI_PATH> [FCS_ATTR]'", "Set FCS attributes");
}

static void SHOW(FcsAttr)(const CMD_DATA_S *opt)
{
	MPI_FCS_ATTR_S *attr = (MPI_FCS_ATTR_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d (0: auto, 2: manual)\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("fcs_auto.strength[%d]=%d\n", i, attr->fcs_auto.strength[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("fcs_auto.threshold[%d]=%d\n", i, attr->fcs_auto.threshold[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("fcs_auto.offset[%d]=%d\n", i, attr->fcs_auto.offset[i]);
	}
	printf("fcs_manual.strength=%d\n", attr->fcs_manual.strength);
	printf("fcs_manual.threshold=%d\n", attr->fcs_manual.threshold);
	printf("fcs_manual.offset=%d\n", attr->fcs_manual.offset);
}

static int PARSE(FcsAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_FCS_ATTR_S *data = (MPI_FCS_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_FCS_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->fcs_auto.strength[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->fcs_auto.threshold[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->fcs_auto.offset[i] = atoi(argv[optind]);
			optind++;
		}

		data->fcs_manual.strength = atoi(argv[optind]);
		optind++;
		data->fcs_manual.threshold = atoi(argv[optind]);
		optind++;
		data->fcs_manual.offset = atoi(argv[optind]);
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

static CMD_S fcs_ops = MAKE_CMD("fcs", MPI_FCS_ATTR_S, FcsAttr);

__attribute__((constructor)) void regFcsCmd(void)
{
	CMD_register(&fcs_ops);
}