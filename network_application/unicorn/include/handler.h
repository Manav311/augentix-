#ifndef UNICORN_HANDLER_H_
#define UNICORN_HANDLER_H_

#include "frame.h"

int unicorn_write_cfg(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_read_cfg(UnicornFrame *frame, char *buf);
int unicorn_access_mode(UnicornFrame *frame, char *buf, const char *type);
int unicorn_factory_reset(UnicornFrame *frame, char *buf);
int unicorn_wifi_mac_addr(UnicornFrame *frame, char *buf, const char *type);
int unicorn_eth_mac_addr(UnicornFrame *frame, char *buf, const char *type);
int unicorn_usb_mac_addr(UnicornFrame *frame, char *buf, const char *type);
int unicorn_read_fw_version(UnicornFrame *frame, char *buf);
int unicorn_mb_number(UnicornFrame *frame, char *buf, const char *type);
int unicorn_serial_number(UnicornFrame *frame, char *buf, const char *type);
int unicorn_product_id(UnicornFrame *frame, char *buf, const char *type);
int unicorn_sd_test(UnicornFrame *frame, char *buf);
int unicorn_ir_cut_ctrl(UnicornFrame *frame, char *buf);
int unicorn_ir_led_ctrl(UnicornFrame *frame, char *buf);
int unicorn_led_ctrl(UnicornFrame *frame, char *buf);
int unicorn_rssi(UnicornFrame *frame, char *buf);
int unicorn_light_sensor(UnicornFrame *frame, char *buf);
int unicorn_audio_speaker(UnicornFrame *frame, char *buf);
int unicorn_audio_mic(UnicornFrame *frame, char *buf);
int unicorn_button_ctrl(UnicornFrame *frame, char *buf);
int unicorn_lvds_test(UnicornFrame *frame, char *buf);
int unicorn_sensor_test(UnicornFrame *frame, char *buf);
int unicorn_send_file_cmd(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_send_file_ack(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_send_file(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_recv_file(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_recv_file_cmd(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_recv_file_data(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_oc_adjust_apply(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_oc_adjust_preview(UnicornFrame *frame, char *buf);
int unicorn_ac_freq_apply(UnicornFrame *frame, char *buf, int sockfd);
int unicorn_ac_freq_preview(UnicornFrame *frame, char *buf);
int unicorn_day_night_mode(UnicornFrame *frame, char *buf);
int unicorn_ota(UnicornFrame *frame, char *buf);
int unicorn_bad_pixel(UnicornFrame *frame, char *buf);
int unicorn_floodlight_ctrl(UnicornFrame *frame, char *buf);
int unicorn_pir_ctrl(UnicornFrame *frame, char *buf);

#endif /* HANDLER_H */
