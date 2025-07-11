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

#define _GNU_SOURCE

extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int normalizeAYUV3544(int *pa, int *py, int *pu, int *pv);
#define MERGE35(a, b) (a << 5) | (b & 0x1f)
#define MERGE44(a, b) (a << 4) | (b & 0x0f)

void help()
{
	printf("usage:\r\n");
	printf("-h help()\r\n");
	printf("-p bmp path\r\n");
	return;
}

int main(int argc, char **argv)
{
	int c = 0;
	char bmpPath[128];
	while ((c = getopt(argc, argv, "hp:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'p':
			libosd_log_info("transfer bmp : %s", argv[optind - 1]);
			snprintf(&bmpPath[0], 128, "%s", argv[optind - 1]);
			break;
		}
	}

	OSD_init();

	OsdHandle *osd_handle;
	osd_handle = OSD_create(600, 600);

	OsdRegion region = { .startX = 16, .startY = 16, .width = 503, .height = 118 };

	if (OSD_addOsd(osd_handle, 0, &region) != 0) {
		OSD_destroy(osd_handle);
	}

	OSD_setImageBmp(osd_handle, 0, &bmpPath[0], NULL);

	OSD_destroy(osd_handle);

	OSD_deinit();
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif