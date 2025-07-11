#include "cmdparser.h"

#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mpi_dev.h"

#include "cmd_util.h"

#define MIN_NUM_STITCH_ATTR (1)

typedef enum opt_val {
	ENABLE = 100,
	CENTER0,
	CENTER1,
	DFT_DIST,
	TABLE_NUM,
	DIST0 = 200,
	VER_DISP0,
	SRC_ZOOM0,
	THETA0,
	RADIUS0,
	CURVATURE0,
	FOV_RATIO0,
	VER_SCALE0,
	VER_SHIFT0,
	DIST1 = 210,
	VER_DISP1,
	SRC_ZOOM1,
	THETA1,
	RADIUS1,
	CURVATURE1,
	FOV_RATIO1,
	VER_SCALE1,
	VER_SHIFT1,
	DIST2 = 220,
	VER_DISP2,
	SRC_ZOOM2,
	THETA2,
	RADIUS2,
	CURVATURE2,
	FOV_RATIO2,
	VER_SCALE2,
	VER_SHIFT2,
} OptVal;

static INT32 GET(WinStitch)(CMD_DATA_S *opt)
{
	return MPI_DEV_getStitchAttr(opt->win_idx, opt->data);
}

static INT32 SET(WinStitch)(const CMD_DATA_S *opt)
{
	return MPI_DEV_setStitchAttr(opt->win_idx, opt->data);
}

static void ARGS(WinStitch)(void)
{
	printf("\t%-48s %s\n", "--enable=[0, 1]", "enable\n");

	for (int i = 0; i < MPI_STITCH_SENSOR_NUM; i++) {
		printf("--center%d=<x>,<y>\tcenter[%d] point\n", i, i);
	}

	printf("\n--dft_dist=[dft_dist]\tdft_dist\n");
	printf("--table_num=[table_num]\ttable_num\n");

	for (int i = 0; i < MPI_STITCH_TABLE_NUM; i++) {
		printf("\n\n--dist%d=[dist]\t\ttable[%d].dist\n", i, i);
		printf("--ver_disp%d=[ver_disp]\ttable[%d].ver_disp\n", i, i);
		printf("--src_zoom%d=[src_zoom]\ttable[%d].src_zoom\n", i, i);
		printf("below options max %d params number:\n", MPI_STITCH_SENSOR_NUM);
		printf("\t--theta%d=[theta[0],theta[1]...]\t\t\ttable[%d].theta\n", i, i);
		printf("\t--radius%d=[radius[0],radius[1]...]\t\ttable[%d].radius\n", i, i);
		printf("\t--curvature%d=[curvature[0],curvature[1]...]\ttable[%d].curvature\n", i, i);
		printf("\t--fov_ratio%d=[theta[0],theta[1]...]\t\ttable[%d].fov_ratio\n", i, i);
		printf("\t--ver_scale%d=[ver_scale[0],ver_scale[1]...]\ttable[%d].ver_scale\n", i, i);
		printf("\t--ver_shift%d=[ver_shift[0],ver_shift[1]...]\ttable[%d].ver_shift\n", i, i);
	}
}

static void HELP(WinStitch)(const char *str)
{
	CMD_PRINT_HELP(str, "'--winStitch <MPI_WIN> [STITCH_ATTR]'", "Set STITCH attributes");
}

static void SHOW(WinStitch)(const CMD_DATA_S *opt)
{
	MPI_STITCH_ATTR_S *attr = (MPI_STITCH_ATTR_S *)opt->data;

	printf("device index: %d, channel index: %d, window index: %d\n", opt->win_idx.dev, opt->win_idx.chn,
	       opt->win_idx.win);
	printf("enable=%d\n", attr->enable);

	for (int i = 0; i < MPI_STITCH_SENSOR_NUM; i++) {
		printf("center[%d].x=%d\n", i, attr->center[i].x);
		printf("center[%d].y=%d\n", i, attr->center[i].y);
	}

	printf("dft_dist=%d\n", attr->dft_dist);
	printf("table_num=%d\n", attr->table_num);

	for (int i = 0; i < attr->table_num; i++) {
		printf("table[%d]:\n", i);
		printf("\tdist=%d\n", attr->table[i].dist);
		printf("\tver_disp=%d\n", attr->table[i].ver_disp);
		printf("\tsrc_zoom=%d\n", attr->table[i].src_zoom);

		for (int j = 0; j < MPI_STITCH_SENSOR_NUM; j++) {
			printf("\t\ttheta[%d]=%d\n", j, attr->table[i].theta[j]);
			printf("\t\tradius[%d]=%d\n", i, attr->table[i].radius[j]);
			printf("\t\tcurvature[%d]=%d\n", j, attr->table[i].curvature[j]);
			printf("\t\tfov_ratio[%d]=%d\n", j, attr->table[i].fov_ratio[j]);
			printf("\t\tver_scale[%d]=%d\n", j, attr->table[i].ver_scale[j]);
			printf("\t\tver_shift[%d]=%d\n", j, attr->table[i].ver_shift[j]);
		}
	}
}

static int parseStitchArgs(CMD_DATA_S *opt, int argc, char **str, MPI_STITCH_ATTR_S *attr)
{
	int c;
	int ret = 0;
	char *del = (char *)",";
	char *p = NULL;

	ret = MPI_DEV_getStitchAttr(opt->win_idx, attr);
	if (ret) {
		fprintf(stderr, "failed to get stitch attr, ret: %d\n", ret);
		return ret;
	}

	// clang-format off
	struct option opts[] = {
		{"enable", 1, NULL, ENABLE},
		{"center0", 1, NULL, CENTER0}, {"center1", 1, NULL, CENTER1},
		{"dft_dist", 1, NULL, DFT_DIST},
		{"table_num", 1, NULL, TABLE_NUM },

		{"dist0", 1, NULL, DIST0},
		{"ver_disp0", 1, NULL, VER_DISP0},
		{"src_zoom0", 1, NULL, SRC_ZOOM0},
		{"theta0", 1, NULL, THETA0},
		{"radius0", 1, NULL, RADIUS0},
		{"curvature0", 1, NULL, CURVATURE0},
		{"fov_ratio0", 1, NULL, FOV_RATIO0},
		{"ver_scale0", 1, NULL, VER_SCALE0},
		{"ver_shift0", 1, NULL, VER_SHIFT0},

		{"dist1", 1, NULL, DIST1},
		{"ver_disp1", 1, NULL, VER_DISP1},
		{"src_zoom1", 1, NULL, SRC_ZOOM1},
		{"theta1", 1, NULL, THETA1},
		{"radius1", 1, NULL, RADIUS1},
		{"curvature1", 1, NULL, CURVATURE1},
		{"fov_ratio1", 1, NULL, FOV_RATIO1},
		{"ver_scale1", 1, NULL, VER_SCALE1},
		{"ver_shift1", 1, NULL, VER_SHIFT1},

		{"dist2", 1, NULL, DIST2},
		{"ver_disp2", 1, NULL, VER_DISP2},
		{"src_zoom2", 1, NULL, SRC_ZOOM2},
		{"theta2", 1, NULL, THETA2},
		{"radius2", 1, NULL, RADIUS2},
		{"curvature2", 1, NULL, CURVATURE2},
		{"fov_ratio2", 1, NULL, FOV_RATIO2},
		{"ver_scale2", 1, NULL, VER_SCALE2},
		{"ver_shift2", 1, NULL, VER_SHIFT2},
	};
	// clang-format on

	while ((c = getopt_long(argc, str, ":", opts, NULL)) != -1) {
		printf("c: %d, opt: %s\n", c, optarg);
		switch (c) {
		case ENABLE:
			attr->enable = atoi(optarg);
			break;
		case CENTER0:
			p = strtok(optarg, del);
			attr->center[0].x = atoi(p);
			p = strtok(NULL, del);
			attr->center[0].y = atoi(p);
			break;
		case CENTER1:
			p = strtok(optarg, del);
			attr->center[1].x = atoi(p);
			p = strtok(NULL, del);
			attr->center[1].y = atoi(p);
			break;
		case DFT_DIST:
			attr->dft_dist = atoi(optarg);
			break;
		case TABLE_NUM:
			if (atoi(optarg) > MPI_STITCH_TABLE_NUM) {
				fprintf(stderr, "table num should less then:%d, input %d\n", MPI_STITCH_TABLE_NUM,
				        atoi(optarg));
				break;
			}
			attr->table_num = atoi(optarg);
			break;
		case DIST0: /** MPI_STITCH_DIST_S  args */
			attr->table[0].dist = atoi(optarg);
			break;
		case VER_DISP0:
			attr->table[0].ver_disp = atoi(optarg);
			break;
		case SRC_ZOOM0:
			attr->table[0].src_zoom = atoi(optarg);
			break;
		case THETA0:
			p = strtok(optarg, del);
			attr->table[0].theta[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[0].theta[1] = atoi(p);
			break;
		case RADIUS0:
			p = strtok(optarg, del);
			attr->table[0].radius[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[0].radius[1] = atoi(p);
			break;
		case CURVATURE0:
			p = strtok(optarg, del);
			attr->table[0].curvature[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[0].curvature[1] = atoi(p);
			break;
		case FOV_RATIO0:
			p = strtok(optarg, del);
			attr->table[0].fov_ratio[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[0].fov_ratio[1] = atoi(p);
			break;
		case VER_SCALE0:
			p = strtok(optarg, del);
			attr->table[0].ver_scale[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[0].ver_scale[1] = atoi(p);
			break;
		case VER_SHIFT0:
			p = strtok(optarg, del);
			attr->table[0].ver_shift[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[0].ver_shift[1] = atoi(p);
			break;
		case DIST1:
			attr->table[1].dist = atoi(optarg);
			break;
		case VER_DISP1:
			attr->table[1].ver_disp = atoi(optarg);
			break;
		case SRC_ZOOM1:
			attr->table[1].src_zoom = atoi(optarg);
			break;
		case THETA1:
			p = strtok(optarg, del);
			attr->table[1].theta[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[1].theta[1] = atoi(p);
			break;
		case RADIUS1:
			p = strtok(optarg, del);
			attr->table[1].radius[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[1].radius[1] = atoi(p);
			break;
		case CURVATURE1:
			p = strtok(optarg, del);
			attr->table[1].curvature[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[1].curvature[1] = atoi(p);
			break;
		case FOV_RATIO1:
			p = strtok(optarg, del);
			attr->table[1].fov_ratio[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[1].fov_ratio[1] = atoi(p);
			break;
		case VER_SCALE1:
			p = strtok(optarg, del);
			attr->table[1].ver_scale[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[1].ver_scale[1] = atoi(p);
			break;
		case VER_SHIFT1:
			p = strtok(optarg, del);
			attr->table[1].ver_shift[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[1].ver_shift[1] = atoi(p);
			break;
		case DIST2:
			attr->table[2].dist = atoi(optarg);
			break;
		case VER_DISP2:
			attr->table[2].ver_disp = atoi(optarg);
			break;
		case SRC_ZOOM2:
			attr->table[2].src_zoom = atoi(optarg);
			break;
		case THETA2:
			p = strtok(optarg, del);
			attr->table[2].theta[0] = atoi(p);
			p = strtok(optarg, del);
			attr->table[2].theta[1] = atoi(p);
			break;
		case RADIUS2:
			p = strtok(optarg, del);
			attr->table[2].radius[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[2].radius[1] = atoi(p);
			break;
		case CURVATURE2:
			p = strtok(optarg, del);
			attr->table[2].curvature[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[2].curvature[1] = atoi(p);
			break;
		case FOV_RATIO2:
			p = strtok(optarg, del);
			attr->table[2].fov_ratio[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[2].fov_ratio[1] = atoi(p);
			break;
		case VER_SCALE2:
			p = strtok(optarg, del);
			attr->table[2].ver_scale[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[2].ver_scale[1] = atoi(p);
			break;
		case VER_SHIFT2:
			p = strtok(optarg, del);
			attr->table[2].ver_shift[0] = atoi(p);
			p = strtok(NULL, del);
			attr->table[2].ver_shift[1] = atoi(p);
			break;
		default:
			fprintf(stderr, "Unknown param: %s\n", optarg);
			ret = -EINVAL;
			break;
		}
	}

	return ret;
}

static int PARSE(WinStitch)(int argc, char **argv, CMD_DATA_S *opt)
{
	MPI_STITCH_ATTR_S *attr = (MPI_STITCH_ATTR_S *)opt->data;
	int num = argc - optind;

	if (num >= 3 + MIN_NUM_STITCH_ATTR) {
		opt->action = CMD_ACTION_SET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);

		if (parseStitchArgs(opt, argc, argv, attr) != 0) {
			return -EINVAL;
		}

	} else if (num == 3) {
		opt->action = CMD_ACTION_GET;
		opt->win_idx.dev = atoi(argv[optind++]);
		opt->win_idx.chn = atoi(argv[optind++]);
		opt->win_idx.win = atoi(argv[optind++]);
	} else {
		return -EINVAL;
	}

	return 0;
}

static CMD_S winstitch_ops = MAKE_CMD("winstitch", MPI_STITCH_ATTR_S, WinStitch);

__attribute__((constructor)) void regWinStitchCmd(void)
{
	CMD_register(&winstitch_ops);
}
