#include "cmdparser.h"

#include "cmd_util.h"

#include "mpi_dip_alg.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static INT32 GET(TeInfo)(CMD_DATA_S *opt)
{
	return MPI_queryTeInfo(opt->path_idx, opt->data);
}

static INT32 SET(TeInfo)(const CMD_DATA_S *opt __attribute__((unused)))
{
	return -ENOSYS;
}

static void ARGS(TeInfo)(void)
{
	printf("\t'--te_info dev_idx path_idx'\n");
	printf("\t'--te_info 0 0'\n");
}

static void HELP(TeInfo)(const char *str)
{
	CMD_PRINT_HELP(str, "'--te_info <MPI_PATH>'", "Query internal tone enhancement status information");
}

static void SHOW(TeInfo)(const CMD_DATA_S *opt)
{
	MPI_TE_INFO_S *attr = (MPI_TE_INFO_S *)opt->data;

	printf("tm_enable = %u\n", attr->tm_enable);
	printf("tm_curve = \n");
	for (int i = 0; i < MPI_TE_CURVE_ENTRY_NUM; i++) {
		if (i % 15 == 14) {
			printf("%5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d, %5d\n",
			       attr->tm_curve[i - 14], attr->tm_curve[i - 13], attr->tm_curve[i - 12],
			       attr->tm_curve[i - 11], attr->tm_curve[i - 10], attr->tm_curve[i - 9],
			       attr->tm_curve[i - 8], attr->tm_curve[i - 7], attr->tm_curve[i - 6],
			       attr->tm_curve[i - 5], attr->tm_curve[i - 4], attr->tm_curve[i - 3],
			       attr->tm_curve[i - 2], attr->tm_curve[i - 1], attr->tm_curve[i]);
		}
	}
}

static int PARSE(TeInfo)(int argc, char **argv, CMD_DATA_S *opt)
{
	const int num = argc - optind;

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

static CMD_S te_info_ops = MAKE_CMD("te_info", MPI_TE_INFO_S, TeInfo);

__attribute__((constructor)) void regTeInfoCmd(void)
{
	CMD_register(&te_info_ops);
}
