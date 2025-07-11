#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_PATH_ATTR (1)

static INT32 GET(PathAttr)(CMD_DATA_S *opt)
{
	return MPI_DEV_getPathAttr(opt->path_idx, opt->data);
}

static INT32 SET(PathAttr)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setPathAttr(opt->path_idx, opt->data);
}

static void ARGS(PathAttr)(void)
{
	printf("\t'--path path_idx eis_strength'\n");
	printf("\t'--path 0 255'\n");
}

static void HELP(PathAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--path <MPI_PATH> [PATH_ATTR]'", "Set path eis_strength");
	CMD_PRINT_HELP(str, "'--path <MPI_PATH>'", "Query path attributes");
}

static void SHOW(PathAttr)(const CMD_DATA_S *opt)
{
	MPI_PATH_ATTR_S *attr = (MPI_PATH_ATTR_S *)opt->data;

	printf("dev_idx: %d, path_idx:%d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("sensor_idx=%d\n", attr->sensor_idx);
	printf("path_fps=%f\n", attr->fps);
	printf("res=%dx%d\n", attr->res.width, attr->res.height);
	printf("eis_strength=%d\n", attr->eis_strength);
}

static int PARSE(PathAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_PATH_ATTR_S *data = (MPI_PATH_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_PATH_ATTR + 1)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = 0;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		MPI_DEV_getPathAttr(opt->path_idx, data);

		data->eis_strength = atoi(argv[optind]);
		optind++;
	} else if (num == NUM_PATH_ATTR) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = 0;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S path_ops = MAKE_CMD("path", MPI_PATH_ATTR_S, PathAttr);

__attribute__((constructor)) void regPathCmd(void)
{
	CMD_register(&path_ops);
}