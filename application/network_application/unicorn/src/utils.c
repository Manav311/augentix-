#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <unistd.h>

#include "action.h"
#include "json.h"
#include "agtx_cmd.h"
#include "agtx_common.h"
#include "app_dip_api.h"
#include "app_view_api.h"
#include "mpi_dip_alg.h"

static int extractFileData(char **buf, char *filepath);

unsigned int get_command_id(const char *setting)
{
	if (strcmp(setting, "exp_info") == 0) {
		return AGTX_CMD_DIP_EXP_INFO;
	} else if (strcmp(setting, "wb_info") == 0) {
		return AGTX_CMD_DIP_WB_INFO;
	} else if (strcmp(setting, "te_info") == 0) {
		return AGTX_CMD_DIP_TE_INFO;
	} else if (strcmp(setting, "audio_conf") == 0) {
		return AGTX_CMD_AUDIO_CONF;
	} else if (strcmp(setting, "evt_conf") == 0) {
		return AGTX_CMD_EVT_CONF;
	} else if (strcmp(setting, "floodlight_conf") == 0) {
		return AGTX_CMD_FLOODLIGHT_CONF;
	} else if (strcmp(setting, "gpio_conf") == 0) {
		return AGTX_CMD_GPIO_CONF;
	} else if (strcmp(setting, "light_sensor_conf") == 0) {
		return AGTX_CMD_LIGHT_SENSOR_CONF;
	} else if (strcmp(setting, "evt_parm") == 0) {
		return AGTX_CMD_EVT_PARAM;
	} else if (strcmp(setting, "pir_conf") == 0) {
		return AGTX_CMD_PIR_CONF;
	} else if (strcmp(setting, "pwm_conf") == 0) {
		return AGTX_CMD_PWM_CONF;
	} else if (strcmp(setting, "lsd_conf") == 0) {
		return AGTX_CMD_LSD_CONF;
	} else if (strcmp(setting, "aroi_conf") == 0) {
		return AGTX_CMD_AROI_CONF;
	} else if (strcmp(setting, "eaif_conf") == 0) {
		return AGTX_CMD_EAIF_CONF;
	} else if (strcmp(setting, "ef_conf") == 0) {
		return AGTX_CMD_EF_CONF;
	} else if (strcmp(setting, "ld_conf") == 0) {
		return AGTX_CMD_LD_CONF;
	} else if (strcmp(setting, "md_conf") == 0) {
		return AGTX_CMD_MD_CONF;
	} else if (strcmp(setting, "od_conf") == 0) {
		return AGTX_CMD_OD_CONF;
	} else if (strcmp(setting, "pd_conf") == 0) {
		return AGTX_CMD_PD_CONF;
	} else if (strcmp(setting, "pfm_conf") == 0) {
		return AGTX_CMD_PFM_CONF;
	} else if (strcmp(setting, "rms_conf") == 0) {
		return AGTX_CMD_RMS_CONF;
	} else if (strcmp(setting, "shd_conf") == 0) {
		return AGTX_CMD_SHD_CONF;
	} else if (strcmp(setting, "td_conf") == 0) {
		return AGTX_CMD_TD_CONF;
	} else if (strcmp(setting, "local_record_conf") == 0) {
		return AGTX_CMD_LOCAL_RECORD_CONF;
	} else if (strcmp(setting, "osd_conf") == 0) {
		return AGTX_CMD_OSD_CONF;
	} else if (strcmp(setting, "osd_pm_conf") == 0) {
		return AGTX_CMD_OSD_PM_CONF;
	} else if (strcmp(setting, "private_mode") == 0) {
		return AGTX_CMD_PRIVATE_MODE;
	} else if (strcmp(setting, "siren_conf") == 0) {
		return AGTX_CMD_SIREN_CONF;
	} else if (strcmp(setting, "sys_db_info") == 0) {
		return AGTX_CMD_SYS_DB_INFO;
	} else if (strcmp(setting, "sys_feature_option") == 0) {
		return AGTX_CMD_SYS_FEATURE_OPTION;
	} else if (strcmp(setting, "product_option_list") == 0) {
		return AGTX_CMD_PRODUCT_OPTION_LIST;
	} else if (strcmp(setting, "sys_info") == 0) {
		return AGTX_CMD_SYS_INFO;
	} else if (strcmp(setting, "vdbg_conf") == 0) {
		return AGTX_CMD_VDBG_CONF;
	} else if (strcmp(setting, "adv_img_pref") == 0) {
		return AGTX_CMD_ADV_IMG_PREF;
	} else if (strcmp(setting, "anti_flicker_conf") == 0) {
		return AGTX_CMD_ANTI_FLICKER_CONF;
	} else if (strcmp(setting, "awb_pref") == 0) {
		return AGTX_CMD_AWB_PREF;
	} else if (strcmp(setting, "color_conf") == 0) {
		return AGTX_CMD_COLOR_CONF;
	} else if (strcmp(setting, "video_dev_conf") == 0) {
		return AGTX_CMD_VIDEO_DEV_CONF;
	} else if (strcmp(setting, "view_type") == 0) {
		return AGTX_CMD_VIDEO_VIEW_TYPE;
	} else if (strcmp(setting, "dip_ae") == 0) {
		return AGTX_CMD_DIP_AE;
	} else if (strcmp(setting, "dip_awb") == 0) {
		return AGTX_CMD_DIP_AWB;
	} else if (strcmp(setting, "dip_cal") == 0) {
		return AGTX_CMD_DIP_CAL;
	} else if (strcmp(setting, "dip_csm") == 0) {
		return AGTX_CMD_DIP_CSM;
	} else if (strcmp(setting, "dip_dms") == 0) {
		return AGTX_CMD_DIP_DMS;
	} else if (strcmp(setting, "dip_coring") == 0) {
		return AGTX_CMD_DIP_CORING;
	} else if (strcmp(setting, "dip_fcs") == 0) {
		return AGTX_CMD_DIP_FCS;
	} else if (strcmp(setting, "dip_dhz") == 0) {
		return AGTX_CMD_DIP_DHZ;
	} else if (strcmp(setting, "dip_hdr_synth") == 0) {
		return AGTX_CMD_DIP_HDR_SYNTH;
	} else if (strcmp(setting, "dip_stat") == 0) {
		return AGTX_CMD_DIP_STAT;
	} else if (strcmp(setting, "dip_ctrl") == 0) {
		return AGTX_CMD_DIP_CTRL;
	} else if (strcmp(setting, "dip_dbc") == 0) {
		return AGTX_CMD_DIP_DBC;
	} else if (strcmp(setting, "dip_dcc") == 0) {
		return AGTX_CMD_DIP_DCC;
	} else if (strcmp(setting, "dip_enh") == 0) {
		return AGTX_CMD_DIP_ENH;
	} else if (strcmp(setting, "dip_gamma") == 0) {
		return AGTX_CMD_DIP_GAMMA;
	} else if (strcmp(setting, "dip_iso") == 0) {
		return AGTX_CMD_DIP_ISO;
	} else if (strcmp(setting, "dip_lsc") == 0) {
		return AGTX_CMD_DIP_LSC;
	} else if (strcmp(setting, "dip_nr") == 0) {
		return AGTX_CMD_DIP_NR;
	} else if (strcmp(setting, "dip_nr_win") == 0) {
		return AGTX_CMD_DIP_NR_WIN;
	} else if (strcmp(setting, "dip_pta") == 0) {
		return AGTX_CMD_DIP_PTA;
	} else if (strcmp(setting, "dip_pca") == 0) {
		return AGTX_CMD_DIP_PCA;
	} else if (strcmp(setting, "dip_roi") == 0) {
		return AGTX_CMD_DIP_ROI;
	} else if (strcmp(setting, "dip_shp") == 0) {
		return AGTX_CMD_DIP_SHP;
	} else if (strcmp(setting, "dip_shp_win") == 0) {
		return AGTX_CMD_DIP_SHP_WIN;
	} else if (strcmp(setting, "dip_te") == 0) {
		return AGTX_CMD_DIP_TE;
	} else if (strcmp(setting, "img_pref") == 0) {
		return AGTX_CMD_IMG_PREF;
	} else if (strcmp(setting, "video_layout_conf") == 0) {
		return AGTX_CMD_VIDEO_LAYOUT_CONF;
	} else if (strcmp(setting, "ldc_conf") == 0) {
		return AGTX_CMD_LDC_CONF;
	} else if (strcmp(setting, "panning_conf") == 0) {
		return AGTX_CMD_PANNING_CONF;
	} else if (strcmp(setting, "panorama_conf") == 0) {
		return AGTX_CMD_PANORAMA_CONF;
	} else if (strcmp(setting, "product_option") == 0) {
		return AGTX_CMD_PRODUCT_OPTION;
	} else if (strcmp(setting, "video_ptz_conf") == 0) {
		return AGTX_CMD_VIDEO_PTZ_CONF;
	} else if (strcmp(setting, "res_option") == 0) {
		return AGTX_CMD_RES_OPTION;
	} else if (strcmp(setting, "stitch_conf") == 0) {
		return AGTX_CMD_STITCH_CONF;
	} else if (strcmp(setting, "video_strm_conf") == 0) {
		return AGTX_CMD_VIDEO_STRM_CONF;
	} else if (strcmp(setting, "surround_conf") == 0) {
		return AGTX_CMD_SURROUND_CONF;
	} else if (strcmp(setting, "venc_option") == 0) {
		return AGTX_CMD_VENC_OPTION;
	} else if (strcmp(setting, "voice_conf") == 0) {
		return AGTX_CMD_VOICE_CONF;
	}

	else if (strcmp(setting, "")) {
		/* Do Nothing */
	} else {
		ERR("'%s' is an unknown setting!\n", setting);
	}
	return -1;
}

int getMpiFunc(MPI_READ_S *read_func, MPI_WRITE_S *write_func, int cmd_id)
{
	if (AGTX_CMD_CAT(cmd_id) != AGTX_CAT_VIDEO) {
		return -1;
	}

	switch (AGTX_CMD_ITEM(cmd_id)) {
	case AGTX_ITEM_VIDEO_STITCH_CONF:
		*read_func = (MPI_READ_S)APP_VIEW_getStitchConf;
		*write_func = (MPI_WRITE_S)APP_VIEW_setStitchConf;
		break;
	case AGTX_ITEM_VIDEO_DIP_CAL:
		*read_func = (MPI_READ_S)APP_DIP_getCal;
		*write_func = (MPI_WRITE_S)APP_DIP_setCal;
		break;
	case AGTX_ITEM_VIDEO_DIP_DBC:
		*read_func = (MPI_READ_S)APP_DIP_getDbc;
		*write_func = (MPI_WRITE_S)APP_DIP_setDbc;
		break;
	case AGTX_ITEM_VIDEO_DIP_DCC:
		*read_func = (MPI_READ_S)APP_DIP_getDcc;
		*write_func = (MPI_WRITE_S)APP_DIP_setDcc;
		break;
	case AGTX_ITEM_VIDEO_DIP_LSC:
		*read_func = (MPI_READ_S)APP_DIP_getLsc;
		*write_func = (MPI_WRITE_S)APP_DIP_setLsc;
		break;
	case AGTX_ITEM_VIDEO_DIP_CTRL:
		*read_func = (MPI_READ_S)APP_DIP_getCtrl;
		*write_func = (MPI_WRITE_S)APP_DIP_setCtrl;
		break;
	case AGTX_ITEM_VIDEO_DIP_AE:
		*read_func = (MPI_READ_S)APP_DIP_getAe;
		*write_func = (MPI_WRITE_S)APP_DIP_setAe;
		break;
	case AGTX_ITEM_VIDEO_DIP_AWB:
		*read_func = (MPI_READ_S)APP_DIP_getAwb;
		*write_func = (MPI_WRITE_S)APP_DIP_setAwb;
		break;
	case AGTX_ITEM_VIDEO_DIP_PTA:
		*read_func = (MPI_READ_S)APP_DIP_getPta;
		*write_func = (MPI_WRITE_S)APP_DIP_setPta;
		break;
	case AGTX_ITEM_VIDEO_DIP_PCA_TABLE:
		*read_func = (MPI_READ_S)APP_DIP_getPca;
		*write_func = (MPI_WRITE_S)APP_DIP_setPca;
		break;
	case AGTX_ITEM_VIDEO_DIP_CSM:
		*read_func = (MPI_READ_S)APP_DIP_getCsm;
		*write_func = (MPI_WRITE_S)APP_DIP_setCsm;
		break;
	case AGTX_ITEM_VIDEO_DIP_DMS:
		*read_func = (MPI_READ_S)APP_DIP_getDms;
		*write_func = (MPI_WRITE_S)APP_DIP_setDms;
		break;
	case AGTX_ITEM_VIDEO_DIP_SHP:
		*read_func = (MPI_READ_S)APP_DIP_getShp;
		*write_func = (MPI_WRITE_S)APP_DIP_setShp;
		break;
	case AGTX_ITEM_VIDEO_DIP_NR:
		*read_func = (MPI_READ_S)APP_DIP_getNr;
		*write_func = (MPI_WRITE_S)APP_DIP_setNr;
		break;
	case AGTX_ITEM_VIDEO_DIP_ROI:
		*read_func = (MPI_READ_S)APP_DIP_getRoi;
		*write_func = (MPI_WRITE_S)APP_DIP_setRoi;
		break;
	case AGTX_ITEM_VIDEO_DIP_TE:
		*read_func = (MPI_READ_S)APP_DIP_getTe;
		*write_func = (MPI_WRITE_S)APP_DIP_setTe;
		break;
	case AGTX_ITEM_VIDEO_DIP_GAMMA:
		*read_func = (MPI_READ_S)APP_DIP_getGamma;
		*write_func = (MPI_WRITE_S)APP_DIP_setGamma;
		break;
	case AGTX_ITEM_VIDEO_DIP_ISO:
		*read_func = (MPI_READ_S)APP_DIP_getIso;
		*write_func = (MPI_WRITE_S)APP_DIP_setIso;
		break;
	case AGTX_ITEM_VIDEO_LDC:
		*read_func = (MPI_READ_S)APP_VIEW_getLdcConf;
		*write_func = (MPI_WRITE_S)APP_VIEW_setLdcConf;
		break;
	case AGTX_ITEM_VIDEO_PANORAMA:
		*read_func = (MPI_READ_S)APP_VIEW_getPanoramaConf;
		*write_func = (MPI_WRITE_S)APP_VIEW_setPanoramaConf;
		break;
	case AGTX_ITEM_VIDEO_PANNING:
		*read_func = (MPI_READ_S)APP_VIEW_getPanningConf;
		*write_func = (MPI_WRITE_S)APP_VIEW_setPanningConf;
		break;
	case AGTX_ITEM_VIDEO_SURROUND:
		*read_func = (MPI_READ_S)APP_VIEW_getSurroundConf;
		*write_func = (MPI_WRITE_S)APP_VIEW_setSurroundConf;
		break;
	case AGTX_ITEM_VIDEO_DIP_SHP_WIN:
		*read_func = (MPI_READ_S)APP_DIP_getWinShp;
		*write_func = (MPI_WRITE_S)APP_DIP_setWinShp;
		break;
	case AGTX_ITEM_VIDEO_DIP_NR_WIN:
		*read_func = (MPI_READ_S)APP_DIP_getWinNr;
		*write_func = (MPI_WRITE_S)APP_DIP_setWinNr;
		break;
	case AGTX_ITEM_VIDEO_DIP_ENH:
		*read_func = (MPI_READ_S)APP_DIP_getEnh;
		*write_func = (MPI_WRITE_S)APP_DIP_setEnh;
		break;
	case AGTX_ITEM_VIDEO_DIP_CORING:
		*read_func = (MPI_READ_S)APP_DIP_getCoring;
		*write_func = (MPI_WRITE_S)APP_DIP_setCoring;
		break;
	case AGTX_ITEM_VIDEO_DIP_FCS:
		*read_func = (MPI_READ_S)APP_DIP_getFcs;
		*write_func = (MPI_WRITE_S)APP_DIP_setFcs;
		break;
	case AGTX_ITEM_VIDEO_DIP_DHZ:
		*read_func = (MPI_READ_S)APP_DIP_getDhz;
		*write_func = (MPI_WRITE_S)APP_DIP_setDhz;
		break;
	case AGTX_ITEM_VIDEO_DIP_HDR_SYNTH:
		*read_func = (MPI_READ_S)APP_DIP_getHdrSynth;
		*write_func = (MPI_WRITE_S)APP_DIP_setHdrSynth;
		break;
	case AGTX_ITEM_VIDEO_DIP_STAT:
		*read_func = (MPI_READ_S)APP_DIP_getStat;
		*write_func = (MPI_WRITE_S)APP_DIP_setStat;
		break;
	case AGTX_ITEM_VIDEO_DIP_EXP_INFO:
		*read_func = (MPI_READ_S)APP_DIP_getExposureInfo;
		*write_func = (MPI_WRITE_S)NULL;
		break;
	case AGTX_ITEM_VIDEO_DIP_WB_INFO:
		*read_func = (MPI_READ_S)APP_DIP_getWhiteBalanceInfo;
		*write_func = (MPI_WRITE_S)NULL;
		break;
	case AGTX_ITEM_VIDEO_DIP_TE_INFO:
		*read_func = (MPI_READ_S)APP_DIP_getTeInfo;
		*write_func = (MPI_WRITE_S)NULL;
		break;
	case AGTX_ITEM_VIDEO_VIEW_TYPE:
		*read_func = (MPI_READ_S)APP_VIEW_getWinViewType;
		*write_func = (MPI_WRITE_S)NULL;
		break;
	default:
		ERR("Unsupport cmd_idx %d\n", cmd_id);
		return -1;
	}

	return 0;
}

static void print_debug(char *buffer)
{
	DBG_MED("\n\n\n--------------------\n");
	DBG_MED("DBG:|%s|", buffer);
	DBG_MED("\n--------------------\n\n\n");
}

char *unicorn_json_add_key_int(char *buffer, char *dKey, int val, int strLen)
{
	json_object *json_obj, *tmp_obj;
	// tokenize buffer
	struct json_tokener *tok = json_tokener_new();
	char json_buf[JSON_STR_LEN];
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	tmp_obj = json_object_new_int(val);
	if (!tmp_obj) {
		ERR("create int json object");
	}
	json_object_object_add(json_obj, dKey, tmp_obj);
	bzero(json_buf, JSON_STR_LEN);
	strcpy(json_buf, json_object_get_string(json_obj));
	strcpy(buffer, json_buf);
	//FIXME: Does it work?
	json_object_put(json_obj);
	json_tokener_free(tok);
	return buffer;
}

char *unicorn_json_add_key_string(char *buffer, char *dKey, char *val, int strLen)
{
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	char json_buf[JSON_STR_LEN];
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	print_debug(json_buf);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	tmp_obj = json_object_new_string(val);
	if (!tmp_obj) {
		ERR("create string json object");
	}
	json_object_object_add(json_obj, dKey, tmp_obj);
	bzero(json_buf, JSON_STR_LEN);
	strcpy(json_buf, json_object_get_string(json_obj));
	strcpy(buffer, json_buf);
	//FIXME: Does it work?
	//
	json_object_put(json_obj);
	json_tokener_free(tok);
	return buffer;
}

void unicorn_json_delete_key(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj;
	struct json_tokener *tok = json_tokener_new();
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	json_object_object_del(json_obj, dKey);
	bzero(buffer, JSON_STR_LEN);
	strcpy(buffer, json_object_get_string(json_obj));
	json_object_put(json_obj);
	json_tokener_free(tok);
}

int unicorn_json_get_int(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	int rval;
	// FIXME: Use memset instead of bzero
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
			//ERR("[%s : %d] \n",dKey, rval);
		} else {
			rval = -99; //since -ve return  are possible we select -99 to avoid conflict with spec
		}
	} else { //the key may not exist
		rval = -99;
	}
	json_object_put(json_obj);
	json_object_put(tmp_obj);
	json_tokener_free(tok);
	return rval;
}

char *unicorn_json_get_string(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	char *retString = NULL;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if ((json_object_object_get_ex(json_obj, dKey, &tmp_obj)) &&
	    (json_object_get_type(tmp_obj) == json_type_string)) {
		retString = malloc(JSON_STR_LEN);
		if (retString) {
			strcpy(retString, json_object_get_string(tmp_obj));
		}
		//ERR(" [%s : %s] \n",dKey, retString);
	}
	json_object_put(json_obj);
	json_object_put(tmp_obj);
	json_tokener_free(tok);
	return retString;
}

char *unicorn_json_get_object(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	char *ret_obj = NULL;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if ((json_object_object_get_ex(json_obj, dKey, &tmp_obj)) &&
	    (json_object_get_type(tmp_obj) == json_type_object)) {
		ret_obj = malloc(JSON_STR_LEN);
		if (ret_obj) {
			strcpy(ret_obj, json_object_to_json_string(tmp_obj));
		}
	}
	json_object_put(json_obj);
	json_object_put(tmp_obj);
	json_tokener_free(tok);
	return ret_obj;
}

int unicorn_json_validation(char *buffer, int strLen)
{
	char json_buf[JSON_STR_LEN];
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;
	int rval;

	if (strLen > 0) {
		bzero(json_buf, 0);
		strcpy(json_buf, (char *)buffer);
		json_tokener_parse_ex(tok, json_buf, strLen);
	} else {
		json_tokener_parse_ex(tok, json_buf, strLen);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr != json_tokener_success) {
		ERR(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		rval = -1;
	} else if (jerr == json_tokener_success) {
		rval = 0;
	}

	json_tokener_free(tok);
	return rval;
}

// {'a';1,'arrKey':[{'subKey': 123, 'subKey2': 1123},{'subKey': 123, 'subKey2': 1123}] }
int unicorn_json_get_int_from_array(char *buffer, char *arrKey, char *subKey, int arrIdx, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	json_object *json_obj_n, *tmp_obj_n;
	struct json_tokener *tok = json_tokener_new();
	int rval;
	DBG_MED("\nbuffer = %s\n", buffer);
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, arrKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
		} else if (json_object_get_type(tmp_obj) == json_type_array) {
			rval = json_object_array_length(tmp_obj);
			json_obj_n = json_object_array_get_idx(tmp_obj, arrIdx);
			if (json_object_object_get_ex(json_obj_n, subKey, &tmp_obj_n)) {
				if (json_object_get_type(tmp_obj_n) == json_type_int) {
					rval = json_object_get_int(tmp_obj_n);
				} else {
					ERR("\nERROR 1\n");
					rval = -99;
				}
			} else {
				ERR("\nERROR 2\n");
				rval = -99;
			}
			json_object_put(json_obj_n);
			json_object_put(tmp_obj_n);
		} else {
			ERR("\nERROR 3\n");
			rval = -99;
		}
	} else { //the key may not exist
		ERR("\nERROR 4 : arrKey = %s does not exist!\n", arrKey);
		rval = -99;
	}
	//DBG_MED( "\nrval from unicorn_json_get_int_from_array is %d\n", rval);
	//
	json_object_put(json_obj);
	json_object_put(tmp_obj);
	json_tokener_free(tok);
	return rval;
}

// {'a';1,'arrKey':[{'subKey': 123, 'subKey2': 1123},{'subKey': 123, 'subKey2': 1123}] }
char *unicorn_json_get_object_from_array(char *buffer, char *arrKey, int arrIdx, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	json_object *json_obj_n;
	struct json_tokener *tok = json_tokener_new();
	char *ret_obj = NULL;
	DBG_MED("\nbuffer = %s\n", buffer);
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, arrKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_array) {
			json_obj_n = json_object_array_get_idx(tmp_obj, arrIdx);
			ret_obj = malloc(JSON_STR_LEN);
			if (ret_obj) {
				strcpy(ret_obj, json_object_to_json_string(json_obj_n));
			}
		} else {
			ERR("\nERROR 3\n");
			ret_obj = NULL;
		}
	} else { //the key may not exist
		ERR("\nERROR 4 : arrKey = %s does not exist!\n", arrKey);
		ret_obj = NULL;
	}
	//DBG_MED( "\nrval from unicorn_json_get_object_from_array is %s\n", ret_obj);
	json_object_put(json_obj);
	json_object_put(tmp_obj);
	json_tokener_free(tok);
	return ret_obj;
}

// {'a';1,'arrKey':[{},{},{}] }
int unicorn_json_get_array_length(char *buffer, char *arrKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	int rval;
	DBG_MED("\nbuffer = %s\n", buffer);
	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	if (json_object_object_get_ex(json_obj, arrKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
		} else if (json_object_get_type(tmp_obj) == json_type_array) {
			rval = json_object_array_length(tmp_obj);
		} else {
			ERR("\nERROR 3\n");
			rval = -99;
		}
	} else { //the key may not exist
		ERR("\nERROR 4 : arrKey = %s does not exist!\n", arrKey);
		rval = -99;
	}
	DBG_MED("\nrval from unicorn_json_get_array_length is %d\n", rval);
	json_object_put(json_obj);
	json_object_put(tmp_obj);
	json_tokener_free(tok);
	return rval;
}

void removeSpaces(char *src)
{
	char *i = src;
	char *j = src;
	while (*j != 0) {
		*i = *j++;
		if (!isspace(*i))
			i++;
	}
	*i = 0;
}

/* Requires: src be null-terminated with single message in it
 *
 * Modifies: src
 *
 * Effects:  removes all leading and trailing spaces in src and
 *           returns a substring that is a single message
 *
 */

char *extractSingleMessage(char *src, int nbytes)
{
	char *end;
	while (isspace((unsigned char)*src))
		++src;
	end = src + nbytes - 1;
	while (end > src && isspace((unsigned char)*end))
		--end;
	end[1] = '\0';
	return src;
}

/* Requires:  src be null-terminated
 *
 * Modifies:  first integer in src will be replaced by spaces
 *
 * Effects:   returns the first number encountered in src
 *
 */
int parseFirstNum(char *src)
{
	char *ptr = src;
	int flag = 0;
	int idx = 0;
	char num[MAX_ACTION_LEN + 1] = {};
	while (*ptr) {
		if (isdigit(*ptr)) {
			num[idx] = *ptr;
			*ptr = ' ';
			++idx;
			flag = 1;
		} else {
			if (flag)
				break;
		}
		++ptr;
	}
	return atoi(num);
}

/*
 * Requires: src has been preprocessed and is now in the
 *           format of src = 'string1 string2 ',where string1
 *           is filePath and string2 is the file size
 *
 * Modifies: replace file size, or string2, in src with spaces
 *           for later processing
 *
 * Effects:  returns file size
 */
long parseFileSize(char *src)
{
	long size = 0;
	int prev_space = 1, group = 0;
	char *ptr = src, *end;
	while (*ptr) {
		if (isspace(*ptr)) {
			prev_space = 1;
			++ptr;
		} else {
			if (prev_space)
				++group;
			if (group == 2) {
				end = ptr + strlen(ptr) - 1;
				end[1] = '\0';
				break;
			}
			prev_space = 0;
			++ptr;
		}
	}
	size = atol(ptr);
	for (int i = 0; (unsigned)i < strlen(ptr); ++i)
		*(ptr + i) = ' ';
	return size;
}

/*
 * Requires: src must contain the sentinel character
 *
 * Modifies: src
 *
 * Effects: truncates src after and including sentinel
 *
 */
void truncateMessage(char *src, const char sentinel, int nbytes)
{
	char *end = src + nbytes - 1;
	for (; *end != sentinel; end--) {
	}
	end[0] = '\0';
}

int findSpase(char *src)
{
	int c = 0;
	while (src[c] != '\0') {
		if (src[c] == ' ') {
			return 1;
		} else {
			c++;
		}
	}

	return -1;
}

int executeSystemCommand(const char *params)
{
	int ret;

	ret = system(params); // system return 0 as success

	if (ret == -1) {
		ERR("System error!\n");
	} else {
		if (WIFEXITED(ret)) {
			if (WEXITSTATUS(ret) == 0) {
				DBG_MED("(%s) be excuted successfully.\n", params);
			} else {
				ERR("Run cmd fail and exit code is %d (%s)!\n", WEXITSTATUS(ret), params);
			}
		} else {
			ERR("exit status is %d (%s)!\n", WEXITSTATUS(ret), params);
		}
	}

	return ret;
}

int captureStdoutStr(const char *cmd, char *stdout)
{
	FILE *fp;
	int ret = 0, status = -1;
	char str[MAX_DATA_SIZE_BYTES] = { 0 };

	fp = popen(cmd, "r");
	if (fp == NULL) {
		pclose(fp);
		return -1;
	}

	if (fgets(str, MAX_DATA_SIZE_BYTES, fp) == NULL) {
		sprintf(stdout, "%d", status);
		ret = -1;
	} else {
		char *token = strtok(str, "\n");
		sprintf(stdout, "%s", token);
	}

	pclose(fp);
	return ret;
}

int accessUbootReg(const char *key, const char *reg, char *buf, const char *type)
{
	int ret = 0, reg_len = 0;
	char ret_str[128] = { 0 };
	char read_cmd[128] = { 0 };
	char write_cmd[128] = { 0 };
	sprintf(read_cmd, "fw_printenv %s", key);
	sprintf(write_cmd, "fw_setenv %s %s", key, reg);

	if (strcmp(type, "READ") == 0) {
		ret = captureStdoutStr(read_cmd, ret_str);
		if (ret == -1) {
			sprintf(buf, "%s", ret_str);
		} else {
			reg_len = strlen(ret_str) - strlen(key) - 1;
			snprintf(buf, reg_len, ret_str + strlen(key) + 1);
		}
	} else if (strcmp(type, "WRITE") == 0) {
		ret = executeSystemCommand(write_cmd);
		sprintf(buf, "%d", ret);
	}

	return ret;
}

int getCalibFilePath(char *type, char *file_path)
{
	if (strcmp(type, "STITCH_CONF") == 0) {
		sprintf(file_path, "%s", STITCH_CONF_JSON_FILE);
	} else if (strcmp(type, "COLOR_DIFF") == 0) {
		sprintf(file_path, "%s", COLOR_DIFF_JSON_FILE);
	} else if (strcmp(type, "PANORAMA") == 0) {
		sprintf(file_path, "%s", PANORAMA_JSON_FILE);
	} else if (strcmp(type, "ANTI_FLICKER") == 0) {
		sprintf(file_path, "%s", ANTI_FLICKER_JSON_FILE);
	} else if (strcmp(type, "LIGHT_SENSOR") == 0) {
		sprintf(file_path, "%s", LIGHT_SENSOR_JSON_FILE);
	} else if (strcmp(type, "STITCH_CONF_UPD") == 0) {
		sprintf(file_path, "%s", STITCH_CONF_UPD_FILE);
	} else if (strcmp(type, "COLOR_DIFF_UPD") == 0) {
		sprintf(file_path, "%s", COLOR_DIFF_UPD_FILE);
	} else if (strcmp(type, "PANORAMA_UPD") == 0) {
		sprintf(file_path, "%s", PANORAMA_UPD_FILE);
	} else if (strcmp(type, "ANTI_FLICKER_UPD") == 0) {
		sprintf(file_path, "%s", ANTI_FLICKER_UPD_FILE);
	} else if (strcmp(type, "LIGHT_SENSOR_UPD") == 0) {
		sprintf(file_path, "%s", LIGHT_SENSOR_UPD_FILE);
	} else {
		DBG_MED("No this type %s for file path!", type);
		return -1;
	}

	return 0;
}

int writeJsonDataToFile(char *file, char *data)
{
	FILE *fp = fopen(file, "w");
	if (fp == NULL) {
		DBG_MED("File %s not found!\n", file);
		fclose(fp);
		return -1;
	}

	fwrite(data, 1, strlen(data), fp);

	fclose(fp);

	return 0;
}

/*
	string: "[a,b,c]"
	str_array: ["a","b","c"]

	ex:
	int i = 0, num = 3;
	char **str_array = malloc(num * sizeof(char*));
    for(i = 0; i < num; i++)
        str_array[i] = malloc(10 * sizeof(char*));

	parseStringArray(string, str_array);


	x = atoi(str_array[0]);
	y = atoi(str_array[1]);
	radius = atoi(str_array[2]);

	for (i = 0; i < num; i++)
		free(str_array[i]);
	free(str_array);
*/
int parseStringArray(char *string, char **str_array, int array_len, int single_size)
{
	int i = 0, cnt = 0;
	int len = (strlen(string) - 2 + 1); // remove "[", "]" + "\0"

	char *substr = malloc(len * sizeof(char *));
	memset(substr, '\0', len);

	char **substr_array = malloc(array_len * sizeof(char *));
	for (i = 0; i < array_len; i++)
		substr_array[i] = malloc(single_size * sizeof(char *));

	strncpy(substr, string + 1, len - 1);
	DBG_MED("substr: %s\n", substr);

	// Returns first token
	char *token = strtok(substr, ",");
	while (token != NULL) {
		strcpy(substr_array[cnt], token);
		DBG_MED("A|%s|", substr_array[cnt]);
		cnt += 1;
		token = strtok(NULL, ",");
	}

	// Keep printing tokens while one of the
	// delimiters present in substr[].
	for (i = 0; i < array_len; i++) {
		token = strtok(substr_array[i], "'");
		while (token != NULL) {
			DBG_MED("token|%s|", token);
			if (strlen(token) > 2) {
				strcpy(str_array[i], token);
				DBG_MED("B|%s|", str_array[i]);
			}
			token = strtok(NULL, "'");
		}
	}

	for (i = 0; i < array_len; i++)
		free(substr_array[i]);
	free(substr_array);
	free(substr);
	return 0;
}

/*
	string: "[10,20,60]"
	str_array: [10,20,60]
*/
int parseIntArray(char *string, int *int_array)
{
	int cnt = 0;
	int len = (strlen(string) - 2 + 1); // remove "[", "]" + "\0"

	char *substr = malloc(len * sizeof(char *));
	memset(substr, '\0', len);

	strncpy(substr, string + 1, len - 1);
	DBG_MED("substr: %s\n", substr);

	// Returns first token
	char *token = strtok(substr, ",");

	// Keep printing tokens while one of the
	// delimiters present in substr[].
	while (token != NULL) {
		DBG_MED("|%d|\n", atoi(token));
		int_array[cnt++] = atoi(token);
		token = strtok(NULL, ",");
	}

	free(substr);
	return 0;
}

int isExecutableMode(char *executable_mode_list)
{
	int ret = 0, execuable_ret = -1;
	char current_mode[16] = { '\0' };

	ret = captureStdoutStr("cat /usrdata/mode", current_mode);
	if (ret == 0) {
		DBG_MED("current_mode %s, executable_mode_list %s ", current_mode, executable_mode_list);
		if (strcmp(executable_mode_list, "[]") == 0) {
			execuable_ret = 0;
		} else {
			int i = 0, mode_num = 3; // ["factory", "develop", "user"]
			int single_size = 16;
			char **str_array = malloc(mode_num * sizeof(char *));

			for (i = 0; i < mode_num; i++)
				str_array[i] = malloc(single_size * sizeof(char *));

			parseStringArray(executable_mode_list, str_array, mode_num, single_size);

			for (i = 0; i < mode_num; i++) {
				DBG_MED("str_array[%d] |%s| |%s| %s", i, str_array[i], current_mode,
				        strstr(str_array[i], current_mode));
				if (strstr(str_array[i], current_mode) != NULL) {
					execuable_ret = 0;
				}
			}

			for (i = 0; i < mode_num; i++)
				free(str_array[i]);
			free(str_array);
		}
	} else {
		sprintf(current_mode, "None");
	}

	DBG_MED("#--- current_mode %s, execuable_ret %d ---#\n", current_mode, execuable_ret);

	return execuable_ret;
}

int folderCtrl(char *jStr)
{
	int ret = 0;
	char *ctrl, *path, *name;
	char cmd[128];
	char stdout[256];

	ctrl = unicorn_json_get_string(jStr, "ctrl", strlen(jStr));
	path = unicorn_json_get_string(jStr, "path", strlen(jStr));
	name = unicorn_json_get_string(jStr, "name", strlen(jStr));

	if (ctrl == NULL || path == NULL || name == NULL) {
		ERR("Error: Unable to get ctrl, path or name, %s\n", jStr);
		ret = -1;
	} else if (strcmp(ctrl, "ISEXIST") == 0) {
		sprintf(cmd, "ls %s | awk '/%s/{print substr($1, 1)}'", path, name);
		ret = captureStdoutStr((const char *)cmd, stdout);
		if (strstr(stdout, name) != NULL) {
			ret = 1; //exist
		} else {
			ret = 0; //not exist
		}
	} else if (strcmp(ctrl, "REMOVE") == 0) {
		sprintf(cmd, "rm -R %s/%s", path, name);
		ret = executeSystemCommand(cmd);
	} else if (strcmp(ctrl, "MKDIR") == 0) {
		sprintf(cmd, "mkdir -p %s/%s", path, name);
		ret = executeSystemCommand(cmd);
	} else {
		ERR("There is no %s ctrl for control ir cut.", ctrl);
		ret = -1;
	}

	if (ctrl) {
		free(ctrl);
	}
	if (path) {
		free(path);
	}
	if (name) {
		free(name);
	}
	return ret;
}

int collectData(char *curDir, char *jStr)
{
	int ret = 0;
	char cmd[256] = { 0 };

	if (curDir == NULL || jStr == NULL) {
		ERR("collect path error\n");
		return -1;
	}

	snprintf(cmd, 256, "\"%s/collect_data.sh\" \"%s/\" %s", curDir, curDir, jStr);
	ret = executeSystemCommand(cmd);
	if (ret != 0) {
		ERR("Collect data failed\n");
		return -1;
	}

	snprintf(cmd, 256, "tar -cvf \"%s/%s.tar\" -C \"%s/%s/\" .", curDir, jStr, curDir, jStr);
	ret = executeSystemCommand(cmd);
	if (ret != 0) {
		ERR("Tar collect data failed\n");
		return -1;
	}

	snprintf(cmd, 256, "rm -rf \"%s/%s\"", curDir, jStr);
	ret = executeSystemCommand(cmd);
	if (ret != 0) {
		ERR("Remove %s directory failed\n", jStr);
		return -1;
	}

	return ret;
}

int getDipExtend(char *params, char **jStr, char **buf)
{
	int request_path_idx = 0;
	if (params == NULL) {
		ERR("Invalid argument.\n");
		return 0;
	}
	request_path_idx = unicorn_json_get_int(params, "video_path_idx", strlen(params));
	MPI_PATH path_idx = MPI_INPUT_PATH(0, request_path_idx < 0 ? 0 : request_path_idx);
	char filepath[50];
	char compress_filepath[50];
	int count = 0;
	do {
		sprintf(filepath, "/tmp/temp_%d.ini", count);
		sprintf(compress_filepath, "/tmp/temp_%d.ini.gz", count);
		count++;
	} while (access(filepath, F_OK) != -1 && access(compress_filepath, F_OK) != -1);

	DBG_MED("path: %s\n", filepath);
	if (MPI_getDipExtendFile(path_idx, filepath) == MPI_FAILURE) {
		return 0;
	}
	DBG_MED("Mpi_getDipExtendFile success\n");
	char command[50];
	sprintf(command, "busybox gzip %s", filepath);
	system(command);
	if (access(compress_filepath, F_OK) == -1) {
		return -1;
	}
	DBG_MED("success compress file\n");
	int fsize = extractFileData(buf, compress_filepath);
	if (fsize < 1 || *buf == NULL) {
		ERR("Fail to read dip_extend file");
		return 0;
	}
	DBG_MED("Get file success\n");
	struct json_object *jobj = json_object_new_object();
	struct json_object *jstLen = json_object_new_int(fsize);
	DBG_MED("Gs : %d\n", strlen(*buf));
	json_object_object_add(jobj, "Content", jstLen);
	const char *result = json_object_get_string(jobj);
	*jStr = malloc(strlen(result) + 1);
	int ret = snprintf(*jStr, strlen(result) + 1, "%s", result);
	json_object_put(jobj);
	remove(compress_filepath);
	return ret;
}

int setDipExtend(char *jStr, int socketfd)
{
	int request_path_idx = unicorn_json_get_int(jStr, "video_path_idx", strlen(jStr));
	MPI_PATH path_idx = MPI_INPUT_PATH(0, request_path_idx < 0 ? 0 : request_path_idx);
	char filepath[50];
	char decompress_filepath[50];
	int count = 0;
	do {
		sprintf(filepath, "/tmp/temp_%d.ini.gz", count);
		sprintf(decompress_filepath, "/tmp/temp_%d.ini", count);
		count++;
	} while (access(filepath, F_OK) != -1 && access(decompress_filepath, F_OK) != -1);

	DBG_MED("path: %s\n", filepath);
	long fSize = unicorn_json_get_int(jStr, "Content", strlen(jStr));
	int ret = recvFile(socketfd, filepath, fSize, false);
	if (ret == ACT_FAILURE) {
		ERR("fail to recieve file\n");
		return -1;
	}
	DBG_MED("success recieve file\n");

	char command[50];
	sprintf(command, "busybox gzip -d %s", filepath);
	system(command);
	if (access(decompress_filepath, F_OK) == -1) {
		return -1;
	}
	DBG_MED("success decompress file\n");
	if (MPI_setDipExtendFile(path_idx, decompress_filepath) == MPI_FAILURE) {
		return -1;
	}
	remove(decompress_filepath);
	return 0;
}
static int extractFileData(char **buf, char *filepath)
{
	FILE *f = fopen(filepath, "rb");
	if (f == NULL) {
		ERR("File can not be open\n");
		return -1;
	}
	fseek(f, 0, SEEK_END);
	long length = ftell(f);
	fseek(f, 0, SEEK_SET);
	*buf = malloc(length + 1);
	if (*buf) {
		fread(*buf, 1, length, f);
		(*buf)[length] = '\0';
	}
	fclose(f);
	return length;
}
