#ifndef _GNU_SOURCE
#define _GNU_SOURCE // getopt_long()
#endif

#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_enc.h"

#include "cmd_util.h"

/* clang-format off */
typedef enum opt_val {
	GET = 0,
	SET,
} OptVal;

static struct option opts[] = {
	{ "set", no_argument, NULL, SET },
	{ "get", no_argument, NULL, GET },
	{ 0 }
};
/* clang-format on */

static INT32 GET(VencExtend)(CMD_DATA_S *opt)
{
	if (!opt || !opt->data) {
		return -EINVAL;
	}

	return MPI_ENC_getVencExtendFile(opt->echn_idx, (const char *)opt->data);
}

static INT32 SET(VencExtend)(const CMD_DATA_S *opt)
{
	INT32 ret;

	if (!opt || !opt->data) {
		return -EINVAL;
	}

	ret = MPI_ENC_setVencExtendFile(opt->echn_idx, (const char *)opt->data);
	if (ret == 0) {
		printf("Set Venc Extend settings successfully.\n");
	}

	return ret;
}

static void ARGS(VencExtend)(void)
{
	printf("\t--venc_extend --get enc_idx file\n");
	printf("\t--venc_extend --set enc_idx file\n");
	printf("\t--venc_extend --get 0 /mnt/sdcard/venc_extend_0.ini\n");
	printf("\t--venc_extend --set 3 /mnt/sdcard/venc_extend_3.ini\n");
}

static void HELP(VencExtend)(const char *str)
{
	CMD_PRINT_HELP(str, "'--venc_extend --get VENC_IDX VENC_EXTEND_FILE'",
	               "Save current VENC Extend parameters to a file.");
	CMD_PRINT_HELP(str, "'--venc_extend --set VENC_IDX VENC_EXTEND_FILE'",
	               "Update VENC Extend parameters from the given file.");
}

static void SHOW(VencExtend)(const CMD_DATA_S *opt)
{
	printf("Venc Extend file saved at: %s\n", (const char *)opt->data);
}

static int PARSE(VencExtend)(int argc, char **argv, CMD_DATA_S *opt)
{
	int ret;
	int arg_cnt = 0;

	opt->action = CMD_ACTION_NON;

	if ((ret = getopt_long(argc, argv, "", opts, NULL)) != -1) {
		switch (ret) {
		case GET:
			opt->action = CMD_ACTION_GET;
			break;
		case SET:
			opt->action = CMD_ACTION_SET;
			break;
		default:
			return -EINVAL;
			break;
		}
	}

	for (; optind < argc; optind++) {
		switch (arg_cnt) {
		case 0:
			opt->echn_idx = MPI_ENC_CHN(atoi(argv[optind]));
			break;
		case 1:
			opt->data = argv[optind];
			break;
		}
		arg_cnt++;
	}
	if (arg_cnt < 2) {
		return -EINVAL;
	}

	return 0;
}

static CMD_S venc_extend_ops = MAKE_CMD("venc_extend", char *, VencExtend);

__attribute__((constructor)) void regVencExtendCmd(void)
{
	CMD_register(&venc_extend_ops);
}
