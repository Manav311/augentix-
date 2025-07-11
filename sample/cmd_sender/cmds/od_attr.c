#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_iva.h"

#include "cmd_util.h"

#define NUM_OD_PARAM (4)

static INT32 GET(OdAttr)(CMD_DATA_S *opt)
{
	return MPI_IVA_getObjParam(opt->win_idx, opt->data);
}

static INT32 SET(OdAttr)(const CMD_DATA_S *opt)
{
	return MPI_IVA_setObjParam(opt->win_idx, opt->data);
}

static void ARGS(OdAttr)(void)
{
	printf("\t'--od dev_idx chn_idx win_idx od_qual od_track_refine od_size_th od_sen'\n");
	printf("\t'--od 0 0 0 53 16 252'\n");
}

static void HELP(OdAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--od <MPI_WIN> [OD_ATTR]'", "Set object detection parameters");
}

static void SHOW(OdAttr)(const CMD_DATA_S *opt)
{
	MPI_IVA_OD_PARAM_S *attr = (MPI_IVA_OD_PARAM_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("od_qual=%d\n", attr->od_qual);
	printf("od_track_refine=%d\n", attr->od_track_refine);
	printf("od_size_th=%d\n", attr->od_size_th);
	printf("od_sen=%d\n", attr->od_sen);

	// Add 5 new OD parameters
	printf("od_conf_th=%d\n", attr->od_conf_th);
	printf("od_iou_th=%d\n", attr->od_iou_th);
	printf("od_snapshot_w=%d\n", attr->od_snapshot_w);
	printf("od_snapshot_h=%d\n", attr->od_snapshot_h);
	printf("od_snapshot_type=%d\n", attr->od_snapshot_type);
}

static int PARSE(OdAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_IVA_OD_PARAM_S *data = (MPI_IVA_OD_PARAM_S *)opt->data;
	int num = argc - optind;

	if (num == (NUM_OD_PARAM + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind]);
		optind++;
		opt->win_idx.chn = atoi(argv[optind]);
		optind++;
		opt->win_idx.win = atoi(argv[optind]);
		optind++;

		data->od_qual = atoi(argv[optind]);
		optind++;
		data->od_track_refine = atoi(argv[optind]);
		optind++;
		data->od_size_th = atoi(argv[optind]);
		optind++;
		data->od_sen = atoi(argv[optind]);
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

static CMD_S od_ops = MAKE_CMD("od", MPI_IVA_OD_PARAM_S, OdAttr);

__attribute__((constructor)) void regOdCmd(void)
{
	CMD_register(&od_ops);
}
