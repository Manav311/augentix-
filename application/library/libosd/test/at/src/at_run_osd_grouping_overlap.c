#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

//#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>

#include "mpi_index.h"
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_osd.h"
#include "mpi_enc.h"
#include "mpi_dev.h"
#include "mpi_index.h"

#include "libosd.h"

extern void logAllOsdHandle(OsdHandle *phd);

#define OSD_NUM (4)
OsdHandle *g_osd_handle[3];
OSD_HANDLE g_osd_chn_handle[3][OSD_NUM];
MPI_OSD_CANVAS_ATTR_S g_osd_canvas_attr[3][OSD_NUM];

pthread_t osd_tid;
int gRunflag = 0;
int channel_num = 1;

uint16_t unicode_by_day[7][3] = { { 0x661f, 0x671f, 0x5929 }, { 0x661f, 0x671f, 0x4e00 }, { 0x661f, 0x671f, 0x4e8c },
	                          { 0x661f, 0x671f, 0x4e09 }, { 0x661f, 0x671f, 0x56db }, { 0x661f, 0x671f, 0x4e94 },
	                          { 0x661f, 0x671f, 0x516d } };

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

int stopEnc(int channel)
{
	MPI_ECHN echn_idx;
	int ret;

	echn_idx = MPI_ENC_CHN(0);
	echn_idx.chn = channel;

	ret = MPI_ENC_stopChn(echn_idx);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("Stop encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
		return -EACCES;
	}

	return 0;
}

int resumeEnc(int channel)
{
	int ret = 0;
	MPI_ECHN echn_idx;
	MPI_ENC_BIND_INFO_S bind_info;

	bind_info.idx = MPI_VIDEO_CHN(0, channel);
	echn_idx.chn = channel;

	ret = MPI_ENC_bindToVideoChn(echn_idx, &bind_info);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("Bind encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
		return MPI_FAILURE;
	}

	ret = MPI_ENC_startChn(echn_idx);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("Start encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
		return MPI_FAILURE;
	}

	return 0;
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

void help()
{
	printf("usage:\r\n");
	printf("-h help()\r\n");
	printf("-c channel to use 0 - 2 (see case config) \r\n");
	return;
}

int main(int argc, char **argv)
{
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
	int channel = 0;
	while ((c = getopt(argc, argv, "hc:w:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
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

	int ret = 0;
	MPI_SYS_init();

	MPI_CHN chn_idx = MPI_VIDEO_CHN(0, channel);
	MPI_CHN_STAT_S stat;
	stat.status = MPI_STATE_NONE;
	ret = MPI_DEV_queryChnState(chn_idx, &stat);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to find chn %d, %d", channel, ret);
		return -EINVAL;
	}
	if (stat.status == MPI_STATE_NONE) {
		libosd_log_err("failed, chn %d not exist", channel);
		return -EINVAL;
	}

	MPI_ECHN echn_idx;

	echn_idx = MPI_ENC_CHN(0);
	echn_idx.chn = channel;

	MPI_OSD_BIND_ATTR_S osd_bind = { { 0 } };
	osd_bind.idx = echn_idx;

	MPI_ENC_CHN_ATTR_S chn_attr;
	ret = MPI_ENC_getChnAttr(echn_idx, &chn_attr);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to get encoder channel %d rect, %d", MPI_GET_ENC_CHN(echn_idx), ret);
		return -EACCES;
	}

	int chn_width = chn_attr.res.width;
	int chn_height = chn_attr.res.height;
	libosd_log_info("get chn %d res (%d, %d)", MPI_GET_ENC_CHN(echn_idx), chn_width, chn_height);

	libosd_log_info("stop ENC0");
	if (channel == 1) {
		stopEnc(0);
	}
	stopEnc(channel);

	OSD_init();
	g_osd_handle[0] = OSD_create(chn_width, chn_height);

	int block_width = chn_width / 4 - 16;
	int block_height = chn_height / 4 - 16;

	OsdRegion region[7] = {
		{ .startX = 0, .startY = 0, .width = block_width, .height = block_height },
		{ .startX = block_width / 2, .startY = block_height, .width = block_width, .height = block_height },
		{ .startX = block_width * 2, .startY = 0, .width = block_width - 96, .height = block_height },
		{ .startX = block_width * 2 + 320, .startY = 320, .width = block_width, .height = block_height },
		{ .startX = block_width * 3,
		  .startY = block_height * 3 - 80,
		  .width = block_width,
		  .height = block_height },
		{ .startX = 32, .startY = 600, .width = 160, .height = 80 },
		{ .startX = 112, .startY = 640, .width = 160, .height = 80 }
	};

	for (int i = 0; i < 7; i++) {
		libosd_log_info("add[%d]", i);
		ret = OSD_addOsd(g_osd_handle[0], i, &region[i]);
		if (ret != 0) {
			libosd_log_err("failed to add:%d osd, %d", i, ret);
			OSD_destroy(g_osd_handle[0]);
		}
	}

	OSD_calcCanvasbygroup(g_osd_handle[0]);
	logAllOsdHandle(g_osd_handle[0]);

	MPI_OSD_RGN_ATTR_S osd_attr = { .show = true,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };
	for (int i = 0; i < 4; i++) {
		osd_attr.size.width = g_osd_handle[0]->canvas[i].width;
		osd_attr.size.height = g_osd_handle[0]->canvas[i].height;
		osd_bind.point.x = g_osd_handle[0]->canvas[i].startX;
		osd_bind.point.y = g_osd_handle[0]->canvas[i].startY;
		createOsdInstance(&osd_attr, &osd_bind, &g_osd_chn_handle[0][i], &g_osd_canvas_attr[0][i]);
	}

	int include_canvas = g_osd_handle[0]->region[0].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 0, "/mnt/nfs/ethnfs/grass-l.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	include_canvas = g_osd_handle[0]->region[1].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 1, "/mnt/nfs/ethnfs/pink-l.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	include_canvas = g_osd_handle[0]->region[2].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 2, "/mnt/nfs/ethnfs/purple-l.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	include_canvas = g_osd_handle[0]->region[3].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 3, "/mnt/nfs/ethnfs/orange-l.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	include_canvas = g_osd_handle[0]->region[4].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 4, "/mnt/nfs/ethnfs/gray-l.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));
	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	include_canvas = g_osd_handle[0]->region[5].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 5, "/mnt/nfs/ethnfs/red.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	include_canvas = g_osd_handle[0]->region[6].include_canvas;
	OSD_setImageBmp(g_osd_handle[0], 6, "/mnt/nfs/ethnfs/green.bmp",
	                (char *)(g_osd_canvas_attr[0][include_canvas].canvas_addr));

	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][include_canvas]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", reet);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][include_canvas]);
	}

	if (channel == 1) {
		libosd_log_info("resume ENC0");
		resumeEnc(0);
	}
	libosd_log_info("resume ENC%d", channel);
	resumeEnc(channel);

	gRunflag = 1;
	while (gRunflag) {
		sleep(1);
	}

	libosd_log_info("remove all OSD");
	libosd_log_info("stop ENC %d", channel);
	if (channel == 1) {
		stopEnc(0);
	}
	stopEnc(channel);


	sleep(2);

	libosd_log_info("start unbind & destroy canvas");
	/*remove all old osd canvas*/
	for (int i = 0; i < 4; i++) {
		if (MPI_unbindOsdFromChn(g_osd_chn_handle[0][i], &osd_bind) != MPI_SUCCESS) {
			libosd_log_err("failed to unbind chn, %d", ret);
		}

		if (MPI_destroyOsdRgn(g_osd_chn_handle[0][i]) != MPI_SUCCESS) {
			libosd_log_err("failed to unbind chn, %d", ret);
		}
	}

	OSD_destroy(g_osd_handle[0]);
	OSD_deinit();


	if (channel == 1) {
		libosd_log_info("resume ENC0");
		resumeEnc(0);
	}
	libosd_log_info("resume ENC %d", channel);
	resumeEnc(channel);

	MPI_SYS_exit();

	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
