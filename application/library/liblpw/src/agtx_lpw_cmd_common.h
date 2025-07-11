#ifndef _AGTX_CMD_COMMON_H_
#define _AGTX_CMD_COMMON_H_

#include <errno.h>
#include <stdint.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef struct {
	uint8_t cmd_type;
	uint8_t sub_type;
	uint16_t length;
	uint8_t payload[];
} agtx_frame_t;

#define CMD_MAX_LEN 1500
#define PAYLOAD_MAX_LEN (CMD_MAX_LEN - sizeof(agtx_frame_t))

typedef enum {
	/* Augentix customized type */
	CMD_INFO = 70,
	CMD_WIFI,
	CMD_PERIPHERAL,
	CMD_TCP,
	CMD_WAKE_EVENT,
	CMD_FW_UPG,
	CMD_LOG,
	CMD_ERR, // subtype: generic error number
} agtx_cmd_t;

typedef enum {
	INFO_H2D_GET_FW_VER, // no payload
	INFO_H2D_GET_PROTOCOL_VER, // no payload
	INFO_D2H_FW_VER, // payload 3 bytes, x.y.z
	INFO_D2H_PROTOCOL_VER, // payload 3 bytes, x.y.z
	INFO_H2D_SET_TIME, // payload 4 bytes.
	INFO_H2D_GET_TIME, // no payload
	INFO_D2H_TIME, // payload 4 bytes.
} agtx_cmd_info_t;

typedef enum {
	WIFI_H2D_SET_MODE, // payload 1 or 1 + sizeof(lpw_wifi_ap_config_t) bytes, payload[0]: 0: STA, 1: AP. When setting as AP mode, payload of lpw_wifi_ap_config_t should be appended.
	WIFI_H2D_GET_STATUS, // no payload
	WIFI_D2H_STATUS, // payload: size of lpw_wifi_state_t
	WIFI_H2D_SCAN, // payload: size of lpw_wifi_scan_t
	WIFI_D2H_SCAN_RESULT, // payload: size of lpw_wifi_scan_result_t
	WIFI_H2D_CONN_AP, // payload: size of lpw_wifi_conn_t
} agtx_cmd_wifi_t;

typedef enum {
	PERI_H2D_GPIO_SET_DIR, // payload 2 bytes, uint8_t gpio_id, uint8_t config: 0: input, 1: output low, 2: output high
	PERI_H2D_GPIO_GET_DIR, // payload 1 byte, gpio_id
	PERI_D2H_GPIO_DIR, // payload 2 bytes, uint8_t gpio_id, uint8_t config: 0: input, 1: output low, 2: output high
	PERI_H2D_GPIO_SET_OUTPUT, // payload 2 bytes, uint8_t gpio_id, uint8_t output_value
	PERI_H2D_GPIO_GET_INPUT, // payload 1 byte, gpio_id
	PERI_D2H_GPIO_INPUT, // payload 2 bytes, uint8_t gpio_id, uint8_t input_value
	PERI_H2D_ADC_GET, // payload 2 bytes, uint16_t adc_channel
	PERI_D2H_ADC_VAL, // payload 4 bytes, uint16_t adc_channel, uint16_t adc_value

} agtx_cmd_peri_t;

typedef enum {
	TCP_H2D_CONN_TO, // payload: server URL & port, ex: "google.com:8080",  "192.168.10.113:8888"
	TCP_H2D_TLS_CONN_TO, // payload: server URL & port, ex: "google.com:8080",  "192.168.10.113:8888"
	TCP_H2D_DISCONN, // no payload
	TCP_D2H_CONN_STATUS, // payload 1 byte, 0: disconnected, other: connected
	TCP_H2D_SEND, // payload: data to send
	TCP_D2H_SEND, // payload: data sent
	TCP_D2H_RECV, // payload: data received
	TCP_H2D_RECV, // no payload
	TCP_H2D_RECV_SUSPEND, // no payload
	TCP_H2D_RECV_RESUME, // no payload
} agtx_cmd_tcp_t;

typedef enum {
	WAKE_H2D_PM, // payload 8 bytes, uint8_t gpio id, uint8_t active_level, uint16_t adc_channel, uint16_t off_val, uint16_t on_val
	WAKE_H2D_TCP, // uint16_t heartbeat_len, uint16_t heartbeat_period, uint16_t wake_len, uint16_t bad_conn_handle, uint8_t heartbeat_pack[heartbeat_len], uint8_t wake_pack[wake_len]
	WAKE_H2D_GPIO, // payload 2 bytes, uint8_t gpio_id, uint8_t event_type (bit0: low, bit1: high, bit2: rising, bit3: falling)
	WAKE_H2D_ADC, // payload 6 bytes, uint16_t adc_channel, uint16_t above_under, uint16_t adc_value
	// after a wakeup event occurs, the wakeup configurations will be reset,
	// the application should configure setting again before going to sleep
	WAKE_H2D_SLEEP_START, // payload 8 bytes,
	// uint32_t timeout, 0 means no timeout limit
	// uint32_t response, level 0~9; the lower response is, the faster sleep detection is.
	WAKE_H2D_GET_WAKE_BY, // no payload
	WAKE_D2H_WAKE_BY, /* payload 8 bytes, uint32_t gpios, uint8_t adcs, uint8_t tcp, uint8_t timeout, uint8_t pm_adc
			     the bits in gpios represent each gpio, ex: bit 3 = gpio_3 event occurs
			     the bits in adcs represent each adc channel
			     tcp: 0: no TCP wake event, 1: TCP wake event occurs.
			     timeout: 0: no timeout event, 1: sleep timeout
			     pm_adc: 0: no power management event 1: power management event occurs */
} agtx_cmd_wake_t;

typedef enum {
	FWUPG_H2D_INIT, // uint32_t new_fw_size, uint32_t crc
	FWUPG_D2H_INIT_DONE,
	FWUPG_H2D_DOWNLOAD, // payload: new FW
	FWUPG_D2H_VERIFY, // payload 1 byte, 0: fail, other: success   (download完時，Hi3861L 自動執行 verify)
	FWUPG_H2D_START, // no payload
} agtx_cmd_fwupg_t;

#endif /* _AGTX_CMD_COMMON_H_ */
