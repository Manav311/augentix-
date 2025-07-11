#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

typedef enum opt_val {
	GET = 0,
	SET,
} OptVal;

static struct option opts[] = { { "set", no_argument, NULL, SET }, { "get", no_argument, NULL, GET } };

static INT32 GET(DipExtend)(CMD_DATA_S *opt)
{
	return MPI_getDipExtendFile(opt->path_idx, opt->data);
}

static INT32 SET(DipExtend)(const CMD_DATA_S *opt)
{
	return MPI_setDipExtendFile(opt->path_idx, opt->data);
}

static void ARGS(DipExtend)(void)
{
	printf("\tGet internal interface settings: cmdsender --dip_extend --get <dev_idx> <path_idx> <file path>\n");
	printf("\tSet internal interface settings: cmdsender --dip_extend --set <dev_idx> <path_idx> <file path>\n");
}

static void HELP(DipExtend)(const char *str)
{
	CMD_PRINT_HELP(str, "'--dip_extend --get <MPI_PATH> [DIP_EXTEND_FILE]'", "Get internal interface parameters");
	CMD_PRINT_HELP(str, "'--dip_extend --set <MPI_PATH> [DIP_EXTEND_FILE]'", "Set internal interface parameters");
}

static void SHOW(DipExtend)(const CMD_DATA_S *opt)
{
	printf("file path: %s\n", (char *)opt->data);
}

static int PARSE(DipExtend)(int argc, char **argv, CMD_DATA_S *opt)
{
	int ret;

	while ((ret = getopt_long(argc, argv, ":", opts, NULL)) != -1) {
		switch (ret) {
		case GET:
			if ((argc - optind) != 3) {
				break;
			}
			opt->path_idx.dev = atoi(argv[optind]);
			optind++;
			opt->path_idx.path = atoi(argv[optind]);
			optind++;
			opt->data = argv[optind];
			opt->action = CMD_ACTION_GET;
			break;
		case SET:
			if ((argc - optind) != 3) {
				break;
			}
			opt->path_idx.dev = atoi(argv[optind]);
			optind++;
			opt->path_idx.path = atoi(argv[optind]);
			optind++;
			opt->data = argv[optind];
			opt->action = CMD_ACTION_SET;
			break;
		case '?':
		default:
			return EINVAL;
			break;
		}
	}

	return 0;
}

static CMD_S dip_extend_ops = MAKE_CMD("dip_extend", char *, DipExtend);

__attribute__((constructor)) void regDipExtendCmd(void)
{
	CMD_register(&dip_extend_ops);
}