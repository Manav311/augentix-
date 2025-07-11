#ifndef CORE_H_
#define CORE_H_

#include <stdio.h>
#include <string.h> // for strlen
#include <stdlib.h>
#include <linux/limits.h>
#include <syslog.h>

#include "mpi_base_types.h"
#include "mpi_dip_alg.h"

#include "agtx_iaa.h"
#include "agtx_iva.h"
#include "agtx_video.h"
#include "agtx_res_option.h"
#include "agtx_dip_shp_win_conf.h"
#include "agtx_dip_nr_win_conf.h"
#include "agtx_video_layout_conf.h"
#include "app_view_api.h"
#include "agtx_gpio_conf.h"

#include "agtx_audio.h"
#include "agtx_cmd.h"
#include "agtx_osd.h"
#include "agtx_color_conf.h"
#include "agtx_types.h"

#include "sample_dip.h"

#define CC_SOCKET_PATH "/tmp/ccUnxSkt"
#define TD_SOCKET_PATH "/tmp/iva_td_skt"
#define MD_SOCKET_PATH "/tmp/iva_md_skt"
#define ONVIF_EVENT_PATH "/tmp/event_onvif"
#define TMP_ACTIVE_DB "/tmp/ini.db"

#define MAX_PUB_POOL MPI_MAX_PUB_POOL
#define MAX_POOL_NAME_LEN MPI_MAX_PUB_POOL
#define MAX_VIDEO_INPUT 1
#define MAX_INPUT_PATH 2
#define MAX_VIDEO_CHNNEL 4
#define MAX_VIDEO_WINDOW 9
#define MAX_VENC_STREAM 4
#define MAX_OSD_STR_LEN 32
#define MAX_MD_RGN_NUM 64
#define MAX_EF_LINE_NUM 16
#define STITCH_SENSOR_NUM 2
#define STITCH_TABLE_NUM 3
#define MAX_ANTI_FLICKER_FREQUENCY_SIZE 2

typedef void *AGTX_CDATA;

#define AGTX_CONF                              \
	AGTX_DEV_CONF_S dev;                   \
	AGTX_LAYOUT_CONF_S layout;             \
	AGTX_STRM_CONF_S strm;                 \
	AGTX_STITCH_CONF_S stitch;             \
	AGTX_LDC_CONF_S ldc;                   \
	AGTX_SURROUND_CONF_S surround;         \
	AGTX_PANORAMA_CONF_S panorama;         \
	AGTX_PANNING_CONF_S panning;           \
	AGTX_OSD_CONF_S osd;                   \
	AGTX_OSD_PM_CONF_S osd_pm;             \
	AGTX_RES_OPTION_S res_option;          \
	AGTX_ANTI_FLICKER_CONF_S anti_flicker; \
	AGTX_IMG_PREF_S img;                   \
	AGTX_ADV_IMG_PREF_S adv_img;           \
	AGTX_AWB_PREF_S awb;                   \
	AGTX_COLOR_CONF_S color;               \
	AGTX_DIP_NR_WIN_CONF_S nr_win;         \
	AGTX_DIP_SHP_WIN_CONF_S shp_win;       \
	AGTX_IVA_AROI_CONF_S aroi;             \
	AGTX_IVA_BM_CONF_S bm;                 \
	AGTX_IVA_DK_CONF_S dk;                 \
	AGTX_IVA_EAIF_CONF_S eaif;             \
	AGTX_IVA_EF_CONF_S ef;                 \
	AGTX_IVA_FLD_CONF_S fld;               \
	AGTX_IAA_LSD_CONF_S lsd;               \
	AGTX_IVA_LD_CONF_S ld;                 \
	AGTX_IVA_MD_CONF_S md;                 \
	AGTX_IVA_OD_CONF_S od;                 \
	AGTX_IVA_PD_CONF_S pd;                 \
	AGTX_IVA_PFM_CONF_S pfm;               \
	AGTX_IVA_RMS_CONF_S rms;               \
	AGTX_IVA_SHD_CONF_S shd;               \
	AGTX_IVA_TD_CONF_S td;                 \
	AGTX_VIDEO_PTZ_CONF_S ptz;             \
	AGTX_VDBG_CONF_S vdbg;                 \
	AGTX_GPIO_CONF_S gpio;                 \
	AGTX_AUDIO_CONF_S audio;

typedef union {
	AGTX_CONF
} AgtxConf;

typedef struct {
	AGTX_CONF
} GlobalConf;



int CORE_init(void);
int CORE_exit(void);

#endif