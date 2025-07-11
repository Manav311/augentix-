#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_CHN_ATTR (1)

static INT32 GET(ChnAttr)(CMD_DATA_S *opt)
{
	return MPI_DEV_getChnAttr(opt->chn_idx, opt->data);
}

static INT32 SET(ChnAttr)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setChnAttr(opt->chn_idx, opt->data);
}

static void ARGS(ChnAttr)(void)
{
	printf("\t'--chn dev_idx chn_idx'\n");
	printf("\t'--chn 0 0'\n");
}

static void HELP(ChnAttr)(const char* str)
{
	CMD_PRINT_HELP(str, "'--chn <MPI_CHN>'", "Query channel attr");
}

static void SHOW(ChnAttr)(const CMD_DATA_S *opt)
{
	MPI_CHN_ATTR_S *attr = (MPI_CHN_ATTR_S *)opt->data;

	PRINT_CHN(opt->chn_idx);
	printf("res=%dx%d\n", attr->res.width, attr->res.height);
	printf("fps=%f\n", attr->fps);
	printf("binding_capability=%d\n", attr->binding_capability);
}

static int PARSE(ChnAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_CHN_ATTR_S *data = (MPI_CHN_ATTR_S *)opt->data;
	int num = argc - optind;
	int ret = 0;

	if (num == (NUM_CHN_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->chn_idx.dev = atoi(argv[optind++]);
		opt->chn_idx.chn = atoi(argv[optind++]);

		if ((ret = MPI_DEV_getChnAttr(opt->chn_idx, data))) {
			return ret;
		}
	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->chn_idx.dev = atoi(argv[optind++]);
		opt->chn_idx.chn = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S chn_ops = MAKE_CMD("chn", MPI_CHN_ATTR_S, ChnAttr);

__attribute__((constructor)) void regChnCmd(void)
{
	CMD_register(&chn_ops);
}