#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_PANORAMA_ATTR (7)

static INT32 GET(WinPanorama)(CMD_DATA_S *opt)
{
	return MPI_DEV_getPanoramaAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinPanorama)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setPanoramaAttr(opt->win_idx, opt->data);
}

static void ARGS(WinPanorama)(void)
{
	printf("\t'--winpano dev_idx chn_idx win_idx enable center_offset.x center_offset.y radius curvature ldc_ratio straighten'\n");
	printf("\t'--winpano 0 0 0 1 0 0 0 0'\n");
}

static void HELP(WinPanorama)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winpano <MPI_WIN> [PANO_ATTR]'", "Set PANORAMA attributes");
}

static void SHOW(WinPanorama)(const CMD_DATA_S *opt)
{
	MPI_PANORAMA_ATTR_S *attr = (MPI_PANORAMA_ATTR_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("enable=%d\n", attr->enable);
	printf("center_offset.x=%d\n", attr->center_offset.x);
	printf("center_offset.y=%d\n", attr->center_offset.y);
	printf("radius=%d\n", attr->radius);
	printf("curvature=%d\n", attr->curvature);
	printf("ldc_ratio=%d\n", attr->ldc_ratio);
	printf("straighten=%d\n", attr->straighten);
}

static int PARSE(WinPanorama)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_PANORAMA_ATTR_S *data = (MPI_PANORAMA_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_PANORAMA_ATTR + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		data->enable = atoi(argv[optind]);
		optind++;

		data->center_offset.x = atoi(argv[optind]);
		optind++;

		data->center_offset.y = atoi(argv[optind]);
		optind++;

		data->radius = atoi(argv[optind]);
		optind++;

		data->curvature = atoi(argv[optind]);
		optind++;

		data->ldc_ratio = atoi(argv[optind]);
		optind++;

		data->straighten = atoi(argv[optind]);
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

static CMD_S winpano_ops = MAKE_CMD("winpano", MPI_PANORAMA_ATTR_S, WinPanorama);

__attribute__((constructor)) void regWinPanoramaCmd(void)
{
	CMD_register(&winpano_ops);
}
