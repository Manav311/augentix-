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
#include "log_define.h"

extern int setBackgroundColor(char *pfcolor, BACKGROUND_COLOR bcolor, SDL_Color *pbackcol);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
extern int checkKeyingBackground(char *pfcolor, int *pr, int *pg, int *pb, int *pa);
extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int readbmpInfo(char *path, char *pfcolor, BACKGROUND_COLOR bcolor);
extern int generateTxtBmp(char *fontpath, char *pfcolor, BACKGROUND_COLOR bcolor, char *txt, int size);

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
	osddemo_log_debug("start libosd");
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
			osddemo_log_debug("generate text to BMP");
			if (atoi(argv[optind - 1]) == 1) {
				osddemo_log_debug("fcolor BLACK");
				color[0] = 0x00;
				color[1] = 0x00;
				color[2] = 0x00;
			} else {
				osddemo_log_debug("fcolor WHITE");
				color[0] = 0xff;
				color[1] = 0xff;
				color[2] = 0xff;
			}

			generateTxtBmp("/system/mpp/font/SourceHanSans-Regular.ttc", &color[0], TRANSPARENT, "hello",
			               48);
			break;
		default:
			help();
			exit(1);
		}
	}

	return 0;
}