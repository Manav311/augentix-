#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_STAT_ATTR (28)

static INT32 GET(StatAttr)(CMD_DATA_S *opt)
{
	return MPI_getStatisticsConfig(opt->path_idx, opt->data);
}

static INT32 SET(StatAttr)(const CMD_DATA_S *opt)
{
	return MPI_setStatisticsConfig(opt->path_idx, opt->data);
}

static void ARGS(StatAttr)(void)
{
	printf("\t'--stat_attr dev_idx path_idx lum_min lum_max lum_slope "
	       "rb_point_x_0 rb_point_x_1 rb_point_x_2 rb_point_x_3 rb_point_x_4 "
	       "rb_point_y_0 rb_point_y_1 rb_point_y_2 rb_point_y_3 rb_point_y_4 "
	       "rb_rgn_th_0 rb_rgn_th_1 rb_rgn_th_2 rb_rgn_th_3 "
	       "rb_rgn_slope_0 rb_rgn_slop_1 rb_rgn_slope_2 rb_rgn_slope_3 "
	       "gwd_auto_lum_thd_enable gwd_auto_lum_thd_param.lum_max_degree "
	       "gwd_auto_lum_thd_param.indoor_ev_thd gwd_auto_lum_thd_param.outdoor_ev_thd "
	       "gwd_auto_lum_thd_param.indoor_lum_range gwd_auto_lum_thd_param.outdoor_lum_range "
	       "gwd_auto_lum_thd_param.min_lum_bnd'\n");
	printf("\t'--stat 0 0 1500 13000 16 102 70 64 58 26 26 58 64 70 102 2 2 1 1 16 16 16 16 "
	       "1 13 640000 256000 4480 8960 640'\n");
}

static void HELP(StatAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--stat_attr <MPI_PATH> [STAT_ATTR]'", "Set STAT attributes");
}

static void SHOW(StatAttr)(const CMD_DATA_S *opt)
{
	MPI_STAT_CFG_S *attr = (MPI_STAT_CFG_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("wb.lum_min=%d\n", attr->wb.lum_min);
	printf("wb.lum_max=%d\n", attr->wb.lum_max);
	printf("wb.lum_slope=%d\n", attr->wb.lum_slope);

	for (int i = 0; i < MPI_WB_RB_POINT_NUM; ++i) {
		printf("wb.rb_point_x[%d]=%d\n", i, attr->wb.rb_point_x[i]);
	}

	for (int i = 0; i < MPI_WB_RB_POINT_NUM; ++i) {
		printf("wb.rb_point_y[%d]=%d\n", i, attr->wb.rb_point_y[i]);
	}

	for (int i = 0; i < MPI_WB_RB_POINT_NUM - 1; ++i) {
		printf("wb.rb_rgn_th[%d]=%d\n", i, attr->wb.rb_rgn_th[i]);
	}

	for (int i = 0; i < MPI_WB_RB_POINT_NUM - 1; ++i) {
		printf("wb.rb_rgn_slope[%d]=%d\n", i, attr->wb.rb_rgn_slope[i]);
	}

	printf("wb.gwd_auto_lum_thd_enable: %d\n", attr->wb.gwd_auto_lum_thd_enable);
	printf("wb.gwd_auto_lum_thd_param.lum_max_degree: %d\n", attr->wb.gwd_auto_lum_thd_param.lum_max_degree);
	printf("wb.gwd_auto_lum_thd_param.indoor_ev_thd: %llu\n", attr->wb.gwd_auto_lum_thd_param.indoor_ev_thd);
	printf("wb.gwd_auto_lum_thd_param.outdoor_ev_thd: %llu\n", attr->wb.gwd_auto_lum_thd_param.outdoor_ev_thd);
	printf("wb.gwd_auto_lum_thd_param.indoor_lum_range: %d\n", attr->wb.gwd_auto_lum_thd_param.indoor_lum_range);
	printf("wb.gwd_auto_lum_thd_param.outdoor_lum_range: %d\n", attr->wb.gwd_auto_lum_thd_param.outdoor_lum_range);
	printf("wb.gwd_auto_lum_thd_param.min_lum_bnd: %d\n", attr->wb.gwd_auto_lum_thd_param.min_lum_bnd);
}

static int PARSE(StatAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_STAT_CFG_S *data = (MPI_STAT_CFG_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_STAT_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->wb.lum_min = atoi(argv[optind]);
		optind++;
		data->wb.lum_max = atoi(argv[optind]);
		optind++;
		data->wb.lum_slope = atoi(argv[optind]);
		optind++;

		for (int i = 0; i < MPI_WB_RB_POINT_NUM; ++i) {
			data->wb.rb_point_x[i] = atoi(argv[optind]);
			optind++;
		}

		for (int i = 0; i < MPI_WB_RB_POINT_NUM; ++i) {
			data->wb.rb_point_y[i] = atoi(argv[optind]);
			optind++;
		}

		for (int i = 0; i < MPI_WB_RB_POINT_NUM - 1; ++i) {
			data->wb.rb_rgn_th[i] = atoi(argv[optind]);
			optind++;
		}

		for (int i = 0; i < MPI_WB_RB_POINT_NUM - 1; ++i) {
			data->wb.rb_rgn_slope[i] = atoi(argv[optind]);
			optind++;
		}

		data->wb.gwd_auto_lum_thd_enable = atoi(argv[optind]);
		optind++;
		data->wb.gwd_auto_lum_thd_param.lum_max_degree = atoi(argv[optind]);
		optind++;
		data->wb.gwd_auto_lum_thd_param.indoor_ev_thd = atoi(argv[optind]);
		optind++;
		data->wb.gwd_auto_lum_thd_param.outdoor_ev_thd = atoi(argv[optind]);
		optind++;
		data->wb.gwd_auto_lum_thd_param.indoor_lum_range = atoi(argv[optind]);
		optind++;
		data->wb.gwd_auto_lum_thd_param.outdoor_lum_range = atoi(argv[optind]);
		optind++;
		data->wb.gwd_auto_lum_thd_param.min_lum_bnd = atoi(argv[optind]);
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

static CMD_S stat_attr_ops = MAKE_CMD("stat_attr", MPI_STAT_CFG_S, StatAttr);

__attribute__((constructor)) void regStatAttrCmd(void)
{
	CMD_register(&stat_attr_ops);
}
