/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#include "cc_data.h"

#include <stdio.h>
#include <string.h>

#include "cm_sys_info.h"
#include "cm_sys_feature_option.h"
#include "cm_sys_db_info.h"
#include "cm_product_option_list.h"
#include "cm_video_dev_conf.h"
#include "cm_view_type.h"
#include "cm_video_strm_conf.h"
#include "cm_video_layout_conf.h"
#include "cm_stitch_conf.h"
#include "cm_audio_conf.h"
#include "cm_voice_conf.h"
#include "cm_siren_conf.h"
#include "cm_awb_pref.h"
#include "cm_img_pref.h"
#include "cm_adv_img_pref.h"
#include "cm_dip_cal_conf.h"
#include "cm_dip_dbc_conf.h"
#include "cm_dip_dcc_conf.h"
#include "cm_dip_lsc_conf.h"
#include "cm_dip_ctrl_conf.h"
#include "cm_dip_ae_conf.h"
#include "cm_dip_iso_conf.h"
#include "cm_dip_awb_conf.h"
#include "cm_dip_pta_conf.h"
#include "cm_dip_pca_conf.h"
#include "cm_dip_csm_conf.h"
#include "cm_dip_dms_conf.h"
#include "cm_dip_shp_conf.h"
#include "cm_dip_nr_conf.h"
#include "cm_dip_roi_conf.h"
#include "cm_dip_te_conf.h"
#include "cm_dip_gamma_conf.h"
#include "cm_dip_enh_conf.h"
#include "cm_dip_coring_conf.h"
#include "cm_dip_fcs_conf.h"
#include "cm_dip_dhz_conf.h"
#include "cm_dip_hdr_synth_conf.h"
#include "cm_dip_stat_conf.h"
#include "cm_dip_exp_info.h"
#include "cm_dip_wb_info.h"
#include "cm_dip_te_info.h"
#include "cm_color_conf.h"
#include "cm_product_option.h"
#include "cm_res_option.h"
#include "cm_venc_option.h"
#include "cm_video_ldc_conf.h"
#include "cm_panorama_conf.h"
#include "cm_panning_conf.h"
#include "cm_surround_conf.h"
#include "cm_anti_flicker_conf.h"
#include "cm_osd_conf.h"
#include "cm_osd_pm_conf.h"
#include "cm_iva_td_conf.h"
#include "cm_iva_md_conf.h"
#include "cm_iva_aroi_conf.h"
#include "cm_iva_pd_conf.h"
#include "cm_iva_od_conf.h"
#include "cm_iva_rms_conf.h"
#include "cm_iva_ld_conf.h"
#include "cm_iva_ef_conf.h"
#include "cm_vdbg_conf.h"
#include "cm_video_ptz_conf.h"
#include "cm_iva_shd_conf.h"
#include "cm_iva_eaif_conf.h"
#include "cm_iva_pfm_conf.h"
#include "cm_iva_bm_conf.h"
#include "cm_iva_dk_conf.h"
#include "cm_iva_fld_conf.h"
#include "cm_iaa_lsd_conf.h"
#include "cm_event_conf.h"
#include "cm_gpio_conf.h"
#include "cm_event_param.h"
#include "cm_local_record_conf.h"
#include "cm_pwm_conf.h"
#include "cm_pir_conf.h"
#include "cm_floodlight_conf.h"
#include "cm_dip_shp_win_conf.h"
#include "cm_dip_nr_win_conf.h"
#include "cm_private_mode_conf.h"
#include "cm_light_sensor_conf.h"

#include "cc_common.h"

const CC_CMD_INFO_S cmd_table[] = {
	{ AGTX_CMD_SYS_INFO, "AGTX_CMD_SYS_INFO", sizeof(AGTX_SYS_INFO_S), 0 },
	{ AGTX_CMD_SYS_FEATURE_OPTION, "AGTX_CMD_SYS_FEATURE_OPTION", sizeof(AGTX_SYS_FEATURE_OPTION_S), 0 },
	{ AGTX_CMD_PRODUCT_OPTION_LIST, "AGTX_CMD_PRODUCT_OPTION_LIST", sizeof(AGTX_PRODUCT_OPTION_LIST_S), 0 },
	{ AGTX_CMD_SYS_DB_INFO, "AGTX_CMD_SYS_DB_INFO", sizeof(AGTX_SYS_DB_INFO_S), 0 },
	{ AGTX_CMD_VIDEO_DEV_CONF, "AGTX_CMD_VIDEO_DEV_CONF", sizeof(AGTX_DEV_CONF_S), 0 },
	{ AGTX_CMD_VIDEO_STRM_CONF, "AGTX_CMD_VIDEO_STRM_CONF", sizeof(AGTX_STRM_CONF_S), 0 },
	{ AGTX_CMD_VIDEO_LAYOUT_CONF, "AGTX_CMD_VIDEO_LAYOUT_CONF", sizeof(AGTX_LAYOUT_CONF_S), 0 },
	{ AGTX_CMD_STITCH_CONF, "AGTX_CMD_STITCH_CONF", sizeof(AGTX_STITCH_CONF_S), 1 },
	{ AGTX_CMD_AWB_PREF, "AGTX_CMD_AWB_PREF", sizeof(AGTX_AWB_PREF_S), 0 },
	{ AGTX_CMD_IMG_PREF, "AGTX_CMD_IMG_PREF", sizeof(AGTX_IMG_PREF_S), 0 },
	{ AGTX_CMD_ADV_IMG_PREF, "AGTX_CMD_ADV_IMG_PREF", sizeof(AGTX_ADV_IMG_PREF_S), 0 },
	{ AGTX_CMD_DIP_CAL, "AGTX_CMD_DIP_CAL", sizeof(AGTX_DIP_CAL_CONF_S), 0 },
	{ AGTX_CMD_DIP_DBC, "AGTX_CMD_DIP_DBC", sizeof(AGTX_DIP_DBC_CONF_S), 0 },
	{ AGTX_CMD_DIP_DCC, "AGTX_CMD_DIP_DCC", sizeof(AGTX_DIP_DCC_CONF_S), 0 },
	{ AGTX_CMD_DIP_LSC, "AGTX_CMD_DIP_LSC", sizeof(AGTX_DIP_LSC_CONF_S), 0 },
	{ AGTX_CMD_DIP_CTRL, "AGTX_CMD_DIP_CTRL", sizeof(AGTX_DIP_CTRL_CONF_S), 0 },
	{ AGTX_CMD_DIP_AE, "AGTX_CMD_DIP_AE", sizeof(AGTX_DIP_AE_CONF_S), 0 },
	{ AGTX_CMD_DIP_ISO, "AGTX_CMD_DIP_ISO", sizeof(AGTX_DIP_ISO_CONF_S), 0 },
	{ AGTX_CMD_DIP_AWB, "AGTX_CMD_DIP_AWB", sizeof(AGTX_DIP_AWB_CONF_S), 0 },
	{ AGTX_CMD_DIP_PTA, "AGTX_CMD_DIP_PTA", sizeof(AGTX_DIP_PTA_CONF_S), 0 },
	{ AGTX_CMD_DIP_CSM, "AGTX_CMD_DIP_CSM", sizeof(AGTX_DIP_CSM_CONF_S), 0 },
	{ AGTX_CMD_DIP_SHP, "AGTX_CMD_DIP_SHP", sizeof(AGTX_DIP_SHP_CONF_S), 0 },
	{ AGTX_CMD_DIP_NR, "AGTX_CMD_DIP_NR", sizeof(AGTX_DIP_NR_CONF_S), 0 },
	{ AGTX_CMD_DIP_ROI, "AGTX_CMD_DIP_ROI", sizeof(AGTX_DIP_ROI_CONF_S), 0 },
	{ AGTX_CMD_DIP_TE, "AGTX_CMD_DIP_TE", sizeof(AGTX_DIP_TE_CONF_S), 0 },
	{ AGTX_CMD_DIP_GAMMA, "AGTX_CMD_DIP_GAMMA", sizeof(AGTX_DIP_GAMMA_CONF_S), 0 },
	{ AGTX_CMD_COLOR_CONF, "AGTX_CMD_COLOR_CONF", sizeof(AGTX_COLOR_CONF_S), 1 },
	{ AGTX_CMD_PRODUCT_OPTION, "AGTX_CMD_PRODUCT_OPTION", sizeof(_AGTX_PRODUCT_OPTION_S), 0 },
	{ AGTX_CMD_RES_OPTION, "AGTX_CMD_RES_OPTION", sizeof(AGTX_RES_OPTION_S), 0 },
	{ AGTX_CMD_VENC_OPTION, "AGTX_CMD_VENC_OPTION", sizeof(AGTX_VENC_OPTION_S), 0 },
	{ AGTX_CMD_LDC_CONF, "AGTX_CMD_LDC_CONF", sizeof(AGTX_LDC_CONF_S), 0 },
	{ AGTX_CMD_PANORAMA_CONF, "AGTX_CMD_PANORAMA_CONF", sizeof(AGTX_PANORAMA_CONF_S), 1 },
	{ AGTX_CMD_PANNING_CONF, "AGTX_CMD_PANNING_CONF", sizeof(AGTX_PANNING_CONF_S), 0 },
	{ AGTX_CMD_SURROUND_CONF, "AGTX_CMD_SURROUND_CONF", sizeof(AGTX_SURROUND_CONF_S), 0 },
	{ AGTX_CMD_ANTI_FLICKER_CONF, "AGTX_CMD_ANTI_FLICKER_CONF", sizeof(AGTX_ANTI_FLICKER_CONF_S), 0 },
	{ AGTX_CMD_AUDIO_CONF, "AGTX_CMD_AUDIO_CONF", sizeof(AGTX_AUDIO_CONF_S), 0 },
	{ AGTX_CMD_VOICE_CONF, "AGTX_CMD_VOICE_CONF", sizeof(AGTX_VOICE_CONF_S), 0 },
	{ AGTX_CMD_SIREN_CONF, "AGTX_CMD_SIREN_CONF", sizeof(AGTX_SIREN_CONF_S), 0 },
	{ AGTX_CMD_OSD_CONF, "AGTX_CMD_OSD_CONF", sizeof(AGTX_OSD_CONF_S), 0 },
	{ AGTX_CMD_OSD_PM_CONF, "AGTX_CMD_OSD_PM_CONF", sizeof(AGTX_OSD_PM_CONF_S), 0 },
	{ AGTX_CMD_TD_CONF, "AGTX_CMD_TD_CONF", sizeof(AGTX_IVA_TD_CONF_S), 0 },
	{ AGTX_CMD_MD_CONF, "AGTX_CMD_MD_CONF", sizeof(AGTX_IVA_MD_CONF_S), 0 },
	{ AGTX_CMD_AROI_CONF, "AGTX_CMD_AROI_CONF", sizeof(AGTX_IVA_AROI_CONF_S), 0 },
	{ AGTX_CMD_PD_CONF, "AGTX_CMD_PD_CONF", sizeof(AGTX_IVA_PD_CONF_S), 0 },
	{ AGTX_CMD_OD_CONF, "AGTX_CMD_OD_CONF", sizeof(AGTX_IVA_OD_CONF_S), 0 },
	{ AGTX_CMD_RMS_CONF, "AGTX_CMD_RMS_CONF", sizeof(AGTX_IVA_RMS_CONF_S), 0 },
	{ AGTX_CMD_LD_CONF, "AGTX_CMD_LD_CONF", sizeof(AGTX_IVA_LD_CONF_S), 0 },
	{ AGTX_CMD_EF_CONF, "AGTX_CMD_EF_CONF", sizeof(AGTX_IVA_EF_CONF_S), 0 },
	{ AGTX_CMD_VDBG_CONF, "AGTX_CMD_VDBG_CONF", sizeof(AGTX_VDBG_CONF_S), 0 },
	{ AGTX_CMD_VIDEO_PTZ_CONF, "AGTX_CMD_VIDEO_PTZ_CONF", sizeof(AGTX_VIDEO_PTZ_CONF_S), 0 },
	{ AGTX_CMD_SHD_CONF, "AGTX_CMD_SHD_CONF", sizeof(AGTX_IVA_SHD_CONF_S), 0 },
	{ AGTX_CMD_EAIF_CONF, "AGTX_CMD_EAIF_CONF", sizeof(AGTX_IVA_EAIF_CONF_S), 0 },
	{ AGTX_CMD_PFM_CONF, "AGTX_CMD_PFM_CONF", sizeof(AGTX_IVA_PFM_CONF_S), 0 },
	{ AGTX_CMD_BM_CONF, "AGTX_CMD_BM_CONF", sizeof(AGTX_IVA_BM_CONF_S), 0 },
	{ AGTX_CMD_DK_CONF, "AGTX_CMD_DK_CONF", sizeof(AGTX_IVA_DK_CONF_S), 0 },
	{ AGTX_CMD_FLD_CONF, "AGTX_CMD_FLD_CONF", sizeof(AGTX_IVA_FLD_CONF_S), 0 },
	{ AGTX_CMD_LSD_CONF, "AGTX_CMD_LSD_CONF", sizeof(AGTX_IAA_LSD_CONF_S), 0 },
	{ AGTX_CMD_EVT_CONF, "AGTX_CMD_EVT_CONF", sizeof(AGTX_EVENT_CONF_S), 1 },
	{ AGTX_CMD_GPIO_CONF, "AGTX_CMD_GPIO_CONF", sizeof(AGTX_GPIO_CONF_S), 1 },
	{ AGTX_CMD_EVT_PARAM, "AGTX_CMD_EVT_PARAM", sizeof(AGTX_EVENT_PARAM_S), 0 },
	{ AGTX_CMD_LOCAL_RECORD_CONF, "AGTX_CMD_LOCAL_RECORD_CONF", sizeof(AGTX_LOCAL_RECORD_CONF_S), 0 },
	{ AGTX_CMD_PWM_CONF, "AGTX_CMD_PWM_CONF", sizeof(AGTX_PWM_CONF_S), 0 },
	{ AGTX_CMD_PIR_CONF, "AGTX_CMD_PIR_CONF", sizeof(AGTX_PIR_CONF_S), 0 },
	{ AGTX_CMD_FLOODLIGHT_CONF, "AGTX_CMD_FLOODLIGHT_CONF", sizeof(AGTX_FLOODLIGHT_CONF_S), 0 },
	{ AGTX_CMD_DIP_SHP_WIN, "AGTX_CMD_DIP_SHP_WIN", sizeof(AGTX_DIP_SHP_WIN_CONF_S), 0 },
	{ AGTX_CMD_DIP_NR_WIN, "AGTX_CMD_DIP_NR_WIN", sizeof(AGTX_DIP_NR_WIN_CONF_S), 0 },
	{ AGTX_CMD_PRIVATE_MODE, "AGTX_CMD_PRIVATE_MODE", sizeof(AGTX_PRIVATE_MODE_CONF_S), 0 },
	{ AGTX_CMD_LIGHT_SENSOR_CONF, "AGTX_CMD_LIGHT_SENSOR_CONF", sizeof(AGTX_LIGHT_SENSOR_CONF_S), 0 },
	{ AGTX_CMD_DIP_ENH, "AGTX_CMD_DIP_ENH", sizeof(AGTX_DIP_ENH_CONF_S), 0 },
	{ AGTX_CMD_DIP_CORING, "AGTX_CMD_DIP_CORING", sizeof(AGTX_DIP_CORING_CONF_S), 0 },
	{ AGTX_CMD_DIP_STAT, "AGTX_CMD_DIP_STAT", sizeof(AGTX_DIP_STAT_CONF_S), 0 },
	{ AGTX_CMD_DIP_EXP_INFO, "AGTX_CMD_DIP_EXP_INFO", sizeof(AGTX_DIP_EXPOSURE_INFO_S), 0 },
	{ AGTX_CMD_DIP_WB_INFO, "AGTX_CMD_DIP_WB_INFO", sizeof(AGTX_DIP_WHITE_BALANCE_INFO_S), 0 },
	{ AGTX_CMD_DIP_FCS, "AGTX_CMD_DIP_FCS", sizeof(AGTX_DIP_FCS_CONF_S), 0 },
	{ AGTX_CMD_DIP_DHZ, "AGTX_CMD_DIP_DHZ", sizeof(AGTX_DIP_DHZ_CONF_S), 0 },
	{ AGTX_CMD_DIP_HDR_SYNTH, "AGTX_CMD_DIP_HDR_SYNTH", sizeof(AGTX_DIP_HDR_SYNTH_CONF_S), 0 },
	{ AGTX_CMD_DIP_TE_INFO, "AGTX_CMD_DIP_TE_INFO", sizeof(AGTX_DIP_TE_INFO_S), 0 },
	{ AGTX_CMD_DIP_DMS, "AGTX_CMD_DIP_DMS", sizeof(AGTX_DIP_DMS_CONF_S), 0 },
	{ AGTX_CMD_DIP_PCA, "AGTX_CMD_DIP_PCA", sizeof(AGTX_DIP_PCA_CONF_S), 0 },
	{ AGTX_CMD_VIDEO_VIEW_TYPE, "AGTX_CMD_VIDEO_VIEW_TYPE", sizeof(AGTX_VIEW_TYPE_INFO_S), 0 },
};

const long cmd_table_size = sizeof(cmd_table) / sizeof(cmd_table[0]);

static void _parse_sys_info(void *data, struct json_object *cmd_obj);
static void _parse_sys_feature_option(void *data, struct json_object *cmd_obj);
static void _parse_product_option_list(void *data, struct json_object *cmd_obj);
static void _parse_sys_db_info(void *data, struct json_object *cmd_obj);
static void _parse_video_dev_conf(void *data, struct json_object *cmd_obj);
static void _parse_view_type(void *data, struct json_object *cmd_obj);
static void _parse_video_strm_conf(void *data, struct json_object *cmd_obj);
static void _parse_video_layout_conf(void *data, struct json_object *cmd_obj);
static void _parse_stitch_conf(void *data, struct json_object *cmd_obj);
static void _parse_awb_pref(void *data, struct json_object *cmd_obj);
static void _parse_img_pref(void *data, struct json_object *cmd_obj);
static void _parse_adv_img_pref(void *data, struct json_object *cmd_obj);
static void _parse_dip_cal_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_dbc_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_dcc_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_lsc_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_ctrl_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_ae_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_iso_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_awb_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_pta_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_pca_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_csm_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_shp_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_nr_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_roi_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_te_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_gamma_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_enh_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_coring_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_fcs_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_dhz_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_dms_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_hdr_synth_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_stat_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_exposure_info(void *data, struct json_object *cmd_obj);
static void _parse_dip_white_balance_info(void *data, struct json_object *cmd_obj);
static void _parse_dip_te_info(void *data, struct json_object *cmd_obj);
static void _parse_color_conf(void *data, struct json_object *cmd_obj);
static void _parse_audio_conf(void *data, struct json_object *cmd_obj);
static void _parse_voice_conf(void *data, struct json_object *cmd_obj);
static void _parse_siren_conf(void *data, struct json_object *cmd_obj);
static void _parse_product_option(void *data, struct json_object *cmd_obj);
static void _parse_res_option(void *data, struct json_object *cmd_obj);
static void _parse_venc_option(void *data, struct json_object *cmd_obj);
static void _parse_ldc_conf(void *data, struct json_object *cmd_obj);
static void _parse_panorama_conf(void *data, struct json_object *cmd_obj);
static void _parse_panning_conf(void *data, struct json_object *cmd_obj);
static void _parse_surround_conf(void *data, struct json_object *cmd_obj);
static void _parse_anti_flicker_conf(void *data, struct json_object *cmd_obj);
static void _parse_osd_conf(void *data, struct json_object *cmd_obj);
static void _parse_osd_pm_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_td_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_md_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_aroi_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_pd_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_od_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_rms_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_ld_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_ef_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_eaif_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_pfm_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_bm_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_dk_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_fld_conf(void *data, struct json_object *cmd_obj);
static void _parse_iaa_lsd_conf(void *data, struct json_object *cmd_obj);
static void _parse_vdbg_conf(void *data, struct json_object *cmd_obj);
static void _parse_video_ptz_conf(void *data, struct json_object *cmd_obj);
static void _parse_iva_shd_conf(void *data, struct json_object *cmd_obj);
static void _parse_event_conf(void *data, struct json_object *cmd_obj);
static void _parse_gpio_conf(void *data, struct json_object *cmd_obj);
static void _parse_event_param(void *data, struct json_object *cmd_obj);
static void _parse_local_record_conf(void *data, struct json_object *cmd_obj);
static void _parse_pwm_conf(void *data, struct json_object *cmd_obj);
static void _parse_pir_conf(void *data, struct json_object *cmd_obj);
static void _parse_floodlight_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_shp_win_conf(void *data, struct json_object *cmd_obj);
static void _parse_dip_nr_win_conf(void *data, struct json_object *cmd_obj);
static void _parse_private_mode_conf(void *data, struct json_object *cmd_obj);
static void _parse_light_sensor_conf(void *data, struct json_object *cmd_obj);

static void _comp_sys_info(struct json_object *cmd_obj, void *data);
static void _comp_sys_feature_option(struct json_object *cmd_obj, void *data);
static void _comp_product_option_list(struct json_object *cmd_obj, void *data);
static void _comp_sys_db_info(struct json_object *cmd_obj, void *data);
static void _comp_video_dev_conf(struct json_object *cmd_obj, void *data);
static void _comp_view_type(struct json_object *cmd_obj, void *data);
static void _comp_video_strm_conf(struct json_object *cmd_obj, void *data);
static void _comp_video_layout_conf(struct json_object *cmd_obj, void *data);
static void _comp_stitch_conf(struct json_object *cmd_obj, void *data);
static void _comp_awb_pref(struct json_object *cmd_obj, void *data);
static void _comp_img_pref(struct json_object *cmd_obj, void *data);
static void _comp_adv_img_pref(struct json_object *cmd_obj, void *data);
static void _comp_dip_cal_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_dbc_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_dcc_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_lsc_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_ctrl_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_ae_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_iso_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_awb_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_pta_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_pca_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_csm_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_shp_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_nr_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_roi_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_te_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_gamma_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_enh_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_coring_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_fcs_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_dhz_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_dms_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_hdr_synth_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_stat_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_exposure_info(struct json_object *cmd_obj, void *data);
static void _comp_dip_white_balance_info(struct json_object *cmd_obj, void *data);
static void _comp_dip_te_info(struct json_object *cmd_obj, void *data);
static void _comp_color_conf(struct json_object *cmd_obj, void *data);
static void _comp_audio_conf(struct json_object *cmd_obj, void *data);
static void _comp_voice_conf(struct json_object *cmd_obj, void *data);
static void _comp_siren_conf(struct json_object *cmd_obj, void *data);
static void _comp_product_option(struct json_object *cmd_obj, void *data);
static void _comp_res_option(struct json_object *cmd_obj, void *data);
static void _comp_venc_option(struct json_object *cmd_obj, void *data);
static void _comp_ldc_conf(struct json_object *cmd_obj, void *data);
static void _comp_panorama_conf(struct json_object *cmd_obj, void *data);
static void _comp_panning_conf(struct json_object *cmd_obj, void *data);
static void _comp_surround_conf(struct json_object *cmd_obj, void *data);
static void _comp_anti_flicker_conf(struct json_object *cmd_obj, void *data);
static void _comp_osd_conf(struct json_object *cmd_obj, void *data);
static void _comp_osd_pm_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_td_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_md_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_aroi_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_pd_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_od_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_rms_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_ld_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_ef_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_eaif_conf(struct json_object *cmd_obj, void *data);
static void _comp_vdbg_conf(struct json_object *cmd_obj, void *data);
static void _comp_video_ptz_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_shd_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_pfm_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_bm_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_dk_conf(struct json_object *cmd_obj, void *data);
static void _comp_iva_fld_conf(struct json_object *cmd_obj, void *data);
static void _comp_iaa_lsd_conf(struct json_object *cmd_obj, void *data);
static void _comp_event_conf(struct json_object *cmd_obj, void *data);
static void _comp_gpio_conf(struct json_object *cmd_obj, void *data);
static void _comp_event_param(struct json_object *cmd_obj, void *data);
static void _comp_local_record_conf(struct json_object *cmd_obj, void *data);
static void _comp_pwm_conf(struct json_object *cmd_obj, void *data);
static void _comp_pir_conf(struct json_object *cmd_obj, void *data);
static void _comp_floodlight_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_shp_win_conf(struct json_object *cmd_obj, void *data);
static void _comp_dip_nr_win_conf(struct json_object *cmd_obj, void *data);
static void _comp_private_mode_conf(struct json_object *cmd_obj, void *data);
static void _comp_light_sensor_conf(struct json_object *ret_obj, void *data);

/* clang-format off */

PARSE_FUNC_S sys_parse_func[AGTX_ITEM_SYS_NUM] = {
	NULL,
	NULL,
	NULL,
	_parse_sys_info,
	_parse_sys_feature_option,
	_parse_product_option_list,
	NULL,
	_parse_sys_db_info,
};

COMP_FUNC_S sys_comp_func[AGTX_ITEM_SYS_NUM] = {
	NULL,
	NULL,
	NULL,
	_comp_sys_info,
	_comp_sys_feature_option,
	_comp_product_option_list,
	NULL,
	_comp_sys_db_info,
};

PARSE_FUNC_S net_parse_func[AGTX_ITEM_NET_NUM] = {
	NULL,
	NULL,
};

COMP_FUNC_S net_comp_func[AGTX_ITEM_NET_NUM] = {
	NULL,
	NULL,
};

PARSE_FUNC_S video_parse_func[AGTX_ITEM_VIDEO_NUM] = {
	NULL,
	NULL,
	_parse_video_dev_conf,
	_parse_video_strm_conf,
	_parse_stitch_conf,
	_parse_awb_pref,
	_parse_img_pref,
	_parse_adv_img_pref,
	_parse_dip_cal_conf,
	_parse_dip_dbc_conf,
	_parse_dip_dcc_conf,
	_parse_dip_lsc_conf,
	_parse_dip_ctrl_conf,
	_parse_dip_ae_conf,
	_parse_dip_awb_conf,
	_parse_dip_pta_conf,
	_parse_dip_csm_conf,
	_parse_dip_shp_conf,
	_parse_dip_nr_conf,
	_parse_dip_roi_conf,
	_parse_dip_te_conf,
	_parse_dip_gamma_conf,
	_parse_dip_iso_conf,
	_parse_color_conf,
	_parse_product_option,
	_parse_res_option,
	_parse_venc_option,
	_parse_ldc_conf,
	_parse_video_layout_conf,
	_parse_panorama_conf,
	_parse_panning_conf,
	_parse_surround_conf,
	_parse_anti_flicker_conf,
	_parse_dip_shp_win_conf,
	_parse_dip_nr_win_conf,
	_parse_private_mode_conf,
	_parse_dip_enh_conf,
	_parse_dip_coring_conf,
	_parse_dip_stat_conf,
	_parse_dip_exposure_info,
	_parse_dip_white_balance_info,
	_parse_dip_fcs_conf,
	_parse_dip_dhz_conf,
	_parse_dip_hdr_synth_conf,
	_parse_dip_te_info,
	_parse_dip_dms_conf,
	_parse_dip_pca_conf,
	_parse_view_type,
};

COMP_FUNC_S video_comp_func[AGTX_ITEM_VIDEO_NUM] = {
	NULL,
	NULL,
	_comp_video_dev_conf,
	_comp_video_strm_conf,
	_comp_stitch_conf,
	_comp_awb_pref,
	_comp_img_pref,
	_comp_adv_img_pref,
	_comp_dip_cal_conf,
	_comp_dip_dbc_conf,
	_comp_dip_dcc_conf,
	_comp_dip_lsc_conf,
	_comp_dip_ctrl_conf,
	_comp_dip_ae_conf,
	_comp_dip_awb_conf,
	_comp_dip_pta_conf,
	_comp_dip_csm_conf,
	_comp_dip_shp_conf,
	_comp_dip_nr_conf,
	_comp_dip_roi_conf,
	_comp_dip_te_conf,
	_comp_dip_gamma_conf,
	_comp_dip_iso_conf,
	_comp_color_conf,
	_comp_product_option,
	_comp_res_option,
	_comp_venc_option,
	_comp_ldc_conf,
	_comp_video_layout_conf,
	_comp_panorama_conf,
	_comp_panning_conf,
	_comp_surround_conf,
	_comp_anti_flicker_conf,
	_comp_dip_shp_win_conf,
	_comp_dip_nr_win_conf,
	_comp_private_mode_conf,
	_comp_dip_enh_conf,
	_comp_dip_coring_conf,
	_comp_dip_stat_conf,
	_comp_dip_exposure_info,
	_comp_dip_white_balance_info,
	_comp_dip_fcs_conf,
	_comp_dip_dhz_conf,
	_comp_dip_hdr_synth_conf,
	_comp_dip_te_info,
	_comp_dip_dms_conf,
	_comp_dip_pca_conf,
	_comp_view_type,
};

PARSE_FUNC_S audio_parse_func[AGTX_ITEM_AUDIO_NUM] = {
	NULL,
	_parse_audio_conf,
	_parse_voice_conf,
	_parse_siren_conf,
};

COMP_FUNC_S audio_comp_func[AGTX_ITEM_AUDIO_NUM] = {
	NULL,
	_comp_audio_conf,
	_comp_voice_conf,
	_comp_siren_conf,
};

PARSE_FUNC_S event_parse_func[AGTX_ITEM_EVT_NUM] = {
	NULL,
	_parse_event_conf,
	_parse_gpio_conf,
	_parse_event_param,
	_parse_local_record_conf,
	_parse_pwm_conf,
	_parse_pir_conf,
	_parse_floodlight_conf,
	_parse_light_sensor_conf,
};

COMP_FUNC_S event_comp_func[AGTX_ITEM_EVT_NUM] = {
	NULL,
	_comp_event_conf,
	_comp_gpio_conf,
	_comp_event_param,
	_comp_local_record_conf,
	_comp_pwm_conf,
	_comp_pir_conf,
	_comp_floodlight_conf,
	_comp_light_sensor_conf,
};

PARSE_FUNC_S osd_parse_func[AGTX_ITEM_OSD_NUM] = {
	NULL,
	_parse_osd_conf,
	_parse_osd_pm_conf,
};

COMP_FUNC_S osd_comp_func[AGTX_ITEM_OSD_NUM] = {
	NULL,
	_comp_osd_conf,
	_comp_osd_pm_conf,
};

PARSE_FUNC_S iva_parse_func[AGTX_ITEM_IVA_NUM] = {
	NULL,
	_parse_iva_td_conf,
	_parse_iva_md_conf,
	_parse_iva_aroi_conf,
	_parse_iva_pd_conf,
	_parse_iva_od_conf,
	_parse_iva_rms_conf,
	_parse_iva_ld_conf,
	_parse_iva_ef_conf,
	_parse_vdbg_conf,
	_parse_video_ptz_conf,
	_parse_iva_shd_conf,
	_parse_iva_eaif_conf,
	_parse_iva_pfm_conf,
	_parse_iva_bm_conf,
	_parse_iva_dk_conf,
	_parse_iva_fld_conf,
};

COMP_FUNC_S iva_comp_func[AGTX_ITEM_IVA_NUM] = {
	NULL,
	_comp_iva_td_conf,
	_comp_iva_md_conf,
	_comp_iva_aroi_conf,
	_comp_iva_pd_conf,
	_comp_iva_od_conf,
	_comp_iva_rms_conf,
	_comp_iva_ld_conf,
	_comp_iva_ef_conf,
	_comp_vdbg_conf,
	_comp_video_ptz_conf,
	_comp_iva_shd_conf,
	_comp_iva_eaif_conf,
	_comp_iva_pfm_conf,
	_comp_iva_bm_conf,
	_comp_iva_dk_conf,
	_comp_iva_fld_conf,
};

PARSE_FUNC_S iaa_parse_func[AGTX_ITEM_IAA_NUM] = {
	NULL,
	_parse_iaa_lsd_conf,
};

COMP_FUNC_S iaa_comp_func[AGTX_ITEM_IAA_NUM] = {
	NULL,
	_comp_iaa_lsd_conf,
};

/* clang-format on */

static void _parse_sys_info(void *data, struct json_object *cmd_obj)
{
	AGTX_SYS_INFO_S *tmp = (AGTX_SYS_INFO_S *)data;

	parse_sys_info(tmp, cmd_obj);
}

static void _parse_sys_feature_option(void *data, struct json_object *cmd_obj)
{
	AGTX_SYS_FEATURE_OPTION_S *tmp = (AGTX_SYS_FEATURE_OPTION_S *)data;

	parse_sys_feature_option(tmp, cmd_obj);
}

static void _parse_product_option_list(void *data, struct json_object *cmd_obj)
{
	AGTX_PRODUCT_OPTION_LIST_S *tmp = (AGTX_PRODUCT_OPTION_LIST_S *)data;

	parse_product_option_list(tmp, cmd_obj);
}

static void _parse_sys_db_info(void *data, struct json_object *cmd_obj)
{
	AGTX_SYS_DB_INFO_S *tmp = (AGTX_SYS_DB_INFO_S *)data;

	parse_sys_db_info(tmp, cmd_obj);
}

static void _parse_video_dev_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DEV_CONF_S *tmp = (AGTX_DEV_CONF_S *)data;

	parse_video_dev_conf(tmp, cmd_obj);
}

static void _parse_view_type(void *data, struct json_object *cmd_obj)
{
	AGTX_VIEW_TYPE_INFO_S *tmp = (AGTX_VIEW_TYPE_INFO_S *)data;

	parse_view_type(tmp, cmd_obj);
}

static void _parse_video_strm_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_STRM_CONF_S *tmp = (AGTX_STRM_CONF_S *)data;

	parse_video_strm_conf(tmp, cmd_obj);
}

static void _parse_video_layout_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_LAYOUT_CONF_S *tmp = (AGTX_LAYOUT_CONF_S *)data;

	parse_layout_conf(tmp, cmd_obj);
}

static void _parse_stitch_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_STITCH_CONF_S *tmp = (AGTX_STITCH_CONF_S *)data;

	parse_stitch_conf(tmp, cmd_obj);
}

static void _parse_audio_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_AUDIO_CONF_S *tmp = (AGTX_AUDIO_CONF_S *)data;

	parse_audio_conf(tmp, cmd_obj);
}

static void _parse_voice_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_VOICE_CONF_S *tmp = (AGTX_VOICE_CONF_S *)data;

	parse_voice_conf(tmp, cmd_obj);
}

static void _parse_siren_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_SIREN_CONF_S *tmp = (AGTX_SIREN_CONF_S *)data;

	parse_siren_conf(tmp, cmd_obj);
}

static void _parse_awb_pref(void *data, struct json_object *cmd_obj)
{
	AGTX_AWB_PREF_S *tmp = (AGTX_AWB_PREF_S *)data;

	parse_awb_pref(tmp, cmd_obj);
}

static void _parse_img_pref(void *data, struct json_object *cmd_obj)
{
	AGTX_IMG_PREF_S *tmp = (AGTX_IMG_PREF_S *)data;

	parse_img_pref(tmp, cmd_obj);
}

static void _parse_adv_img_pref(void *data, struct json_object *cmd_obj)
{
	AGTX_ADV_IMG_PREF_S *tmp = (AGTX_ADV_IMG_PREF_S *)data;

	parse_adv_img_pref(tmp, cmd_obj);
}

static void _parse_dip_cal_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_CAL_CONF_S *tmp = (AGTX_DIP_CAL_CONF_S *)data;

	parse_dip_cal_conf(tmp, cmd_obj);
}

static void _parse_dip_dbc_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_DBC_CONF_S *tmp = (AGTX_DIP_DBC_CONF_S *)data;

	parse_dip_dbc_conf(tmp, cmd_obj);
}

static void _parse_dip_dcc_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_DCC_CONF_S *tmp = (AGTX_DIP_DCC_CONF_S *)data;

	parse_dip_dcc_conf(tmp, cmd_obj);
}

static void _parse_dip_lsc_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_LSC_CONF_S *tmp = (AGTX_DIP_LSC_CONF_S *)data;

	parse_dip_lsc_conf(tmp, cmd_obj);
}

static void _parse_dip_ctrl_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_CTRL_CONF_S *tmp = (AGTX_DIP_CTRL_CONF_S *)data;

	parse_dip_ctrl_conf(tmp, cmd_obj);
}

static void _parse_dip_ae_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_AE_CONF_S *tmp = (AGTX_DIP_AE_CONF_S *)data;

	parse_dip_ae_conf(tmp, cmd_obj);
}

static void _parse_dip_iso_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_ISO_CONF_S *tmp = (AGTX_DIP_ISO_CONF_S *)data;

	parse_dip_iso_conf(tmp, cmd_obj);
}

static void _parse_dip_awb_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_AWB_CONF_S *tmp = (AGTX_DIP_AWB_CONF_S *)data;

	parse_dip_awb_conf(tmp, cmd_obj);
}

static void _parse_dip_pta_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_PTA_CONF_S *tmp = (AGTX_DIP_PTA_CONF_S *)data;

	parse_dip_pta_conf(tmp, cmd_obj);
}

static void _parse_dip_pca_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_PCA_CONF_S *tmp = (AGTX_DIP_PCA_CONF_S *)data;

	parse_dip_pca_conf(tmp, cmd_obj);
}

static void _parse_dip_csm_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_CSM_CONF_S *tmp = (AGTX_DIP_CSM_CONF_S *)data;

	parse_dip_csm_conf(tmp, cmd_obj);
}

static void _parse_dip_shp_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_SHP_CONF_S *tmp = (AGTX_DIP_SHP_CONF_S *)data;

	parse_dip_shp_conf(tmp, cmd_obj);
}

static void _parse_dip_nr_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_NR_CONF_S *tmp = (AGTX_DIP_NR_CONF_S *)data;

	parse_dip_nr_conf(tmp, cmd_obj);
}

static void _parse_dip_roi_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_ROI_CONF_S *tmp = (AGTX_DIP_ROI_CONF_S *)data;

	parse_dip_roi_conf(tmp, cmd_obj);
}

static void _parse_dip_te_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_TE_CONF_S *tmp = (AGTX_DIP_TE_CONF_S *)data;

	parse_dip_te_conf(tmp, cmd_obj);
}

static void _parse_dip_gamma_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_GAMMA_CONF_S *tmp = (AGTX_DIP_GAMMA_CONF_S *)data;

	parse_dip_gamma_conf(tmp, cmd_obj);
}

static void _parse_dip_enh_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_ENH_CONF_S *tmp = (AGTX_DIP_ENH_CONF_S *)data;

	parse_dip_enh_conf(tmp, cmd_obj);
}

static void _parse_dip_coring_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_CORING_CONF_S *tmp = (AGTX_DIP_CORING_CONF_S *)data;

	parse_dip_coring_conf(tmp, cmd_obj);
}

static void _parse_dip_fcs_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_FCS_CONF_S *tmp = (AGTX_DIP_FCS_CONF_S *)data;

	parse_dip_fcs_conf(tmp, cmd_obj);
}

static void _parse_dip_dhz_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_DHZ_CONF_S *tmp = (AGTX_DIP_DHZ_CONF_S *)data;

	parse_dip_dhz_conf(tmp, cmd_obj);
}

static void _parse_dip_dms_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_DMS_CONF_S *tmp = (AGTX_DIP_DMS_CONF_S *)data;

	parse_dip_dms_conf(tmp, cmd_obj);
}

static void _parse_dip_hdr_synth_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_HDR_SYNTH_CONF_S *tmp = (AGTX_DIP_HDR_SYNTH_CONF_S *)data;

	parse_dip_hdr_synth_conf(tmp, cmd_obj);
}

static void _parse_dip_stat_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_STAT_CONF_S *tmp = (AGTX_DIP_STAT_CONF_S *)data;

	parse_dip_stat_conf(tmp, cmd_obj);
}

static void _parse_dip_exposure_info(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_EXPOSURE_INFO_S *tmp = (AGTX_DIP_EXPOSURE_INFO_S *)data;

	parse_dip_exposure_info(tmp, cmd_obj);
}

static void _parse_dip_white_balance_info(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_WHITE_BALANCE_INFO_S *tmp = (AGTX_DIP_WHITE_BALANCE_INFO_S *)data;

	parse_dip_white_balance_info(tmp, cmd_obj);
}

static void _parse_dip_te_info(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_TE_INFO_S *tmp = (AGTX_DIP_TE_INFO_S *)data;

	parse_dip_te_info(tmp, cmd_obj);
}

static void _parse_color_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_COLOR_CONF_S *tmp = (AGTX_COLOR_CONF_S *)data;

	parse_color_conf(tmp, cmd_obj);
}

static void _parse_product_option(void *data, struct json_object *cmd_obj)
{
	_AGTX_PRODUCT_OPTION_S *tmp = (_AGTX_PRODUCT_OPTION_S *)data;

	parse_product_option(tmp, cmd_obj);
}

static void _parse_res_option(void *data, struct json_object *cmd_obj)
{
	AGTX_RES_OPTION_S *tmp = (AGTX_RES_OPTION_S *)data;

	parse_res_option(tmp, cmd_obj);
}

static void _parse_venc_option(void *data, struct json_object *cmd_obj)
{
	AGTX_VENC_OPTION_S *tmp = (AGTX_VENC_OPTION_S *)data;

	parse_venc_option(tmp, cmd_obj);
}

static void _parse_ldc_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_LDC_CONF_S *tmp = (AGTX_LDC_CONF_S *)data;

	parse_ldc_conf(tmp, cmd_obj);
}

static void _parse_panorama_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_PANORAMA_CONF_S *tmp = (AGTX_PANORAMA_CONF_S *)data;

	parse_panorama_conf(tmp, cmd_obj);
}

static void _parse_panning_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_PANNING_CONF_S *tmp = (AGTX_PANNING_CONF_S *)data;

	parse_panning_conf(tmp, cmd_obj);
}

static void _parse_surround_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_SURROUND_CONF_S *tmp = (AGTX_SURROUND_CONF_S *)data;

	parse_surround_conf(tmp, cmd_obj);
}

static void _parse_anti_flicker_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_ANTI_FLICKER_CONF_S *tmp = (AGTX_ANTI_FLICKER_CONF_S *)data;

	parse_anti_flicker_conf(tmp, cmd_obj);
}

static void _parse_osd_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_OSD_CONF_S *tmp = (AGTX_OSD_CONF_S *)data;

	parse_osd_conf(tmp, cmd_obj);
}

static void _parse_osd_pm_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_OSD_PM_CONF_S *tmp = (AGTX_OSD_PM_CONF_S *)data;

	parse_osd_pm_conf(tmp, cmd_obj);
}

static void _parse_iva_td_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_TD_CONF_S *tmp = (AGTX_IVA_TD_CONF_S *)data;

	parse_iva_td_conf(tmp, cmd_obj);
}

static void _parse_iva_md_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_MD_CONF_S *tmp = (AGTX_IVA_MD_CONF_S *)data;

	parse_iva_md_conf(tmp, cmd_obj);
}

static void _parse_iva_aroi_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_AROI_CONF_S *tmp = (AGTX_IVA_AROI_CONF_S *)data;

	parse_iva_aroi_conf(tmp, cmd_obj);
}

static void _parse_iva_pd_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_PD_CONF_S *tmp = (AGTX_IVA_PD_CONF_S *)data;

	parse_iva_pd_conf(tmp, cmd_obj);
}

static void _parse_iva_od_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_OD_CONF_S *tmp = (AGTX_IVA_OD_CONF_S *)data;

	parse_iva_od_conf(tmp, cmd_obj);
}

static void _parse_iva_rms_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_RMS_CONF_S *tmp = (AGTX_IVA_RMS_CONF_S *)data;

	parse_iva_rms_conf(tmp, cmd_obj);
}

static void _parse_iva_ld_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_LD_CONF_S *tmp = (AGTX_IVA_LD_CONF_S *)data;

	parse_iva_ld_conf(tmp, cmd_obj);
}

static void _parse_iva_ef_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_EF_CONF_S *tmp = (AGTX_IVA_EF_CONF_S *)data;

	parse_iva_ef_conf(tmp, cmd_obj);
}

static void _parse_vdbg_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_VDBG_CONF_S *tmp = (AGTX_VDBG_CONF_S *)data;

	parse_vdbg_conf(tmp, cmd_obj);
}

static void _parse_video_ptz_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_VIDEO_PTZ_CONF_S *tmp = (AGTX_VIDEO_PTZ_CONF_S *)data;

	parse_video_ptz_conf(tmp, cmd_obj);
}

static void _parse_iva_shd_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_SHD_CONF_S *tmp = (AGTX_IVA_SHD_CONF_S *)data;

	parse_iva_shd_conf(tmp, cmd_obj);
}

static void _parse_iva_eaif_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_EAIF_CONF_S *tmp = (AGTX_IVA_EAIF_CONF_S *)data;

	parse_iva_eaif_conf(tmp, cmd_obj);
}

static void _parse_iva_pfm_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_PFM_CONF_S *tmp = (AGTX_IVA_PFM_CONF_S *)data;

	parse_iva_pfm_conf(tmp, cmd_obj);
}

static void _parse_iva_bm_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_BM_CONF_S *tmp = (AGTX_IVA_BM_CONF_S *)data;

	parse_iva_bm_conf(tmp, cmd_obj);
}

static void _parse_iva_dk_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_DK_CONF_S *tmp = (AGTX_IVA_DK_CONF_S *)data;

	parse_iva_dk_conf(tmp, cmd_obj);
}

static void _parse_iva_fld_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IVA_FLD_CONF_S *tmp = (AGTX_IVA_FLD_CONF_S *)data;

	parse_iva_fld_conf(tmp, cmd_obj);
}

static void _parse_iaa_lsd_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_IAA_LSD_CONF_S *tmp = (AGTX_IAA_LSD_CONF_S *)data;

	parse_iaa_lsd_conf(tmp, cmd_obj);
}

static void _parse_event_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_EVENT_CONF_S *tmp = (AGTX_EVENT_CONF_S *)data;

	parse_event_conf(tmp, cmd_obj);
}

static void _parse_gpio_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_GPIO_CONF_S *tmp = (AGTX_GPIO_CONF_S *)data;

	parse_gpio_conf(tmp, cmd_obj);
}

static void _parse_event_param(void *data, struct json_object *cmd_obj)
{
	AGTX_EVENT_PARAM_S *tmp = (AGTX_EVENT_PARAM_S *)data;

	parse_event_param(tmp, cmd_obj);
}

static void _parse_local_record_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_LOCAL_RECORD_CONF_S *tmp = (AGTX_LOCAL_RECORD_CONF_S *)data;

	parse_local_record_conf(tmp, cmd_obj);
}

static void _parse_pwm_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_PWM_CONF_S *tmp = (AGTX_PWM_CONF_S *)data;

	parse_pwm_conf(tmp, cmd_obj);
}

static void _parse_pir_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_PIR_CONF_S *tmp = (AGTX_PIR_CONF_S *)data;

	parse_pir_conf(tmp, cmd_obj);
}

static void _parse_floodlight_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_FLOODLIGHT_CONF_S *tmp = (AGTX_FLOODLIGHT_CONF_S *)data;

	parse_floodlight_conf(tmp, cmd_obj);
}

static void _parse_dip_shp_win_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_SHP_WIN_CONF_S *tmp = (AGTX_DIP_SHP_WIN_CONF_S *)data;

	parse_dip_shp_win_conf(tmp, cmd_obj);
}

static void _parse_dip_nr_win_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_DIP_NR_WIN_CONF_S *tmp = (AGTX_DIP_NR_WIN_CONF_S *)data;

	parse_dip_nr_win_conf(tmp, cmd_obj);
}

static void _parse_private_mode_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_PRIVATE_MODE_CONF_S *tmp = (AGTX_PRIVATE_MODE_CONF_S *)data;

	parse_private_mode_conf(tmp, cmd_obj);
}

static void _parse_light_sensor_conf(void *data, struct json_object *cmd_obj)
{
	AGTX_LIGHT_SENSOR_CONF_S *tmp = (AGTX_LIGHT_SENSOR_CONF_S *)data;

	parse_light_sensor_conf(tmp, cmd_obj);
}

static void _comp_sys_info(struct json_object *ret_obj, void *data)
{
	AGTX_SYS_INFO_S *tmp = (AGTX_SYS_INFO_S *)data;

	comp_sys_info(ret_obj, tmp);
}

static void _comp_sys_feature_option(struct json_object *ret_obj, void *data)
{
	AGTX_SYS_FEATURE_OPTION_S *tmp = (AGTX_SYS_FEATURE_OPTION_S *)data;

	comp_sys_feature_option(ret_obj, tmp);
}

static void _comp_product_option_list(struct json_object *ret_obj, void *data)
{
	AGTX_PRODUCT_OPTION_LIST_S *tmp = (AGTX_PRODUCT_OPTION_LIST_S *)data;

	comp_product_option_list(ret_obj, tmp);
}

static void _comp_sys_db_info(struct json_object *ret_obj, void *data)
{
	AGTX_SYS_DB_INFO_S *tmp = (AGTX_SYS_DB_INFO_S *)data;

	comp_sys_db_info(ret_obj, tmp);
}

static void _comp_video_dev_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DEV_CONF_S *tmp = (AGTX_DEV_CONF_S *)data;

	comp_video_dev_conf(ret_obj, tmp);
}

static void _comp_view_type(struct json_object *ret_obj, void *data)
{
	AGTX_VIEW_TYPE_INFO_S *tmp = (AGTX_VIEW_TYPE_INFO_S *)data;

	comp_view_type(ret_obj, tmp);
}

static void _comp_video_strm_conf(struct json_object *ret_obj, void *data)
{
	AGTX_STRM_CONF_S *tmp = (AGTX_STRM_CONF_S *)data;

	comp_video_strm_conf(ret_obj, tmp);
}

static void _comp_video_layout_conf(struct json_object *ret_obj, void *data)
{
	AGTX_LAYOUT_CONF_S *tmp = (AGTX_LAYOUT_CONF_S *)data;

	comp_layout_conf(ret_obj, tmp);
}

static void _comp_stitch_conf(struct json_object *ret_obj, void *data)
{
	AGTX_STITCH_CONF_S *tmp = (AGTX_STITCH_CONF_S *)data;

	comp_stitch_conf(ret_obj, tmp);
}

static void _comp_audio_conf(struct json_object *ret_obj, void *data)
{
	AGTX_AUDIO_CONF_S *tmp = (AGTX_AUDIO_CONF_S *)data;

	comp_audio_conf(ret_obj, tmp);
}

static void _comp_voice_conf(struct json_object *ret_obj, void *data)
{
	AGTX_VOICE_CONF_S *tmp = (AGTX_VOICE_CONF_S *)data;

	comp_voice_conf(ret_obj, tmp);
}

static void _comp_siren_conf(struct json_object *ret_obj, void *data)
{
	AGTX_SIREN_CONF_S *tmp = (AGTX_SIREN_CONF_S *)data;

	comp_siren_conf(ret_obj, tmp);
}

static void _comp_awb_pref(struct json_object *ret_obj, void *data)
{
	AGTX_AWB_PREF_S *tmp = (AGTX_AWB_PREF_S *)data;

	comp_awb_pref(ret_obj, tmp);
}

static void _comp_img_pref(struct json_object *ret_obj, void *data)
{
	AGTX_IMG_PREF_S *tmp = (AGTX_IMG_PREF_S *)data;

	comp_img_pref(ret_obj, tmp);
}

static void _comp_adv_img_pref(struct json_object *ret_obj, void *data)
{
	AGTX_ADV_IMG_PREF_S *tmp = (AGTX_ADV_IMG_PREF_S *)data;

	comp_adv_img_pref(ret_obj, tmp);
}

static void _comp_dip_cal_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_CAL_CONF_S *tmp = (AGTX_DIP_CAL_CONF_S *)data;

	comp_dip_cal_conf(ret_obj, tmp);
}

static void _comp_dip_dbc_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_DBC_CONF_S *tmp = (AGTX_DIP_DBC_CONF_S *)data;

	comp_dip_dbc_conf(ret_obj, tmp);
}

static void _comp_dip_dcc_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_DCC_CONF_S *tmp = (AGTX_DIP_DCC_CONF_S *)data;

	comp_dip_dcc_conf(ret_obj, tmp);
}

static void _comp_dip_lsc_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_LSC_CONF_S *tmp = (AGTX_DIP_LSC_CONF_S *)data;

	comp_dip_lsc_conf(ret_obj, tmp);
}

static void _comp_dip_ctrl_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_CTRL_CONF_S *tmp = (AGTX_DIP_CTRL_CONF_S *)data;

	comp_dip_ctrl_conf(ret_obj, tmp);
}

static void _comp_dip_ae_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_AE_CONF_S *tmp = (AGTX_DIP_AE_CONF_S *)data;

	comp_dip_ae_conf(ret_obj, tmp);
}

static void _comp_dip_iso_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_ISO_CONF_S *tmp = (AGTX_DIP_ISO_CONF_S *)data;

	comp_dip_iso_conf(ret_obj, tmp);
}

static void _comp_dip_awb_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_AWB_CONF_S *tmp = (AGTX_DIP_AWB_CONF_S *)data;

	comp_dip_awb_conf(ret_obj, tmp);
}

static void _comp_dip_pta_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_PTA_CONF_S *tmp = (AGTX_DIP_PTA_CONF_S *)data;

	comp_dip_pta_conf(ret_obj, tmp);
}

static void _comp_dip_pca_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_PCA_CONF_S *tmp = (AGTX_DIP_PCA_CONF_S *)data;

	comp_dip_pca_conf(ret_obj, tmp);
}

static void _comp_dip_csm_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_CSM_CONF_S *tmp = (AGTX_DIP_CSM_CONF_S *)data;

	comp_dip_csm_conf(ret_obj, tmp);
}

static void _comp_dip_shp_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_SHP_CONF_S *tmp = (AGTX_DIP_SHP_CONF_S *)data;

	comp_dip_shp_conf(ret_obj, tmp);
}

static void _comp_dip_nr_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_NR_CONF_S *tmp = (AGTX_DIP_NR_CONF_S *)data;

	comp_dip_nr_conf(ret_obj, tmp);
}

static void _comp_dip_roi_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_ROI_CONF_S *tmp = (AGTX_DIP_ROI_CONF_S *)data;

	comp_dip_roi_conf(ret_obj, tmp);
}

static void _comp_dip_te_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_TE_CONF_S *tmp = (AGTX_DIP_TE_CONF_S *)data;

	comp_dip_te_conf(ret_obj, tmp);
}

static void _comp_dip_gamma_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_GAMMA_CONF_S *tmp = (AGTX_DIP_GAMMA_CONF_S *)data;

	comp_dip_gamma_conf(ret_obj, tmp);
}

static void _comp_dip_enh_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_ENH_CONF_S *tmp = (AGTX_DIP_ENH_CONF_S *)data;

	comp_dip_enh_conf(ret_obj, tmp);
}

static void _comp_dip_coring_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_CORING_CONF_S *tmp = (AGTX_DIP_CORING_CONF_S *)data;

	comp_dip_coring_conf(ret_obj, tmp);
}

static void _comp_dip_fcs_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_FCS_CONF_S *tmp = (AGTX_DIP_FCS_CONF_S *)data;

	comp_dip_fcs_conf(ret_obj, tmp);
}

static void _comp_dip_dhz_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_DHZ_CONF_S *tmp = (AGTX_DIP_DHZ_CONF_S *)data;

	comp_dip_dhz_conf(ret_obj, tmp);
}

static void _comp_dip_dms_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_DMS_CONF_S *tmp = (AGTX_DIP_DMS_CONF_S *)data;

	comp_dip_dms_conf(ret_obj, tmp);
}

static void _comp_dip_hdr_synth_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_HDR_SYNTH_CONF_S *tmp = (AGTX_DIP_HDR_SYNTH_CONF_S *)data;

	comp_dip_hdr_synth_conf(ret_obj, tmp);
}

static void _comp_dip_stat_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_STAT_CONF_S *tmp = (AGTX_DIP_STAT_CONF_S *)data;

	comp_dip_stat_conf(ret_obj, tmp);
}

static void _comp_dip_exposure_info(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_EXPOSURE_INFO_S *tmp = (AGTX_DIP_EXPOSURE_INFO_S *)data;

	comp_dip_exposure_info(ret_obj, tmp);
}

static void _comp_dip_white_balance_info(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_WHITE_BALANCE_INFO_S *tmp = (AGTX_DIP_WHITE_BALANCE_INFO_S *)data;

	comp_dip_white_balance_info(ret_obj, tmp);
}

static void _comp_dip_te_info(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_TE_INFO_S *tmp = (AGTX_DIP_TE_INFO_S *)data;

	comp_dip_te_info(ret_obj, tmp);
}

static void _comp_color_conf(struct json_object *ret_obj, void *data)
{
	AGTX_COLOR_CONF_S *tmp = (AGTX_COLOR_CONF_S *)data;

	comp_color_conf(ret_obj, tmp);
}

static void _comp_product_option(struct json_object *ret_obj, void *data)
{
	_AGTX_PRODUCT_OPTION_S *tmp = (_AGTX_PRODUCT_OPTION_S *)data;

	comp_product_option(ret_obj, tmp);
}

static void _comp_res_option(struct json_object *ret_obj, void *data)
{
	AGTX_RES_OPTION_S *tmp = (AGTX_RES_OPTION_S *)data;

	comp_res_option(ret_obj, tmp);
}

static void _comp_venc_option(struct json_object *ret_obj, void *data)
{
	AGTX_VENC_OPTION_S *tmp = (AGTX_VENC_OPTION_S *)data;

	comp_venc_option(ret_obj, tmp);
}

static void _comp_ldc_conf(struct json_object *ret_obj, void *data)
{
	AGTX_LDC_CONF_S *tmp = (AGTX_LDC_CONF_S *)data;

	comp_ldc_conf(ret_obj, tmp);
}

static void _comp_panorama_conf(struct json_object *ret_obj, void *data)
{
	AGTX_PANORAMA_CONF_S *tmp = (AGTX_PANORAMA_CONF_S *)data;

	comp_panorama_conf(ret_obj, tmp);
}

static void _comp_panning_conf(struct json_object *ret_obj, void *data)
{
	AGTX_PANNING_CONF_S *tmp = (AGTX_PANNING_CONF_S *)data;

	comp_panning_conf(ret_obj, tmp);
}

static void _comp_surround_conf(struct json_object *ret_obj, void *data)
{
	AGTX_SURROUND_CONF_S *tmp = (AGTX_SURROUND_CONF_S *)data;

	comp_surround_conf(ret_obj, tmp);
}

static void _comp_anti_flicker_conf(struct json_object *ret_obj, void *data)
{
	AGTX_ANTI_FLICKER_CONF_S *tmp = (AGTX_ANTI_FLICKER_CONF_S *)data;

	comp_anti_flicker_conf(ret_obj, tmp);
}

static void _comp_osd_conf(struct json_object *ret_obj, void *data)
{
	AGTX_OSD_CONF_S *tmp = (AGTX_OSD_CONF_S *)data;

	comp_osd_conf(ret_obj, tmp);
}

static void _comp_osd_pm_conf(struct json_object *ret_obj, void *data)
{
	AGTX_OSD_PM_CONF_S *tmp = (AGTX_OSD_PM_CONF_S *)data;

	comp_osd_pm_conf(ret_obj, tmp);
}

static void _comp_iva_td_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_TD_CONF_S *tmp = (AGTX_IVA_TD_CONF_S *)data;

	comp_iva_td_conf(ret_obj, tmp);
}

static void _comp_iva_md_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_MD_CONF_S *tmp = (AGTX_IVA_MD_CONF_S *)data;

	comp_iva_md_conf(ret_obj, tmp);
}

static void _comp_iva_aroi_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_AROI_CONF_S *tmp = (AGTX_IVA_AROI_CONF_S *)data;

	comp_iva_aroi_conf(ret_obj, tmp);
}

static void _comp_iva_pd_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_PD_CONF_S *tmp = (AGTX_IVA_PD_CONF_S *)data;

	comp_iva_pd_conf(ret_obj, tmp);
}

static void _comp_iva_od_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_OD_CONF_S *tmp = (AGTX_IVA_OD_CONF_S *)data;

	comp_iva_od_conf(ret_obj, tmp);
}

static void _comp_iva_rms_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_RMS_CONF_S *tmp = (AGTX_IVA_RMS_CONF_S *)data;

	comp_iva_rms_conf(ret_obj, tmp);
}

static void _comp_iva_ld_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_LD_CONF_S *tmp = (AGTX_IVA_LD_CONF_S *)data;

	comp_iva_ld_conf(ret_obj, tmp);
}

static void _comp_iva_ef_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_EF_CONF_S *tmp = (AGTX_IVA_EF_CONF_S *)data;

	comp_iva_ef_conf(ret_obj, tmp);
}

static void _comp_vdbg_conf(struct json_object *ret_obj, void *data)
{
	AGTX_VDBG_CONF_S *tmp = (AGTX_VDBG_CONF_S *)data;

	comp_vdbg_conf(ret_obj, tmp);
}

static void _comp_video_ptz_conf(struct json_object *ret_obj, void *data)
{
	AGTX_VIDEO_PTZ_CONF_S *tmp = (AGTX_VIDEO_PTZ_CONF_S *)data;

	comp_video_ptz_conf(ret_obj, tmp);
}

static void _comp_iva_shd_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_SHD_CONF_S *tmp = (AGTX_IVA_SHD_CONF_S *)data;

	comp_iva_shd_conf(ret_obj, tmp);
}

static void _comp_iva_eaif_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_EAIF_CONF_S *tmp = (AGTX_IVA_EAIF_CONF_S *)data;

	comp_iva_eaif_conf(ret_obj, tmp);
}

static void _comp_iva_pfm_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_PFM_CONF_S *tmp = (AGTX_IVA_PFM_CONF_S *)data;

	comp_iva_pfm_conf(ret_obj, tmp);
}

static void _comp_iva_bm_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_BM_CONF_S *tmp = (AGTX_IVA_BM_CONF_S *)data;

	comp_iva_bm_conf(ret_obj, tmp);
}

static void _comp_iva_dk_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_DK_CONF_S *tmp = (AGTX_IVA_DK_CONF_S *)data;

	comp_iva_dk_conf(ret_obj, tmp);
}

static void _comp_iva_fld_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IVA_FLD_CONF_S *tmp = (AGTX_IVA_FLD_CONF_S *)data;

	comp_iva_fld_conf(ret_obj, tmp);
}

static void _comp_iaa_lsd_conf(struct json_object *ret_obj, void *data)
{
	AGTX_IAA_LSD_CONF_S *tmp = (AGTX_IAA_LSD_CONF_S *)data;

	comp_iaa_lsd_conf(ret_obj, tmp);
}

static void _comp_event_conf(struct json_object *ret_obj, void *data)
{
	AGTX_EVENT_CONF_S *tmp = (AGTX_EVENT_CONF_S *)data;

	comp_event_conf(ret_obj, tmp);
}

static void _comp_gpio_conf(struct json_object *ret_obj, void *data)
{
	AGTX_GPIO_CONF_S *tmp = (AGTX_GPIO_CONF_S *)data;

	comp_gpio_conf(ret_obj, tmp);
}

static void _comp_event_param(struct json_object *ret_obj, void *data)
{
	AGTX_EVENT_PARAM_S *tmp = (AGTX_EVENT_PARAM_S *)data;

	comp_event_param(ret_obj, tmp);
}

static void _comp_local_record_conf(struct json_object *ret_obj, void *data)
{
	AGTX_LOCAL_RECORD_CONF_S *tmp = (AGTX_LOCAL_RECORD_CONF_S *)data;

	comp_local_record_conf(ret_obj, tmp);
}

static void _comp_pwm_conf(struct json_object *ret_obj, void *data)
{
	AGTX_PWM_CONF_S *tmp = (AGTX_PWM_CONF_S *)data;

	comp_pwm_conf(ret_obj, tmp);
}

static void _comp_pir_conf(struct json_object *ret_obj, void *data)
{
	AGTX_PIR_CONF_S *tmp = (AGTX_PIR_CONF_S *)data;

	comp_pir_conf(ret_obj, tmp);
}

static void _comp_floodlight_conf(struct json_object *ret_obj, void *data)
{
	AGTX_FLOODLIGHT_CONF_S *tmp = (AGTX_FLOODLIGHT_CONF_S *)data;

	comp_floodlight_conf(ret_obj, tmp);
}

static void _comp_dip_shp_win_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_SHP_WIN_CONF_S *tmp = (AGTX_DIP_SHP_WIN_CONF_S *)data;

	comp_dip_shp_win_conf(ret_obj, tmp);
}

static void _comp_dip_nr_win_conf(struct json_object *ret_obj, void *data)
{
	AGTX_DIP_NR_WIN_CONF_S *tmp = (AGTX_DIP_NR_WIN_CONF_S *)data;

	comp_dip_nr_win_conf(ret_obj, tmp);
}

static void _comp_private_mode_conf(struct json_object *ret_obj, void *data)
{
	AGTX_PRIVATE_MODE_CONF_S *tmp = (AGTX_PRIVATE_MODE_CONF_S *)data;

	comp_private_mode_conf(ret_obj, tmp);
}

static void _comp_light_sensor_conf(struct json_object *ret_obj, void *data)
{
	AGTX_LIGHT_SENSOR_CONF_S *tmp = (AGTX_LIGHT_SENSOR_CONF_S *)data;

	comp_light_sensor_conf(ret_obj, tmp);
}

int determine_func(PARSE_FUNC_S *parse_func, COMP_FUNC_S *comp_func, int cmd_id)
{
	int ret = -1;
	unsigned int cat = AGTX_CMD_CAT(cmd_id);
	unsigned int item = AGTX_CMD_ITEM(cmd_id);

	switch (cat) {
	case AGTX_CAT_SYS:
		if (item < AGTX_ITEM_SYS_NUM) {
			*parse_func = sys_parse_func[item];
			*comp_func = sys_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_NET:
		if (item < AGTX_ITEM_NET_NUM) {
			*parse_func = net_parse_func[item];
			*comp_func = net_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_VIDEO:
		if (item < AGTX_ITEM_VIDEO_NUM) {
			*parse_func = video_parse_func[item];
			*comp_func = video_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_AUDIO:
		if (item < AGTX_ITEM_AUDIO_NUM) {
			*parse_func = audio_parse_func[item];
			*comp_func = audio_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_EVT:
		if (item < AGTX_ITEM_EVT_NUM) {
			*parse_func = event_parse_func[item];
			*comp_func = event_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_OSD:
		if (item < AGTX_ITEM_OSD_NUM) {
			*parse_func = osd_parse_func[item];
			*comp_func = osd_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_IVA:
		if (item < AGTX_ITEM_IVA_NUM) {
			*parse_func = iva_parse_func[item];
			*comp_func = iva_comp_func[item];
			ret = 0;
		}
		break;
	case AGTX_CAT_IAA:
		if (item < AGTX_ITEM_IAA_NUM) {
			*parse_func = iaa_parse_func[item];
			*comp_func = iaa_comp_func[item];
			ret = 0;
		}
		break;
	default:
		fprintf(stderr, "Unknown caterory\n");
		break;
	}

	return ret;
}

void list_cmd_table(void)
{
	for (int i = 0; i < cmd_table_size; ++i) {
		printf("%-28s", cmd_table[i].str);
		if ((i % 2) == 1 || i == cmd_table_size - 1) {
			printf(",\n");
		} else {
			printf(", ");
		}
	}
}

int get_cmd_id(const char *str)
{
	int found = 0;

	for (int i = 0; i < cmd_table_size; ++i) {
		if (!strcmp(str, cmd_table[i].str)) {
			found = cmd_table[i].cmd_id;
			break;
		}
	}

	return found;
}
