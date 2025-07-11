#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_GAMMA_ATTR (1)

static INT32 GET(GammaAttr)(CMD_DATA_S *opt)
{
	return MPI_getGammaAttr(opt->path_idx, opt->data);
}

static INT32 SET(GammaAttr)(const CMD_DATA_S *opt)
{
	return MPI_setGammaAttr(opt->path_idx, opt->data);
}

static void ARGS(GammaAttr)(void)
{
	printf("\t'--gamma dev_idx path_idx mode gma_manual.curve[0 ~ MPI_GAMMA_CURVE_ENTRY_NUM-1]'\n");
	printf("\t'--gamma 0 0 0   0   288   576   864  1152  1442  1704  1943  2163  2563 2921  3247  3549  3830  4095  4346  4584  4811  5030  5239\n");
	printf("\t               5442  5637  5826  6010  6188  6530  6855  7166  7464  7751 8027  8294  8552  8803  9046  9283  9514  9739  9959 10173\n");
	printf("\t               10383 10790 11182 11559 11924 12277 12619 12951 13275 13590 13897 14198 14491 14778 15059 15334 15604 15869 16129 16384'\n");
}

static void HELP(GammaAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--gamma <MPI_PATH> [GAMMA_ATTR]'", "Set GAMMA attributes");
}

static void SHOW(GammaAttr)(const CMD_DATA_S *opt)
{
	MPI_GAMMA_ATTR_S *attr = (MPI_GAMMA_ATTR_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode = %d\n", attr->mode);
	for (i = 0; i < MPI_GAMMA_CURVE_ENTRY_NUM; ++i) {
		printf("gma_manual.curve[%d]=%d\n", i, attr->gma_manual.curve[i]);
	}
}

static int PARSE(GammaAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_GAMMA_ATTR_S *gamma = (MPI_GAMMA_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_GAMMA_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]++);
		opt->path_idx.path = atoi(argv[optind++]);
		gamma->mode = atoi(argv[optind++]);
	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = atoi(argv[optind]++);
		opt->path_idx.path = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S gamma_ops = MAKE_CMD("gamma", MPI_GAMMA_ATTR_S, GammaAttr);

__attribute__((constructor)) void regGammaCmd(void)
{
	CMD_register(&gamma_ops);
}