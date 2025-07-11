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
#include "mpi_index.h"

#include "SDL.h"
#include "SDL_video.h"
#include "SDL_ttf.h"

#include "libosd.h"
#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
extern int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol);
extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);

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
	printf("-s text size to use 1 - 100\r\n");
	printf("-c channel to use 0 - 2 (see case config) \r\n");
	printf("-w outline width in pixel dft :1 \r\n");
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

	int size = 30;
	int c = 0;
	int channel = 0;
	int ret = 0;
	int outline_width = 1;
	while ((c = getopt(argc, argv, "hs:c:w:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 's':
			size = atoi(argv[optind - 1]);
			if ((size < 0) || (size > 100)) {
				libosd_log_err("invalid size: %d", size);
				return -EINVAL;
			}
			libosd_log_info("set font size :%d", size);
			break;
		case 'w':
			outline_width = atoi(argv[optind - 1]);
			if ((outline_width < 0) || (outline_width > 100)) {
				libosd_log_err("invalid outline_width: %d", outline_width);
				return -EINVAL;
			}
			libosd_log_info("set outline_width size :%d", outline_width);
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
		libosd_log_err("failed to get encoder channel %d  rect, %d", MPI_GET_ENC_CHN(echn_idx), ret);
		return -EACCES;
	}

	int chn_width = chn_attr.res.width;
	int chn_height = chn_attr.res.height;
	libosd_log_info("get chn %d res (%d, %d)", MPI_GET_ENC_CHN(echn_idx), chn_width, chn_height);

	for (int i = 0; i < 2; i++) {
		echn_idx.chn = i;
		ret = MPI_ENC_stopChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Stop encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
			return -EACCES;
		}
	}

	sleep(2);

	OSD_init();
	g_osd_handle[0] = OSD_create(chn_width, chn_height);

	OsdRegion region[3] = {
		{ .startX = 16, .startY = 16, .width = 320, .height = 160 },
		{ .startX = chn_width - 160, .startY = 16, .width = 160, .height = 48 },
		{ .startX = 16, .startY = chn_height / 2, .width = chn_width / 2 - 100, .height = 118 }
	};
	for (int j = 0; j < 3; j++) {
		if (OSD_addOsd(g_osd_handle[0], j, &region[j]) != 0) {
			OSD_destroy(g_osd_handle[0]);
		}
	}

	if (OSD_calcCanvas(g_osd_handle[0]) != 0) {
		OSD_destroy(g_osd_handle[0]);
	}

	MPI_OSD_RGN_ATTR_S osd_attr = { .show = true,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };

	for (int i = 0; i < 3; i++) {
		osd_attr.size.width = g_osd_handle[0]->canvas[i].width;
		osd_attr.size.height = g_osd_handle[0]->canvas[i].height;
		osd_bind.point.x = g_osd_handle[0]->canvas[i].startX;
		osd_bind.point.y = g_osd_handle[0]->canvas[i].startY;
		createOsdInstance(&osd_attr, &osd_bind, &g_osd_chn_handle[0][i], &g_osd_canvas_attr[0][i]);
	}

	OsdText txt = {
		.size = size, .background = WHITE, .color = { 0xff, 0xff, 0xff }, .outline_color = { 0x00, 0x00, 0x00 }
	};

	snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansTC-Normal.otf");
	char text[64];
	txt.outline_width = outline_width;
	snprintf(&text[0], 64, "Camera 0%d", 1);
	memcpy(&txt.txt[0], &text[0], sizeof(text));

	OSD_setTextUTF8(g_osd_handle[0], 0, &txt, (char *)(g_osd_canvas_attr[0][0].canvas_addr));
	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][0]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][0]);
	}

	time_t t = { 0 };
	struct tm tm = { 0 };
	uint16_t unicode_list[31];

	t = time(NULL);
	tm = *localtime(&t);

	snprintf(&text[0], 64, "   %d-%02d-%02d  ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
	for (int i = 0; i < 15; i++) {
		unicode_list[i] = OSD_trans2Unicode(text[i]);
	}

	memcpy(&unicode_list[15], &unicode_by_day[tm.tm_wday][0], sizeof(uint16_t) * 3);

	snprintf(&text[0], 64, "  %02d:%02d:%02d   ", tm.tm_hour, tm.tm_min, tm.tm_sec);

	for (int i = 0; i < 13; i++) {
		unicode_list[15 + 3 + i] = OSD_trans2Unicode(text[i]);
	}

	memset(&txt.unicode_txt[0], 0x00, sizeof(txt.unicode_txt));
	memcpy(&txt.unicode_txt[0], &unicode_list[0], sizeof(unicode_list));

	OSD_setTextUnicode(g_osd_handle[0], 2, &txt, (char *)(g_osd_canvas_attr[0][2].canvas_addr));
	ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][2]);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to update canvas, %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[0][2]);
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
			libosd_log_err("Start encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx), ret);
			return MPI_FAILURE;
		}
	}
	gRunflag = 1;

	while (gRunflag) {
		sleep(1);
	}

	libosd_log_info("start unbind & destroy canvas");

	libosd_log_info("stop all ENC");
	for (int i = 0; i < 2; i++) {
		echn_idx.chn = i;
		ret = MPI_ENC_stopChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("Stop encoder channel %d failed.%d", MPI_GET_ENC_CHN(echn_idx, ret);
			return -EACCES;
		}
	}

	sleep(2);

	/*remove all old osd canvas*/
	libosd_log_info("remove all OSD");
	for (int i = 0; i < 3; i++) {
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
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif