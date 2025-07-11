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
#include <time.h>

#include "libosd.h"

#define _GNU_SOURCE

extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

#define TRUE (1)
#define FALSE (0)

uint16_t unicode_by_day[7][3] = { { 0x661f, 0x671f, 0x5929 }, { 0x661f, 0x671f, 0x4e00 }, { 0x661f, 0x671f, 0x4e8c },
	                          { 0x661f, 0x671f, 0x4e09 }, { 0x661f, 0x671f, 0x56db }, { 0x661f, 0x671f, 0x4e94 },
	                          { 0x661f, 0x671f, 0x516d } };
uint16_t g_text_unicode_array[28] = { 0x0030, 0x0031, 0x0032, 0x0033,       0x0034,       0x0035,       0x0036,
	                              0x0037, 0x0038, 0x0039, 0x003a /*:*/, 0x002f /*/*/, 0x0020 /* */, 0x661f,
	                              0x671f, 0x5929, 0x4e00, 0x4e8c,       0x4e09,       0x56db,       0x4e94,
	                              0x516d, 0x0020, 0x0079, 0x0067,       0x0071,       0x0070,       0x006a };

extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);

int parseTimestampUnicode(uint16_t *unicode_txt)
{
	time_t t = { 0 };
	struct tm tm = { 0 };
	char text[64];
	uint16_t unicode_list[31];

	t = time(NULL);
	tm = *localtime(&t);

	snprintf(&text[0], 64, "   %d/%02d/%02d  ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
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

void help()
{
	printf("usage:\r\n");
	printf("-h help()\r\n");
	printf("-p ttf ttf_path\r\n"
	       "-u unicode / -U utf8\r\n"
	       "-t timestamp (unicode only)\r\n"
	       "-c create src\r\n"
	       "-w outline width\r\n"
	       "-8 is palette8\r\n"
	       "-k kerning len\r\n"
	       "-s size\r\n");
	return;
}

int main(int argc, char **argv)
{
	int c = 0;
	char ttf_path[128];
	memset(&ttf_path[0], 0x00, 128);
	int outline_width = 0;
	int is_create_src = TRUE;
	int is_UTF8 = TRUE;
	int is_TIMESTAMP = FALSE;
	int is_palette8 = FALSE;
	int kerning_len = 3;
	int size = 60;
	while ((c = getopt(argc, argv, "hp:w:uUcs:tk:8i:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'p':
			libosd_log_info("transfer ttf : %s", argv[optind - 1]);
			snprintf(&ttf_path[0], 128, "%s", argv[optind - 1]);
			break;
		case 'w':
			outline_width = atoi(argv[optind - 1]);
			libosd_log_info("set outline width:%d", outline_width);
			break;
		case 'c':
			libosd_log_info("create src");
			is_create_src = TRUE;
			break;
		case 't':
			libosd_log_info("Use timestamp");
			is_TIMESTAMP = TRUE;
			is_UTF8 = FALSE;
			break;
		case 'u':
			libosd_log_info("use unicode");
			is_UTF8 = FALSE;
			break;
		case 'U':
			libosd_log_info("use utf8");
			is_UTF8 = TRUE;
			break;
		case '8':
			libosd_log_info("set color mode palette 8");
			is_palette8 = TRUE;
			break;
		case 'k':
			kerning_len = atoi(argv[optind - 1]);
			libosd_log_info("set kerning len:%d", kerning_len);
			break;
		case 's':
			size = atoi(argv[optind - 1]);
			libosd_log_info("set size: %d", size);
			break;
		}
	}

	OsdText txt_unicode_info = {
		.size = size,
		.color = { 0x00, 0xcc, 0xcc },
		.outline_color = { 0x00, 0x00, 0x00 },
#if 0
		                     .unicode_txt = { 0x0020, 0x0032, 0x0030, 0x0032, 0x0031, 0x002f, 0x0030, 0x0037, 0x002f,
		                                      0x0030, 0x0033, 0x0020, 0x661f, 0x671f, 0x4e94, 0x0020, 0x0031, 0x0034, 0x003a, 0x0031,
                                              0x0032, 0x003a, 0x0031, 0x0038, 0x0020, 0x0000}
#else
		.unicode_txt = { 0x4e2d, 0x6587, 0x8f49, 0x63db, 0x5de5, 0x5177, 0xff11, 0xff12, 0xff13,
		                 0xff14, 0xff1f, 0x0020, 0x0070, 0x0067, 0x0062, 0x006c, 0x006f, 0x0063,
		                 0x006b, 0x0079, 0x0061, 0x0062, 0x0071, 0x006a, 0x0031, 0x0032, 0xff04,
		                 0x5929, 0x6c23, 0x597d, 0x5929, 0x6c14, 0x597d, 0xff4b, 0xff42, 0xff59 }
#endif
	};

	snprintf(&txt_unicode_info.ttf_path[0], MAX_TXT_LEN, "./simhei.ttf");
	if (ttf_path[0] != 0) {
		snprintf(&txt_unicode_info.ttf_path[0], MAX_TXT_LEN, &ttf_path[0]);
	}
	if (is_palette8) {
		txt_unicode_info.mode = PALETTE_8;
	} else {
		txt_unicode_info.mode = AYUV_3544;
	}
	txt_unicode_info.outline_width = outline_width;
	txt_unicode_info.background = WHITE;
	txt_unicode_info.kerning_mode = MANUAL;
	txt_unicode_info.kerning = kerning_len;

	OsdText txt_info = { .size = size,
		             .color = { 0x00, 0xcc, 0xcc },
		             .outline_color = { 0xff, 0x00, 0x00 },
		             .txt = { 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
		                      0x20, 0x20, 0x20, 0x20, 0xE8, 0x81, 0x94, 0x67, 0x68, 0xE7, 0xBB, 0x9C, 0xE4,
		                      0xBA, 0xBA, 0xE5, 0xB0, 0x91, 0xE8, 0xA7, 0x81, 0xE5, 0x9D, 0x9B, 0xE5, 0x9D,
		                      0x8F, 0xE6, 0x89, 0xB0, 0xE5, 0x9D, 0x9D, 0xE8, 0xB4, 0xA1, 0xE6, 0x8A, 0xA2,
		                      0xE5, 0x9D, 0x9F, 0xE5, 0xA5, 0xBD, 0x20, 0xe9, 0xa5, 0xbd, 0x20 } };
#if 0
	snprintf(&txt_info.txt[0], MAX_TXT_LEN, "block01 HSIPC H265+BR=119K 10:34");
#else

#endif
	snprintf(&txt_info.ttf_path[0], MAX_TXT_LEN, "./simhei.ttf");
	if (ttf_path[0] != 0) {
		snprintf(&txt_info.ttf_path[0], MAX_TXT_LEN, &ttf_path[0]);
	}
	if (is_palette8) {
		txt_info.mode = PALETTE_8;
	} else {
		txt_info.mode = AYUV_3544;
	}
	txt_info.background = WHITE;
	txt_info.outline_width = outline_width;
	txt_info.kerning_mode = MANUAL;
	txt_info.kerning = kerning_len;

	OSD_init();

	OsdHandle *osd_handle;
	osd_handle = OSD_create(600, 600);

	OsdRegion region = { .startX = 16, .startY = 16, .width = 320, .height = 240 };

	if (OSD_addOsd(osd_handle, 0, &region) != 0) {
		OSD_destroy(osd_handle);
	}

	int width = 0;
	int height = 0;
	char *src = NULL;

	if (is_TIMESTAMP) {
		libosd_log_info("is timestamp");
		int len = 31;
		uint16_t s[len];
		parseTimestampUnicode(&s[0]);
#if 0
		s[26]= 0x0070;
		s[27]= 0x0071;
		s[28]= 0x0067;
		s[29]= 0x006a;
		s[30]= 0x0079;
#endif
		if (!is_palette8) {
			char *ayuv_list = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 28);

			OSD_getUnicodeSizetoGenerate(&s[0], len, ayuv_list, &width, &height);
			libosd_log_err("get total (%d, %d)", width, height);

			src = malloc(width * height * 2);

			OSD_generateUnicodeFromList(&s[0], len, ayuv_list, src, width, height);
			OSD_destroyUnicodeFontList((char *)ayuv_list);

		} else {
			char *ayuv_list =
			        OSD_createUnicodeFontList8bit(&txt_unicode_info, &g_text_unicode_array[0], 28);
			/*merge a new list */

			OSD_getUnicodeSizetoGenerate(&s[0], len, ayuv_list, &width, &height);
			libosd_log_err("get total (%d, %d)", width, height);

			src = malloc(width * height);

			OSD_generateUnicodeFromList8bit(&s[0], len, ayuv_list, src, width, height);
			OSD_destroyUnicodeFontList((char *)ayuv_list);
		}
	}

	if (is_create_src && !is_palette8 && !is_TIMESTAMP) {
		if (is_UTF8) {
			libosd_log_err("here\n");
			src = OSD_createTextUTF8Src(&txt_info, &width, &height);
		} else {
			src = OSD_createTextUnicodeSrc(&txt_unicode_info, &width, &height);
		}
	}

	if (is_create_src && is_palette8 && !is_TIMESTAMP) {
		if (is_UTF8 == TRUE) {
			src = OSD_createTextUTF8Src8bit(&txt_info, &width, &height);
		} else {
			src = OSD_createTextUnicodeSrc8bit(&txt_unicode_info, &width, &height);
		}
	}

	/*save output*/

	char tmpfile[32];

	if (!is_palette8) {
		libosd_log_err("here\n");
		if (is_UTF8) {
			snprintf(&tmpfile[0], 32, "./%s.ayuv", "utf8");
		} else {
			snprintf(&tmpfile[0], 32, "./%s.ayuv", "unicode");
		}
		saveAYUV(&tmpfile[0], width, height, src, width * height * 2);

	} else {
		if (is_UTF8) {
			snprintf(&tmpfile[0], 32, "./%s.bin", "utf8");
		} else {
			snprintf(&tmpfile[0], 32, "./%s.bin", "unicode");
		}
		FILE *fp = fopen(tmpfile, "w");
		fwrite(src, 1, width * height, fp);
		fclose(fp);
	}

	libosd_log_err("get w, h = %d, %d\n", width, height);
	libosd_log_err("save %s\n", &tmpfile[0]);
	if (src != NULL) {
		free(src);
	}

	OSD_destroy(osd_handle);

	OSD_deinit();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif