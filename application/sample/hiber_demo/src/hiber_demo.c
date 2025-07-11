#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "hiber_demo.h"
#include "log.h"

#define DEBUG

/* default setting */
#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define EVENT_TRUE_CHAR '1'
#define GPIO_OUTPUT 1

/**
 * @brief transform event from string to enum gpio_event_t 
 * @details
 *  event which is read from config is in char array type and is stored in 
 *  parameter structure, it needs to be transformed into enum gpio_event_t type.
 * @param[in] *event event from parameter
 * @return gpio_event_t
 * @retval 0      no event
 * @retval Not 0  with event
 * @see
 */
static gpio_event_t strToGpioEvent(char *event)
{
	gpio_event_t gpio_event = 0;
	int shift_cnt = 0;

	for (int i = 0; i < EVENT_LEN; i++) {
		shift_cnt = (EVENT_LEN - i - 1);
		if (EVENT_TRUE_CHAR == event[i]) {
			gpio_event = (gpio_event | (0x01 << shift_cnt));
		}
	}
	return gpio_event;
}

/**
 * @brief synchronize current wifi status
 * @details
 *  get firmware version and wifi state
 * @param[in] hd          lpw handler
 * @param[in] *fw_ver     firmware version data structure
 * @param[in] *wifi_state wifi state data structure
 * @return ret
 * @retval 0      success
 * @retval Not 0  error no
 * @see
 */
static int syncModuleWifiStatus(lpw_handle hd, lpw_fw_ver_t *fw_ver, lpw_wifi_state_t *wifi_state)
{
	int ret = 0;

	/* get LPW version */
	lpw_module_get_version(hd, fw_ver);
	log_info("[HIBER_appGoToSleep] LPW module version: %d.%d.%d\n", fw_ver->ver[0], fw_ver->ver[1], fw_ver->ver[2]);

	/* get wifi status */
	ret = lpw_wifi_get_status(hd, wifi_state);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Cannot get wifi status.\n");
		goto error;
	}
	DBG("[HIBER_appGoToSleep] wifi connected to %s\n", wifi_state->ssid);

error:
	return ret;
}

/**
 * @brief access module's peripherals
 * @details
 *  set/get GPIO's direction and output value
 *  get ADC's value
 * @param[in] hd                lpw handler
 * @param[in] gpio_dir_cnt      count of element in *gpio_dir
 * @param[in] *gpio_dir         a pointer to a list of GPIO_DIR
 * @param[in] sleep_adc_det_cnt count of element in *sleep_adc_det
 * @param[in] *sleep_adc_det    a pointer to a list of SLEEP_ADC_DET
 * @return ret
 * @retval 0      success
 * @retval Not 0  error no
 * @see
 */
static int accessModulePeripherals(lpw_handle hd, int gpio_dir_cnt, GPIO_DIR *gpio_dir, int sleep_adc_det_cnt,
                                   SLEEP_ADC_DET *sleep_adc_det)
{
	int ret = 0;

	/* set gpio direction */
	for (int i = 0; i < gpio_dir_cnt; i++) {
		ret = lpw_gpio_set_dir(hd, gpio_dir[i].gpio, gpio_dir[i].dir, gpio_dir[i].out_val);
		if (ret < 0) {
			log_err("[HIBER_appGoToSleep] Set GPIO[%d] to dir: %d with val: %d fail.\n", gpio_dir[i].gpio,
			        gpio_dir[i].dir, gpio_dir[i].out_val);
			goto error;
		}
		DBG("[HIBER_appGoToSleep] Set GPIO[%d] to dir: %d with val: %d success.\n", gpio_dir[i].gpio,
		    gpio_dir[i].dir, gpio_dir[i].out_val);
	}

	/* get gpio direction */
	for (int i = 0; i < gpio_dir_cnt; i++) {
		ret = lpw_gpio_get_dir(hd, gpio_dir[i].gpio);
		if (ret < 0) {
			log_err("[HIBER_appGoToSleep] Get GPIO[%d] direction fail.\n", gpio_dir[i].gpio);
			goto error;
		}
		DBG("[HIBER_appGoToSleep] GPIO[%d] direction: %d.\n", gpio_dir[i].gpio, ret);
	}

	for (int i = 0; i < gpio_dir_cnt; i++) {
		/* get input value of gpio which direction is input*/
		if (gpio_dir[i].dir == 0) {
			ret = lpw_gpio_get_input(hd, gpio_dir[i].gpio);
			if (ret < 0) {
				log_err("[HIBER_appGoToSleep] Get GPIO[%d] input value fail.\n", gpio_dir[i].gpio);
				goto error;
			}
			DBG("[HIBER_appGoToSleep] GPIO[%d] input value: %d.\n", gpio_dir[i].gpio, ret);
		}
	}

	/* get adc value */
	for (int i = 0; i < sleep_adc_det_cnt; i++) {
		ret = lpw_adc_get(hd, sleep_adc_det[i].adc);
		if (ret < 0) {
			log_err("[HIBER_appGoToSleep] Get ADC[%d] value fail.\n", sleep_adc_det[i].adc);
		}
		DBG("[HIBER_appGoToSleep] Get ADC[%d] value: %d.\n", sleep_adc_det[i].adc, ret);
	}

error:
	return ret;
}

/**
 * @brief access Tcp
 * @details
 * connect to TCP and send/receive data
 * @param[in] hd       lpw handler
 * @param[in] ser_ip   IP of server
 * @param[in] ser_port port of server
 * @return ret
 * @retval 0      success
 * @retval Not 0  error no
 * @see
 */
static int accessTcp(lpw_handle hd, in_addr_t ser_ip, in_port_t ser_port)
{
	int ret = 0;
	DBG("[HIBER_appGoToSleep] Server IP: %x port: %d.\n", ser_ip, ser_port);

	/* connect to the server via TCP */
	ret = lpw_tcp_connect_to(hd, ser_ip, ser_port);
	if (ret != 0) {
		log_err("[HIBER_appGoToSleep] Connect server fail.\n");
		goto error;
	}
	DBG("[HIBER_appGoToSleep] Connect server success.\n");
	/* (optional) device authentication: the server gives authorization to the device*/
error:
	return ret;
}

/**
 * @brief set wake-up event and let SoC go to sleep
 * @details
 * @param[in] hd                 lpw handler
 * @param[in] pwr_mgmt           power management info.
 * @param[in] tcp                TCP setting info.
 * @param[in] sleep_tcp_wake     0: no TCP wake-up event; 1: has TCP wake-up event
 * @param[in] sleep_gpio_det     info. of GPIO which are in charge of wake-up event
 * @param[in] sleep_gpio_det_cnt count of element in *sleep_gpio_det
 * @param[in] sleep_adc_det      info. of ADC which are in charge of wake-up event
 * @param[in] sleep_adc_det_cnt  count of element in *sleep_adc_det
 * @param[in] sleep_timeout      0: no timeout wake-up event; > 0: has timeout wake-up event 
 * @param[in] sleep_response     response level(0~9); the lower value makes response faster
 * @return ret
 * @retval 0      success
 * @retval Not 0  error no
 * @see
 */
static int setWakeUpEventAndSleep(lpw_handle hd, PWR_MGMT pwr_mgmt, SLEEP_TCP *tcp, int sleep_tcp_wake,
                                  SLEEP_GPIO_DET *sleep_gpio_det, int sleep_gpio_det_cnt, SLEEP_ADC_DET *sleep_adc_det,
                                  int sleep_adc_det_cnt, int sleep_timeout, int sleep_response)
{
	int ret = 0;
	tcp_wake_t tcp_wake_pattern;

	/* sleep init: initialize through power control GPIO */
	ret = lpw_pm_init(hd, pwr_mgmt.gpio, pwr_mgmt.active_status, pwr_mgmt.adc_ch, pwr_mgmt.off_thre,
	                  pwr_mgmt.on_thre);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Power management init fail. Power control GPIO[%d], Power Management ADC[%d].\n",
		        pwr_mgmt.gpio, pwr_mgmt.adc_ch);
		goto error;
	}
	DBG("[HIBER_appGoToSleep] Sleep init success. Power control GPIO[%d], Power Management ADC[%d], off_thre[%d], on_thre[%d].\n",
	    pwr_mgmt.gpio, pwr_mgmt.adc_ch, pwr_mgmt.off_thre, pwr_mgmt.on_thre);

	/* set power control GPIO direction and value */
	ret = lpw_gpio_set_dir(hd, pwr_mgmt.gpio, GPIO_OUTPUT, pwr_mgmt.active_status);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Power control GPIO[%d] set direction and value fail.\n", pwr_mgmt.gpio);
		goto error;
	}
	DBG("[HIBER_appGoToSleep] Power control GPIO[%d] set direction: %d and value: %d.\n", pwr_mgmt.gpio,
	    GPIO_OUTPUT, pwr_mgmt.active_status);

	/*
	 * one of the following wake-up event should be chosen:
	 * 1. TCP: is enabled by lpw_sleep_tcp_wake_en()
	 * 2. GPIO: is enabled by lpw_sleep_detect_gpio_en()
	 * 3. ADC: is ebabled by lpw_sleep_detect_adc_en() 
	 * 4. timeout: is enabled by lpw_sleep_start() which timeout 0 equals to no timeout 
	 */

	/* enable TCP wakeup */
	if (sleep_tcp_wake) {
		/* setup tcp wake patter */
		tcp_wake_pattern.keep_alive_pack_len = tcp->heartbeat_pkg_len;
		tcp_wake_pattern.keep_alive_period = tcp->heartbeat_period;
		tcp_wake_pattern.wake_pack_len = tcp->wakeup_pkg_len;
		tcp_wake_pattern.keep_alive_pack = tcp->heartbeat_pkg;
		tcp_wake_pattern.wake_pack = tcp->wakeup_pkg;
		tcp_wake_pattern.bad_conn_handle = tcp->bad_conn_handle;

		ret = lpw_sleep_tcp_wake_en(hd, &tcp_wake_pattern);
		if (ret < 0) {
			log_err("[HIBER_appGoToSleep] Enable TCP detection fail.\n");
			goto error;
		}
		DBG("[HIBER_appGoToSleep] Enable TCP detection success.\n");
	}

	/* enable GPIO wakeup */
	for (int i = 0; i < sleep_gpio_det_cnt; i++) {
		gpio_event_t tmp_event = strToGpioEvent(sleep_gpio_det[i].event);

		ret = lpw_sleep_detect_gpio_en(hd, sleep_gpio_det[i].gpio, tmp_event);
		if (ret < 0) {
			log_err("[HIBER_appGoToSleep] Enable GPIO[%d] detection fail.\n", sleep_gpio_det[i].gpio);
			goto error;
		}
		DBG("[HIBER_appGoToSleep] Enable GPIO[%d] detection success with event: %d.\n", sleep_gpio_det[i].gpio,
		    tmp_event);
	}

	/* enable ADC wakeup */
	for (int i = 0; i < sleep_adc_det_cnt; i++) {
		ret = lpw_sleep_detect_adc_en(hd, sleep_adc_det[i].adc, sleep_adc_det[i].threshold,
		                              sleep_adc_det[i].above_under);
		if (ret < 0) {
			log_err("[HIBER_appGoToSleep] Enable ADC[%d] detection fail.\n", sleep_adc_det[i].adc);
			goto error;
		}
		DBG("[HIBER_appGoToSleep] Enable ADC[%d] detection success.\n", sleep_adc_det[i].adc);
	}

	/* 
	 * synchronize data to non-volatile storage 
	 * NOTE: this step should be done before sleep
	 * */
	system("sync");

	/* sleep start */
	ret = lpw_sleep_start(hd, sleep_timeout, sleep_response);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Sleep start fail.\n");
		goto error;
	}

error:
	return ret;
}

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
int HIBER_appGoToSleep(HIBER_DEMO_PARAM param)
{
	int ret = 0;
	lpw_fw_ver_t fw_ver;
	lpw_wifi_state_t wifi_state;

	DBG("[HIBER_appGoToSleep] Sleep preparation start...\n");

	/* initialize lpw handler */
	lpw_handle hd = lpw_open();
	if (hd == (lpw_handle)NULL) {
		ret = -EPERM;
		log_err("[HIBER_appGoToSleep] Open LPW device fail.\n");
		goto error;
	}
	DBG("[HIBER_appGoToSleep] Open LPW device success.\n");

	/* sync. module wifi status */
	ret = syncModuleWifiStatus(hd, &fw_ver, &wifi_state);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Sync. module wifi status fail. Return val: %d.\n", ret);
		goto error;
	}
	DBG("[HIBER_appGoToSleep] Sync. module wifi status success.\n");

	/* access module peripherals */
	ret = accessModulePeripherals(hd, param.gpio_dir_cnt, param.gpio_dir, param.sleep_adc_det_cnt,
	                              param.sleep_adc_det);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Access module peripherals fail. Return val: %d.\n", ret);
		goto error;
	}
	DBG("[HIBER_appGoToSleep] Access module peripherals success.\n");

	/* access TCP */
	if (param.sleep_tcp_wake) {
		ret = accessTcp(hd, param.tcp->ser_ip, param.tcp->ser_port);
		if (ret != 0) {
			log_err("[HIBER_appGoToSleep] Access TCP fail. Return val: %d.\n", ret);
			goto error;
		}
		DBG("[HIBER_appGoToSleep] Access TCP success.\n");
	}

	/* set wake-up event and go to sleep */
	ret = setWakeUpEventAndSleep(hd, param.pwr_mgmt, param.tcp, param.sleep_tcp_wake, param.sleep_gpio_det,
	                             param.sleep_gpio_det_cnt, param.sleep_adc_det, param.sleep_adc_det_cnt,
	                             param.sleep_timeout, param.sleep_response);
	if (ret < 0) {
		log_err("[HIBER_appGoToSleep] Set wake-up event and sleep fail. Return val: %d.\n", ret);
		goto error;
	}

error:
	lpw_close(hd);

	return ret;
}
