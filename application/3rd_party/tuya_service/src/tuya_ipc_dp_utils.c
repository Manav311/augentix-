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

#include "tuya_ipc_define.h"
#include "tuya_ipc_api.h"
#include "tuya_ipc_dp_utils.h"
#include "tuya_ipc_dp_handler.h"
#include "tuya_cloud_com_defs.h"
#include "tuya_ipc_common_demo.h"
#include "tuya_ipc_motion_detect_demo.h"
#include "tuya_ipc_system_control_demo.h"
#include "agtx_cmd.h"

#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <sys/time.h>
//Globals:

int cmdTransFD;
int g_master_id;

extern StateDataPoint requested;
extern StateDataPoint acknowledged;
extern StateDataPoint active;

static char * dp_video_layout_str[] = { "0", "1", "2" };

STATIC VOID handle_DP_SD_STORAGE_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp);
//------------------------------------------

#ifdef TUYA_DP_MOTION_DIR
#define FOREACH_MOTION_DIR_MODE(MVD_INT) \
	MVD_INT(NONE, "0") \
	MVD_INT(LEFT, "1") \
	MVD_INT(RIGHT, "2") \

#define GENERATE_MOTION_DIR_ENUM(a, b)  AG_MV_DIR_##a,
#define GENERATE_MOTION_DIR_NAME(a, b)  b,

typedef enum {
	FOREACH_MOTION_DIR_MODE(GENERATE_MOTION_DIR_ENUM)
	AG_MVD_MODE_NUM
} AG_MV_DIR_E;

static char* mvd_int_name[] = {
	FOREACH_MOTION_DIR_MODE(GENERATE_MOTION_DIR_NAME)
};

void IPC_APP_report_motion_direction(INT_T motion_dir)
{
	respone_dp_enum(TUYA_DP_MOTION_DIR, mvd_int_name[motion_dir]);
}
#endif /* !TUYA_DP_MOTION_DIR */

#ifdef TUYA_DP_MOTION_REG_LEFT
#define FOREACH_MOTION_REGL_MODE(MVRL_INT) \
	MVRL_INT(NONE, "0") \
	MVRL_INT(LEFT, "1") \

#define GENERATE_MOTION_REGL_ENUM(a, b)  AG_MV_REGL_##a,
#define GENERATE_MOTION_REGL_NAME(a, b)  b,

typedef enum {
	FOREACH_MOTION_REGL_MODE(GENERATE_MOTION_REGL_ENUM)
	AG_MV_REGL_NUM
} AG_MV_REGL_E;

static char* mvrl_int_name[] = {
	FOREACH_MOTION_REGL_MODE(GENERATE_MOTION_REGL_NAME)
};

void IPC_APP_report_motion_reg_left(INT_T motion_reg_left)
{
	respone_dp_enum(TUYA_DP_MOTION_REG_LEFT, mvrl_int_name[motion_reg_left]);
}
#endif /* !TUYA_DP_MOTION_REG_LEFT */

#ifdef TUYA_DP_MOTION_REG_RIGHT
#define FOREACH_MOTION_REGR_MODE(MVRR_INT) \
	MVRR_INT(NONE, "0") \
	MVRR_INT(RIGHT, "1") \

#define GENERATE_MOTION_REGR_ENUM(a, b)  AG_MV_REGR_##a,
#define GENERATE_MOTION_REGR_NAME(a, b)  b,

typedef enum {
	FOREACH_MOTION_REGR_MODE(GENERATE_MOTION_REGR_ENUM)
	AG_MV_REGR_NUM
} AG_MV_REGR_E;

static char* mvrr_int_name[] = {
	FOREACH_MOTION_REGR_MODE(GENERATE_MOTION_REGR_NAME)
};

void IPC_APP_report_motion_reg_right(INT_T motion_reg_right)
{
	respone_dp_enum(TUYA_DP_MOTION_REG_RIGHT, mvrr_int_name[motion_reg_right]);
}
#endif /* !TUYA_DP_MOTION_REG_RIGHT */

//------------------------------------------
VOID IPC_APP_upload_all_status(VOID)
{
#ifdef TUYA_DP_SLEEP_MODE
	respone_dp_bool(TUYA_DP_SLEEP_MODE, IPC_APP_get_sleep_mode());
#endif

#ifdef TUYA_DP_LIGHT
	respone_dp_bool(TUYA_DP_LIGHT, IPC_APP_get_light_onoff());
#endif

#ifdef TUYA_DP_FLIP
	respone_dp_bool(TUYA_DP_FLIP, IPC_APP_get_flip_onoff());
#endif

#ifdef TUYA_DP_WATERMARK
	reportDataPointBool(TUYA_DP_WATERMARK, active.dp104_basic_osd);
#endif

#ifdef TUYA_DP_IPC_AUTO_SIREN
	reportDataPointBool(TUYA_DP_IPC_AUTO_SIREN, active.dp120_ipc_auto_siren);
#endif

#ifdef TUYA_DP_BRIGHTNESS
	respone_dp_value(TUYA_DP_BRIGHTNESS, IPC_APP_get_brightness());
#endif

#ifdef TUYA_DP_CONTRAST
	respone_dp_value(TUYA_DP_CONTRAST, IPC_APP_get_contrast());
#endif

#ifdef TUYA_DP_WDR
	respone_dp_bool(TUYA_DP_WDR, IPC_APP_get_wdr_onoff());
#endif

#ifdef TUYA_DP_NIGHT_MODE
	respone_dp_enum(TUYA_DP_NIGHT_MODE, IPC_APP_get_night_mode());
#endif

#ifdef TUYA_DP_ALARM_FUNCTION
	respone_dp_bool(TUYA_DP_ALARM_FUNCTION, IPC_APP_get_alarm_function_onoff());
#endif

#ifdef TUYA_DP_ALARM_SENSITIVITY
	respone_dp_enum(TUYA_DP_ALARM_SENSITIVITY, IPC_APP_get_alarm_sensitivity());
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
	respone_dp_bool(TUYA_DP_ALARM_ZONE_ENABLE, IPC_APP_get_alarm_zone_onoff());
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW
	respone_dp_str(TUYA_DP_ALARM_ZONE_DRAW, IPC_APP_get_alarm_zone_draw());
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
	respone_dp_value(TUYA_DP_SD_STATUS_ONLY_GET, IPC_APP_get_sd_status());
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
	handle_DP_SD_STORAGE_ONLY_GET(NULL);
#endif

#ifdef TUYA_DP_SD_RECORD_ENABLE
	respone_dp_bool(TUYA_DP_SD_RECORD_ENABLE, IPC_APP_get_sd_record_onoff());
#endif

#ifdef TUYA_DP_SD_RECORD_MODE
	CHAR_T sd_mode[4];
	snprintf(sd_mode, 4, "%d", IPC_APP_get_sd_record_mode());
	respone_dp_enum(TUYA_DP_SD_RECORD_MODE, sd_mode);
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
	respone_dp_value(TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, 0);
#endif

#ifdef TUYA_DP_PTZ_MODE
	respone_dp_enum(TUYA_DP_PTZ_MODE, IPC_APP_get_ptz_mode());
#endif

#ifdef TUYA_DP_MOTION_TRACKING
	reportDataPointBool(TUYA_DP_MOTION_TRACKING, active.dp161_motion_tracking);
#endif

#ifdef TUYA_DP_PATROL_SWITCH
	respone_dp_bool(TUYA_DP_PATROL_SWITCH, IPC_APP_get_patrol_switch());
#endif

#ifdef TUYA_DP_BLUB_SWITCH
	respone_dp_bool(TUYA_DP_BLUB_SWITCH, IPC_APP_get_blub_onoff());
#endif

#ifdef TUYA_DP_POWERMODE
	IPC_APP_update_battery_status();
#endif

#ifdef TUYA_DP_SIREN_SWITCH
	respone_dp_bool(TUYA_DP_SIREN_SWITCH, IPC_APP_get_siren_switch());
#endif

#ifdef TUYA_DP_SIREN_VOLUME
	respone_dp_value(TUYA_DP_SIREN_VOLUME, IPC_APP_get_siren_volume());
#endif

#ifdef TUYA_DP_SIREN_INTERVAL
	respone_dp_value(TUYA_DP_SIREN_INTERVAL, IPC_APP_get_siren_interval());
#endif

#ifdef TUYA_DP_VIDEO_LAYOUT
	respone_dp_enum(TUYA_DP_VIDEO_LAYOUT, dp_video_layout_str[active.dp196_ipc_video_layout]);
#endif

#ifdef TUYA_DP_IPC_OBJECT_OUTLINE
	reportDataPointBool(TUYA_DP_IPC_OBJECT_OUTLINE, active.dp198_ipc_object_outline);
#endif

#ifdef TUYA_DP_MOTION_DIR
	respone_dp_enum(TUYA_DP_MOTION_DIR, mvd_int_name[0]);
#endif /* !TUYA_DP_MOTION_DIR */

#ifdef TUYA_DP_MOTION_REG_LEFT
	respone_dp_enum(TUYA_DP_MOTION_REG_LEFT, mvrl_int_name[0]);
#endif /* !TUYA_DP_MOTION_REG_LEFT */

#ifdef TUYA_DP_MOTION_REG_RIGHT
	respone_dp_enum(TUYA_DP_MOTION_REG_RIGHT, mvrr_int_name[0]);
#endif /* !TUYA_DP_MOTION_REG_RIGHT */
}

#ifdef TUYA_DP_DOOR_BELL
VOID IPC_APP_trigger_door_bell(VOID)
{
	PR_INFO("door bell is triggered\n");

	BYTE_T wakeup_data_arr[10] = { 0 };
	UINT_T wakeup_data_len = 10;
	CHAR_T data[128] = { 0 };
	unsigned int wakeup_data = 0;
	tuya_iot_get_wakeup_data(wakeup_data_arr, &wakeup_data_len);
	wakeup_data = (wakeup_data_arr[8] & 0xFF) | ((wakeup_data_arr[7] << 8) & (0xFF00)) |
	              ((wakeup_data_arr[7] << 16) & (0xFF0000)) | ((wakeup_data_arr[7] << 24) & (0xFF000000));

	TIME_T timeutc = 0;
	INT_T timezone = 0;
	tuya_ipc_get_service_time(&timeutc, &timezone);
	snprintf(data, 128, "{\"etype\":\"doorbell_press\",\"edata\":\"%x%d\"}", wakeup_data, (INT_T)timeutc);
	PR_INFO("DoorBell PUSH:%s\n", data);
	tuya_iot_send_custom_mqtt_msg(43, (BYTE_T *)data);

	UINT_T intval = time(NULL);
	CHAR_T strval[64] = { 0 };
	snprintf(strval, 64, "%d", intval);
	respone_dp_str(TUYA_DP_DOOR_BELL, strval);
}
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
VOID IPC_APP_report_sd_format_status(INT_T status)
{
	respone_dp_value(TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, status);
}
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
VOID IPC_APP_report_sd_status_changed(INT_T status)
{
	respone_dp_value(TUYA_DP_SD_STATUS_ONLY_GET, status);
}
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
VOID IPC_APP_report_sd_storage()
{
	CHAR_T tmp_str[100] = { 0 };

	UINT_T total = 100;
	UINT_T used = 0;
	UINT_T empty = 100;
	IPC_APP_get_sd_storage(&total, &used, &empty);

	//"total capacity|Current usage|remaining capacity"
	snprintf(tmp_str, 100, "%u|%u|%u", total, used, empty);
	respone_dp_str(TUYA_DP_SD_STORAGE_ONLY_GET, tmp_str);
}
#endif

#ifdef TUYA_DP_POWERMODE
VOID IPC_APP_update_battery_status(VOID)
{
	CHAR_T *power_mode = IPC_APP_get_power_mode();
	INT_T percent = IPC_APP_get_battery_percent();

	PR_INFO("current power mode:%s\n", power_mode);
	respone_dp_enum(TUYA_DP_POWERMODE, power_mode);
	PR_INFO("current battery percent:%d\n", percent);
	respone_dp_value(TUYA_DP_ELECTRICITY, percent);
}
#endif


//------------------------------------------
VOID respone_dp_value(BYTE_T dp_id, INT_T val)
{
	tuya_ipc_dp_report(NULL, dp_id, PROP_VALUE, &val, 1);
}

/**
 * @brief Report boolean-type data point to server
 * @param[in] id ID of the data point
 * @param[in] value Value to be reported
 * @return None
 */
inline void reportDataPointBool(BYTE_T id, BOOL_T value)
{
	tuya_ipc_dp_report(NULL, id, PROP_BOOL, &value, 1);
}

VOID respone_dp_bool(BYTE_T dp_id, BOOL_T true_false)
{
	tuya_ipc_dp_report(NULL, dp_id, PROP_BOOL, &true_false, 1);
}

VOID respone_dp_enum(BYTE_T dp_id, CHAR_T *p_val_enum)
{
	tuya_ipc_dp_report(NULL, dp_id, PROP_ENUM, p_val_enum, 1);
}

VOID respone_dp_str(BYTE_T dp_id, CHAR_T *p_val_str)
{
	tuya_ipc_dp_report(NULL, dp_id, PROP_STR, p_val_str, 1);
}

/**
 * @brief Parse boolean-type data point
 * @param[in] dp Data point
 * @param[out] value Parsed value
 * @return
 * - 0 on success
 * - -1 when DP is null
 * - -2 when DP is not boolean type
 * - -3 when failed to parse DP value
 */
int parseDataPointBool(TY_OBJ_DP_S *dp, BOOL_T *value)
{
	if (dp == NULL) {
		PR_WARN("DP is null.\n");
		return -1;
	}

	if (dp->type != PROP_BOOL) {
		PR_WARN("DP is not boolean type.\n");
		return -2;
	}

	switch(dp->value.dp_bool) {
	case 0:
		*value = FALSE;
		break;
	case 1:
		*value = TRUE;
		break;
	default:
		*value = *value;
		PR_WARN("Failed to parse DP value. (0x%08X)\n", dp->value.dp_bool);
		return -3;
		break;
	}

	return 0;
}

STATIC BOOL_T check_dp_bool_invalid(IN TY_OBJ_DP_S *p_obj_dp)
{
	if (p_obj_dp == NULL) {
		PR_ERR("error! input is null \n");
		return -1;
	}

	if (p_obj_dp->type != PROP_BOOL) {
		PR_ERR("error! input is not bool %d \n", p_obj_dp->type);
		return -2;
	}

	if (p_obj_dp->value.dp_bool == 0) {
		return FALSE;
	} else if (p_obj_dp->value.dp_bool == 1) {
		return TRUE;
	} else {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->value.dp_bool);
		return -2;
	}
}

STATIC INT_T check_dp_value_invalid(IN TY_OBJ_DP_S *p_obj_dp)
{
	if (p_obj_dp == NULL) {
		PR_ERR("error! input is null \r\n");
		return -1;
	}

	if (p_obj_dp->type != PROP_VALUE) {
		PR_ERR("error! input is not value %d \r\n", p_obj_dp->type);
		return -2;
	}

	return p_obj_dp->value.dp_value;
}
//------------------------------------------

#ifdef TUYA_DP_SLEEP_MODE
STATIC VOID handle_DP_SLEEP_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T sleep_mode = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_sleep_mode(sleep_mode);
	sleep_mode = IPC_APP_get_sleep_mode();

	respone_dp_bool(TUYA_DP_SLEEP_MODE, sleep_mode);
}
#endif

#ifdef TUYA_DP_LIGHT
STATIC VOID handle_DP_LIGHT(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T light_on_off = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_light_onoff(light_on_off);
	light_on_off = IPC_APP_get_light_onoff();

	respone_dp_bool(TUYA_DP_LIGHT, light_on_off);
}
#endif

#ifdef TUYA_DP_FLIP
STATIC VOID handle_DP_FLIP(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T flip_on_off = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_flip_onoff(flip_on_off);
	flip_on_off = IPC_APP_get_flip_onoff();

	respone_dp_bool(TUYA_DP_FLIP, flip_on_off);
}
#endif

#ifdef TUYA_DP_BRIGHTNESS
STATIC VOID handle_DP_brightness(IN TY_OBJ_DP_S *p_obj_dp)
{
	INT_T brightness = check_dp_value_invalid(p_obj_dp);

	IPC_APP_set_brightness(brightness);
	brightness = IPC_APP_get_brightness();

	respone_dp_value(TUYA_DP_BRIGHTNESS, brightness);
}
#endif

#ifdef TUYA_DP_CONTRAST
STATIC VOID handle_DP_contrast(IN TY_OBJ_DP_S *p_obj_dp)
{
	INT_T contrast = check_dp_value_invalid(p_obj_dp);

	IPC_APP_set_contrast(contrast);
	contrast = IPC_APP_get_contrast();

	respone_dp_value(TUYA_DP_CONTRAST, contrast);
}
#endif

#ifdef TUYA_DP_EXPOSURE
STATIC VOID handle_DP_exposure(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \r\n", p_obj_dp->type);
		return;
	}

	IPC_APP_set_exposure(p_obj_dp->value.dp_enum);
	CHAR_T *p_exp = IPC_APP_get_exposure();
	respone_dp_enum(TUYA_DP_EXPOSURE, p_exp);
}
#endif

#ifdef TUYA_DP_WDR
STATIC VOID handle_DP_WDR(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T wdr_on_off = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_wdr_onoff(wdr_on_off);
	wdr_on_off = IPC_APP_get_wdr_onoff();

	respone_dp_bool(TUYA_DP_WDR, wdr_on_off);
}
#endif

#ifdef TUYA_DP_NIGHT_MODE
STATIC VOID handle_DP_NIGHT_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}

	IPC_APP_set_night_mode(p_obj_dp->value.dp_enum);
	CHAR_T *p_night_mode = IPC_APP_get_night_mode();

	respone_dp_enum(TUYA_DP_NIGHT_MODE, p_night_mode);
}
#endif

#ifdef TUYA_DP_ALARM_FUNCTION
STATIC VOID handle_DP_ALARM_FUNCTION(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T alarm_on_off = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_alarm_function_onoff(alarm_on_off);
	alarm_on_off = IPC_APP_get_alarm_function_onoff();

	respone_dp_bool(TUYA_DP_ALARM_FUNCTION, alarm_on_off);
}
#endif

#ifdef TUYA_DP_ALARM_SENSITIVITY
STATIC VOID handle_DP_ALARM_SENSITIVITY(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}

	IPC_APP_set_alarm_sensitivity(p_obj_dp->value.dp_enum);
	CHAR_T *p_sensitivity = IPC_APP_get_alarm_sensitivity();

	respone_dp_enum(TUYA_DP_ALARM_SENSITIVITY, p_sensitivity);
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_ENABLE
STATIC VOID handle_DP_ALARM_ZONE_ENABLE(IN TY_OBJ_DP_S *p_dp_json)
{
	if (p_dp_json == NULL) {
		PR_ERR("Error!! type invalid %p \n", p_dp_json);
		return;
	}
	BOOL_T alarm_zone_enable = check_dp_bool_invalid(p_dp_json);
	IPC_APP_set_alarm_zone_onoff(alarm_zone_enable);
	respone_dp_bool(TUYA_DP_ALARM_ZONE_ENABLE, IPC_APP_get_alarm_zone_onoff());
}
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW
STATIC VOID handle_DP_ALARM_ZONE_DRAW(IN TY_OBJ_DP_S *p_dp_json)
{
	if (p_dp_json == NULL) {
		PR_ERR("Error!! type invalid\n");
		return;
	}
	IPC_APP_set_alarm_zone_draw((cJSON *)(p_dp_json->value.dp_str));
	respone_dp_str(TUYA_DP_ALARM_ZONE_DRAW, IPC_APP_get_alarm_zone_draw());
}
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
STATIC VOID handle_DP_SD_STATUS_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	INT_T sd_status = IPC_APP_get_sd_status();

	respone_dp_value(TUYA_DP_SD_STATUS_ONLY_GET, sd_status);
}
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
STATIC VOID handle_DP_SD_STORAGE_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	CHAR_T tmp_str[100] = { 0 };

	UINT_T total = 100;
	UINT_T used = 0;
	UINT_T empty = 100;
	IPC_APP_get_sd_storage(&total, &used, &empty);

	//"total capacity|Current usage|remaining capacity"
	snprintf(tmp_str, 100, "%u|%u|%u", total, used, empty);
	respone_dp_str(TUYA_DP_SD_STORAGE_ONLY_GET, tmp_str);
}
#endif

#ifdef TUYA_DP_SD_RECORD_ENABLE
STATIC VOID handle_DP_SD_RECORD_ENABLE(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T sd_record_on_off = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_sd_record_onoff(sd_record_on_off);
	sd_record_on_off = IPC_APP_get_sd_record_onoff();

	respone_dp_bool(TUYA_DP_SD_RECORD_ENABLE, sd_record_on_off);
}
#endif

#ifdef TUYA_DP_SD_RECORD_MODE
STATIC VOID handle_DP_SD_RECORD_MODE(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}

	IPC_APP_set_sd_record_mode(p_obj_dp->value.dp_enum);
	UINT_T mode = IPC_APP_get_sd_record_mode();
	CHAR_T sMode[2];
	snprintf(sMode, 2, "%d", mode);
	respone_dp_enum(TUYA_DP_SD_RECORD_MODE, sMode);
}
#endif

#ifdef TUYA_DP_SD_UMOUNT
STATIC VOID handle_DP_SD_UMOUNT(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	BOOL_T umount_result = IPC_APP_unmount_sd_card();
	respone_dp_bool(TUYA_DP_SD_UMOUNT, umount_result);
}
#endif

#ifdef TUYA_DP_SD_FORMAT
STATIC VOID handle_DP_SD_FORMAT(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	IPC_APP_format_sd_card();
	respone_dp_bool(TUYA_DP_SD_FORMAT, TRUE);
}
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
STATIC VOID handle_DP_SD_FORMAT_STATUS_ONLY_GET(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	INT_T progress = IPC_APP_get_sd_format_status();
	respone_dp_value(TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, progress);
}
#endif

#ifdef TUYA_DP_PTZ_CONTROL
/* direction 0:UPPER, 1:UPPER-RIGHT, 2:RIGHT, 3:LOWER-RIGHT, 4:LOWER, 5:LOWER-LEFT, 6:LEFT, 7:UPPER-LEFT */
UINT_T dp_directions[8] = { 1, 2, 3, 4, 5, 6, 7, 0 };
char * dp_directions_str[8] = { "1", "2", "3", "4", "5", "6", "7", "0" };
extern int g_ptz_direction;

STATIC VOID handle_DP_PTZ_CONTROL(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}

	g_ptz_direction = dp_directions[p_obj_dp->value.dp_enum];
	respone_dp_enum(TUYA_DP_PTZ_CONTROL, dp_directions_str[p_obj_dp->value.dp_enum]);
}
#endif

#ifdef TUYA_DP_PTZ_STOP
#define PTZ_DIR_NONE -1
STATIC VOID handle_DP_PTZ_STOP(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	g_ptz_direction = PTZ_DIR_NONE;
	respone_dp_bool(TUYA_DP_PTZ_STOP, TRUE);
}
#endif

#ifdef TUYA_DP_PTZ_ZOOM_CONTROL
UINT_T dp_zoom[2] = { 0, 1 };
char * dp_zoom_str[2] = { "0", "1" };
extern int g_zoom;

STATIC VOID handle_DP_PTZ_ZOOM_CONTROL(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}

	PR_INFO("handle_DP_PTZ_ZOOM_CONTROL %d\n", dp_zoom[p_obj_dp->value.dp_enum]);

	g_zoom = dp_zoom[p_obj_dp->value.dp_enum];
	respone_dp_enum(TUYA_DP_PTZ_ZOOM_CONTROL, dp_zoom_str[p_obj_dp->value.dp_enum]);
}
/*
STATIC VOID handle_DP_PTZ_ZOOM_CONTROL(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}

	//dp 119 format: {"range":["1","2","3","4","5","6","7","0"],"type":"enum"}
	UINT_T dp_zoom[2] = { 0, 1 };
	UINT_T zoom = dp_zoom[p_obj_dp->value.dp_enum];
	CHAR_T tmp_str[2] = { 0 };
	snprintf(tmp_str, 2, "%d", zoom);
	IPC_APP_ptz_start_zoom(p_obj_dp->value.dp_enum);
	respone_dp_enum(TUYA_DP_PTZ_ZOOM_CONTROL, tmp_str);
}
*/
#endif

#ifdef TUYA_DP_PTZ_ZOOM_STOP
#define ZOOM_NONE -1
STATIC VOID handle_DP_PTZ_ZOOM_STOP(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	PR_INFO("handle_DP_PTZ_ZOOM_STOP\n");

	g_zoom = ZOOM_NONE;
	respone_dp_bool(TUYA_DP_PTZ_ZOOM_STOP, TRUE);
}
/*
STATIC VOID handle_DP_PTZ_ZOOM_STOP(IN TY_OBJ_DP_S *p_obj_dp)
{
	IPC_APP_ptz_stop_zoom();
	respone_dp_bool(TUYA_DP_PTZ_ZOOM_STOP, TRUE);
}
*/
#endif

#ifdef TUYA_DP_PTZ_CHECK
STATIC VOID handle_DP_PTZ_CHECK(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	IPC_APP_ptz_check();
	respone_dp_bool(TUYA_DP_PTZ_CHECK, TRUE);
}
#endif

#ifdef TUYA_DP_LINK_MOVE_ACTION
STATIC VOID handle_DP_LINK_MOVE(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		printf("Error!! type invalid %d \r\n", p_obj_dp->type);
		return;
	}

	CHAR_T tmp_str[2] = { 0 };
	int bind_move = 0;
	bind_move = p_obj_dp->value.dp_enum;
	tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

	IPC_APP_set_link_move(bind_move);
	respone_dp_enum(TUYA_DP_LINK_MOVE_ACTION, tmp_str);
}
#endif

#ifdef TUYA_DP_LINK_MOVE_SET
STATIC VOID handle_DP_LINK_MOVE_SET(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		printf("Error!! type invalid %d \r\n", p_obj_dp->type);
		return;
	}

	CHAR_T tmp_str[2] = { 0 };
	int bind_move = 0;
	bind_move = p_obj_dp->value.dp_enum;
	tmp_str[0] = '0' + p_obj_dp->value.dp_enum;

	IPC_APP_set_link_pos(bind_move);
	respone_dp_enum(TUYA_DP_LINK_MOVE_SET, tmp_str);
}
#endif

#ifdef TUYA_DP_HUM_FILTER
STATIC VOID handle_DP_HUM_FILTER(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T hum_filter = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_human_filter(hum_filter);

	respone_dp_bool(TUYA_DP_HUM_FILTER, hum_filter);
}
#endif

#ifdef TUYA_DP_PATROL_MODE
STATIC VOID handle_DP_patrol_mode(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}
	IPC_APP_set_patrol_mode(p_obj_dp->value.dp_enum);
	CHAR_T sMode[2];
	snprintf(sMode, 2, "%d", p_obj_dp->value.dp_enum);

	respone_dp_enum(TUYA_DP_PATROL_MODE, sMode);
}

#endif

#ifdef TUYA_DP_PATROL_SWITCH
STATIC VOID handle_DP_patrol_switch(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T patrol_mode = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_patrol_switch(patrol_mode);
	patrol_mode = IPC_APP_get_patrol_switch();

	respone_dp_bool(TUYA_DP_PATROL_SWITCH, patrol_mode);
}
#endif

#ifdef TUYA_DP_PATROL_TMODE
STATIC VOID handle_DP_patrol_tmode(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_ENUM)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}
	IPC_APP_set_patrol_tmode(p_obj_dp->value.dp_enum);
	CHAR_T sMode[2];
	snprintf(sMode, 2, "%d", p_obj_dp->value.dp_enum);
	respone_dp_enum(TUYA_DP_PATROL_TMODE, sMode);
}

#endif

#ifdef TUYA_DP_PATROL_TIME
STATIC VOID handle_DP_patrol_time(IN TY_OBJ_DP_S *p_dp_json)
{
	PR_INFO("---%s---\n", p_dp_json->value.dp_str);
	IPC_APP_set_patrol_time((cJSON *)(p_dp_json->value.dp_str));
	return;
}
#endif
#ifdef TUYA_DP_PATROL_STATE
STATIC VOID handle_DP_patrol_state(IN TY_OBJ_DP_S *p_dp_json __attribute__((unused)))
{
	int patrol_state = 0;
	//PR_INFO("---get_patrol_state\n");
	IPC_APP_patrol_state(&patrol_state);
	PR_INFO("---get_patrol_state:%d\n", patrol_state);

	CHAR_T sd_mode[4];
	snprintf(sd_mode, 4, "%d", patrol_state);
	respone_dp_enum(TUYA_DP_PATROL_STATE, sd_mode);
	return;
}
#endif

#ifdef TUYA_DP_PRESET_SET
STATIC VOID handle_DP_SET_PRESET(IN TY_OBJ_DP_S *p_dp_json)
{
	IPC_APP_set_preset((cJSON *)(p_dp_json->value.dp_str));
	return;
}
#endif

#ifdef TUYA_DP_DOOR_BELL
STATIC VOID handle_DP_DOOR_BELL(IN TY_OBJ_DP_S *p_obj_dp)
{
	PR_ERR("error! door bell can only trigged by IPC side.\n");
	respone_dp_str(TUYA_DP_DOOR_BELL, "-1");
}
#endif

#ifdef TUYA_DP_BLUB_SWITCH
STATIC VOID handle_DP_BLUB_SWITCH(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T blub_on_off = check_dp_bool_invalid(p_obj_dp);

	IPC_APP_set_blub_onoff(blub_on_off);
	blub_on_off = IPC_APP_get_blub_onoff();

	respone_dp_bool(TUYA_DP_BLUB_SWITCH, blub_on_off);
}
#endif

#ifdef TUYA_DP_ELECTRICITY
STATIC VOID handle_DP_ELECTRICITY(IN TY_OBJ_DP_S *p_obj_dp)
{
	INT_T percent = IPC_APP_get_battery_percent();
	PR_INFO("current battery percent:%d\n", percent);
	respone_dp_value(TUYA_DP_ELECTRICITY, percent);
}
#endif

#ifdef TUYA_DP_POWERMODE
STATIC VOID handle_DP_POWERMODE(IN TY_OBJ_DP_S *p_obj_dp)
{
	CHAR_T *power_mode = IPC_APP_get_power_mode();
	PR_INFO("current power mode:%s\n", power_mode);
	respone_dp_enum(TUYA_DP_POWERMODE, power_mode);
}
#endif

#ifdef TUYA_DP_LOWELECTRIC
STATIC VOID handle_DP_LOWELECTRIC(IN TY_OBJ_DP_S *p_obj_dp)
{
	if ((p_obj_dp == NULL) || (p_obj_dp->type != PROP_VALUE)) {
		PR_ERR("Error!! type invalid %d \n", p_obj_dp->type);
		return;
	}
	respone_dp_value(TUYA_DP_LOWELECTRIC, p_obj_dp->value.dp_value);
}
#endif

#ifdef TUYA_DP_SIREN_SWITCH
STATIC VOID handle_DP_SIREN_SWITCH(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T siren_switch = check_dp_bool_invalid(p_obj_dp);

	TUYA_APP_Force_Siren_On(siren_switch);
	siren_switch = IPC_APP_get_siren_switch();

	respone_dp_bool(TUYA_DP_SIREN_SWITCH, siren_switch);
}
#endif

#ifdef TUYA_DP_SIREN_VOLUME
STATIC VOID handle_DP_SIREN_VOLUME(IN TY_OBJ_DP_S *p_obj_dp)
{
	INT_T volume = check_dp_value_invalid(p_obj_dp);

	IPC_APP_set_siren_volume(volume);
	volume = IPC_APP_get_siren_volume();
	
	TUYA_APP_Update_Siren_Parameter();
	TUYA_APP_Update_Voice_Parameter();
	respone_dp_value(TUYA_DP_SIREN_VOLUME, volume);
}
#endif

#ifdef TUYA_DP_SIREN_INTERVAL
STATIC VOID handle_DP_SIREN_INTERVAL(IN TY_OBJ_DP_S *p_obj_dp)
{
	INT_T interval_mode = check_dp_value_invalid(p_obj_dp);

	IPC_APP_set_siren_interval(interval_mode);
	interval_mode = IPC_APP_get_siren_interval();

	TUYA_APP_Update_Siren_Parameter();
	TUYA_APP_Update_Voice_Parameter();
	respone_dp_value(TUYA_DP_SIREN_INTERVAL, interval_mode);
}
#endif

#ifdef TUYA_DP_DEVICE_RESTART
STATIC VOID handle_DP_DEVICE_RESTART(IN TY_OBJ_DP_S *p_obj_dp)
{
	BOOL_T restart = check_dp_bool_invalid(p_obj_dp);

	if (restart == TRUE) {
		IPC_APP_restart_device();
	}

	respone_dp_bool(TUYA_DP_DEVICE_RESTART, restart);
}
#endif

#if defined(WIFI_GW) && (WIFI_GW == 1)
#ifdef TUYA_DP_AP_MODE
STATIC VOID handle_DP_AP_MODE(IN TY_OBJ_DP_S *p_dp_json)
{
	if (p_dp_json == NULL) {
		printf("Error!! type invalid\r\n");
		return;
	}
	respone_dp_str(TUYA_DP_AP_MODE, IPC_APP_get_ap_mode());
}
#endif
#ifdef TUYA_DP_AP_SWITCH
STATIC VOID handle_DP_AP_SWITCH(IN TY_OBJ_DP_S *p_dp_json)
{
	CHAR_T resp[32] = { 0 };
	INT_T ap_enable = IPC_APP_set_ap_mode((cJSON *)p_dp_json->value.dp_str);
	if (ap_enable < 0) {
		snprintf(resp, 32, "{\\\"ap_enable\\\":0,\\\"errcode\\\":0}");
	} else {
		snprintf(resp, 32, "{\\\"ap_enable\\\":%d,\\\"errcode\\\":0}", ap_enable);
	}
	respone_dp_str(TUYA_DP_AP_SWITCH, resp);
	if (ap_enable >= 0) {
		change_ap_process();
	}
}
#endif
#endif

STATIC VOID handle_DP_RESERVED(IN TY_OBJ_DP_S *p_obj_dp __attribute__((unused)))
{
	PR_ERR("error! not implememt yet.\n");
}

typedef VOID (*TUYA_DP_HANDLER)(IN TY_OBJ_DP_S *p_obj_dp);
typedef struct {
	BYTE_T dp_id;
	TUYA_DP_HANDLER handler;
} TUYA_DP_INFO_S;

STATIC TUYA_DP_INFO_S s_dp_table[] = {
#ifdef TUYA_DP_SLEEP_MODE
	{ TUYA_DP_SLEEP_MODE, handle_DP_SLEEP_MODE },
#endif
#ifdef TUYA_DP_LIGHT
	{ TUYA_DP_LIGHT, handle_DP_LIGHT },
#endif
#ifdef TUYA_DP_FLIP
	{ TUYA_DP_FLIP, handle_DP_FLIP },
#endif
#ifdef TUYA_DP_WATERMARK
	{ TUYA_DP_WATERMARK, handleDataPointBasicOsd },
#endif
#ifdef TUYA_DP_IPC_AUTO_SIREN
	{ TUYA_DP_IPC_AUTO_SIREN, handleDataPointIpcAutoSiren },
#endif
#ifdef TUYA_DP_BRIGHTNESS
	{ TUYA_DP_BRIGHTNESS, handle_DP_brightness },
#endif
#ifdef TUYA_DP_CONTRAST
	{ TUYA_DP_CONTRAST, handle_DP_contrast },
#endif
#ifdef TUYA_DP_EXPOSURE
	{ TUYA_DP_EXPOSURE, handle_DP_exposure },
#endif
#ifdef TUYA_DP_WDR
	{ TUYA_DP_WDR, handle_DP_WDR },
#endif
#ifdef TUYA_DP_NIGHT_MODE
	{ TUYA_DP_NIGHT_MODE, handle_DP_NIGHT_MODE },
#endif
#ifdef TUYA_DP_ALARM_FUNCTION
	{ TUYA_DP_ALARM_FUNCTION, handle_DP_ALARM_FUNCTION },
#endif
#ifdef TUYA_DP_ALARM_SENSITIVITY
	{ TUYA_DP_ALARM_SENSITIVITY, handle_DP_ALARM_SENSITIVITY },
#endif
//#ifdef TUYA_DP_ALARM_INTERVAL
//    {TUYA_DP_ALARM_INTERVAL,        handle_DP_ALARM_INTERVAL},
//#endif
#ifdef TUYA_DP_ALARM_ZONE_ENABLE
	{ TUYA_DP_ALARM_ZONE_ENABLE, handle_DP_ALARM_ZONE_ENABLE },
#endif

#ifdef TUYA_DP_ALARM_ZONE_DRAW
	{ TUYA_DP_ALARM_ZONE_DRAW, handle_DP_ALARM_ZONE_DRAW },
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
	{ TUYA_DP_SD_STATUS_ONLY_GET, handle_DP_SD_STATUS_ONLY_GET },
#endif
#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
	{ TUYA_DP_SD_STORAGE_ONLY_GET, handle_DP_SD_STORAGE_ONLY_GET },
#endif
#ifdef TUYA_DP_SD_RECORD_ENABLE
	{ TUYA_DP_SD_RECORD_ENABLE, handle_DP_SD_RECORD_ENABLE },
#endif
#ifdef TUYA_DP_SD_RECORD_MODE
	{ TUYA_DP_SD_RECORD_MODE, handle_DP_SD_RECORD_MODE },
#endif
#ifdef TUYA_DP_SD_UMOUNT
	{ TUYA_DP_SD_UMOUNT, handle_DP_SD_UMOUNT },
#endif
#ifdef TUYA_DP_SD_FORMAT
	{ TUYA_DP_SD_FORMAT, handle_DP_SD_FORMAT },
#endif
#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
	{ TUYA_DP_SD_FORMAT_STATUS_ONLY_GET, handle_DP_SD_FORMAT_STATUS_ONLY_GET },
#endif
#ifdef TUYA_DP_PTZ_CONTROL
	{ TUYA_DP_PTZ_CONTROL, handle_DP_PTZ_CONTROL },
#endif
#ifdef TUYA_DP_PTZ_STOP
	{ TUYA_DP_PTZ_STOP, handle_DP_PTZ_STOP },
#endif
#ifdef TUYA_DP_PTZ_ZOOM_CONTROL
	{ TUYA_DP_PTZ_ZOOM_CONTROL, handle_DP_PTZ_ZOOM_CONTROL },
#endif
#ifdef TUYA_DP_PTZ_ZOOM_STOP
	{ TUYA_DP_PTZ_ZOOM_STOP, handle_DP_PTZ_ZOOM_STOP },
#endif
#ifdef TUYA_DP_PTZ_CHECK
	{ TUYA_DP_PTZ_CHECK, handle_DP_PTZ_CHECK },
#endif
#ifdef TUYA_DP_MOTION_TRACKING
	{ TUYA_DP_MOTION_TRACKING, handleDataPointMotionTracking },
#endif
#ifdef TUYA_DP_HUM_FILTER
	{ TUYA_DP_HUM_FILTER, handle_DP_HUM_FILTER },
#endif
#ifdef TUYA_DP_PATROL_MODE
	{ TUYA_DP_PATROL_MODE, handle_DP_patrol_mode },
#endif
#ifdef TUYA_DP_PATROL_TMODE
	{ TUYA_DP_PATROL_TMODE, handle_DP_patrol_tmode },
#endif
#ifdef TUYA_DP_PATROL_TIME
	{ TUYA_DP_PATROL_TIME, handle_DP_patrol_time },
#endif

#ifdef TUYA_DP_PATROL_STATE
	{ TUYA_DP_PATROL_STATE, handle_DP_patrol_state },
#endif

#ifdef TUYA_DP_PRESET_SET
	{ TUYA_DP_PRESET_SET, handle_DP_SET_PRESET },
#endif

#ifdef TUYA_DP_LINK_MOVE_ACTION
	{ TUYA_DP_LINK_MOVE_ACTION, handle_DP_LINK_MOVE },
#endif
#ifdef TUYA_DP_LINK_MOVE_SET
	{ TUYA_DP_LINK_MOVE_SET, handle_DP_LINK_MOVE_SET },
#endif

#ifdef TUYA_DP_DOOR_BELL
	{ TUYA_DP_DOOR_BELL, handle_DP_DOOR_BELL },
#endif
#ifdef TUYA_DP_BLUB_SWITCH
	{ TUYA_DP_BLUB_SWITCH, handle_DP_BLUB_SWITCH },
#endif
#ifdef TUYA_DP_SOUND_DETECT
	{ TUYA_DP_SOUND_DETECT, handle_DP_RESERVED },
#endif
#ifdef TUYA_DP_SOUND_SENSITIVITY
	{ TUYA_DP_SOUND_SENSITIVITY, handle_DP_RESERVED },
#endif
#ifdef TUYA_DP_SOUND_ALARM
	{ TUYA_DP_SOUND_ALARM, handle_DP_RESERVED },
#endif
#ifdef TUYA_DP_TEMPERATURE
	{ TUYA_DP_TEMPERATURE, handle_DP_RESERVED },
#endif
#ifdef TUYA_DP_HUMIDITY
	{ TUYA_DP_HUMIDITY, handle_DP_RESERVED },
#endif
#ifdef TUYA_DP_ELECTRICITY
	{ TUYA_DP_ELECTRICITY, handle_DP_ELECTRICITY },
#endif
#ifdef TUYA_DP_POWERMODE
	{ TUYA_DP_POWERMODE, handle_DP_POWERMODE },
#endif
#ifdef TUYA_DP_LOWELECTRIC
	{ TUYA_DP_LOWELECTRIC, handle_DP_LOWELECTRIC },
#endif
#ifdef TUYA_DP_SIREN_SWITCH
	{ TUYA_DP_SIREN_SWITCH, handle_DP_SIREN_SWITCH },
#endif
#ifdef TUYA_DP_SIREN_VOLUME
	{ TUYA_DP_SIREN_VOLUME, handle_DP_SIREN_VOLUME },
#endif
#ifdef TUYA_DP_SIREN_INTERVAL
	{ TUYA_DP_SIREN_INTERVAL, handle_DP_SIREN_INTERVAL },
#endif
#ifdef TUYA_DP_DEVICE_RESTART
	{ TUYA_DP_DEVICE_RESTART, handle_DP_DEVICE_RESTART },
#endif
#ifdef TUYA_DP_VIDEO_LAYOUT
	{ TUYA_DP_VIDEO_LAYOUT, handle_DP_VIDEO_LAYOUT },
#endif

#ifdef TUYA_DP_IPC_OBJECT_OUTLINE
	{ TUYA_DP_IPC_OBJECT_OUTLINE, handleDataPointIpcObjectOutline },
#endif
#if defined(WIFI_GW) && (WIFI_GW == 1)
#ifdef TUYA_DP_AP_MODE
	{ TUYA_DP_AP_MODE, handle_DP_AP_MODE },
#endif
#ifdef TUYA_DP_AP_SWITCH
	{ TUYA_DP_AP_SWITCH, handle_DP_AP_SWITCH },
#endif
#endif
};

VOID IPC_APP_handle_dp_cmd_objs(IN CONST TY_RECV_OBJ_DP_S *dp_rev)
{
	TY_OBJ_DP_S *dp_data = (TY_OBJ_DP_S *)(dp_rev->dps);
	UINT_T cnt = dp_rev->dps_cnt;
	INT_T table_idx = 0;
	INT_T table_count = (sizeof(s_dp_table) / sizeof(s_dp_table[0]));
	INT_T index = 0;
	for (index = 0; (UINT_T)index < cnt; index++) {
		TY_OBJ_DP_S *p_dp_obj = dp_data + index;

		for (table_idx = 0; table_idx < table_count; table_idx++) {
			if (s_dp_table[table_idx].dp_id == p_dp_obj->dpid) {
				s_dp_table[table_idx].handler(p_dp_obj);
				break;
			}
		}
	}
}

VOID IPC_APP_handle_dp_query_objs(IN CONST TY_DP_QUERY_S *dp_query)
{
	INT_T table_idx = 0;
	INT_T table_count = (sizeof(s_dp_table) / sizeof(s_dp_table[0]));
	INT_T index = 0;
	for (index = 0; (UINT_T)index < dp_query->cnt; index++) {
		for (table_idx = 0; table_idx < table_count; table_idx++) {
			if (s_dp_table[table_idx].dp_id == dp_query->dpid[index]) {
				s_dp_table[table_idx].handler(NULL);
				break;
			}
		}
	}
}

VOID IPC_APP_Report_siren_switch(BOOL on)
{
	respone_dp_bool(TUYA_DP_SIREN_SWITCH, on);
}

/*  The following interface has been abandoned,please refer to "tuya_ipc_notify_motion_detection" and "tuya_ipc_notify_door_bell_press" in tuya_ipc_api.h

OPERATE_RET IPC_APP_Send_Motion_Alarm_From_Buffer(CHAR_T *data, UINT_T size, NOTIFICATION_CONTENT_TYPE_E type)
{
    OPERATE_RET ret = OPRT_OK;
    INT_T try = 3;
    INT_T count = 1;
    VOID *message = NULL;
    INT_T message_size = 0;
#ifdef TUYA_DP_ALARM_FUNCTION
    if(IPC_APP_get_alarm_function_onoff() != TRUE)
    {
        PR_INFO("motion alarm upload not enabled.skip \n");
        return OPRT_COM_ERROR;
    }
#endif

    PR_INFO("Send Motion Alarm. size:%d type:%d\n", size, type);
    message_size = tuya_ipc_notification_message_malloc(count, &message);
    if((message_size == 0)||(message == NULL))
    {
        PR_INFO("tuya_ipc_notification_message_malloc failed\n");
        return OPRT_COM_ERROR;
    }

    memset(message, 0, message_size);
    while (try != 0)
    {
        ret = tuya_ipc_notification_content_upload_from_buffer(type,data,size,message);
        if(ret != OPRT_OK)
        {
            try --;
            continue;
        }
        break;
    }
    if(ret == OPRT_OK)
    {
        ret = tuya_ipc_notification_message_upload(TUYA_DP_MOTION_DETECTION_ALARM, message, 5);
    }

    tuya_ipc_notification_message_free(message);

    return ret;
}

OPERATE_RET IPC_APP_Send_Motion_Alarm(CHAR_T *p_abs_file, NOTIFICATION_CONTENT_TYPE_E file_type)
{
#ifdef TUYA_DP_ALARM_FUNCTION
    if(IPC_APP_get_alarm_function_onoff() != TRUE)
    {
        PR_INFO("motion alarm upload not enabled.skip \n");
        return OPRT_COM_ERROR;
    }
#endif

    OPERATE_RET ret = OPRT_OK;
    INT_T try = 3;
    INT_T count = 1;
    VOID *message = NULL;
    INT_T size = 0;

    PR_INFO("Send Motion Alarm. type:%d File:%s\n", file_type, p_abs_file);

    size = tuya_ipc_notification_message_malloc(count, &message);
    if((size == 0)||(message == NULL))
    {
        PR_INFO("tuya_ipc_notification_message_malloc failed\n");
        return OPRT_COM_ERROR;
    }

    memset(message, 0, size);
    while (try != 0)
    {
        ret = tuya_ipc_notification_content_upload_from_file(p_abs_file, file_type, message);
        if(ret != OPRT_OK)
        {
            try --;
            continue;
        }
        break;
    }
    if(ret == OPRT_OK)
    {
        ret = tuya_ipc_notification_message_upload(TUYA_DP_MOTION_DETECTION_ALARM, message, 5);
    }

    tuya_ipc_notification_message_free(message);

    return ret;
}

OPERATE_RET IPC_APP_Send_DoorBell_Snap(CHAR_T *p_snap_file, NOTIFICATION_CONTENT_TYPE_E file_type)
{
    OPERATE_RET ret = OPRT_OK;
    INT_T try = 3;
    INT_T count = 1;
    VOID *message = NULL;
    INT_T size = 0;

    PR_INFO("Send DoorBell Snap. type:%d File:%s\n", file_type, p_snap_file);
    size = tuya_ipc_notification_message_malloc(count, &message);
    if((size == 0)||(message == NULL))
    {
        PR_INFO("tuya_ipc_notification_message_malloc failed\n");
        return OPRT_COM_ERROR;
    }

    memset(message, 0, size);
    while (try != 0)
    {
        ret = tuya_ipc_notification_content_upload_from_file(p_snap_file, file_type, message);
        if(ret != OPRT_OK)
        {
            try --;
            continue;
        }
        break;
    }
    if(ret == OPRT_OK)
    {
        ret = tuya_ipc_snapshot_message_upload(TUYA_DP_DOOR_BELL_SNAP, message, 5);
    }

    tuya_ipc_notification_message_free(message);

    return ret;
}
*/

static int aux_set_strm_config(int sockfd, int len)
{
	INT32 ret = 0;

	TUYA_AG_CONF_S *conf;
 	AG_Get_Conf(&conf);
	AGTX_STRM_CONF_S *strm = &conf->strm.data;

	if (len < 0) {
		PR_ERR("AGTX_STRM_CONF_S request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_STRM_CONF_S)) {
		PR_ERR("AGTX_CMD_VIDEO_STRM_CONF size doesn't match %d / %d\n", len, sizeof(AGTX_STRM_CONF_S));
		return -1;
	}

	ret = read(sockfd, strm, sizeof(AGTX_STRM_CONF_S));
	if (ret != sizeof(AGTX_STRM_CONF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
#ifdef TUYA_DP_FLIP
	respone_dp_bool(TUYA_DP_FLIP, IPC_APP_get_flip_onoff());
#endif
	return ret;
}

int aux_set_img_config(int sockfd, int len)
{
	INT32 ret = 0;
	TUYA_AG_CONF_S *conf;
	AG_Get_Conf(&conf);
	AGTX_IMG_PREF_S *img = &conf->img.data;

	if (len < 0) {
		PR_ERR("AGTX_IMG_PREF_S request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_IMG_PREF_S)) {
		PR_ERR("AGTX_CMD_IMG_PREF size doesn't match %d / %d\n", len, sizeof(AGTX_IMG_PREF_S));
		return -1;
	}

	ret = read(sockfd, img, sizeof(AGTX_IMG_PREF_S));
	if (ret != sizeof(AGTX_IMG_PREF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
#ifdef TUYA_DP_BRIGHTNESS
	respone_dp_value(TUYA_DP_BRIGHTNESS, IPC_APP_get_brightness());
#endif
#ifdef TUYA_DP_CONTRAST
	respone_dp_value(TUYA_DP_CONTRAST, IPC_APP_get_contrast());
#endif
#ifdef TUYA_DP_EXPOSURE
	respone_dp_enum(TUYA_DP_EXPOSURE, IPC_APP_get_exposure());
#endif
	return 0;
}

int aux_set_adv_img_pref_config(int sockfd, int len)
{
	INT32 ret = 0;
	TUYA_AG_CONF_S *conf;
	AG_Get_Conf(&conf);
	AGTX_ADV_IMG_PREF_S *adv_img = &conf->pref.data;

	if (len < 0) {
		PR_ERR("AGTX_ADV_IMG_PREF_S request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_ADV_IMG_PREF_S)) {
		PR_ERR("AGTX_CMD_ADV_IMG_PREF size doesn't match %d / %d\n", len, sizeof(AGTX_ADV_IMG_PREF_S));
		return -1;
	}

	ret = read(sockfd, adv_img, sizeof(AGTX_ADV_IMG_PREF_S));
	if (ret != sizeof(AGTX_ADV_IMG_PREF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
#ifdef TUYA_DP_WDR
	respone_dp_bool(TUYA_DP_WDR, IPC_APP_get_wdr_onoff());
#endif
#ifdef TUYA_DP_NIGHT_MODE
	respone_dp_enum(TUYA_DP_NIGHT_MODE, IPC_APP_get_night_mode());
#endif
	return 0;
}

int aux_set_md_config(int sockfd, int len)
{
	INT32 ret = 0;
	TUYA_AG_CONF_S *conf;
	AG_Get_Conf(&conf);
	AGTX_IVA_MD_CONF_S *md = &conf->md.data;

	if (len < 0) {
		PR_ERR("AGTX_CMD_IVA_MD request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_IVA_MD_CONF_S)) {
		PR_ERR("AGTX_CMD_IVA_MD size doesn't match %d / %d\n", len, sizeof(AGTX_IVA_MD_CONF_S));
		return -1;
	}

	ret = read(sockfd, md, sizeof(AGTX_IVA_MD_CONF_S));
	if (ret != sizeof(AGTX_IVA_MD_CONF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
#ifdef TUYA_DP_ALARM_FUNCTION
	respone_dp_bool(TUYA_DP_ALARM_FUNCTION, IPC_APP_get_alarm_function_onoff());
#endif
#ifdef TUYA_DP_ALARM_SENSITIVITY
	respone_dp_enum(TUYA_DP_ALARM_SENSITIVITY, IPC_APP_get_alarm_sensitivity());
#endif
	return ret;
}

int aux_set_video_ptz_config(int sockfd, int len)
{
	INT32 ret = 0;
	TUYA_AG_CONF_S *conf;
	AG_Get_Conf(&conf);
	AGTX_VIDEO_PTZ_CONF_S *ptz = &conf->ptz.data;

	if (len < 0) {
		PR_ERR("AGTX_CMD_PTZ request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_VIDEO_PTZ_CONF_S)) {
		PR_ERR("AGTX_VIDEO_PTZ_CONF_S size doesn't match %d / %d\n", len, sizeof(AGTX_VIDEO_PTZ_CONF_S));
		return -1;
	}

	ret = read(sockfd, ptz, sizeof(AGTX_VIDEO_PTZ_CONF_S));
	if (ret != sizeof(AGTX_VIDEO_PTZ_CONF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
#ifdef TUYA_DP_PTZ_MODE
	/* FIXME: Current TUYA_DP_PTZ_MODE is changed to TUYA_DP_MOTION_TRACKING */
	//respone_dp_enum(TUYA_DP_PTZ_MODE, IPC_APP_get_ptz_mode());
#endif
	return ret;
}

int aux_set_osd_config(INT32 sockfd, INT32 len)
{
	INT32 ret = 0;
	TUYA_AG_CONF_S *conf;
	AG_Get_Conf(&conf);
	AGTX_OSD_CONF_S *osd = &conf->osd.data;

	if (len < 0) {
		PR_ERR("AGTX_OSD_CONF_S request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_OSD_CONF_S)) {
		PR_ERR("AGTX_OSD_CONF_S size doesn't match %d / %d\n", len, sizeof(AGTX_OSD_CONF_S));
		return -1;
	}

	ret = read(sockfd, osd, sizeof(AGTX_OSD_CONF_S));
	if (ret != sizeof(AGTX_OSD_CONF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
#ifdef TUYA_DP_WATERMARK
	respone_dp_bool(TUYA_DP_WATERMARK, IPC_APP_get_watermark_onoff());
#endif
	return 0;
}

int aux_set_iva_od_config(INT32 sockfd, INT32 len)
{
	INT32 ret = 0;
	TUYA_AG_CONF_S *conf;
	AG_Get_Conf(&conf);
	AGTX_IVA_OD_CONF_S *od = &conf->od.data;

	if (len < 0) {
		PR_ERR("AGTX_IVA_OD_CONF_S request failed err %d\n", len);
		return -1;
	}

	if (len != sizeof(AGTX_IVA_OD_CONF_S)) {
		PR_ERR("AGTX_IVA_OD_CONF_S size doesn't match %d / %d\n", len, sizeof(AGTX_IVA_OD_CONF_S));
		return -1;
	}

	ret = read(sockfd, od, sizeof(AGTX_IVA_OD_CONF_S));
	if (ret != sizeof(AGTX_IVA_OD_CONF_S)) {
		PR_ERR("Read too short %d(%m)\n", errno);
		return -1;
	}
	return 0;
}

static int aux_cc_cmd(int sockfd, AGTX_MSG_HEADER_S *cmd)
{
	int ret = 0;

	switch (cmd->cid) {
	case AGTX_CMD_VIDEO_STRM_CONF:
		ret = aux_set_strm_config(sockfd, cmd->len);
		break;
	case AGTX_CMD_IMG_PREF:
		ret = aux_set_img_config(sockfd, cmd->len);
		break;
	case AGTX_CMD_ADV_IMG_PREF:
		ret = aux_set_adv_img_pref_config(sockfd, cmd->len);
		break;
	case AGTX_CMD_MD_CONF:
		ret = aux_set_md_config(sockfd, cmd->len);
		break;
	case AGTX_CMD_VIDEO_PTZ_CONF:
		ret = aux_set_video_ptz_config(sockfd, cmd->len);
		break;
	case AGTX_CMD_OSD_CONF:
		ret = aux_set_osd_config(sockfd, cmd->len);
		break;
	case AGTX_CMD_OD_CONF:
		ret = aux_set_iva_od_config(sockfd, cmd->len);
		break;
	default:
		PR_ERR("Unknown command\n");
		ret = -1;
		break;
	}

	return ret;
}

static int aux_register_to_cc(int sockfd)
{
	int ret;
	char buf[128] = { 0 };
	char reg_buf[128] = { 0 };
	char ret_cmd[128] = { 0 };

	sprintf(reg_buf, "{ \"master_id\":0, \"cmd_id\":%d, \"cmd_type\":\"ctrl\", \"name\":\"APP\"}",
	        AGTX_CMD_REG_CLIENT);
	sprintf(ret_cmd, "{ \"master_id\": 0, \"cmd_id\": %d, \"cmd_type\": \"reply\", \"rval\": 0 }",
	        AGTX_CMD_REG_CLIENT);

	/*Send register information*/
	if (write(sockfd, &reg_buf, strlen(reg_buf)) < 0) {
		PR_ERR("write socket error %d(%m)\n", errno);
		return -1;
	}

	while (1) {
		ret = read(sockfd, buf, strlen(ret_cmd));
		if ((unsigned long)ret != strlen(ret_cmd)) {
			PR_ERR("read socket error %d(%m)\n", errno);
			PR_WARN("Failed to read from CC socket! Retry...\n");
			continue;
		}

		if (strncmp(buf, ret_cmd, strlen(ret_cmd))) {
			usleep(100000);
			PR_INFO("Waiting CC replay register cmd\n");
			PR_INFO("Waiting CC to replay register cmd! Retry...\n");
			continue;
		} else {
			PR_NOTICE("Registered to CC from APP\n");
			break;
		}
	}

	return 0;
}

static int aux_cc_send_reply(int sockfd, AGTX_MSG_HEADER_S *cmd, int ret)
{
	if (write(sockfd, cmd, sizeof(*cmd)) < 0) {
		PR_ERR("write socket error %d(%m)\n", errno);
		return -1;
	}

	if (write(sockfd, &ret, sizeof(INT32)) < 0) {
		PR_ERR("write socket error %d(%m)\n", errno);
		return -1;
	}

	return 0;
}

VOID *AG_Connect_CC(VOID *data __attribute__((unused)))
{
	INT32 sockfd = -1;
	INT32 servlen = 0;
	INT32 ret = 0;
	INT32 err_cnt = 0;
	fd_set read_fds;
	struct timeval tv = { 0 };
	struct sockaddr_un serv_addr;
	AGTX_MSG_HEADER_S cmd_header = { 0 };
	AGTX_MSG_HEADER_S cmd_reply = { 0 };

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, auxCmdTransSocketFile);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		PR_ERR("Create sockfd failed %d(%m)\n", errno);
		return NULL;
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		PR_ERR("Connecting to server failed %d(%m)\n", errno);
		close(sockfd);
		return NULL;
	}

	/*Register module to CC*/
	aux_register_to_cc(sockfd);

	while (1) {
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);
		ret = select(sockfd + 1, &read_fds, NULL, NULL, &tv);
		if (ret < 0) {
			PR_ERR("select error\n");
			continue;
		} else if (ret == 0) {
			//			PR_ERR( "select timeout\n" );
			continue;
		} else {
			if (err_cnt > 5) {
				PR_ERR("Too many error close socket and leave.\n");
				break;
			}

			ret = read(sockfd, &cmd_header, sizeof(cmd_header));
			if (ret < 0) {
				PR_ERR("Read failed %d(%m),leave thread.\n", errno);
				break;
			} else if (ret != sizeof(cmd_header)) {
				PR_ERR("Read too short %d(%m).\n", errno);
				err_cnt++;
				continue;
			}

			err_cnt = 0;

			ret = aux_cc_cmd(sockfd, &cmd_header);

			cmd_reply.cid = cmd_header.cid;
			cmd_reply.sid = 0;
			cmd_reply.len = sizeof(INT32);

			aux_cc_send_reply(sockfd, &cmd_reply, ret);
		}
	}

	close(sockfd);

	return NULL;
}

BOOL AG_Init_CClient(VOID)
{
	int servlen;
	struct sockaddr_un serv_addr;
	int idx = 1, errNo;
	struct timeval tv = {.tv_sec = 10, .tv_usec = 0 };
	char buffer[256];

	char name[] = { "app_cc" };
	pthread_t tid = 0;

	g_master_id = 0;

	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, auxCmdTransSocketFile);
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
	if ((cmdTransFD = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		PR_ERR("Failed to creating socket with CmdTranslator.\n");
	}

	//Set sockopts for time out
	setsockopt(cmdTransFD, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(struct timeval));

	errNo = connect(cmdTransFD, (struct sockaddr *)&serv_addr, servlen);
	if (errNo < 0) {
		PR_ERR("Error: Connecting to ccserver...\n");
		close(cmdTransFD);
		goto fail_conn;
	} else { //send Greetings Message to server in jSON
		int retry_cnt = 3; /* try three times to initialize */
		PR_NOTICE("Connected to cc skt successfully.\n");

		while (idx) {
			// step 1
			if (retry_cnt <= 0) {
				goto fail_conn;
			}

			char *cmd_start1 = "{'master_id':0, 'cmd_id':1048577, 'cmd_type':'ctrl', 'name':'APP_HOST'}";
			bzero(buffer, 256);

			if (write(cmdTransFD, cmd_start1, strlen(cmd_start1)) < 0) {
				PR_ERR("Failed to send message to command Translator: %s\n", cmd_start1);
			} else {
				PR_INFO("Sent %s to CC.\n", cmd_start1);
				AG_getCCReply(buffer);
				PR_INFO("Reply %s from CC.\n", buffer);
				errNo = aux_json_validation(buffer, strlen(buffer));

				if (errNo == 0) {
					//is return is 0(success)
					if (aux_json_get_int(buffer, "rval", strlen(buffer)) < 0) {
						--retry_cnt;
						continue;
					}
				}
			}

			sleep(1);
			// step 2
			char *cmd_start2 = "{'master_id':0, 'cmd_id':1048578, 'cmd_type':'ctrl'}";
			bzero(buffer, 256);
			if (write(cmdTransFD, cmd_start2, strlen(cmd_start2)) < 0) {
				PR_ERR("Failed to send message to command Translator: %s \n", cmd_start2);
			} else {
				PR_INFO("Sent %s to CC.\n", cmd_start2);
				AG_getCCReply(buffer);
				PR_INFO("Reply %s from CC.\n", buffer);
				errNo = aux_json_validation(buffer, strlen(buffer));

				if (errNo == 0) {
					//check if rval = 0
					if (aux_json_get_int(buffer, "rval", strlen(buffer)) < 0) {
						--retry_cnt;
						continue;
					} else {
						if (strstr(buffer, "master_id")) {
							g_master_id =
							        aux_json_get_int(buffer, "master_id", strlen(buffer));
							idx = 0;
						}
					}
				}
			}
		}
	}

	if (AG_Collect_Conf() < 0) {
		PR_ERR("Collect config failed\n");
		goto fail_conn;
	}

	if (pthread_create(&tid, NULL, AG_Connect_CC, NULL) != 0) {
		PR_ERR("Create thread to tuya APP CC failed.\n");
		goto fail_conn;
	}

	if (pthread_setname_np(tid, name) != 0) {
		PR_ERR("Set thread to tuya APP CC failed.\n");
		goto fail_conn;
	}

	return TRUE;
fail_conn:
	return FALSE;
}

VOID AG_Exit_CClient(VOID)
{
	close(cmdTransFD);
}
