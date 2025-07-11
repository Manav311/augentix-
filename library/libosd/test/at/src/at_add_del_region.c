#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <pthread.h>

#include "libosd.h"

extern void logAllOsdHandle(OsdHandle *phd);
OsdHandle *g_osd_handle;

void help()
{
}

int main(int argc, char **argv)
{
	int c = 0;
	int channel = 0;
	int ret = 0;
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

	OSD_init();
	int chn_width = 1920;
	int chn_height = 1080;

	g_osd_handle = OSD_create(chn_width, chn_height);

#if 1
	OsdRegion region[8] = {
		{ .startX = 32, .startY = 32, .width = 400, .height = 400 },
		{ .startX = chn_width / 2 + 32, .startY = 32, .width = 400, .height = 400 },
		{ .startX = chn_width / 2 + 32, .startY = chn_height / 2 + 32, .width = 400, .height = 400 },
		{ .startX = 32, .startY = chn_height / 2 + 32, .width = 400, .height = 400 },
		{ .startX = 64, .startY = 64, .width = 208, .height = 80 },
		{ .startX = chn_width / 2 + 64, .startY = 64, .width = 208, .height = 80 },
		{ .startX = chn_width / 2 - 64, .startY = chn_height / 2 + 64, .width = 208, .height = 80 },
		{ .startX = 64, .startY = chn_height / 2 + 64, .width = 208, .height = 80 }
	};
#else
	OsdRegion region[8] = {
		{ .startX = 0, .startY = 0, .width = 120, .height = 80 },
		{ .startX = chn_width * 3 / 4, .startY = 0, .width = 120, .height = 80 },
		{ .startX = chn_width * 3 / 4, .startY = chn_height * 3 / 4, .width = 120, .height = 80 },
		{ .startX = 0, .startY = chn_height * 3 / 4, .width = 120, .height = 80 },
		{ .startX = chn_width / 4, .startY = chn_height / 4, .width = 120, .height = 80 },
		{ .startX = chn_width / 2, .startY = chn_height / 4, .width = 120, .height = 80 },
		{ .startX = chn_width / 2, .startY = chn_height * 3 / 4, .width = 120, .height = 80 },
		{ .startX = chn_width / 4, .startY = chn_height * 3 / 4, .width = 120, .height = 80 }
	};
#endif

	OsdRegion change_region = { .startX = chn_width / 4, .startY = chn_height * 3 / 4, .width = 120, .height = 80 };

	int add_list[2] = { 0, 6 };
	for (int i = 0; i < 2; i++) {
		ret = OSD_addOsd(g_osd_handle, add_list[i], &region[add_list[i]]);
		if (ret != 0) {
			libosd_log_err("failed to add %d, %d", i, ret);
			OSD_destroy(g_osd_handle);
			return -EINVAL;
		}
	}
	OSD_calcCanvas(g_osd_handle);
	logAllOsdHandle(g_osd_handle);

	int add_list2[2] = { 1, 4 };
	for (int i = 0; i < 2; i++) {
		ret = OSD_addOsd(g_osd_handle, add_list2[i], &region[add_list2[i]]);
		if (ret != 0) {
			libosd_log_err("failed to add %d, ret:%d", i, ret);
			OSD_destroy(g_osd_handle);
			return -EINVAL;
		}
	}
	OSD_calcCanvas(g_osd_handle);
	logAllOsdHandle(g_osd_handle);

	ret = OSD_addOsd(g_osd_handle, 2, &change_region);
	if (ret != 0) {
		libosd_log_err("failed to add %d, %d", 2, ret);
		OSD_destroy(g_osd_handle);
		return -EINVAL;
	}

	OSD_calcCanvasbygroup(g_osd_handle);
	logAllOsdHandle(g_osd_handle);
#if 1
	OSD_delOsd(g_osd_handle, 6);
	OSD_calcCanvas(g_osd_handle);
	logAllOsdHandle(g_osd_handle);

	int add_list3[3] = { 1, 3, 6 };
	for (int i = 0; i < 3; i++) {
		ret = OSD_addOsd(g_osd_handle, add_list3[i], &region[add_list3[i]]);
		if (ret != 0) {
			libosd_log_err("failed to add %d, %d", i, ret);
			OSD_destroy(g_osd_handle);
			return -EINVAL;
		}
	}
	OSD_calcCanvasbygroup(g_osd_handle);
	logAllOsdHandle(g_osd_handle);
#endif
	OSD_destroy(g_osd_handle);

	return 0;
}