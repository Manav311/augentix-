#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_ENH_ATTR (1 + 11 * MPI_ISO_LUT_ENTRY_NUM + 11)

static INT32 GET(EnhAttr)(CMD_DATA_S *opt)
{
	return MPI_getEnhAttr(opt->path_idx, opt->data);
}

static INT32 SET(EnhAttr)(const CMD_DATA_S *opt)
{
	return MPI_setEnhAttr(opt->path_idx, opt->data);
}

static void ARGS(EnhAttr)(void)
{
	printf("\t'--enh dev_idx path_idx mode enh_auto.y_txr_strength[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.y_txr_edge[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.y_txr_detail[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]\n");
	printf("\t'      enh_auto.y_zone_strength[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.y_zone_edge[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.y_zone_detail[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.y_zone_radius[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.y_zone_weight[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]\n");
	printf("\t'      enh_auto.c_radius[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.c_strength[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] enh_auto.c_edge[0 ~ MPI_ISO_LUT_ENTRY_NUM-1]\n");
	printf("\t'      enh_manual.y_txr_strength enh_manual.y_txr_edge enh_manual.y_txr_detail\n");
	printf("\t'      enh_manual.y_zone_strength enh_manual.y_zone_edge enh_manual.y_zone_detail enh_manual.y_zone_radius enh_manual.y_zone_weight\n");
	printf("\t'      enh_manual.c_radius enh_manual.c_strength enh_manual.c_edge'\n");
	printf("\t'--enh 0 0 0 512 512 512 256 256 128 128 256 512 1023 1023 32 32 32 32 32 32 32 16 16 16 16 30 30 30 30 28 24 20 -4 -8 -12 -16\n");
	printf("\t               256 256 128 64 64 64 128 512 1023 1023 1023 0 0 0 0 0 0 0 16 16 32 32 10 10 10 10 10 8 5 -4 -8 -12 -16 0 0 0 0 1 1 1 2 2 2 2 0 0 0 0 0 0 0 0 0 0 0\n");
	printf("\t               0 0 1 1 2 2 2 3 3 3 3 32 64 125 256 512 512 512 1023 1023 1023 1023 32 32 16 16 16 16 8 4 0 0 0\n");
	printf("\t               1023 32 -8 1023 32 -16 2 0 2 1023 0'\n");
}

static void HELP(EnhAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--enh <MPI_PATH> [ENH_ATTR]'", "Set ENH attributes");
}

static void SHOW(EnhAttr)(const CMD_DATA_S *opt)
{
	MPI_ENH_ATTR_S *attr = (MPI_ENH_ATTR_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_txr_strength[%d]=%d\n", i, attr->enh_auto.y_txr_strength[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_txr_edge[%d]=%d\n", i, attr->enh_auto.y_txr_edge[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_txr_detail[%d]=%d\n", i, attr->enh_auto.y_txr_detail[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_zone_strength[%d]=%d\n", i, attr->enh_auto.y_zone_strength[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_zone_edge[%d]=%d\n", i, attr->enh_auto.y_zone_edge[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_zone_detail[%d]=%d\n", i, attr->enh_auto.y_zone_detail[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_zone_radius[%d]=%d\n", i, attr->enh_auto.y_zone_radius[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.y_zone_weight[%d]=%d\n", i, attr->enh_auto.y_zone_weight[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.c_radius[%d]=%d\n", i, attr->enh_auto.c_radius[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.c_strength[%d]=%d\n", i, attr->enh_auto.c_strength[i]);
	}
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("enh_auto.c_edge[%d]=%d\n", i, attr->enh_auto.c_edge[i]);
	}
	printf("enh_manual.y_txr_strength=%d\n", attr->enh_manual.y_txr_strength);
	printf("enh_manual.y_txr_edge=%d\n", attr->enh_manual.y_txr_edge);
	printf("enh_manual.y_txr_detail=%d\n", attr->enh_manual.y_txr_detail);
	printf("enh_manual.y_zone_strength=%d\n", attr->enh_manual.y_zone_strength);
	printf("enh_manual.y_zone_edge=%d\n", attr->enh_manual.y_zone_edge);
	printf("enh_manual.y_zone_detail=%d\n", attr->enh_manual.y_zone_detail);
	printf("enh_manual.y_zone_radius=%d\n", attr->enh_manual.y_zone_radius);
	printf("enh_manual.y_zone_weight=%d\n", attr->enh_manual.y_zone_weight);
	printf("enh_manual.c_radius=%d\n", attr->enh_manual.c_radius);
	printf("enh_manual.c_strength=%d\n", attr->enh_manual.c_strength);
	printf("enh_manual.c_edge=%d\n", attr->enh_manual.c_edge);
}

static int PARSE(EnhAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_ENH_ATTR_S *data = (MPI_ENH_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_ENH_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_txr_strength[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_txr_edge[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_txr_detail[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_zone_strength[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_zone_edge[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_zone_detail[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_zone_radius[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.y_zone_weight[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.c_radius[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.c_strength[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->enh_auto.c_edge[i] = atoi(argv[optind]);
			optind++;
		}

		data->enh_manual.y_txr_strength = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_txr_edge = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_txr_detail = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_zone_strength = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_zone_edge = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_zone_detail = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_zone_radius = atoi(argv[optind]);
		optind++;
		data->enh_manual.y_zone_weight = atoi(argv[optind]);
		optind++;
		data->enh_manual.c_radius = atoi(argv[optind]);
		optind++;
		data->enh_manual.c_strength = atoi(argv[optind]);
		optind++;
		data->enh_manual.c_edge = atoi(argv[optind]);
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

static CMD_S enh_ops = MAKE_CMD("enh", MPI_ENH_ATTR_S, EnhAttr);

__attribute__((constructor)) void regEnhCmd(void)
{
	CMD_register(&enh_ops);
}