#ifndef TUYA_TEST_HANDLER_H
#define TUYA_TEST_HANDLER_H

/*
 * AUGENTIX INC. - PROPRIETARY
 *
 * tuya_test_handler.h - Tuya test item handler
 * Copyright (C) 2019 ShihChieh Lin, Augentix Inc. <shihchieh.lin@augentix.com>
 *
 * NOTICE: The information contained herein is the property of Augentix Inc.
 * Copying and distributing of this file, via any medium,
 * must be licensed by Augentix Inc.
 *
 */

#include "tuya_test_frame.h"
#include "tuya_mptt_common.h"

enum tuya_day_night_mode {
	TUYA_DAY_MODE = 0,
	TUYA_NIGHT_MODE = 1,
};

int tuya_enter_test_mode(TuyaTestFrame *frame, char *buf);
int tuya_read_master_firmware(TuyaTestFrame *frame, char *buf);
int tuya_write_cfg(TuyaTestFrame *frame, char *buf);
int tuya_read_cfg(TuyaTestFrame *frame, char *buf);
int tuya_write_mac(TuyaTestFrame *frame, char *buf);
int tuya_read_mac(TuyaTestFrame *frame, char *buf);
void *tuya_button_test(void *arg);
int tuya_write_bsn(TuyaTestFrame *frame, char *buf);
int tuya_read_bsn(TuyaTestFrame *frame, char *buf);
int tuya_write_sn(TuyaTestFrame *frame, char *buf);
int tuya_read_sn(TuyaTestFrame *frame, char *buf);
int tuya_write_cn(TuyaTestFrame *frame, char *buf);
int tuya_read_cn(TuyaTestFrame *frame, char *buf);
int tuya_speaker_test(TuyaTestFrame *frame, char *buf);
int tuya_mic_test(TuyaTestFrame *frame, char *buf);
int tuya_pir_test(TuyaTestFrame *frame, char *buf);
void *tuya_led_test(void *arg);
void *tuya_irled_test(void *arg);
void *tuya_ircut_test(void *arg);
void *tuya_rtsp_test(void *arg);
int tuya_wifi_rssi_test(TuyaTestFrame *frame, char *buf);
int tuya_wifi_iperf_test(TuyaTestFrame *frame, char *buf);

#endif /* TUYA_TEST_HANDLER_H */
