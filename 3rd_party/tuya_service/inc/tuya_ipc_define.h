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
 * @file tuya_ipc_define.h
 * @brief
 */

#ifndef TUYA_IPC_DEFINE_H_
#define TUYA_IPC_DEFINE_H_

#define TUYA_DISABLE_AUDIO 0

/* Path to save tuya sdk DB files, should be readable, writeable and storable */
/* Must end with a slash */
#define IPC_APP_STORAGE_PATH "/usrdata/active_setting/tuya/"

/* File with path to download file during OTA */
#define IPC_APP_UPGRADE_FILE "/tmp/update.swu"

/* SD card mount directory */
#define IPC_APP_SD_BASE_PATH "/mnt/sdcard"

#define CMD_MODE "/system/bin/mode"

/* File that overrides PID/UUID/AUTHKEY */
#define TUYA_ID_FILE "ApplyId"
#define TUYA_ID_SEARCH_PATH_1 IPC_APP_SD_BASE_PATH
#define TUYA_ID_SEARCH_PATH_2 "/root"
#define TUYA_ID_KEY_COUNT 3
#define TUYA_ID_PID_KEY "IPC_APP_PID"
#define TUYA_ID_PID_SIZE 16
#define TUYA_ID_UUID_KEY "IPC_APP_UUID"
#define TUYA_ID_UUID_SIZE 20
#define TUYA_ID_AUTHKEY_KEY "IPC_APP_AUTHKEY"
#define TUYA_ID_AUTHKEY_SIZE 32

/* WiFi related settings */
#define IWCONFIG_CMD "/usr/bin/iwconfig"
#define WLAN_DEV "wlan0"
#define WPA_SUPPLICANT_CONF_FILE "/tmp/wpa_supplicant.conf"
#define WEAK_WIFI_ASSERT_TH 30
#define WEAK_WIFI_DEASSERT_TH 40

/* Augentix pairing modes: for m=0 (auto mode)
 *  qr/sniff (default) AGTX_TUYA_PAIR_QR_MODE=AGTX_TUYA_PAIR_SNIFF_MODE=1
 *  qr code,           AGTX_TUYA_PAIR_QR_MODE=1,AGTX_TUYA_PAIR_SNIFF_MODE=0
 *  sniff mode         AGTX_TUYA_PAIR_QR_MODE=0,AGTX_TUYA_PAIR_SNIFF_MODE=1
 */
extern unsigned int AGTX_TUYA_PAIR_QR_MODE;
extern unsigned int AGTX_TUYA_PAIR_SNIFF_MODE;
/* Echo show and chromecast */
extern unsigned int enable_echoShow;
extern unsigned int enable_chromecast;

/*  CUSTOMER SPECIFIC WIFI SSID */
#define CUSTOMER_SSID_PREFIX "SmartLife_"
extern char WLAN_DEV_MAC[6];

//#define AVMAIN_RDY_FILE "/tmp/avmain_ready"
#define AVMAIN_RDY_FILE "/tmp/av_main2_ready"

/* Ethernet related settings */
#define NET_DEV "eth0"

/* SD card related settings */
#define LINUX_SD_DEV_FILE "/dev/mmcblk0"
#define LINUX_MOUNT_INFO_FILE "/proc/mounts"
#define FORMAT_CMD "mkfs.vfat"

/* FIXME read from system */
/* Camera firmware version to be displayed on app */
#define IPC_APP_VERSION "1.0.0"

/* Common String Length */
#define IPC_APP_TOKEN_SIZE 16

#ifndef MAX_PATH
#define MAX_PATH 256
#endif

#endif /* TUYA_IPC_DEFINE_H_ */
