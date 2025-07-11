/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * Copyright (C) 2019 Augentix Inc. - All Rights Reserved
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 */

/**
 * @file tuya_ipc_dp_handler.h
 * @brief
 */

#ifndef TUYA_IPC_DP_HANDLER_H_
#define TUYA_IPC_DP_HANDLER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "agtx_video_strm_conf.h"
#include "agtx_osd_conf.h"
#include "agtx_iva_od_conf.h"
#include "agtx_iva_md_conf.h"
#include "agtx_img_pref.h"
#include "agtx_adv_img_pref.h"
#include "agtx_video_ptz_conf.h"
#include "agtx_video_layout_conf.h"
#include "agtx_siren_conf.h"
#include "agtx_voice_conf.h"
#include "agtx_local_record_conf.h"

#include "tuya_cloud_types.h"
#include "tuya_ipc_dp_utils.h"
#include "tuya_ipc_ptz.h"

#include <json.h>
#include <sys/socket.h>
#include <sys/un.h>

#define auxCmdTransSocketFile "/tmp/ccUnxSkt"

int aux_json_validation(char *buffer, int strLen);
int aux_json_get_int(char *buffer, char *dKey, int strLen);
void AG_getCCReply(char *text);

typedef struct {
	int cmd;
	void* data;
	int *update;
} AG_CMD_DATA_S;

typedef struct {
	int update;
	AGTX_STRM_CONF_S data;
} TUYA_STRM_CONF_S;

typedef struct {
	int update;
	AGTX_IMG_PREF_S data;
} TUYA_IMG_PREF_S;

typedef struct {
	int update;
	AGTX_ADV_IMG_PREF_S data;
} TUYA_ADV_IMG_PREF_S;

typedef struct {
	int update;
	AGTX_OSD_CONF_S data;
} TUYA_OSD_CONF_S;

typedef struct {
	int update;
	AGTX_IVA_OD_CONF_S data;
} TUYA_IVA_OD_CONF_S;

typedef struct {
	int update;
	AGTX_IVA_MD_CONF_S data;
} TUYA_IVA_MD_CONF_S;

typedef struct {
	int update;
	AGTX_VIDEO_PTZ_CONF_S data;
} TUYA_VIDEO_PTZ_CONF_S;

typedef struct {
	int update;
	AGTX_VOICE_CONF_S data;
} TUYA_VOICE_CONF_S;

typedef struct {
	int update;
	AGTX_SIREN_CONF_S data;
} TUYA_SIREN_CONF_S;

typedef struct {
	int update;
	AGTX_LOCAL_RECORD_CONF_S data;
} TUYA_LOCAL_RECORD_CONF_S;

typedef struct {
	int update;
	AGTX_LAYOUT_CONF_S data;
} TUYA_LAYOUT_CONF_S;

typedef struct {
	TUYA_STRM_CONF_S strm;
	TUYA_LAYOUT_CONF_S layout;
	TUYA_IMG_PREF_S img;
	TUYA_ADV_IMG_PREF_S pref;
	TUYA_OSD_CONF_S osd;
	TUYA_IVA_OD_CONF_S od;
	TUYA_IVA_MD_CONF_S md;
	TUYA_VIDEO_PTZ_CONF_S ptz;
	TUYA_VOICE_CONF_S voice;
	TUYA_SIREN_CONF_S siren;
	TUYA_LOCAL_RECORD_CONF_S local_record;
} TUYA_AG_CONF_S;

typedef struct state_data_point {
	BOOL_T dp104_basic_osd;
	BOOL_T dp120_ipc_auto_siren;
	BOOL_T dp161_motion_tracking;
	UINT_T dp196_ipc_video_layout;
	BOOL_T dp198_ipc_object_outline;
} StateDataPoint;

VOID *AG_Connect_CC(VOID *data);
INT_T AG_Collect_Conf(VOID);
INT_T AG_Get_Conf(TUYA_AG_CONF_S **conf);

#ifdef TUYA_DP_DEVICE_RESTART
VOID IPC_APP_restart_device(VOID);
#endif

#ifdef TUYA_DP_SLEEP_MODE
VOID IPC_APP_set_sleep_mode(BOOL_T sleep_mode);
BOOL_T IPC_APP_get_sleep_mode(VOID);
#endif

#ifdef TUYA_DP_LIGHT
VOID IPC_APP_set_light_onoff(BOOL_T light_on_off);
BOOL_T IPC_APP_get_light_onoff(VOID);
#endif

#ifdef TUYA_DP_FLIP
VOID IPC_APP_set_flip_onoff(BOOL_T flip_on_off);
BOOL_T IPC_APP_get_flip_onoff(VOID);
#endif

#ifdef TUYA_DP_WATERMARK
void loadDataPointBasicOsd(void);
void handleDataPointBasicOsd(TY_OBJ_DP_S *dp);
int IPC_APP_set_watermark_onoff(BOOL_T enabled);
BOOL_T IPC_APP_get_watermark_onoff(VOID);
#endif

#ifdef TUYA_DP_BRIGHTNESS
VOID IPC_APP_set_brightness(UINT_T brightness);
UINT_T IPC_APP_get_brightness(VOID);
#endif

#ifdef TUYA_DP_CONTRAST
VOID IPC_APP_set_contrast(UINT_T contrast);
UINT_T IPC_APP_get_contrast(VOID);
#endif

#ifdef TUYA_DP_EXPOSURE
VOID IPC_APP_set_exposure(UINT_T exp_mode);
CHAR_T* IPC_APP_get_exposure(VOID);
#endif

#ifdef TUYA_DP_WDR
VOID IPC_APP_set_wdr_onoff(BOOL_T wdr_on_off);
BOOL_T IPC_APP_get_wdr_onoff(VOID);
#endif

#ifdef TUYA_DP_NIGHT_MODE
VOID IPC_APP_set_night_mode(UINT_T night_mode);
CHAR_T *IPC_APP_get_night_mode(VOID);
#endif

#ifdef TUYA_DP_ALARM_FUNCTION
VOID IPC_APP_set_alarm_function_onoff(BOOL_T alarm_on_off);
BOOL_T IPC_APP_get_alarm_function_onoff(VOID);
#endif

#ifdef TUYA_DP_ALARM_SENSITIVITY
VOID IPC_APP_set_alarm_sensitivity(UINT_T sensitivity);
CHAR_T *IPC_APP_get_alarm_sensitivity(VOID);
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW
VOID IPC_APP_set_alarm_zone_draw(cJSON *p_alarm_zone);
char *IPC_APP_get_alarm_zone_draw(VOID);
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
VOID IPC_APP_set_alarm_zone_onoff(BOOL_T alarm_zone_on_off);
BOOL_T IPC_APP_get_alarm_zone_onoff(VOID);
#endif

//#ifdef TUYA_DP_ALARM_INTERVAL
//VOID IPC_APP_set_alarm_interval(CHAR_T *p_interval);
//CHAR_T *IPC_APP_get_alarm_interval(VOID);
//#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_status(VOID);
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
VOID IPC_APP_get_sd_storage(UINT_T *p_total, UINT_T *p_used, UINT_T *p_empty);
#endif

#ifdef TUYA_DP_SD_RECORD_ENABLE
VOID IPC_APP_set_sd_record_onoff(BOOL_T sd_record_on_off);
BOOL_T IPC_APP_get_sd_record_onoff(VOID);
#endif

#ifdef TUYA_DP_SD_RECORD_MODE
VOID IPC_APP_set_sd_record_mode(UINT_T sd_record_mode);
UINT_T IPC_APP_get_sd_record_mode(VOID);
#endif

#ifdef TUYA_DP_SD_UMOUNT
BOOL_T IPC_APP_unmount_sd_card(VOID);
#endif

#ifdef TUYA_DP_SD_FORMAT
VOID IPC_APP_format_sd_card(VOID);
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
INT_T IPC_APP_get_sd_format_status(VOID);
#endif

#ifdef TUYA_DP_PTZ_CONTROL
VOID IPC_APP_ptz_start_move(UINT_T dir);
#endif

#ifdef TUYA_DP_PTZ_STOP
VOID IPC_APP_ptz_stop_move(VOID);
#endif

#ifdef TUYA_DP_PTZ_ZOOM_CONTROL
VOID IPC_APP_ptz_start_zoom(INT_T p_zoom);
#endif

#ifdef TUYA_DP_PTZ_ZOOM_STOP
VOID IPC_APP_ptz_stop_zoom(VOID);
#endif

#ifdef TUYA_DP_PTZ_CHECK
void IPC_APP_ptz_check(VOID);
#endif

#ifdef TUYA_DP_MOTION_TRACKING
void IPC_APP_motion_tracking(BOOL_T motion_tracking);
BOOL_T IPC_APP_get_motion_tracking(void);
#endif
#ifdef TUYA_DP_LINK_MOVE_ACTION
VOID IPC_APP_set_link_move(INT_T bind_seq);
#endif

#ifdef TUYA_DP_LINK_MOVE_SET
VOID IPC_APP_set_link_pos(INT_T bind_seq);
#endif

#ifdef TUYA_DP_HUM_FILTER
void IPC_APP_human_filter(BOOL_T filter_enable);
#endif

#ifdef TUYA_DP_PATROL_MODE
void IPC_APP_set_patrol_mode(BOOL_T patrol_mode);
char IPC_APP_get_patrol_mode(void);

#endif

#ifdef TUYA_DP_PATROL_SWITCH
void IPC_APP_set_patrol_switch(BOOL_T patrol_switch);

BOOL_T IPC_APP_get_patrol_switch(void);

void IPC_APP_ptz_preset_reset(S_PRESET_CFG *preset_cfg);

#endif

#ifdef TUYA_DP_PATROL_TMODE
void IPC_APP_set_patrol_tmode(BOOL_T patrol_tmode);

char IPC_APP_get_patrol_tmode(void);
#endif

#ifdef TUYA_DP_PATROL_TIME
void IPC_APP_set_patrol_time(cJSON *p_patrol_time);
#endif

#ifdef TUYA_DP_PRESET_SET
void IPC_APP_set_preset(cJSON *p_preset_param);

#endif

#ifdef TUYA_DP_PATROL_STATE
void IPC_APP_patrol_state(int *patrol_state);
#endif

#ifdef TUYA_DP_BLUB_SWITCH
VOID IPC_APP_set_blub_onoff(BOOL_T blub_on_off);
BOOL_T IPC_APP_get_blub_onoff(VOID);
#endif

#ifdef TUYA_DP_ELECTRICITY
INT_T IPC_APP_get_battery_percent(VOID);
#endif

#ifdef TUYA_DP_POWERMODE
CHAR_T *IPC_APP_get_power_mode(VOID);
#endif

#ifdef TUYA_DP_SIREN_SWITCH
void IPC_APP_set_siren_switch(BOOL_T siren_switch);
BOOL_T IPC_APP_get_siren_switch(void);
#endif

#ifdef TUYA_DP_SIREN_VOLUME
VOID IPC_APP_set_siren_volume(UINT_T volume);
UINT_T IPC_APP_get_siren_volume(VOID);
#endif

#ifdef TUYA_DP_SIREN_INTERVAL
VOID IPC_APP_set_siren_interval(UINT_T interval_mode);
UINT_T IPC_APP_get_siren_interval(VOID);
#endif

#ifdef TUYA_DP_VIDEO_LAYOUT
int IPC_APP_set_video_layout(UINT_T mode);
UINT_T IPC_APP_get_video_layout(VOID);
void handle_DP_VIDEO_LAYOUT(TY_OBJ_DP_S *dp);
#endif

VOID IPC_APP_set_bitrate(int channel, int bit_rate);

#ifdef TUYA_DP_MOTION_TRACKING
void handleDataPointMotionTracking(TY_OBJ_DP_S *dp);
#endif

#ifdef TUYA_DP_IPC_AUTO_SIREN
void loadDataPointIpcAutoSiren(void);
void handleDataPointIpcAutoSiren(TY_OBJ_DP_S *dp);
int IPC_APP_set_ipc_auto_siren(BOOL_T enabled);
BOOL_T IPC_APP_get_ipc_auto_siren(VOID);
#endif
#ifdef TUYA_DP_IPC_OBJECT_OUTLINE
void loadDataPointIpcObjectOutline(void);
void handleDataPointIpcObjectOutline(TY_OBJ_DP_S *dp);
int IPC_APP_set_ipc_object_outline(BOOL_T enabled);
BOOL_T IPC_APP_get_ipc_object_outline(VOID);
#endif

#ifdef TUYA_DP_AP_MODE
CHAR_T *IPC_APP_get_ap_mode(VOID);
#endif

#ifdef TUYA_DP_AP_SWITCH
VOID change_ap_process();
INT_T IPC_APP_set_ap_mode(IN cJSON *p_ap_info);
#endif

void loadPtzSetting(void);
void loadVideoLayoutSetting(void);

void *threadDataPoint(void *arg);

#ifdef __cplusplus
}
#endif

#endif /* TUYA_IPC_DP_HANDLER_H_ */
