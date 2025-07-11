#include "sample_osd.h"

#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "sample_stream.h"

#ifndef LIBOSD_ENABLE
#define FONT_FILE "/system/mpp/font/360_959_font.ayuv"
#define LOGO_FILE "/system/mpp/font/LOGO_Augentix_v2.imgayuv"

#define DISPLAYABLE_CHARACTER_NUMBERS 104 /**< Number of displayable ASCII character. */

#define LOGO_OSD_SIZE_WIDTH 544 /**< Default width of logo OSD region. */
#define LOGO_OSD_SIZE_HEIGHT 128 /**< Default height of logo OSD region. */
#define NUMBER_OSD_SIZE_WIDTH 160 /**< Default width of number OSD region. */
#define NUMBER_OSD_SIZE_HEIGHT 48 /**< Default height of number OSD region. */
#define TIME_OSD_SIZE_WIDTH 416 /**< Default width of time OSD region. */
#define TIME_OSD_SIZE_HEIGHT 48 /**< Default height of time OSD region. */

#define ALIGN_16(x) (((x) / 16) * 16) /**< Round down number to multiples of 16 */

typedef struct {
	UINT32 index_offset;
	UINT32 index_size;
	UINT32 ascii_index;
	UINT32 ascii_width;
	UINT32 cht_index;
	UINT32 cht_width;
	UINT32 other_index;
	UINT32 other_width;
	UINT32 data_offset;
	UINT32 data_size;
} AyuvInfo_S;
/**
* @brief Struct for ASCII index.
*/
typedef struct ascii_index {
	uint32_t image_offset;
	uint32_t image_size;
	uint32_t image_width;
	uint32_t image_height;
} ASCII_INDEX;

typedef struct osd_instance {
	OSD_HANDLE handle;
	MPI_OSD_CANVAS_ATTR_S canvas;
} OSD_INSTANCE;

static OSD_HANDLE g_handle[MPI_MAX_VIDEO_CHN_NUM][MPI_OSD_MAX_BIND_CHANNEL] = { { 0 } };
static pthread_t threadOSDupdate;
static int g_osd_run = 0;
static char *p_font_ayuv = NULL;
static char *p_logo_ayuv = NULL;
static ASCII_INDEX osdindex[DISPLAYABLE_CHARACTER_NUMBERS];
static uint32_t channel_numbers;
static uint32_t pic_image_width;
static uint32_t pic_image_height;

static MPI_OSD_CANVAS_ATTR_S p_canvas_attr_time[MPI_MAX_VIDEO_CHN_NUM] = { { 0 } };
static MPI_OSD_CANVAS_ATTR_S p_canvas_attr_logo[MPI_MAX_VIDEO_CHN_NUM] = { { 0 } };
static MPI_OSD_CANVAS_ATTR_S p_canvas_attr_number[MPI_MAX_VIDEO_CHN_NUM] = { { 0 } };

static int getFontAYUV(void)
{
	FILE *fp;
	AyuvInfo_S info;
	int ret = MPI_SUCCESS;

	fp = fopen(FONT_FILE, "rb");
	if (fp == NULL) {
		printf("Cannot open %s. err: %d", FONT_FILE, -errno);
		return -errno;
	}

	fread(&info, sizeof(info), 1, fp);

	fseek(fp, info.index_offset, SEEK_SET);
	fread(&osdindex[0], info.index_size, 1, fp);

	fseek(fp, info.data_offset, SEEK_SET);

	p_font_ayuv = malloc(info.data_size);
	if (p_font_ayuv == NULL) {
		printf("Cannot allocate memory for %s. err: %d", FONT_FILE, -ENOMEM);
		ret = -ENOMEM;
		goto closefd;
	}
	fread(p_font_ayuv, info.data_size, 1, fp);

closefd:

	fclose(fp);
	return ret;
}

static int getLogoAYUV(void)
{
	FILE *fp;
	int pic_image_size;
	int ret = MPI_SUCCESS;

	fp = fopen(LOGO_FILE, "rb");
	if (fp == NULL) {
		printf("Cannot open %s. err: %d", LOGO_FILE, -errno);
		return -errno;
	}

	fread(&pic_image_width, sizeof(UINT32), 1, fp);

	fseek(fp, 4, SEEK_SET);
	fread(&pic_image_height, sizeof(UINT32), 1, fp);

	fseek(fp, 8, SEEK_SET);
	fread(&pic_image_size, sizeof(UINT32), 1, fp);

	p_logo_ayuv = malloc(pic_image_size);
	if (p_logo_ayuv == NULL) {
		printf("Cannot allocate memory for %s. err: %d", LOGO_FILE, -ENOMEM);
		ret = -ENOMEM;
		goto closefd;
	}

	fseek(fp, 12, SEEK_SET);
	fread(p_logo_ayuv, pic_image_size, 1, fp);

closefd:

	fclose(fp);
	return ret;
}

/**
 * @brief Create OSD instances.
 * @param[in]  attr      OSD region attribute
 * @param[in]  bind      OSD binding attribute
 * @param[out] handle    OSD handle
 * @param[out] canvas    OSD canvas
 * @return The execution result
 * @retval MPI_SUCCESS    success
 * @retval others         unexpected failure
 */
static INT32 createOsdInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle,
                               MPI_OSD_CANVAS_ATTR_S *canvas)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		printf("MPI_createOsdRgn() failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_getOsdCanvas(*handle, canvas);
	if (ret != MPI_SUCCESS) {
		printf("MPI_getOsdCanvas() failed. err: %d\n", ret);
		goto release;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		printf("Bind OSD %d to encoder channel %d failed. err: %d\n", *handle, bind->idx.chn, ret);
		goto release;
	}

	return ret;

release:

	MPI_destroyOsdRgn(*handle);

	return ret;
}

/**
 * @brief Thread for updating clock for every second.
 */
static int updateOsdClock(void)
{
	int string_lens = 0;
	unsigned char input_ascii = 0;
	void *real_addr = 0;
	INT32 ret = 0;
	char timestring[128];
	int width_acc = 0;
	const int week[7][3] = { { 95, 96, 103 }, { 95, 96, 97 },  { 95, 96, 98 }, { 95, 96, 99 },
		                 { 95, 96, 100 }, { 95, 96, 101 }, { 95, 96, 102 } };
	int i = 0, j = 0;

	struct tm *gmt_tm;
	struct timespec start_time, wall_time;
	time_t t;
	int32_t timeout_first_ms = 2200;
	int32_t timeout_other_ms = 1200;
	int32_t timeout_ms = 0;

	clock_gettime(CLOCK_REALTIME, &wall_time);
	start_time = wall_time;

	wall_time.tv_sec -= 2;

	int wait_first_frame = 1;

	while (g_osd_run) {
		width_acc = 0;
		/** 5 sec is adjustable for early video*/
		if (wall_time.tv_sec - start_time.tv_sec > 5) {
			clock_gettime(CLOCK_REALTIME, &wall_time);
		}

		t = time(NULL);
		gmt_tm = localtime(&t);
		sprintf(timestring, " %4u-%02u-%02u  %02u:%02u:%02u ", gmt_tm->tm_year + 1900, gmt_tm->tm_mon + 1,
		        gmt_tm->tm_mday, gmt_tm->tm_hour, gmt_tm->tm_min, gmt_tm->tm_sec);
		string_lens = strlen(timestring);

		timeout_ms = (wait_first_frame ? timeout_first_ms : timeout_other_ms);

		if (p_canvas_attr_time[0].canvas_addr != 0) {
			/** Update the pixel row-by-row, then update the text letter-by-letter */
			for (i = 0; i < 12; i++) {
				input_ascii = timestring[i];
				input_ascii = input_ascii - 32;
				real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

				for (j = 0; (UINT32)j < (osdindex[input_ascii].image_height); j++) {
					memcpy((void *)((p_canvas_attr_time[0].canvas_addr) +
					                (j * TIME_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
					       real_addr, (osdindex[input_ascii].image_width * 2));
					real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
				}
				width_acc = width_acc + osdindex[input_ascii].image_width;
			}

			for (i = 0; i < 3; i++) {
				input_ascii = week[gmt_tm->tm_wday][i];
				real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

				for (j = 0; (UINT32)j < (osdindex[input_ascii].image_height); j++) {
					memcpy((void *)((p_canvas_attr_time[0].canvas_addr) +
					                (j * TIME_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
					       real_addr, (osdindex[input_ascii].image_width * 2));
					real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
				}
				width_acc = width_acc + osdindex[input_ascii].image_width;
			}

			for (i = 12; i < string_lens; i++) {
				input_ascii = timestring[i];
				input_ascii = input_ascii - 32;
				real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

				for (j = 0; (UINT32)j < (osdindex[input_ascii].image_height); j++) {
					memcpy((void *)((p_canvas_attr_time[0].canvas_addr) +
					                (j * TIME_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
					       real_addr, (osdindex[input_ascii].image_width * 2));
					real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
				}
				width_acc = width_acc + osdindex[input_ascii].image_width;
			}
			if (g_osd_run) {
				ret = MPI_updateOsdCanvasAtTimeV2(g_handle[0][2], &wall_time, timeout_ms);
				if (ret == MPI_SUCCESS) {
					/* do nothing */
				} else if (ret == -ETIMEDOUT) {
					/* do nothing */
				} else {
					printf("OSD %d to update canvas failed. err: %d\n", g_handle[0][2], ret);
					return MPI_FAILURE;
				}
			}
		}

		if (p_canvas_attr_time[1].canvas_addr != 0) {
			if (channel_numbers > 1) {
				memcpy((void *)(p_canvas_attr_time[1].canvas_addr),
				       (void *)(p_canvas_attr_time[0].canvas_addr),
				       TIME_OSD_SIZE_WIDTH * TIME_OSD_SIZE_HEIGHT * 2);
			}

			if (channel_numbers > 1 && g_osd_run) {
				ret = MPI_updateOsdCanvasAtTimeV2(g_handle[1][2], &wall_time, timeout_ms);
				if (ret == MPI_SUCCESS) {
					/* do nothing */
				} else if (ret == -ETIMEDOUT) {
					/* do nothing */
				} else {
					printf("OSD %d to update canvas failed.\n", g_handle[1][2]);
					return MPI_FAILURE;
				}
			}
		}

		if (p_canvas_attr_time[2].canvas_addr != 0) {
			if (channel_numbers == 3) {
				memcpy((void *)(p_canvas_attr_time[2].canvas_addr),
				       (void *)(p_canvas_attr_time[0].canvas_addr),
				       TIME_OSD_SIZE_WIDTH * TIME_OSD_SIZE_HEIGHT * 2);
			}

			if (channel_numbers == 3 && g_osd_run) {
				ret = MPI_updateOsdCanvasAtTimeV2(g_handle[2][2], &wall_time, timeout_ms);
				if (ret == MPI_SUCCESS) {
					/* do nothing */
				} else if (ret == -ETIMEDOUT) {
					/* do nothing */
				} else {
					printf("OSD %d to update canvas failed.\n", g_handle[2][2]);
					return MPI_FAILURE;
				}
			}
		}

		if (wait_first_frame) {
			wait_first_frame = 0;
		}

		wall_time.tv_sec++;
	}

	return 0;
}

#else

#include "libosd.h"
#define OSD_NUM (3)

OsdHandle *g_osd_handle[3];
OSD_HANDLE g_osd_chn_handle[3][OSD_NUM];
MPI_OSD_CANVAS_ATTR_S g_osd_canvas_attr[3][OSD_NUM];

pthread_t osd_tid;
int g_run_osd_flag;
int channel_num = 0;

#define ALIGN_16(x) (((x) / 16) * 16) /**< Round down number to multiples of 16 */

uint16_t unicode_by_day[7][3] = { { 0x661f, 0x671f, 0x5929 }, { 0x661f, 0x671f, 0x4e00 }, { 0x661f, 0x671f, 0x4e8c },
	                          { 0x661f, 0x671f, 0x4e09 }, { 0x661f, 0x671f, 0x56db }, { 0x661f, 0x671f, 0x4e94 },
	                          { 0x661f, 0x671f, 0x516d } };

static INT32 createOsdInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle,
                               MPI_OSD_CANVAS_ATTR_S *canvas)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		printf("MPI_createOsdRgn() failed. err: %d\n", ret);
		return ret;
	}

	ret = MPI_getOsdCanvas(*handle, canvas);
	if (ret != MPI_SUCCESS) {
		printf("MPI_getOsdCanvas() failed. err: %d\n", ret);
		goto release;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		printf("Bind OSD %d to encoder channel %d failed. err: %d\n", *handle, bind->idx.chn, ret);
		goto release;
	}

	return ret;

release:

	MPI_destroyOsdRgn(*handle);

	return ret;
}

static void *run_update_timestamp(void *arg)
{
	(void)(arg);

	struct tm *gmt_tm;
	struct timespec start_time, wall_time;
	char text[64];
	int include_canvas = 0;
	uint16_t unicode_list[31];
	OsdText txt = { .size = 27,
		        .mode = AYUV_3544,
		        .background = TRANSPARENT,
		        .color = { 0xff, 0xff, 0xff },
		        .outline_color = { 0x00, 0x00, 0x00 },
		        .outline_width = 1,
		        .kerning_mode = AUTO,
		        .kerning = AUTO_KERNING_RATE };
	int32_t timeout_first_ms = 2200;
	int32_t timeout_other_ms = 1200;
	int32_t timeout_ms = 0;
	INT32 ret = 0;

#ifdef INSTALL_TRADITIONAL_OTF
	snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansTC-Normal.otf");
#endif

#ifdef INSTALL_SIMPLIFIED_OTF
	snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansCN-Regular_1.otf");
#endif

	clock_gettime(CLOCK_REALTIME, &wall_time);
	wall_time.tv_sec -= 2;
	start_time = wall_time;

	int wait_first_frame = 1;

	while (g_run_osd_flag) {
		for (int i = 0; i < channel_num; i++) {
			if (g_osd_canvas_attr[i][2].canvas_addr == 0) {
				continue;
			}

			/** 5 sec is adjustable for early video*/
			if (wall_time.tv_sec - start_time.tv_sec > 5) {
				clock_gettime(CLOCK_REALTIME, &wall_time);
			}
			gmt_tm = gmtime(&wall_time.tv_sec);

			snprintf(&text[0], 64, " %d-%02d-%02d  ", gmt_tm->tm_year + 1900, gmt_tm->tm_mon + 1,
			         gmt_tm->tm_mday);
			for (int i = 0; i < 13; i++) {
				unicode_list[i] = OSD_trans2Unicode(text[i]);
			}

			memcpy(&unicode_list[13], &unicode_by_day[gmt_tm->tm_wday][0], sizeof(uint16_t) * 3);

			snprintf(&text[0], 64, "  %02d:%02d:%02d ", gmt_tm->tm_hour, gmt_tm->tm_min, gmt_tm->tm_sec);

			for (int i = 0; i < 11; i++) {
				unicode_list[13 + 3 + i] = OSD_trans2Unicode(text[i]);
			}

			memset(&txt.unicode_txt[0], 0x00, sizeof(txt.unicode_txt));
			memcpy(&txt.unicode_txt[0], &unicode_list[0], sizeof(unicode_list));
			include_canvas = g_osd_handle[i]->region[2].include_canvas;
			char *ayuv_src;
			int w, h;
			ayuv_src = OSD_createTextUnicodeSrc(&txt, &w, &h);
			OSD_setImageAYUVptr(g_osd_handle[i], 2, ayuv_src, w, h, AYUV_3544,
			                    (char *)(g_osd_canvas_attr[i][include_canvas].canvas_addr));

			if (!g_run_osd_flag) {
				continue;
			}
			timeout_ms = (wait_first_frame ? timeout_first_ms : timeout_other_ms);
			ret = MPI_updateOsdCanvasAtTimeV2(g_osd_chn_handle[i][include_canvas], &wall_time, timeout_ms);
			if (ret == MPI_SUCCESS) {
				/* do nothing*/
			} else if (ret == -ETIMEDOUT) {
				/* do nothing */
			} else {
				fprintf(stderr, "failed to update canvas\r\n");
				MPI_destroyOsdRgn(g_osd_chn_handle[i][include_canvas]);
			}

			OSD_destroySrc(ayuv_src);
		}

		if (wait_first_frame) {
			wait_first_frame = 0;
		}

		wall_time.tv_sec++;
	}

	return NULL;
}
#endif

/**
 * @brief Initialize OSD resources.
 * @details Include OSD handle and AYUV files.
 */
void SAMPLE_initOsd(void)
{
#ifdef LIBOSD_ENABLE
	OSD_init();
#else
	/* Initialize OSD handle to invalid value */
	memset(g_handle, -1, sizeof(g_handle));

	/* Initialize AYUV files */
	if (p_font_ayuv == NULL) {
		getFontAYUV();
	}

	if (p_logo_ayuv == NULL) {
		getLogoAYUV();
	}
#endif
}

/**
 * @brief Stop OSD.
 * @return The execution result.
 */
INT32 SAMPLE_stopOsd(MPI_CHN chn_idx)
{
#ifdef LIBOSD_ENABLE
	MPI_ECHN echn = MPI_ENC_CHN(chn_idx.chn);
	const uint8_t c_idx = MPI_GET_VIDEO_CHN(chn_idx);
	INT32 ret = 0;

	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { 0 },
		.module = 0,
		.idx = { { 0 } },
	};

	osd_bind.idx = echn;

	for (int i = 0; i < OSD_NUM; i++) {
		if (g_osd_chn_handle[c_idx][i] >= 0) {
			if (MPI_unbindOsdFromChn(g_osd_chn_handle[c_idx][i], &osd_bind) != MPI_SUCCESS) {
				fprintf(stderr, "failed to unbind chn, %d\r\n", ret);
			}

			if (MPI_destroyOsdRgn(g_osd_chn_handle[c_idx][i]) != MPI_SUCCESS) {
				fprintf(stderr, "failed to unbind chn, %d\r\n", ret);
			}
		}
	}
	OSD_destroy(g_osd_handle[c_idx]);
#else
	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { 0 },
		.module = 0,
		.idx = { { 0 } },
	};
	MPI_ECHN e_chn = MPI_ENC_CHN(chn_idx.chn);
	INT32 ret = 0;
	const uint8_t c_idx = MPI_GET_VIDEO_CHN(chn_idx);
	int i;

	/* Unbind OSD from a video channel */
	for (i = 0; i < MPI_OSD_MAX_BIND_CHANNEL; i++) {
		if (g_handle[c_idx][i] >= 0) {
			osd_bind.idx = e_chn;
			ret = MPI_unbindOsdFromChn(g_handle[c_idx][i], &osd_bind);
			if (ret != MPI_SUCCESS) {
				printf("Unbind OSD %d from channel %d failed. err: %d\n", g_handle[c_idx][i],
				       MPI_GET_VIDEO_CHN(chn_idx), ret);
			}
		}
	}

	/* Destroy OSD region */
	for (i = 0; i < MPI_OSD_MAX_BIND_CHANNEL; i++) {
		if (g_handle[c_idx][i] >= 0) {
			ret = MPI_destroyOsdRgn(g_handle[c_idx][i]);
			if (ret != MPI_SUCCESS) {
				printf("Destroy OSD failed. err: %d\n", ret);
			}
			g_handle[c_idx][i] = -1;
		}
	}
#endif

	return MPI_SUCCESS;
}

void SAMPLE_freeOsdResources(void)
{
#ifdef LIBOSD_ENABLE
	OSD_deinit();
#else
	free(p_font_ayuv);
	free(p_logo_ayuv);
	p_font_ayuv = NULL;
	p_logo_ayuv = NULL;
#endif
}

INT32 SAMPLE_createOsdUpdateThread(void)
{
#ifdef LIBOSD_ENABLE
	g_run_osd_flag = 1;
	pthread_create(&osd_tid, NULL, run_update_timestamp, NULL);
#else
	INT32 ret;

	g_osd_run = 1;

	ret = pthread_create(&threadOSDupdate, NULL, (void *)updateOsdClock, NULL);
	if (ret) {
		printf("Failed to create OSDUpdate thread! err: %d\n", ret);
		return ret;
	}
#endif

	return MPI_SUCCESS;
}

INT32 SAMPLE_destroyOsdUpdateThread(void)
{
#ifdef LIBOSD_ENABLE
	if (g_run_osd_flag != 0) {
		g_run_osd_flag = 0;
		pthread_join(osd_tid, NULL);
	}
#else
	if (g_osd_run != 0) {
		g_osd_run = 0;
		pthread_join(threadOSDupdate, NULL);
	}

#endif
	return MPI_SUCCESS;
}

/**
 * @brief Create OSD regions to target channel.
 * @param[in] visible       default value for OSD visible
 * @param[in] chn_idx       video channel index
 * @param[in] output_num    total number of channels (deprecated)
 * @param[in] width         width of video channel
 * @param[in] height        height of video channel
 * @return The execution result.
 */
INT32 SAMPLE_createOsd(bool visible, MPI_CHN chn_idx, INT32 output_num, UINT16 width, UINT16 height)
{
#ifdef LIBOSD_ENABLE
	const int c_idx = MPI_GET_VIDEO_CHN(chn_idx);
	int include_canvas = 0;
	g_osd_handle[c_idx] = OSD_create(width, height);
	OsdRegion region[OSD_NUM] = {
		{ .startX = 16, .startY = 16, .width = 503, .height = 118 },
		{ .startX = width - 160, .startY = 16, .width = 160, .height = 48 },
		{ .startX = ALIGN_16(width - 420), .startY = ALIGN_16(height - 48), .width = 420, .height = 48 }
	};

	for (int i = 0; i < OSD_NUM; i++) {
		if (OSD_addOsd(g_osd_handle[c_idx], i, &region[i]) != 0) {
			fprintf(stderr, "failed to add region[%d], chn[%d]\r\n", i, c_idx);
		}
	}

	if (OSD_calcCanvas(g_osd_handle[c_idx]) != 0) {
		fprintf(stderr, "failed to calc canvas %d\n", c_idx);
		OSD_destroy(g_osd_handle[c_idx]);
	}
	MPI_OSD_RGN_ATTR_S osd_attr = { .show = visible,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };

	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { 0 },
		.module = 0,
		.idx = { { 0 } },
	};
	MPI_ECHN echn = MPI_ENC_CHN(chn_idx.chn);
	osd_bind.idx = echn;

	for (int i = 0; i < OSD_NUM; i++) {
		include_canvas = g_osd_handle[c_idx]->region[i].include_canvas;
		osd_attr.size.width = g_osd_handle[c_idx]->canvas[include_canvas].width;
		osd_attr.size.height = g_osd_handle[c_idx]->canvas[include_canvas].height;
		osd_bind.point.x = g_osd_handle[c_idx]->canvas[include_canvas].startX;
		osd_bind.point.y = g_osd_handle[c_idx]->canvas[include_canvas].startY;
		createOsdInstance(&osd_attr, &osd_bind, &g_osd_chn_handle[c_idx][include_canvas],
		                  &g_osd_canvas_attr[c_idx][include_canvas]);
	}

	if ((region[0].width <= width) && (region[0].height <= height)) {
		include_canvas = g_osd_handle[c_idx]->region[0].include_canvas;
		OSD_setImage(g_osd_handle[c_idx], 0, "/system/mpp/font/LOGO_Augentix_v2.imgayuv",
		             (char *)(g_osd_canvas_attr[c_idx][include_canvas].canvas_addr));

		if (MPI_updateOsdCanvas(g_osd_chn_handle[c_idx][include_canvas]) != MPI_SUCCESS) {
			fprintf(stderr, "failed to update canvas\r\n");
			MPI_destroyOsdRgn(g_osd_chn_handle[c_idx][include_canvas]);
		}
	}

	if ((region[1].width <= width) && (region[1].height <= height)) {
		OsdText txt = {.size = 27,
			       .mode = AYUV_3544,
			       .background = TRANSPARENT,
			       .color = { 0xff, 0xff, 0xff },
			       .outline_color = { 0x00, 0x00, 0x00 },
			       .outline_width = 1,
			       .kerning_mode = AUTO,
			       .kerning = AUTO_KERNING_RATE };

#ifdef INSTALL_TRADITIONAL_OTF
		snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansTC-Normal.otf");
#endif

#ifdef INSTALL_SIMPLIFIED_OTF
		snprintf(&txt.ttf_path[0], 128, "%s", "/system/mpp/font/SourceHanSansCN-Regular_1.otf");
#endif
		char text[128];
		char *ayuv_src;
		int w, h;

		snprintf(&text[0], 128, "Camera 0%d", c_idx + 1);
		sprintf(&txt.txt[0], "%s", &text[0]);

		include_canvas = g_osd_handle[c_idx]->region[1].include_canvas;
		ayuv_src = OSD_createTextUTF8Src(&txt, &w, &h);
		OSD_setImageAYUVptr(g_osd_handle[c_idx], 1, ayuv_src, w, h, AYUV_3544,
		                    (char *)(g_osd_canvas_attr[c_idx][include_canvas].canvas_addr));

		if (MPI_updateOsdCanvas(g_osd_chn_handle[c_idx][include_canvas]) != MPI_SUCCESS) {
			fprintf(stderr, "failed to update canvas\r\n");
			MPI_destroyOsdRgn(g_osd_chn_handle[c_idx][include_canvas]);
		}
	}

	channel_num = output_num;
#else
	MPI_OSD_RGN_ATTR_S osd_attr = { .show = visible,
		                        .qp_enable = false,
		                        .color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544,
		                        .osd_type = MPI_OSD_OVERLAY_BITMAP };
	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { 0 },
		.module = 0,
		.idx = { { 0 } },
	};
	MPI_ECHN e_chn = MPI_ENC_CHN(chn_idx.chn);
	INT32 ret = 0;
	const int c_idx = MPI_GET_VIDEO_CHN(chn_idx);
	void *real_addr = NULL;
	int width_acc = 0;
	int j;

	channel_numbers = output_num;

	/* Create Augentix logo and update canvas */
	if ((LOGO_OSD_SIZE_WIDTH < width) && (LOGO_OSD_SIZE_HEIGHT < height)) {
		osd_attr.size.width = LOGO_OSD_SIZE_WIDTH;
		osd_attr.size.height = LOGO_OSD_SIZE_HEIGHT;
		osd_bind.point.x = 16;
		osd_bind.point.y = 16;
		osd_bind.idx = e_chn;

		ret = createOsdInstance(&osd_attr, &osd_bind, &g_handle[c_idx][0], &p_canvas_attr_logo[c_idx]);
		if (ret) {
			printf("Logo OSD created failure. err: %d\n", ret);
		}

		for (j = 0; (UINT32)j < pic_image_height; j++) {
			memcpy((void *)(p_canvas_attr_logo[c_idx].canvas_addr + (j * LOGO_OSD_SIZE_WIDTH * 2)),
			       (void *)(p_logo_ayuv + (j * pic_image_width * 2)), pic_image_width * 2);
		}

		ret = MPI_updateOsdCanvas(g_handle[c_idx][0]);
		if (ret != MPI_SUCCESS) {
			printf("OSD %d to update canvas failed. err: %d\n", g_handle[c_idx][0], ret);
			return MPI_FAILURE;
		}
	}

	/* Write camera name on the screen */
	if ((NUMBER_OSD_SIZE_WIDTH < width) && (NUMBER_OSD_SIZE_HEIGHT < height)) {
		char camera_name[10];
		unsigned char input_ascii = 0;
		unsigned int i;

		osd_attr.size.width = NUMBER_OSD_SIZE_WIDTH;
		osd_attr.size.height = NUMBER_OSD_SIZE_HEIGHT;
		osd_bind.point.x = ALIGN_16(width - NUMBER_OSD_SIZE_WIDTH);
		osd_bind.point.y = 16;
		osd_bind.idx = e_chn;

		ret = createOsdInstance(&osd_attr, &osd_bind, &g_handle[c_idx][1], &p_canvas_attr_number[c_idx]);
		if (ret) {
			printf("Channel number OSD created failure. err: %d\n", ret);
		}

		sprintf(camera_name, "Camera %2d", c_idx + 1);

		for (i = 0; i < strlen(camera_name); i++) {
			input_ascii = camera_name[i];
			input_ascii = input_ascii - 32;
			real_addr = (void *)p_font_ayuv + osdindex[input_ascii].image_offset;

			for (j = 0; (UINT32)j < (osdindex[input_ascii].image_height); j++) {
				memcpy((void *)((p_canvas_attr_number[c_idx].canvas_addr) +
				                (j * NUMBER_OSD_SIZE_WIDTH * 2) + (width_acc * 2)),
				       real_addr, (osdindex[input_ascii].image_width * 2));
				real_addr = (real_addr + (osdindex[input_ascii].image_width * 2));
			}
			width_acc += osdindex[input_ascii].image_width;
		}

		ret = MPI_updateOsdCanvas(g_handle[c_idx][1]);
		if (ret != MPI_SUCCESS) {
			printf("OSD %d to update canvas failed.\n", g_handle[c_idx][1]);
			return MPI_FAILURE;
		}
	}

	/* Update timestamp */
	if ((TIME_OSD_SIZE_WIDTH < width) && (TIME_OSD_SIZE_HEIGHT < height)) {
		osd_attr.size.width = TIME_OSD_SIZE_WIDTH;
		osd_attr.size.height = TIME_OSD_SIZE_HEIGHT;
		osd_bind.point.x = ALIGN_16(width - TIME_OSD_SIZE_WIDTH);
		osd_bind.point.y = ALIGN_16(height - TIME_OSD_SIZE_HEIGHT);
		osd_bind.idx = e_chn;

		ret = createOsdInstance(&osd_attr, &osd_bind, &g_handle[c_idx][2], &p_canvas_attr_time[c_idx]);
		if (ret) {
			printf("Clock OSD created failure. err: %d\n", ret);
		}
	}

#endif

	return MPI_SUCCESS;
}
