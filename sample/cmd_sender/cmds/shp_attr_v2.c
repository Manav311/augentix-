#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_SIGN_SHP_ATTR                                                                                           \
	(1 + 2 * MPI_ISO_LUT_ENTRY_NUM + 2 + 2 + 2 * MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM +                        \
	 2 * MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM + MPI_ISO_LUT_ENTRY_NUM * 2 * MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM + \
	 MPI_ISO_LUT_ENTRY_NUM * 2 * MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM + MPI_ISO_LUT_ENTRY_NUM + 1 + 3 + 1)
static INT32 GET(ShpAttrV2)(CMD_DATA_S *opt)
{
	return MPI_getShpAttrV2(opt->path_idx, opt->data);
}

static INT32 SET(ShpAttrV2)(const CMD_DATA_S *opt)
{
	return MPI_setShpAttrV2(opt->path_idx, opt->data);
}

static void ARGS(ShpAttrV2)(void)
{
	printf("\t'--shpv2 dev_idx path_idx mode shp_auto_v2.sharpness[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] "
	       "shp_manual_v2.sharpness motion_adaptive_en'\n");
	printf("\t'--shpv2 0 0 0 255 100 60 0 0 0 0 0 0 0 0 0 0 128 0 0 0 0 0 0 0 0 0 0 0 0 0 16 32 416 800 1023 0 16 32 416 "
	       "800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32"
	       "0 16 32 416 800 1023 0 16 32 416 800 1023 0 64 128 256 384 512 640 768 896 960 1023 32 32 32 32 32 32 32 32 32 32 32'\n");
}

static void HELP(ShpAttrV2)(const char *str)
{
	CMD_PRINT_HELP(str, "'--shp <MPI_PATH> [SHP_V2_ATTR]'", "Set SHP_V2 attributes");
}

static void SHOW(ShpAttrV2)(const CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_V2_S *attr = (MPI_SHP_ATTR_V2_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("shp_auto_v2.sharpness[%d]=%d\n", i, attr->shp_auto_v2.sharpness[i]);
	}
	printf("shp_manual_v2.sharpness=%d\n", attr->shp_manual_v2.sharpness);
	printf("motion_adaptive_en=%hhu\n", attr->motion_adaptive_en);

	printf("shp_type=%d\n", attr->shp_type);
	printf("strength=%d\n", attr->strength);
	printf("shp_ex_manual.hpf_ratio=%d\n", attr->shp_ex_manual.hpf_ratio);
	printf("shp_ex_manual.transfer_curve_x=");
	for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.transfer_curve.x[i]);
		if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_manual.transfer_curve_y=");
	for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.transfer_curve.y[i]);
		if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_manual.luma_ctrl_gain_x=");
	for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.luma_ctrl_gain.x[i]);
		if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_manual.luma_ctrl_gain_y=");
	for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
		printf("%d", attr->shp_ex_manual.luma_ctrl_gain.y[i]);
		if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("shp_ex_auto.hpf_ratio=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->shp_ex_auto.hpf_ratio[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	for (int j = 0; j < MPI_ISO_LUT_ENTRY_NUM; j++) {
		printf("shp_ex_auto.transfer_curve[%d].x=", j);
		for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.transfer_curve[j].x[i]);
			if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
		printf("shp_ex_auto.transfer_curve[%d].y=", j);
		for (int i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.transfer_curve[j].y[i]);
			if (i == MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
		printf("shp_ex_auto.luma_ctrl_gain[%d].x=", j);
		for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.luma_ctrl_gain[j].x[i]);
			if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
		printf("shp_ex_auto.luma_ctrl_gain[%d].y=", j);
		for (int i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
			printf("%d", attr->shp_ex_auto.luma_ctrl_gain[j].y[i]);
			if (i == MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM - 1) {
				printf("\n");
			} else {
				printf(", ");
			}
		}
	}
	printf("shp_ex_manual.soft_clip_slope=%d\n", attr->shp_ex_manual.soft_clip_slope);
	printf("shp_ex_auto.soft_clip_slope=");
	for (int i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		printf("%d", attr->shp_ex_auto.soft_clip_slope[i]);
		if (i == MPI_ISO_LUT_ENTRY_NUM - 1) {
			printf("\n");
		} else {
			printf(", ");
		}
	}
	printf("ma_weak_shp_ratio=%hhu\n", attr->ma_weak_shp_ratio);
	printf("ma_conf_low_th=%hu\n", attr->ma_conf_low_th);
	printf("ma_conf_high_th=%hu\n", attr->ma_conf_high_th);
}

static int PARSE(ShpAttrV2)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_V2_S *data = (MPI_SHP_ATTR_V2_S *)opt->data;
	int num = argc - optind;
	int i, j;

	if (num == (NUM_SIGN_SHP_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->shp_auto_v2.sharpness[i] = atoi(argv[optind]);
			optind++;
		}

		data->shp_manual_v2.sharpness = atoi(argv[optind]);
		optind++;
		data->motion_adaptive_en = atoi(argv[optind]);
		optind++;
		data->shp_type = atoi(argv[optind]);
		optind++;
		data->strength = atoi(argv[optind]);
		optind++;
		data->shp_ex_manual.hpf_ratio = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
			data->shp_ex_manual.transfer_curve.x[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
			data->shp_ex_manual.transfer_curve.y[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
			data->shp_ex_manual.luma_ctrl_gain.x[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
			data->shp_ex_manual.luma_ctrl_gain.y[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
			data->shp_ex_auto.hpf_ratio[i] = atoi(argv[optind]);
			optind++;
		}
		for (j = 0; j < MPI_ISO_LUT_ENTRY_NUM; j++) {
			for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
				data->shp_ex_auto.transfer_curve[j].x[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_SHP_TRANSFER_CURVE_CTRL_POINT_NUM; i++) {
				data->shp_ex_auto.transfer_curve[j].y[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
				data->shp_ex_auto.luma_ctrl_gain[j].x[i] = atoi(argv[optind]);
				optind++;
			}
			for (i = 0; i < MPI_SHP_LUMA_CTRL_GAIN_POINT_NUM; i++) {
				data->shp_ex_auto.luma_ctrl_gain[j].y[i] = atoi(argv[optind]);
				optind++;
			}
		}
		data->shp_ex_manual.soft_clip_slope = atoi(argv[optind]);
		optind++;
		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
			data->shp_ex_auto.soft_clip_slope[i] = atoi(argv[optind]);
			optind++;
		}
		data->ma_weak_shp_ratio = atoi(argv[optind]);
		optind++;
		data->ma_conf_low_th = atoi(argv[optind]);
		optind++;
		data->ma_conf_high_th = atoi(argv[optind]);
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

static CMD_S shpv2_ops = MAKE_CMD("shpv2", MPI_SHP_ATTR_V2_S, ShpAttrV2);

__attribute__((constructor)) void regShpV2Cmd(void)
{
	CMD_register(&shpv2_ops);
}