#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_PTA_ATTR (48)

static INT32 GET(PtaAttr)(CMD_DATA_S *opt)
{
	return MPI_getPtaAttr(opt->path_idx, opt->data);
}

static INT32 SET(PtaAttr)(const CMD_DATA_S *opt)
{
	return MPI_setPtaAttr(opt->path_idx, opt->data);
}

static void ARGS(PtaAttr)(void)
{
	printf("\t'--pta dev_idx path_idx mode brightness_value contrast_value break_point_value pta_auto.tone[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] pta_manual.curve[0 ~ MPI_PTA_CURVE_ENTRY_NUM-1]'\n");
	printf("\t'--pta 0 0 0 128 128 64 1024 1024 1024 1024 1024 1024 1024 1024 1024 1024 1024 0 32 64 96 128 160 192 224 256 288 320 352 384 416 448 480 512 544 576 608 640 672 704 736 768 800 832 864 896 928 960 992 1024'\n");
}

static void HELP(PtaAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--pta <MPI_PATH> [PTA_ATTR]'", "Set PTA attributes");
}

static void SHOW(PtaAttr)(const CMD_DATA_S *opt)
{
	MPI_PTA_ATTR_S *attr = (MPI_PTA_ATTR_S *)opt->data;
	int i;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("mode=%d\n", attr->mode);
	printf("brightness=%d\n", attr->brightness);
	printf("strength=%d\n", attr->contrast);
	printf("break_point=%d\n", attr->break_point);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("pta_auto.tone[%d]=%d\n", i, attr->pta_auto.tone[i]);
	}

	for (i = 0; i < MPI_PTA_CURVE_ENTRY_NUM; ++i) {
		printf("pta_manual.curve[%d]=%d\n", i, attr->pta_manual.curve[i]);
	}
}

static int PARSE(PtaAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_PTA_ATTR_S *data = (MPI_PTA_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_PTA_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;
		data->brightness = atoi(argv[optind]);
		optind++;
		data->contrast = atoi(argv[optind]);
		optind++;
		data->break_point = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->pta_auto.tone[i] = atoi(argv[optind]);
			optind++;
		}
		for (i = 0; i < MPI_PTA_CURVE_ENTRY_NUM; ++i) {
			data->pta_manual.curve[i] = atoi(argv[optind]);
			optind++;
		}
	} else if (num == 2) {
		opt->action = CMD_ACTION_GET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S pta_ops = MAKE_CMD("pta", MPI_PTA_ATTR_S, PtaAttr);

__attribute__((constructor)) void regPtaCmd(void)
{
	CMD_register(&pta_ops);
}