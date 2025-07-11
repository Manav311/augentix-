#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_AE_ATTR (104)

static INT32 GET(AeAttr)(CMD_DATA_S *opt)
{
	return MPI_getAeAttr(opt->path_idx, opt->data);
}

static INT32 SET(AeAttr)(const CMD_DATA_S *opt)
{
	return MPI_setAeAttr(opt->path_idx, opt->data);
}

static void ARGS(AeAttr)(void)
{
	printf("\t'--ae dev_idx path_idx sys_gain_range.min sys_gain_range.max sensor_gain_range.min\n");
	printf("\t      sensor_gain_range.max isp_gain_range.min isp_gain_range.max\n");
	printf("\t      frame_rate slow_frame_rate speed black_speed_bias interval brightness tolerance\n");
	printf("\t      gain_thr_up gain_thr_down strategy.mode strategy.strength\n");
	printf("\t      roi.luma_weight roi.awb_weight roi.zone_lum_avg_weight delay.black_delay_frame delay.white_delay_frame\n");
	printf("\t      anti_flicker.enable anti_flicker.frequency anti_flicker.luma_delta fps_mode\n");
	printf("\t      manual.is_valid manual.enable.bit.exp_value manual.enable.bit.inttime\n");
	printf("\t      manual.enable.bit.sensor_gain manual.enable.bit.isp_gain manual.enable.bit.sys_gain\n");
	printf("\t      manual.exp_value manual.inttime manual.sensor_gain manual.isp_gain manual.sys_gain\n");
	printf("\t      zone_weight.mode zone_weight.manual_table[8][8]\n");
	printf("\t      inttime_range.max inttime_range.min'\n");
	printf("\t'--ae 0 0 32 2048 32 7938 32 48 25 15 160 128 0 7500 700 1024 1536\n");
	printf("\t      0 2 1 1 0 7 0 1 60 4000 0 0 0 0 0 0 0 928 29 32 32 32\n");
	printf("\t      0 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1\n");
	printf("\t      1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1\n");
	printf("\t      10000000 0'\n");
}

static void HELP(AeAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--ae <MPI_PATH> [AE_ATTR]'", "Set AE attributes");
}

static void SHOW(AeAttr)(const CMD_DATA_S *opt)
{
	char str[512];
	int offset;

	MPI_AE_ATTR_S *attr = (MPI_AE_ATTR_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("sys_gain_range.min=%d\n", attr->sys_gain_range.min);
	printf("sys_gain_range.max=%d\n", attr->sys_gain_range.max);
	printf("sensor_gain_range.min=%d\n", attr->sensor_gain_range.min);
	printf("sensor_gain_range.max=%d\n", attr->sensor_gain_range.max);
	printf("isp_gain_range.min=%d\n", attr->isp_gain_range.min);
	printf("isp_gain_range.max=%d\n", attr->isp_gain_range.max);
	printf("frame_rate=%f\n", attr->frame_rate);
	printf("slow_frame_rate=%f\n", attr->slow_frame_rate);
	printf("speed=%d\n", attr->speed);
	printf("black_speed_bias=%d\n", attr->black_speed_bias);
	printf("interval=%d\n", attr->interval);
	printf("brightness=%d\n", attr->brightness);
	printf("tolerance=%d\n", attr->tolerance);
	printf("gain_thr_up=%d\n", attr->gain_thr_up);
	printf("gain_thr_down=%d\n", attr->gain_thr_down);
	printf("strategy.mode=%d\n", attr->strategy.mode);
	printf("strategy.strength=%d\n", attr->strategy.strength);
	printf("roi.luma_weight=%d\n", attr->roi.luma_weight);
	printf("roi.awb_weight=%d\n", attr->roi.awb_weight);
	printf("roi.zone_lum_avg_weight=%d\n", attr->roi.zone_lum_avg_weight);
	printf("delay.black_delay_frame=%d\n", attr->delay.black_delay_frame);
	printf("delay.white_delay_frame=%d\n", attr->delay.white_delay_frame);
	printf("anti_flicker.enable=%d\n", attr->anti_flicker.enable);
	printf("anti_flicker.frequency=%d\n", attr->anti_flicker.frequency);
	printf("anti_flicker.luma_delta=%d\n", attr->anti_flicker.luma_delta);
	printf("fps_mode=%d\n", attr->fps_mode);
	printf("manual.is_valid=%d\n", attr->manual.is_valid);
	printf("manual.enable.bit.exp_value=%d\n", attr->manual.enable.bit.exp_value);
	printf("manual.enable.bit.inttime=%d\n", attr->manual.enable.bit.inttime);
	printf("manual.enable.bit.sensor_gain=%d\n", attr->manual.enable.bit.sensor_gain);
	printf("manual.enable.bit.isp_gain=%d\n", attr->manual.enable.bit.isp_gain);
	printf("manual.enable.bit.sys_gain=%d\n", attr->manual.enable.bit.sys_gain);
	printf("manual.exp_value=%d\n", attr->manual.exp_value);
	printf("manual.inttime=%d\n", attr->manual.inttime);
	printf("manual.sensor_gain=%d\n", attr->manual.sensor_gain);
	printf("manual.isp_gain=%d\n", attr->manual.isp_gain);
	printf("manual.sys_gain=%d\n", attr->manual.sys_gain);
	printf("zone_weight.mode=%d\n", attr->zone_weight.mode);
	memset(str, 0, sizeof(str));
	offset = 0;
	printf("zone_weight.manual_table[%d][%d]=\n", MPI_AE_ZONE_ROW, MPI_AE_ZONE_COLUMN);
	for (int r = 0; r < MPI_AE_ZONE_ROW; r++) {
		for (int c = 0; c < MPI_AE_ZONE_COLUMN; c++) {
			offset += sprintf(str + offset, "%3u, ", attr->zone_weight.manual_table[r][c]);
			if (((r * MPI_AE_ZONE_ROW + c) % MPI_AE_ZONE_ROW) == MPI_AE_ZONE_ROW - 1) {
				printf("%s\n", str);
				offset = 0;
			}
		}
	}
	printf("inttime_range.max=%u\n", attr->inttime_range.max);
	printf("inttime_range.min=%u\n", attr->inttime_range.min);
}

static int PARSE(AeAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_AE_ATTR_S *data = (MPI_AE_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_AE_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->sys_gain_range.min = atoi(argv[optind]);
		optind++;
		data->sys_gain_range.max = atoi(argv[optind]);
		optind++;
		data->sensor_gain_range.min = atoi(argv[optind]);
		optind++;
		data->sensor_gain_range.max = atoi(argv[optind]);
		optind++;
		data->isp_gain_range.min = atoi(argv[optind]);
		optind++;
		data->isp_gain_range.max = atoi(argv[optind]);
		optind++;
		data->frame_rate = atof(argv[optind]);
		optind++;
		data->slow_frame_rate = atof(argv[optind]);
		optind++;
		data->speed = atoi(argv[optind]);
		optind++;
		data->black_speed_bias = atoi(argv[optind]);
		optind++;
		data->interval = atoi(argv[optind]);
		optind++;
		data->brightness = atoi(argv[optind]);
		optind++;
		data->tolerance = atoi(argv[optind]);
		optind++;
		data->gain_thr_up = atoi(argv[optind]);
		optind++;
		data->gain_thr_down = atoi(argv[optind]);
		optind++;
		data->strategy.mode = atoi(argv[optind]);
		optind++;
		data->strategy.strength = atoi(argv[optind]);
		optind++;
		data->roi.luma_weight = atoi(argv[optind]);
		optind++;
		data->roi.awb_weight = atoi(argv[optind]);
		optind++;
		data->roi.zone_lum_avg_weight = atoi(argv[optind]);
		optind++;
		data->delay.black_delay_frame = atoi(argv[optind]);
		optind++;
		data->delay.white_delay_frame = atoi(argv[optind]);
		optind++;
		data->anti_flicker.enable = atoi(argv[optind]);
		optind++;
		data->anti_flicker.frequency = atoi(argv[optind]);
		optind++;
		data->anti_flicker.luma_delta = atoi(argv[optind]);
		optind++;
		data->fps_mode = atoi(argv[optind]);
		optind++;
		data->manual.is_valid = atoi(argv[optind]);
		optind++;
		data->manual.enable.bit.exp_value = atoi(argv[optind]);
		optind++;
		data->manual.enable.bit.inttime = atoi(argv[optind]);
		optind++;
		data->manual.enable.bit.sensor_gain = atoi(argv[optind]);
		optind++;
		data->manual.enable.bit.isp_gain = atoi(argv[optind]);
		optind++;
		data->manual.enable.bit.sys_gain = atoi(argv[optind]);
		optind++;
		data->manual.exp_value = atoi(argv[optind]);
		optind++;
		data->manual.inttime = atoi(argv[optind]);
		optind++;
		data->manual.sensor_gain = atoi(argv[optind]);
		optind++;
		data->manual.isp_gain = atoi(argv[optind]);
		optind++;
		data->manual.sys_gain = atoi(argv[optind]);
		optind++;
		data->zone_weight.mode = atoi(argv[optind]);
		optind++;

		for (int r = 0; r < MPI_AE_ZONE_ROW; r++) {
			for (int c = 0; c < MPI_AE_ZONE_COLUMN; c++) {
				data->zone_weight.manual_table[r][c] = atoi(argv[optind]);
				optind++;
			}
		}

		data->inttime_range.max = strtoul(argv[optind], NULL, 0);
		optind++;
		data->inttime_range.min = strtoul(argv[optind], NULL, 0);
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

static CMD_S ae_ops = MAKE_CMD("ae", MPI_AE_ATTR_S, AeAttr);

__attribute__((constructor)) void regAeCmd(void)
{
	CMD_register(&ae_ops);
}
