#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_DEV_ATTR (1)

static INT32 GET(DevAttr)(CMD_DATA_S *opt)
{
	return MPI_DEV_getDevAttr(opt->dev_idx, opt->data);
}

static INT32 SET(DevAttr)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setDevAttr(opt->dev_idx, opt->data);
}

static void ARGS(DevAttr)(void)
{
	printf("\t'--dev dev_idx fps'\n");
	printf("\t'--dev 0 20'\n");
}

static void HELP(DevAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dev <MPI_DEV> [DEV_ATTR]'", "Set device fps");
	CMD_PRINT_HELP(str, "'--dev <MPI_DEV>'", "Query device attributes");
}

static void SHOW(DevAttr)(const CMD_DATA_S *opt)
{
	MPI_DEV_ATTR_S *attr = (MPI_DEV_ATTR_S *)opt->data;

	printf("device index: %d\n", opt->dev_idx.dev);
	printf("stitch_en=%d\n", attr->stitch_en);
	printf("eis_en=%d\n", attr->eis_en);
	printf("hdr_mode=%d\n", attr->hdr_mode);
	printf("fps=%f\n", attr->fps);
	printf("bayer=%d\n", attr->bayer);
	printf("path: bmp=%d, path0_en=%d, path1_en=%d\n", attr->path.bmp, attr->path.bit.path0_en,
	       attr->path.bit.path1_en);
}

static int PARSE(DevAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_DEV_ATTR_S *data = (MPI_DEV_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_DEV_ATTR + 1)) {
		opt->action = CMD_ACTION_SET;
		opt->dev_idx.dev = atoi(argv[optind]);
		optind++;

		MPI_DEV_getDevAttr(opt->dev_idx, data);

		data->fps = atoi(argv[optind]);
		optind++;
	} else if (num == 1) {
		opt->action = CMD_ACTION_GET;
		opt->dev_idx.dev = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S dev_ops = MAKE_CMD("dev", MPI_DEV_ATTR_S, DevAttr);

__attribute__((constructor)) void regDevCmd(void)
{
	CMD_register(&dev_ops);
}