#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>

#include "mpi_index.h"
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_osd.h"
#include "mpi_enc.h"
#include "mpi_index.h"

#include "libosd.h"

#define _GNU_SOURCE

#define LINE_THICKNESS_MIDDLE 0

extern int calcNewLine(int startx, int starty, int endx, int endy, int thickness, OsdRegion *targetReg,
                       OsdLine *output);
extern void logAllOsdHandle(OsdHandle *phd);

#define OSD_NUM (4)
OsdHandle *g_osd_handle[3];
OSD_HANDLE g_osd_chn_handle[3][OSD_NUM];
MPI_OSD_CANVAS_ATTR_S g_osd_canvas_attr[3][OSD_NUM];

pthread_t osd_tid;
int gRunflag = 0;
int channel_num = 1;

void help()
{
	printf("usage:\r\n");
	printf("-h help()\r\n");
	printf("-w width 1 - 70 \r\n");
	printf("-c channel to use 0 - 2 (see case config) \r\n");
	return;
}

INT32 createOsdInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle,
                        MPI_OSD_CANVAS_ATTR_S *canvas)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("MPI_createOsdRgn() failed. err: %d", ret);
		return ret;
	}

	ret = MPI_getOsdCanvas(*handle, canvas);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("MPI_getOsdCanvas() failed. err: %d", ret);
		goto release;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("Bind OSD %d to encoder channel %d failed. err: %d", *handle, bind->idx.chn, ret);
		goto release;
	}

	return ret;

release:

	MPI_destroyOsdRgn(*handle);

	return ret;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}
	gRunflag = 0;
}

int main(int argc, char **argv)
{
	int ret = 0;

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	int c = 0;
	int thickness = 1;
	int channel = 0;
	while ((c = getopt(argc, argv, "hc:w:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'w':
			thickness = atoi(argv[optind - 1]);
			if (thickness <= 0) {
				libosd_log_err("invalid thickness: %d", thickness);
				return -EINVAL;
			}
			libosd_log_info("set pt:%d", thickness);
			break;
		case 'c':
			channel = atoi(argv[optind - 1]);
			if ((channel < 0) || (channel > 2)) {
				libosd_log_err("invalid channel: %d", channel);
				return -EINVAL;
			}
			libosd_log_info("set to channel :%d", channel);
			break;
		default:
			help();
			exit(1);
		}
	}

	libosd_log_info("stop ENC0");
	MPI_ECHN echn_idx;
	MPI_SYS_init();

	echn_idx = MPI_ENC_CHN(0);
	echn_idx.chn = channel;

	MPI_OSD_BIND_ATTR_S osd_bind = { { 0 } };
	osd_bind.idx = echn_idx;

	MPI_ENC_CHN_ATTR_S chn_attr;
	ret = MPI_ENC_getChnAttr(echn_idx, &chn_attr);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to get encoder channel %d  rect", MPI_GET_ENC_CHN(echn_idx));
		return -EACCES;
	}

	int chn_width = chn_attr.res.width;
	int chn_height = chn_attr.res.height;
	libosd_log_info("get chn %d res (%d, %d)", MPI_GET_ENC_CHN(echn_idx), chn_width, chn_height);

	OsdRegion region[2] = {
		{ .startX = chn_width / 8, .startY = chn_height / 8, .width = 320, .height = 180 },
		{ .startX = (chn_width / 8) * 4, .startY = chn_height / 8, .width = 320, .height = 180 }
	};

	OsdLine newLIne;
	int startx = chn_width / 8 + 16;
	int starty = chn_height / 8 + 16;
	int endx = startx + 200;
	int endy = starty + 100;
	ret = calcNewLine(startx, starty, endx, endy, thickness, &region[0], &newLIne);
	if (ret != 0) {
		libosd_log_err("failed to add new line");
		return -EINVAL;
	}
	char color[4] = { 0xff, 0xff, 0xff, 0xff };
	color[0] = rand() % 255;
	color[1] = rand() % 255;
	color[2] = rand() % 255;
	memcpy(&newLIne.color[0], &color[0], sizeof(color));
	libosd_log_info("new line (%d, %d) (%d, %d) %d", newLIne.start.x, newLIne.start.y, newLIne.end.x, newLIne.end.y,
	                newLIne.thickness);
	newLIne.mode = AYUV_3544;

	for (int i = 0; i < 2; i++) {
		echn_idx.chn = i;
		ret = MPI_ENC_stopChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Stop encoder channel %d failed.: %d", MPI_GET_ENC_CHN(echn_idx), ret);
			return -EACCES;
		}
	}

	sleep(2);

	OSD_init();
	g_osd_handle[0] = OSD_create(chn_width, chn_height);

	for (int i = 0; i < 2; i++) {
		if (OSD_addOsd(g_osd_handle[0], i, &region[i]) != 0) {
			OSD_destroy(g_osd_handle[0]);
			return -EINVAL;
		}
	}

	if (OSD_calcCanvas(g_osd_handle[0]) != 0) {
		OSD_destroy(g_osd_handle[0]);
		return -EINVAL;
	}
	logAllOsdHandle(g_osd_handle[0]);

	MPI_OSD_RGN_ATTR_S osd_attr = { .show = true,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };
	for (int i = 0; i < 2; i++) {
		osd_attr.size.width = g_osd_handle[0]->canvas[i].width;
		osd_attr.size.height = g_osd_handle[0]->canvas[i].height;
		osd_bind.point.x = g_osd_handle[0]->canvas[i].startX;
		osd_bind.point.y = g_osd_handle[0]->canvas[i].startY;
		createOsdInstance(&osd_attr, &osd_bind, &g_osd_chn_handle[0][i], &g_osd_canvas_attr[0][i]);
	}

	int include_canvas = g_osd_handle[0]->region[0].include_canvas;
	OSD_setRegionTransparent(g_osd_handle[0], 0, AYUV_3544,
	                         (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	OSD_setLine(g_osd_handle[0], 0, &newLIne, (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas. %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	OsdLine newLIne1[4];
	newLIne1[0].mode = AYUV_3544;
	newLIne1[1].mode = AYUV_3544;
	newLIne1[2].mode = AYUV_3544;
	newLIne1[3].mode = AYUV_3544;

	OsdPoint p[4];
	srand(time(NULL));

	p[0].x = region[1].startX + 0;
	p[0].y = region[1].startY + 0;
	p[1].x = region[1].startX + 120;
	p[1].y = region[1].startY + 0;
	for (int i = 2; i < 4; i++) {
		p[i].x = region[1].startX + rand() % (100) + 10;
		p[i].y = region[1].startY + rand() % (100) + 10;
	}

	ret = calcNewLine(p[0].x, p[0].y, p[1].x, p[1].y, thickness, &region[1], &newLIne1[0]);
	if (ret != 0) {
		libosd_log_err("failed to add new line, %d", ret);
		return -EINVAL;
	}

	ret = calcNewLine(p[1].x, p[1].y, p[2].x, p[2].y, thickness, &region[1], &newLIne1[1]);
	if (ret != 0) {
		libosd_log_err("failed to add new line, %d", ret);
		return -EINVAL;
	}

	ret = calcNewLine(p[2].x, p[2].y, p[3].x, p[3].y, thickness, &region[1], &newLIne1[2]);
	if (ret != 0) {
		libosd_log_err("failed to add new line, %d", ret);
		return -EINVAL;
	}

	ret = calcNewLine(p[3].x, p[3].y, p[0].x, p[0].y, thickness, &region[1], &newLIne1[3]);
	if (ret != 0) {
		libosd_log_err("failed to add new line, %d", ret);
		return -EINVAL;
	}

	for (int i = 0; i < 4; i++) {
		color[0] = rand() % 255;
		color[1] = rand() % 255;
		color[2] = rand() % 255;

		memcpy(&newLIne1[i].color[0], &color[0], sizeof(color));
	}

	include_canvas = g_osd_handle[0]->region[1].include_canvas;
	OSD_setRegionTransparent(g_osd_handle[0], 1, AYUV_3544,
	                         (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	OSD_setLine(g_osd_handle[0], 1, &newLIne1[0], (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
	OSD_setLine(g_osd_handle[0], 1, &newLIne1[1], (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
	OSD_setLine(g_osd_handle[0], 1, &newLIne1[2], (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
	OSD_setLine(g_osd_handle[0], 1, &newLIne1[3], (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]) if (ret != MPI_SUCCESS) if (ret != 0)
	{
		libosd_log_err("failed to update canvas, ret:%d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	libosd_log_info("resume ENC0 & 1");
	MPI_ENC_BIND_INFO_S bind_info;
	for (int i = 0; i < 2; i++) {
		bind_info.idx = MPI_VIDEO_CHN(0, i);
		echn_idx.chn = i;

		ret = MPI_ENC_bindToVideoChn(echn_idx, &bind_info);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Bind encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
			return MPI_FAILURE;
		}

		ret = MPI_ENC_startChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Start encoder channel %d failed. %d", MPI_GET_ENC_CHN(echn_idx), ret);
			return MPI_FAILURE;
		}
	}

	gRunflag = 1;
	while (gRunflag) {

		p[0].x = region[1].startX + 45;
		p[0].y = region[1].startY + 45;
		for (int i = 1; i < 4; i++) {
			p[i].x = region[1].startX + rand() % (100) + 10;
			p[i].y = region[1].startY + rand() % (100) + 10;
		}

		ret = calcNewLine(p[0].x, p[0].y, p[1].x, p[1].y, thickness, &region[1], &newLIne1[0]);
		if (ret != 0) {
			libosd_log_err("failed to add new line");
			return -EINVAL;
		}
		ret = calcNewLine(p[1].x, p[1].y, p[2].x, p[2].y, thickness, &region[1], &newLIne1[1]);
		if (ret != 0) {
			libosd_log_err("failed to add new line");
			return -EINVAL;
		}
		ret = calcNewLine(p[2].x, p[2].y, p[3].x, p[3].y, thickness, &region[1], &newLIne1[2]);
		if (ret != 0) {
			libosd_log_err("failed to add new line");
			return -EINVAL;
		}
		ret = calcNewLine(p[3].x, p[3].y, p[0].x, p[0].y, thickness, &region[1], &newLIne1[3]);
		if (ret != 0) {
			libosd_log_err("failed to add new line");
			return -EINVAL;
		}

		for (int i = 0; i < 4; i++) {
			color[0] = rand() % 255;
			color[1] = rand() % 255;
			color[2] = rand() % 255;
			memcpy(&newLIne1[i].color[0], &color[0], sizeof(color));
		}

		include_canvas = g_osd_handle[0]->region[1].include_canvas;
		OSD_setRegionTransparent(g_osd_handle[0], 1, AYUV_3544,
		                         (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

		OSD_setLine(g_osd_handle[0], 1, &newLIne1[0],
		            (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
		OSD_setLine(g_osd_handle[0], 1, &newLIne1[1],
		            (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
		OSD_setLine(g_osd_handle[0], 1, &newLIne1[2],
		            (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
		OSD_setLine(g_osd_handle[0], 1, &newLIne1[3],
		            (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

		if (MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]) != MPI_SUCCESS) {
			libosd_log_err("failed to update canvas");
			MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
		}

		sleep(3);
	}

	libosd_log_info("start unbind & destroy canvas");

	libosd_log_info("stop all ENC");

	for (int i = 0; i < 2; i++) {
		echn_idx.chn = i;
		ret = MPI_ENC_stopChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Stop encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
			return -EACCES;
		}
	}

	sleep(2);

	libosd_log_info("remove all OSD");
	/*remove all old osd canvas*/
	for (int i = 0; i < 2; i++) {
		if (MPI_unbindOsdFromChn(g_osd_chn_handle[0][i], &osd_bind) != MPI_SUCCESS) {
			libosd_log_err("failed to unbind chn, %d", ret);
		}

		if (MPI_destroyOsdRgn(g_osd_chn_handle[0][i]) != MPI_SUCCESS) {
			libosd_log_err("failed to unbind chn, %d", ret);
		}
	}

	OSD_destroy(g_osd_handle[0]);
	OSD_deinit();

	libosd_log_info("resume ENC0 & 1");
	for (int i = 0; i < 2; i++) {
		bind_info.idx = MPI_VIDEO_CHN(0, i);
		echn_idx.chn = i;

		ret = MPI_ENC_bindToVideoChn(echn_idx, &bind_info);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Bind encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
			return MPI_FAILURE;
		}

		ret = MPI_ENC_startChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Start encoder channel %d failed. %d", MPI_GET_ENC_CHN(echn_idx), ret);
			return MPI_FAILURE;
		}
	}

	MPI_SYS_exit();

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif