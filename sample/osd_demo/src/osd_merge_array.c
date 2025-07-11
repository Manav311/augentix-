#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>

#include "SDL.h"
#include "SDL_video.h"
#include "SDL_ttf.h"

#include "libosd.h"

extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
extern int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol);
extern int alignUVVal(char *ayuv_buf, int width, int height);
#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

uint16_t unicode_by_day[7][3] = { { 0x661f, 0x671f, 0x5929 }, { 0x661f, 0x671f, 0x4e00 }, { 0x661f, 0x671f, 0x4e8c },
	                          { 0x661f, 0x671f, 0x4e09 }, { 0x661f, 0x671f, 0x56db }, { 0x661f, 0x671f, 0x4e94 },
	                          { 0x661f, 0x671f, 0x516d } };

uint16_t g_text_unicode_array[22] = { 0x0030, 0x0031, 0x0032,       0x0033,       0x0034,       0x0035, 0x0036, 0x0037,
	                              0x0038, 0x0039, 0x003a /*:*/, 0x002d /*-*/, 0x0020 /* */, 0x661f, 0x671f, 0x5929,
	                              0x4e00, 0x4e8c, 0x4e09,       0x56db,       0x4e94,       0x516d };

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

#if 0
#define GETUPPER4BIT(x) (x >> 4)
#define GETLOWER4BIT(x) (x & 0x0f)

int alignUVVal(char *ayuv_buf, int width, int height)
{
	int u;
	int v;

	/*4 * 4 normal case*/
	for (int i = 0; i < height; i += 2) {
		if (i > height - 1) {
			osddemo_log_err("end at : %d\r", i);
			break;
		}
		for (int j = 0; j < width; j += 2) {
			if (j > width - 1) {
				osddemo_log_err("end at : %d", j);
				break;
			}
			u = 0;
			u += GETUPPER4BIT(ayuv_buf[(i * height + j) * 2 + 1]);
			u += GETUPPER4BIT(ayuv_buf[(i * height + j) * 2 + 1 + 2]);
			u += GETUPPER4BIT(ayuv_buf[(i * height + j) * 2 + 1 + height]);
			u += GETUPPER4BIT(ayuv_buf[(i * height + j) * 2 + 1 + height + 2]);

			u = u >> 2;

			v = 0;
			v += GETLOWER4BIT(ayuv_buf[(i * height + j) * 2 + 1]);
			v += GETLOWER4BIT(ayuv_buf[(i * height + j) * 2 + 1 + 2]);
			v += GETLOWER4BIT(ayuv_buf[(i * height + j) * 2 + 1 + height]);
			v += GETLOWER4BIT(ayuv_buf[(i * height + j) * 2 + 1 + height + 2]);

			v = v >> 2;

			ayuv_buf[(i * height + j) * 2 + 1] = MERGE44(u, v);
			ayuv_buf[(i * height + j) * 2 + 1 + 2] = MERGE44(u, v);
			ayuv_buf[(i * height + j) * 2 + 1 + height] = MERGE44(u, v);
			ayuv_buf[(i * height + j) * 2 + 1 + height + 2] = MERGE44(u, v);
		}
	}

	if ((width % 2) == 1) {
		for (int i = 0; i < height; i += 2) {
			if (i > height) {
				osddemo_log_err("end at : %d", i);
				break;
			}
			u = GETUPPER4BIT(ayuv_buf[(i * width) * 2 + 1]);
			u += GETUPPER4BIT(ayuv_buf[((i + 1) * width) * 2 + 1]);
			u = u >> 1;
			v = GETLOWER4BIT(ayuv_buf[(i * width) * 2 + 1]);
			v += GETLOWER4BIT(ayuv_buf[((i + 1) * width) * 2 + 1]);
			v = v >> 1;
			ayuv_buf[(i * width) * 2 + 1] = MERGE44(u, v);
			ayuv_buf[((i + 1) * width) * 2 + 1] = MERGE44(u, v);
		}
	}

	if ((height % 2) == 1) {
		for (int j = 0; j < width; j += 2) {
			if (j > width - 1) {
				osddemo_log_err("end at : %d", j);
				break;
			}
			u = GETUPPER4BIT(ayuv_buf[(height * (width - 1)) * 2 + j]);
			u += GETUPPER4BIT(ayuv_buf[(height * (width - 1)) * 2 + j + 1]);
			u = u >> 1;

			v = GETLOWER4BIT(ayuv_buf[(height * (width - 1)) * 2 + j]);
			v += GETLOWER4BIT(ayuv_buf[(height * (width - 1)) * 2 + j + 1]);
			v = v >> 1;
			ayuv_buf[(height * (width - 1)) * 2 + j] = MERGE44(u, v);
			ayuv_buf[(height * (width - 1)) * 2 + j + 1] = MERGE44(u, v);
		}
	}

	return 0;
}
#endif

int main(void)
{
#if 0
	OsdText txt = { .size = 45,
		        .background = WHITE,
		        .color = { 0xff, 0xff, 0xff },
		        .outline_width = 2,
		        .outline_color = { 0x00, 0x00, 0x00 } };
	snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansTC-Normal.otf");

	char *ayuv_list = OSD_createUnicodeFontList(&txt, &g_text_unicode_array[0], 22);

	/*merge a new list */
	uint16_t s[64];
	int len = 31;
	parseTimestampUnicode(&s[0]);

	int total_width = 0;
	int total_height = 0;
	OSD_getUnicodeSizetoGenerate(&s[0], len, (char *)ayuv_list, &total_width, &total_height);

	osddemo_log_err("get total (%d, %d)", total_width, total_height);
	char ayuv_buf[total_width * total_height * 2];

	OSD_generateUnicodeFromList(&s[0], 31, ayuv_list, &ayuv_buf[0], total_width, total_height);

	saveAYUV("/mnt/nfs/ethnfs/merge-out.ayuv", total_width, total_height, &ayuv_buf[0],
	         total_width * total_height * 2);

	saveAYUV("/mnt/nfs/ethnfs/merge-out-2.ayuv", total_width, total_height, &ayuv_buf[0],
	         total_width * total_height * 2);

	OSD_destroyUnicodeFontList((char *)ayuv_list);
#else
	OsdHandle *g_osd_handle[3];
	OSD_init();
	int chn_width = 1920;
	int chn_height = 1080;
	g_osd_handle[0] = OSD_create(chn_width, chn_height);

	OsdRegion region[3] = { { .startX = 16, .startY = 16, .width = 503, .height = 118 },
		                { .startX = chn_width - 160, .startY = 16, .width = 160, .height = 48 },
		                { .startX = chn_width - 640, .startY = chn_height - 80, .width = 384, .height = 48 } };

	for (int j = 0; j < 3; j++) {
		if (OSD_addOsd(g_osd_handle[0], j, &region[j]) != 0) {
			OSD_destroy(g_osd_handle[0]);
		}
	}

	if (OSD_calcCanvas(g_osd_handle[0]) != 0) {
		OSD_destroy(g_osd_handle[0]);
	}
	OSD_setImage(g_osd_handle[0], 0, "/system/mpp/font/LOGO_Augentix_v2.imgayuv", NULL);
	OSD_destroy(g_osd_handle[0]);
	OSD_deinit();
#endif

	return 0;
}