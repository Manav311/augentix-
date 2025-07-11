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
#include "mpi_limits.h"
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_sys.h"
#include "mpi_osd.h"
#include "mpi_enc.h"
#include "mpi_dev.h"
#include "mpi_index.h"

#include "libosd.h"
#include "log_define.h"

extern void logAllOsdHandle(OsdHandle *phd);
extern int calcNewLine(int startx, int starty, int endx, int endy, int thickness, OsdRegion *targetReg,
                       OsdLine *output);
extern int saveAYUV(char *filePath, uint32_t w, uint32_t h, char *payuvBuf, int bufSize);
extern int alignUVVal(char *ayuv_buf, int width, int height);

#define MAX_CANVAS_NUM (4)
OsdHandle *g_osd_handle[MPI_MAX_ENC_CHN_NUM];
OSD_HANDLE g_osd_chn_handle[MPI_MAX_ENC_CHN_NUM][MAX_CANVAS_NUM];
MPI_OSD_CANVAS_ATTR_S g_osd_canvas_attr[MPI_MAX_ENC_CHN_NUM][MAX_CANVAS_NUM];

int gRunflag = 0;

uint16_t unicode_by_day[7][3] = { { 0x661f, 0x671f, 0x5929 }, { 0x661f, 0x671f, 0x4e00 }, { 0x661f, 0x671f, 0x4e8c },
	                          { 0x661f, 0x671f, 0x4e09 }, { 0x661f, 0x671f, 0x56db }, { 0x661f, 0x671f, 0x4e94 },
	                          { 0x661f, 0x671f, 0x516d } };

uint16_t g_text_unicode_array[22] = { 0x0030, 0x0031, 0x0032,       0x0033,       0x0034,       0x0035, 0x0036, 0x0037,
	                              0x0038, 0x0039, 0x003a /*:*/, 0x002d /*-*/, 0x0020 /* */, 0x661f, 0x671f, 0x5929,
	                              0x4e00, 0x4e8c, 0x4e09,       0x56db,       0x4e94,       0x516d };

typedef enum { NONE, UTF8_TS, UNICODE_TS } TIMESTAMP_TYPE;
typedef struct {
	int region_num[MPI_MAX_ENC_CHN_NUM];
	int region_list[MPI_MAX_ENC_CHN_NUM][MAX_CANVAS_NUM];
	TIMESTAMP_TYPE region_type[MPI_MAX_ENC_CHN_NUM][MAX_CANVAS_NUM];
	OsdText region_txt[MPI_MAX_ENC_CHN_NUM][MAX_CANVAS_NUM];
	char *ayuv_src_list[MPI_MAX_ENC_CHN_NUM][MAX_CANVAS_NUM];
} DynamicTimestampRegions;


DynamicTimestampRegions g_timestamp_regions;

INT32 createOsdInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle,
                        MPI_OSD_CANVAS_ATTR_S *canvas)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		osddemo_log_err("MPI_createOsdRgn() failed. err: %d", ret);
		return ret;
	}

	ret = MPI_getOsdCanvas(*handle, canvas);
	if (ret != MPI_SUCCESS) {
		osddemo_log_err("MPI_getOsdCanvas() failed. err: %d", ret);
		goto release;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		osddemo_log_err("Bind OSD %d to encoder channel %d failed. err: %d", *handle, bind->idx.chn, ret);
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
		osddemo_log_err("Stop encoder channel %d failed.", MPI_GET_ENC_CHN(echn_idx));
		return -EACCES;
	}

	return 0;
}

int checkChnExist(int channel, MPI_ENC_CHN_ATTR_S *chn_attr)
{
	int ret = 0;
	MPI_CHN_STAT_S stat;
	MPI_CHN chn_idx = MPI_VIDEO_CHN(0, channel);
	stat.status = MPI_STATE_NONE;
	ret = MPI_DEV_queryChnState(chn_idx, &stat);
	if (ret != MPI_SUCCESS) {
		osddemo_log_err("failed to find chn %d", channel);
		return -EINVAL;
	}
	if (stat.status == MPI_STATE_NONE) {
		osddemo_log_err("failed, chn %d not exist", channel);
		return -EACCES;
	}

	if (stat.status == MPI_STATE_STOP) {
		osddemo_log_err("failed, chn %d stop", channel);
		return -EACCES;
	}

	if (stat.status == MPI_STATE_SUSPEND) {
		osddemo_log_err("failed, chn %d suspend", channel);
		return -EACCES;
	}

	MPI_ECHN echn_idx;

	echn_idx = MPI_ENC_CHN(0);
	echn_idx.chn = channel;

	ret = MPI_ENC_getChnAttr(echn_idx, chn_attr);
	if (ret != MPI_SUCCESS) {
		osddemo_log_debug("failed to get encoder channel %d  rect", MPI_GET_ENC_CHN(echn_idx));
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
		osddemo_log_err("Bind encoder channel %d failed.", MPI_GET_ENC_CHN(echn_idx));
		return MPI_FAILURE;
	}

	ret = MPI_ENC_startChn(echn_idx);
	if (ret != MPI_SUCCESS) {
		osddemo_log_err("Start encoder channel %d failed.", MPI_GET_ENC_CHN(echn_idx));
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

int parseUTF8(json_object *src_obj, OsdText *txt, int chn_idx __attribute__((unused)),
              int reg_idx __attribute__((unused)))
{
	json_object *tmp4_obj = NULL;
	json_object *tmp5_obj = NULL;

	json_object_object_get_ex(src_obj, "txt", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open txt");
		goto end;
	}
	if (strcmp("timestamp", json_object_get_string(tmp4_obj)) == 0) {
		osddemo_log_err("utf8 has no timestamp");
	} else {
		sprintf(&txt->txt[0], "%s", json_object_get_string(tmp4_obj));
	}

	json_object_object_get_ex(src_obj, "ttf_path", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open ttf path");
		goto end;
	}
	sprintf(&txt->ttf_path[0], "%s", json_object_get_string(tmp4_obj));

	json_object_object_get_ex(src_obj, "forecol", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot find forecol");
		goto end;
	}
	for (int i = 0; i < 3; i++) {
		tmp5_obj = json_object_array_get_idx(tmp4_obj, i);
		if (!tmp5_obj) {
			osddemo_log_err("Cannot open forecol");
			goto end;
		}
		txt->color[i] = json_object_get_int(tmp5_obj);
	}

	json_object_object_get_ex(src_obj, "outcol", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot find outcol");
		goto end;
	}
	for (int i = 0; i < 3; i++) {
		tmp5_obj = json_object_array_get_idx(tmp4_obj, i);
		if (!tmp5_obj) {
			osddemo_log_err("Cannot open outline_color");
			goto end;
		}
		txt->outline_color[i] = json_object_get_int(tmp5_obj);
	}

	json_object_object_get_ex(src_obj, "backcol", &tmp4_obj);
	if (strcmp("WHITE", json_object_get_string(tmp4_obj)) == 0) {
		txt->background = WHITE;
	} else if (strcmp("BLACK", json_object_get_string(tmp4_obj)) == 0) {
		txt->background = BLACK;
	} else if (strcmp("TRANSPARENT", json_object_get_string(tmp4_obj)) == 0) {
		txt->background = TRANSPARENT;
	}

	json_object_object_get_ex(src_obj, "size", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open size");
		goto end;
	}
	txt->size = json_object_get_int(tmp4_obj);

	json_object_object_get_ex(src_obj, "outline_width", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open outline_width");
		goto end;
	}
	txt->outline_width = json_object_get_int(tmp4_obj);

	/*kerning mode*/
	json_object_object_get_ex(src_obj, "kerning_mode", &tmp4_obj);
	if (!tmp4_obj) {
		txt->kerning_mode = AUTO;
	} else if (strcmp("AUTO", json_object_get_string(tmp4_obj)) == 0) {
		txt->kerning_mode = AUTO;
	} else if (strcmp("MANUAL", json_object_get_string(tmp4_obj)) == 0) {
		txt->kerning_mode = MANUAL;
	}
	/*kerning*/
	json_object_object_get_ex(src_obj, "kerning", &tmp4_obj);
	if (!tmp4_obj) {
		txt->kerning = AUTO_KERNING_RATE;
	} else {
		txt->kerning = json_object_get_int(tmp4_obj);
	}

	return 0;
end:
	return -1;
}
#define TIMESTAMP (2)
int parseUnicode(json_object *src_obj, OsdText *txt, int chn_idx, int reg_idx, COLOR_MODE mode)
{
	json_object *tmp4_obj = NULL;
	json_object *tmp5_obj = NULL;

	char is_timestamp = 0;
	uint16_t timestamp_list[31];

	txt->mode = mode;

	json_object_object_get_ex(src_obj, "txt", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open txt");
		goto end;
	}
	if (strcmp("timestamp", json_object_get_string(tmp4_obj)) == 0) {
		osddemo_log_err("unicode timestamp");
		parseTimestampUnicode(&timestamp_list[0]);
		g_timestamp_regions.region_list[chn_idx][g_timestamp_regions.region_num[chn_idx]] = reg_idx;
		g_timestamp_regions.region_type[chn_idx][g_timestamp_regions.region_num[chn_idx]] = UNICODE_TS;

		is_timestamp = 1;
	} else {
	}
	json_object_object_get_ex(src_obj, "ttf_path", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open ttf path");
		goto end;
	}
	sprintf(&txt->ttf_path[0], "%s", json_object_get_string(tmp4_obj));

	json_object_object_get_ex(src_obj, "forecol", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot find forecol");
		goto end;
	}
	for (int i = 0; i < 3; i++) {
		tmp5_obj = json_object_array_get_idx(tmp4_obj, i);
		if (!tmp5_obj) {
			osddemo_log_err("Cannot open forecol");
			goto end;
		}
		txt->color[i] = json_object_get_int(tmp5_obj);
	}

	json_object_object_get_ex(src_obj, "outcol", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open outcol");
		goto end;
	}
	for (int i = 0; i < 3; i++) {
		tmp5_obj = json_object_array_get_idx(tmp4_obj, i);
		if (!tmp5_obj) {
			osddemo_log_err("Cannot open outline_color");
			goto end;
		}
		txt->outline_color[i] = json_object_get_int(tmp5_obj);
	}

	json_object_object_get_ex(src_obj, "backcol", &tmp4_obj);
	if (strcmp("WHITE", json_object_get_string(tmp4_obj)) == 0) {
		txt->background = WHITE;
	} else if (strcmp("BLACK", json_object_get_string(tmp4_obj)) == 0) {
		txt->background = BLACK;
	} else if (strcmp("TRANSPARENT", json_object_get_string(tmp4_obj)) == 0) {
		txt->background = TRANSPARENT;
	}

	json_object_object_get_ex(src_obj, "size", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open size");
		goto end;
	}
	txt->size = json_object_get_int(tmp4_obj);

	json_object_object_get_ex(src_obj, "outline_width", &tmp4_obj);
	if (!tmp4_obj) {
		osddemo_log_err("Cannot open outline_width");
		goto end;
	}
	txt->outline_width = json_object_get_int(tmp4_obj);

	/*kerning mode*/
	json_object_object_get_ex(src_obj, "kerning_mode", &tmp4_obj);
	if (!tmp4_obj) {
		txt->kerning_mode = AUTO;
	} else if (strcmp("AUTO", json_object_get_string(tmp4_obj)) == 0) {
		txt->kerning_mode = AUTO;
	} else if (strcmp("MANUAL", json_object_get_string(tmp4_obj)) == 0) {
		txt->kerning_mode = MANUAL;
	}
	/*kerning*/
	json_object_object_get_ex(src_obj, "kerning", &tmp4_obj);
	if (!tmp4_obj) {
		txt->kerning = AUTO_KERNING_RATE;
	} else {
		txt->kerning = json_object_get_int(tmp4_obj);
	}

	if (is_timestamp) {
		osddemo_log_debug("save ayuv list at : %d, %d <--", chn_idx, reg_idx);
		g_timestamp_regions.region_num[chn_idx] += 1;
		if (mode == PALETTE_8) {
			g_timestamp_regions.ayuv_src_list[chn_idx][reg_idx] =
			        OSD_createUnicodeFontList8bit(txt, &g_text_unicode_array[0], 22);
		} else if (mode == AYUV_3544) {
			g_timestamp_regions.ayuv_src_list[chn_idx][reg_idx] =
			        OSD_createUnicodeFontList(txt, &g_text_unicode_array[0], 22);
		}

		int total_width = 0;
		int total_height = 0;

		OSD_getUnicodeSizetoGenerate(&timestamp_list[0], 31,
		                             g_timestamp_regions.ayuv_src_list[chn_idx][reg_idx], &total_width,
		                             &total_height);

		osddemo_log_debug("get total (%d, %d)", total_width, total_height);
#ifdef DEBUG
		if (mode == AYUV_3544) {
			char ayuv_buf[total_width * total_height * 2];
			OSD_generateUnicodeFromList(&timestamp_list[0], 31,
			                            g_timestamp_regions.ayuv_src_list[chn_idx][reg_idx], &ayuv_buf[0],
			                            total_width, total_height);
			saveAYUV("/mnt/nfs/ethnfs/merge-demo.ayuv", total_width, total_height, &ayuv_buf[0],
			         total_width * total_height * 2);
		}
#endif
		goto parse_timestamp;
	}

	return 0;
end:
	return -1;
parse_timestamp:
	return TIMESTAMP;
}

int parseLine(json_object *src_obj, OsdLine *newLine, OsdHandle *hd, int reg_idx)
{
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;

	OsdPoint p[2];
	int thickness = 0;
	json_object_object_get_ex(src_obj, "start", &tmp_obj);
	if (!tmp_obj) {
		osddemo_log_err("Failed to parse line start");
		goto end;
	}
	tmp1_obj = json_object_array_get_idx(tmp_obj, 0);
	p[0].x = json_object_get_int(tmp1_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, 1);
	p[0].y = json_object_get_int(tmp1_obj);

	json_object_object_get_ex(src_obj, "end", &tmp_obj);
	if (!tmp_obj) {
		osddemo_log_err("Failed to parse line end");
		goto end;
	}
	tmp1_obj = json_object_array_get_idx(tmp_obj, 0);
	p[1].x = json_object_get_int(tmp1_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, 1);
	p[1].y = json_object_get_int(tmp1_obj);

	json_object_object_get_ex(src_obj, "thickness", &tmp_obj);
	if (!tmp_obj) {
		osddemo_log_err("Failed to parse line thickness");
		goto end;
	}

	thickness = json_object_get_int(tmp_obj);

	if (calcNewLine(p[0].x, p[0].y, p[1].x, p[1].y, thickness, &hd->region[reg_idx], newLine) != 0) {
		osddemo_log_err("failed to add new line");
		goto end;
	}

	json_object_object_get_ex(src_obj, "color", &tmp_obj);
	if (!tmp_obj) {
		osddemo_log_err("Failed to parse line end");
		goto end;
	}
	tmp1_obj = json_object_array_get_idx(tmp_obj, 0);
	newLine->color[0] = json_object_get_int(tmp1_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, 1);
	newLine->color[1] = json_object_get_int(tmp1_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, 2);
	newLine->color[2] = json_object_get_int(tmp1_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, 3);
	newLine->color[3] = json_object_get_int(tmp1_obj);

	return 0;
end:
	return -EINVAL;
}

int freeParseHandle(json_object *config_obj)
{
	json_object_put(config_obj);
	return 0;
}

int setRegionSrc(char *file_name, int idx, OsdHandle *hd, COLOR_MODE mode)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;
	json_object *tmp3_obj = NULL;
	json_object *tmp4_obj = NULL;
	json_object *tmp5_obj = NULL;
	json_object *tmp6_obj = NULL;
	json_object *tmp7_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}

	json_object_object_get_ex(test_obj, "regions", &tmp_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, idx);
	if (!tmp1_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}

	json_object_object_get_ex(tmp1_obj, "src", &tmp2_obj);
	if (!tmp2_obj) {
		osddemo_log_err("failed to find src");
		goto end;
	}
	int src_num = json_object_array_length(tmp2_obj);
	if (src_num < hd->osd_num) {
		osddemo_log_err("some region no define src. src_num: %d, region_num: %d", src_num, hd->osd_num);
		goto end;
	}

	for (int i = 0; i < hd->osd_num; i++) {
		osddemo_log_debug("CHN %d has osd: %d", idx, i);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, i);
		if (!tmp3_obj) {
			osddemo_log_err("Cannot open %s", file_name);
			goto end;
		}

		json_object_object_get_ex(tmp3_obj, "type", &tmp4_obj);
		if (!tmp4_obj) {
			osddemo_log_err("Cannot open %s", file_name);
			goto end;
		}

		if (strcmp("none", json_object_get_string(tmp4_obj)) == 0) {
			osddemo_log_err("reg[%d] define empty, skip it", i);
			continue;
		}

		if (strcmp("utf8", json_object_get_string(tmp4_obj)) == 0) {
			osddemo_log_debug("reg[%d] utf8", i);
			OsdText txt;
			int w, h;

			if (parseUTF8(tmp3_obj, &txt, idx, i) != 0) {
				goto end;
			}
			txt.mode = mode;

			char *ayuv_src;
			if (mode == PALETTE_8) {
				ayuv_src = OSD_createTextUTF8Src8bit(&txt, &w, &h);
			} else {
				ayuv_src = OSD_createTextUTF8Src(&txt, &w, &h);
			}

			if (ayuv_src == NULL) {
				osddemo_log_err("failed to create utf8 src ptr");
				goto end;
			}

#ifdef DEBUG
			saveAYUV("save-at.ayuv", w, h, ayuv_src, w * h * 2);
#endif

			OSD_setImageAYUVptr(g_osd_handle[idx], i, ayuv_src, w, h, mode,
			                    (char *)(g_osd_canvas_attr[idx][hd->region[i].include_canvas].canvas_addr));

			OSD_destroySrc(ayuv_src);
		} else if (strcmp("unicode", json_object_get_string(tmp4_obj)) == 0) {
			OsdText txt;
			int ret = parseUnicode(tmp3_obj, &txt, idx, i, mode);
			osddemo_log_debug("parse unicode ret: %d", ret);

			if (ret != TIMESTAMP) {
				osddemo_log_err("reg[%d] unicode", i);

				int w, h;
				char *ayuv_unicode_src = OSD_createTextUnicodeSrc(&txt, &w, &h);
				if (ayuv_unicode_src == NULL) {
					osddemo_log_err("Failed to create ayuv src");
					goto end;
				}
				OSD_setImageAYUVptr(
				        g_osd_handle[idx], i, ayuv_unicode_src, w, h, mode,
				        (char *)(g_osd_canvas_attr[idx][hd->region[i].include_canvas].canvas_addr));

				OSD_destroySrc(ayuv_unicode_src);

			} else if (ret == TIMESTAMP) {
				osddemo_log_debug("reg[%d] unicode-timestamp", i);
			} else if (ret < 0) {
				goto end;
			}

		} else if (strcmp("ayuv", json_object_get_string(tmp4_obj)) == 0) {
			osddemo_log_debug("reg[%d] ayuv", i);
			if (mode == PALETTE_8) {
				osddemo_log_err("PALETTE_8 color mode not support ayuv3544 src, skip it");
				continue;
			}

			char src_path[128];
			json_object_object_get_ex(tmp3_obj, "src_path", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Cannot open src path");
				goto end;
			}
			sprintf(&src_path[0], "%s", json_object_get_string(tmp4_obj));
			OSD_setImage(g_osd_handle[idx], i, &src_path[0],
			             (char *)(g_osd_canvas_attr[idx][hd->region[i].include_canvas].canvas_addr));

		} else if (strcmp("bmp", json_object_get_string(tmp4_obj)) == 0) {
			osddemo_log_debug("reg[%d] bmp", i);
			if (mode == PALETTE_8) {
				osddemo_log_err("PALETTE_8 color mode not support ayuv3544 src, skip it");
				continue;
			}

			char src_path[128];
			json_object_object_get_ex(tmp3_obj, "src_path", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Cannot open src path");
				goto end;
			}
			sprintf(&src_path[0], "%s", json_object_get_string(tmp4_obj));
			osddemo_log_debug("chn[%d]reg[%d]include canvas %d", idx, i, hd->region[i].include_canvas);
			OSD_setImageBmp(g_osd_handle[idx], i, &src_path[0],
			                (char *)(g_osd_canvas_attr[idx][hd->region[i].include_canvas].canvas_addr));
		} else if (strcmp("line", json_object_get_string(tmp4_obj)) == 0) {
			osddemo_log_debug("reg[%d] line", i);

			int line_num = 0;
			json_object_object_get_ex(tmp3_obj, "line_num", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Cannot open line num");
				goto end;
			}

			line_num = json_object_get_int(tmp4_obj);
			OsdLine newLine[line_num];
			OsdPoint p[2];
			char color[4] = { 0, 0, 0, 0xff };
			int thickness;
			json_object_object_get_ex(tmp3_obj, "thickness", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Cannot open line num");
				goto end;
			}
			thickness = json_object_get_int(tmp4_obj);

			json_object_object_get_ex(tmp3_obj, "color", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Failed to parse line end");
				goto end;
			}
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 0);
			color[0] = json_object_get_int(tmp5_obj);
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 1);
			color[1] = json_object_get_int(tmp5_obj);
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 2);
			color[2] = json_object_get_int(tmp5_obj);
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 3);
			color[3] = json_object_get_int(tmp5_obj);

			json_object_object_get_ex(tmp3_obj, "lines", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Cannot open line num");
				goto end;
			}
			for (int j = 0; j < line_num; j++) {
				tmp5_obj = json_object_array_get_idx(tmp4_obj, j);
				if (!tmp5_obj) {
					osddemo_log_err("Cannot open line[%d]", j);
					goto end;
				}
				for (int k = 0; k < 2; k++) {
					/*start or end*/
					tmp6_obj = json_object_array_get_idx(tmp5_obj, k);
					if (!tmp6_obj) {
						osddemo_log_err("Cannot open line[%d]", j);
						goto end;
					}
					tmp7_obj = json_object_array_get_idx(tmp6_obj, 0);
					p[k].x = json_object_get_int(tmp7_obj);
					tmp7_obj = json_object_array_get_idx(tmp6_obj, 1);
					p[k].y = json_object_get_int(tmp7_obj);
				}

				osddemo_log_debug("%d %d %d %d", p[0].x, p[0].y, p[1].x, p[1].y);
				if (calcNewLine(p[0].x, p[0].y, p[1].x, p[1].y, thickness, &hd->region[i],
				                &newLine[j]) != 0) {
					osddemo_log_err("Failed to calc line [%d]", j);
					goto end;
				}

				memcpy(&newLine[j].color[0], &color[0], sizeof(color));
				newLine[j].mode = mode;
				OSD_setLine(g_osd_handle[idx], i, &newLine[j],
				            (char *)(g_osd_canvas_attr[idx][hd->region[i].include_canvas].canvas_addr));
			}

		} else if (strcmp("privacy_mask", json_object_get_string(tmp4_obj)) == 0) {
			osddemo_log_debug("reg[%d] privacy mask", i);

			char color[4] = { 0, 0, 0, 0xff };
			json_object_object_get_ex(tmp3_obj, "color", &tmp4_obj);
			if (!tmp4_obj) {
				osddemo_log_err("Failed to parse line end");
				goto end;
			}
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 0);
			color[0] = json_object_get_int(tmp5_obj);
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 1);
			color[1] = json_object_get_int(tmp5_obj);
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 2);
			color[2] = json_object_get_int(tmp5_obj);
			tmp5_obj = json_object_array_get_idx(tmp4_obj, 3);
			color[3] = json_object_get_int(tmp5_obj);

			OSD_setPrivacyMask(g_osd_handle[idx], i, &color[0], mode,
			                   (char *)(g_osd_canvas_attr[idx][hd->region[i].include_canvas].canvas_addr));

		} else {
			osddemo_log_err("Invalid region type: %s", json_object_get_string(tmp4_obj));
			break;
		}

		if (MPI_updateOsdCanvas(g_osd_chn_handle[idx][hd->region[i].include_canvas]) != MPI_SUCCESS) {
			osddemo_log_err("failed to update canvas");
			MPI_destroyOsdRgn(g_osd_chn_handle[idx][hd->region[i].include_canvas]);
		}
	}

end:
	freeParseHandle(test_obj);
	return 0;
}

int getRegionRect(char *file_name, int idx, int reg_idx, int *x, int *y, int *w, int *h)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;
	json_object *tmp3_obj = NULL;
	json_object *tmp4_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		return -EBADF;
	}

	json_object_object_get_ex(test_obj, "regions", &tmp_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, idx);
	if (!tmp1_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}
	json_object_object_get_ex(tmp1_obj, "region", &tmp2_obj);
	tmp3_obj = json_object_array_get_idx(tmp2_obj, reg_idx);
	if (!tmp3_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}

	tmp4_obj = json_object_array_get_idx(tmp3_obj, 0);
	*x = json_object_get_int(tmp4_obj);

	tmp4_obj = json_object_array_get_idx(tmp3_obj, 1);
	*y = json_object_get_int(tmp4_obj);

	tmp4_obj = json_object_array_get_idx(tmp3_obj, 2);
	*w = json_object_get_int(tmp4_obj);

	tmp4_obj = json_object_array_get_idx(tmp3_obj, 3);
	*h = json_object_get_int(tmp4_obj);

end:
	freeParseHandle(test_obj);
	return 0;
}

int getRegionNum(char *file_name, int idx, int *num)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}

	json_object_object_get_ex(test_obj, "regions", &tmp_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, idx);
	if (!tmp1_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}
	json_object_object_get_ex(tmp1_obj, "region_num", &tmp2_obj);
	if (json_object_get_int(tmp2_obj) > MAX_OSD) {
		osddemo_log_err("region num > max %d", MAX_OSD);
		goto end;
	}
	*num = json_object_get_int(tmp2_obj);

	freeParseHandle(test_obj);
	return 0;

end:
	freeParseHandle(test_obj);
	return -EINVAL;
}

int getColorMode(char *file_name, COLOR_MODE *mode)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}

	json_object_object_get_ex(test_obj, "color_mode", &tmp_obj);
	if (!tmp_obj) {
		osddemo_log_err("Failed to find color mode");
		goto end;
	}

	if (strcmp("AYUV_3544", json_object_get_string(tmp_obj)) == 0) {
		*mode = AYUV_3544;
	} else if (strcmp("PALETTE_8", json_object_get_string(tmp_obj)) == 0) {
		*mode = PALETTE_8;
	} else {
		osddemo_log_err("Failed to find color mode");
		goto end;
	}

	freeParseHandle(test_obj);
	return 0;
end:
	freeParseHandle(test_obj);
	return -EBADF;
}

int getHandleRect(char *file_name, int idx, int *width, int *height)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}

	json_object_object_get_ex(test_obj, "rect", &tmp_obj);
	tmp1_obj = json_object_array_get_idx(tmp_obj, idx);
	if (!tmp1_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		goto end;
	}
	tmp2_obj = json_object_array_get_idx(tmp1_obj, 0);
	*width = json_object_get_int(tmp2_obj);
	tmp2_obj = json_object_array_get_idx(tmp1_obj, 1);
	*height = json_object_get_int(tmp2_obj);

	freeParseHandle(test_obj);
	return 0;
end:
	freeParseHandle(test_obj);
	return -EBADF;
}

int getConfigChnNum(char *file_name, int *num, char *channel_id)
{
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	int channel_idx = 0;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		osddemo_log_err("Cannot open %s", file_name);
		return -EBADF;
	}

	json_object_object_get_ex(test_obj, "channel", &tmp_obj);
	if (!tmp_obj) {
		osddemo_log_err("failed to find channel");
	}
	*num = json_object_array_length(tmp_obj);
	osddemo_log_debug("channel num: %d", *num);

	for (int i = 0; i < *num; i++) {
		tmp1_obj = json_object_array_get_idx(tmp_obj, i);
		if (!tmp1_obj) {
			osddemo_log_err("Failed to get chn id");
			break;
		}
		channel_idx = json_object_get_int(tmp1_obj);
		memcpy((void *)(channel_id + i), &channel_idx, 1);
	}

	freeParseHandle(test_obj);

	return 0;
}

void help()
{
	printf("usage:\n");
	printf("-h help()\n");
	printf("-c config path\n");
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
		perror("Cannot handle SIGINT!");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!");
		exit(1);
	}

	int c = 0;
	char config_path[128];
	while ((c = getopt(argc, argv, "hc:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'c':
			printf("config : %s", argv[optind - 1]);
			snprintf(&config_path[0], 128, "%s", argv[optind - 1]);
			break;
		}
	}
	MPI_SYS_init();

	int channel_num = 0;
	MPI_ENC_CHN_ATTR_S chn_attr[MPI_MAX_ENC_CHN_NUM];

	for (int i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		if (0 == checkChnExist(i, &chn_attr[i])) {
			channel_num++;
		} else {
			break;
		}
	}

	for (int i = 0; i < channel_num; i++) {
		stopEnc(i);
		osddemo_log_debug("%d, %d", chn_attr[i].res.width, chn_attr[i].res.height);
	}

	sleep(2);

	int config_num, width, height, region_num;
	char osd_channel[MPI_MAX_ENC_CHN_NUM];
	if (0 != getConfigChnNum(&config_path[0], &config_num, &osd_channel[0])) {
		goto end;
	}
	osddemo_log_debug("CONFIG num: %d", config_num);

	printf("\nconfig_num : %d\n", config_num);
	printf("\nchannel_number : %d\n", channel_num);
	printf("osd_channel size : %u\n", sizeof(osd_channel) / sizeof(osd_channel[0]));
	for (unsigned int i = 0; i < sizeof(osd_channel) / sizeof(osd_channel[0]); ++i) {
		printf("osd_channel[%u] : %d\n", i, osd_channel[i]);
	}

	if (config_num > channel_num) {
		osddemo_log_err("Config chn > exist chanenel, err");
		goto end;
	}

	for (int i = 0; i < config_num; i++) {
		osddemo_log_debug("[%d] chn %d", i, osd_channel[i]);
	}

	OSD_init();

	memset(&g_timestamp_regions, 0, sizeof(g_timestamp_regions));

	MPI_ECHN echn_idx;
	echn_idx = MPI_ENC_CHN(0);
	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { .x = 0, .y = 0 },
		.module = 0,
		.idx = { .chn = 0 },
	};

	MPI_OSD_RGN_ATTR_S osd_attr = { .show = true,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };
	COLOR_MODE mode = AYUV_3544;
	if (0 != getColorMode(&config_path[0], &mode)) {
		goto deinit;
	}

	if (mode == PALETTE_8) {
		osddemo_log_err("get color mode PALETTE_8");
		osd_attr.color_format = MPI_OSD_COLOR_FORMAT_PALETTE_8;
	}
	printf("\nconfig_num : %d\n", config_num);
	for (int i = 0; i < config_num; i++) {
		if (0 != checkChnExist(osd_channel[i], &chn_attr[i])) {
			osddemo_log_err("chn[%d] not exist", i);
			goto deinit;
		}
		printf("osd_channel[%d] : %d \n", i, osd_channel[i]);

		if (0 != getHandleRect(&config_path[0], i, &width, &height)) {
			goto end;
		}

		osddemo_log_debug("(%d, %d)", width, height);

		if ((width <= chn_attr[i].res.width) && (height <= chn_attr[i].res.height)) {
			g_osd_handle[i] = OSD_create(width, height);
		} else {
			osddemo_log_err("exceed width, height of chn %d, %d", width, height);
			goto deinit;
		}

		if (0 != getRegionNum(&config_path[0], i, &region_num)) {
			goto deinit;
		}

		osddemo_log_debug("REGION num: %d", region_num);
		OsdRegion region[8];
		memset(&region[0], 0, sizeof(region));

		int x, y, w, h;

		for (int j = 0; j < region_num; j++) {
			getRegionRect(&config_path[0], i, j, &x, &y, &w, &h);
			region[j].startX = x;
			region[j].startY = y;
			region[j].width = w;
			region[j].height = h;
			osddemo_log_debug("get %d %d %d %d", x, y, w, h);
		}

		for (int j = 0; j < region_num; j++) {
			if (OSD_addOsd(g_osd_handle[i], j, &region[j]) != 0) {
				goto deinit;
			}
		}

		if (region_num > 4) {
			if (OSD_calcCanvasbygroup(g_osd_handle[i]) != 0) {
				goto deinit;
			}
		} else {
			if (OSD_calcCanvas(g_osd_handle[i]) != 0) {
				goto deinit;
			}
		}

		logAllOsdHandle(g_osd_handle[i]);

		echn_idx = MPI_ENC_CHN(0);
		echn_idx.chn = osd_channel[i];
		osd_bind.idx = echn_idx;

		int calc_canvas = 4;
		if (region_num < 4) {
			calc_canvas = region_num;
		}
		for (int j = 0; j < calc_canvas; j++) {
			osd_attr.size.width = g_osd_handle[i]->canvas[j].width;
			osd_attr.size.height = g_osd_handle[i]->canvas[j].height;
			osd_bind.point.x = g_osd_handle[i]->canvas[j].startX;
			osd_bind.point.y = g_osd_handle[i]->canvas[j].startY;
			if (0 != createOsdInstance(&osd_attr, &osd_bind, &g_osd_chn_handle[i][j],
			                           &g_osd_canvas_attr[i][j])) {
				osddemo_log_err("failed to init canvas[%d]", j);
				goto deinit;
			}
		}
		osddemo_log_debug("\nFisnish createOsdInstance for loop in channel %d \n", osd_channel[i]);
		setRegionSrc(&config_path[0], i, g_osd_handle[i], mode);
		osddemo_log_debug("\nFisinsh setRegionSrc for loop in channel %d \n", osd_channel[i]);
	}

	for (int i = 0; i < channel_num; i++) {
		resumeEnc(i);
	}

	gRunflag = 1;
	int include_canvas = 0;
	int reg_idx = 0;
	uint16_t timestamp_list[31];
	int total_ayuv_width = 0;
	int total_ayuv_height = 0;
	int cnt = 0;

	while (gRunflag) {
		for (int i = 0; i < MAX_CANVAS_NUM; i++) {
			osddemo_log_debug("chn %d, has timestamp :%d", i, g_timestamp_regions.region_num[i]);
			for (int j = 0; j < g_timestamp_regions.region_num[i]; j++) {
				osddemo_log_debug("type: %d, idx: %d", g_timestamp_regions.region_type[i][j],
				                  g_timestamp_regions.region_list[i][j]);
				reg_idx = g_timestamp_regions.region_list[i][j];

				include_canvas = g_osd_handle[i]->region[reg_idx].include_canvas;

				OSD_setRegionTransparent(g_osd_handle[i], reg_idx, mode,
				                         (char *)(g_osd_canvas_attr[i][include_canvas].canvas_addr));

				if (g_timestamp_regions.region_type[i][j] == UTF8_TS) {
					osddemo_log_err("utf8 not support timestamp");
				} else if (g_timestamp_regions.region_type[i][j] == UNICODE_TS) {
					parseTimestampUnicode(&timestamp_list[0]);
					OSD_getUnicodeSizetoGenerate(&timestamp_list[0], 31,
					                             g_timestamp_regions.ayuv_src_list[i][reg_idx],
					                             &total_ayuv_width, &total_ayuv_height);

					osddemo_log_debug("get total (%d, %d)", total_ayuv_width, total_ayuv_height);

					char ayuv_buf[total_ayuv_width * total_ayuv_height * 2];
					if (mode == PALETTE_8) {
						OSD_generateUnicodeFromList8bit(
						        &timestamp_list[0], 31,
						        g_timestamp_regions.ayuv_src_list[i][reg_idx], &ayuv_buf[0],
						        total_ayuv_width, total_ayuv_height);
					} else if (mode == AYUV_3544) {
						OSD_generateUnicodeFromList(
						        &timestamp_list[0], 31,
						        g_timestamp_regions.ayuv_src_list[i][reg_idx], &ayuv_buf[0],
						        total_ayuv_width, total_ayuv_height);
					}

					osddemo_log_debug("set timestamp to %d %d", i, reg_idx);
					OSD_setImageAYUVptr(g_osd_handle[i], reg_idx, &ayuv_buf[0], total_ayuv_width,
					                    total_ayuv_height, mode,
					                    (char *)(g_osd_canvas_attr[i][include_canvas].canvas_addr));
#ifdef DEBUG
					char ayuv_name[32];
					sprintf(&ayuv_name[0], "/mnt/nfs/ethnfs/%d-%d-%d.ayuv", i, reg_idx, cnt);
					saveAYUV(&ayuv_name[0], total_ayuv_width, total_ayuv_height, &ayuv_buf[0],
					         total_ayuv_width * total_ayuv_width * 2);

#endif
				}

				if (MPI_updateOsdCanvas(g_osd_chn_handle[i][include_canvas]) != MPI_SUCCESS) {
					osddemo_log_err("failed to update canvas");
					MPI_destroyOsdRgn(g_osd_chn_handle[i][include_canvas]);
					gRunflag = 0;
				}
			}
		}

		sleep(1);
		cnt++;
	}

	for (int i = 0; i < channel_num; i++) {
		stopEnc(i);
	}

	sleep(2);

	/*remove all old osd canvas*/
	osddemo_log_debug("remove all OSD");
	int all_canvas_in_handle = 4;
	for (int i = 0; i < config_num; i++) {
		if (g_osd_handle[i]->osd_num < 4) {
			all_canvas_in_handle = g_osd_handle[i]->osd_num;
		}

		echn_idx.chn = osd_channel[i];
		osd_bind.idx = echn_idx;
		for (int j = 0; j < all_canvas_in_handle; j++) {
			if (MPI_unbindOsdFromChn(g_osd_chn_handle[i][j], &osd_bind) != MPI_SUCCESS) {
				osddemo_log_debug("failed to unbind chn");
			}

			if (MPI_destroyOsdRgn(g_osd_chn_handle[i][j]) != MPI_SUCCESS) {
				osddemo_log_debug("failed to unbind chn");
			}
		}
	}

	for (int i = 0; i < config_num; i++) {
		OSD_destroy(g_osd_handle[i]);
	}

	for (int i = 0; i < MPI_MAX_ENC_CHN_NUM; i++) {
		osddemo_log_debug("chn %d, has timestamp :%d", i, g_timestamp_regions.region_num[i]);
		for (int j = 0; j < g_timestamp_regions.region_num[i]; j++) {
			reg_idx = g_timestamp_regions.region_list[i][j];
			osddemo_log_debug("at region: %d", reg_idx);
			OSD_destroyUnicodeFontList(g_timestamp_regions.ayuv_src_list[i][reg_idx]);
		}
	}

deinit:
	OSD_deinit();

end:
	for (int i = 0; i < channel_num; i++) {
		resumeEnc(i);
	}

	MPI_SYS_exit();

	return 0;
}
