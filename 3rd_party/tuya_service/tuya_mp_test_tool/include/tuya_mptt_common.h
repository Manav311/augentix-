#ifndef TUYA_MPTT_COMMON_H
#define TUYA_MPTT_COMMON_H
/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tuya_mptt_common.h - Common resource
 * Copyright (C) 2019-2020 ShihChieh Lin, Augentix Inc. <shihchieh.lin@augentix.com>
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 */

#define DEBUG

#ifdef DEBUG
#define DBG(format, args...) printf("[DBG] %s, line %d: " format, __FILE__, __LINE__, ##args)
#else
#define DBG(format, args...)
#endif /* DEBUG */

#define FW_NAME "agtx_fw"
#define VERSION_FILE "/etc/sw-version"

#define CONFIG_FILE "/mnt/sdcard/tuya/fac/cfg/config.ini"
#define WPA_SUPP_FILE "/tmp/wpa_supplicant.conf"

#define TUYA_MQTT_SOCKET_PORT 8090
#define TUYA_MQTT_NETWORK_IF "wlan0"

char g_client_addr[32];

#endif /* TUYA_MPTT_COMMON_H */
