#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_CSM_ATTR (172)

static INT32 GET(CsmAttr)(CMD_DATA_S *opt)
{
	return MPI_getCsmAttr(opt->path_idx, opt->data);
}

static INT32 SET(CsmAttr)(const CMD_DATA_S *opt)
{
	return MPI_setCsmAttr(opt->path_idx, opt->data);
}

static void ARGS(CsmAttr)(void)
{
	printf("\t--csm dev_idx path_idx bw_en hue_angle mode\n");
	printf("\t       csm_auto.saturation[0..10] csm_manual.saturation\n");
	printf("\t       cst_color.coeff[0..8] cst_color.offset[0..2]\n");
	printf("\t       cst_bw.coeff[0..8] cst_bw.offset[0..2]\n");
	printf("\t       cst_auto_en\n");
	printf("\t       cst_auto[0..10].coeff[0..8] cst_auto[0..10].offset[0..2]\n");
	printf("\t--csm 0 0 0 0 0\n");
	printf("\t       160 150 140 128 85 85 85 85 85 85 85 128\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       0 2048 0 0 0 0 0 0 0 0 512 512\n");
	printf("\t       0\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
	printf("\t       435 1465 148 -240 -807 1047 1047 -951 -96 0 512 512\n");
}

static void HELP(CsmAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--csm [MPI_PATH] [CSM_ATTR]'", "Set CSM attributes");
}

static void SHOW(CsmAttr)(const CMD_DATA_S *opt)
{
	MPI_CSM_ATTR_S *attr = (MPI_CSM_ATTR_S *)opt->data;
	int i;
	int j;
	const int matrix_entry_num = MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("bw_en=%d\n", attr->bw_en);
	printf("hue_angle=%d\n", attr->hue_angle);
	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("csm_auto.saturation[%d]=%d\n", i, attr->csm_auto.saturation[i]);
	}
	printf("csm_manual.saturation=%d\n", attr->csm_manual.saturation);
	for (i = 0; i < matrix_entry_num; i++) {
		printf("cst_color.coeff[%d]=%d\n", i, (int)attr->cst_color.coeff[i]);
	}
	for (i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		printf("cst_color.offset[%d]=%d\n", i, (int)attr->cst_color.offset[i]);
	}
	for (i = 0; i < matrix_entry_num; i++) {
		printf("cst_bw.coeff[%d]=%d\n", i, (int)attr->cst_bw.coeff[i]);
	}
	for (i = 0; i < MPI_COLOR_CHN_NUM; i++) {
		printf("cst_bw.offset[%d]=%d\n", i, (int)attr->cst_bw.offset[i]);
	}
	printf("cst_auto_en=%d\n", attr->cst_auto_en);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		for (j = 0; j < matrix_entry_num; j++) {
			printf("cst_auto[%d].coeff[%d]=%d\n", i, j, (int)attr->cst_auto[i].coeff[j]);
		}
		for (j = 0; j < MPI_COLOR_CHN_NUM; j++) {
			printf("cst_auto[%d].offset[%d]=%d\n", i, j, (int)attr->cst_auto[i].offset[j]);
		}
	}
}

static int PARSE(CsmAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_CSM_ATTR_S *data = (MPI_CSM_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;
	int j;
	const int matrix_entry_num = MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM;

	if (num == (NUM_CSM_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->bw_en = atoi(argv[optind]);
		optind++;
		data->hue_angle = atoi(argv[optind]);
		optind++;
		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->csm_auto.saturation[i] = atoi(argv[optind]);
			optind++;
		}

		data->csm_manual.saturation = atoi(argv[optind]);
		optind++;

		for (i = 0; i < matrix_entry_num; ++i) {
			data->cst_color.coeff[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_COLOR_CHN_NUM; ++i) {
			data->cst_color.offset[i] = atoi(argv[optind]);
			optind++;
		}

		for (i = 0; i < matrix_entry_num; ++i) {
			data->cst_bw.coeff[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_COLOR_CHN_NUM; ++i) {
			data->cst_bw.offset[i] = atoi(argv[optind]);
			optind++;
		}

		data->cst_auto_en = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
			for (j = 0; j < matrix_entry_num; j++) {
				data->cst_auto[i].coeff[j] = atoi(argv[optind]);
				optind++;
			}
			for (j = 0; j < MPI_COLOR_CHN_NUM; j++) {
				data->cst_auto[i].offset[j] = atoi(argv[optind]);
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

static CMD_S csm_ops = MAKE_CMD("csm", MPI_CSM_ATTR_S, CsmAttr);

__attribute__((constructor)) void regCsmCmd(void)
{
	CMD_register(&csm_ops);
}
