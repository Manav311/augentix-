#include "cmdparser.h"

#include "cmd_util.h"

#include "mpi_enc.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

static INT32 GET(VencInfo)(CMD_DATA_S *opt)
{
	return MPI_ENC_queryVencInfo(opt->echn_idx, opt->data);
}

static INT32 SET(VencInfo)(const CMD_DATA_S *opt __attribute__((unused)))
{
	return -ENOSYS;
}

static void ARGS(VencInfo)(void)
{
	printf("\t'--venc_info echn'\n");
	printf("\t'--venc_info 0'\n");
}

static void HELP(VencInfo)(const char *str)
{
	CMD_PRINT_HELP(str, "'--venc_info <MPI_ECHN>'", "Query internal encoder information");
}

static void SHOW(VencInfo)(const CMD_DATA_S *opt)
{
	MPI_VENC_INFO_S *attr = (MPI_VENC_INFO_S *)opt->data;

	printf("encoder index: %hhu\n", opt->echn_idx.chn);
	printf("last_qp = %u\n", attr->last_qp);
	printf("last_i_qp = %u\n", attr->last_i_qp);
	printf("bps = %u\n", attr->bps);
	printf("fps = %u\n", attr->fps);
	printf("frame_cnt_in_streambuffer = %u\n", attr->frame_cnt_in_streambuffer);
	printf("frame_drop_cnt = %u\n", attr->frame_drop_cnt);
}

static int PARSE(VencInfo)(int argc, char **argv, CMD_DATA_S *opt)
{
	const int num = argc - optind;

	if (num != 1) {
		return -EINVAL;
	}

	opt->action = CMD_ACTION_GET;
	opt->echn_idx.chn = (uint8_t)atoi(argv[optind]);

	return 0;
}

static CMD_S venc_info_ops = MAKE_CMD("venc_info", MPI_VENC_INFO_S, VencInfo);

__attribute__((constructor)) void regVencInfoCmd(void)
{
	CMD_register(&venc_info_ops);
}
