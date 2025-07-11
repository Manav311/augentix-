#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dip_alg.h"

#include "cmd_util.h"

#define NUM_ROI_ATTR (12)

static INT32 GET(RoiAttr)(CMD_DATA_S *opt)
{
	return MPI_getRoiAttr(opt->path_idx, opt->data);
}

static INT32 SET(RoiAttr)(const CMD_DATA_S *opt)
{
	return MPI_setRoiAttr(opt->path_idx, opt->data);
}

static void ARGS(RoiAttr)(void)
{
	printf("\t'--roi dev_idx path_idx luma_roi.sx luma_roi.sy luma_roi.ex luma_roi.ey awb_roi.sx awb_roi.sy awb_roi.ex awb_roi.ey zone_lum_avg_roi.sx zone_lum_avg_roi.sy zone_lum_avg_roi.ex zone_lum_avg_roi.ey'\n");
	printf("\t'--roi 0 0 0 0 1024 1024 0 0 1024 1024 0 0 1024 1024'\n");
}

static void HELP(RoiAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--roi <MPI_PATH> [ROI_ATTR]'", "Set ROI attributes");
}

static void SHOW(RoiAttr)(const CMD_DATA_S *opt)
{
	MPI_ROI_ATTR_S *attr = (MPI_ROI_ATTR_S *)opt->data;

	printf("device index: %d, path index: %d\n", opt->path_idx.dev, opt->path_idx.path);
	printf("luma_roi.sx=%d\n", attr->luma_roi.sx);
	printf("luma_roi.sy=%d\n", attr->luma_roi.sy);
	printf("luma_roi.ex=%d\n", attr->luma_roi.ex);
	printf("luma_roi.ey=%d\n", attr->luma_roi.ey);

	printf("awb_roi.sx=%d\n", attr->awb_roi.sx);
	printf("awb_roi.sy=%d\n", attr->awb_roi.sy);
	printf("awb_roi.ex=%d\n", attr->awb_roi.ex);
	printf("awb_roi.ey=%d\n", attr->awb_roi.ey);

	printf("zone_lum_avg_roi.sx=%d\n", attr->zone_lum_avg_roi.sx);
	printf("zone_lum_avg_roi.sy=%d\n", attr->zone_lum_avg_roi.sy);
	printf("zone_lum_avg_roi.ex=%d\n", attr->zone_lum_avg_roi.ex);
	printf("zone_lum_avg_roi.ey=%d\n", attr->zone_lum_avg_roi.ey);
}

static int PARSE(RoiAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_ROI_ATTR_S *data = (MPI_ROI_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_ROI_ATTR + 2)) {
		opt->action = CMD_ACTION_SET;
		opt->path_idx.dev = atoi(argv[optind]);
		optind++;
		opt->path_idx.path = atoi(argv[optind]);
		optind++;

		data->luma_roi.sx = atoi(argv[optind]);
		optind++;
		data->luma_roi.sy = atoi(argv[optind]);
		optind++;
		data->luma_roi.ex = atoi(argv[optind]);
		optind++;
		data->luma_roi.ey = atoi(argv[optind]);
		optind++;
		data->awb_roi.sx = atoi(argv[optind]);
		optind++;
		data->awb_roi.sy = atoi(argv[optind]);
		optind++;
		data->awb_roi.ex = atoi(argv[optind]);
		optind++;
		data->awb_roi.ey = atoi(argv[optind]);
		optind++;
		data->zone_lum_avg_roi.sx = atoi(argv[optind]);
		optind++;
		data->zone_lum_avg_roi.sy = atoi(argv[optind]);
		optind++;
		data->zone_lum_avg_roi.ex = atoi(argv[optind]);
		optind++;
		data->zone_lum_avg_roi.ey = atoi(argv[optind]);
		optind++;

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

static CMD_S roi_ops = MAKE_CMD("roi", MPI_ROI_ATTR_S, RoiAttr);

__attribute__((constructor)) void regRoiCmd(void)
{
	CMD_register(&roi_ops);
}