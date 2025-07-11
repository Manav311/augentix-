#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_lpw.h"

#define PERI_VERIFY_GPIO_SET_DIR(hd, pin, dir, output, expect)                                           \
	do {                                                                                             \
		int result = lpw_gpio_set_dir(hd, pin, dir, output);                                     \
		printf("[liblpw] run lpw_gpio_set_dir(%d, %d, %d), ret = %d", pin, dir, output, result); \
		if (result != expect) {                                                                  \
			printf("... Fail!\n");                                                           \
		} else {                                                                                 \
			printf("... OK!\n");                                                             \
		}                                                                                        \
	} while (0)
#define PERI_VERIFY_GPIO_GET_DIR(hd, pin, expect)                                   \
	do {                                                                        \
		int result = lpw_gpio_get_dir(hd, pin);                             \
		printf("[liblpw] run lpw_gpio_get_dir(%d), ret = %d", pin, result); \
		if (result != expect) {                                             \
			printf("... Fail!\n");                                      \
		} else {                                                            \
			printf("... OK!\n");                                        \
		}                                                                   \
	} while (0)
#define PERI_VERIFY_GPIO_SET_OUTPUT(hd, pin, output, expect)                                       \
	do {                                                                                       \
		int result = lpw_gpio_set_output(hd, pin, output);                                 \
		printf("[liblpw] run lpw_gpio_set_output(%d, %d), ret = %d", pin, output, result); \
		if (result != expect) {                                                            \
			printf("... Fail!\n");                                                     \
		} else {                                                                           \
			printf("... OK!\n");                                                       \
		}                                                                                  \
	} while (0)
#define PERI_VERIFY_GPIO_GET_INPUT(hd, pin, expect)                                   \
	do {                                                                          \
		int result = lpw_gpio_get_input(hd, pin);                             \
		printf("[liblpw] run lpw_gpio_get_input(%d), ret = %d", pin, result); \
		if (result != expect) {                                               \
			printf("... Fail!\n");                                        \
		} else {                                                              \
			printf("... OK!\n");                                          \
		}                                                                     \
	} while (0)
#define PERI_VERIFY_ADC_GET(hd, adc_ch, expect)                                   \
	do {                                                                      \
		int result = lpw_adc_get(hd, adc_ch);                             \
		printf("[liblpw] run lpw_adc_get(%d), ret = %d", adc_ch, result); \
		if (result < expect) {                                            \
			printf("... Fail!\n");                                    \
		} else {                                                          \
			printf("... OK!\n");                                      \
		}                                                                 \
	} while (0)
#define WAKE_VERIFY_PM_INIT(hd, pin, level, adc_ch, off_thre, on_thre, expect)                                 \
	do {                                                                                                   \
		int result = lpw_pm_init(hd, pin, level, adc_ch, off_thre, on_thre);                           \
		printf("[liblpw] run lpw_pm_init(%d, %d, %d, %d, %d), ret = %d", pin, level, adc_ch, off_thre, \
		       on_thre, result);                                                                       \
		if (result != expect) {                                                                        \
			printf("... Fail!\n");                                                                 \
		} else {                                                                                       \
			printf("... OK!\n");                                                                   \
		}                                                                                              \
	} while (0)
#define WAKE_VERIFY_TCP_EN(hd, pattern, expect)                                             \
	do {                                                                                \
		int result = lpw_sleep_tcp_wake_en(hd, pattern);                            \
		printf("[liblpw] run lpw_sleep_tcp_wake_en(tcp_wake_t), ret = %d", result); \
		if (result != expect) {                                                     \
			printf("... Fail!\n");                                              \
		} else {                                                                    \
			printf("... OK!\n");                                                \
		}                                                                           \
	} while (0)
#define WAKE_VERIFY_GPIO_EN(hd, pin, event, expect)                                                    \
	do {                                                                                           \
		int result = lpw_sleep_detect_gpio_en(hd, pin, event);                                 \
		printf("[liblpw] run lpw_sleep_detect_gpio_en(%d, %d), ret = %d", pin, event, result); \
		if (result != expect) {                                                                \
			printf("... Fail!\n");                                                         \
		} else {                                                                               \
			printf("... OK!\n");                                                           \
		}                                                                                      \
	} while (0)
#define WAKE_VERIFY_ADC_EN(hd, adc_ch, thre, ab_un, expect)                                                        \
	do {                                                                                                       \
		int result = lpw_sleep_detect_adc_en(hd, adc_ch, thre, ab_un);                                     \
		printf("[liblpw] run lpw_sleep_detect_adc_en(%d, %d, %d), ret = %d", adc_ch, thre, ab_un, result); \
		if (result != expect) {                                                                            \
			printf("... Fail!\n");                                                                     \
		} else {                                                                                           \
			printf("... OK!\n");                                                                       \
		}                                                                                                  \
	} while (0)
#define WAKE_VERIFY_SLEEP_START(hd, timeout, response, expect)                                       \
	do {                                                                                         \
		int result = lpw_sleep_start(hd, timeout, response);                                 \
		printf("[liblpw] run lpw_sleep_start(%d, %d), ret = %d", timeout, response, result); \
		if (result != expect) {                                                              \
			printf("... Fail!\n");                                                       \
		} else {                                                                             \
			printf("... OK!\n");                                                         \
		}                                                                                    \
	} while (0)
#define WAKE_VERIFY_WAKE_BY(hd, event, expect)                                                   \
	do {                                                                                     \
		int result = lpw_sleep_get_wake_event(hd, event);                                \
		printf("[liblpw] run lpw_sleep_get_wake_event(wake_event_t), ret = %d", result); \
		if (result != expect) {                                                          \
			printf("... Fail!\n");                                                   \
		} else {                                                                         \
			printf("... OK!\n");                                                     \
		}                                                                                \
	} while (0)

#define WAKE_VERIFY_WAKE_BY(hd, event, expect)                                                   \
	do {                                                                                     \
		int result = lpw_sleep_get_wake_event(hd, event);                                \
		printf("[liblpw] run lpw_sleep_get_wake_event(wake_event_t), ret = %d", result); \
		if (result != expect) {                                                          \
			printf("... Fail!\n");                                                   \
		} else {                                                                         \
			printf("... OK!\n");                                                     \
		}                                                                                \
	} while (0)

/*---------------------------------------------------------------------------------------------*/

#define WIFI_SET_STA_MODE_VERIFY(hd, expect)                                        \
	do {                                                                        \
		int result = lpw_wifi_set_sta_mode(hd);                             \
		printf("[liblpw] run lpw_wifi_set_sta_mode(hd), ret = %d", result); \
		if (result != expect) {                                             \
			printf("... Fail!\n");                                      \
		} else {                                                            \
			printf("... OK!\n");                                        \
		}                                                                   \
	} while (0)

#define WIFI_SET_AP_MODE_VERIFY(hd, config, expect)                                        \
	do {                                                                               \
		int result = lpw_wifi_set_ap_mode(hd, config);                             \
		printf("[liblpw] run lpw_wifi_set_ap_mode(hd, config), ret = %d", result); \
		if (result != expect) {                                                    \
			printf("... Fail!\n");                                             \
		} else {                                                                   \
			printf("... OK!\n");                                               \
		}                                                                          \
	} while (0)

#define WIFI_CONN_TO_VERIFY(hd, config, expect)                                           \
	do {                                                                              \
		int result = lpw_wifi_connect_to(hd, config);                             \
		printf("[liblpw] run lpw_wifi_connect_to(hd, config), ret = %d", result); \
		if (result != expect) {                                                   \
			printf("... Fail!\n");                                            \
		} else {                                                                  \
			printf("... OK!\n");                                              \
		}                                                                         \
	} while (0)

#define WIFI_GET_STATUS_VERIFY(hd, config, expect)                                        \
	do {                                                                              \
		int result = lpw_wifi_get_status(hd, config);                             \
		printf("[liblpw] run lpw_wifi_get_status(hd, config), ret = %d", result); \
		if (result != expect) {                                                   \
			printf("... Fail!\n");                                            \
		} else {                                                                  \
			printf("... OK!\n");                                              \
		}                                                                         \
	} while (0)

#define WIFI_SCAN_VERIFY(hd, config, res, expect)                                        \
	do {                                                                             \
		int result = lpw_wifi_scan(hd, config, res);                             \
		printf("[liblpw] run lpw_wifi_scan(hd, config, res), ret = %d", result); \
		if (result != expect) {                                                  \
			printf("... Fail!\n");                                           \
		} else {                                                                 \
			printf("... OK!\n");                                             \
		}                                                                        \
	} while (0)

/*---------------------------------------------------------------------------------------------*/

#define TCP_CONN_TO_VERIFY(hd, ip, port, expect)                                           \
	do {                                                                               \
		int result = lpw_tcp_connect_to(hd, ip, port);                             \
		printf("[liblpw] run lpw_tcp_connect_to(hd, ip, port), ret = %d", result); \
		if (result != expect) {                                                    \
			printf("... Fail!\n");                                             \
		} else {                                                                   \
			printf("... OK!\n");                                               \
		}                                                                          \
	} while (0)

#define TCP_DISCONNECT_VERIFY(hd, expect)                      \
	do {                                                   \
		lpw_tcp_disconnect(hd);                        \
		printf("[liblpw] run lpw_tcp_disconnect(hd)"); \
		printf("... OK!\n");                           \
	} while (0)

#define TCP_SEND_VERIFY(hd, data, len, expect)                                        \
	do {                                                                          \
		int result = lpw_tcp_send(hd, data, len);                             \
		printf("[liblpw] run lpw_tcp_send(hd, data, len), ret = %d", result); \
		if (result != expect) {                                               \
			printf("... Fail!\n");                                        \
		} else {                                                              \
			printf("... OK!\n");                                          \
		}                                                                     \
	} while (0)

#define TCP_RECV_VERIFY(hd, buf, len, expect)                                        \
	do {                                                                         \
		int result = lpw_tcp_recv(hd, buf, len);                             \
		printf("[liblpw] run lpw_tcp_recv(hd, buf, len), ret = %d", result); \
		if (result != expect) {                                              \
			printf("... Fail!\n");                                       \
		} else {                                                             \
			printf("... OK!\n");                                         \
		}                                                                    \
	} while (0)

/*---------------------------------------------------------------------------------------------*/

/* ssid > 32 */
lpw_wifi_conn_t conn0 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHA",
	.key = "00000000",
};

/* key < 8 */
lpw_wifi_conn_t conn1 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "0000",
};

/* key > 64 */
lpw_wifi_conn_t conn2 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "12346758123467581234675812346758123467581234675812346758123467581",
};

/* bssid and ssid are empty in same time */
lpw_wifi_conn_t conn3 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = NULL,
	.key = "00000000",
};

/* invalid auth */
lpw_wifi_conn_t conn4 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "00000000",
};

/* invalid auth */
lpw_wifi_conn_t conn5 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPA,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "00000000",
};

/* invalid auth */
lpw_wifi_conn_t conn6 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPA2,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "00000000",
};

/* invalid auth */
lpw_wifi_conn_t conn7 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_UNKNOWN,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "00000000",
};

/* invalid pairwise */
lpw_wifi_conn_t conn8 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = 4,
	.ssid = "A",
	.key = "00000000",
};

/* invalid WIFI_SECUIRTY_WEP case */
lpw_wifi_conn_t conn9 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WEP,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "1234",
};

/*---------------------------------------------------------------------------------------------*/

/* invalid scan type */
lpw_wifi_scan_t sc0 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "A",
	.channel = 0,
	.scan_type = 0,
};

/* invalid channel number */
lpw_wifi_scan_t sc1 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "A",
	.channel = 0,
	.scan_type = WIFI_CHANNEL_SCAN,
};

/* invalid ssid length */
lpw_wifi_scan_t sc2 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "",
	.channel = 0,
	.scan_type = WIFI_SSID_SCAN,
};

/* invalid ssid length */
lpw_wifi_scan_t sc3 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "",
	.channel = 0,
	.scan_type = WIFI_SSID_PREFIX_SCAN,
};

/* invalid bssid */
lpw_wifi_scan_t sc4 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "A",
	.channel = 0,
	.scan_type = WIFI_BSSID_SCAN,
};

/*---------------------------------------------------------------------------------------------*/

/* invalid channel */
lpw_wifi_ap_config_t ap_conf0 = {
	.channel_num = 0,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

/* invalid channel */
lpw_wifi_ap_config_t ap_conf1 = {
	.channel_num = 15,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

/* invalid auth */
lpw_wifi_ap_config_t ap_conf2 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WEP,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

/* invalid auth */
lpw_wifi_ap_config_t ap_conf3 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

/* invalid auth */
lpw_wifi_ap_config_t ap_conf4 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPA2,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

/* invalid auth */
lpw_wifi_ap_config_t ap_conf5 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_UNKNOWN,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

/* invalid ssid length */
lpw_wifi_ap_config_t ap_conf6 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "AAAABBBBCCCCDDDDEEEEFFFFGGGGHHHHA",
	.key = "12345678",
};

/* invalid ssid length */
lpw_wifi_ap_config_t ap_conf7 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "",
	.key = "12345678",
};

/* invalid key length */
lpw_wifi_ap_config_t ap_conf8 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "1234567",
};

/* invalid key length */
lpw_wifi_ap_config_t ap_conf9 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12346758123467581234675812346758123467581234675812346758123467581",
};

/* invalid pairwise */
lpw_wifi_ap_config_t ap_conf10 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = 4,
	.ssid = "Hi3861L-AP",
	.key = "00000000",
};

/*---------------------------------------------------------------------------------------------*/

lpw_wifi_scan_t sc5 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "A",
	.channel = 0,
	.scan_type = WIFI_SSID_SCAN,
};

lpw_wifi_scan_t sc6 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "ASU",
	.channel = 0,
	.scan_type = WIFI_SSID_PREFIX_SCAN,
};

lpw_wifi_scan_t sc7 = {
	.bssid = { 0x60, 0x45, 0xcb, 0x8e, 0xc3, 0xa0 },
	.ssid = "",
	.channel = 0,
	.scan_type = WIFI_BSSID_SCAN,
};

lpw_wifi_scan_t sc8 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "",
	.channel = 1,
	.scan_type = WIFI_CHANNEL_SCAN,
};

lpw_wifi_scan_t sc9 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "ABCDE",
	.channel = 0,
	.scan_type = WIFI_SSID_SCAN,
};

lpw_wifi_scan_t sc10 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "AAAAAAAAAAA",
	.channel = 0,
	.scan_type = WIFI_SSID_PREFIX_SCAN,
};

lpw_wifi_scan_t sc11 = {
	.bssid = { 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA },
	.ssid = "",
	.channel = 0,
	.scan_type = WIFI_BSSID_SCAN,
};

lpw_wifi_scan_t sc12 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.ssid = "",
	.channel = 1,
	.scan_type = WIFI_CHANNEL_SCAN,
};

/*---------------------------------------------------------------------------------------------*/

lpw_wifi_conn_t conn10 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "A",
	.key = "00000000",
};

lpw_wifi_conn_t conn11 = {
	.bssid = { 0, 0, 0, 0, 0, 0 },
	.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "B",
	.key = "00000000",
};

lpw_wifi_ap_config_t ap_conf11 = {
	.channel_num = 1,
	.authmode = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX,
	.ssid = "Hi3861L-AP",
	.key = "12345678",
};

lpw_wifi_state_t st;

lpw_wifi_scan_result_t res;

/*---------------------------------------------------------------------------------------------*/

in_addr_t ip1, ip2, ip3;
in_port_t port1 = 0;
in_port_t port2 = 80;
in_port_t port3 = 100;
char ip_array1[] = { 192, 168, 10, 166 };
char ip_array2[] = { 0, 0, 0, 0 };
char ip_array3[] = { 192, 168, 10, 88 };

uint8_t tcp_send[] = { 0x54, 0x43, 0x50, 0x20, 0x43, 0x6F, 0x6E, 0x6E, 0x65, 0x63,
	               0x74, 0x69, 0x6F, 0x6E, 0x20, 0x74, 0x65, 0x73, 0x74 };

uint8_t tcp_recv1[10];
uint8_t tcp_recv2[748];

uint8_t *tcp_null = NULL;

/*---------------------------------------------------------------------------------------------*/

int ret;
int sum = 0;

/*---------------------------------------------------------------------------------------------*/

int main(void)
{
	lpw_handle hd = lpw_open();
	if (hd == (lpw_handle)NULL) {
		printf("open lpw device fail\n");
	}
	lpw_fw_ver_t ver;
	lpw_module_get_version(hd, &ver);
	printf("LPW device module version: %d\n.%d\n.%d\n\n", ver.ver[0], ver.ver[1], ver.ver[2]);

	typedef struct {
		uint8_t src;
		uint8_t pm_adc_ch;
		uint16_t off_thre;
		uint16_t on_thre;
		uint32_t time;
		uint8_t response;
		uint8_t gpio[3];
		uint16_t adc[2];
		uint16_t adc_up_thre;
		uint16_t adc_down_thre;
		uint8_t config;
		gpio_event_t event;
		tcp_wake_t tcp;
	} data;

	//	uint8_t input_gpio = 10;
	//	uint8_t output_gpio = 5;
	unsigned char hb_pack[] = { '6' };
	unsigned char wake_pack[] = { '7' };

	// used for invalid test

	data invalid_data = {
		.src = 7,
		.pm_adc_ch = 3,
		.off_thre = 1000,
		.on_thre = 1000,
		.gpio = { 11 },
		//2 for output cond., 5 for invalid
		.adc = { 2, 5 },
		.tcp = {
			.keep_alive_pack_len = 0,
			.keep_alive_period = 0,
			.wake_pack_len = 0,
			.keep_alive_pack = hb_pack,
			.wake_pack = wake_pack,
			.bad_conn_handle = 1,
		},
	};

	// used for sleep_detect setting
	data sleep_data = {
		.src = 0,
		.pm_adc_ch = 3,
		.off_thre = 1000,
		.on_thre = 5000,
		.adc_up_thre = 5000,
		.adc_down_thre = 50,
		.config = 1,
		.event = (GPIO_FALLING),
		.tcp = {
			.keep_alive_pack_len = 1,
			.keep_alive_period = 10,
			.wake_pack_len = 1,
			.keep_alive_pack = hb_pack,
			.wake_pack = wake_pack,
			.bad_conn_handle = 0,
		},
		.time = 10,
		.response = 9,
		.gpio = {9, 10, 14},
		.adc = {2, 3},
	};
	enum { GPIO_INPUT, GPIO_OUTPUT };

	/*	
	PERI_VERIFY_GPIO_SET_DIR(hd, output_gpio, GPIO_INPUT, 0, 0);
	PERI_VERIFY_GPIO_GET_DIR(hd, output_gpio, 0);
	PERI_VERIFY_GPIO_SET_DIR(hd, output_gpio, GPIO_OUTPUT, 0, 0);
	PERI_VERIFY_GPIO_GET_DIR(hd, output_gpio, 1);
	PERI_VERIFY_GPIO_SET_DIR(hd, output_gpio, GPIO_OUTPUT, 1, 0);
	PERI_VERIFY_GPIO_GET_DIR(hd, output_gpio, 2);

	PERI_VERIFY_GPIO_SET_OUTPUT(hd, output_gpio, 0, 0);
	PERI_VERIFY_GPIO_GET_DIR(hd, output_gpio, 1);
	PERI_VERIFY_GPIO_SET_OUTPUT(hd, output_gpio, 1, 0);
	PERI_VERIFY_GPIO_GET_DIR(hd, output_gpio, 2);
	
	PERI_VERIFY_GPIO_GET_INPUT(hd, input_gpio, 1);

	PERI_VERIFY_ADC_GET(hd, sleep_data.adc[1], 0);

	//failed case 0 : peri
	PERI_VERIFY_GPIO_SET_DIR(hd, invalid_data.gpio[0], GPIO_INPUT, 0, -ENXIO);
	PERI_VERIFY_GPIO_GET_DIR(hd, invalid_data.gpio[0], -ENXIO);
	PERI_VERIFY_GPIO_SET_OUTPUT(hd, invalid_data.gpio[0], 0, -ENXIO);
	PERI_VERIFY_GPIO_GET_INPUT(hd, invalid_data.gpio[0], -ENXIO);
	PERI_VERIFY_ADC_GET(hd, invalid_data.adc[1], -ENXIO);
	PERI_VERIFY_GPIO_SET_OUTPUT(hd, input_gpio, 0, -EIO);
	PERI_VERIFY_GPIO_GET_INPUT(hd, output_gpio, -EIO);
	PERI_VERIFY_ADC_GET(hd, invalid_data.adc[0], -EIO);

	//failed case 1 : en & sleep start before sleep_init
	WAKE_VERIFY_TCP_EN(hd, &sleep_data.tcp, -EACCES);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], sleep_data.event, -EACCES);
	WAKE_VERIFY_ADC_EN(hd, sleep_data.adc[0], sleep_data.adc_up_thre, 1, -EACCES);
	WAKE_VERIFY_SLEEP_START(hd, sleep_data.time, sleep_data.response, -EACCES);
*/
	//src = 7 to conflict with en
	WAKE_VERIFY_PM_INIT(hd, invalid_data.src, 0, invalid_data.pm_adc_ch, invalid_data.off_thre,
	                    invalid_data.on_thre, 0);
	WAKE_VERIFY_PM_INIT(hd, invalid_data.src, 0, invalid_data.pm_adc_ch, invalid_data.off_thre,
	                    invalid_data.on_thre, 0);
	WAKE_VERIFY_PM_INIT(hd, invalid_data.src, 1, invalid_data.pm_adc_ch, invalid_data.off_thre,
	                    invalid_data.on_thre, 0);

	//failed case 2 : sleep_start before en
	WAKE_VERIFY_SLEEP_START(hd, 0, 1, -EACCES);
	/*
	//sleep_start with only time (will be turn off)
	WAKE_VERIFY_SLEEP_START(hd, sleep_data.time, sleep_data.response, 0);

	//failed case 3 : src en confilct
	WAKE_VERIFY_ADC_EN(hd, sleep_data.adc[1], sleep_data.adc_up_thre, 1, -EACCES);
*/
	//src = 0, high
	/*
	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	WIFI_GET_STATUS_VERIFY(hd, &st, 0);
	*/
	/*
	memcpy(&ip1, ip_array1, 4);
	TCP_CONN_TO_VERIFY(hd, ip1, port2, 0);
	*/

	lpw_gpio_set_dir(hd, sleep_data.src, 1, 1);
	WAKE_VERIFY_PM_INIT(hd, sleep_data.src, 1, sleep_data.pm_adc_ch, sleep_data.off_thre, sleep_data.on_thre, 0);

	/*
	//failed case 4 : gpio_en
	WAKE_VERIFY_GPIO_EN(hd, invalid_data.gpio[0], sleep_data.event, -ENXIO);
	WAKE_VERIFY_GPIO_EN(hd, output_gpio, sleep_data.event, -EIO);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.src, sleep_data.event, -EACCES);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], 0xFF, -EINVAL);
	//failed case5 : adc_en
	WAKE_VERIFY_ADC_EN(hd, invalid_data.adc[0], sleep_data.adc_up_thre, 1, -EIO);
	WAKE_VERIFY_ADC_EN(hd, invalid_data.adc[1], sleep_data.adc_up_thre, 1, -ENXIO);
	WAKE_VERIFY_ADC_EN(hd, 4, sleep_data.adc_up_thre, 1, -EACCES);

	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], GPIO_LOW, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], GPIO_HIGH, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], GPIO_RISING, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], GPIO_FALLING, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], GPIO_LOW | GPIO_HIGH | GPIO_RISING | GPIO_FALLING, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[0], sleep_data.event, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[1], sleep_data.event, 0);
	WAKE_VERIFY_GPIO_EN(hd, sleep_data.gpio[2], sleep_data.event, 0);

	//set 5 input
	lpw_gpio_set_dir(hd, 5, 0, 1);
	//ADC
	WAKE_VERIFY_ADC_EN(hd, sleep_data.adc[0], sleep_data.adc_up_thre, 1, 0);
	WAKE_VERIFY_ADC_EN(hd, sleep_data.adc[0], sleep_data.adc_down_thre, 0, 0);
	WAKE_VERIFY_ADC_EN(hd, sleep_data.adc[1], sleep_data.adc_up_thre, 1, 0);
	WAKE_VERIFY_ADC_EN(hd, sleep_data.adc[1], sleep_data.adc_down_thre, 0, 0);

	// TCP_WAKE_EN
	WAKE_VERIFY_TCP_EN(hd, &invalid_data.tcp, -EINVAL);
	invalid_data.tcp.keep_alive_pack_len = 1;
	WAKE_VERIFY_TCP_EN(hd, &invalid_data.tcp, -EINVAL);
	invalid_data.tcp.keep_alive_period = 5;
	WAKE_VERIFY_TCP_EN(hd, &invalid_data.tcp, -EINVAL);
	invalid_data.tcp.wake_pack_len = 1;
	//now it's valid
	WAKE_VERIFY_TCP_EN(hd, &invalid_data.tcp, 0);
*/
	//sleep
	WAKE_VERIFY_SLEEP_START(hd, 0, sleep_data.response, 0);
	//wake_by is commented now, for sleep_start will be stuck due to recv
	/*	wake_event_t wake_event = { 0 };
	WAKE_VERIFY_WAKE_BY(hd, &wake_event, 0);
	printf("[liblpw] after sleep, event.gpios = %u\n", wake_event.gpios);
	printf("[liblpw] after sleep, event.adcs = %u\n", wake_event.adcs);
	printf("[liblpw] after sleep, event.tcp = %u\n", wake_event.tcp);
	printf("[liblpw] after sleep, event.timeout = %u\n", wake_event.timeout);
*/
	//	printf("==================================================================\n");
	//
	//	WIFI_CONN_TO_VERIFY(hd, &conn0, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn1, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn2, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn3, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn4, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn5, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn6, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn7, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn8, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn9, -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	WIFI_SCAN_VERIFY(hd, &sc0, &res, -EINVAL);
	//	WIFI_SCAN_VERIFY(hd, &sc1, &res, -EINVAL);
	//	WIFI_SCAN_VERIFY(hd, &sc2, &res, -EINVAL);
	//	WIFI_SCAN_VERIFY(hd, &sc3, &res, -EINVAL);
	//	WIFI_SCAN_VERIFY(hd, &sc4, &res, -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf0, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf1, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf2, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf3, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf4, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf5, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf6, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf7, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf8, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf9, -EINVAL);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf10, -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	/* without wifi set mode */
	//	WIFI_GET_STATUS_VERIFY(hd, &st, -EINVAL);
	//	WIFI_SCAN_VERIFY(hd, &sc5, &res, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	/* wifi set ap mode */
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf11, 0);
	//	WIFI_GET_STATUS_VERIFY(hd, &st, -EINVAL);
	//	WIFI_SCAN_VERIFY(hd, &sc5, &res, -EINVAL);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf11, 0);
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf11, 0);
	//
	//	printf("==================================================================\n");
	//
	//	/* wifi set sta mode */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_GET_STATUS_VERIFY(hd, &st, -EIO);
	//
	//	/* get wifi status after wifi is connected */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//	WIFI_GET_STATUS_VERIFY(hd, &st, 0);
	//
	//	/* disconect wifi from connected state then get wifi status */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//	/* Please power off AP when 10 seconds */
	//	sleep(10);
	//	WIFI_GET_STATUS_VERIFY(hd, &st, 0);
	//
	//	printf("==================================================================\n");
	//
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc5, &res, 0);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc6, &res, 0);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc7, &res, 0);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc8, &res, 0);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc9, &res, -ENODEV);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc10, &res, -ENODEV);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc11, &res, -ENODEV);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	WIFI_SCAN_VERIFY(hd, &sc12, &res, -ENODEV);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	sleep(2);
	//
	//	printf("==================================================================\n");
	//
	//	/* connect target exist */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//
	//	/* connect target doesn't exist */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn11, -EACCES);
	//
	//	/* when wifi is connected then reconnect again */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//
	//	/* when wifi is connected then reconnect again */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn11, -EACCES);
	//
	//	printf("==================================================================\n");
	//
	//	memcpy(&ip1, ip_array1, 4);
	//	memcpy(&ip2, ip_array2, 4);
	//
	//	/* invalid port */
	//	TCP_CONN_TO_VERIFY(hd, ip1, port1, -EINVAL);
	//	/* invalid ip */
	//	TCP_CONN_TO_VERIFY(hd, ip2, port2, -EINVAL);
	//
	//	/* send and recv without tcp connected */
	//	TCP_SEND_VERIFY(hd, tcp_send, sizeof(tcp_send), -EINVAL);
	//	TCP_RECV_VERIFY(hd, tcp_recv, sizeof(tcp_recv), -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	/* no set mode */
	//	memcpy(&ip1, ip_array1, 4);
	//	TCP_CONN_TO_VERIFY(hd, ip1, port2, -EINVAL);
	//
	//	/* connect tcp under ap mode */
	//	WIFI_SET_AP_MODE_VERIFY(hd, &ap_conf11, 0);
	//	TCP_CONN_TO_VERIFY(hd, ip1, port2, -EINVAL);
	//
	//	/* set sta mode */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	TCP_CONN_TO_VERIFY(hd, ip1, port2, -EINVAL);
	//
	//	/* target non-exist with wifi connected */
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//	memcpy(&ip3, ip_array3, 4);
	//	TCP_CONN_TO_VERIFY(hd, ip3, port3, -EACCES);
	//
	//	/* target exist with wifi connected */
	//	TCP_CONN_TO_VERIFY(hd, ip1, port2, 0);
	//
	//	/* disconnect when tcp is unconnected */
	//	TCP_DISCONNECT_VERIFY(hd, 0);
	//
	//	/* tcp_null == NULL */
	//	TCP_SEND_VERIFY(hd, tcp_null, sizeof(tcp_null), -EINVAL);
	//	TCP_RECV_VERIFY(hd, tcp_null, sizeof(tcp_null), -EINVAL);
	//
	//	printf("==================================================================\n");
	//
	//	/* following section is normal wifi connection procedure */
	//
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_SCAN_VERIFY(hd, &sc5, &res, 0);
	//	printf("bssid = %d %d %d %d %d %d\n", (int)res.bssid[0], (int)res.bssid[1],
	//					      (int)res.bssid[2], (int)res.bssid[3],
	//					      (int)res.bssid[4], (int)res.bssid[5]);
	//	printf("channel = %d\n", res.channel);
	//	printf("auth = %d\n", res.auth);
	//	printf("ssid = %s\n", res.ssid);
	//	WIFI_CONN_TO_VERIFY(hd, &conn10, 0);
	//	WIFI_GET_STATUS_VERIFY(hd, &st, 0);
	//	printf("conn = %d\n", (int)st.conn);
	//	printf("ssid = %s\n", st.ssid);
	//	printf("ip = %d %d %d %d\n", (int)st.ip[0], (int)st.ip[1], (int)st.ip[2], (int)st.ip[3]);
	//	printf("mask = %d %d %d %d\n", (int)st.mask[0], (int)st.mask[1], (int)st.mask[2], (int)st.mask[3]);
	//	printf("gw = %d %d %d %d\n", (int)st.gw[0], (int)st.gw[1], (int)st.gw[2], (int)st.gw[3]);
	//	printf("mac = %d %d %d %d %d %d\n", (int)st.mac[0], (int)st.mac[1], (int)st.mac[2],
	//					    (int)st.mac[3], (int)st.mac[4], (int)st.mac[5]);
	//
	//	printf("==================================================================\n");
	//
	//	/* Connect TCP */
	//	memcpy(&ip1, ip_array1, 4);
	//	TCP_CONN_TO_VERIFY(hd, ip1, port2, 0);
	//
	//	/* Send data */
	//	TCP_SEND_VERIFY(hd, tcp_send, sizeof(tcp_send), sizeof(tcp_send));
	//
	//	printf("==================================================================\n");
	//	printf("[APP] Please send data to Hi3861L from tcp server in 10 seconds\n");
	//	printf("==================================================================\n");
	//	sleep(10);
	//
	//	/* Receive data */
	//	for (int cnt = 0; cnt < 2; cnt++) {
	//		ret = lpw_tcp_recv(hd, tcp_recv1, sizeof(tcp_recv1));
	//		sum += ret;
	//		printf("[APP] This is %d times read\n", cnt + 1);
	//		printf("[APP] Try to read %d bytes from lpw_controller, ret = %d\n", sizeof(tcp_recv1), ret);
	//		//printf("[APP] receive message = ");
	//		//for (int cnt = 0; cnt < ret; cnt++) {
	//		//	printf("%c", tcp_recv1[cnt]);
	//		//}
	//		//printf("\n");
	//	}
	//	printf("==================================================================\n");
	//	printf("[APP] total read = %d\n", sum);
	//	printf("==================================================================\n");
	//
	//	printf("==================================================================\n");
	//
	//	/* following section is for tcp flow control */
	//
	//	printf("==================================================================\n");
	//	printf("[APP] Please send 10 x 1496 data to Hi3861L from tcp server in 60 seconds\n");
	//	printf("==================================================================\n");
	//	sleep(60);
	//
	//	printf("==================================================================\n");
	//	printf("[APP] Start to read data from lpw controller\n");
	//	printf("==================================================================\n");
	//	/* Receive data */
	//	for (int cnt = 0; cnt < 8; cnt++) {
	//		ret = lpw_tcp_recv(hd, tcp_recv2, sizeof(tcp_recv2));
	//		sum += ret;
	//		printf("[APP] This is %d times read\n", cnt + 1);
	//		printf("[APP] Try to read %d bytes from lpw_controller, ret = %d\n", sizeof(tcp_recv2), ret);
	//	}
	//	printf("==================================================================\n");
	//	printf("[APP] total read = %d\n", sum);
	//	printf("==================================================================\n");
	//
	//	printf("==================================================================\n");
	//	printf("[APP] Wait 5 seconds after resume tcp receive to get data haven't been received\n");
	//	printf("==================================================================\n");
	//	sleep(5);
	//
	//	printf("==================================================================\n");
	//	printf("[APP] Start to read data from lpw controller\n");
	//	printf("==================================================================\n");
	//	/* Receive data */
	//	for (int cnt = 0; cnt < 12; cnt++) {
	//		ret = lpw_tcp_recv(hd, tcp_recv2, sizeof(tcp_recv2));
	//		sum += ret;
	//		printf("[APP] This is %d times read\n", cnt + 1);
	//		printf("[APP] Try to read %d bytes from lpw_controller, ret = %d\n", sizeof(tcp_recv2), ret);
	//	}
	//	printf("==================================================================\n");
	//	printf("[APP] total read = %d\n", sum);
	//	printf("==================================================================\n");
	//
	//	/* Disconnect tcp */
	//	TCP_DISCONNECT_VERIFY(hd, 0);
	//
	//	printf("==================================================================\n");
	//
	//	/* iperf test */
	//	WIFI_SET_STA_MODE_VERIFY(hd, 0);
	//	WIFI_CONN_TO_VERIFY(hd, &conn, 0);
	//	while(1);

	lpw_close(hd);
	printf("Finish lpw_test program\n");

	printf("==================================================================\n");

	return 0;
}
