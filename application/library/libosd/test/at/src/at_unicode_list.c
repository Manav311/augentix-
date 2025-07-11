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
#include <signal.h>

#include "libosd.h"
int gRunflag = 0;

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
uint16_t g_text_unicode_array[31] = { 0x0030, 0x0031, 0x0032,       0x0033,       0x0034,       0x0035, 0x0036, 0x0037,
	                              0x0038, 0x0039, 0x003a /*:*/, 0x002f /*/*/, 0x0020 /* */, 0x661f, 0x671f, 0x5929,
	                              0x4e00, 0x4e8c, 0x4e09,       0x56db,       0x4e94,       0x516d, 0x0020, 0x0079,
	                              0x0067, 0x0071, 0x0070,       0x006a,       0x005f,       0x002d, 0x3000 };

void help()
{
	printf("usage:\r\n");
	printf("-h help()\r\n");
	printf("-p ttf ttf_path\r\n"
	       "-u unicode / -U utf8\r\n"
	       "-t timestamp (unicode only)\r\n"
	       "-w outline width\r\n"
	       "-k kerning len\r\n"
	       "-s size\r\n");
	return;
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

	int c = 0;
	char ttf_path[128];
	memset(&ttf_path[0], 0x00, 128);
	int outline_width = 0;
	int kerning_len = 1;
	int size = 60;

	while ((c = getopt(argc, argv, "hp:w:s:k:8i:")) != -1) {
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

	gRunflag = 1;

	OsdText txt_unicode_info = { .size = size,
		                     .color = { 0x00, 0xcc, 0xcc },
		                     .outline_color = { 0x00, 0x00, 0x00 },
		                     .unicode_txt = { 0x4e2d, 0x6587, 0x8f49, 0x63db, 0x5de5, 0x5177, 0xff11, 0xff12,
		                                      0xff13, 0xff14, 0xff1f, 0x0020, 0x0070, 0x0067, 0x0062, 0x006c,
		                                      0x006f, 0x0063, 0x006b, 0x0079, 0x0061, 0x0062, 0x0071, 0x006a,
		                                      0x0031, 0x0032, 0xff04, 0x5929, 0x6c23, 0x597d, 0x5929, 0x6c14,
		                                      0x597d, 0xff4b, 0xff42, 0xff59 } };
	txt_unicode_info.mode = AYUV_3544;
	txt_unicode_info.outline_width = outline_width;
	txt_unicode_info.background = WHITE;
	txt_unicode_info.kerning_mode = MANUAL;
	txt_unicode_info.kerning = kerning_len;
	snprintf(&txt_unicode_info.ttf_path[0], MAX_TXT_LEN, "./simhei.ttf");
	if (ttf_path[0] != 0) {
		snprintf(&txt_unicode_info.ttf_path[0], MAX_TXT_LEN, &ttf_path[0]);
	}

	OSD_init();

	OsdHandle *osd_handle;
	osd_handle = OSD_create(600, 600);

	OsdRegion region = { .startX = 16, .startY = 16, .width = 320, .height = 240 };

	if (OSD_addOsd(osd_handle, 0, &region) != 0) {
		OSD_destroy(osd_handle);
	}

	int cnt = 0;
#if 1
	char *ayuv_list[16];
	while (gRunflag) {
		ayuv_list[0] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[1] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[2] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[3] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[4] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[5] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[6] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[7] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[8] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[9] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[10] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[11] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[12] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[13] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[14] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);
		ayuv_list[15] = OSD_createUnicodeFontList(&txt_unicode_info, &g_text_unicode_array[0], 31);

		OSD_destroyUnicodeFontList((char *)ayuv_list[0]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[1]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[2]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[3]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[4]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[5]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[6]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[7]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[8]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[9]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[10]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[11]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[12]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[13]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[14]);
		OSD_destroyUnicodeFontList((char *)ayuv_list[15]);

		//libosd_log_info("time [%d]\n", cnt);
		system("cat /proc/meminfo | grep MemAvailable");
		sleep(1);
		cnt++;
	}
#else
	char *buf = NULL;
	while (gRunflag) {
		buf = malloc((sizeof(AyuvSrcList) + 31 * sizeof(AyuvSrc) + 620) * 100);
		free(buf);
		system("cat /proc/meminfo | grep MemAvailable");
		sleep(1);
		cnt++;
	}
#endif
	libosd_log_info("leave\n");

	OSD_destroy(osd_handle);
	OSD_deinit();
	return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
