#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_LSC_ATTR (6)

static INT32 GET(LscAttr)(CMD_DATA_S *opt)
{
	return MPI_getLscAttr(opt->path_idx, opt->data);
}

static INT32 SET(LscAttr)(const CMD_DATA_S *opt)
{
	return MPI_setLscAttr(opt->path_idx, opt->data);
}

static void ARGS(LscAttr)(void)
{
	printf("\t'--lsc dev_idx path_idx origin x_trend_2s y_trend_2s x_curvature y_curvature tilt_2s'\n");
	printf("\t'--lsc 0 0 93586 1452 1016214 32892 1289137 134196533'\n");
}

static void HELP(LscAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--lsc <MPI_PATH> [LSC_ATTR]'", "Set LSC attributes");
}

static void SHOW(LscAttr)(const CMD_DATA_S *opt)
{
	MPI_LSC_ATTR_S *attr = (MPI_LSC_ATTR_S *)opt->data;
	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("origin=%d, x_trend_2s=%d, y_trend_2s=%d, x_curvature=%d, y_curvature=%d, tilt_2s=%d\n", attr->origin,
	       attr->x_trend_2s, attr->y_trend_2s, attr->x_curvature, attr->y_curvature, attr->tilt_2s);
}

static int PARSE(LscAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_LSC_ATTR_S *data = (MPI_LSC_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_LSC_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->origin = atoi(argv[optind]);
		optind++;
		data->x_trend_2s = atoi(argv[optind]);
		optind++;
		data->y_trend_2s = atoi(argv[optind]);
		optind++;
		data->x_curvature = atoi(argv[optind]);
		optind++;
		data->y_curvature = atoi(argv[optind]);
		optind++;
		data->tilt_2s = atoi(argv[optind]);
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

static CMD_S lsc_ops = MAKE_CMD("lsc", MPI_LSC_ATTR_S, LscAttr);

__attribute__((constructor)) void regLscCmd(void)
{
	CMD_register(&lsc_ops);
}
