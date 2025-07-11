#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_SHP_ATTR (13)

static INT32 GET(WinShp)(CMD_DATA_S *opt)
{
	return MPI_getWinShpAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinShp)(const CMD_DATA_S *opt)
{
	return MPI_setWinShpAttr(opt->win_idx, opt->data);
}

static void ARGS(WinShp)(void)
{
	printf("\t'--winshp dev_idx chn_idx win_idx mode shp_auto.sharpness[0 ~ MPI_ISO_LUT_ENTRY_NUM-1] shp_manual.sharpness'\n");
	printf("\t'--winshp 0 0 0 0 255 100 60 0 0 0 0 0 0 0 0 128'\n");
}

static void HELP(WinShp)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winshp <MPI_WIN> [SHP_ATTR]'", "Set WinSHP attributes");
}

static void SHOW(WinShp)(const CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_S *attr = (MPI_SHP_ATTR_S *)opt->data;
	int i;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("mode=%d\n", attr->mode);
	for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
		printf("shp_auto.sharpness[%d]=%d\n", i, attr->shp_auto.sharpness[i]);
	}
	printf("shp_manual.sharpness=%d\n", attr->shp_manual.sharpness);
}

static int PARSE(WinShp)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_SHP_ATTR_S *data = (MPI_SHP_ATTR_S *)opt->data;
	int num = argc - optind;
	int i;

	if (num == (NUM_SHP_ATTR + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		data->mode = atoi(argv[optind]);
		optind++;

		for (i = 0; i < MPI_ISO_LUT_ENTRY_NUM; ++i) {
			data->shp_auto.sharpness[i] = atoi(argv[optind]);
			optind++;
		}

		data->shp_manual.sharpness = atoi(argv[optind]);
		optind++;
	} else if (num == 3) {
		opt->action = CMD_ACTION_GET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S winshp_ops = MAKE_CMD("winshp", MPI_SHP_ATTR_S, WinShp);

__attribute__((constructor)) void regWinShpCmd(void)
{
	CMD_register(&winshp_ops);
}
