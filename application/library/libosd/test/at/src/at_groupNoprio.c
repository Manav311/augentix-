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

extern int get_max(int items[], int num);
extern int get_min(int items[], int num);
extern int get_min_idx(int items[], int num, int *idx);
extern void logAllOsdHandle(OsdHandle *phd);

int main(int argc, char **argv)
{
	OSD_init();

	OsdHandle *osd_handle;
	osd_handle = OSD_create(2596, 1944);
	int OSD_num = 5;
#if 0
	OsdRegion region[8] = { { .startX = 0, .startY = 0, .width = 160, .height = 80 },
		                { .startX = 1800, .startY = 0, .width = 160, .height = 80 },
		                { .startX = 1800, .startY = 1000, .width = 160, .height = 80 },
		                { .startX = 0, .startY = 1000, .width = 160, .height = 80 },
		                { .startX = 200, .startY = 200, .width = 160, .height = 80 },
		                { .startX = 1600, .startY = 200, .width = 160, .height = 80 },
		                { .startX = 1600, .startY = 800, .width = 160, .height = 80 },
		                { .startX = 200, .startY = 800, .width = 160, .height = 80 } };
		OsdRegion region[8] = { { .startX = 0, .startY = 0, .width = 160, .height = 80 },
		                { .startX = 56, .startY = 56, .width = 160, .height = 80 },
		                { .startX = 48, .startY = 48, .width = 160, .height = 80 },
		                { .startX = 8, .startY = 8, .width = 160, .height = 80 },
		                { .startX = 16, .startY = 16, .width = 160, .height = 80 },
		                { .startX = 40, .startY = 40, .width = 160, .height = 80 },
		                { .startX = 32, .startY = 32, .width = 160, .height = 80 },
		                { .startX = 24, .startY = 24, .width = 160, .height = 80 } };
#else

	int block_width = 2592 / 4 - 16;
	int block_height = 1944 / 4 - 16;
	OsdRegion region[5] = { { .startX = 0, .startY = 0, .width = block_width, .height = block_height },
		                { .startX = block_width - 80,
		                  .startY = block_height - 80,
		                  .width = block_width,
		                  .height = block_height },
		                { .startX = block_width * 2, .startY = 0, .width = block_width, .height = block_height },
		                { .startX = block_width * 2 - 80,
		                  .startY = block_height * 2,
		                  .width = block_width,
		                  .height = block_height },
		                { .startX = block_width * 3,
		                  .startY = block_height * 3 - 80,
		                  .width = block_width,
		                  .height = block_height } };

#endif

	for (int i = 0; i < OSD_num; i++) {
		if (OSD_addOsd(osd_handle, i, &region[i]) != 0) {
			OSD_destroy(osd_handle);
			return -1;
		}
	}

	OSD_calcCanvasbygroup(osd_handle);
	logAllOsdHandle(osd_handle);

	OSD_destroy(osd_handle);
	OSD_deinit();

	return 0;
}

#define _GNU_SOURCE

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif