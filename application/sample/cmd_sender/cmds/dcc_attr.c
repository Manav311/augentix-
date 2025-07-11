#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_DCC_ATTR (146)

static INT32 GET(DccAttr)(CMD_DATA_S *opt)
{
	return MPI_getDccAttr(opt->path_idx, opt->data);
}

static INT32 SET(DccAttr)(const CMD_DATA_S *opt)
{
	return MPI_setDccAttr(opt->path_idx, opt->data);
}

static void ARGS(DccAttr)(void)
{
	printf("\t'--dcc dev_idx path_idx mode type gain[0~MPI_DCC_CHN_NUM] offset[0~MPI_DCC_CHN_NUM] dcc_manual.gain[0~MPI_DCC_CHN_NUM] dcc_manual.offset[0~MPI_DCC_CHN_NUM] auto_table[0].gain[0~MPI_DCC_CHN_NUM] auto_table[0].offset[0~MPI_DCC_CHN_NUM]'\n");
	printf("\t'--dcc 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0 1024 1024 1024 1024 0 0 0 0'\n");
}

static void HELP(DccAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dcc <MPI_PATH> [DCC_ATTR]'", "Set DCC attributes");
}

static void SHOW(DccAttr)(const CMD_DATA_S *opt)
{
	MPI_DCC_ATTR_S *attr = (MPI_DCC_ATTR_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode = %d\n", attr->mode);
	printf("type = %d\n", attr->type);
	printf("gain[0]=%d, offset[0]=%d\n", attr->gain[0], attr->offset_2s[0]);
	printf("gain[1]=%d, offset[1]=%d\n", attr->gain[1], attr->offset_2s[1]);
	printf("gain[2]=%d, offset[2]=%d\n", attr->gain[2], attr->offset_2s[2]);
	printf("gain[3]=%d, offset[3]=%d\n", attr->gain[3], attr->offset_2s[3]);
	printf("dcc_manual.manual.gain[0] = %d, dcc_manual.manual.offset[0] = %d\n", attr->dcc_manual.manual.gain[0],
	       attr->dcc_manual.manual.offset_2s[0]);
	printf("dcc_manual.manual.gain[1] = %d, dcc_manual.manual.offset[1] = %d\n", attr->dcc_manual.manual.gain[1],
	       attr->dcc_manual.manual.offset_2s[1]);
	printf("dcc_manual.manual.gain[2] = %d, dcc_manual.manual.offset[2] = %d\n", attr->dcc_manual.manual.gain[2],
	       attr->dcc_manual.manual.offset_2s[2]);
	printf("dcc_manual.manual.gain[3] = %d, dcc_manual.manual.offset[3] = %d\n", attr->dcc_manual.manual.gain[3],
	       attr->dcc_manual.manual.offset_2s[3]);

	for (int i = 0; i < MPI_SYS_GAIN_ENTRY_NUM; i++) {
		printf("dcc_auto.auto_table[%d].gain[0] = %d, dcc_auto.auto_table[%d].offset[0] = %d\n", i,
		       attr->dcc_auto.auto_table[i].gain[0], i, attr->dcc_auto.auto_table[i].offset_2s[0]);
		printf("dcc_auto.auto_table[%d].gain[1] = %d, dcc_auto.auto_table[%d].offset[1] = %d\n", i,
		       attr->dcc_auto.auto_table[i].gain[1], i, attr->dcc_auto.auto_table[i].offset_2s[1]);
		printf("dcc_auto.auto_table[%d].gain[2] = %d, dcc_auto.auto_table[%d].offset[2] = %d\n", i,
		       attr->dcc_auto.auto_table[i].gain[2], i, attr->dcc_auto.auto_table[i].offset_2s[2]);
		printf("dcc_auto.auto_table[%d].gain[3] = %d, dcc_auto.auto_table[%d].offset[3] = %d\n", i,
		       attr->dcc_auto.auto_table[i].gain[3], i, attr->dcc_auto.auto_table[i].offset_2s[3]);
	}
}

static int PARSE(DccAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_DCC_ATTR_S *data = (MPI_DCC_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_DCC_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;
		data->type = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_DCC_CHN_NUM; i++) {
			data->gain[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_DCC_CHN_NUM; i++) {
			data->offset_2s[i] = atoi(argv[optind]);
			optind++;
		}

		// manual
		for (i = 0; i < MPI_DCC_CHN_NUM; i++) {
			data->dcc_manual.manual.gain[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_DCC_CHN_NUM; i++) {
			data->dcc_manual.manual.offset_2s[i] = atoi(argv[optind]);
			optind++;
		}

		// auto table
		for (int j = 0; j < MPI_SYS_GAIN_ENTRY_NUM; j++) {
			for (i = 0; i < MPI_DCC_CHN_NUM; ++i) {
				data->dcc_auto.auto_table[j].gain[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_DCC_CHN_NUM; i++) {
				data->dcc_auto.auto_table[j].offset_2s[i] = atoi(argv[optind]);
				optind++;
			}
		}
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

static CMD_S dcc_ops = MAKE_CMD("dcc", MPI_DCC_ATTR_S, DccAttr);

__attribute__((constructor)) void regDccCmd(void)
{
	CMD_register(&dcc_ops);
}