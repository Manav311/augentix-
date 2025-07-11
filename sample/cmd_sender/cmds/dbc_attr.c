#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_DBC_ATTR (3 + 4 + 4 * 11)

static INT32 GET(DbcAttr)(CMD_DATA_S *opt)
{
	return MPI_getDbcAttr(opt->path_idx, opt->data);
}

static INT32 SET(DbcAttr)(const CMD_DATA_S *opt)
{
	return MPI_setDbcAttr(opt->path_idx, opt->data);
}

static void ARGS(DbcAttr)(void)
{
	printf("\t'--dbc dev_idx path_idx mode dbc_level type dbc_manual.manual.black_level[0 ~ MPI_DBC_CHN_NUM] dbc_auto.auto_table[0 ~ MPI_ISO_LUT_ENTRY_NUM].black_level[0 ~ MPI_DBC_CHN_NUM]'\n");
	printf("\t'--dbc 0 0 2 3845 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'\n");
}

static void HELP(DbcAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dbc <MPI_PATH> [DBC_ATTR]'", "Set DBC attributes");
}

static void SHOW(DbcAttr)(const CMD_DATA_S *opt)
{
	int i = 0;
	int j = 0;
	MPI_DBC_ATTR_S *attr = (MPI_DBC_ATTR_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d\n", attr->mode);
	printf("dbc_level=%d\n", attr->dbc_level);
	printf("type=%d\n", attr->type);
	for (i = 0; i < MPI_DBC_CHN_NUM; i++) {
		printf("dbc_manual.manual.black_level[%d] = %d\n", i, attr->dbc_manual.manual.black_level[i]);
	}

	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		for (j = 0; j < MPI_DBC_CHN_NUM; j++) {
			printf("dbc_auto.auto_table[%d].black_level[%d] = %d\n", i, j,
			       attr->dbc_auto.auto_table[i].black_level[j]);
		}
	}
}

static int PARSE(DbcAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_DBC_ATTR_S *data = (MPI_DBC_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_DBC_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;
		data->dbc_level = atoi(argv[optind]);
		optind++;

		data->type = atoi(argv[optind]);
		optind++;
		data->dbc_manual.manual.black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_manual.manual.black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_manual.manual.black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_manual.manual.black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[0].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[0].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[0].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[0].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[1].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[1].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[1].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[1].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[2].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[2].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[2].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[2].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[3].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[3].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[3].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[3].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[4].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[4].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[4].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[4].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[5].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[5].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[5].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[5].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[6].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[6].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[6].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[6].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[7].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[7].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[7].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[7].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[8].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[8].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[8].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[8].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[9].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[9].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[9].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[9].black_level[3] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[10].black_level[0] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[10].black_level[1] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[10].black_level[2] = atoi(argv[optind]);
		optind++;
		data->dbc_auto.auto_table[10].black_level[3] = atoi(argv[optind]);
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

static CMD_S dbc_ops = MAKE_CMD("dbc", MPI_DBC_ATTR_S, DbcAttr);

__attribute__((constructor)) void regDbcCmd(void)
{
	CMD_register(&dbc_ops);
}