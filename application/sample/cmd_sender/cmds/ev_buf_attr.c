#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_EV_BUF_ATTR (2)

static INT32 GET(EvBufAttr)(CMD_DATA_S *opt)
{
	return MPI_DEV_getPathEvBufStatus(opt->path_idx, opt->data);
}

static INT32 SET(EvBufAttr)(const CMD_DATA_S *opt)
{
	(void)opt;
	return -ENOSYS;
}

static void ARGS(EvBufAttr)(void)
{
	printf("\t'--ev_buf <dev_idx> <path_idx>'\n");
	printf("\t'--ev_buf 0 0'\n");
}

static void HELP(EvBufAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--ev_buf <MPI_PATH>'", "Get the current early video buffer status");
}

static void SHOW(EvBufAttr)(const CMD_DATA_S *opt)
{
	MPI_EV_BUF_STATUS_S *attr = (MPI_EV_BUF_STATUS_S *)opt->data;

	printf("dev_idx: %d, path_idx:%d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("total_alloc_size=%u\n", attr->total_alloc_size);
	printf("curr_used_size=%u\n", attr->curr_used_size);
	printf("buf_release=%u\n", attr->buf_release);
}

static int PARSE(EvBufAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;

	if (num == NUM_EV_BUF_ATTR) {
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;
		opt->action = CMD_ACTION_GET;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S ev_buf_ops = MAKE_CMD("ev_buf", MPI_EV_BUF_STATUS_S, EvBufAttr);

__attribute__((constructor)) void regEvBufCmd(void)
{
	CMD_register(&ev_buf_ops);
}
