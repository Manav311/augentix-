#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_AWB_ATTR (13 + 14 * MPI_K_TABLE_ENTRY_NUM + 7 * MPI_K_TABLE_ENTRY_NUM + 4 * MPI_K_TABLE_ENTRY_NUM)

static INT32 GET(AwbAttr)(CMD_DATA_S *opt)
{
	return MPI_getAwbAttr(opt->path_idx, opt->data);
}

static INT32 SET(AwbAttr)(const CMD_DATA_S *opt)
{
	return MPI_setAwbAttr(opt->path_idx, opt->data);
}

static void ARGS(AwbAttr)(void)
{
	printf("\t'--awb dev_idx path_idx speed wht_density r_extra_gain b_extra_gain g_extra_gain wht_weight gwd_weight color_tolerance max_lum_gain k_table_valid_size low_k high_k over_exp_th ccm_domain k_table[0].k\n");
	printf("\t       k_table[0].gain[0 ~ MPI_AWB_CHN_NUM-1] k_table[0].matrix[0 ~ MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM-1]...delta_table[0].gain[0 ~ MPI_K_TABLE_ENTRY_NUM-1]...\n");
	printf("\t       bias_table[0].k bias_table[0].color_tolerance_bias bias_table[0].wht_weight_bias bias_table[0].gwd_weight_bias bias_table[0].r_extra_gain_bias bias_table[0].g_extra_gain_bias bias_table[0].b_extra_gain_bias... \n");
	printf("\t       delta_table[0].gain[0 ~ MPI_AWB_CHN_NUM-1]...'\n");
	printf("\t'--awb 0 0 64 0 128 128 128 64 96 32 90 4 2700 8000 13000 13000 0 2700 256 162 520 255 3124 -735 -341 -1334 3723 -340 712 -4365 5700 0 0 0 0 4150 256 229 400 255 3776 -1820 -91 -1135 3285 -102 496 -2508 4059 0 0 0 0 6500 256 252 261 255 3282 -1398 -163 -790 3464 -626 499 -2248 3796 0 0 0 0 8000 256 277 248 255 3314 -1400 134 -677 3085 -360 248 -1701 3501 0 0 0 0 0 256 256 256 256 2048 0 0 0 2048 0 0 0 2048 0 0 0 0 0 256 256 256 256 2048 0 0 0 2048 0 0 0 2048 0 0 0 0 0 256 256 256 256 2048 0 0 0 2048 0 0 0 2048 0 0 0 0 0 256 256 256 256 2048 0 0 0 2048 0 0 0 2048 0 0 0 0 2700 0 0 0 0 0 0 4150 0 0 0 0 0 0 6500 0 0 0 0 0 0 8000 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'\n");
}

static void HELP(AwbAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--awb <MPI_PATH> [AWB_ATTR]'", "Set AWB attributes");
}

static void SHOW(AwbAttr)(const CMD_DATA_S *opt)
{
	MPI_AWB_ATTR_S *attr = (MPI_AWB_ATTR_S *)opt->data;
	int i, j;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("speed=%d\n", attr->speed);
	printf("wht_density=%d\n", attr->wht_density);
	printf("r_extra_gain=%d\n", attr->r_extra_gain);
	printf("b_extra_gain=%d\n", attr->b_extra_gain);
	printf("g_extra_gain=%d\n", attr->g_extra_gain);
	printf("wht_weight=%d\n", attr->wht_weight);
	printf("gwd_weight=%d\n", attr->gwd_weight);
	printf("color_tolerance=%d\n", attr->color_tolerance);
	printf("max_lum_gain=%d\n", attr->max_lum_gain);
	printf("k_table_valid_size=%d\n", attr->k_table_valid_size);
	printf("low_k=%d\n", attr->low_k);
	printf("high_k=%d\n", attr->high_k);
	printf("over_exp_th=%d\n", attr->over_exp_th);
	printf("ccm_domain=%d\n", attr->ccm_domain);

	for (i = 0; i < MPI_K_TABLE_ENTRY_NUM; ++i) {
		printf("k_table[%d].k=%d\n", i, attr->k_table[i].k);

		for (j = 0; j < MPI_AWB_CHN_NUM; ++j) {
			printf("k_table[%d].gain[%d]=%d\n", i, j, attr->k_table[i].gain[j]);
		}

		for (j = 0; j < MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM; ++j) {
			printf("k_table[%d].matrix[%d]=%d\n", i, j, attr->k_table[i].matrix[j]);
		}

		for (j = 0; j < MPI_AWB_CHN_NUM; ++j) {
			printf("delta_table[%d].gain[%d]=%d\n", i, j, attr->delta_table[i].gain[j]);
		}
	}

	for (i = 0; i < MPI_K_TABLE_ENTRY_NUM; ++i) {
		printf("bias_table[%d].k=%d\n", i, attr->bias_table[i].k);
		printf("bias_table[%d].color_tolerance_bias=%d\n", i, attr->bias_table[i].color_tolerance_bias);
		printf("bias_table[%d].wht_weight_bias=%d\n", i, attr->bias_table[i].wht_weight_bias);
		printf("bias_table[%d].gwd_weight_bias=%d\n", i, attr->bias_table[i].gwd_weight_bias);
		printf("bias_table[%d].r_extra_gain_bias=%d\n", i, attr->bias_table[i].r_extra_gain_bias);
		printf("bias_table[%d].g_extra_gain_bias=%d\n", i, attr->bias_table[i].g_extra_gain_bias);
		printf("bias_table[%d].b_extra_gain_bias=%d\n", i, attr->bias_table[i].b_extra_gain_bias);
	}
}

static int PARSE(AwbAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_AWB_ATTR_S *data = (MPI_AWB_ATTR_S *)opt->data;
	int i, j, num = argc - optind;

	if (num == (NUM_AWB_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->speed = atoi(argv[optind]);
		optind++;
		data->wht_density = atoi(argv[optind]);
		optind++;
		data->r_extra_gain = atoi(argv[optind]);
		optind++;
		data->b_extra_gain = atoi(argv[optind]);
		optind++;
		data->g_extra_gain = atoi(argv[optind]);
		optind++;
		data->wht_weight = atoi(argv[optind]);
		optind++;
		data->gwd_weight = atoi(argv[optind]);
		optind++;
		data->color_tolerance = atoi(argv[optind]);
		optind++;
		data->max_lum_gain = atoi(argv[optind]);
		optind++;
		data->k_table_valid_size = atoi(argv[optind]);
		optind++;
		data->low_k = atoi(argv[optind]);
		optind++;
		data->high_k = atoi(argv[optind]);
		optind++;
		data->over_exp_th = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_K_TABLE_ENTRY_NUM; ++i) {
			data->k_table[i].k = atoi(argv[optind]);
			optind++;

			for (j = 0; j < MPI_AWB_CHN_NUM; ++j) {
				data->k_table[i].gain[j] = atoi(argv[optind]);
				optind++;
			}
			for (j = 0; j < MPI_COLOR_CHN_NUM * MPI_COLOR_CHN_NUM; ++j) {
				data->k_table[i].matrix[j] = atoi(argv[optind]);
				optind++;
			}
			for (j = 0; j < MPI_AWB_CHN_NUM; ++j) {
				data->delta_table[i].gain[j] = atoi(argv[optind]);
				optind++;
			}
		}

		for (i = 0; i < MPI_K_TABLE_ENTRY_NUM; ++i) {
			data->bias_table[i].k = atoi(argv[optind]);
			optind++;
			data->bias_table[i].color_tolerance_bias = atoi(argv[optind]);
			optind++;
			data->bias_table[i].wht_weight_bias = atoi(argv[optind]);
			optind++;
			data->bias_table[i].gwd_weight_bias = atoi(argv[optind]);
			optind++;
			data->bias_table[i].r_extra_gain_bias = atoi(argv[optind]);
			optind++;
			data->bias_table[i].g_extra_gain_bias = atoi(argv[optind]);
			optind++;
			data->bias_table[i].b_extra_gain_bias = atoi(argv[optind]);
			optind++;
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

static CMD_S awb_ops = MAKE_CMD("awb", MPI_AWB_ATTR_S, AwbAttr);

__attribute__((constructor)) void regAwbCmd(void)
{
	CMD_register(&awb_ops);
}
