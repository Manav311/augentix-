#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define NUM_WIN_ATTR (2)

static INT32 GET(WinAttr)(CMD_DATA_S *opt)
{
	return MPI_DEV_getWindowAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinAttr)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setWindowAttr(opt->win_idx, opt->data);
}

static void ARGS(WinAttr)(void)
{
	printf("\t'--win dev_idx chn_idx win_idx'\n");
	printf("\t'--win 0 0 0'\n");
}

static void HELP(WinAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--win <MPI_WIN>'", "Query window attributes");
}

static char *getViewtypeName(MPI_WIN_VIEW_TYPE_E view_type)
{
	switch (view_type) {
	case MPI_WIN_VIEW_TYPE_NORMAL:
		return "NORMAL";
	case MPI_WIN_VIEW_TYPE_LDC:
		return "LDC";
	case MPI_WIN_VIEW_TYPE_PANORAMA:
		return "PANORAMA";
	case MPI_WIN_VIEW_TYPE_PANNING:
		return "PANNING";
	case MPI_WIN_VIEW_TYPE_SURROUND:
		return "SURROUND";
	case MPI_WIN_VIEW_TYPE_STITCH:
		return "STITCH";
	case MPI_WIN_VIEW_TYPE_GRAPHICS:
		return "GRAPHICS";
	default:
		break;
	}

	return NULL;
}

static char *getRotateName(MPI_ROTATE_TYPE_E rotate)
{
	switch (rotate) {
	case MPI_ROTATE_0:
		return "MPI_ROTATE_0";
	case MPI_ROTATE_90:
		return "MPI_ROTATE_90";
	case MPI_ROTATE_180:
		return "MPI_ROTATE_180";
	case MPI_ROTATE_270:
		return "MPI_ROTATE_270";
	default:
		break;
	}
	return NULL;
}

static void SHOW(WinAttr)(const CMD_DATA_S *opt)
{
	MPI_WIN_ATTR_S *attr = (MPI_WIN_ATTR_S *)opt->data;

	PRINT_WIN(opt->win_idx);
	printf("fps=%f\n", attr->fps);
	printf("rotate=%d,(%s)\n", attr->rotate, getRotateName(attr->rotate));
	printf("mirror=%d, flip=%d\n", attr->mirr_en, attr->flip_en);
	printf("eis_en=%d\n", attr->eis_en);
	printf("viewtype=%d %s\n", attr->view_type, getViewtypeName(attr->view_type));
	printf("roi=(%d, %d, %d, %d)\n", attr->roi.x, attr->roi.y, attr->roi.width, attr->roi.height);
	printf("prio=%d\n", attr->prio);
	printf("src_id=");
	PRINT_WIN(attr->src_id);
	printf("const_qual=%d\n", attr->const_qual);
	printf("dyn_adj=%d\n", attr->dyn_adj);
}

static int PARSE(WinAttr)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_WIN_ATTR_S *data = (MPI_WIN_ATTR_S *)opt->data;
	MPI_CHN_LAYOUT_S attr;
	int num = argc - optind;
	int ret = 0;
	if (num == (NUM_WIN_ATTR + 3)) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		if ((ret = MPI_DEV_getWindowAttr(opt->win_idx, data))) {
			return ret;
		}

	} else if (num == 3) {
		opt->action = CMD_ACTION_GET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		ret = MPI_DEV_getChnLayout(MPI_VIDEO_CHN(opt->win_idx.dev, opt->win_idx.chn), &attr);
		if (ret != 0) {
			return ret;
		}

		if (opt->win_idx.win >= attr.window_num) {
			return -EACCES;
		}

	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S win_ops = MAKE_CMD("win", MPI_WIN_ATTR_S, WinAttr);

__attribute__((constructor)) void regWinCmd(void)
{
	CMD_register(&win_ops);
}
