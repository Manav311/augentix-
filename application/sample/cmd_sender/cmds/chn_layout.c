#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_CHN_LAYOPUT_ATTR (2)

static INT32 GET(ChnLayout)(CMD_DATA_S *opt)
{
	return MPI_DEV_getChnLayout(opt->chn_idx, opt->data);
}

static INT32 SET(ChnLayout)(const CMD_DATA_S *opt)
{
	(void)(opt);
	printf("%s need to re-stream, not support\n", "setChnLayout");
	return MPI_SUCCESS;
}

static void ARGS(ChnLayout)(void)
{
	printf("\t'--chn_layout dev_idx chn_idx [GET only]'\n");
	printf("\t'--chn_layout 0 0'\n");
}

static void HELP(ChnLayout)(const char *str)
{
	CMD_PRINT_HELP(str, "'--chn_layout <MPI_CHN>'", "Query channel layout");
}

static void SHOW(ChnLayout)(const CMD_DATA_S *opt)
{
	MPI_CHN_LAYOUT_S *attr = (MPI_CHN_LAYOUT_S *)opt->data;
	printf("window num: %d\n", attr->window_num);

	for (int i = 0; i < attr->window_num; i++) {
		PRINT_WIN(opt->win_idx);
		printf("x=%d,\ny=%d,\nwidth=%d,\nheight=%d\n", attr->window[i].x, attr->window[i].y,
		       attr->window[i].width, attr->window[i].height);
	}
}

static int PARSE(ChnLayout)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;

	if (num == NUM_CHN_LAYOPUT_ATTR + 2) {
		opt->action = CMD_ACTION_SET;
		opt->chn_idx.dev = atoi(argv[optind++]);
		opt->chn_idx.chn = atoi(argv[optind++]);
	} else if (num == NUM_CHN_LAYOPUT_ATTR) {
		opt->action = CMD_ACTION_GET;
		opt->chn_idx.dev = atoi(argv[optind++]);
		opt->chn_idx.chn = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S chnlayout_ops = MAKE_CMD("chn_layout", MPI_CHN_LAYOUT_S, ChnLayout);

__attribute__((constructor)) void regChnLayoutCmd(void)
{
	CMD_register(&chnlayout_ops);
}