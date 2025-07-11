#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_CHN_ATTR (1)

static INT32 GET(ChnFps)(CMD_DATA_S *opt)
{
	return MPI_DEV_getChnAttr(opt->chn_idx, opt->data);
}

static INT32 SET(ChnFps)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setChnAttr(opt->chn_idx, opt->data);
}

static void ARGS(ChnFps)(void)
{
	printf("\t'--chn dev_idx chn_idx fps'\n");
	printf("\t'--chn 0 0 20'\n");
}

static void HELP(ChnFps)(const char *str)
{
	CMD_PRINT_HELP(str, "'--chnfps <MPI_CHN> [CHN_FPS]'", "Set channel fps");
}

static void SHOW(ChnFps)(const CMD_DATA_S *opt)
{
	MPI_CHN_ATTR_S *attr = (MPI_CHN_ATTR_S *)opt->data;

	PRINT_CHN(opt->chn_idx);
	printf("fps=%f\n", attr->fps);
}

static int PARSE(ChnFps)(int argc, char **argv, CMD_DATA_S *opt)
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

		data->fps = atof(argv[optind++]);
	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->chn_idx.dev = atoi(argv[optind++]);
		opt->chn_idx.chn = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S chnfps_ops = MAKE_CMD("chnfps", MPI_CHN_ATTR_S, ChnFps);

__attribute__((constructor)) void regChnFpsCmd(void)
{
	CMD_register(&chnfps_ops);
}
