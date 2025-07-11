/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/*
 * Caution: Include mpi_base_types.h first to overrule TRUE/FALSE definition of TUYA IPC SDK.
 */
#include "mpi_base_types.h"
#include "agtx_types.h"

#include "cm_video_strm_conf.h"
#include "cm_osd_conf.h"
#include "cm_iva_od_conf.h"
#include "cm_iva_md_conf.h"
#include "cm_img_pref.h"
#include "cm_adv_img_pref.h"
#include "cm_video_ptz_conf.h"
#include "agtx_cmd.h"
#include "cm_siren_conf.h"
#include "cm_voice_conf.h"
#include "cm_video_layout_conf.h"
#include "cm_local_record_conf.h"

#include "tuya_ipc_define.h"
#include "tuya_ipc_dp_utils.h"
#include "tuya_ipc_dp_handler.h"
#include "tuya_ipc_stream_storage.h"
#if defined(WIFI_GW) && (WIFI_GW == 1)
#include "wifi_hwl.h"
#endif
#include "tuya_ipc_sd_demo.h"
#include "tuya_ipc_system_control_demo.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_motion_detect_demo.h"
#include "tuya_cloud_com_defs.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/reboot.h>
#include <errno.h>

#define COMB(a, b) a##b

#define AG_DEBUG(fmt, args...) PR_DEBUG(fmt, ##args)
#define JSON_STR_LEN 8192

#define _STR(s) #s
#define STR(s) _STR(s)

//Globals:
extern int cmdTransFD;
extern int g_master_id;
STATIC INT_T s_sd_format_progress = 0;
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
TUYA_AG_CONF_S g_ag_conf = { { 0 } };
#pragma GCC diagnostic pop
extern void IPC_APP_Live_Video_Sub_En(int enable);

#define AV_MAIN_RESTART_LATENCY (3*1000*1000)

/* FIXME: workaround for #11606 #11650 */
#define TUYA_IVA_OD_SET_DEFAULT_PARAM(od)   \
	do {                                \
		(od)->en_shake_det = 1;     \
		(od)->od_qual = 99;         \
		(od)->od_sen = 99;          \
		(od)->od_size_th = 6;       \
		(od)->od_track_refine = 86; \
		(od)->video_chn_idx = 0;    \
	} while (0)

#define AG_GET_CONF() (&g_ag_conf)

StateDataPoint requested;
StateDataPoint acknowledged;
StateDataPoint active;

#define PTZ_DIR_NONE -1
int g_ptz_direction = PTZ_DIR_NONE;
int g_ptz_active_direction = PTZ_DIR_NONE;
int g_ptz_count = 0;

#define ZOOM_NONE -1
int g_zoom = ZOOM_NONE;
int g_active_zoom = ZOOM_NONE;
int g_zoom_count = 0;

static char * dp_video_layout_str[] = { "0", "1", "2" };

int AG_Get_Conf(TUYA_AG_CONF_S **conf)
{
	*conf = AG_GET_CONF();

	return 0;
}

static int aux_parse_cc_config(int cmdId, void *data, struct json_object *json_obj)
{
	if (cmdId == AGTX_CMD_VIDEO_STRM_CONF) {
		parse_video_strm_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VIDEO_PTZ_CONF) {
		parse_video_ptz_conf(data, json_obj);
	} /*else if (cmdId == AGTX_CMD_VIDEO_DEV_CONF) {
		parse_video_dev_conf(data, json_obj);
	} */ else if (cmdId == AGTX_CMD_MD_CONF) {
		parse_iva_md_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_IMG_PREF) {
		parse_img_pref(data, json_obj);
	} else if (cmdId == AGTX_CMD_ADV_IMG_PREF) {
		parse_adv_img_pref(data, json_obj);
	} /*else if (cmdId == AGTX_CMD_AUDIO_CONF) {
		parse_audio_conf(data, json_obj);
	} */ else if (cmdId == AGTX_CMD_OSD_CONF) {
		parse_osd_conf(data, json_obj);
	}else if (cmdId == AGTX_CMD_OD_CONF) {
		parse_iva_od_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_LOCAL_RECORD_CONF) {
		parse_local_record_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VOICE_CONF) {
		parse_voice_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_SIREN_CONF) {
		parse_siren_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_VIDEO_LAYOUT_CONF) {
		parse_layout_conf(data, json_obj);
	} else {
		PR_ERR("Unknown cmdId %d\n", cmdId);
		return -1;
	}

	return 0;
}

static int aux_comp_cc_config(int cmdId, void *data, struct json_object *json_obj)
{
	if (cmdId == AGTX_CMD_VIDEO_STRM_CONF) {
		comp_video_strm_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_VIDEO_PTZ_CONF) {
		comp_video_ptz_conf(json_obj, data);
		//	} else if (cmdId == AGTX_CMD_VIDEO_DEV_CONF) {
		//		parse_video_dev_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_MD_CONF) {
		comp_iva_md_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_IMG_PREF) {
		comp_img_pref(json_obj, data);
	} else if (cmdId == AGTX_CMD_ADV_IMG_PREF) {
		comp_adv_img_pref(json_obj, data);
		//	} else if (cmdId == AGTX_CMD_TD_CONF) {
		//		parse_iva_td_conf(data, json_obj);
		//	} else if (cmdId == AGTX_CMD_AUDIO_CONF) {
		//		parse_audio_conf(data, json_obj);
	} else if (cmdId == AGTX_CMD_OSD_CONF) {
		comp_osd_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_OD_CONF) {
		comp_iva_od_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_LOCAL_RECORD_CONF) {
		comp_local_record_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_VOICE_CONF) {
		comp_voice_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_SIREN_CONF) {
		comp_siren_conf(json_obj, data);
	} else if (cmdId == AGTX_CMD_VIDEO_LAYOUT_CONF) {
		comp_layout_conf(json_obj, data);
	} else {
		PR_ERR("Unknown cmdId %d\n", cmdId);
		return -1;
	}

	return 0;
}

static int getCCReturn(char *str)
{
	int ret = -1;
	/* retry 2 times */
	int cnt = 2;

	while (cnt--) {
		ret = read(cmdTransFD, str, JSON_STR_LEN);

		if (ret < 0) {
			PR_NOTICE("getCCReturn: ret = %d, errno = %d(%s)\n", ret, errno, strerror(errno));
			continue;
		} else if (ret == 0) {
			PR_NOTICE("getCCReturn == 0\n");
			continue;
		} else {
			PR_NOTICE("getCCReturn: ret = %d, str = %s\n", ret, str);
			return 0;
		}
	}

	return -1;
}

void AG_getCCReply(char *text)
{
	int ret = -1;

	while (1) {
		ret = read(cmdTransFD, text, 10240);

		if (ret < 0) {
			PR_NOTICE("ret = %d, errno = %d(%s)\n", ret, errno, strerror(errno));
			break;
		} else if (ret == 0) {
			PR_NOTICE("AG_getCCReply == 0\n");
			break;
		} else {
			PR_NOTICE("AG_getCCReply > 0\n");
			break;
		}
	}
}

//validate the cc read string is in Json format
int aux_json_validation(char *buffer, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;
	int rval = 0;

	if (strLen > 0) {
		bzero(json_buf, 0);
		strcpy(json_buf, (char *)buffer);
		json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	} else {
		json_obj = json_tokener_parse_ex(tok, json_buf, strLen);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_continue) {
		json_obj = json_tokener_parse_ex(tok, " ", -1);
		if (json_tokener_get_error(tok) != json_tokener_success)
			json_obj = NULL;
	} else if (jerr != json_tokener_success) {
		PR_ERR(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		rval = -1;
	} else if (jerr == json_tokener_success) {
		rval = 0;
	}
	if (json_obj != NULL)
		json_object_put(json_obj); //Decrement the ref count and free if it reaches zero

	json_tokener_free(tok);
	return rval;
}

//JSON: Get int val for given Key
int aux_json_get_int(char *buffer, char *dKey, int strLen)
{
	char json_buf[JSON_STR_LEN];
	json_object *json_obj, *tmp_obj;
	struct json_tokener *tok = json_tokener_new();
	int rval;

	bzero(json_buf, 0);
	strcpy(json_buf, buffer);
	json_obj = json_tokener_parse_ex(tok, json_buf, strLen);

	if (json_object_object_get_ex(json_obj, dKey, &tmp_obj)) {
		if (json_object_get_type(tmp_obj) == json_type_int) {
			rval = json_object_get_int(tmp_obj);
		} else {
			rval = -99; //since -ve return  are possible we select -99 to avoid conflict with spec
		}
	} else { //the key may not exist
		rval = -99;
	}
	if (json_obj != NULL)
		json_object_put(json_obj);
	if (tmp_obj != NULL)
		json_object_put(tmp_obj);

	json_tokener_free(tok);
	return rval;
}

static int aux_get_cc_config(int cmdId, void *data)
{
	int len = 0;
	int rval = 0;
	int jsonRetLen = 0;
	char json_buf[JSON_STR_LEN] = { 0 };
	char ccRetStr[JSON_STR_LEN] = { 0 };
	enum json_tokener_error jerr;
	struct json_object *json_obj = NULL;
	struct json_tokener *tok = NULL;

	sprintf(json_buf, "{'master_id': %d, 'cmd_id': %d ,'cmd_type': 'get'}\n", g_master_id, cmdId);
	AG_DEBUG("CMD: %s\n", json_buf);

	//CC Send
	//	PR_DEBUG("Get CC command %s \n", json_buf);
	if (write(cmdTransFD, json_buf, strlen(json_buf)) < 0) {
		PR_ERR("failed to send message to command Translator \n");
		return -1;
	} else {
		bzero(ccRetStr, JSON_STR_LEN);
	}

	//CC recv
	if (getCCReturn(ccRetStr) != 0) {
		PR_ERR("Get cc config failed no data\n");
		return -1;
	}

	tok = json_tokener_new();

	len = strlen(ccRetStr);
	if (len > 0) {
		bzero(json_buf, 0);
		strcpy(json_buf, (char *)ccRetStr);
		json_obj = json_tokener_parse_ex(tok, json_buf, len);
	} else {
		json_obj = json_tokener_parse_ex(tok, json_buf, len);
	}

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_success) {
		if (aux_parse_cc_config(cmdId, data, json_obj) < 0) {
			// Need to do something?
		}

		if (json_obj != NULL) {
			json_object_put(json_obj); //Decrement the ref count and free if it reaches zero
		}

		//cc check return val to process furthe
		rval = aux_json_get_int(ccRetStr, "rval", strlen(ccRetStr));
		if (rval < 0) {
			PR_ERR(" CC response: FAIL  Return = %d \n", rval);
			jsonRetLen = -1;
		} else { // cc return success
			jsonRetLen = strlen(ccRetStr);
		}
	} else {
		PR_ERR(" JSON Tokener errNo: %d = %s \n\n", jerr, json_tokener_error_desc(jerr));
		PR_ERR(" CC response: Invalid Json Str : rval = %d \n", rval);
		jsonRetLen = -1;
	}

	json_tokener_free(tok);

	return jsonRetLen;
}

static int aux_set_cc_config(int cmdId, void *data)
{
	char jsonCmd[JSON_STR_LEN] = { 0 };
	char ccRetStr[JSON_STR_LEN] = { 0 };
	struct json_object *json_obj = NULL;
	struct json_object *tmp_obj = NULL;

	json_obj = json_object_new_object();
	if (!json_obj) {
		PR_ERR("Cannot create object\n");
		return -1;
	}

	if (aux_comp_cc_config(cmdId, data, json_obj) < 0) {
		// Need to do something?
	}

	tmp_obj = json_object_new_int(cmdId);
	if (tmp_obj) {
		json_object_object_add(json_obj, "cmd_id", tmp_obj);
	} else {
		PR_ERR("Cannot create %s object\n", "cmd_id");
	}

	tmp_obj = json_object_new_int(g_master_id);
	if (tmp_obj) {
		json_object_object_add(json_obj, "master_id", tmp_obj);
	} else {
		PR_ERR("Cannot create %s object\n", "master_id");
	}

	tmp_obj = json_object_new_string("set");
	if (tmp_obj) {
		json_object_object_add(json_obj, "cmd_type", tmp_obj);
	} else {
		PR_ERR("Cannot create %s object\n", "cmd_type");
	}

	sprintf(jsonCmd, "%s", json_object_to_json_string(json_obj));
	PR_INFO("jsonCmd %s\n", jsonCmd);
	if (json_obj) {
		json_object_put(json_obj);
	}

	if (write(cmdTransFD, jsonCmd, strlen(jsonCmd)) < 0) {
		PR_ERR("Failed to send message to command Translator \n");
	}

	if (getCCReturn(ccRetStr) != 0) {
		PR_ERR("ccRetStr %s\n", ccRetStr);
		return -1;
	}

	return 0;
}

INT_T AG_Collect_Conf(VOID)
{
	INT32 i = 0;
	INT32 cmd_num = 0;
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AG_CMD_DATA_S cmd[] = {
		{ AGTX_CMD_VIDEO_STRM_CONF, &conf->strm.data, &conf->strm.update },
		{ AGTX_CMD_MD_CONF, &conf->md.data, &conf->md.update },
		{ AGTX_CMD_LOCAL_RECORD_CONF, &conf->local_record.data, &conf->local_record.update },
		{ AGTX_CMD_VOICE_CONF, &conf->voice.data, &conf->voice.update },
		{ AGTX_CMD_SIREN_CONF, &conf->siren.data, &conf->siren.update },
		{ AGTX_CMD_VIDEO_LAYOUT_CONF, &conf->layout.data, &conf->layout.update },
		{ AGTX_CMD_VIDEO_PTZ_CONF, &conf->ptz.data, &conf->ptz.update },
		{ AGTX_CMD_IMG_PREF, &conf->img.data, &conf->img.update },
		//{AGTX_CMD_IMG_PREF, &conf->img.data},
		//{AGTX_CMD_ADV_IMG_PREF, &conf->pref.data},
		//{AGTX_CMD_MD_CONF, &conf->md.data},
		//{AGTX_CMD_OSD_CONF, &conf->osd.data},
	};

	cmd_num = sizeof(cmd) / sizeof(AG_CMD_DATA_S);
	for (i = 0; i < cmd_num; i++) {
		if (aux_get_cc_config(cmd[i].cmd, cmd[i].data) < 0) {
			PR_ERR("num %d cmd %d get config\n", i, cmd[i].cmd);
			return -1;
		}
		*cmd[i].update = 1;
	}

	PR_INFO("AG collect config success\n");
	return 0;
}

#ifdef TUYA_DP_DEVICE_RESTART
static pthread_t reboot_thread;

static void syncAndReboot(void) {
    usleep(3000 * 1000);
    sync();
    reboot(RB_AUTOBOOT);
}

VOID IPC_APP_restart_device(VOID)
{
    pthread_create(&reboot_thread, NULL, (void *)syncAndReboot, NULL);
    pthread_detach(reboot_thread);
}
#endif
#ifdef TUYA_DP_SLEEP_MODE
VOID IPC_APP_set_sleep_mode(BOOL_T sleep_mode)
{
	PR_INFO("set sleep_mode:%d \n", sleep_mode);
	//TODO
	/* sleep mode,BOOL type,true means sleep,false means wake */

	//writeConfigInt("tuya_sleep_mode", sleep_mode);
}

BOOL_T IPC_APP_get_sleep_mode(VOID)
{
	BOOL_T sleep_mode = FALSE; // = readConfigInt("tuya_sleep_mode");
	PR_INFO("curr sleep_mode:%d \n", sleep_mode);
	return sleep_mode;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_LIGHT
VOID IPC_APP_set_light_onoff(BOOL_T light_on_off)
{
	PR_INFO("set light_on_off:%d \n", light_on_off);
	//TODO
	/* Status indicator,BOOL type,true means on,false means off */

	//writeConfigInt("tuya_light_onoff", light_on_off);
}

BOOL_T IPC_APP_get_light_onoff(VOID)
{
	BOOL_T light_on_off = FALSE; // = readConfigInt("tuya_light_onoff");
	PR_INFO("curr light_on_off:%d \n", light_on_off);
	return light_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_FLIP
VOID IPC_APP_set_flip_onoff(BOOL_T flip_on_off)
{
	PR_INFO("set flip_on_off:%d \n", flip_on_off);
	/* flip state,BOOL type,true means inverse,false means normal */
	int flip = (flip_on_off == TRUE);
	int old_flip;

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_STRM_CONF_S *strm = &conf->strm.data;

	if (conf->strm.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, strm) < 0) {
			PR_ERR("Failed tp get flip conf.\n");
			return;
		}
		conf->strm.update = 1;
	}

	old_flip = strm->video_strm[0].flip_en;
	for (int i = 0; (AGTX_UINT32)i < strm->video_strm_cnt; i++) {
		strm->video_strm[i].flip_en = flip;
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_STRM_CONF, strm) < 0) {
		PR_ERR("Failed to set flip conf.\n");
		for (int i = 0; (AGTX_UINT32)i < strm->video_strm_cnt; i++) {
			strm->video_strm[i].flip_en = old_flip;
		}
		return;
	}
}

BOOL_T IPC_APP_get_flip_onoff(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_STRM_CONF_S *strm = &conf->strm.data;

	if (conf->strm.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, &strm) < 0) {
			PR_ERR("Failed tp get flip conf.\n");
			return FALSE;
		}
		conf->strm.update = 1;
	}

	BOOL_T flip_on_off;
	if (strm->video_strm[0].flip_en) {
		flip_on_off = TRUE;
	} else {
		flip_on_off = FALSE;
	}

	PR_INFO("curr flip_on_off:%d \n", flip_on_off);

	return flip_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_WATERMARK
#define OSD_TO_OD(enabled) ((enabled) ? FALSE : TRUE)
void loadDataPointBasicOsd(void)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_OSD_CONF_S *osd = &conf->osd.data;

	if (aux_get_cc_config(AGTX_CMD_OSD_CONF, osd) < 0) {
		PR_ERR("Failed to get AGTX_CMD_OSD_CONF.\n");
		return;
	}

	conf->osd.update = 1;

	active.dp104_basic_osd = IPC_APP_get_watermark_onoff();
	requested.dp104_basic_osd = active.dp104_basic_osd;
	acknowledged.dp104_basic_osd = active.dp104_basic_osd;

	PR_DEBUG("requested.dp104_basic_osd = %d.\n", requested.dp104_basic_osd);
	PR_DEBUG("acknowledged.dp104_basic_osd = %d.\n", acknowledged.dp104_basic_osd);
	PR_DEBUG("active.dp104_basic_osd = %d.\n", active.dp104_basic_osd);
}

void handleDataPointBasicOsd(TY_OBJ_DP_S *dp)
{
	int err;
	BOOL_T response;

	err = parseDataPointBool(dp, &requested.dp104_basic_osd);

	if(err) {
		response = active.dp104_basic_osd;
	} else {
		response = requested.dp104_basic_osd;
	}

	reportDataPointBool(TUYA_DP_WATERMARK, response);
}

int IPC_APP_set_watermark_onoff(BOOL_T enabled)
{
	PR_DEBUG("Setting OSD (%d)\n", enabled);

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_OSD_CONF_S *osd = &conf->osd.data;
	int old_enabled = 0;

	for (int chn = 0; chn < MAX_AGTX_OSD_CONF_S_STRM_SIZE; chn++) {
		for (int i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			if (osd->strm[chn].region[i].type == AGTX_OSD_TYPE_INFO) {
				old_enabled = osd->strm[chn].region[i].enabled;
				osd->strm[chn].region[i].enabled = enabled;
			}
		}
	}

	if (aux_set_cc_config(AGTX_CMD_OSD_CONF, osd) < 0) {
		PR_ERR("Failed to set AGTX_CMD_OSD_CONF.\n");

		for (int chn = 0; chn < MAX_AGTX_OSD_CONF_S_STRM_SIZE; chn++) {
			for (int i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
				if (osd->strm[chn].region[i].type == AGTX_OSD_TYPE_INFO) {
					osd->strm[chn].region[i].enabled = old_enabled;
				}
			}
		}
		return -1;
	}

	return 0;
}

BOOL_T IPC_APP_get_watermark_onoff(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_OSD_CONF_S *osd = &conf->osd.data;

	int chn = 0;
	int enable = 0;

	for (int i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
		if (osd->strm[chn].region[i].type == AGTX_OSD_TYPE_INFO) {
			enable = osd->strm[chn].region[i].enabled;
			break;
		}
	}

	BOOL_T watermark_on_off;
	if (enable) {
		watermark_on_off = TRUE;
	} else {
		watermark_on_off = FALSE;
	}

	PR_INFO("curr watermark_on_off:%d \n", watermark_on_off);
	return watermark_on_off;
}
#endif

#ifdef TUYA_DP_IPC_AUTO_SIREN
void loadDataPointIpcAutoSiren(void)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;

	if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
		PR_ERR("Failed to get AGTX_CMD_SIREN_CONF.\n");
		return;
	}

	conf->siren.update = 1;

	active.dp120_ipc_auto_siren = IPC_APP_get_ipc_auto_siren();
	requested.dp120_ipc_auto_siren = active.dp120_ipc_auto_siren;
	acknowledged.dp120_ipc_auto_siren = active.dp120_ipc_auto_siren;

	PR_DEBUG("requested.dp120_ipc_auto_siren = %d.\n", requested.dp120_ipc_auto_siren);
	PR_DEBUG("acknowledged.dp120_ipc_auto_siren = %d.\n", acknowledged.dp120_ipc_auto_siren);
	PR_DEBUG("active.dp120_ipc_auto_siren = %d.\n", active.dp120_ipc_auto_siren);
}

void handleDataPointIpcAutoSiren(TY_OBJ_DP_S *dp)
{
	int err;
	BOOL_T response;

	err = parseDataPointBool(dp, &requested.dp120_ipc_auto_siren);

	if(err) {
		response = active.dp120_ipc_auto_siren;
	} else {
		response = requested.dp120_ipc_auto_siren;
	}

	reportDataPointBool(TUYA_DP_IPC_AUTO_SIREN, response);
}

int IPC_APP_set_ipc_auto_siren(BOOL_T enabled)
{
	PR_DEBUG("Setting Auto Siren (%d)\n", enabled);

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;
	BOOL_T old_enabled = 0;

	if (conf->siren.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
			PR_ERR("Failed to get siren conf.\n");
			return -1;
		}
		conf->siren.update = 1;
	}

	old_enabled = siren->enabled ? 1 : 0;
	siren->enabled = enabled ? 1 : 0;

	if (aux_set_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
		PR_ERR("Failed to set AGTX_CMD_SIREN_CONF.\n");
		siren->enabled = old_enabled;
		return -1;
	}

	TUYA_APP_Update_Siren_Parameter();

	return 0;
}

BOOL_T IPC_APP_get_ipc_auto_siren(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;

	if(siren->enabled) {
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

#ifdef TUYA_DP_BRIGHTNESS
VOID IPC_APP_set_brightness(UINT_T brightness)
{
	PR_INFO("set brightness:%d \n", brightness);
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IMG_PREF_S *img = &conf->img.data;
	UINT_T old_brightness;

	if (conf->img.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
			PR_ERR("Failed to get brightness conf.\n");
			return;
		}
		conf->img.update = 1;
	}

	if (img->brightness == (AGTX_INT16)brightness) {
		return;
	}
	old_brightness = img->brightness;
	img->brightness = brightness;

	if (aux_set_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
		img->brightness = old_brightness;
		PR_ERR("Failed to set brightness conf.\n");
		return;
	}
}

UINT_T IPC_APP_get_brightness(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IMG_PREF_S *img = &conf->img.data;

	if (conf->img.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
			PR_ERR("Failed to get brightness conf.\n");
			return 0;
		}
		conf->img.update = 1;
	}

	PR_INFO("curr brightness:%d \n", img->brightness);
	return img->brightness;
}
#endif

#ifdef TUYA_DP_CONTRAST
VOID IPC_APP_set_contrast(UINT_T contrast)
{
	PR_INFO("set contrast:%d \n", contrast);
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IMG_PREF_S *img = &conf->img.data;
	UINT_T old_contrast;

	if (conf->img.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
			PR_ERR("Failed to get contrast conf.\n");
			return;
		}
		conf->img.update = 1;
	}

	if (img->contrast == (AGTX_INT16)contrast) {
		return;
	}
	old_contrast = img->contrast;
	img->contrast = contrast;

	if (aux_set_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
		PR_ERR("Failed to set contrast conf.\n");
		img->contrast = old_contrast;
		return;
	}
}

UINT_T IPC_APP_get_contrast(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IMG_PREF_S *img = &conf->img.data;

	if (conf->img.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
			PR_ERR("Failed to get contrast conf.\n");
			return 0;
		}
		conf->img.update = 1;
	}

	PR_INFO("curr contrast:%d \n", img->contrast);
	return img->contrast;
}
#endif

#ifdef TUYA_DP_IPC_OBJECT_OUTLINE
void loadDataPointIpcObjectOutline(void)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_OD_CONF_S *od = &conf->od.data;

	if (aux_get_cc_config(AGTX_CMD_OD_CONF, od) < 0) {
		PR_ERR("Failed to get AGTX_CMD_OD_CONF.\n");
		return;
	}

	conf->od.update = 1;

	active.dp198_ipc_object_outline = IPC_APP_get_ipc_object_outline();
	requested.dp198_ipc_object_outline = active.dp198_ipc_object_outline;
	acknowledged.dp198_ipc_object_outline = active.dp198_ipc_object_outline;

	TUYA_IVA_OD_SET_DEFAULT_PARAM(od);

	PR_DEBUG("requested.dp198_ipc_object_outline = %d.\n", requested.dp198_ipc_object_outline);
	PR_DEBUG("acknowledged.dp198_ipc_object_outline = %d.\n", acknowledged.dp198_ipc_object_outline);
	PR_DEBUG("active.dp198_ipc_object_outline = %d.\n", active.dp198_ipc_object_outline);
}

void handleDataPointIpcObjectOutline(TY_OBJ_DP_S *dp)
{
	int err;
	BOOL_T response;

	err = parseDataPointBool(dp, &requested.dp198_ipc_object_outline);

	if(err) {
		response = active.dp198_ipc_object_outline;
	} else {
		response = requested.dp198_ipc_object_outline;
	}

	reportDataPointBool(TUYA_DP_IPC_OBJECT_OUTLINE, response);
}

int IPC_APP_set_ipc_object_outline(BOOL_T enabled)
{
	PR_DEBUG("Setting Object Outline (%d)\n", enabled);

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_OD_CONF_S *od = &conf->od.data;
	BOOL_T old_enabled = od->enabled ? TRUE : FALSE;

	od->enabled = enabled ? 1 : 0;
	TUYA_IVA_OD_SET_DEFAULT_PARAM(od);

	if (aux_set_cc_config(AGTX_CMD_OD_CONF, od) < 0) {
		PR_ERR("Failed to set AGTX_CMD_OD_CONF.\n");
		od->enabled = old_enabled ? 1 : 0;
		return -1;
	}

	return 0;
}

BOOL_T IPC_APP_get_ipc_object_outline(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_OD_CONF_S *od = &conf->od.data;

	if(od->enabled) {
		return TRUE;
	} else {
		return FALSE;
	}
}
#endif

#ifdef TUYA_DP_EXPOSURE
static int expmode_tbl[] = {
		AGTX_ANTI_FLICKER_AUTO,
		AGTX_ANTI_FLICKER_50HZ,
		AGTX_ANTI_FLICKER_60HZ,
};

char *expmode_str[] = { "0", "1", "2" };

VOID IPC_APP_set_exposure(UINT_T exp_mode)
{
	PR_INFO("set exposure:%d \n", exp_mode);
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IMG_PREF_S *img = &conf->img.data;

	if (conf->img.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
			PR_ERR("Failed to get exposure conf.\n");
			return;
		}
		conf->img.update = 1;
	}

	AGTX_ANTI_FLICKER_E flicker, old_flicker;

	if (0 < exp_mode && exp_mode < 3) {
		flicker = expmode_tbl[exp_mode];
	} else {
		flicker = AGTX_ANTI_FLICKER_AUTO;
	}

	if (flicker == img->anti_flicker) {
		return;
	}

	old_flicker = img->anti_flicker;
	img->anti_flicker = flicker;

	if (aux_set_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
		PR_ERR("Failed to set anti flicker conf.\n");
		img->anti_flicker = old_flicker;
		return;
	}
}

CHAR_T* IPC_APP_get_exposure(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IMG_PREF_S *img = &conf->img.data;
	UINT_T flicker;

	if (conf->img.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_IMG_PREF, img) < 0) {
			PR_ERR("Failed to get exposure conf.\n");
			return 0;
		}
		conf->img.update = 1;
	}

	switch (img->anti_flicker) {
	case AGTX_ANTI_FLICKER_50HZ:
		flicker = 1;
		break;
	case AGTX_ANTI_FLICKER_60HZ:
		flicker = 2;
		break;
	case AGTX_ANTI_FLICKER_AUTO:
		flicker = 0;
		break;
	default:
		flicker = 0;
		PR_ERR("Unsupported anti-flicker mode\n");
		break;
	}
	return expmode_str[flicker];
}
#endif
//------------------------------------------

#ifdef TUYA_DP_WDR
VOID IPC_APP_set_wdr_onoff(BOOL_T wdr_on_off)
{
	PR_INFO("set wdr_on_off:%d \n", wdr_on_off);
	/* Wide Dynamic Range Model,BOOL type,true means on,false means off */
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_ADV_IMG_PREF_S *pref = &conf->pref.data;

	if (conf->pref.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_ADV_IMG_PREF, pref) < 0) {
			PR_ERR("Failed to get wdr conf.\n");
			return;
		}
		conf->pref.update = 1;
	}

	int old_enable;
	int enable = wdr_on_off == TRUE;

	if (pref->wdr_en == enable) {
		return;
	}

	old_enable = pref->wdr_en;
	pref->wdr_en = enable;

	if (aux_set_cc_config(AGTX_CMD_ADV_IMG_PREF, pref) < 0) {
		PR_ERR("Failed to set osd conf.\n");
		pref->wdr_en = old_enable;
		return;
	}
}

BOOL_T IPC_APP_get_wdr_onoff(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_ADV_IMG_PREF_S *pref = &conf->pref.data;

	if (conf->pref.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_ADV_IMG_PREF, pref) < 0) {
			PR_ERR("Failed to get wdr conf.\n");
			return FALSE;
		}
		conf->pref.update = 1;
	}

	BOOL_T wdr_on_off;
	if (pref->wdr_en) {
		wdr_on_off = TRUE;
	} else {
		wdr_on_off = FALSE;
	}

	PR_INFO("curr watermark_on_off:%d \n", wdr_on_off);
	return wdr_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_NIGHT_MODE
STATIC CHAR_T s_night_mode[4] = { 0 }; //for demo
VOID IPC_APP_set_night_mode(UINT_T night_mode)
{ //0-automatic 1-off 2-on
	PR_INFO("set night_mode:%d \n", night_mode);
	/* Infrared night vision function,ENUM type*/
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_ADV_IMG_PREF_S *pref = &conf->pref.data;

	if (conf->pref.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_ADV_IMG_PREF, pref) < 0) {
			PR_ERR("Failed to get night mode conf.\n");
			return;
		}
		conf->pref.update = 1;
	}

	AGTX_NIGHT_MODE_E agx_mode, old_agx_mode;
	// 0-auto 1-turn 2-open
	switch (night_mode) {
	case 0:
		agx_mode = AGTX_NIGHT_MODE_AUTO;
		break;
	case 1:
		agx_mode = AGTX_NIGHT_MODE_OFF;
		break;
	case 2:
		agx_mode = AGTX_NIGHT_MODE_ON;
		break;
	default:
		agx_mode = AGTX_NIGHT_MODE_AUTO;
		PR_ERR("Unknow night mode type, set to auto\n");
		break;
	}

	if (pref->night_mode == agx_mode) {
		return;
	}

	old_agx_mode = pref->night_mode;
	pref->night_mode = agx_mode;

	if (aux_set_cc_config(AGTX_CMD_ADV_IMG_PREF, pref) < 0) {
		PR_ERR("Failed to set night mode conf.\n");
		pref->night_mode = old_agx_mode;
		return;
	}
}

CHAR_T *IPC_APP_get_night_mode(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_ADV_IMG_PREF_S *pref = &conf->pref.data;

	if (conf->pref.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_ADV_IMG_PREF, pref) < 0) {
			PR_ERR("Failed to get night mode conf.\n");
			return "0";
		}
		conf->pref.update = 1;
	}

	INT_T night_mode;
	// 0-auto 1-turn 2-open
	switch (pref->night_mode) {
	case AGTX_NIGHT_MODE_AUTO:
		night_mode = 0;
		break;
	case AGTX_NIGHT_MODE_OFF:
		night_mode = 1;
		break;
	case AGTX_NIGHT_MODE_ON:
		night_mode = 2;
		break;
	default:
		PR_ERR("Unknown night mode type, return auto\n");
		night_mode = 0;
		break;
	}
	sprintf(s_night_mode, "%d", night_mode);

	PR_INFO("curr watermark_on_off:%s \n", s_night_mode);
	return s_night_mode;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_ALARM_FUNCTION
VOID IPC_APP_set_alarm_function_onoff(BOOL_T alarm_on_off)
{
	PR_INFO("set alarm_on_off:%d \n", alarm_on_off);
	/* motion detection alarm switch,BOOL type,true means on,false means off.
     * This feature has been implemented, and developers can make local configuration settings and properties.*/
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_MD_CONF_S *md = &conf->md.data;
	AGTX_IVA_OD_CONF_S *od = &conf->od.data;

	if (conf->md.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_MD_CONF, md) < 0) {
			PR_ERR("Failed to get md conf.\n");
			return;
		}
		conf->md.update = 1;
	}
	/* FIXME: workaround for #11606 #11650 */
	if (conf->od.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_OD_CONF, md) < 0) {
			PR_ERR("Failed to get md conf.\n");
			return;
		}
		conf->od.update = 1;
	}

	int old_enabled;
	int enabled = alarm_on_off == TRUE;

	if (md->enabled == enabled) {
		return;
	}

	old_enabled = md->enabled;
	md->enabled = enabled;

	if (aux_set_cc_config(AGTX_CMD_MD_CONF, md) < 0) {
		PR_ERR("Failed to set md conf.\n");
		md->enabled = old_enabled;
		return;
	}

	/* FIXME: workaround for #11606 #11650 */
	TUYA_IVA_OD_SET_DEFAULT_PARAM(od);

	if (aux_set_cc_config(AGTX_CMD_OD_CONF, od) < 0) {
		PR_ERR("Failed to set od conf.\n");
		return;
	}
}

BOOL_T IPC_APP_get_alarm_function_onoff(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_MD_CONF_S *md = &conf->md.data;

	if (conf->md.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_MD_CONF, md) < 0) {
			PR_ERR("Failed to get md conf.\n");
			return FALSE;
		}
		conf->md.update = 1;
	}

	BOOL_T alarm_on_off;
	if (md->enabled) {
		alarm_on_off = TRUE;
	} else {
		alarm_on_off = FALSE;
	}

	PR_INFO("curr alarm_on_off:%d \n", alarm_on_off);
	return alarm_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_ALARM_SENSITIVITY
// AG_ALARM_SENS_L, "0", 2s
#define FOREACH_ALARM_SENS(SENS) \
	SENS(L, "0", 2)          \
	SENS(M, "1", 1)          \
	SENS(H, "2", 0)

#define GENERATE_ALARM_SENS_ENUM(a, b, c)  AG_ALARM_SENS_##a,
#define GENERATE_ALARM_SENS_NAME(a, b, c)  b,
#define GENERATE_ALARM_SENS_VALUE(a, b, c) c,

typedef enum {
	FOREACH_ALARM_SENS(GENERATE_ALARM_SENS_ENUM)
	AG_ALARM_SENS_NUM
} AG_ALARM_SENS_E;

static int alarm_sens_tbl[] = {
	FOREACH_ALARM_SENS(GENERATE_ALARM_SENS_VALUE)
};

static char* alarm_sens_name[] = {
	FOREACH_ALARM_SENS(GENERATE_ALARM_SENS_NAME)
};

VOID IPC_APP_set_alarm_sensitivity(UINT_T sensitivity)
{
	PR_INFO("set alarm_sensitivity:%d \n", sensitivity);
	/* Motion detection alarm sensitivity,ENUM type,0 means low sensitivity,1 means medium sensitivity,2 means high sensitivity*/
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_MD_CONF_S *md = &conf->md.data;

	if (conf->md.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_MD_CONF, md) < 0) {
			PR_ERR("Failed to get md conf.\n");
			return;
		}
		conf->md.update = 1;
	}

	int old_sens = md->alarm_buffer;
	int sens;

	if (sensitivity < AG_ALARM_SENS_NUM) {
		sens = alarm_sens_tbl[sensitivity];
	} else {
		PR_ERR("Unknow motion sensitivity type, set to HIGHT\n");
		sens = alarm_sens_tbl[AG_ALARM_SENS_H];
	}

	if (old_sens == sens) {
		return;
	}
	md->alarm_buffer = sens;

	if (aux_set_cc_config(AGTX_CMD_MD_CONF, md) < 0) {
		PR_ERR("Failed to set md conf.\n");
		md->alarm_buffer = old_sens;
		return;
	}
}

CHAR_T *IPC_APP_get_alarm_sensitivity(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_IVA_MD_CONF_S *md = &conf->md.data;

	if (conf->md.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_MD_CONF, md) < 0) {
			PR_ERR("Failed to get md conf.\n");
			return "0";
		}
		conf->md.update = 1;
	}

	int sens;
	if (md->alarm_buffer >= alarm_sens_tbl[AG_ALARM_SENS_L]) {
		sens = AG_ALARM_SENS_L;
	} else if (md->alarm_buffer >= alarm_sens_tbl[AG_ALARM_SENS_M]) {
		sens = AG_ALARM_SENS_M;
	} else {
		sens = AG_ALARM_SENS_H;
	}

	PR_INFO("curr alarm_sensitivity:%s \n", alarm_sens_name[sens]);
	return alarm_sens_name[sens];
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
VOID IPC_APP_set_alarm_zone_onoff(BOOL_T alarm_zone_on_off)
{
	/* Motion detection area setting switch,BOOL type,true means on,false is off*/
	PR_INFO("set alarm_zone_onoff:%d \n", alarm_zone_on_off);

	/* This feature has been implemented, and developers can make local configuration settings and properties.*/

	//writeConfigInt("alarm_zone_on_off", alarm_zone_on_off);
}

BOOL_T IPC_APP_get_alarm_zone_onoff(VOID)
{
	BOOL_T alarm_zone_on_off = FALSE; // = readConfigInt("alarm_zone_on_off");
	PR_INFO("curr alarm_on_off:%d \n", alarm_zone_on_off);
	return alarm_zone_on_off;
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW

#define MAX_ALARM_ZONE_NUM (6) //Supports the maximum number of detection areas
//Detection area structure
typedef struct {
	char pointX; //Starting point x  [0-100]
	char pointY; //Starting point Y  [0-100]
	char width; //width    [0-100]
	char height; //height    [0-100]
} ALARM_ZONE_T;

typedef struct {
	int iZoneNum; //Number of detection areas
	ALARM_ZONE_T alarmZone[MAX_ALARM_ZONE_NUM];
} ALARM_ZONE_INFO_T;

VOID IPC_APP_set_alarm_zone_draw(cJSON *p_alarm_zone)
{
	if (NULL == p_alarm_zone) {
		return;
	}
#if 0
    /*demo code*/
    /*Motion detection area setting switch*/
    PR_INFO("%s %d set alarm_zone_set:%s \n",__FUNCTION__,__LINE__, (char *)p_alarm_zone);
    ALARM_ZONE_INFO_T strAlarmZoneInfo;
    INT_T i = 0;
    cJSON * pJson = cJSON_Parse((CHAR_T *)p_alarm_zone);

    if (NULL == pJson){
        PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
        //free(pResult);
        return;
    }
    cJSON * tmp = cJSON_GetObjectItem(pJson, "num");
    if (NULL == tmp){
        PR_ERR("%s %d step error\n",__FUNCTION__,__LINE__);
        cJSON_Delete(pJson);
        //free(pResult);
        return ;
    }
    memset(&strAlarmZoneInfo, 0x00, sizeof(ALARM_ZONE_INFO_T));
    strAlarmZoneInfo.iZoneNum = tmp->valueint;
    PR_INFO("%s %d step num[%d]\n",__FUNCTION__,__LINE__,strAlarmZoneInfo.iZoneNum);
    if (strAlarmZoneInfo.iZoneNum > MAX_ALARM_ZONE_NUM){
        PR_ERR("#####error zone num too big[%d]\n",strAlarmZoneInfo.iZoneNum);
        cJSON_Delete(pJson);
        //free(pResult);
        return ;
    }
    for (i = 0; i < strAlarmZoneInfo.iZoneNum; i++){
        char region[12] = {0};
        cJSON * cJSONRegion = NULL;
        snprintf(region, 12, "region%d",i);
        cJSONRegion = cJSON_GetObjectItem(pJson, region);
        if (NULL == cJSONRegion){
            PR_ERR("#####[%s][%d]error\n",__FUNCTION__,__LINE__);
            cJSON_Delete(pJson);
            //free(pResult);
            return;
        }
        strAlarmZoneInfo.alarmZone[i].pointX = cJSON_GetObjectItem(cJSONRegion, "x")->valueint;
        strAlarmZoneInfo.alarmZone[i].pointY = cJSON_GetObjectItem(cJSONRegion, "y")->valueint;
        strAlarmZoneInfo.alarmZone[i].width = cJSON_GetObjectItem(cJSONRegion,  "xlen")->valueint;
        strAlarmZoneInfo.alarmZone[i].height = cJSON_GetObjectItem(cJSONRegion, "ylen")->valueint;
        PR_INFO("#####[%s][%d][%d,%d,%d,%d]\n",__FUNCTION__,__LINE__,strAlarmZoneInfo.alarmZone[i].pointX,\
            strAlarmZoneInfo.alarmZone[i].pointY,strAlarmZoneInfo.alarmZone[i].width,strAlarmZoneInfo.alarmZone[i].height);
    }
    cJSON_Delete(pJson);
    //free(pResult);
#endif
	return;
}
static char s_alarm_zone[256] = { 0 };
char *IPC_APP_get_alarm_zone_draw(VOID)
{
	/*demo code*/
	int i;
	ALARM_ZONE_INFO_T strAlarmZoneInfo;

	memset(&strAlarmZoneInfo, 0x00, sizeof(ALARM_ZONE_INFO_T));
	//tycam_kv_db_read(BASIC_IPC_ALARM_ZONE_SET,&strAlarmZoneInfo);
	/*get param of alarmzoneInfo yourself*/
	memset(s_alarm_zone, 0x00, 256);
	if (strAlarmZoneInfo.iZoneNum > MAX_ALARM_ZONE_NUM) {
		PR_ERR("[%s] [%d ]get iZoneNum[%d] error", __FUNCTION__, __LINE__, strAlarmZoneInfo.iZoneNum);
		return s_alarm_zone;
	}
	for (i = 0; i < strAlarmZoneInfo.iZoneNum; i++) {
		char region[64] = { 0 };
		//{"169":"{\"num\":1,\"region0\":{\"x\":0,\"y\":0,\"xlen\":50,\"ylen\":50}}"}
		if (0 == i) {
			snprintf(s_alarm_zone, 256, "{\\\"num\\\":%d", strAlarmZoneInfo.iZoneNum);
		}
		snprintf(region, 64, ",\\\"region%d\\\":{\\\"x\\\":%d,\\\"y\\\":%d,\\\"xlen\\\":%d,\\\"ylen\\\":%d}", i,
		         strAlarmZoneInfo.alarmZone[i].pointX, strAlarmZoneInfo.alarmZone[i].pointY,
		         strAlarmZoneInfo.alarmZone[i].width, strAlarmZoneInfo.alarmZone[i].height);
		strcat(s_alarm_zone, region);
		if (i == (strAlarmZoneInfo.iZoneNum - 1)) {
			strcat(s_alarm_zone, "}");
		}
	}
	PR_INFO("[%s][%d] alarm zone[%s]\n", __FUNCTION__, __LINE__, s_alarm_zone);
	return s_alarm_zone;
}
#endif

//------------------------------------------

//#ifdef TUYA_DP_ALARM_INTERVAL
//STATIC CHAR_T s_alarm_interval[4] = {0};//for demo
//VOID IPC_APP_set_alarm_interval(CHAR_T *p_interval)
//{
//    PR_INFO("set alarm_interval:%s \n", p_interval);
//    //TODO
//    /* Motion detection alarm interval,unit is minutes,ENUM type,"1","5","10","30","60" */

//    writeConfigStr("tuya_alarm_interval", p_interval);
//}

//CHAR_T *IPC_APP_get_alarm_interval(VOID)
//{
//    /* Motion detection alarm interval,unit is minutes,ENUM type,"1","5","10","30","60" */
//    readConfigStr("tuya_alarm_interval", s_alarm_interval, 4);
//    PR_INRO("curr alarm_intervaly:%s \n", s_alarm_interval);
//    return s_alarm_interval;
//}
//#endif

//------------------------------------------

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_status(VOID)
{
	INT_T sd_status = 1;
	/* SD card status, VALUE type, 1-normal, 2-anomaly, 3-insufficient space, 4-formatting, 5-no SD card */
	/* Developer needs to return local SD card status */
	//TODO

	if (s_sd_format_progress > 0 && s_sd_format_progress != 100) {
		sd_status = 4;
	} else {
		sd_status = tuya_ipc_sd_get_status();
	}

	PR_INFO("curr sd_status:%d \n", sd_status);
	return sd_status;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
VOID IPC_APP_get_sd_storage(UINT_T *p_total, UINT_T *p_used, UINT_T *p_empty)
{ //unit is kb
	//TODO
	/* Developer needs to return local SD card status */
	tuya_ipc_sd_get_capacity(p_total, p_used, p_empty);

	PR_INFO("curr sd total:%u used:%u empty:%u \n", *p_total, *p_used, *p_empty);
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_RECORD_ENABLE
VOID IPC_APP_set_sd_record_onoff(BOOL_T sd_record_on_off)
{
	int old_enable = 0;

	PR_INFO("set sd_record_on_off:%d \n", sd_record_on_off);
	/* SD card recording function swith, BOOL type, true means on, false means off.
     * This function has been implemented, and developers can make local configuration settings and properties.*/

	//    if(sd_record_on_off == TRUE)
	//    {
	//         IPC_APP_set_sd_record_mode( IPC_APP_get_sd_record_mode()  );
	//    }else
	//    {
	//        tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
	//    }

	//writeConfigInt("tuya_sd_record_on_off", sd_record_on_off);

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_LOCAL_RECORD_CONF_S *record = &conf->local_record.data;

	if (conf->local_record.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_LOCAL_RECORD_CONF, record) < 0) {
			PR_ERR("Failed to get local record conf.\n");
			return;
		}
		conf->local_record.update = 1;
	}

	old_enable = record->enabled;

	record->enabled = sd_record_on_off;

	if (aux_set_cc_config(AGTX_CMD_LOCAL_RECORD_CONF, record) < 0) {
		PR_ERR("Failed to set local record conf.\n");
		record->enabled = old_enable;
		return;
	}

	TUYA_APP_Update_Sd_Parameter();

	return;
}

BOOL_T IPC_APP_get_sd_record_onoff(VOID)
{
	BOOL_T sd_record_on_off = (BOOL_T)(TUYA_AG_CONF_S *)(AG_GET_CONF())->local_record.data.enabled;
	PR_INFO("curr sd_record_on_off:%d \n", sd_record_on_off);
	return sd_record_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_RECORD_MODE
VOID IPC_APP_set_sd_record_mode(UINT_T sd_record_mode)
{
	int old_mode = 0;
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_LOCAL_RECORD_CONF_S *record = &conf->local_record.data;

	if (conf->local_record.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_LOCAL_RECORD_CONF, record) < 0) {
			PR_ERR("Failed to get local record conf.\n");
			return;
		}
		conf->local_record.update = 1;
	}

	old_mode = record->mode;

	if (0 == sd_record_mode) {
		record->mode = AGTX_RECORD_MODE_event;
	} else if (1 == sd_record_mode) {
		record->mode = AGTX_RECORD_MODE_continuous;
	} else {
		PR_ERR("Error, should not happen sd_record_mode %d\n", sd_record_mode);
		record->mode = AGTX_RECORD_MODE_continuous;
	}

	if (aux_set_cc_config(AGTX_CMD_LOCAL_RECORD_CONF, record) < 0) {
		PR_ERR("Failed to set local record conf.\n");
		record->mode = old_mode;
		return;
	}

	TUYA_APP_Update_Sd_Parameter();

	return;
}

UINT_T IPC_APP_get_sd_record_mode(VOID)
{
	BOOL_T sd_record_mode = (BOOL_T)(TUYA_AG_CONF_S *)(AG_GET_CONF())->local_record.data.mode;
	PR_INFO("curr sd_record_mode:%d \n", sd_record_mode);
	return sd_record_mode;
}

#endif

//------------------------------------------

#ifdef TUYA_DP_SD_UMOUNT
BOOL_T IPC_APP_unmount_sd_card(VOID)
{
	BOOL_T umount_ok = TRUE;
	CHAR_T format_cmd[128] = { 0 };
	char buffer[512] = { 0 };

	//TODO
	/* unmount sdcard */
	snprintf(format_cmd, 128, "/lib/mdev/automount.sh sdcard;/bin/sleep 1");
	FILE *pp = popen(format_cmd, "r");
	if (NULL != pp) {
		fgets(buffer, sizeof(buffer), pp);
		PR_INFO("%s\n", buffer);
		pclose(pp);
	} else {
		PR_ERR("unmount_sd_card failed\n");
		umount_ok = FALSE;
		return umount_ok;
	}

	PR_INFO("unmount result:%d \n", umount_ok);
	return umount_ok;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_SD_FORMAT
/* -2000: SD card is being formatted, -2001: SD card formatting is abnormal, -2002: No SD card,
   -2003: SD card error. Positive number is formatting progress */
void *thread_sd_format(void *data)
{
	AGTX_UNUSED(data);
	int ret = 0;

	/* First notify to app, progress 0% */
	s_sd_format_progress = 0;
	IPC_APP_report_sd_format_status(s_sd_format_progress);
	sleep(1);

	/* Stop local SD card recording and playback, progress 10%*/
	s_sd_format_progress = 10;
	IPC_APP_report_sd_format_status(s_sd_format_progress);
	tuya_ipc_set_format_flag(1);
	tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_NONE);
	tuya_ipc_ss_pb_stop_all();
	sleep(1);

	/* Delete the media files in the SD card, the progress is 30% */
	s_sd_format_progress = 30;
	IPC_APP_report_sd_format_status(s_sd_format_progress);
	//tuya_ipc_ss_delete_all_files();
	sleep(1);

	/* Perform SD card formatting operation */
	ret = tuya_ipc_sd_format();
	if (ret < 0) {
		s_sd_format_progress = ret;
		IPC_APP_report_sd_format_status(s_sd_format_progress);
	} else {
		s_sd_format_progress = 80;
		IPC_APP_report_sd_format_status(s_sd_format_progress);
		//TODO
		tuya_ipc_set_format_flag(0);
		TUYA_APP_Update_Sd_Parameter();
//		tuya_ipc_ss_set_write_mode(SS_WRITE_MODE_ALL);
		//    IPC_APP_set_sd_record_onoff( IPC_APP_get_sd_record_onoff());

		sleep(1);
		IPC_APP_report_sd_storage();
		/* progress 100% */
		s_sd_format_progress = 100;
		IPC_APP_report_sd_format_status(s_sd_format_progress);
	}

	pthread_exit(0);
}

VOID IPC_APP_format_sd_card(VOID)
{
	char name[16] = "sdformat"; // max. length is 16
	PR_INFO("start to format sd_card \n");
	/* SD card formatting.
     * The SDK has already completed the writing of some of the code,
     and the developer only needs to implement the formatting operation. */

	pthread_t sd_format_thread;
	pthread_create(&sd_format_thread, NULL, thread_sd_format, name);
	pthread_detach(sd_format_thread);
}
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_format_status(VOID)
{
	return s_sd_format_progress;
}
#endif

#if defined(TUYA_DP_PTZ_CONTROL) || defined(TUYA_DP_PTZ_STOP)
#define AG_PTZ_SPEED 128
#define AG_PTZ_DIR_NUM 8
//0-up, 1-upper right, 2-right, 3-lower right, 4-down, 5-down left, 6-left, 7-top left
static int ptz_move_tbl[AG_PTZ_DIR_NUM + 1][2] = {
	{ 0, -AG_PTZ_SPEED }, //0-up
	{ AG_PTZ_SPEED, -AG_PTZ_SPEED }, //1-upper right
	{ AG_PTZ_SPEED, 0 }, //2-right
	{ AG_PTZ_SPEED, AG_PTZ_SPEED }, //3-lower right
	{ 0, AG_PTZ_SPEED }, //4-down
	{ -AG_PTZ_SPEED, AG_PTZ_SPEED }, //5-down left
	{ -AG_PTZ_SPEED, 0 }, //6-left
	{ -AG_PTZ_SPEED, -AG_PTZ_SPEED }, //7-top left
	{ 0, 0 } //8-STOP
};
#endif
#ifdef TUYA_DP_PTZ_CONTROL
VOID IPC_APP_ptz_start_move(UINT_T dir)
{
	//PR_INFO("ptz start move:%d \n", dir);
	//0-up, 1-upper right, 2-right, 3-lower right, 4-down, 5-down left, 6-left, 7-top left
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (conf->ptz.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
			PR_ERR("Failed to get ptz conf.\n");
			return;
		}
		conf->ptz.update = 1;
	}

	if (ptz->mode == AGTX_VIDEO_PTZ_MODE_SCAN) {
		PR_NOTICE("PTZ mode is not support\n");
		return;
	}

	ptz->zoom_speed_width = 0;
	ptz->zoom_speed_height = 0;

	if (dir < AG_PTZ_DIR_NUM) {
		if (ptz->win_speed_x == ptz_move_tbl[dir][0] && ptz->win_speed_y == ptz_move_tbl[dir][1]) {
			/* Same cmd, no need to send to cc */
			return;
		}
		ptz->win_speed_x = ptz_move_tbl[dir][0];
		ptz->win_speed_y = ptz_move_tbl[dir][1];
	} else {
		ptz->win_speed_x = ptz_move_tbl[AG_PTZ_DIR_NUM][0];
		ptz->win_speed_y = ptz_move_tbl[AG_PTZ_DIR_NUM][1];
		PR_NOTICE("PTZ receives unknown direction cmd, stop.\n");
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to set ptz conf.\n");
		return;
	}
}
#endif

#ifdef TUYA_DP_PTZ_STOP
VOID IPC_APP_ptz_stop_move(VOID)
{
	//PR_INFO("ptz stop move\n");

	/* PTZ rotation stops */
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (conf->ptz.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
			PR_ERR("Failed to get ptz conf.\n");
			return;
		}
		conf->ptz.update = 1;
	}

	if (ptz->mode == AGTX_VIDEO_PTZ_MODE_SCAN) {
		PR_NOTICE("PTZ mode is not support\n");
		return;
	}

	if (ptz->win_speed_x == 0 && ptz->win_speed_y == 0) {
		return;
	}

	ptz->zoom_speed_width = 0;
	ptz->zoom_speed_height = 0;
	ptz->win_speed_x = 0;
	ptz->win_speed_y = 0;

	if (aux_set_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to set ptz conf.\n");
		return;
	}
}
#endif

#if defined(TUYA_DP_PTZ_ZOOM_CONTROL) || defined(TUYA_DP_PTZ_ZOOM_STOP)
#define AG_PTZ_ZOOM_SPEED_X 16368
#define AG_PTZ_ZOOM_SPEED_Y 18432
#define AG_PTZ_ZOOM_DIR_NUM 2
//0-zoom out, 1-zoom in
static int ptz_zoom_tbl[AG_PTZ_ZOOM_DIR_NUM + 1][2] = {
	{ -AG_PTZ_ZOOM_SPEED_X, -AG_PTZ_ZOOM_SPEED_Y }, //0-zoom in
	{ AG_PTZ_ZOOM_SPEED_X, AG_PTZ_ZOOM_SPEED_Y }, //1-zoom out
	{ 0, 0 } //2-STOP
};
#endif
#ifdef TUYA_DP_PTZ_ZOOM_CONTROL
VOID IPC_APP_ptz_start_zoom(INT_T zoom)
{
	PR_NOTICE("ptz start zoom:%d.\n", zoom);
	PR_INFO("ptz start zoom:%d.\n", zoom);
	//0-zoom out, 1-zoom in
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (conf->ptz.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
			PR_ERR("Failed to get ptz conf.\n");
			return;
		}
		conf->ptz.update = 1;
	}

	if (ptz->mode == AGTX_VIDEO_PTZ_MODE_SCAN) {
		PR_NOTICE("PTZ mode is not support\n");
		return;
	}

	ptz->win_speed_x = 0;
	ptz->win_speed_y = 0;
	if (zoom < AG_PTZ_ZOOM_DIR_NUM) {
		if (ptz->zoom_speed_width == ptz_zoom_tbl[zoom][0] && ptz->zoom_speed_height == ptz_zoom_tbl[zoom][1]) {
			/* Same cmd, no need to send to cc */
			return;
		}
		ptz->zoom_speed_width = ptz_zoom_tbl[zoom][0];
		ptz->zoom_speed_height = ptz_zoom_tbl[zoom][1];
	} else {
		ptz->zoom_speed_width = ptz_move_tbl[AG_PTZ_ZOOM_DIR_NUM][0];
		ptz->zoom_speed_height = ptz_move_tbl[AG_PTZ_ZOOM_DIR_NUM][1];
		PR_NOTICE("PTZ receives unknown direction cmd, stop.\n");
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to set ptz zoom conf.\n");
		return;
	}
}
#endif

#ifdef TUYA_DP_PTZ_ZOOM_STOP
VOID IPC_APP_ptz_stop_zoom(VOID)
{
	PR_INFO("ptz stop zoom.\n");

	/* PTZ rotation stops */
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (conf->ptz.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
			PR_ERR("Failed to get ptz conf.\n");
			return;
		}
		conf->ptz.update = 1;
	}

	if (ptz->mode == AGTX_VIDEO_PTZ_MODE_SCAN) {
		PR_NOTICE("PTZ mode is not support\n");
		return;
	}

	if (ptz->zoom_speed_width == 0 && ptz->zoom_speed_height == 0) {
		return;
	}

	ptz->win_speed_x = 0;
	ptz->win_speed_y = 0;
	ptz->zoom_speed_width = 0;
	ptz->zoom_speed_height = 0;

	if (aux_set_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to set ptz zoom conf.\n");
		return;
	}
}
#endif

#ifdef TUYA_DP_PTZ_CHECK
void IPC_APP_ptz_check(VOID)
{
	PR_INFO("ptz check \n");
}
#endif

#ifdef TUYA_DP_HUM_FILTER
void IPC_APP_human_filter(BOOL_T filter_enable)
{
	PR_INFO("filter_enable %d.\n", filter_enable);
	return;
}
#endif

#ifdef TUYA_DP_PATROL_MODE
void IPC_APP_set_patrol_mode(BOOL_T patrol_mode)
{
	PR_INFO("patrol_mode %d.\n", patrol_mode);
	return;
}

char IPC_APP_get_patrol_mode(void)
{
	char patrol_mode = 0;
	//the value you get yourself
	return patrol_mode;
}

#endif

#ifdef TUYA_DP_PATROL_TMODE
void IPC_APP_set_patrol_tmode(BOOL_T patrol_tmode)
{
	PR_INFO("patrol_tmode %d.\n", patrol_tmode);
	return;
}

char IPC_APP_get_patrol_tmode(void)
{
	char patrol_tmode = 0;
	//the value you get yourself
	return patrol_tmode;
}

#endif

#ifdef TUYA_DP_PATROL_TIME
void IPC_APP_set_patrol_time(cJSON *p_patrol_time __attribute__((unused)))
{
	//set your patrol_time
	/*

    cJSON * pJson = cJSON_Parse((CHAR_T *)p_patrol_time);
    if (NULL == pJson){
        TYWARN("----error---\n");

        return -1;
    }
    cJSON* t_start = cJSON_GetObjectItem(pJson, "t_start");
    cJSON* t_end = cJSON_GetObjectItem(pJson, "t_end");
    if ((NULL == t_start) || (NULL == t_end)){
        TYWARN("----t_start---\n");
        cJSON_Delete(pJson);
        return -1;
    }
    TYDEBUG("stare%s--end:%s\n", t_start->valuestring,t_end->valuestring);

    */
	return;
}

#endif

#ifdef TUYA_DP_PRESET_SET
void IPC_APP_set_preset(cJSON *p_preset_param __attribute__((unused)))
{
//preset add ,preset del, preset go
#if 0
    cJSON * pJson = cJSON_Parse((CHAR_T *)p_preset_param);
    if (NULL == pJson){
        TYERROR("null preset set input");
        return -1;
    }
    cJSON* type = cJSON_GetObjectItem(pJson, "type");
    cJSON* data = cJSON_GetObjectItem(pJson, "data");
    if ((NULL == type) || (NULL == data)){
        TYERROR("invalid preset set input");
        return -1;
    }


    TYDEBUG("preset set type: %d",type->valueint);
    //1:add preset point 2:delete preset point 3:call preset point
    if(type->valueint == 1)
    {
        char pic_buffer[PRESET_PIC_MAX_SIZE] = {0};
        int pic_size = sizeof(pic_buffer);  /*Image to be shown*/
        S_PRESET_POSITION preset_pos;
        char respond_add[128] = {0};
        /*mpId is 1,2,3,4,5,6 The server will generate a set of its own preset point number information based on the mpid.*/
        preset_pos.mpId = 1;
        preset_pos.ptz.pan = 100; /*horizontal position*/
        preset_pos.ptz.tilt = 60;/*vertical position*/
        cJSON* name = cJSON_GetObjectItem(data, "name");
        int name_len = 0;
        int error_num = 0;

        if(name == NULL)
        {
            TYERROR("name is null\n");
            return -1;
        }
        name_len = strlen(name->valuestring);
        name_len = (name_len > 30)?30:name_len;
        memcpy(&preset_pos.name,name->valuestring,(name_len));
        preset_pos.name[name_len] = '\0';
        error_num = tuya_ipc_preset_add(&preset_pos);

        snprintf(respond_add,128,"{\\\"type\\\":%d,\\\"data\\\":{\\\"error\\\":%d}}",type->valueint,error_num);

        tuya_ipc_dp_report(NULL, TUYA_DP_PRESET_SET,PROP_STR,respond_add,1);

        //tuya_ipc_preset_add_pic(pic_buffer,pic_size); /*if you need show pic ,you should set this api*/
    }
    else if(type->valueint == 2)
    {
        cJSON* num = cJSON_GetObjectItem(data, "num"); //can delete one or more
        cJSON* sets = cJSON_GetObjectItem(data, "sets");
        char respond_del[128] = {0};
        cJSON* del_data;
        int del_num = num->valueint;
        for(i = 0; i < del_num; i++)
        {
            del_data = cJSON_GetArrayItem(sets,i);
            cJSON* devId = cJSON_GetObjectItem(del_data, "devId");  /*devid is the preset point number registered in the server*/
            cJSON* mpId = cJSON_GetObjectItem(del_data, "mpId");  /*mpid is the preset point number managed on the device*/
            if((NULL == devId) || (NULL == mpId))
            {
                PR_ERR("devid or mpid is error\n");
                return -1;
            }
            del_preset.seq = atoi(mpId->valuestring);

            PR_INFO("%d---%s\n",del_preset.seq,devId->valuestring);

            error_num = (int)time(NULL);

            tuya_ipc_preset_del(devId->valuestring);

            snprintf(respond_add,128,"{\\\"type\\\":%d,\\\"data\\\":{\\\"error\\\":%d}}",type->valueint,error_num);
        }
    }
    else if(type->valueint == 3)
    {
        cJSON* mpId = cJSON_GetObjectItem(data, "mpId");

        preset_seq = atoi(mpId->valuestring);
        //get your seq pos and go there
    }
#endif
	return;
}

#endif

#ifdef TUYA_DP_PATROL_STATE
void IPC_APP_patrol_state(int *patrol_state __attribute__((unused)))
{
	//PR_INFO("patrol_state %d\n",atoi(patrol_state));
	//return your patrol_state
	return;
}

#endif

#ifdef TUYA_DP_LINK_MOVE_SET
VOID IPC_APP_set_link_pos(INT_T bind_seq)
{
	/*set the link pos*/
	printf("IPC_APP_set_bind_pos:%d \r\n", bind_seq);
	/*demo
    step1: get the current position
    step2: save the position to flash
    */
	return;
}
#endif

#ifdef TUYA_DP_LINK_MOVE_ACTION
VOID IPC_APP_set_link_move(INT_T bind_seq)
{
	/*move to the link pos*/
	printf("IPC_APP_set_bind_move:%d \r\n", bind_seq);
	/*demo
     step1: get the position base seq
     step2: go to the target position
    */
	return;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_BLUB_SWITCH
VOID IPC_APP_set_blub_onoff(BOOL_T blub_on_off)
{
	PR_INFO("set blub_on_off:%d.\n", blub_on_off);
	//TODO
	/* light control switch, BOOL type,true means on,false menas off */

	writeConfigInt("tuya_blub_on_off", blub_on_off);
}

BOOL_T IPC_APP_get_blub_onoff(VOID)
{
	BOOL_T blub_on_off = readConfigInt("tuya_blub_on_off");
	PR_INFO("curr blub_on_off:%d.\n", blub_on_off);
	return blub_on_off;
}
#endif

//------------------------------------------

#ifdef TUYA_DP_ELECTRICITY
INT_T IPC_APP_get_battery_percent(VOID)
{
	//TODO
	/* battery power percentage VALUE type,[0-100] */

	return 80;
}
#endif

#ifdef TUYA_DP_POWERMODE
CHAR_T *IPC_APP_get_power_mode(VOID)
{
	//TODO
	/* Power supply mode, ENUM type,
    "0" is the battery power supply state, "1" is the plug-in power supply state (or battery charging state) */

	return "1";
}
#endif

#ifdef TUYA_DP_SIREN_SWITCH
#define AG_SIREN_SWITCH_NUM 2

void IPC_APP_set_siren_switch(BOOL_T siren_switch)
{
	PR_INFO("siren_switch %d\n", siren_switch);
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;

	if (conf->siren.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
			PR_ERR("Failed to get siren conf.\n");
			return;
		}
		conf->siren.update = 1;
	}

	UINT_T old_enabled = siren->enabled;
	if (siren_switch < AG_SIREN_SWITCH_NUM) {
		siren->enabled = siren_switch;
	} else {
		PR_ERR("Unsupport siren_switch %d.\n", siren_switch);
		return;
	}

	if (aux_set_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
		PR_ERR("Failed to set siren switch conf.\n");
		siren->enabled = old_enabled;
		return;
	}

	return;
}

BOOL_T IPC_APP_get_siren_switch(void)
{
	BOOL_T siren_switch = TUYA_APP_Get_Siren_Status();

	return siren_switch;
}

#endif

#ifdef TUYA_DP_SIREN_VOLUME
#define FOREACH_SIREN_VOL(SIREN_VOL) \
	SIREN_VOL(L, "0", 20) \
	SIREN_VOL(M, "1", 60) \
	SIREN_VOL(H, "2", 100) \


#define GENERATE_SIREN_VOL_ENUM(a, b, c)  AG_SIREN_VOL_##a,
#define GENERATE_SIREN_VOL_NAME(a, b, c)  b,
#define GENERATE_SIREN_VOL_VALUE(a, b, c) c,
#define AG_SIREN_VOL_NUM 101

typedef enum {
	FOREACH_SIREN_VOL(GENERATE_SIREN_VOL_ENUM)
} AG_SIREN_VOL_E;
/*
static char* siren_vol_name[] = {
	FOREACH_SIREN_VOL(GENERATE_SIREN_VOL_NAME)
};
*/

static int siren_vol_tbl[] = {
	FOREACH_SIREN_VOL(GENERATE_SIREN_VOL_VALUE)
};

VOID IPC_APP_set_siren_volume(UINT_T volume)
{
	PR_INFO("set siren volume:%d \n", volume);
	/* Motion detection alarm sensitivity,ENUM type,0 means low,1 means medium,2 means high*/
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;
	AGTX_VOICE_CONF_S *voice = &conf->voice.data;


	if (conf->siren.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
			PR_ERR("Failed to get siren conf.\n");
			return;
		}
		conf->siren.update = 1;
	}
	if (conf->voice.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VOICE_CONF, voice) < 0) {
			PR_ERR("Failed to get voice conf.\n");
			return;
		}
		conf->voice.update = 1;
	}

	int old_volume;
	int vol;

	if (volume < AG_SIREN_VOL_NUM) {
		vol = volume;
	} else {
		PR_ERR("Unknow siren volume type, set to HIGHT\n");
		vol = siren_vol_tbl[AG_SIREN_VOL_H];
	}

	old_volume = siren->volume;
	siren->volume = vol;
	voice->volume = vol;

	if ((aux_set_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) || aux_set_cc_config(AGTX_CMD_VOICE_CONF, voice) < 0) {
		PR_ERR("Failed to set siren and voice conf.\n");
		siren->volume = old_volume;
		voice->volume = old_volume;
		return;
	}
}

UINT_T IPC_APP_get_siren_volume(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;

	if (conf->siren.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
			PR_ERR("Failed to get siren conf.\n");
			return 0;
		}
		conf->siren.update = 1;
	}

	return siren->volume;
}
#endif

#ifdef TUYA_DP_SIREN_INTERVAL
#define FOREACH_SIREN_INT(SIREN_INT) \
	SIREN_INT(5,  "0") \
	SIREN_INT(10, "1") \
	SIREN_INT(15, "2") \
	SIREN_INT(30, "3") \
	SIREN_INT(60, "4") \
	SIREN_INT(120,"5") \
	SIREN_INT(300,"6") \

#define GENERATE_SIREN_INTERVAL_ENUM(a, b)  COMB(AG_SIREN_INT_##a, S),
#define GENERATE_SIREN_INTERVAL_NAME(a, b)  b,
#define GENERATE_SIREN_INTERVAL_INTEGER(a, b) (a),
#define AG_SIREN_INT_NUM 601

typedef enum {
	FOREACH_SIREN_INT(GENERATE_SIREN_INTERVAL_ENUM)
} AG_SIREN_INTERVAL_E;
/*
static int siren_int_tbl[] = {
	FOREACH_SIREN_INT(GENERATE_SIREN_INTERVAL_INTEGER)
};

static char* siren_int_name[] = {
	FOREACH_SIREN_INT(GENERATE_SIREN_INTERVAL_NAME)
};
*/
VOID IPC_APP_set_siren_interval(UINT_T interval_mode)
{
	PR_INFO("set siren interval:%d \n", interval_mode);
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;

	if (conf->siren.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
			PR_ERR("Failed to get siren conf.\n");
			return;
		}
		conf->siren.update = 1;
	}

	if (interval_mode >= AG_SIREN_INT_NUM) {
		PR_ERR("Unknown siren interval mode\n");
		return;
	}

	int old_interval = siren->duration;
	siren->duration = interval_mode;

	if (aux_set_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
		PR_ERR("Failed to set siren conf.\n");
		siren->duration = old_interval;
		return;
	}
}

UINT_T IPC_APP_get_siren_interval(VOID)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_SIREN_CONF_S *siren = &conf->siren.data;

	if (conf->siren.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_SIREN_CONF, siren) < 0) {
			PR_ERR("Failed to get siren conf.\n");
			return 0;
		}
		conf->siren.update = 1;
	}

	return siren->duration;
}
#endif


#ifdef TUYA_DP_VIDEO_LAYOUT
extern int g_get_jpeg_snapshot;
extern pthread_mutex_t g_snapshot_mutex;
void handle_DP_VIDEO_LAYOUT(TY_OBJ_DP_S *dp)
{
	UINT_T response;

	if ((dp == NULL) || (dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", dp->type);
		return;
	}

	if (TUYA_APP_Check_Event_Triggerd() == FALSE) {
		requested.dp196_ipc_video_layout = dp->value.dp_enum;
		response = requested.dp196_ipc_video_layout;
	} else {
		PR_WARN("Can't change layout when event happened.\n");
		response = active.dp196_ipc_video_layout;
	}

	respone_dp_enum(TUYA_DP_VIDEO_LAYOUT, dp_video_layout_str[response]);
}

void loadVideoLayoutSetting(void)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_LAYOUT_CONF_S *video_layout = &conf->layout.data;
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to get AGTX_CMD_VIDEO_PTZ_CONF.\n");
		return;
	}

	if (aux_get_cc_config(AGTX_CMD_VIDEO_LAYOUT_CONF, video_layout) < 0) {
		PR_ERR("Failed to get AGTX_CMD_VIDEO_LAYOUT_CONF.\n");
		return;
	}

	conf->ptz.update = 1;
	conf->layout.update = 1;

	active.dp196_ipc_video_layout = IPC_APP_get_video_layout();
	requested.dp196_ipc_video_layout = active.dp196_ipc_video_layout;
	acknowledged.dp196_ipc_video_layout = active.dp196_ipc_video_layout;
	if (active.dp196_ipc_video_layout == 1) {
		IPC_APP_Live_Video_Sub_En(0);
	}

	PR_DEBUG("requested.dp196_ipc_video_layout = %d.\n", requested.dp196_ipc_video_layout);
	PR_DEBUG("acknowledged.dp196_ipc_video_layout = %d.\n", acknowledged.dp196_ipc_video_layout);
	PR_DEBUG("active.dp196_ipc_video_layout = %d.\n", active.dp196_ipc_video_layout);
}

int IPC_APP_set_video_layout(UINT_T mode)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_LAYOUT_CONF_S *video_layout = &conf->layout.data;
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;
	AGTX_LAYOUT_PARAM_S *layout = NULL;
	AGTX_STRM_CONF_S *video_strm = &conf->strm.data;
	AGTX_STRM_PARAM_S *strm = &video_strm->video_strm[1];

	PR_NOTICE("set video_layout_mode %d.\n", mode);

	if (conf->ptz.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
			PR_ERR("Failed to get ptz conf.\n");
			return -1;
		}
		conf->ptz.update = 1;
	}

	if (conf->layout.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_LAYOUT_CONF, video_layout) < 0) {
			PR_ERR("Failed to get layout conf.\n");
			return -1;
		}
		conf->layout.update = 1;
	}

	layout = &video_layout->video_layout[1];

	PR_NOTICE("product id %s.\n", STR(PROD_ID));

	strm = &video_strm->video_strm[1];
	if (mode == 1) {
		IPC_APP_Live_Video_Sub_En(0);
		strm->output_fps = 1;
	} else {
		IPC_APP_Live_Video_Sub_En(1);
		strm->output_fps = 15;
	}
	/*
	 * Set layout based on product id (AGT902_5, AGT902_7)
	 * Refer to Makefile to obtain PROD_ID (-DPROD_ID)
	 */

	if (strcmp(STR(PROD_ID), "AGT902_5") == 0) {
		PR_NOTICE("set layout for product (%s).\n", STR(PROD_ID));
		/*pano + PIP*/
		if (mode == 0) {
			/*Enable ePTZ*/
			ptz->enabled = 1;
			/*set layout*/
			layout->window_num = 2;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 794;
			layout->window_array[0].pos_y = 768;
			layout->window_array[0].pos_width = 205;
			layout->window_array[0].pos_height = 205;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
		} else if (mode == 1) { /*pano + pano*/
			/*Disable ePTZ*/
			ptz->enabled = 0;
			/*Set layout*/
			layout->window_num = 1;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 0;
			layout->window_array[0].pos_y = 0;
			layout->window_array[0].pos_width = 1024;
			layout->window_array[0].pos_height = 1024;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
		} else if (mode == 2) { /*pano + zoom*/
			/*Enable ePTZ*/
			ptz->enabled = 1;
			/*Set layout*/
			layout->window_num = 1;
			layout->window_array[0].window_idx = 1;
			layout->window_array[0].pos_x = 0;
			layout->window_array[0].pos_y = 0;
			layout->window_array[0].pos_width = 1024;
			layout->window_array[0].pos_height = 1024;
			layout->window_array[0].roi_x = 341;
			layout->window_array[0].roi_y = 320;
			layout->window_array[0].roi_width = 341;
			layout->window_array[0].roi_height = 384;
		} else { /*pano + PIP*/
			/*Enable ePTZ*/
			ptz->enabled = 1;
			/*Set layout*/
			layout->window_num = 2;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 794;
			layout->window_array[0].pos_y = 768;
			layout->window_array[0].pos_width = 205;
			layout->window_array[0].pos_height = 205;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
			PR_ERR("Unknown layout set to PIP mode %d.\n", mode);
		}
	} else if (strcmp(STR(PROD_ID), "AGT902_7") == 0) {
		PR_NOTICE("set layout for product (%s).\n", STR(PROD_ID));
		/*pano + BCD*/
		if (mode == 0) {
			/*Enable ePTZ*/
			ptz->enabled = 1;
			/*set layout*/
			layout->window_num = 2;
			layout->window_array[0].window_idx = 1;
			layout->window_array[0].pos_height = 512;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
		} else if (mode == 1) { /*pano + pano*/
			/*Disable ePTZ*/
			ptz->enabled = 0;
			/*Set layout*/
			layout->window_num = 1;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_height = 1024;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
		} else if (mode == 2) { /*pano + zoom*/
			/*Enable ePTZ*/
			ptz->enabled = 1;
			/*Set layout*/
			layout->window_num = 1;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_height = 1024;
			layout->window_array[0].roi_x = 341;
			layout->window_array[0].roi_y = 320;
			layout->window_array[0].roi_width = 341;
			layout->window_array[0].roi_height = 384;
		} else { /*pano + BCD*/
			/*Enable ePTZ*/
			ptz->enabled = 1;
			/*Set layout*/
			layout->window_num = 2;
			layout->window_array[0].window_idx = 1;
			layout->window_array[0].pos_height = 512;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
			PR_ERR("Unknown layout set to BCD mode %d.\n", mode);
		}
	} else {
		/*pano + PIP*/
		if (mode == 0) {
			/*Enable ePTZ*/
			ptz->enabled = 1;
			ptz->subwindow_disp.win[0].win_idx = 1;
			/*set layout*/
			layout->window_num = 2;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 683;
			layout->window_array[0].pos_y = 721;
			layout->window_array[0].pos_width = 341;
			layout->window_array[0].pos_height = 303;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;

		} else if (mode == 1) { /*pano + pano*/
			/*Disable ePTZ*/
			ptz->enabled = 0;
			/*Set layout*/
			layout->window_num = 1;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 0;
			layout->window_array[0].pos_y = 0;
			layout->window_array[0].pos_width = 1024;
			layout->window_array[0].pos_height = 1024;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
		} else if (mode == 2) { /*pano + zoom*/
			/*Enable ePTZ*/
			ptz->enabled = 1;
			ptz->subwindow_disp.win[0].win_idx = 0;
			/*Set layout*/
			layout->window_num = 1;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 0;
			layout->window_array[0].pos_y = 0;
			layout->window_array[0].pos_width = 1024;
			layout->window_array[0].pos_height = 1024;
			layout->window_array[0].roi_x = 341;
			layout->window_array[0].roi_y = 320;
			layout->window_array[0].roi_width = 341;
			layout->window_array[0].roi_height = 384;
		} else { /*pano + PIP*/
			/*Enable ePTZ*/
			ptz->enabled = 1;
			ptz->subwindow_disp.win[0].win_idx = 1;
			/*Set layout*/
			layout->window_num = 2;
			layout->window_array[0].window_idx = 0;
			layout->window_array[0].pos_x = 683;
			layout->window_array[0].pos_y = 721;
			layout->window_array[0].pos_width = 341;
			layout->window_array[0].pos_height = 303;
			layout->window_array[0].roi_x = 0;
			layout->window_array[0].roi_y = 0;
			layout->window_array[0].roi_width = 1024;
			layout->window_array[0].roi_height = 1024;
			PR_ERR("Unknown layout set to PIP mode %d.\n", mode);
		}
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_STRM_CONF, video_strm) < 0) {
		PR_ERR("Failed to set strm conf.\n");
		return -1;
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to set ptz conf.\n");
		return -1;
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_LAYOUT_CONF, video_layout) < 0) {
		PR_ERR("Failed to set layout conf.\n");
		return -1;
	}

	return 0;
}

UINT_T IPC_APP_get_video_layout(VOID)
{
	UINT_T mode = 0;
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_LAYOUT_CONF_S *video_layout = &conf->layout.data;
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;
	AGTX_LAYOUT_PARAM_S *layout = NULL;

	if (conf->ptz.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
			PR_ERR("Failed to get ptz conf.\n");
			return mode;
		}
		conf->ptz.update = 1;
	}

	if (conf->layout.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_LAYOUT_CONF, video_layout) < 0) {
			PR_ERR("Failed to get layout conf.\n");
			return mode;
		}
		conf->layout.update = 1;
	}

	layout = &video_layout->video_layout[1];
	if (ptz->enabled && layout->window_num == 1) {
		mode = 2;
	} else if (!ptz->enabled && layout->window_num == 1) {
		mode = 1;
	} else if(layout->window_num == 2) {
		mode = 0;
	} else {
		PR_ERR("Unknown layout mode win %d / %d\n",layout->window_num, ptz->enabled);
		mode = 0;
	}

	PR_NOTICE("get video_layout_mode from CC mode %d\n",mode);

	return mode;
}
#endif

VOID IPC_APP_set_bitrate(int channel, int bit_rate)
{
	PR_INFO("set bit-rate:%d \n", bit_rate);
	int old_bit_rate;

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_STRM_CONF_S *strm = &conf->strm.data;

	if (conf->strm.update == 0) {
		if (aux_get_cc_config(AGTX_CMD_VIDEO_STRM_CONF, strm) < 0) {
			PR_ERR("Failed tp get flip conf.\n");
			return;
		}
		conf->strm.update = 1;
	}

	old_bit_rate = strm->video_strm[channel].bit_rate;
	strm->video_strm[channel].bit_rate = bit_rate;

	if (aux_set_cc_config(AGTX_CMD_VIDEO_STRM_CONF, strm) < 0) {
		PR_ERR("Failed to set bit-rate conf.\n");
		strm->video_strm[channel].bit_rate = old_bit_rate;
		return;
	}
}

#ifdef TUYA_DP_MOTION_TRACKING
void handleDataPointMotionTracking(TY_OBJ_DP_S *dp)
{
	int err;
	BOOL_T response;

	err = parseDataPointBool(dp, &requested.dp161_motion_tracking);

	if(err) {
		response = active.dp161_motion_tracking;
	} else {
		response = requested.dp161_motion_tracking;
	}

	reportDataPointBool(TUYA_DP_MOTION_TRACKING, response);
}

#define FOREACH_PTZ_MODE(MODE) \
	MODE(AUTO,   "0", AGTX_VIDEO_PTZ_MODE_AUTO) \
	MODE(MANUAL, "1", AGTX_VIDEO_PTZ_MODE_MANUAL) \
	MODE(SCAN,   "2", AGTX_VIDEO_PTZ_MODE_SCAN) \

#define GENERATE_PTZ_MODE_ENUM(a, b, c)  AG_PTZ_MODE_##a,
#define GENERATE_PTZ_MODE_NAME(a, b, c)  b,
#define GENERATE_PTZ_MODE_VALUE(a, b, c) c,

typedef enum {
	FOREACH_PTZ_MODE(GENERATE_PTZ_MODE_ENUM)
	AG_PTZ_MODE_NUM
} AG_PTZ_MODE_E;

static int ptz_int_tbl[] = {
	FOREACH_PTZ_MODE(GENERATE_PTZ_MODE_VALUE)
};

int IPC_APP_set_ptz_mode(UINT_T mode)
{
	PR_DEBUG("Setting PTZ mode (%d).\n", mode);

	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (mode < AG_PTZ_MODE_NUM) {
		ptz->mode = ptz_int_tbl[mode];
	} else {
		PR_ERR("Unsupport PTZ mode (%d).\n", mode);
		return -1;
	}

	if (aux_set_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to set AGTX_CMD_VIDEO_PTZ_CONF.\n");
		return -1;
	}

	return 0;
}

BOOL_T IPC_APP_get_ptz_mode(void)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	switch (ptz->mode) {
	case AGTX_VIDEO_PTZ_MODE_MANUAL:
		return FALSE;
		break;
	case AGTX_VIDEO_PTZ_MODE_AUTO:
		return TRUE;
		break;
	default:
		return FALSE;
		break;
	}

	return FALSE;
}
#endif

void loadPtzSetting(void)
{
	TUYA_AG_CONF_S *conf = AG_GET_CONF();
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (aux_get_cc_config(AGTX_CMD_VIDEO_PTZ_CONF, ptz) < 0) {
		PR_ERR("Failed to get AGTX_CMD_VIDEO_PTZ_CONF.\n");
		return;
	}

	conf->ptz.update = 1;

	active.dp161_motion_tracking = IPC_APP_get_ptz_mode();
	requested.dp161_motion_tracking = active.dp161_motion_tracking;
	acknowledged.dp161_motion_tracking = active.dp161_motion_tracking;

	PR_DEBUG("requested.dp161_motion_tracking = %d.\n", requested.dp161_motion_tracking);
	PR_DEBUG("acknowledged.dp161_motion_tracking = %d.\n", acknowledged.dp161_motion_tracking);
	PR_DEBUG("active.dp161_motion_tracking = %d.\n", active.dp161_motion_tracking);
}

void *threadDataPoint(void *arg __attribute__((unused)))
{
	int err;

	while (1) {
		/* ===== TUYA_DP_MOTION_TRACKING ===== */
		acknowledged.dp161_motion_tracking = requested.dp161_motion_tracking;
		if(acknowledged.dp161_motion_tracking != active.dp161_motion_tracking) {
			if(acknowledged.dp161_motion_tracking) {
				err = IPC_APP_set_ptz_mode(AGTX_VIDEO_PTZ_MODE_AUTO);
			} else {
				err = IPC_APP_set_ptz_mode(AGTX_VIDEO_PTZ_MODE_MANUAL);
			}
			if(err) {
				reportDataPointBool(TUYA_DP_MOTION_TRACKING, active.dp161_motion_tracking);
				requested.dp161_motion_tracking = active.dp161_motion_tracking;
				acknowledged.dp161_motion_tracking = active.dp161_motion_tracking;
				PR_DEBUG("fall-back to active.dp161_motion_tracking = %d.\n", active.dp161_motion_tracking);
			} else {
				active.dp161_motion_tracking = acknowledged.dp161_motion_tracking;
			}
			/* av_main does not need to restart, so no need to sleep */
		}
		/* ===== TUYA_DP_WATERMARK ===== */
		acknowledged.dp104_basic_osd = requested.dp104_basic_osd;
		if(acknowledged.dp104_basic_osd != active.dp104_basic_osd) {
			err = IPC_APP_set_watermark_onoff(acknowledged.dp104_basic_osd);
			if(err) {
				reportDataPointBool(TUYA_DP_MOTION_TRACKING, active.dp104_basic_osd);
				requested.dp104_basic_osd = active.dp104_basic_osd;
				acknowledged.dp104_basic_osd = active.dp104_basic_osd;
				PR_DEBUG("fall-back to active.dp104_basic_osd = %d.\n", active.dp104_basic_osd);
			} else {
				active.dp104_basic_osd = acknowledged.dp104_basic_osd;
			}
			usleep(AV_MAIN_RESTART_LATENCY);
		}
		/* ===== TUYA_DP_VIDEO_LAYOUT ===== */
		acknowledged.dp196_ipc_video_layout = requested.dp196_ipc_video_layout;
		if(acknowledged.dp196_ipc_video_layout != active.dp196_ipc_video_layout) {
			pthread_mutex_lock(&g_snapshot_mutex);
			err = IPC_APP_set_video_layout(acknowledged.dp196_ipc_video_layout);
			pthread_mutex_unlock(&g_snapshot_mutex);
			if(err) {
				respone_dp_enum(TUYA_DP_VIDEO_LAYOUT, dp_video_layout_str[active.dp196_ipc_video_layout]);
				requested.dp196_ipc_video_layout = active.dp196_ipc_video_layout;
				acknowledged.dp196_ipc_video_layout = active.dp196_ipc_video_layout;
				PR_DEBUG("fall-back to active.dp196_ipc_video_layout = %d.\n", active.dp196_ipc_video_layout);
			} else {
				active.dp196_ipc_video_layout = acknowledged.dp196_ipc_video_layout;
			}
			usleep(AV_MAIN_RESTART_LATENCY);
		}
		/* ===== TUYA_DP_PTZ_CONTROL & TUYA_DP_PTZ_STOP ===== */
		int sampled_ptz_direction = g_ptz_direction;
		if(g_ptz_count == 0) {
			if(sampled_ptz_direction != PTZ_DIR_NONE) {
				//PR_INFO("inform cc ptz dir = %d, count = %d\n", sampled_ptz_direction, g_ptz_count);
				g_ptz_active_direction = sampled_ptz_direction;
				IPC_APP_ptz_start_move(sampled_ptz_direction);
				g_ptz_count++;
			} else {
				/* do nothing */
			}
		} else if (g_ptz_count < 5) {
			g_ptz_count++;
		} else {
			if(sampled_ptz_direction == g_ptz_active_direction) {
				g_ptz_count++;
			} else if (sampled_ptz_direction != PTZ_DIR_NONE) {
				//PR_INFO("inform cc ptz dir = %d, count = %d\n", sampled_ptz_direction, g_ptz_count);
				g_ptz_active_direction = sampled_ptz_direction;
				IPC_APP_ptz_start_move(sampled_ptz_direction);
				g_ptz_count = 1;
			} else {
				//PR_INFO("inform cc ptz dir = %d, count = %d\n", sampled_ptz_direction, g_ptz_count);
				g_ptz_active_direction = PTZ_DIR_NONE;
				IPC_APP_ptz_stop_move();
				g_ptz_count = 0;
			}
		}
#if 0 //TUYA_DP_PTZ_ZOOM_CONTROL & TUYA_DP_PTZ_ZOOM_STOP
		/* ===== TUYA_DP_PTZ_ZOOM_CONTROL & TUYA_DP_PTZ_ZOOM_STOP ===== */
		int sampled_zoom = g_zoom;
		if(g_zoom_count == 0) {
			if(sampled_zoom != ZOOM_NONE) {
				//PR_INFO("inform cc zoom = %d, count = %d\n", sampled_zoom, g_zoom_count);
				g_active_zoom = sampled_zoom;
				IPC_APP_ptz_start_zoom(sampled_zoom);
				g_zoom_count++;
			} else {
				/* do nothing */
			}
		} else if (g_zoom_count < 3) {
			g_zoom_count++;
		} else {
			if(sampled_zoom == g_active_zoom) {
				g_zoom_count++;
			} else if (sampled_zoom != ZOOM_NONE) {
				//PR_INFO("inform cc zoom = %d, count = %d\n", sampled_zoom, g_zoom_count);
				g_active_zoom = sampled_zoom;
				IPC_APP_ptz_start_zoom(sampled_zoom);
				g_zoom_count = 1;
			} else {
				//PR_INFO("inform cc zoom = %d, count = %d\n", sampled_zoom, g_zoom_count);
				g_active_zoom = ZOOM_NONE;
				IPC_APP_ptz_stop_zoom();
				g_zoom_count = 0;
			}
		}
#endif //TUYA_DP_PTZ_ZOOM_CONTROL & TUYA_DP_PTZ_ZOOM_STOP
		/* ===== TUYA_DP_IPC_OBJECT_OUTLINE ===== */
		acknowledged.dp198_ipc_object_outline = requested.dp198_ipc_object_outline;
		if(acknowledged.dp198_ipc_object_outline != active.dp198_ipc_object_outline) {
			err = IPC_APP_set_ipc_object_outline(acknowledged.dp198_ipc_object_outline);
			if(err) {
				reportDataPointBool(TUYA_DP_MOTION_TRACKING, active.dp198_ipc_object_outline);
				requested.dp198_ipc_object_outline = active.dp198_ipc_object_outline;
				acknowledged.dp198_ipc_object_outline = active.dp198_ipc_object_outline;
				PR_DEBUG("fall-back to active.dp104_basic_osd = %d.\n", active.dp104_basic_osd);
			} else {
				active.dp198_ipc_object_outline = acknowledged.dp198_ipc_object_outline;
			}
			//usleep(AV_MAIN_RESTART_LATENCY);
		}
		/* ===== TUYA_DP_IPC_AUTO_SIREN ===== */
		acknowledged.dp120_ipc_auto_siren = requested.dp120_ipc_auto_siren;
		if(acknowledged.dp120_ipc_auto_siren != active.dp120_ipc_auto_siren) {
			err = IPC_APP_set_ipc_auto_siren(acknowledged.dp120_ipc_auto_siren);
			if(err) {
				reportDataPointBool(TUYA_DP_IPC_AUTO_SIREN, active.dp120_ipc_auto_siren);
				requested.dp120_ipc_auto_siren = active.dp120_ipc_auto_siren;
				acknowledged.dp120_ipc_auto_siren = active.dp120_ipc_auto_siren;
				PR_DEBUG("fall-back to active.dp120_ipc_auto_siren = %d.\n", active.dp120_ipc_auto_siren);
			} else {
				active.dp120_ipc_auto_siren = acknowledged.dp120_ipc_auto_siren;
			}
		}
		/* ===== Sleep before next round ===== */
		usleep(100*1000);
	}

	pthread_exit(0);
}

#if defined(WIFI_GW) && (WIFI_GW == 1)
#ifdef TUYA_DP_AP_MODE
STATIC CHAR_T response[128] = { 0 };
CHAR_T *IPC_APP_get_ap_mode(VOID)
{
	CHAR_T ap_ssid[WIFI_SSID_LEN + 1] = { 0 };
	CHAR_T ap_pw[WIFI_PASSWD_LEN + 1] = { 0 };
	WF_WK_MD_E cur_mode = WWM_STATION;
	NW_MAC_S mac = { { 0 } };
	hwl_wf_wk_mode_get(&cur_mode);
	INT_T is_ap = 0;

	OPERATE_RET op_ret = hwl_wf_get_mac(WF_AP, &mac);
	if (OPRT_OK != op_ret) {
		strncpy(ap_ssid, "TUYA_IPC_AP", WIFI_SSID_LEN);
	} else {
		snprintf(ap_ssid, WIFI_SSID_LEN, "TUYA_IPC-%02X%02X", mac.mac[4], mac.mac[5]);
	}
	//{ is_ap: 1, ap_ssid: "xxxx"}

	if (cur_mode == WWM_SOFTAP) {
		is_ap = 1;
	}
	__tuya_app_read_STR("tuya_ap_passwd", ap_pw, WIFI_PASSWD_LEN);

	snprintf(response, 128, "{\\\"is_ap\\\":%d,\\\"ap_ssid\\\":\\\"%s\\\",\\\"password\\\":\\\"%s\\\"}", is_ap,
	         ap_ssid, ap_pw);
	return response;
}
#endif
#ifdef TUYA_DP_AP_SWITCH
VOID *__ap_change_thread(void *arg)
{
	WF_WK_MD_E cur_mode = WWM_STATION;
	hwl_wf_wk_mode_get(&cur_mode);
	WF_AP_CFG_IF_S ap_cfg;
	INT_T ap_onoff = 0;
	sleep(5);

	ap_onoff = __tuya_app_read_INT("tuya_ap_on_off");
	memset(&ap_cfg, 0, SIZEOF(WF_AP_CFG_IF_S));
	printf("%s : ap_onoff:%d\n", __func__, ap_onoff);
	if (ap_onoff) {
		//ap, sta, monitor, do same
		__tuya_app_read_STR("tuya_ap_ssid", ap_cfg.ssid, WIFI_SSID_LEN);
		__tuya_app_read_STR("tuya_ap_passwd", ap_cfg.passwd, WIFI_PASSWD_LEN);

		ap_cfg.s_len = strlen((CHAR_T *)(ap_cfg.ssid));
		ap_cfg.ssid_hidden = 0;
		ap_cfg.p_len = strlen((CHAR_T *)(ap_cfg.passwd));
		if (ap_cfg.p_len == 0) {
			ap_cfg.md = WAAM_OPEN;
		} else {
			ap_cfg.md = WAAM_WPA2_PSK;
		}
		hwl_wf_ap_start(&ap_cfg);
	} else {
		if (cur_mode == WWM_SOFTAP) {
			hwl_wf_ap_stop();
			tuya_ipc_reconnect_wifi();
		} else {
			//not ap mode,do nothing
		}
	}
	return NULL;
}

VOID change_ap_process()
{
	pthread_t ap_change_thread;
	int ret = pthread_create(&ap_change_thread, NULL, __ap_change_thread, NULL);
	if (ret != 0) {
		printf("ap_change_thread ,create fail! ret:%d\n", ret);
	}

	pthread_detach(ap_change_thread);
}

INT_T IPC_APP_set_ap_mode(IN cJSON *p_ap_info)
{
	if (NULL == p_ap_info) {
		return 0;
	}

	INT_T ap_onoff = -1;

	printf("%s %d handle_DP_AP_SWITCH:%s \r\n", __FUNCTION__, __LINE__, (char *)p_ap_info);

	cJSON *pJson = cJSON_Parse((CHAR_T *)p_ap_info);

	if (NULL == pJson) {
		printf("%s %d step error\n", __FUNCTION__, __LINE__);
		return ap_onoff;
	}
	//{ ap_enable : 1, ap_ssid : xxxx, ap_pwd : xxx }
	cJSON *tmp = cJSON_GetObjectItem(pJson, "ap_enable");
	if (NULL == tmp) {
		printf("%s %d step error\n", __FUNCTION__, __LINE__);
		cJSON_Delete(pJson);
		return ap_onoff;
	}
	ap_onoff = tmp->valueint;
	__tuya_app_write_INT("tuya_ap_on_off", &ap_onoff);

	if (ap_onoff == 1) {
		cJSON *tmpSsid = cJSON_GetObjectItem(pJson, "ap_ssid");
		if (NULL == tmpSsid) {
			printf("%s %d step error\n", __FUNCTION__, __LINE__);
			cJSON_Delete(pJson);
			return ap_onoff;
		}
		printf("###[%d] get ssid:%s\n", ap_onoff, tmpSsid->valuestring);

		cJSON *tmpPwd = cJSON_GetObjectItem(pJson, "ap_pwd");
		if (NULL == tmpPwd) {
			printf("%s %d step error\n", __FUNCTION__, __LINE__);
			cJSON_Delete(pJson);
			return ap_onoff;
		}
		printf("###get pwd:%s\n", tmpPwd->valuestring);

		__tuya_app_write_STR("tuya_ap_ssid", tmpSsid->valuestring);
		__tuya_app_write_STR("tuya_ap_passwd", tmpPwd->valuestring);
		tuya_ipc_save_ap_info(tmpSsid->valuestring, tmpPwd->valuestring);
	}

	cJSON_Delete(pJson);

	return ap_onoff;
}
#endif
#endif
