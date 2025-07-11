#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

static INT32 GET(Stat)(CMD_DATA_S *opt)
{
	return MPI_getStatistics(opt->path_idx, opt->data);
}

static INT32 SET(Stat)(const CMD_DATA_S *opt)
{
	(void)(opt);
	return -EINVAL;
}

static void ARGS(Stat)(void)
{
	printf("\t'--stat path_idx dev_idx'\n");
	printf("\t'--stat 0 0'\n");
}

static void HELP(Stat)(const char *str)
{
	CMD_PRINT_HELP(str, "'--stat <MPI_PATH>'", "Query Statistics");
}

static void SHOW(Stat)(const CMD_DATA_S *opt)
{
	MPI_DIP_STAT_S *attr = (MPI_DIP_STAT_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("focus_stat:\n");
	printf("hor_var_sum:%d\n", attr->focus_stat.hor_var_sum);
	printf("ver_var_sum:%d\n", attr->focus_stat.ver_var_sum);
	printf("wb_stat:\n");
	printf("global_r_avg:%d %d %d %d\n", attr->wb_stat.global_r_avg[0], attr->wb_stat.global_r_avg[1],
	       attr->wb_stat.global_r_avg[2], attr->wb_stat.global_r_avg[3]);
	printf("global_g_avg:%d %d %d %d\n", attr->wb_stat.global_g_avg[0], attr->wb_stat.global_g_avg[1],
	       attr->wb_stat.global_g_avg[2], attr->wb_stat.global_g_avg[3]);
	printf("global_b_avg:%d %d %d %d\n", attr->wb_stat.global_b_avg[0], attr->wb_stat.global_b_avg[1],
	       attr->wb_stat.global_b_avg[2], attr->wb_stat.global_b_avg[3]);

	printf("zone_r_avg:\n");
	for (unsigned int i = 0; i < MPI_AWB_ZONE_NUM; i++) {
		printf("\t[%d] %d\n", i, attr->wb_stat.zone_r_avg[i]);
	}

	printf("zone_g_avg:\n");
	for (unsigned int i = 0; i < MPI_AWB_ZONE_NUM; i++) {
		printf("\t[%d] %d\n", i, attr->wb_stat.zone_g_avg[i]);
	}

	printf("zone_b_avg:\n");
	for (unsigned int i = 0; i < MPI_AWB_ZONE_NUM; i++) {
		printf("\t[%d] %d\n", i, attr->wb_stat.zone_b_avg[i]);
	}

	printf("ae_stat:\n");
	printf("\tlum_hist:\n");
	for (unsigned int i = 0; i < MPI_LUM_HIST_ENTRY_NUM; i++) {
		printf("\t[%d] %d\n", i, attr->ae_stat.lum_hist[i]);
	}
}

static int PARSE(Stat)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;

	if (num == 2) {
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

static CMD_S stat_ops = MAKE_CMD("stat", MPI_DIP_STAT_S, Stat);

__attribute__((constructor)) void regStatCmd(void)
{
	CMD_register(&stat_ops);
}
