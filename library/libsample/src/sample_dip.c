#include <asm-generic/errno-base.h>
#define _GNU_SOURCE //For pthread_setname_np
#include "sample_dip.h"

#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ini.h"
#include "mpi_dip_types.h"
#include "mpi_dip_sns.h"
#include "mpi_dip_alg.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_errno.h"

#include "sample_light_src.h"
#include "utlist.h"

typedef enum {
	DIP_STAT_NULL,
	DIP_STAT_SINGLE,
	DIP_STAT_DUAL,
	DIP_STAT_STITCH,
	DIP_STAT_NUM,
} SAMPLE_DIP_STAT_E;

typedef struct {
	INT32 shp_select;
} SAMPLE_SHP_SELECT_S;

typedef struct {
	INT32 enable;
} SAMPLE_STAT_ENABLE_S;

/* Define the config struct type */
typedef struct {
	SAMPLE_DIP_STAT_E mode;
	INT32 hdr_en;

	/* For both single and dual paths.
	 * In the case of dual sensors, there will be 2 independent threads
	 * and 2 copies of this data structure for containing
	 * attributes for each sensor.
	 */
	MPI_AE_ATTR_S AE;
	MPI_ISO_ATTR_S DIP_ISO;
	MPI_AWB_ATTR_S AWB;
	MPI_PTA_ATTR_S PTA;
	MPI_CSM_ATTR_S CSM;
	MPI_SHP_ATTR_S SHP;
	MPI_NR_ATTR_S NR;
	MPI_GAMMA_ATTR_S GAMMA;
	MPI_TE_ATTR_S TE;
	MPI_DIP_ATTR_S DIP;
	MPI_DBC_ATTR_S DBC0;
	MPI_DCC_ATTR_S DCC0;
	MPI_LSC_ATTR_S LSC0;
	MPI_CAL_ATTR_S CAL0;
	MPI_DBC_ATTR_S DBC1;
	MPI_DCC_ATTR_S DCC1;
	MPI_LSC_ATTR_S LSC1;
	MPI_CAL_ATTR_S CAL1;
	MPI_ROI_ATTR_S ROI0;
	MPI_ROI_ATTR_S ROI1;
	MPI_ENH_ATTR_S ENH;
	MPI_FCS_ATTR_S FCS;
	MPI_DHZ_ATTR_S DHZ;
	MPI_DMS_ATTR_S DMS;
	MPI_CORING_ATTR_S CORING;
	MPI_SHP_ATTR_V2_S SHP_V2;
	SAMPLE_SHP_SELECT_S SAMPLE_SHP_SELECT;
	SAMPLE_STAT_ENABLE_S SAMPLE_STAT_CONFIG;
	MPI_STAT_CFG_S STAT;
	MPI_HDR_SYNTH_ATTR_S HDR_SYNTH;

	// Backup settings for easy switch back
	MPI_DCC_ATTR_S dcc0_bak;
	MPI_DCC_ATTR_S dcc1_bak;
	MPI_DIP_ATTR_S dip_bak;
} SAMPLE_DIP_CONF_S;

typedef struct sample_win_scale_info {
	FLOAT scaling_ratio;
} SAMPLE_WIN_SCALE_INFO_S;

typedef struct sample_dip_win_conf {
	SAMPLE_WIN_SCALE_INFO_S SCALE;
	MPI_ENH_ATTR_S ENH;
	MPI_SHP_ATTR_V2_S SHP_V2;
	struct sample_dip_win_conf *prev, *next;
} SAMPLE_DIP_WIN_CONF_S;

typedef struct sample_update_ini_args {
	// Pthread instance of this thread
	pthread_t *tid;
	// Flag to stop the thread
	volatile int *ini_run;
	// MPI index of the target video system
	MPI_PATH path_idx;
	// Filenames of the different ini IQ file
	char sensor_path[PATH_MAX];
	// Filenames of dip_extend ini file
	char dip_extend_filepath[PATH_MAX];
} SAMPLE_UpdateIniArgs;

typedef struct sample_adv_img_pref {
	int updated;
	MPI_PTA_ATTR_S pta;
	MPI_CSM_ATTR_S csm;
	MPI_TE_ATTR_S te;
	MPI_AE_ATTR_S ae;
	MPI_SHP_ATTR_V2_S shp_attr_v2;
	MPI_AWB_ATTR_S awb;
	MPI_DIP_ATTR_S dip;
} SAMPLE_ADV_IMG_PREF;

static SAMPLE_ADV_IMG_PREF g_sample_adv_img_pref[MPI_MAX_INPUT_PATH_NUM] = { { 0 } };

/* Data of update ini threads */
static pthread_t g_ini_tid[MPI_MAX_INPUT_PATH_NUM]; /*record for join*/
static int g_ini_run[MPI_MAX_INPUT_PATH_NUM] = { 0 };

/* Internal function prototype */
static int handler(void *user, const char *section, const char *name, const char *value);
static void dumpConfig(SAMPLE_DIP_CONF_S *cfg);
static int parseIniFile(const char *filename, SAMPLE_DIP_CONF_S *config);
static void *updateIniThread(void *_args);
static void getDevAttr(MPI_DEV idx, SAMPLE_DIP_CONF_S *config);
static void setDipAttr(MPI_PATH idx, SAMPLE_DIP_CONF_S *config);
static void getDipAttr(MPI_PATH idx, SAMPLE_DIP_CONF_S *config);

#define CLAMP_8B(x) ((x) > 255 ? 255 : ((x) < 0 ? 0 : x))
#define MAX_DARK_ENHANCE_LEVEL (128)
#define MAX_DARK_VALUE (100)

/* Process a line of the INI file, storing valid values into config struct */
static int handler(void *user, const char *section, const char *name, const char *value)
{
	SAMPLE_DIP_CONF_S *cfg = (SAMPLE_DIP_CONF_S *)user;
	if (0)
		;
#define CFG(s, n, default)                                                                                             \
	else if (strcmp(section, #s) == 0 && strcmp(name, #n) == 0) cfg->s.n = (__typeof__(cfg->s.n))atof(value);
#include "config.def"
	return 1;
}

static int win_handler(void *user, const char *section, const char *name, const char *value)
{
	SAMPLE_DIP_WIN_CONF_S **win_head = (SAMPLE_DIP_WIN_CONF_S **)user;
	if (strcmp(section, "SCALE") == 0 && strcmp(name, "scaling_ratio") == 0) {
		SAMPLE_DIP_WIN_CONF_S *cfg = calloc(1, sizeof(*cfg));
		if (!cfg) {
			return 1;
		}

		memcpy(cfg, *win_head, sizeof(*cfg));
		cfg->SCALE.scaling_ratio = atof(value);
		DL_APPEND(*win_head, cfg);
	} else {
		assert(*win_head != NULL);
		SAMPLE_DIP_WIN_CONF_S *cfg = (*win_head)->prev;

		if (!cfg)
			;
#define CFG(s, n, default) \
	else if (strcmp(section, #s) == 0 && strcmp(name, #n) == 0) cfg->s.n = (__typeof__(cfg->s.n))atof(value);
#include "config_win.def"
	}

	return 1;
}

/* Print all the variables in the config, one per line */
static void dumpConfig(SAMPLE_DIP_CONF_S *cfg __attribute__((unused)))
{
//#define CFG(s, n, default) printf("%s_%s = %.2lf\n", #s, #n, (float)cfg->s.n);
//#include "config.def"
}

/**
 * @brief dump the IQ configuration to global variable
 * @return the execution result
 * @retval  0 success
 * @retval -1 failure
 */
static int parseIniFile(const char *filename, SAMPLE_DIP_CONF_S *config)
{
	if (filename == NULL || config == NULL) {
		return -1;
	}

	if (ini_parse(filename, handler, config) < 0) {
		fprintf(stderr, "Can't load '%s', using defaults\n", filename);
		return -1;
	} else {
		dumpConfig(config);
	}

	return 0;
}

static int ratio_cmp(SAMPLE_DIP_WIN_CONF_S *a, SAMPLE_DIP_WIN_CONF_S *b)
{
	return (a->SCALE.scaling_ratio > b->SCALE.scaling_ratio) ?
	               1 :
	               ((a->SCALE.scaling_ratio == b->SCALE.scaling_ratio) ? 0 : -1);
}

/*
 * @brief only get recent MPI adv_img_pref attr
 */
static int updateMpiAdvImgPref(MPI_PATH path_idx)
{
	INT32 ret = 0;

	ret = MPI_getPtaAttr(path_idx, &g_sample_adv_img_pref[path_idx.path].pta);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getCsmAttr(path_idx, &g_sample_adv_img_pref[path_idx.path].csm);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getTeAttr(path_idx, &g_sample_adv_img_pref[path_idx.path].te);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getAeAttr(path_idx, &g_sample_adv_img_pref[path_idx.path].ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getShpAttrV2(path_idx, &g_sample_adv_img_pref[path_idx.path].shp_attr_v2);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getAwbAttr(path_idx, &g_sample_adv_img_pref[path_idx.path].awb);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getDipAttr(path_idx, &g_sample_adv_img_pref[path_idx.path].dip);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}
/*
 * @brief implement updateMpiAdvImgPref to macro
 */
#define UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret)                        \
	do {                                                                \
		if (g_sample_adv_img_pref[(path_idx).path].updated == 0) {  \
			ret = updateMpiAdvImgPref(path_idx);                \
			if (ret != MPI_SUCCESS) {                           \
				return ret;                                 \
			}                                                   \
			g_sample_adv_img_pref[(path_idx).path].updated = 1; \
		}                                                           \
	} while (0)

/**
 * @brief dump the win IQ configuration to global variable
 * @return the execution result
 * @retval  0 success
 * @retval -1 failure
 */
int parseWinIniFile(const char *filename, SAMPLE_DIP_WIN_CONF_S **head)
{
	if (filename == NULL) {
		return -1;
	}

	if (ini_parse(filename, win_handler, head) < 0) {
		fprintf(stderr, "Can't load '%s', using defaults\n", filename);
		return -1;
	}

	/* SORT the setting according to the scaling ratio */
	DL_SORT(*head, ratio_cmp);

	return 0;
}

/**
 * @brief thread for updating IQ parameters in .ini
 * @param[in] _args pointer to the function arguments
 */
static void *updateIniThread(void *_args)
{
	SAMPLE_UpdateIniArgs *args = (SAMPLE_UpdateIniArgs *)_args;

	MPI_PATH path_idx = args->path_idx;
	MPI_DEV dev_idx = MPI_VIDEO_DEV(path_idx.dev);
	SAMPLE_DIP_CONF_S config;
	const char *iq_filename = args->sensor_path;
	const char *dip_extend_filename = args->dip_extend_filepath;
	struct stat last_status = { 0 };
	struct stat dip_extend_last_status = { 0 };
	struct stat buf = { 0 };
	struct stat dip_extend_buff = { 0 };
	int ret = 0;


	/** Check if path is stitch and path cnt */
	getDevAttr(dev_idx, &config);

	/** Init last_status value to the IQ file last changed */
	stat(iq_filename, &last_status);
	stat(dip_extend_filename, &dip_extend_last_status);

	while (*args->ini_run) {
		/** Check if the IQ file has been updated */
		ret = stat(iq_filename, &buf);
		if (ret) {
			fprintf(stderr, "failed to get %s stat\n", iq_filename);
		}

		if (buf.st_mtime != last_status.st_mtime) {
			/** dip file not changed */
			getDipAttr(path_idx, &config);
			ret = parseIniFile(iq_filename, &config);
			if (ret == 0) {
				setDipAttr(path_idx, &config);
			}
			last_status = buf;

			/*update ini attr adv_img_pref*/
			ret = updateMpiAdvImgPref(path_idx);
			if (ret != MPI_SUCCESS) {
				fprintf(stderr, "Failed to update new ini adv img pref: %s\n", iq_filename);
			}
			g_sample_adv_img_pref[path_idx.path].updated = 1;
		}
		/* check if dip_extend ini has been updated */
		if (dip_extend_filename) {
			ret = stat(dip_extend_filename, &dip_extend_buff);
			if (dip_extend_buff.st_mtime == dip_extend_last_status.st_mtime) {
				goto sleep;
			}
			if (!ret && MPI_setDipExtendFile(path_idx, dip_extend_filename) == MPI_SUCCESS) {
				fprintf(stdout, "Update dip_extend file %s success\n", dip_extend_filename);
			} else {
				fprintf(stdout, "Failed to get %s dip_extend file stat\n", dip_extend_filename);
			}
		}
		dip_extend_last_status = dip_extend_buff;

	sleep:
		/* Wait for 2 frame*/
		usleep(200000);
	}

	free(args);

	return NULL;
}

/**
 * @brief update the device operating state to global variable
 */
static void getDevAttr(MPI_DEV idx, SAMPLE_DIP_CONF_S *config)
{
	MPI_DEV_ATTR_S dev_attr;
	UINT8 path_cnt;
	MPI_DEV_getDevAttr(idx, &dev_attr);

	path_cnt = (dev_attr.path.bmp == 0x3) ? 2 : 1;

	/* Determine parameters for get/set DIP attribute */
	if (dev_attr.stitch_en == 0 && path_cnt == 1) {
		/** Only valid on (d, p) = (0, 0) */
		config->mode = DIP_STAT_SINGLE;
	} else if (dev_attr.stitch_en == 0 && path_cnt == 2) {
		/** all valid (d, p) */
		config->mode = DIP_STAT_DUAL;
	} else if (dev_attr.stitch_en == 1 && path_cnt == 2) {
		/** DBC, DCC, LSC, CAL, ROI could accept (d, p) = (0~1, 0~1) */
		config->mode = DIP_STAT_STITCH;
	} else {
		config->mode = DIP_STAT_NULL;
		fprintf(stderr, "%s(): Invalid device attribute, stitch_en = %d, path_cnt = %d, impossible case!\n",
		        __func__, dev_attr.stitch_en, path_cnt);
	}

	config->hdr_en = (dev_attr.hdr_mode == 0) ? 0 : 1;
}

/**
 * @brief write the proper DIP attributes to video device
 */
static void setDipAttr(MPI_PATH idx, SAMPLE_DIP_CONF_S *config)
{
	/** TODO: support set attributes by window */

	/* Set attribute */
	MPI_setAeAttr(idx, &config->AE);
	MPI_setIsoAttr(idx, &config->DIP_ISO);
	MPI_setAwbAttr(idx, &config->AWB);
	MPI_setPtaAttr(idx, &config->PTA);
	MPI_setCsmAttr(idx, &config->CSM);
	if (config->SAMPLE_SHP_SELECT.shp_select == 0) {
		MPI_setShpAttr(idx, &config->SHP);
	} else {
		MPI_setShpAttrV2(idx, &config->SHP_V2);
	}
	MPI_setNrAttr(idx, &config->NR);
	MPI_setTeAttr(idx, &config->TE);
	MPI_setGammaAttr(idx, &config->GAMMA);
	MPI_setDipAttr(idx, &config->DIP);
	MPI_setRoiAttr(idx, &config->ROI0);
	MPI_setDbcAttr(idx, &config->DBC0);
	MPI_setDccAttr(idx, &config->DCC0);
	MPI_setLscAttr(idx, &config->LSC0);
	MPI_setCalAttr(idx, &config->CAL0);
	MPI_setEnhAttr(idx, &config->ENH);
	MPI_setFcsAttr(idx, &config->FCS);
	MPI_setDhzAttr(idx, &config->DHZ);
	MPI_setDmsAttr(idx, &config->DMS);
	MPI_setCoringAttr(idx, &config->CORING);
	if (config->SAMPLE_STAT_CONFIG.enable) {
		MPI_setStatisticsConfig(idx, &config->STAT);
	}
	if (config->hdr_en) {
		MPI_setHdrSynthAttr(idx, &config->HDR_SYNTH);
	}
	if (config->mode == DIP_STAT_STITCH) {
		idx.path = idx.path ^ 1;
		MPI_setRoiAttr(idx, &config->ROI1);
		MPI_setDbcAttr(idx, &config->DBC1);
		MPI_setDccAttr(idx, &config->DCC1);
		MPI_setLscAttr(idx, &config->LSC1);
		MPI_setCalAttr(idx, &config->CAL1);
	}
}

/**
 * @brief update the DIP attributes to video device
 */
static void getDipAttr(MPI_PATH idx, SAMPLE_DIP_CONF_S *config)
{
	/* Get attribute */
	MPI_getAeAttr(idx, &config->AE);
	MPI_getIsoAttr(idx, &config->DIP_ISO);
	MPI_getAwbAttr(idx, &config->AWB);
	MPI_getPtaAttr(idx, &config->PTA);
	MPI_getCsmAttr(idx, &config->CSM);
	MPI_getShpAttr(idx, &config->SHP);
	MPI_getShpAttrV2(idx, &config->SHP_V2);
	MPI_getNrAttr(idx, &config->NR);
	MPI_getTeAttr(idx, &config->TE);
	MPI_getGammaAttr(idx, &config->GAMMA);
	MPI_getDipAttr(idx, &config->DIP);
	MPI_getRoiAttr(idx, &config->ROI0);
	MPI_getDbcAttr(idx, &config->DBC0);
	MPI_getDccAttr(idx, &config->DCC0);
	MPI_getLscAttr(idx, &config->LSC0);
	MPI_getCalAttr(idx, &config->CAL0);
	MPI_getEnhAttr(idx, &config->ENH);
	MPI_getFcsAttr(idx, &config->FCS);
	MPI_getDhzAttr(idx, &config->DHZ);
	MPI_getDmsAttr(idx, &config->DMS);
	MPI_getCoringAttr(idx, &config->CORING);
	MPI_getStatisticsConfig(idx, &config->STAT);
	MPI_getHdrSynthAttr(idx, &config->HDR_SYNTH);

	if (config->mode == DIP_STAT_STITCH) {
		idx.path = idx.path ^ 1;
		MPI_getRoiAttr(idx, &config->ROI1);
		MPI_getDbcAttr(idx, &config->DBC1);
		MPI_getDccAttr(idx, &config->DCC1);
		MPI_getLscAttr(idx, &config->LSC1);
		MPI_getCalAttr(idx, &config->CAL1);
	}
}

/**
 * @brief update the DIP attributes to video device
 */
static void getDipWinAttrFromPath(MPI_PATH idx, SAMPLE_DIP_WIN_CONF_S *config)
{
	/* Get attribute */
	MPI_getShpAttrV2(idx, &config->SHP_V2);
	MPI_getEnhAttr(idx, &config->ENH);
}

/*
 * @brief apply window setting
 */
static void applyDipWinAttr(MPI_WIN win_idx, SAMPLE_DIP_WIN_CONF_S *cfg)
{
	MPI_setWinEnhAttr(win_idx, &cfg->ENH);
	MPI_setWinShpAttrV2(win_idx, &cfg->SHP_V2);
}

/**
 * @brief Set pta brightness.
 * @details set PTA brightness by ratio.
 * @param[in] path_idx video input path to switch
 * @param[in] ratio 0-100 ratio
 */
int SAMPLE_setBrightnessRatio(MPI_PATH path_idx, INT32 ratio)
{
	INT32 tmp = 0;
	INT32 ret = 0;

	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	/**> copy global sample_adv_img_pref value to local */
	MPI_PTA_ATTR_S pta;
	ret = MPI_getPtaAttr(path_idx, &pta);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	tmp = CLAMP_8B((((INT32)g_sample_adv_img_pref[path_idx.path].pta.brightness * ratio) + (1 << 7)) >> 8);
	pta.brightness = (UINT8)tmp;

	ret = MPI_setPtaAttr(path_idx, &pta);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}

/**
 * @brief Set pta contrast.
 * @details set PTA contrast by ratio.
 * @param[in] path_idx video input path to switch
 * @param[in] ratio 0-100 ratio
 */
int SAMPLE_setContrastRatio(MPI_PATH path_idx, INT32 ratio)
{
	INT32 tmp = 0;
	INT32 ret = 0;

	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_PTA_ATTR_S pta;
	ret = MPI_getPtaAttr(path_idx, &pta);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	tmp = CLAMP_8B((((INT32)g_sample_adv_img_pref[path_idx.path].pta.contrast * ratio) + (1 << 7)) >> 8);
	pta.contrast = (UINT8)tmp;

	ret = MPI_setPtaAttr(path_idx, &pta);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}

/**
 * @brief Set csm saturation.
 * @details set csm saturation by 0-100 ratio.
 * @param[in] path_idx video input path to switch
 * @param[in] ratio 0-100 ratio
 */
int SAMPLE_setSaturationRatio(MPI_PATH path_idx, INT32 ratio)
{
	INT32 tmp = 0;
	INT32 ret = 0;

	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_CSM_ATTR_S csm;
	ret = MPI_getCsmAttr(path_idx, &csm);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	for (INT32 i = 0; i < MPI_ISO_LUT_ENTRY_NUM; i++) {
		tmp = CLAMP_8B(
		        (((INT32)g_sample_adv_img_pref[path_idx.path].csm.csm_auto.saturation[i] * ratio) + (1 << 7)) >>
		        8);
		csm.csm_auto.saturation[i] = (UINT8)tmp;
	}

	ret = MPI_setCsmAttr(path_idx, &csm);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}

/**
 * @brief Set csm hue.
 * @details set csm hue angle by 0-100.
 * @param[in] path_idx video input path to switch
 * @param[in] hue 0~100, hue_angle -x ~ x
 */
int SAMPLE_setHueRatio(MPI_PATH path_idx, INT32 hue)
{
	INT32 ret = 0;

	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_CSM_ATTR_S csm;
	ret = MPI_getCsmAttr(path_idx, &csm);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	INT32 each_step_degree = 20 * 4; /**> degree_accuracy = 4 */
	/**> hue is 0~100, hue_angle -x ~ x,*/
	INT32 rotate_angle = ((INT32)hue - 50) * each_step_degree / 50;
	csm.hue_angle = (INT32)g_sample_adv_img_pref[path_idx.path].csm.hue_angle + rotate_angle;

	ret = MPI_setCsmAttr(path_idx, &csm);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}

/**
 * @brief Set sharpness.
 * @details set shpv2 by 0-100 strength.
 * @param[in] path_idx video input path to switch
 * @param[in] sharpness 0-100 ratio 
 */
int SAMPLE_setSharpnessRatio(MPI_PATH path_idx, INT32 sharpness)
{
	INT32 ret = 0;

	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_SHP_ATTR_V2_S shp_attr_v2;
	ret = MPI_getShpAttrV2(path_idx, &shp_attr_v2);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	UINT16 strength = (g_sample_adv_img_pref[path_idx.path].shp_attr_v2.strength * 2 * (sharpness)) / 100;
	shp_attr_v2.strength = strength;

	ret = MPI_setShpAttrV2(path_idx, &shp_attr_v2);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}

/**
 * @brief Set antiflicker.
 * @details set antiflicker enable/frequency/frame_rate.
 * @param[in] path_idx video input path to switch
 * @param[in] enable antiflicker on/off
 * @param[in] frequency antiflicker frequency attr
 * @param[in] frame_rate antflocker frame rate 
 */
int SAMPLE_setAntiflicker(MPI_PATH path_idx, INT32 enable, INT32 frequency, INT32 frame_rate)
{
	INT32 ret = 0;
	MPI_AE_ATTR_S ae;
	ret = MPI_getAeAttr(path_idx, &ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ae.anti_flicker.enable = enable;
	ae.anti_flicker.frequency = frequency;
	ae.frame_rate = frame_rate;

	ret = MPI_setAeAttr(path_idx, &ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return 0;
}

static INT32 getDarkEnhance(const INT32 dft, const INT32 value)
{
	if (dft < 0 || dft > MAX_DARK_ENHANCE_LEVEL) {
		fprintf(stderr, "Invaild dark default range.\n");
		return MPI_FAILURE;
	}

	if (value < 0 || value > MAX_DARK_VALUE) {
		fprintf(stderr, "Invaild dark value range.\n");
		return MPI_FAILURE;
	}

	INT32 result = ((value * MAX_DARK_ENHANCE_LEVEL + (100 - value) * dft) + 50) / 100;

	if (result < 0 || result > UINT8_MAX) {
		fprintf(stderr, "Impossible result.\n");
		return MPI_FAILURE;
	}

	return result;
}

/**
 * @brief Set WDR.
 * @details set WDR enable and strength.
 * @param[in] path_idx video input path to switch
 * @param[in] wdr_en WDR on/off
 * @param[in] wdr_strength WDR strength
 */
int SAMPLE_setWdr(MPI_PATH path_idx, INT32 wdr_en, INT32 wdr_strength)
{
	INT32 ret = 0;

	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_TE_ATTR_S te;
	MPI_AE_ATTR_S ae;
	MPI_DIP_ATTR_S dip;
	ret = MPI_getDipAttr(path_idx, &dip);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getAeAttr(path_idx, &ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_getTeAttr(path_idx, &te);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	if (wdr_en == 1) {
		/* DIP */
		dip.is_ae_en = 1;
		dip.is_te_en = 1;
		/* AE */
		ae.strategy.mode = AE_EXP_HIGHLIGHT_PRIOR;
		ae.isp_gain_range.min = 32;
		ae.isp_gain_range.max = 32;
		/* TE */
		te.mode = TE_WDR;
		te.te_wdr.dark_enhance =
		        getDarkEnhance(g_sample_adv_img_pref[path_idx.path].te.te_wdr.dark_enhance, wdr_strength);
	} else {
		/**> set to default value */
		memcpy(&te, &g_sample_adv_img_pref[path_idx.path].te, sizeof(MPI_TE_ATTR_S));
		memcpy(&ae, &g_sample_adv_img_pref[path_idx.path].ae, sizeof(MPI_AE_ATTR_S));
		memcpy(&dip, &g_sample_adv_img_pref[path_idx.path].dip, sizeof(MPI_DIP_ATTR_S));
	}

	ret = MPI_setDipAttr(path_idx, &dip);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_setAeAttr(path_idx, &ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_setTeAttr(path_idx, &te);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return ret;
}

/**
 * @brief Set BLC.
 * @details set backlight compensation.
 * @param[in] path_idx video input path to switch
 * @param[in] backlight_compensation 
 */
int SAMPLE_setBlc(MPI_PATH path_idx, INT32 backlight_compensation)
{
	INT32 ret = 0;
	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_AE_ATTR_S ae;
	ret = MPI_getAeAttr(path_idx, &ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	if (backlight_compensation == 1) {
		ae.roi.luma_weight = 9;
		ae.roi.awb_weight = 2;
	} else {
		/**> set to default value */
		memcpy(&ae, &g_sample_adv_img_pref[path_idx.path].ae, sizeof(MPI_AE_ATTR_S));
	}

	ret = MPI_setAeAttr(path_idx, &ae);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return ret;
}

/**
 * @brief Set AWB pref.
 * @details set AWB mode and reb and blue gain and extra gain.
 * @param[in] path_idx video input path to switch
 * @param[in] awb_mode 0 for default value, 1 to config high_k and low k as color_temp
 * @param[in] r_gain_ratio 
 * @param[in] b_gain_ratio 
 */
int SAMPLE_setAwbPref(MPI_PATH path_idx, INT32 awb_mode, INT32 color_temp, INT32 r_gain_ratio, INT32 b_gain_ratio)
{
	INT32 ret = 0;
	UPDATE_AND_CHECK_ADV_IMG_PREF(path_idx, ret);

	MPI_AWB_ATTR_S awb_output;
	/**> set MPI_AWB_ATTR_S to default value */
	memcpy(&awb_output, &g_sample_adv_img_pref[path_idx.path].awb, sizeof(awb_output));

	if (awb_mode == 0) {
		awb_output.high_k = color_temp;
		awb_output.low_k = color_temp;
	}

	awb_output.r_extra_gain = CLAMP_8B(
	        (((INT32)g_sample_adv_img_pref[path_idx.path].awb.r_extra_gain * r_gain_ratio) + (1 << 7)) >> 8);
	awb_output.b_extra_gain = CLAMP_8B(
	        (((INT32)g_sample_adv_img_pref[path_idx.path].awb.b_extra_gain * b_gain_ratio) + (1 << 7)) >> 8);

	ret = MPI_setAwbAttr(path_idx, &awb_output);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	return ret;
}

/**
 * @brief Switch from black/white mode to other.
 * @details Black/white mode has no AWB parameters but others have.This function
 * aims to demo how to set IQ parameters to avoid bitrate increase suddenly.
 * @param[in] path_idx video input path to switch
 * @param[in] src old mode light src ptr
 * @param[in] dst new mode light src ptr
 */
static void switchMethodIr2Day(MPI_PATH path_idx, LightSrc *src,
		LightSrc *dst) {
	SAMPLE_DIP_CONF_S config;
	int ret = 0;

	/** Initialize config struct */
	getDevAttr(MPI_VIDEO_DEV(path_idx.dev), &config);
	getDipAttr(path_idx, &config);

	ret = parseIniFile(dst->sensor_path, &config);
	if (ret) {
		/** Use a simple method to switch settings instead */
		config.DIP = config.dip_bak;
		config.DCC0 = config.dcc0_bak;
		config.DCC1 = config.dcc1_bak;
		config.CSM.bw_en = 0;
	}

	/** Disable SHP */
	MPI_DIP_ATTR_S dip_attr_tmp;
	MPI_getDipAttr(path_idx, &dip_attr_tmp);
	dip_attr_tmp.is_shp_en = 0;
	MPI_setDipAttr(path_idx, &dip_attr_tmp);

	/** Switch 2 mode light src */
	if (src->off != NULL) {
		src->off(src->private);
	} else {
		fprintf(stderr, "src off handler is null\n");
	}

	if (dst->on != NULL) {
		dst->on(dst->private);
	} else {
		fprintf(stderr, "dst on handler is null\n");
	}

	/** Load CAL, DBC, DCC, LSC, ROI.STAT for AWB */
	MPI_setDbcAttr(path_idx, &config.DBC0);
	MPI_setDccAttr(path_idx, &config.DCC0);
	MPI_setLscAttr(path_idx, &config.LSC0);
	MPI_setCalAttr(path_idx, &config.CAL0);
	MPI_setRoiAttr(path_idx, &config.ROI0);
	if (config.SAMPLE_STAT_CONFIG.enable) {
		MPI_setStatisticsConfig(path_idx, &config.STAT);
	}
	if (config.mode == DIP_STAT_STITCH) {
		MPI_setDbcAttr(path_idx, &config.DBC1);
		MPI_setDccAttr(path_idx, &config.DCC1);
		MPI_setLscAttr(path_idx, &config.LSC1);
		MPI_setCalAttr(path_idx, &config.CAL1);
		MPI_setRoiAttr(path_idx, &config.ROI1);
	}

	/** Wait for a frame */
	usleep(100000u);

	/** Load all except SHP & BW to prevent large bitrate */
	SAMPLE_DIP_CONF_S tmp_config = config;
	tmp_config.DIP.is_shp_en = 0;
	tmp_config.CSM.bw_en = 1;
	setDipAttr(path_idx, &tmp_config);

	/** Wait for a frame */
	usleep(100000u);

	/** Load SHP & BW */
	MPI_setCsmAttr(path_idx, &config.CSM);
	MPI_setDipAttr(path_idx, &config.DIP);
	MPI_setDipExtendFile(path_idx, dst->dip_extend_path);
}

/**
 * @brief Switch from color mode to other.
 * @param[in] path_idx video input path to switch
 * @param[in] src old mode light src ptr
 * @param[in] dst new mode light src ptr
 */
static void switchMethodDay2Any(MPI_PATH path_idx, LightSrc *src,
		LightSrc *dst) {
	SAMPLE_DIP_CONF_S config;
	int ret = 0;

	/** Initialize config struct */
	getDevAttr(MPI_VIDEO_DEV(path_idx.dev), &config);
	getDipAttr(path_idx, &config);

	/** backup some settings from Day mode for a easy restore */
	config.dcc0_bak = config.DCC0;
	config.dcc1_bak = config.DCC1;
	config.dip_bak = config.DIP;

	ret = parseIniFile(dst->sensor_path, &config);
	if (ret == 0) {
		/** Use a simple method to switch settings instead */
		config.DIP.is_awb_en = 0;
		config.DIP.is_csm_en = 1;
		config.DIP.is_fcs_en = 0;

		for (int i = 0; i < MPI_DCC_CHN_NUM; i++) {
			config.DCC0.gain[i] = config.DCC0.gain[0];
			config.DCC0.offset_2s[i] = config.DCC0.offset_2s[0];
			config.DCC1.gain[i] = config.DCC1.gain[0];
			config.DCC1.offset_2s[i] = config.DCC1.offset_2s[0];
		}
		config.CSM.bw_en = 1;

	} else {
		fprintf(stderr, "failed to parse ini: %s\n", dst->sensor_path);
		return;
	};

	/** Pass Dst IQ attributes to MPP system. */
	setDipAttr(path_idx, &config);
	MPI_setDipExtendFile(path_idx, dst->dip_extend_path);

	/** Wait for a frame */
	usleep(100000u);

	fprintf(stderr, "%s after parse dip, set light src\n", __func__);
	/** Switch 2 mode light src */
	if (src->off != NULL) {
		src->off(src->private);
	} else {
		fprintf(stderr, "src off handler is null\n");
	}

	if (dst->on != NULL) {
		dst->on(dst->private);
	} else {
		fprintf(stderr, "dst on handler is null\n");
	}
}

/**
 * @brief update IQ parameters once by specified .ini
 * @param[in] path_idx input path index
 * @param[in] filename name of the .ini file
 */
int SAMPLE_updateDipAttrOnce(MPI_PATH path_idx, const char *filename, const char *dip_extend_filepath)
{
	MPI_DEV dev_idx = MPI_VIDEO_DEV(path_idx.dev);
	SAMPLE_DIP_CONF_S config;

	/** Check filename exist, path idx is valid */
	if (access(filename, R_OK) != 0) {
		fprintf(stderr, "Failed to read: %s, errno: %d\n", filename, errno);
		return -EINVAL;
	}

	if (path_idx.path >= MPI_MAX_INPUT_PATH_NUM) {
		fprintf(stderr, "Invalid path idx: %d\n", path_idx.path);
		return -EINVAL;
	}

	/** Initialize config struct */
	getDevAttr(dev_idx, &config);
	getDipAttr(path_idx, &config);

	if (!parseIniFile(filename, &config)) {
		setDipAttr(path_idx, &config);
		printf("updated by ini %s.\n", filename);
	}

	if (dip_extend_filepath && access(dip_extend_filepath, R_OK) == 0) {
		if (MPI_setDipExtendFile(path_idx, dip_extend_filepath) != MPI_SUCCESS) {
			fprintf(stdout, "MPI_setDipExtendFile failed\n");
		} else {
			fprintf(stdout, "\n[SAMPLE_updateDipAttrOnce]sensor ini : %s ; dip_extend ini : %s\n", filename, dip_extend_filepath);
		}
	}

	if (updateMpiAdvImgPref(path_idx) != MPI_SUCCESS) {
		fprintf(stderr, "Failed to update ini adv img pref: %s\n", filename);
		return -EINVAL;
	}
	g_sample_adv_img_pref[path_idx.path].updated = 1;

	return 0;
}

/**
 * @brief create a thread to track the new IQ parameters if the .ini is updated
 * @param[in] path_idx video input path index
 * @param[in] sensor_path the IQ parameters to tracked by thread
 * @see SAMPLE_destroyDipAttrUpdateThread()
 * @see updateIniThread()
 */
int SAMPLE_createDipAttrUpdateThread(MPI_PATH path_idx, const char *sensor_path, const char *dip_extend_filepath)
{
	SAMPLE_UpdateIniArgs *tmp_arg = malloc(sizeof(SAMPLE_UpdateIniArgs));
	const int thd_idx = path_idx.path;
	char tid_name[32];

	/** Argument check */
	if (path_idx.path >= MPI_MAX_INPUT_PATH_NUM) {
		fprintf(stderr, "Failed to access: %d path\n", path_idx.path);
		free(tmp_arg);
		return -EINVAL;
	}

	if (thd_idx >= MPI_MAX_INPUT_PATH_NUM) {
		fprintf(stderr, "Invalid path index %d.\n", thd_idx);
		free(tmp_arg);
		return -EINVAL;
	}
	if (sensor_path == NULL || access(sensor_path, R_OK) != 0) {
		fprintf(stderr, "Invalid attributes.\n");
		free(tmp_arg);
		return -EACCES;
	}

	/** Generate thread name and switch mode filename */
	sprintf(tid_name, "ini_update_%d", thd_idx);

	/** convert func arg to thread arg */

	tmp_arg->tid = &g_ini_tid[thd_idx];
	tmp_arg->ini_run = &g_ini_run[thd_idx];
	tmp_arg->path_idx = path_idx;

	strncpy(&tmp_arg->sensor_path[0], sensor_path, sizeof(tmp_arg->sensor_path));

	if (dip_extend_filepath && access(dip_extend_filepath, R_OK) == 0) {
		strncpy(&tmp_arg->dip_extend_filepath[0], dip_extend_filepath, sizeof(tmp_arg->dip_extend_filepath));
	}

	/** create thread for updating ini */
	g_ini_run[thd_idx] = 1;
	if (pthread_create(&g_ini_tid[thd_idx], NULL, updateIniThread, (void *)tmp_arg) != 0) {
		fprintf(stderr, "Create thread updateIniThread %d failed.\n", thd_idx);
		g_ini_run[thd_idx] = 0;
		return -EACCES;
	}
	if (pthread_setname_np(g_ini_tid[thd_idx], tid_name) != 0) {
		fprintf(stderr, "Set thread name to updateIniThread %d failed.\n", thd_idx);
	}

	printf("Create thread updateIniThread %d\n", thd_idx);

	return 0;
}

/**
 * @brief stop the thread of tracking the new IQ parameters
 * @param[in] path_idx  video input path index of updateIniThread()
 * @see SAMPLE_createDipAttrUpdateThread()
 * @see updateIniThread()
 */
int SAMPLE_destroyDipAttrUpdateThread(MPI_PATH path_idx) {
	int thd_idx = path_idx.path;

	if (thd_idx >= MPI_MAX_INPUT_PATH_NUM) {
		fprintf(stderr, "Invalid path index %d.\n", thd_idx);
		return -EINVAL;
	}

	if (g_ini_run[thd_idx]) {
		g_ini_run[thd_idx] = 0;
		if (pthread_join(g_ini_tid[thd_idx], NULL) != 0) {
			fprintf(stderr, "Failed to join thread updateIniThread %d.\n", thd_idx);
			return -EINVAL;
		}
		printf("Have joined thread updateIniThread %d.\n", thd_idx);
	}

	return 0;
}

/**
 * @brief Switch IQ mode from old to new one.
 * @details This function will destroy the thread to track IQ parameter files,
 * reset IQ paramters of new mode, on/off light src fo 2 mode, then create the
 * thread to track IQ parameter files (with new IQ mode) again.
 *  @param[in] path_idx video input path that change IQ mode.
 *  @param[in] src old mode light src ptr.
 *  @param[in] dst new mode light src ptr.
 */
int SAMPLE_switchLightSrc(MPI_PATH path_idx, LightSrc *src, LightSrc *dst) {
	if (src->type == SRC_TYPE_IR) {
		switchMethodIr2Day(path_idx, src, dst);
	} else {
		switchMethodDay2Any(path_idx, src, dst);
	}

	return 0;
}

/**
 * @brief Read and write PCA settings into IQ systems.
 * @param[in] idx      video device index
 * @param[in] filename name of the PCA settings file (Eg. /system/mpp/script/pca_cal_0.lut)
 * @return 0 or failure code.
 * @retval 0 Success.
 * @retval -EINVAL Invalid arguments.
 * @retval -EBADF Specified file does not exist or cannot be opened.
 * @retval -EIO Errors happenned while reading the file or the specified filesize is not large enough.
 * @retval other Operation failed.
 */
int SAMPLE_updatePca(MPI_PATH path_idx, const char *filename)
{
	/** Check attributes */
	if (filename == NULL) {
		return -EINVAL;
	}

	/** Open PCA binary file */
	FILE *fin = fopen(filename, "rb");
	if (fin == NULL) {
		return -EBADF;
	}

	/** Read PCA data from the file */
	MPI_PCA_TABLE_S pca_table;
	unsigned char *const read_ptr = (unsigned char *)&pca_table;
	const size_t total_size = sizeof(MPI_PCA_TABLE_S);
	size_t read_byte = 0;
	while (read_byte < total_size) {
		read_byte += fread(read_ptr + read_byte, sizeof(uint8_t), total_size - read_byte, fin);
		if (ferror(fin) && read_byte < total_size) {
			fclose(fin);
			return -EIO;
		} else if (feof(fin)) {
			break;
		}
	}
	if (read_byte < total_size) {
		fclose(fin);
		return -EIO;
	}

	fclose(fin);

	/**  Set PCA table */
	int ret = MPI_setPcaTable(path_idx, &pca_table);

	return ret;
}

/**
 * @brief Update channel setting according to scaling ratio
 * @details This function will automatically update channel IQ by applying corresponding
 * ENH and SHP_V2 setting recorded in the window.ini file. The variable fields are defined in config_win.def.
 * @param[in] idx      video channel index
 * @param[in] filename name of the window setting file (Eg. /system/mpp/script/window.ini)
 * @return 0 or failure code.
 * @retval 0 Success.
 * @retval -EINVAL Invalid arguments.
 * @retval -EIO Errors happenned while reading the file or the specified filesize is not large enough.
 * @retval -ENOMEM Errors happenned no memory to be allocated.
 * @retval -EFAULT Errors happenned when encounter function limitation. See @note.
 * @retval other Operation failed.
 * @note Current implmentation does note support PIP or POP of a video channel, nether non-NORMAL view_type 
 */
int SAMPLE_updateChnDipAttr(MPI_CHN chn_idx, const char *filename)
{
	SAMPLE_DIP_WIN_CONF_S *head = NULL, *tmp, *elt;
	/* check if it is NULL pointer */
	if (filename == NULL || access(filename, R_OK) != 0) {
		return -EINVAL;
	}

	/* update corresponding setting */
	int ret = 0;
	MPI_WIN_ATTR_S win_attr;
	MPI_CHN_LAYOUT_S layout;

	if ((ret = MPI_DEV_getChnLayout(chn_idx, &layout)) != MPI_SUCCESS) {
		fprintf(stderr, "Unable to get channel layout !\n");
		return ret;
	}

	for (int i = 0; i < layout.window_num; ++i) {
		/* only support 1 window in the channel */
		MPI_WIN win_idx = layout.win_id[i];
		if ((ret = MPI_DEV_getWindowAttr(win_idx, &win_attr)) != MPI_SUCCESS) {
			/* FIXME: A Workaround for the bug MPI_DEV_getWindowAttr always failed before
			 * MPI_DEV_startAllChn, even if successfully get the attrubute */
		}

		/* Avoid fisheye lens application */
		if (win_attr.view_type >= MPI_WIN_VIEW_TYPE_LDC) {
			printf("[Warning] View type is not NORMAL, skip window(%d, %d) iq setting.\n", win_idx.chn,
			       win_idx.win);
			return -EFAULT;
		}

		MPI_PATH path_idx = MPI_INPUT_PATH(0, 0);
		for (unsigned bit = 0; bit < 32; bit++) {
			if (win_attr.path.bmp & (1u << bit)) {
				path_idx = MPI_INPUT_PATH(0, bit);
				break;
			}
		}

		/* allocate first element, assign value from path index */
		tmp = calloc(1, sizeof(*tmp));
		if (tmp == NULL) {
			return -ENOMEM;
		}

		/* create setting for no-scaling or up-scaling case */
		tmp->SCALE.scaling_ratio = 32.f;
		getDipWinAttrFromPath(path_idx, tmp);
		DL_APPEND(head, tmp);

		/* parse iq setting from file */
		if (parseWinIniFile(filename, &head) == 0) {
			/* calculate scaling ratio */
			MPI_PATH_ATTR_S path_attr;
			if ((ret = MPI_DEV_getPathAttr(path_idx, &path_attr)) != MPI_SUCCESS) {
				fprintf(stderr, "Unable to get path attribute !\n");
				return ret;
			}

			MPI_SIZE_S in_res = path_attr.res;
			MPI_RECT_S out_res = layout.window[i];

			/* channel resolution * window area / input resolution */
			float scaling_ratio =
			        (float)(out_res.width * out_res.height) / (float)(in_res.width * in_res.height);

			/* search for the setting */
			DL_FOREACH(head, elt)
			{
				if (elt->SCALE.scaling_ratio >= scaling_ratio) {
					if (elt->SCALE.scaling_ratio != 32.f) { // if does not apply path setting */
						printf("Apply scaling_ratio = %.3f setting to window(%d, %d)\n",
						       elt->SCALE.scaling_ratio, win_idx.chn, win_idx.win);
					}
					applyDipWinAttr(win_idx, elt);
					break;
				}
			}
		} else {
			fprintf(stderr, "faile to parse %s\n", filename);
		}

		/* delete all element */
		DL_FOREACH_SAFE(head, elt, tmp)
		{
			DL_DELETE(head, elt);
			free(elt);
		}

		/* reset head */
		head = NULL;
	}

	return 0;
}
