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
 * @file tuya_ipc_system_control.h
 * @brief
 */

/*********************************************************************************
  *Copyright(C),2015-2020, 
  *TUYA 
  *www.tuya.comm
**********************************************************************************/

#ifndef TUYA_IPC_SYSTEM_CONTROL_H_
#define TUYA_IPC_SYSTEM_CONTROL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tuya_cloud_com_defs.h"
#include <sys/ioctl.h>
#include <sys/time.h>

#define RD_VALUE_SECS _IOR('a', 'S', long int *)
#define RD_VALUE_USEC _IOR('a', 'U', long int *)

typedef enum {
	IPC_BOOTUP_FINISH,
	IPC_START_WIFI_CFG,
	IPC_REV_WIFI_CFG,
	IPC_CONNECTING_WIFI,
	IPC_MQTT_ONLINE,
	IPC_RESET_SUCCESS,
	IPC_STREAM_READY,
	IPC_WEAK_WIFI_TRUE,
	IPC_WEAK_WIFI_FALSE,
	IPC_LIVE_VIDEO_START,
	IPC_LIVE_VIDEO_STOP
} IPC_APP_NOTIFY_EVENT_E;

/* Update local time */
int gettimeofdayMonotonic(struct timeval *t1, int *tz);

OPERATE_RET IPC_APP_Sync_Utc_Time(VOID);

VOID IPC_APP_Show_OSD_Time(VOID);

VOID IPC_APP_Reset_System_CB(GW_RESET_TYPE_E reboot_type);

VOID IPC_APP_Upgrade_Inform_cb(IN CONST FW_UG_S *fw);

VOID IPC_APP_Restart_Process_CB(VOID);

VOID IPC_APP_Upgrade_Firmware_CB(VOID);

VOID IPC_APP_Notify_LED_Sound_Status_CB(IPC_APP_NOTIFY_EVENT_E notify_event);

VOID get_sw_version(char **);

VOID IPC_APP_Notify_Siren(int alarm);

BOOL_T TUYA_APP_Get_Siren_Status();

VOID TUYA_APP_Force_Siren_On(BOOL_T on);

VOID TUYA_APP_Update_Siren_Parameter();

VOID TUYA_APP_Update_Voice_Parameter();

VOID TUYA_APP_Siren_init();

VOID TUYA_APP_Siren_deinit();

#ifdef __cplusplus
}
#endif

#endif /* TUYA_IPC_SYSTEM_CONTROL_H_ */
