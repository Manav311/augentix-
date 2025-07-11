#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <time.h>
#include <json.h>

#include "mpi_index.h"
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_osd.h"
#include "mpi_enc.h"
#include "mpi_dev.h"
#include "mpi_index.h"

#include "libosd.h"


extern void logAllOsdHandle(OsdHandle *phd);
extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);

#define MAX_CANVAS_NUM (4)
OsdHandle *g_osd_handle[3];
OSD_HANDLE g_osd_chn_handle[3][MAX_CANVAS_NUM];
MPI_OSD_CANVAS_ATTR_S g_osd_canvas_attr[3][MAX_CANVAS_NUM];

int gRunflag = 0;

static int parseUnicode(json_object *src_obj, OsdText *txt)
{
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;

	json_object_object_get_ex(src_obj, "color_mode", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot open txt\n");
		goto end;
	}

	if (strcmp("AYUV_3544", json_object_get_string(tmp1_obj)) == 0) {
		txt->mode = AYUV_3544;
	} else if (strcmp("PALETTE_8", json_object_get_string(tmp1_obj)) == 0) {
		txt->mode = PALETTE_8;
	}

	json_object_object_get_ex(src_obj, "ttf_path", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot open ttf path\n");
		goto end;
	}

	sprintf(&txt->ttf_path[0], "%s", json_object_get_string(tmp1_obj));

	json_object_object_get_ex(src_obj, "forecol", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot find forecol\n");
		goto end;
	}
	for (int i = 0; i < 3; i++) {
		tmp2_obj = json_object_array_get_idx(tmp1_obj, i);
		if (!tmp2_obj) {
			libosd_log_err("Cannot open forecol\n");
			goto end;
		}
		txt->color[i] = json_object_get_int(tmp2_obj);
	}

	json_object_object_get_ex(src_obj, "outcol", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot find outcol\n");
		goto end;
	}
	for (int i = 0; i < 3; i++) {
		tmp2_obj = json_object_array_get_idx(tmp1_obj, i);
		if (!tmp2_obj) {
			libosd_log_err("Cannot open outline_color\n");
			goto end;
		}
		txt->outline_color[i] = json_object_get_int(tmp2_obj);
	}

	json_object_object_get_ex(src_obj, "backcol", &tmp1_obj);
	if (strcmp("WHITE", json_object_get_string(tmp1_obj)) == 0) {
		txt->background = WHITE;
	} else if (strcmp("BLACK", json_object_get_string(tmp1_obj)) == 0) {
		txt->background = BLACK;
	} else if (strcmp("TRANSPARENT", json_object_get_string(tmp1_obj)) == 0) {
		txt->background = TRANSPARENT;
	}

	json_object_object_get_ex(src_obj, "size", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot open size\n");
		goto end;
	}
	txt->size = json_object_get_int(tmp1_obj);

	json_object_object_get_ex(src_obj, "outline_width", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot open outline_width\n");
		goto end;
	}
	txt->outline_width = json_object_get_int(tmp1_obj);

	/*input any text*/
	json_object_object_get_ex(src_obj, "txt", &tmp1_obj);
	if (!tmp1_obj) {
		libosd_log_err("Cannot find outcol\n");
		goto end;
	}

	sprintf((char *)&txt->unicode_txt[0], "%s", json_object_get_string(tmp1_obj));

end:
	return -1;
}

int parseJsonText(char *file_name, OsdText *txt)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		libosd_log_err("Cannot open %s\n", file_name);
		goto end;
	}

	json_object_object_get_ex(test_obj, "input_text", &tmp_obj);
	if (!tmp_obj) {
		libosd_log_err("Cannot open %s\n", file_name);
		goto end;
	}

	parseUnicode(tmp_obj, txt);

	json_object_put(test_obj);

end:
	return -1;
}

void help()
{
	printf("usage:\n-c <json path>\n-h help()\n");
}

int main(int argc, char **argv)
{
	OsdText txt_info;

	int c = 0;
	char config_path[128];

	while ((c = getopt(argc, argv, "hc:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'c':
			libosd_log_info("config : %s", argv[optind - 1]);
			snprintf(&config_path[0], 128, "%s", argv[optind - 1]);
			break;
		}
	}

	parseJsonText(&config_path[0], &txt_info);

	OSD_init();

	OsdHandle *osd_handle;
	osd_handle = OSD_create(600, 600);

	OsdRegion region = { .startX = 16, .startY = 16, .width = 320, .height = 240 };

	if (OSD_addOsd(osd_handle, 0, &region) != 0) {
		OSD_destroy(osd_handle);
	}

	//int width = 0;
	//int height = 0;
	//char *src = NULL;

	OSD_destroy(osd_handle);
	OSD_deinit();
}
