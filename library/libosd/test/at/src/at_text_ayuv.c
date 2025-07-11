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

#include "SDL.h"
#include "SDL_video.h"
#include "SDL_ttf.h"

#include "libosd.h"

extern int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
extern int checkKeyingBackground(char *pfcolor, int *pr, int *pg, int *pb, int *pa);
extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int readbmpInfo(char *path, char *pfcolor, BACKGROUND_COLOR bcolor);
extern int generateTxtBmp(char *fontpath, char *pfcolor, BACKGROUND_COLOR bcolor, char *txt, int size);
uint16_t g_text_unicode_array[22] = { 0x0030, 0x0031, 0x0032,       0x0033,       0x0034,       0x0035, 0x0036, 0x0037,
	                              0x0038, 0x0039, 0x003a /*:*/, 0x002d /*-*/, 0x0020 /* */, 0x661f, 0x671f, 0x5929,
	                              0x4e00, 0x4e8c, 0x4e09,       0x56db,       0x4e94,       0x516d };

int help()
{
	printf("[usage]:\r\n");
	printf("-i bmp path\r\n");
	printf("-b generate bmp <1, 2, 3> \r\n");
	printf("\t 1,2 for fcolor = BLACK,WHITE\r\n");
	printf("-h help()\r\n");
	return 0;
}

int main(int argc, char **argv)
{
	libosd_log_debug("start libosd");
	int c = 0;
	char color[3] = { 0x00, 0x00, 0x00 };
	while ((c = getopt(argc, argv, "hi:b:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'i':
			readbmpInfo(argv[optind - 1], &color[0], TRANSPARENT);
			break;
		case 'b':
			libosd_log_debug("generate text to BMP");
			if (atoi(argv[optind - 1]) == 1) {
				libosd_log_debug("fcolor BLACK");
				color[0] = 0x00;
				color[1] = 0x00;
				color[2] = 0x00;
			} else {
				libosd_log_debug("fcolor WHITE");
				color[0] = 0xff;
				color[1] = 0xff;
				color[2] = 0xff;
			}

			OsdText txt = { .size = 30,
				        .background = WHITE,
				        .color = { 0x00, 0x00, 0x00 },
				        .outline_color = { 0x00, 0x10, 0xff } };
			int w, h;

			txt.outline_width = 2;
			snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansTC-Normal.otf");
			char text[64];

			snprintf(&text[0], 64, "Here-is-Camera-0%d", 1);
			sprintf(&txt.txt[0], "%s", &text[0]);

			char *ayuv_src = OSD_createTextUTF8Src(&txt, &w, &h);
			if (ayuv_src == NULL) {
				libosd_log_err("failed to create utf8 src ptr");
			}

			libosd_log_info("image w: %d h: %d", w, h);
			saveAYUV("save-at.ayuv", w, h, ayuv_src, w * h * 2);
			OSD_destroySrc(ayuv_src);

			txt.outline_width = 1;
			memset(&txt.unicode_txt[0], 0x00, sizeof(txt.unicode_txt));
			memcpy(&txt.unicode_txt[0], &g_text_unicode_array[0], sizeof(g_text_unicode_array));

			char *ayuv_unicode_src = OSD_createTextUnicodeSrc(&txt, &w, &h);

			libosd_log_info("unicode image w: %d h: %d", w, h);
			saveAYUV("save-at-unicode.ayuv", w, h, ayuv_unicode_src, w * h * 2);
			OSD_destroySrc(ayuv_unicode_src);

			break;
		default:
			help();
			exit(1);
		}
	}

	return 0;
}

#define _GNU_SOURCE

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif