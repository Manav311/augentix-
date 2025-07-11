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
 * @file tuya_ipc_dp_utils.h
 * @brief
 */

#ifndef TUYA_IPC_DP_UTILS_H_
#define TUYA_IPC_DP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_types.h"
#include "tuya_ipc_api.h"
#include "tuya_cloud_com_defs.h"

#include "cJSON.h"

//#define TUYA_DP_SLEEP_MODE 105
//#define TUYA_DP_LIGHT 101
#define TUYA_DP_FLIP 103
#define TUYA_DP_WATERMARK 104
#define TUYA_DP_WDR 107
#define TUYA_DP_NIGHT_MODE 108
#define TUYA_DP_ALARM_FUNCTION 134
#define TUYA_DP_ALARM_SENSITIVITY 106
#define TUYA_DP_SD_STATUS_ONLY_GET 110
#define TUYA_DP_SD_STORAGE_ONLY_GET 109
#define TUYA_DP_SD_RECORD_ENABLE 150
#define TUYA_DP_SD_RECORD_MODE 151
#define TUYA_DP_SD_UMOUNT 112
#define TUYA_DP_SD_FORMAT 111
#define TUYA_DP_SD_FORMAT_STATUS_ONLY_GET 117
#define TUYA_DP_PTZ_CONTROL 119
#define TUYA_DP_PTZ_STOP 116
#define TUYA_DP_PTZ_ZOOM_CONTROL 163
#define TUYA_DP_PTZ_ZOOM_STOP 164
#define TUYA_DP_PTZ_CHECK 132
#define TUYA_DP_IPC_AUTO_SIREN 120
#define TUYA_DP_MOTION_TRACKING 161
#define TUYA_DP_HUM_FILTER 170
//#define TUYA_DP_PATROL_SWITCH 174
#define TUYA_DP_PATROL_MODE 175
#define TUYA_DP_PATROL_TMODE 176
#define TUYA_DP_PATROL_TIME 177
#define TUYA_DP_PRESET_SET 178
#define TUYA_DP_PATROL_STATE 179
//#define TUYA_DP_ALARM_ZONE_ENABLE 168
//#define TUYA_DP_ALARM_ZONE_DRAW 169
//#define TUYA_DP_BLUB_SWITCH 138
#define TUYA_DP_SOUND_DETECT 139
#define TUYA_DP_SOUND_SENSITIVITY 140
#define TUYA_DP_SOUND_ALARM 141
#define TUYA_DP_TEMPERATURE 142
#define TUYA_DP_HUMIDITY 143
//#define TUYA_DP_ELECTRICITY 145
//#define TUYA_DP_POWERMODE 146
#define TUYA_DP_LOWELECTRIC 147
#define TUYA_DP_DOOR_STATUS 149
#define TUYA_DP_MOTION_DETECTION_ALARM 115
#define TUYA_DP_DOOR_BELL_SNAP 154
#define TUYA_DP_DEVICE_RESTART 162
#define TUYA_DP_BRIGHTNESS 193
#define TUYA_DP_CONTRAST 192
#define TUYA_DP_IPC_OBJECT_OUTLINE 198
#define TUYA_DP_EXPOSURE 188
#define TUYA_DP_SIREN_SWITCH 159
#define TUYA_DP_SIREN_VOLUME 195
#define TUYA_DP_SIREN_INTERVAL 194
#define TUYA_DP_VIDEO_LAYOUT 196
#define TUYA_DP_MOTION_DIR 231
#define TUYA_DP_MOTION_REG_LEFT 232
#define TUYA_DP_MOTION_REG_RIGHT 233

VOID respone_dp_value(BYTE_T dp_id, INT_T val);
VOID respone_dp_bool(BYTE_T dp_id, BOOL_T true_false);
VOID respone_dp_enum(BYTE_T dp_id, CHAR_T *p_val_enum);
VOID respone_dp_str(BYTE_T dp_id, CHAR_T *p_val_str);

/* Report the latest status of all local DP points*/
VOID IPC_APP_upload_all_status(VOID);

#ifdef TUYA_DP_DOOR_BELL
/* In the doorbell product form, when the doorbell is pressed, the notification is pushed to the APP */
VOID IPC_APP_trigger_door_bell(VOID);
#endif

#ifdef TUYA_DP_SD_FORMAT_STATUS_ONLY_GET
/* SD card formatting progress report*/
VOID IPC_APP_report_sd_format_status(INT_T status);
#endif

#ifdef TUYA_DP_SD_STATUS_ONLY_GET
/*When the SD card changes (plug and unplug), call this API to notify to APP*/
VOID IPC_APP_report_sd_status_changed(INT_T status);
#endif

#ifdef TUYA_DP_SD_STORAGE_ONLY_GET
/* After formatting, call this API to report the storage capacity. */
VOID IPC_APP_report_sd_storage();
#endif

#ifdef TUYA_DP_POWERMODE
/* Call this API to notify the app when the battery status changes. */
VOID IPC_APP_update_battery_status(VOID);
#endif

#ifdef TUYA_DP_MOTION_DETECTION_ALARM
/* When a motion detection event occurs, the API is called to push an alarm picture to the APP. */
OPERATE_RET IPC_APP_Send_Motion_Alarm_From_Buffer(CHAR_T *data, UINT_T size, NOTIFICATION_CONTENT_TYPE_E type);
OPERATE_RET IPC_APP_Send_Motion_Alarm(CHAR_T *p_abs_file, NOTIFICATION_CONTENT_TYPE_E file_type);
#endif

#ifdef TUYA_DP_DOOR_BELL_SNAP
/* When the doorbell is pressed, it will captures a picture and calls the API to push the picture to the APP. */
OPERATE_RET IPC_APP_Send_DoorBell_Snap(CHAR_T *p_snap_file, NOTIFICATION_CONTENT_TYPE_E file_type);
#endif

#ifdef TUYA_DP_MOTION_DIR
/* Report Motion direction define */
VOID IPC_APP_report_motion_direction(INT_T motion_dir);
#endif

#ifdef TUYA_DP_MOTION_REG_LEFT
/* Report Motion region left define */
VOID IPC_APP_report_motion_reg_left(INT_T motion_reg_left);
#endif

#ifdef TUYA_DP_MOTION_REG_RIGHT
/* Report Motion region right define */
VOID IPC_APP_report_motion_reg_right(INT_T motion_reg_right);
#endif

/* basic API */
VOID IPC_APP_handle_dp_cmd_objs(IN CONST TY_RECV_OBJ_DP_S *dp_rev);
/* API */
VOID IPC_APP_handle_dp_query_objs(IN CONST TY_DP_QUERY_S *dp_query);

BOOL AG_Init_CClient(VOID);
VOID AG_Exit_CClient(VOID);

VOID IPC_APP_Report_siren_switch(BOOL on);

void reportDataPointBool(BYTE_T id, BOOL_T value);

int parseDataPointBool(TY_OBJ_DP_S *dp, BOOL_T *value);

#ifdef __cplusplus
}
#endif

#endif /* TUYA_IPC_DP_UTILS_H_ */
