#ifndef HIBER_DEMO_H_
#define HIBER_DEMO_H_

#include <agtx_lpw.h>
#include <netinet/in.h>
#include <stdint.h>

#define EVENT_LEN 4 // count of enum: gpio_event_t
#define IPV4(a, b, c, d) ((a << 0) | (b << 8) | (c << 16) | (d << 24))

/**
 * @brief enum of verbose level
 * @details
 */
typedef enum { V0, V1 } VERBOSE;

/**
 * @brief structure to store the power management information
 * @details
 */
typedef struct {
	uint8_t gpio; // power control gpio
	uint8_t active_status; // 0: active low; 1: active high
	uint8_t adc_ch; // adc channel for power management
	uint16_t off_thre; // power off threshold
	uint16_t on_thre; // power on threshold
} PWR_MGMT;

/**
 * @brief structure to store the TCP information
 * @details
 */
typedef struct {
	in_addr_t ser_ip; // server ip
	in_port_t ser_port; // server port
	int heartbeat_pkg_len; // length of TCP heartbeat package
	unsigned char *heartbeat_pkg; // TCP heartbeat package
	uint16_t heartbeat_period; // TCP heartbeat period
	int wakeup_pkg_len; // length of TCP wake up package
	unsigned char *wakeup_pkg; // TCP wake up package
	int bad_conn_handle; // wake SoC or not when tcp is disconnected
} SLEEP_TCP;

/**
 * @brief structure to store the GPIO information related to wake event
 * @details
 */
typedef struct {
	uint8_t gpio; // gpio which is used to detect wake event
	char event[EVENT_LEN + 1]; // event to be detected
} SLEEP_GPIO_DET;

/**
 * @brief structure to store the ADC information related to wake event
 * @details
 */
typedef struct {
	uint8_t adc; // adc channel which is used to detect wake event
	uint8_t above_under; // 0: detect under threshold; 1: detect above threshold event
	uint16_t threshold; // adc threshold
} SLEEP_ADC_DET;

/**
 * @brief structure to set gpio direction
 * @details
 */
typedef struct {
	uint8_t gpio;
	uint8_t dir; // 0: input; others: output
	uint8_t out_val; // ignore while config as input
} GPIO_DIR;

/**
 * @brief structure to store the parameters used in main function
 * @details
 */
typedef struct {
	PWR_MGMT pwr_mgmt;
	SLEEP_TCP *tcp;
	uint8_t sleep_tcp_wake; // 0: tcp wake disable; 1: tcp wake enable
	SLEEP_GPIO_DET *sleep_gpio_det;
	uint8_t sleep_gpio_det_cnt; // 0: gpio detect disable; others: gpio detect enable
	SLEEP_ADC_DET *sleep_adc_det;
	uint8_t sleep_adc_det_cnt; // 0: adc detect disable; others: adc detect enable
	uint32_t sleep_timeout; // 0: timeout detect disable; others: timeout (sec)
	uint8_t sleep_response; // response level from 0~9
	GPIO_DIR *gpio_dir;
	uint8_t gpio_dir_cnt; // 0: no gpio need to set direction; others: gpio(s) need to set direction
} HIBER_DEMO_PARAM;

/**
 * @brief call API from Lib LPW to start to sleep
 * @details
 *  the process is about:
 *  1. connect the device to the server
 *  2. enable the wake condition(s) which relates TCP, GPIO, ADC or timeout
 *  3. let the device go to sleep
 * @param[in] hiber_param parameter structure which is updated by config
 * @return int
 * @retval 0  	 sleep success
 * @retval Not 0 sleep failure
 * @see
 */
int HIBER_appGoToSleep(HIBER_DEMO_PARAM hiber_param);

#endif /* HIBER_DEMO_H_ */
