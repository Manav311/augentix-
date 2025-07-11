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

#include "SDL.h"
#include "SDL_video.h"
#include "SDL_ttf.h"

#include "libosd.h"

char g_PALETTE_8_mode_palatte_rgb[32][3] = {
	{ 0, 0, 0 },       { 0, 0, 170 },     { 0, 170, 0 },     { 0, 170, 170 },   { 170, 0, 0 },
	{ 170, 0, 170 },   { 170, 85, 0 },    { 170, 170, 170 }, { 85, 85, 85 },    { 85, 85, 255 },
	{ 85, 225, 85 },   { 85, 255, 255 },  { 255, 85, 85 },   { 255, 85, 255 },  { 255, 255, 85 },
	{ 255, 255, 255 }, { 0, 0, 0 },       { 20, 20, 20 },    { 32, 32, 32 },    { 44, 44, 44 },
	{ 56, 56, 56 },    { 69, 69, 69 },    { 81, 81, 81 },    { 97, 97, 97 },    { 113, 113, 113 },
	{ 130, 130, 130 }, { 146, 146, 146 }, { 162, 162, 162 }, { 182, 182, 182 }, { 203, 203, 203 },
	{ 227, 227, 227 }, { 255, 255, 255 }
};

uint16_t unicode_by_day[7][3] = { { 0x661f, 0x671f, 0x5929 }, { 0x661f, 0x671f, 0x4e00 }, { 0x661f, 0x671f, 0x4e8c },
	                          { 0x661f, 0x671f, 0x4e09 }, { 0x661f, 0x671f, 0x56db }, { 0x661f, 0x671f, 0x4e94 },
	                          { 0x661f, 0x671f, 0x516d } };
uint16_t g_text_unicode_array[22] = { 0x0030, 0x0031, 0x0032,       0x0033,       0x0034,       0x0035, 0x0036, 0x0037,
	                              0x0038, 0x0039, 0x003a /*:*/, 0x002d /*-*/, 0x0020 /* */, 0x661f, 0x671f, 0x5929,
	                              0x4e00, 0x4e8c, 0x4e09,       0x56db,       0x4e94,       0x516d };

extern void logAllOsdHandle(OsdHandle *phd);
extern int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol);
extern int alignUVVal(char *ayuv_buf, int width, int height);
extern int RGBTrans2YUV(int *pr, int *pg, int *pb, int *py, int *pu, int *pv);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int calcNewLine(int startx, int starty, int endx, int endy, int thickness, OsdRegion *targetReg,
                       OsdLine *output);

#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

#define OSD_NUM (4)
OsdHandle *g_osd_handle;
OSD_HANDLE g_osd_chn_handle[3][OSD_NUM];
MPI_OSD_CANVAS_ATTR_S g_osd_canvas_attr[3][OSD_NUM];

pthread_t osd_tid;
int gRunflag = 0;

INT32 createOsdInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle,
                        MPI_OSD_CANVAS_ATTR_S *canvas)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("MPI_createOsdRgn() failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_getOsdCanvas(*handle, canvas);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("MPI_getOsdCanvas() failed. err: %d\n", ret);
		goto release;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("Bind OSD %d to encoder channel %d failed. err: %d\n", *handle, bind->idx.chn, ret);
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

int parseTimestampUnicode(uint16_t *unicode_txt)
{
	time_t t = { 0 };
	struct tm tm = { 0 };
	char text[64];
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

	memcpy(unicode_txt, &unicode_list[0], sizeof(unicode_list));
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

	MPI_SYS_init();

	int channel = 0;
	int ret = 0;
	MPI_CHN chn_idx = MPI_VIDEO_CHN(0, channel);
	MPI_CHN_STAT_S stat;
	stat.status = MPI_STATE_NONE;
	ret = MPI_DEV_queryChnState(chn_idx, &stat);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to find chn %d", channel);
		return -EINVAL;
	}
	if (stat.status == MPI_STATE_NONE) {
		libosd_log_err("failed, chn %d not exist", channel);
		return -EINVAL;
	}

	MPI_ECHN echn_idx;
	echn_idx = MPI_ENC_CHN(0);
	echn_idx.chn = channel;

	MPI_ENC_CHN_ATTR_S chn_attr;
	ret = MPI_ENC_getChnAttr(echn_idx, &chn_attr);
	if (ret != MPI_SUCCESS) {
		libosd_log_err("failed to get encoder channel %d rect, %d", MPI_GET_ENC_CHN(echn_idx), ret);
		return -EACCES;
	}

	int chn_width = chn_attr.res.width;
	int chn_height = chn_attr.res.height;
	libosd_log_info("get chn %d res (%d, %d)", MPI_GET_ENC_CHN(echn_idx), chn_width, chn_height);

	libosd_log_info("stop ENC %d", channel);
	if (channel == 1) {
		stopEnc(0);
	}
	stopEnc(channel);

	sleep(2);

	MPI_OSD_BIND_ATTR_S osd_bind = { { 0 } };
	osd_bind.idx = echn_idx;
	bool visible = true;
	MPI_OSD_RGN_ATTR_S osd_attr = { .show = visible,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_PALETTE_8,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };
	INT32 l30_priority = 0;

	OsdRegion region[4] = {
		{ .startX = 32, .startY = 32, .width = 480, .height = 480 },
		{ .startX = chn_width / 2 + 32, .startY = 32, .width = 480, .height = 480 },
		{ .startX = chn_width / 2 + 32, .startY = chn_height / 2 + 32, .width = 960, .height = 480 },
		{ .startX = 32, .startY = chn_height / 2 + 32, .width = 480, .height = 480 }
	};

	int add_list[4] = { 0, 1, 2, 3 };
	OSD_init();
	g_osd_handle = OSD_create(chn_width, chn_height);

	for (int i = 0; i < 4; i++) {
		ret = OSD_addOsd(g_osd_handle, add_list[i], &region[add_list[i]]);
		if (ret != 0) {
			libosd_log_err("failed to add %d, %d", i, ret);
			OSD_destroy(g_osd_handle);
			return -EINVAL;
		}
	}
	OSD_calcCanvas(g_osd_handle);
	logAllOsdHandle(g_osd_handle);

	for (int i = 0; i < 4; i++) {
		osd_attr.size.width = g_osd_handle->canvas[i].width;
		osd_attr.size.height = g_osd_handle->canvas[i].height;
		osd_attr.priority = l30_priority;
		l30_priority++;
		osd_bind.point.x = g_osd_handle->canvas[i].startX;
		osd_bind.point.y = g_osd_handle->canvas[i].startY;
		ret = createOsdInstance(&osd_attr, &osd_bind, &g_osd_chn_handle[0][i], &g_osd_canvas_attr[0][i]);
		if (ret) {
			libosd_log_err("Pm & line OSD created failure. err: %d", ret);
		}
	}

	char color_idx[3] = { 0x07, 0x0a, 0x00 };
	OSD_setPrivacyMask(g_osd_handle, 0, &color_idx[0], PALETTE_8, (char *)(g_osd_canvas_attr[0][0].canvas_addr));

	OsdText txt = {
		.size = 50, .background = BLACK, .color = { 0x07, 0x0f, 0x00 }, .outline_color = { 0x07, 0x0c, 0x00 }
	};

	snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansTC-Normal.otf");
	char text[64];
	txt.outline_width = 0;
	txt.mode = PALETTE_8;
	snprintf(&text[0], 64, "Camera 0%d", 1);
	sprintf(&txt.txt[0], "%s", &text[0]);
	int txt_width, txt_height;

	char *txt_src = OSD_createTextUTF8Src8bit(&txt, &txt_width, &txt_height);
	libosd_log_info("get src %d %d", txt_width, txt_height);
	OSD_setImageAYUVptr(g_osd_handle, 1, &txt_src[0], txt_width, txt_height, PALETTE_8,
	                    (char *)(g_osd_canvas_attr[0][1].canvas_addr));
	OSD_destroySrc(txt_src);

	txt.outline_width = 2;
	char *ayuv_list = OSD_createUnicodeFontList8bit(&txt, &g_text_unicode_array[0], 22);
	/*merge a new list */
	uint16_t s[64];
	int len = 31;
	parseTimestampUnicode(&s[0]);
	int total_width = 0;
	int total_height = 0;
	OSD_getUnicodeSizetoGenerate(&s[0], len, (char *)ayuv_list, &total_width, &total_height);
	libosd_log_info("get total (%d, %d)", total_width, total_height);
	char ayuv_buf[total_width * total_height];

	OSD_generateUnicodeFromList8bit(&s[0], 31, ayuv_list, &ayuv_buf[0], total_width, total_height);
	OSD_setImageAYUVptr(g_osd_handle, 2, &ayuv_buf[0], total_width, total_height, PALETTE_8,
	                    (char *)(g_osd_canvas_attr[0][2].canvas_addr));


	OsdLine newLIne;
	int startx = region[3].startX;
	int starty = region[3].startY;
	int endx = startx + 200;
	int endy = starty + 100;
	int thickness = 5;
	ret = calcNewLine(startx, starty, endx, endy, thickness, &region[3], &newLIne);
	if (ret != 0) {
		libosd_log_err("failed to add new line, %d", ret);
		return -EINVAL;
	}
	char color[4] = { 0xff, 0x0e, 0x00, 0x00 };
	memcpy(&newLIne.color[0], &color[0], sizeof(color));
	newLIne.mode = PALETTE_8;
	libosd_log_info("new line (%d, %d) (%d, %d) %d, mode: %d", newLIne.start.x, newLIne.start.y, newLIne.end.x,
	                newLIne.end.y, newLIne.thickness, newLIne.mode);
	OSD_setLine(g_osd_handle, 3, &newLIne, (char *)(g_osd_canvas_attr[0][3].canvas_addr));

    ret = calcNewLine(startx, starty, endx, endy + 20, thickness, &region[3], &newLIne);
	if (ret != 0) {
		libosd_log_err("failed to add new line, %d", ret);
		return -EINVAL;
	}
	libosd_log_info("new line (%d, %d) (%d, %d) %d, mode: %d", newLIne.start.x, newLIne.start.y, newLIne.end.x,
	                newLIne.end.y, newLIne.thickness, newLIne.mode);
	OSD_setLine(g_osd_handle, 3, &newLIne, (char *)(g_osd_canvas_attr[0][3].canvas_addr));

	for (int i = 0; i < OSD_NUM; i++) {
		ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][i]);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("failed to update canvas, %d", ret);
			MPI_destroyOsdRgn(g_osd_chn_handle[0][i]);
		}
	}

	if (channel == 1) {
		libosd_log_info("resume ENC0");
		resumeEnc(0);
	}
	libosd_log_info("resume ENC%d", channel);
	resumeEnc(channel);

	gRunflag = 1;
	while (gRunflag) {
		parseTimestampUnicode(&s[0]);
		OSD_getUnicodeSizetoGenerate(&s[0], len, (char *)ayuv_list, &total_width, &total_height);
		libosd_log_debug("get total (%d, %d)", total_width, total_height);
		char ayuv_buf[total_width * total_height];

		OSD_generateUnicodeFromList8bit(&s[0], 31, ayuv_list, &ayuv_buf[0], total_width, total_height);
		OSD_setImageAYUVptr(g_osd_handle, 2, &ayuv_buf[0], total_width, total_height, PALETTE_8,
		                    (char *)(g_osd_canvas_attr[0][2].canvas_addr));
		ret = MPI_updateOsdCanvas(g_osd_chn_handle[0][2]);
		if (ret != MPI_SUCCESS) {
			libosd_log_err("failed to update canvas, %d", ret);
			MPI_destroyOsdRgn(g_osd_chn_handle[0][2]);
		}
		sleep(1);
	}

	libosd_log_info("stop ENC %d", channel);
	if (channel == 1) {
		stopEnc(0);
	}
	stopEnc(channel);

	sleep(2);

	OSD_destroyUnicodeFontList((char *)ayuv_list);
	OSD_destroy(g_osd_handle);
	OSD_deinit();

	libosd_log_info("remove all OSD");
	/*remove all old osd canvas*/
	for (int i = 0; i < OSD_NUM; i++) {
		if (MPI_unbindOsdFromChn(g_osd_chn_handle[0][i], &osd_bind) != MPI_SUCCESS) {
			libosd_log_err("failed to unbind chn, %d", ret);
		}

		if (MPI_destroyOsdRgn(g_osd_chn_handle[0][i]) != MPI_SUCCESS) {
			libosd_log_err("failed to unbind chn, %d", ret);
		}
	}

	if (channel == 1) {
		libosd_log_info("resume ENC0");
		resumeEnc(0);
	}
	libosd_log_info("resume ENC %d", channel);
	resumeEnc(channel);

	MPI_SYS_exit();

	return 0;
}