#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_sys.h"

#include "cmd_util.h"

#define NUM_VB_ATTR (0)

static INT32 GET(VbAttr)(CMD_DATA_S *opt)
{
	return MPI_VB_getConf(opt->data);
}

static INT32 SET(VbAttr)(const CMD_DATA_S *opt)
{
	(void)(opt);
	printf("VB not support set func\n");
	return MPI_SUCCESS;
}

static void ARGS(VbAttr)(void)
{
	printf("\t'--vb'\n");
}

static void HELP(VbAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--vb'", "Query video buffer");
}

static void SHOW(VbAttr)(const CMD_DATA_S *opt)
{
	MPI_VB_CONF_S *attr = (MPI_VB_CONF_S *)opt->data;

	printf("max_pool_cnt=%d\n", attr->max_pool_cnt);

	for (int i = 0; i < (int)attr->max_pool_cnt; i++) {
		printf("pub_pool[%d]:\n", i);
		printf("\tblk_size=%d\n", attr->pub_pool[i].blk_size);
		printf("\tblk_cnt=%d\n", attr->pub_pool[i].blk_cnt);
		printf("\tname=%s\n", attr->pub_pool[i].name);
	}
}

static int PARSE(VbAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	int num = argc - optind;
	(void)(argv);

	if (num == 1 + NUM_VB_ATTR) {
		opt->action = CMD_ACTION_GET;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S vb_ops = MAKE_CMD("vb", MPI_VB_CONF_S, VbAttr);

__attribute__((constructor)) void regVbCmd(void)
{
	CMD_register(&vb_ops);
}