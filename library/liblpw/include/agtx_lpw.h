#ifndef LPW_H_
#define LPW_H_

#include <netinet/in.h>

#if defined __cplusplus
extern "C" {
#endif

#define WIFI_MAX_SSID_LEN 32
#define WIFI_MAX_KEY_LEN 64
#define WIFI_MAC_LEN 6

//lpw device handle
typedef int lpw_handle;

typedef struct {
	unsigned char ver[3];
} lpw_fw_ver_t;

typedef enum {
	WIFI_SECURITY_OPEN,
	WIFI_SECURITY_WEP,
	WIFI_SECURITY_WPA2PSK,
	WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	WIFI_SECURITY_WPAPSK,
	WIFI_SECURITY_WPA,
	WIFI_SECURITY_WPA2,
	WIFI_SECURITY_SAE,
	WIFI_SECURITY_WPA3_WPA2_PSK_MIX,
	WIFI_SECURITY_UNKNOWN
} wifi_auth_mode;

typedef enum { WIFI_PAIRWISE_UNKNOWN, WIFI_PAIRWISE_AES, WIFI_PAIRWISE_TKIP, WIFI_PAIRWISE_TKIP_AES_MIX } wifi_pairwise;

typedef enum {
	WIFI_CHANNEL_SCAN = 1,
	WIFI_SSID_SCAN,
	WIFI_SSID_PREFIX_SCAN,
	WIFI_BSSID_SCAN,
} wifi_scan_type;

typedef struct {
	char channel_num; // range: 1~14
	char authmode; // use wifi_auth_mode value
	char pairwise; // use wifi_pairwise value
	char *ssid;
	char *key; // key could be wep_key0 or psk value
} lpw_wifi_ap_config_t;

typedef struct {
	char mac[WIFI_MAC_LEN];
	char ip[4];
	char mask[4];
	char gw[4];
	char cc[4]; // country code
	unsigned int mode; // 0: STA mode, 1: AP mode, 2: DEFAULT mode
	signed char conn; // 0: unconnected, 1: connected
	char ssid[WIFI_MAX_SSID_LEN + 1];
} lpw_wifi_state_t;

typedef struct {
	char bssid[WIFI_MAC_LEN];
	char channel; // range: 1~14
	char scan_type; // use wifi_scan_type value
	char *ssid;
} lpw_wifi_scan_t;

typedef struct {
	char num; // numbers of matching target
	char bssid[WIFI_MAC_LEN];
	char channel; // range: 1~14
	char auth; // use wifi_auth_mode value
	char ssid[WIFI_MAX_SSID_LEN + 1];
} lpw_wifi_scan_result_t;

typedef struct {
	char bssid[WIFI_MAC_LEN];
	char auth; // use wifi_auth_mode value
	char pairwise; // use wifi_pairwise value
	char *ssid;
	char *key;
} lpw_wifi_conn_t;

typedef enum {
	GPIO_LOW = 0x01,
	GPIO_HIGH = 0x01 << 1,
	GPIO_RISING = 0x01 << 2,
	GPIO_FALLING = 0x01 << 3,
} gpio_event_t;

typedef struct {
	uint16_t keep_alive_pack_len; // length of keep alive package
	uint16_t keep_alive_period; // period for module to send keep alive package
	uint16_t wake_pack_len; // length of wakeup package
	uint16_t bad_conn_handle; // when bad connection occurred(wifi or tcp), 0: do nothing for battery performance, others: wake up for reconnection
	unsigned char *keep_alive_pack; // keep alive package pattern
	unsigned char *wake_pack; // wakeup package pattern
} tcp_wake_t;

typedef struct {
	unsigned int gpios; // each bit indicates each gpio pin, ex: if bit 3 is 1, means gpio 3 event occurs.
	unsigned char adcs; // each bit indicates each adc channel, ex: if bit 3 is 1, means adc 3 event occurs.
	unsigned char tcp; // 0: no TCP wake event, 1: TCP wake event occurs.
	unsigned char timeout; // 0: no timeout event, 1: sleep timeout
	unsigned char pm_adc; //0: no power management event 1: power management event occurs
} wake_event_t;
/**
 * lpw_open - open lpw device
 *
 * return lpw device handle or NULL for failure
 *
 */
lpw_handle lpw_open(void);

/**
 * lpw_close - close lpw device
 *
 * @hd: the lpw device handle to be closed.
 *
 */
void lpw_close(lpw_handle hd);

/**
 * lpw_module_get_version - get the FW version of lpw module
 *
 * Get fw version in 3 bytes representing x.y.z
 *
 * @hd: the lpw device handle
 * @ver: pointer of lpw_fw_ver_t struct to receive fw version.
 *
 */
void lpw_module_get_version(lpw_handle hd, lpw_fw_ver_t *ver);

/**
 * lpw_wifi_get_status - get wifi status
 *
 * Get all the wifi status from the lpw_wifi_state_t data structure.
 *
 * @hd: the lpw device handle
 * @st: pointer of lpw_wifi_state_t struct to get wifi status.
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_wifi_get_status(lpw_handle hd, lpw_wifi_state_t *st);

/**
 * lpw_wifi_set_sta_mode - set wifi module as sta mode
 *
 * Set the wifi module as STA mode
 *
 * @hd: the lpw device handle
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_wifi_set_sta_mode(lpw_handle hd);

/**
 * lpw_wifi_scan - scan specific wifi AP
 *
 * Scan specific wifi AP
 *
 * @hd: the lpw device handle
 * @scan: pointer of lpw_wifi_scan_t struct to describe scan target
 * @result: pointer of lpw_wifi_scan_result_t for the scan result
 *
 * return 0 for target exist, negative number for error
 *
 */
int lpw_wifi_scan(lpw_handle hd, lpw_wifi_scan_t *scan, lpw_wifi_scan_result_t *result);

/**
 * lpw_wifi_connect_to - connect to a specific wifi AP
 *
 * Connect to a specific wifi AP
 * This function is only workable under STA mode.
 *
 * @hd: the lpw device handle
 * @conn: pointer of lpw_wifi_conn_t struct describe connection configurations
 *
 * return 0 for connected, negative number for error
 *
 */
int lpw_wifi_connect_to(lpw_handle hd, lpw_wifi_conn_t *conn);

/**
 * lpw_wifi_set_ap_mode - set wifi module as ap mode
 *
 * Set the wifi module as AP mode
 *
 * @hd: the lpw device handle
 * @config: pointer of lpw_wifi_ap_config_t struct for wifi AP configurations.
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_wifi_set_ap_mode(lpw_handle hd, lpw_wifi_ap_config_t *config);

/**
 * lpw_gpio_set_dir - set module's gpio direction
 *
 * Set module's gpio direction
 *
 * @hd: the lpw device handle
 * @gpio_id: gpio pin
 * @dir: gpio direction, 0: input, others: output.
 * @out: default output value, this parameter would be ignored when dir = 0(input)
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_gpio_set_dir(lpw_handle hd, int gpio_id, int dir, int out);

/**
 * lpw_gpio_get_dir - get module's gpio direction
 *
 * Get module's gpio direction
 *
 * @hd: the lpw device handle
 * @gpio_id: gpio pin
 *
 * return 0: input, positive: output, negative: error
 *
 */
int lpw_gpio_get_dir(lpw_handle hd, int gpio_id);

/**
 * lpw_gpio_set_output - set module's gpio output value
 *
 * Set module's gpio output value
 * The gpio pin should be set as output direction, which can be set by lpw_gpio_set_dir() function call.
 *
 * @hd: the lpw device handle
 * @gpio_id: gpio pin
 * @out: output value
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_gpio_set_output(lpw_handle hd, int gpio_id, int out);

/**
 * lpw_gpio_get_input - get module's gpio input value
 *
 * Get module's gpio input value
 *
 * @hd: the lpw device handle
 * @gpio_id: gpio pin
 *
 * return 0: low, 1: high, negative number for error
 *
 */
int lpw_gpio_get_input(lpw_handle hd, int gpio_id);

/**
 * lpw_adc_get - get module's adc value
 *
 * Get module's adc value
 *
 * @hd: the lpw device handle
 * @adc_ch: adc channel
 *
 * return positive number for adc value, negative number for error
 *
 */
int lpw_adc_get(lpw_handle hd, int adc_ch);

/**
 * lpw_fw_upg - launch FW upgrade procedure
 *
 * Launch FW upgrade procedure.
 *
 * @hd: the lpw device handle.
 * @new_fw: provide the path and file name of new FW image file.
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_fw_upg(lpw_handle hd, unsigned char *new_fw);

/**
 * lpw_tcp_connect_to - Request module to connect TCP server
 *
 * Request module to connect TCP server.
 *
 * If tcp is connected already, call lpw_tcp_disconnect first to create a new tcp connection.
 *
 * @hd: the lpw device handle.
 * @ip: TCP server ip.
 * @port: TCP server port.
 *
 * return 0 for connecting successfully, negative number for error
 *
 */
int lpw_tcp_connect_to(lpw_handle hd, in_addr_t ip, in_port_t port);

/**
 * lpw_tcp_disconnect - Request module to disconnect TCP connection
 *
 * Request module to disconnect TCP connection.
 *
 * @hd: the lpw device handle.
 *
 */
void lpw_tcp_disconnect(lpw_handle hd);

/**
 * lpw_tcp_send - Request module to send data through TCP connection
 *
 * Request module to send data through TCP connection
 *
 * @hd: the lpw device handle.
 * @data: data to be sent.
 * @len: data length.
 *
 * return positive number for sent bytes, negative number for error
 * if it failed to send all data (sent bytes = len), return -EIO and disconnect tcp connection
 *
 */
int lpw_tcp_send(lpw_handle hd, unsigned char *data, int len);

/**
 * lpw_tcp_recv - Request module to receive data through TCP connection
 *
 * Request module to receive data through TCP connection
 *
 * This is a non-blocking function, return 0 when there is no data can be read.
 *
 * @hd: the lpw device handle.
 * @buf: buffer space.
 * @len: buffer size.
 *
 * return positive number for received bytes, negative number for error
 *
 */
int lpw_tcp_recv(lpw_handle hd, unsigned char *buf, int len);

/**
 * lpw_pm_init - Start the power management configuration procedure
 *
 * Start the power management configuration procedure
 * The gpio which is used to wake host up should be provided.
 * The adc_ch, power on threshold and power off threshold for host which are used to do power management should be provided.
 * This function should be called before other lpw_sleep_xx_en functions.
 * The power management takes effect once this function is first called, and keeps managing power even though rebooting wifi module.
 * The parameters would be replaced after this function is called again.
 *
 * Note:
 * the gpio should be configured as output direction by lpw_gpio_set_dir() function call.
 * The adc channel must be configured as input direction.
 *
 * @hd: the lpw device handle.
 * @gpio_id: gpio id
 * @active_level: 0: gpio low -> makes host power-on
 *                others: gpio high -> makes host power-on
 * @adc_ch: adc channel
 * @off: power off threshold
 * @on: power on threshold
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_pm_init(lpw_handle hd, int gpio_id, int active_level, int adc_ch, int off, int on);

/**
 * lpw_sleep_detect_gpio_en - Request module to detect gpio event during host sleeping
 *
 * Request module to detect gpio event during host sleeping
 * The gpio pin must be configured as input direction.
 *
 * @hd: the lpw device handle.
 * @gpio_id: detect events on specific gpio pin
 * @event: event to be detected. Multiple events could be set at the same time. ex: GPIO_HIGH | GPIO_RISING
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_sleep_detect_gpio_en(lpw_handle hd, int gpio_id, gpio_event_t event);

/**
 * lpw_sleep_detect_adc_en - Request module to detect adc event during host sleeping
 *
 * Request module to detect adc event during host sleeping
 * The adc channel must be configured as input direction.
 *
 * @hd: the lpw device handle.
 * @adc_ch: detect events on specific adc channel
 * @threshold: the adc threshold
 * @above_under: 0: detect under threshold event, 1: detect above threshold event
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_sleep_detect_adc_en(lpw_handle hd, int adc_ch, int threshold, int above_under);

/**
 * lpw_sleep_tcp_wake_en - Config wakeable by TCP connection
 *
 * The TCP connection must have been established by lpw_tcp_connect_to() function call.
 * During sleeping, the module periodically sends keep-alive packages to notify cloud server of the presence of device.
 * The wireless module monitors received TCP data while host SoC is sleeping.
 * Once received data matches wakeup packages, the wakeup flow will be activated.
 *
 * Note: The module only handles the keep-alive package while SoC is sleeping.
 * 	 Before SoC is set to sleep mode, application should handle keep-alive package to meet
 * 	 the requirement of cloud service.
 *
 * @hd: the lpw device handle.
 * @pattern: pointer to tcp_wake_t struct defining keep-alive and wakeup patterns.
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_sleep_tcp_wake_en(lpw_handle hd, tcp_wake_t *pattern);

/**
 * lpw_sleep_start - Request module to start the sleep_procedure.
 *
 * This function will never return success because the SoC is powered off during sleeping.
 * The user should do lpw_sleep_init and setup wakeup events by lpw_sleep_xxx_en functions or timeout in this function before going to sleep.
 *
 * @hd: the lpw device handle.
 * @timeout: define the max sleep_time, 0 for no limit on sleep_time
 * @response: define response level from 1~100, the lower value indicates faster response for tcp, gpio and adc detection
 *
 * return negative number for error
 *
 */
int lpw_sleep_start(lpw_handle hd, int timeout, int response);

/**
 * lpw_sleep_get_wake_event - Get the wake up event for the previous sleep
 *
 * Get the wake up event for the previous sleep.
 * The application might need this information for further operation.
 * For example, application might use adc to monitor battery voltage.
 * When application gets an adc event, it might initiate a battery status notification.
 *
 * Note: this function should be called before lpw_sleep_start which resets the sleep functionality
 * 	 and information of previous wake-up event.
 *
 * @hd: the lpw device handle.
 * @event: the data structure of wake_event_t indicating which event has been triggered
 *
 * return 0 for success, negative number for error
 *
 */
int lpw_sleep_get_wake_event(lpw_handle hd, wake_event_t *event);

#if defined __cplusplus
}
#endif

#endif /* LPW_H_ */
