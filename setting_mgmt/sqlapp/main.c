
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <json.h>
#include "sqllib.h"
#include "sql_script.h"
#include "cc_common.h"
#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_cmd.h"
#include "cc_utils.h"
#include "cc_data.h"
#include "cm_product_option.h"
#include "cm_venc_option.h"

#define CREATE_DB_WITH_HEADER_SUPPORT 0

#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))

char *get_file_content(const char *path);
int get_id(const char *str);

typedef struct {
	int cmd_id;
	char *json_content;
} JSON_CONTENT_S;

typedef struct {
	AGTX_SYS_INFO_S       sys_info;
	AGTX_SYS_FEATURE_OPTION_S sys_feature_option;
	AGTX_SYS_DB_INFO_S    sys_db_info;
	AGTX_DEV_CONF_S       dev_conf;
	AGTX_VIEW_TYPE_INFO_S view_type_info;
	AGTX_STRM_CONF_S      strm_conf;
	AGTX_LAYOUT_CONF_S    layout_conf;
	AGTX_STITCH_CONF_S    stitch_conf;
	AGTX_AWB_PREF_S       awb_pref;
	AGTX_IMG_PREF_S       img_pref;
	AGTX_ADV_IMG_PREF_S   adv_img_pref;
	AGTX_DIP_CAL_CONF_S   dip_cal;
	AGTX_DIP_DBC_CONF_S   dip_dbc;
	AGTX_DIP_DCC_CONF_S   dip_dcc;
	AGTX_DIP_LSC_CONF_S   dip_lsc;
	AGTX_DIP_CTRL_CONF_S  dip_ctrl;
	AGTX_DIP_AE_CONF_S    dip_ae;
	AGTX_DIP_AWB_CONF_S   dip_awb;
	AGTX_DIP_PTA_CONF_S   dip_pta;
	AGTX_DIP_PCA_CONF_S   dip_pca;
	AGTX_DIP_CSM_CONF_S   dip_csm;
	AGTX_DIP_SHP_CONF_S   dip_shp;
	AGTX_DIP_NR_CONF_S    dip_nr;
	AGTX_DIP_ROI_CONF_S   dip_roi;
	AGTX_DIP_TE_CONF_S    dip_te;
	AGTX_DIP_GAMMA_CONF_S dip_gamma;
	AGTX_DIP_ISO_CONF_S   dip_iso;
	AGTX_COLOR_CONF_S     color_conf;
	AGTX_AUDIO_CONF_S     audio_conf;
	AGTX_VOICE_CONF_S     voice_conf;
	AGTX_SIREN_CONF_S     siren_conf;
	_AGTX_PRODUCT_OPTION_S product_option;
	AGTX_PRODUCT_OPTION_LIST_S product_option_list;
	AGTX_RES_OPTION_S     res_option;
	AGTX_VENC_OPTION_S    venc_option;
	AGTX_EVENT_CONF_S     evt_conf;
	AGTX_EVENT_PARAM_S    evt_param;
	AGTX_GPIO_CONF_S      gpio_conf;
	AGTX_LOCAL_RECORD_CONF_S local_record_conf;
	AGTX_OSD_CONF_S       osd_conf;
	AGTX_OSD_PM_CONF_S    osd_pm_conf;
	AGTX_IVA_TD_CONF_S    td_conf;
	AGTX_IVA_MD_CONF_S    md_conf;
	AGTX_IVA_AROI_CONF_S  aroi_conf;
	AGTX_IVA_PD_CONF_S    pd_conf;
	AGTX_IVA_OD_CONF_S    od_conf;
	AGTX_IVA_RMS_CONF_S   rms_conf;
	AGTX_IVA_LD_CONF_S    ld_conf;
	AGTX_IVA_EF_CONF_S    ef_conf;
	AGTX_VDBG_CONF_S      vdbg_conf;
	AGTX_VIDEO_PTZ_CONF_S ptz_conf;
	AGTX_IVA_SHD_CONF_S   shd_conf;
	AGTX_IVA_EAIF_CONF_S  eaif_conf;
	AGTX_IVA_PFM_CONF_S   pfm_conf;
	AGTX_IVA_BM_CONF_S   bm_conf;
	AGTX_IVA_DK_CONF_S dk_conf;
	AGTX_IVA_FLD_CONF_S fld_conf;
	AGTX_IAA_LSD_CONF_S   lsd_conf;
	AGTX_LDC_CONF_S       ldc_conf;
	AGTX_PANORAMA_CONF_S  panorama_conf;
	AGTX_PANNING_CONF_S   panning_conf;
	AGTX_SURROUND_CONF_S  surround_conf;
	AGTX_ANTI_FLICKER_CONF_S anti_flicker_conf;
	AGTX_PWM_CONF_S       pwm_conf;
	AGTX_PIR_CONF_S       pir_conf;
	AGTX_FLOODLIGHT_CONF_S floodlight_conf;
	AGTX_DIP_SHP_WIN_CONF_S   dip_shp_win;
	AGTX_DIP_NR_WIN_CONF_S    dip_nr_win;
	AGTX_PRIVATE_MODE_CONF_S  private_mode;
	AGTX_LIGHT_SENSOR_CONF_S light_sensor_conf;
	AGTX_DIP_ENH_CONF_S dip_enh;
	AGTX_DIP_CORING_CONF_S dip_coring;
	AGTX_DIP_STAT_CONF_S dip_stat;
	AGTX_DIP_FCS_CONF_S dip_fcs;
	AGTX_DIP_DHZ_CONF_S dip_dhz;
	AGTX_DIP_HDR_SYNTH_CONF_S dip_hdr_synth;
} DB_CONF_S;


void *get_conf_ptr(DB_CONF_S *conf, int cmd_id)
{
	void *ptr = NULL;

	if (cmd_id == AGTX_CMD_SYS_INFO) {
		ptr = &conf->sys_info;
	} else if (cmd_id == AGTX_CMD_SYS_FEATURE_OPTION) {
		ptr = &conf->sys_feature_option;
	} else if (cmd_id == AGTX_CMD_PRODUCT_OPTION_LIST) {
		ptr = &conf->product_option_list;
	} else if (cmd_id == AGTX_CMD_SYS_DB_INFO) {
		ptr = &conf->sys_db_info;
	} else if (cmd_id == AGTX_CMD_VIDEO_DEV_CONF) {
		ptr = &conf->dev_conf;
	} else if (cmd_id == AGTX_CMD_VIDEO_VIEW_TYPE) {
		ptr = &conf->view_type_info;
	} else if (cmd_id == AGTX_CMD_VIDEO_STRM_CONF) {
		ptr = &conf->strm_conf;
	} else if (cmd_id == AGTX_CMD_VIDEO_LAYOUT_CONF) {
		ptr = &conf->layout_conf;
	} else if (cmd_id == AGTX_CMD_STITCH_CONF) {
		ptr = &conf->stitch_conf;
	} else if (cmd_id == AGTX_CMD_AWB_PREF) {
		ptr = &conf->awb_pref;
	} else if (cmd_id == AGTX_CMD_IMG_PREF) {
		ptr = &conf->img_pref;
	} else if (cmd_id == AGTX_CMD_ADV_IMG_PREF) {
		ptr = &conf->adv_img_pref;
	} else if (cmd_id == AGTX_CMD_DIP_CAL) {
		ptr = &conf->dip_cal;
	} else if (cmd_id == AGTX_CMD_DIP_DBC) {
		ptr = &conf->dip_dbc;
	} else if (cmd_id == AGTX_CMD_DIP_DCC) {
		ptr = &conf->dip_dcc;
	} else if (cmd_id == AGTX_CMD_DIP_LSC) {
		ptr = &conf->dip_lsc;
	} else if (cmd_id == AGTX_CMD_DIP_CTRL) {
		ptr = &conf->dip_ctrl;
	} else if (cmd_id == AGTX_CMD_DIP_AE) {
		ptr = &conf->dip_ae;
	} else if (cmd_id == AGTX_CMD_DIP_ISO) {
		ptr = &conf->dip_iso;
	} else if (cmd_id == AGTX_CMD_DIP_AWB) {
		ptr = &conf->dip_awb;
	} else if (cmd_id == AGTX_CMD_DIP_PTA) {
		ptr = &conf->dip_pta;
	} else if (cmd_id == AGTX_CMD_DIP_CSM) {
		ptr = &conf->dip_csm;
	} else if (cmd_id == AGTX_CMD_DIP_SHP) {
		ptr = &conf->dip_shp;
	} else if (cmd_id == AGTX_CMD_DIP_NR) {
		ptr = &conf->dip_nr;
	} else if (cmd_id == AGTX_CMD_DIP_ROI) {
		ptr = &conf->dip_roi;
	} else if (cmd_id == AGTX_CMD_DIP_TE) {
		ptr = &conf->dip_te;
	} else if (cmd_id == AGTX_CMD_DIP_GAMMA) {
		ptr = &conf->dip_gamma;
	} else if (cmd_id == AGTX_CMD_DIP_ENH) {
		ptr = &conf->dip_enh;
	} else if (cmd_id == AGTX_CMD_DIP_CORING) {
		ptr = &conf->dip_coring;
	} else if (cmd_id == AGTX_CMD_DIP_FCS) {
		ptr = &conf->dip_fcs;
	} else if (cmd_id == AGTX_CMD_DIP_HDR_SYNTH) {
		ptr = &conf->dip_hdr_synth;
	} else if (cmd_id == AGTX_CMD_DIP_DHZ) {
		ptr = &conf->dip_dhz;
	} else if (cmd_id == AGTX_CMD_DIP_STAT) {
		ptr = &conf->dip_stat;
	} else if (cmd_id == AGTX_CMD_COLOR_CONF) {
		ptr = &conf->color_conf;
	} else if (cmd_id == AGTX_CMD_PRODUCT_OPTION) {
		ptr = &conf->product_option;
	} else if (cmd_id == AGTX_CMD_RES_OPTION) {
		ptr = &conf->res_option;
	} else if (cmd_id == AGTX_CMD_VENC_OPTION) {
		ptr = &conf->venc_option;
	} else if (cmd_id == AGTX_CMD_LDC_CONF) {
		ptr = &conf->ldc_conf;
	} else if (cmd_id == AGTX_CMD_PANORAMA_CONF) {
		ptr = &conf->panorama_conf;
	} else if (cmd_id == AGTX_CMD_PANNING_CONF) {
		ptr = &conf->panning_conf;
	} else if (cmd_id == AGTX_CMD_SURROUND_CONF) {
		ptr = &conf->surround_conf;
	} else if (cmd_id == AGTX_CMD_AUDIO_CONF) {
		ptr = &conf->audio_conf;
	} else if (cmd_id == AGTX_CMD_VOICE_CONF) {
		ptr = &conf->voice_conf;
	} else if (cmd_id == AGTX_CMD_SIREN_CONF) {
		ptr = &conf->siren_conf;
	} else if (cmd_id == AGTX_CMD_OSD_CONF) {
		ptr = &conf->osd_conf;
	} else if (cmd_id == AGTX_CMD_OSD_PM_CONF) {
		ptr = &conf->osd_pm_conf;
	} else if (cmd_id == AGTX_CMD_TD_CONF) {
		ptr = &conf->td_conf;
	} else if (cmd_id == AGTX_CMD_MD_CONF) {
		ptr = &conf->md_conf;
	} else if (cmd_id == AGTX_CMD_AROI_CONF) {
		ptr = &conf->aroi_conf;
	} else if (cmd_id == AGTX_CMD_EF_CONF) {
		ptr = &conf->ef_conf;
	} else if (cmd_id == AGTX_CMD_PD_CONF) {
		ptr = &conf->pd_conf;
	} else if (cmd_id == AGTX_CMD_OD_CONF) {
		ptr = &conf->od_conf;
	} else if (cmd_id == AGTX_CMD_RMS_CONF) {
		ptr = &conf->rms_conf;
	} else if (cmd_id == AGTX_CMD_LD_CONF) {
		ptr = &conf->ld_conf;
	} else if (cmd_id == AGTX_CMD_VDBG_CONF) {
		ptr = &conf->vdbg_conf;
	} else if (cmd_id == AGTX_CMD_VIDEO_PTZ_CONF) {
		ptr = &conf->ptz_conf;
	} else if (cmd_id == AGTX_CMD_SHD_CONF) {
		ptr = &conf->shd_conf;
	} else if (cmd_id == AGTX_CMD_PFM_CONF) {
		ptr = &conf->pfm_conf;
	} else if (cmd_id == AGTX_CMD_BM_CONF) {
		ptr = &conf->bm_conf;
	} else if (cmd_id == AGTX_CMD_DK_CONF) {
		ptr = &conf->dk_conf;
	} else if (cmd_id == AGTX_CMD_FLD_CONF) {
		ptr = &conf->fld_conf;
	} else if (cmd_id == AGTX_CMD_EAIF_CONF) {
		ptr = &conf->eaif_conf;
	} else if (cmd_id == AGTX_CMD_LSD_CONF) {
		ptr = &conf->lsd_conf;
	} else if (cmd_id == AGTX_CMD_EVT_CONF) {
		ptr = &conf->evt_conf;
	} else if (cmd_id == AGTX_CMD_GPIO_CONF) {
		ptr = &conf->gpio_conf;
	} else if (cmd_id == AGTX_CMD_EVT_PARAM) {
		ptr = &conf->evt_param;
	} else if (cmd_id == AGTX_CMD_LOCAL_RECORD_CONF) {
		ptr = &conf->local_record_conf;
	} else if (cmd_id == AGTX_CMD_ANTI_FLICKER_CONF) {
		ptr = &conf->anti_flicker_conf;
	} else if (cmd_id == AGTX_CMD_PWM_CONF) {
		ptr = &conf->pwm_conf;
	} else if (cmd_id == AGTX_CMD_PIR_CONF) {
		ptr = &conf->pir_conf;
	} else if (cmd_id == AGTX_CMD_FLOODLIGHT_CONF) {
		ptr = &conf->floodlight_conf;
	} else if (cmd_id == AGTX_CMD_DIP_SHP_WIN) {
		ptr = &conf->dip_shp_win;
	} else if (cmd_id == AGTX_CMD_DIP_NR_WIN) {
		ptr = &conf->dip_nr_win;
	} else if (cmd_id == AGTX_CMD_PRIVATE_MODE) {
		ptr = &conf->private_mode;
	} else if (cmd_id == AGTX_CMD_LIGHT_SENSOR_CONF) {
		ptr = &conf->light_sensor_conf;
	}
	if (!ptr) {
		fprintf(stderr, "Cannot find config element for cmd_id: %d!\n", cmd_id);
	}
	return ptr;
}

JSON_CONTENT_S *get_json_s(char *path)
{
	JSON_CONTENT_S *json_ptr = NULL;
	json_object *jobj = NULL;
	json_object *ch_jobj;
	char *ptr = NULL;

	ptr = get_file_content(path);

	if (!ptr) {
		goto end;
	}

	jobj = json_tokener_parse(ptr);

	if (jobj == NULL) {
		printf("+++got error as expected+++\n");
		goto end;
	}

	json_ptr = (JSON_CONTENT_S *)malloc(sizeof(JSON_CONTENT_S));

	if (!json_ptr)
		goto end;

	ch_jobj = json_object_object_get(jobj, "cmd_id");

	json_ptr->cmd_id = get_id(json_object_get_string(ch_jobj));

	ch_jobj = json_object_object_get(jobj, "json_content");

	json_ptr->json_content = (char *)malloc(strlen(json_object_get_string(ch_jobj)) + 1);

	if (!json_ptr->json_content) {
		free(json_ptr);
		json_ptr = NULL;
		goto end;
	}

	memcpy(json_ptr->json_content, json_object_get_string(ch_jobj), strlen(json_object_get_string(ch_jobj)) + 1);

end:

	if (jobj)
		json_object_put(jobj);

	if (ptr)
		free(ptr);

	return json_ptr;
}

static struct json_object *check_db_record(const char *db_path, const int id)
{
	int ret = 0;
	char *rec = NULL, *buf = NULL;
	struct json_object *obj = NULL;

	buf = malloc((size_t)CC_JSON_STR_BUF_SIZE);
	if (!buf) {
		fprintf(stderr, "No memory\n");
		goto end;
	}

	rec = get_sqldata_by_id_str_path(db_path, "json_tbl", "jstr", id);
	if (!rec) {
		fprintf(stderr, "Record (id 0x%08X) is not in DB %s\n", id, db_path);
		goto err;
	}

	bzero(buf, CC_JSON_STR_BUF_SIZE);
	strcpy(buf, rec);

	ret = validate_json_string(&obj, buf, strlen(buf));
	if (ret < 0) {
		//	obj = NULL:
		fprintf(stderr, "No a valid JSON string\n");
	}

err:
	free(buf);
end:
	return obj;
}

void create_column()
{
	exec_sql_cmd(DB_PATH, CREATE_JSON_TABLE);
}

#if CREATE_DB_WITH_HEADER_SUPPORT
void create_all_table()
{
	exec_sql_cmd(DB_PATH, CREATE_JSON_TABLE);
	exec_sql_cmd(DB_PATH, INSERT_CMD_SYS_SYS_INFO);
	exec_sql_cmd(DB_PATH, INSERT_CMD_SYS_FEATURE_OPTION);
	exec_sql_cmd(DB_PATH, INSERT_CMD_SYS_PRODUCT_OPTION_LIST);
	exec_sql_cmd(DB_PATH, INSERT_CMD_SYS_DB_INFO);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_BUF_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DEV_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_STRM_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_STITCH_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_AWB_PREF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_IMG_PREF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_ADV_IMG_PREF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_CAL);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_DBC);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_DCC);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_LSC);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_CTRL);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_AE);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_AWB);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_PTA);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_CSM);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_SHP);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_NR);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_ROI);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_TE);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_GAMMA);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_DIP_ISO);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_COLOR_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_AUDIO_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_PRODUCT_OPTION);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_RES_OPTION);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VIDEO_VENC_OPTION);
	exec_sql_cmd(DB_PATH, INSERT_CMD_EVT_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_EVT_GPIO_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_EVT_PARAM);
	exec_sql_cmd(DB_PATH, INSERT_CMD_OSD_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_OSD_PM_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_TD_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_MD_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_AROI_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_PD_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_OD_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_RMS_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_LD_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_IVA_EF_CONF);
	exec_sql_cmd(DB_PATH, INSERT_CMD_VDBG_CONF);
}
#endif

struct str_map {
	unsigned int id;
	char str[256];
};

struct str_map map[] = {
	{ AGTX_CMD_VIDEO_BUF_CONF, "AGTX_CMD_VIDEO_BUF_CONF" },
	{ AGTX_CMD_VIDEO_DEV_CONF, "AGTX_CMD_VIDEO_DEV_CONF" },
	{ AGTX_CMD_VIDEO_VIEW_TYPE, "AGTX_CMD_VIDEO_VIEW_TYPE" },
	{ AGTX_CMD_VIDEO_STRM_CONF, "AGTX_CMD_VIDEO_STRM_CONF" },
	{ AGTX_CMD_VIDEO_LAYOUT_CONF, "AGTX_CMD_VIDEO_LAYOUT_CONF" },
	{ AGTX_CMD_STITCH_CONF, "AGTX_CMD_STITCH_CONF" },
	{ AGTX_CMD_AWB_PREF, "AGTX_CMD_AWB_PREF" },
	{ AGTX_CMD_IMG_PREF, "AGTX_CMD_IMG_PREF" },
	{ AGTX_CMD_ADV_IMG_PREF, "AGTX_CMD_ADV_IMG_PREF" },
	{ AGTX_CMD_DIP_CAL, "AGTX_CMD_DIP_CAL" }, /* deprecated */
	{ AGTX_CMD_DIP_DBC, "AGTX_CMD_DIP_DBC" }, /* deprecated */
	{ AGTX_CMD_DIP_DCC, "AGTX_CMD_DIP_DCC" }, /* deprecated */
	{ AGTX_CMD_DIP_LSC, "AGTX_CMD_DIP_LSC" }, /* deprecated */
	{ AGTX_CMD_DIP_CTRL, "AGTX_CMD_DIP_CTRL" }, /* deprecated */
	{ AGTX_CMD_DIP_AE, "AGTX_CMD_DIP_AE" }, /* deprecated */
	{ AGTX_CMD_DIP_AWB, "AGTX_CMD_DIP_AWB" }, /* deprecated */
	{ AGTX_CMD_DIP_PTA, "AGTX_CMD_DIP_PTA" }, /* deprecated */
	{ AGTX_CMD_DIP_CSM, "AGTX_CMD_DIP_CSM" }, /* deprecated */
	{ AGTX_CMD_DIP_SHP, "AGTX_CMD_DIP_SHP" }, /* deprecated */
	{ AGTX_CMD_DIP_NR, "AGTX_CMD_DIP_NR" }, /* deprecated */
	{ AGTX_CMD_DIP_ROI, "AGTX_CMD_DIP_ROI" }, /* deprecated */
	{ AGTX_CMD_DIP_TE, "AGTX_CMD_DIP_TE" }, /* deprecated */
	{ AGTX_CMD_DIP_GAMMA, "AGTX_CMD_DIP_GAMMA" }, /* deprecated */
	{ AGTX_CMD_DIP_ISO, "AGTX_CMD_DIP_ISO" }, /* deprecated */
	{ AGTX_CMD_COLOR_CONF, "AGTX_CMD_COLOR_CONF" },
	{ AGTX_CMD_AUDIO_CONF, "AGTX_CMD_AUDIO_CONF" },
	{ AGTX_CMD_VOICE_CONF, "AGTX_CMD_VOICE_CONF" },
	{ AGTX_CMD_SIREN_CONF, "AGTX_CMD_SIREN_CONF" },
	{ AGTX_CMD_PRODUCT_OPTION, "AGTX_CMD_PRODUCT_OPTION" },
	{ AGTX_CMD_EVT_CONF, "AGTX_CMD_EVT_CONF" },
	{ AGTX_CMD_GPIO_CONF, "AGTX_CMD_GPIO_CONF" },
	{ AGTX_CMD_EVT_PARAM, "AGTX_CMD_EVT_PARAM" },
	{ AGTX_CMD_LOCAL_RECORD_CONF, "AGTX_CMD_LOCAL_RECORD_CONF" },
	{ AGTX_CMD_OSD_CONF, "AGTX_CMD_OSD_CONF" },
	{ AGTX_CMD_OSD_PM_CONF, "AGTX_CMD_OSD_PM_CONF" },
	{ AGTX_CMD_TD_CONF, "AGTX_CMD_TD_CONF" },
	{ AGTX_CMD_MD_CONF, "AGTX_CMD_MD_CONF" },
	{ AGTX_CMD_AROI_CONF, "AGTX_CMD_AROI_CONF" },
	{ AGTX_CMD_PD_CONF, "AGTX_CMD_PD_CONF" },
	{ AGTX_CMD_OD_CONF, "AGTX_CMD_OD_CONF" },
	{ AGTX_CMD_RMS_CONF, "AGTX_CMD_RMS_CONF" },
	{ AGTX_CMD_LD_CONF, "AGTX_CMD_LD_CONF" },
	{ AGTX_CMD_EF_CONF, "AGTX_CMD_EF_CONF" },
	{ AGTX_CMD_VDBG_CONF, "AGTX_CMD_VDBG_CONF" },
	{ AGTX_CMD_VIDEO_PTZ_CONF, "AGTX_CMD_VIDEO_PTZ_CONF" },
	{ AGTX_CMD_SHD_CONF, "AGTX_CMD_SHD_CONF" },
	{ AGTX_CMD_EAIF_CONF, "AGTX_CMD_EAIF_CONF" },
	{ AGTX_CMD_PFM_CONF, "AGTX_CMD_PFM_CONF" },
	{ AGTX_CMD_BM_CONF, "AGTX_CMD_BM_CONF" },
	{ AGTX_CMD_DK_CONF, "AGTX_CMD_DK_CONF" },
	{ AGTX_CMD_FLD_CONF, "AGTX_CMD_FLD_CONF" },
	{ AGTX_CMD_LSD_CONF, "AGTX_CMD_LSD_CONF" },
	{ AGTX_CMD_SYS_INFO, "AGTX_CMD_SYS_INFO" },
	{ AGTX_CMD_SYS_FEATURE_OPTION, "AGTX_CMD_SYS_FEATURE_OPTION" },
	{ AGTX_CMD_SYS_DB_INFO, "AGTX_CMD_SYS_DB_INFO" },
	{ AGTX_CMD_PRODUCT_OPTION_LIST, "AGTX_CMD_PRODUCT_OPTION_LIST" },
	{ AGTX_CMD_RES_OPTION, "AGTX_CMD_RES_OPTION" },
	{ AGTX_CMD_VENC_OPTION, "AGTX_CMD_VENC_OPTION" },
	{ AGTX_CMD_LDC_CONF, "AGTX_CMD_LDC_CONF" },
	{ AGTX_CMD_PANORAMA_CONF, "AGTX_CMD_PANORAMA_CONF" },
	{ AGTX_CMD_PANNING_CONF, "AGTX_CMD_PANNING_CONF" },
	{ AGTX_CMD_SURROUND_CONF, "AGTX_CMD_SURROUND_CONF" },
	{ AGTX_CMD_ANTI_FLICKER_CONF, "AGTX_CMD_ANTI_FLICKER_CONF" },
	{ AGTX_CMD_PWM_CONF, "AGTX_CMD_PWM_CONF" },
	{ AGTX_CMD_PIR_CONF, "AGTX_CMD_PIR_CONF" },
	{ AGTX_CMD_FLOODLIGHT_CONF, "AGTX_CMD_FLOODLIGHT_CONF" },
	{ AGTX_CMD_DIP_SHP_WIN, "AGTX_CMD_DIP_SHP_WIN" },
	{ AGTX_CMD_DIP_NR_WIN, "AGTX_CMD_DIP_NR_WIN" },
	{ AGTX_CMD_PRIVATE_MODE, "AGTX_CMD_PRIVATE_MODE_CONF" },
	{ AGTX_CMD_LIGHT_SENSOR_CONF, "AGTX_CMD_LIGHT_SENSOR_CONF" },
	{ AGTX_CMD_DIP_ENH, "AGTX_CMD_DIP_ENH" }, /* deprecated */
	{ AGTX_CMD_DIP_CORING, "AGTX_CMD_DIP_CORING" }, /* deprecated */
	{ AGTX_CMD_DIP_STAT, "AGTX_CMD_DIP_STAT" }, /* deprecated */
	{ AGTX_CMD_DIP_FCS, "AGTX_CMD_DIP_FCS" }, /* deprecated */
	{ AGTX_CMD_DIP_DHZ, "AGTX_CMD_DIP_DHZ" }, /* deprecated */
	{ AGTX_CMD_DIP_HDR_SYNTH, "AGTX_CMD_DIP_HDR_SYNTH" }, /* deprecated */
};

int get_id(const char *str)
{
	int i = 0;
	unsigned int id = -1;

	for (i = 0; (unsigned)i < NELEMS(map); i++) {
		if (strcmp(str, map[i].str) == 0) {
			id = map[i].id;
			break;
		}
	}

	return id;
}

char *get_file_content(const char *path)
{
	char *ptr = NULL;
	FILE *fp = NULL;
	long fsize = 0;
	size_t unused __attribute__((unused)) = 0;

	if ((fp = fopen(path, "r")) != NULL) {
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		fseek(fp, 0L, SEEK_SET);

		ptr = (char *)malloc(fsize + 1);

		if (!ptr) {
			goto end;
		}

		memset(ptr, 0, fsize);

		unused = fread(ptr, fsize, 1, fp);

		ptr[fsize] = 0;

		if (ferror(fp)) {
			free(ptr);
		}
	}

end:
	fclose(fp);
	return ptr;
}

void update_db(const char *dir_path)
{
	DIR *d;
	char *p1 = NULL, *p2 = NULL;
	int ret;
	struct dirent *dir;
	char *ptr = NULL;
	unsigned int id = 0;
	d = opendir(dir_path);
	char path[512];
	memset(path, 0, 512);

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			p1 = strtok(dir->d_name, ".");
			p1 = p1;
			p2 = strtok(NULL, ".");

			if (p2 != NULL) {
				ret = strcmp(p2, "json");

				if (ret == 0) {
					sprintf(path, "%s%s.json", dir_path, dir->d_name);
					printf("path=%s\n", path);
					ptr = get_file_content(path);

					if (ptr) {
						id = get_id(dir->d_name);

						if ((int)id != -1) {
							char *tmp_ptr = get_sqldata_by_id_str_path(DB_PATH, "json_tbl",
							                                           "jstr", id);

							if (!tmp_ptr) {
								free(tmp_ptr);

								ret = add_sqldata_by_id_str_path(DB_PATH, "json_tbl",
								                                 id, "jstr", ptr);

								if (ret == -1) {
									printf("error:insert id %d fail!!!\n", id);
								} else {
									printf("insert id %d success!!!\n", id);
								}

							} else {
								ret = update_sqldata_by_id_str_path(DB_PATH, "json_tbl",
								                                    id, "jstr", ptr);

								if (ret == -1) {
									printf("error:update id %d fail!!!\n", id);
								} else {
									printf("update id %d success!!!\n", id);
								}
							}
						}

						free(ptr);
					}
				}
			}
		}

		closedir(d);
	}
}

void update_db2(const char *dir_path)
{
	DIR *d;
	char *p1 = NULL, *p2 = NULL;
	int ret;
	struct dirent *dir;
	char *ptr = NULL;
	unsigned int id = 0;
	d = opendir(dir_path);
	char path[512];
	json_object *jobj = NULL;
	memset(path, 0, 512);

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			p1 = strtok(dir->d_name, ".");
			p1 = p1;
			p2 = strtok(NULL, ".");

			if (p2 != NULL) {
				ret = strcmp(p2, "json");

				if (ret == 0) {
					sprintf(path, "%s%s.json", dir_path, dir->d_name);
					printf("path=%s\n", path);
					ptr = get_file_content(path);

					if (ptr) {
						jobj = json_tokener_parse(ptr);

						if (jobj == NULL) {
							printf("+++got error as expected+++\n");
							continue;
						}

						json_object *ch_jobj = json_object_object_get(jobj, "cmd_id");

						if (ch_jobj == NULL) {
							continue;
						}

						id = get_id(json_object_get_string(ch_jobj));

						if ((int)id != -1) {
							ch_jobj = json_object_object_get(jobj, "json_content");

							if (ch_jobj == NULL) {
								continue;
							}

							char *tmp_ptr = get_sqldata_by_id_str_path(DB_PATH, "json_tbl",
							                                           "jstr", id);

							if (!tmp_ptr) {
								free(tmp_ptr);

								ret = add_sqldata_by_id_str_path(
								        DB_PATH, "json_tbl", id, "jstr",
								        json_object_get_string(ch_jobj));

								if (ret == -1) {
									printf("error:insert id %d fail!!!\n", id);
								} else {
									printf("insert id %d success!!!\n", id);
								}

							} else {
								ret = update_sqldata_by_id_str_path(
								        DB_PATH, "json_tbl", id, "jstr",
								        json_object_get_string(ch_jobj));

								if (ret == -1) {
									printf("error:update id %d fail!!!\n", id);
								} else {
									printf("update id %d success!!!\n", id);
								}
							}
						} else {
							printf("cannot find corresponding module id !!!\n");
						}

						json_object_put(jobj);
						free(ptr);
					} else {
						printf("dir_path is wrong !!!\n");
					}
				}
			}
		}

		closedir(d);
	}
}

void update_db3(const char *dir_path)
{
	DIR *d;
	char *p1 = NULL, *p2 = NULL;
	int ret;
	struct dirent *dir;

	d = opendir(dir_path);
	char path[512];
	json_object *file_obj = NULL;
	json_object *db_obj = NULL;
	json_object *obj = NULL;

	PARSE_FUNC_S parse_func = NULL;
	COMP_FUNC_S comp_func = NULL;
	JSON_CONTENT_S *json_content_ptr = NULL;
	DB_CONF_S conf = {};

	memset(path, 0, 512);

	if (d) {
		while ((dir = readdir(d)) != NULL) {
			p1 = strtok(dir->d_name, ".");
			p1 = p1;
			p2 = strtok(NULL, ".");

			if (p2 != NULL) {
				ret = strcmp(p2, "json");

				if (ret == 0) {
					sprintf(path, "%s%s.json", dir_path, dir->d_name);
					printf("path=%s\n", path);

					json_content_ptr = get_json_s(path);

					if (!json_content_ptr) {
						printf("warning:fail to get %s content", path);
						continue;
					}

					ret = determine_func(&parse_func, &comp_func, json_content_ptr->cmd_id);

					if (ret) {
						printf("warning: can not find corresponding parsing function cm_id=%d\n",
						       json_content_ptr->cmd_id);
						continue;
					}

					db_obj = check_db_record(DB_PATH, json_content_ptr->cmd_id);

					if (!db_obj) {
						ret = add_sqldata_by_id_str_path(DB_PATH, "json_tbl",
						                                 json_content_ptr->cmd_id, "jstr",
						                                 json_content_ptr->json_content);

					} else {
						ret = validate_json_string(&file_obj, json_content_ptr->json_content,
						                           strlen(json_content_ptr->json_content));

						if (ret < 0) {
							fprintf(stderr, "No a valid JSON string\n");

						} else {
							if (json_content_ptr->cmd_id == AGTX_CMD_PRODUCT_OPTION) {
								init_product_option(get_conf_ptr(&conf, json_content_ptr->cmd_id));
							} else if (json_content_ptr->cmd_id == AGTX_CMD_VENC_OPTION) {
								init_venc_option(get_conf_ptr(&conf, json_content_ptr->cmd_id));
							}

							parse_func(get_conf_ptr(&conf, json_content_ptr->cmd_id),
							           db_obj);
							parse_func(get_conf_ptr(&conf, json_content_ptr->cmd_id),
							           file_obj);

							obj = json_object_new_object();

							if (obj) {
								comp_func(obj, get_conf_ptr(&conf,
								                            json_content_ptr->cmd_id));

								update_sqldata_by_id_str_path(
								        DB_PATH, "json_tbl", json_content_ptr->cmd_id,
								        "jstr", json_object_to_json_string(obj));
							}
						}
					}

					if (db_obj)
						json_object_put(db_obj);

					if (file_obj)
						json_object_put(file_obj);

					if (obj)
						json_object_put(obj);

					if (json_content_ptr) {
						free(json_content_ptr->json_content);
						free(json_content_ptr);
						json_content_ptr = NULL;
					}
				}
			}
		}

		closedir(d);
	}
}

int main(int argc, char *argv[])
{
	char *p;

	long option = strtol(argv[1], &p, 10);

	if (option == 0) {
#if CREATE_DB_WITH_HEADER_SUPPORT
		printf("Create DB with header\n");
		create_all_table();
#else
		printf("Not support to create database with header\n");
#endif
	} else if (option == 1) {
		create_column();
	} else if (option == 2) {
		printf("dir path=%s\n", argv[2]);

		if (argc == 3) {
			update_db(argv[2]);
		}
	} else if (option == 3) {
		printf("dir path=%s\n", argv[2]);

		if (argc == 3) {
			update_db3(argv[2]);
		}
	}

	return 0;
}
