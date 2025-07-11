#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_DMS_ATTR (1 + 3 * MPI_ISO_LUT_ENTRY_NUM + 3 + 1)

static INT32 GET(DmsAttr)(CMD_DATA_S *opt)
{
	return MPI_getDmsAttr(opt->path_idx, opt->data);
}

static INT32 SET(DmsAttr)(const CMD_DATA_S *opt)
{
	return MPI_setDmsAttr(opt->path_idx, opt->data);
}

static void ARGS(DmsAttr)(void)
{
	printf("\t'--dms dev_idx path_idx mode dms_auto.g_at_m_inter_ratio[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]'\n");
	printf("\t'dms_auto.m_at_m_inter_ratio[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] dms_auto.m_at_g_inter_ratio[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]'\n");
	printf("\t'dms_manual_g_at_m_inter_ratio dms_manual_m_at_m_inter_ratio dms_manual_m_at_g_inter_ratio'\n");
	printf("\t'dms_ctrl_method'\n");
}

static void HELP(DmsAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dms <MPI_PATH> [DMS_ATTR]'", "Set DMS attributes");
}

static void SHOW(DmsAttr)(const CMD_DATA_S *opt)
{
	MPI_DMS_ATTR_S *attr = (MPI_DMS_ATTR_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d\n", attr->mode);

	printf("dms_auto.g_at_m_inter_ratio=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->dms_auto.g_at_m_inter_ratio[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("dms_auto.m_at_m_inter_ratio=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->dms_auto.m_at_m_inter_ratio[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("dms_auto.m_at_g_inter_ratio=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->dms_auto.m_at_g_inter_ratio[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}

	printf("dms_manual.g_at_m_inter_ratio=%d\n", attr->dms_manual.g_at_m_inter_ratio);
	printf("dms_manual.m_at_m_inter_ratio=%d\n", attr->dms_manual.m_at_m_inter_ratio);
	printf("dms_manual.m_at_g_inter_ratio=%d\n", attr->dms_manual.m_at_g_inter_ratio);
	printf("dms_ctrl_method=%d\n", attr->dms_ctrl_method);
}

static int PARSE(DmsAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_DMS_ATTR_S *data = (MPI_DMS_ATTR_S *)opt->data;
	MPI_getDmsAttr(opt->path_idx, data);
	int num = argc - optind;
	int i;

	if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;
		return 0;
	}

	if (num == 2 + NUM_DMS_ATTR) {
		opt->action = CMD_ACTION_SET;
		data->mode = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->dms_auto.g_at_m_inter_ratio[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->dms_auto.m_at_m_inter_ratio[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->dms_auto.m_at_g_inter_ratio[i] = atoi(argv[optind]);
			optind++;
		}
		data->dms_manual.g_at_m_inter_ratio = atoi(argv[optind]);
		optind++;
		data->dms_manual.m_at_m_inter_ratio = atoi(argv[optind]);
		optind++;
		data->dms_manual.m_at_g_inter_ratio = atoi(argv[optind]);
		optind++;
		data->dms_ctrl_method = atoi(argv[optind]);
		optind++;
	}
	return 0;
}

static CMD_S dms_ops = MAKE_CMD("dms", MPI_DMS_ATTR_S, DmsAttr);

__attribute__((constructor)) void regDmsCmd(void)
{
	CMD_register(&dms_ops);
}
