#include "agtx_lpw_cmd_common.h"

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>

#define MAX_RESPONSE 100

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif
#define WAKE_BY_LEN 8
static sem_t g_wake_sem;
static uint8_t g_wake_by[WAKE_BY_LEN];

void wake_init(void)
{
	sem_init(&g_wake_sem, 0, 0);
}
void wake_cmd_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	if (sub_type == WAKE_D2H_WAKE_BY) {
		if (len < WAKE_BY_LEN)
			printf("[ERR] len:%u is too low\n", len);
		memcpy(g_wake_by, data, WAKE_BY_LEN);
		sem_post(&g_wake_sem);
	} else {
		printf("[ERR] unknown wake cmd\n");
	}
}

void wake_req_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	static uint8_t wake_source;
	static uint8_t wake_set = 0; //bit[0]:src, bit[1]:sleep_detect

	//enabled sleep_detect_gpio, range from 0~31
	static uint32_t gpio_id_gpio = 0;
	uint8_t gpio_dir;
	uint8_t tmp;

	uint16_t tcp_hb_len;
	uint16_t tcp_hb_period;
	uint16_t tcp_wake_len;

	//enabled sleep_detect_adc, save corresponding gpio_id for checking if it's valid
	static uint32_t gpio_id_adc = 0;
	uint16_t adc_ch;
	uint16_t off_thre;
	uint16_t on_thre;

	uint32_t time;
	uint32_t response;

	switch (sub_type) {
	// uint8_t gpio_id , uint8_t active_level, uint16_t adc_channel, uint16_t off_val, uint16_t on_val
	case WAKE_H2D_PM:
		if (len < 8) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		memcpy(&tmp, data, 1);
		memcpy(&adc_ch, data + 2, 2);
		memcpy(&off_thre, data + 4, 2);
		memcpy(&on_thre, data + 6, 2);
		if (gpio_is_not_valid(tmp)) {
			printf("[ERR] invalid gpio id\n");
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (adc_is_not_valid(adc_ch)) {
			printf("[ERR] invalid adc channel\n");
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (on_thre <= off_thre) {
			printf("[ERR] power_on threshold can't be lower than power_off threshold\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		}
		//only when it's valid wake_src, the static wake_src will be set
		wake_source = tmp;
		adc_ch_to_gpio(adc_ch, &tmp);
		get_gpio_dir(tmp, &gpio_dir);
		if (tmp == wake_source) {
			printf("[ERR] gpio%u has been set as pwr_ctrl\n", tmp);
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else if (gpio_dir != 0) {
			printf("[ERR] gpio%u should be set as input direction\n", tmp);
			agtx_req_send(CMD_ERR, EIO, 0, NULL);
		} else {
			wake_set |= 0x1;
			//only when it's valid gpio, the static gpio_id_adc will be set
			gpio_id_adc = (1 << tmp);
			//set adc as wake_action
			agtx_cmd_send(CMD_WAKE_EVENT, WAKE_H2D_PM, len, data);
			agtx_req_send(CMD_ERR, 0, 0, NULL);
		}
		break;
	/* uint16_t heartbeat_len, uint16_t heartbeat_period, uint16_t wake_len, 
	uint8_t heartbeat_pack[heartbeat_len], uint8_t wake_pack[wake_len]*/
	case WAKE_H2D_TCP:
		if (!is_tcp_connected()) {
			printf("[ERR] tcp is not connected\n");
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
			break;
		}
		memcpy(&tcp_hb_len, data, 2);
		memcpy(&tcp_hb_period, data + 2, 2);
		memcpy(&tcp_wake_len, data + 4, 2);

		if (!tcp_hb_len) {
			printf("[ERR] heartbeat_len can't be 0\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		} else if (!tcp_hb_period) {
			printf("[ERR] heartbeat_period can't be 0\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		} else if (!tcp_wake_len) {
			printf("[ERR] wake_len can't be 0\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		} else if (!(wake_set & 0x1)) {
			printf("[ERR] sleep_init has to be called first\n");
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else {
			agtx_cmd_send(CMD_WAKE_EVENT, WAKE_H2D_TCP, len, data);
			wake_set |= (1 << 1);
			agtx_req_send(CMD_ERR, 0, 0, NULL);
		}
		break;
	// uint8_t gpio_id, uint8_t event_type (bit0: low, bit1: high, bit2: rising, bit3: falling)
	case WAKE_H2D_GPIO:
		if (len < 2) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		tmp = data[0];
		get_gpio_dir(tmp, &gpio_dir);
		if (!(wake_set & 0x1)) {
			printf("[ERR] sleep_init has to be called first\n");
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else if (gpio_is_not_valid(tmp)) {
			printf("[ERR] invalid gpio id\n");
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (tmp == wake_source) {
			printf("[ERR] gpio%u has been set as pwr_ctrl\n", tmp);
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else if (gpio_dir != 0) {
			printf("[ERR] gpio%u should be set as input direction\n", tmp);
			agtx_req_send(CMD_ERR, EIO, 0, NULL);
		} else if ((data[1] >> 4)) {
			printf("[ERR] event_type:%u is unknown\n", data[1]);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		} else if (gpio_id_adc & (0x1 << tmp)) {
			printf("[ERR] gpio%u has been set as sleep_detect adc\n", tmp);
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else {
			//only when it's valid gpio, the static gpio_id_gpio will be set
			gpio_id_gpio |= (1 << tmp);
			//set gpio as wake_action
			agtx_cmd_send(CMD_WAKE_EVENT, WAKE_H2D_GPIO, len, data);
			wake_set |= (1 << 1);
			agtx_req_send(CMD_ERR, 0, 0, NULL);
		}
		break;
	// uint16_t adc_ch, uint16_t above_under, uint16_t adc_value
	case WAKE_H2D_ADC:
		if (len < 6) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		memcpy(&adc_ch, data, 2);
		if (adc_is_not_valid(adc_ch)) {
			printf("[ERR] invalid adc channel\n");
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
			break;
		} else if (!(wake_set & 0x1)) {
			printf("[ERR] sleep_init has to be called first\n");
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
			break;
		}
		adc_ch_to_gpio(adc_ch, &tmp);
		get_gpio_dir(tmp, &gpio_dir);
		if (tmp == wake_source) {
			printf("[ERR] adc%u(gpio%u) has been set as pwr_ctrl\n", adc_ch, tmp);
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else if (gpio_dir != 0) {
			printf("[ERR] adc%u(gpio%u) should be set as input direction\n", adc_ch, tmp);
			agtx_req_send(CMD_ERR, EIO, 0, NULL);
		} else if (gpio_id_gpio & (0x1 << tmp)) {
			printf("[ERR] gpio%u has been set as sleep_detect_gpio\n", tmp);
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else {
			//only when it's valid gpio, the static gpio_id_adc will be set
			gpio_id_adc |= (1 << tmp);
			//set adc as wake_action
			agtx_cmd_send(CMD_WAKE_EVENT, WAKE_H2D_ADC, len, data);
			wake_set |= (1 << 1);
			agtx_req_send(CMD_ERR, 0, 0, NULL);
		}
		break;
	// uint32_t timeout, wake up SoC if timeout triggers, if it's 0, ignore it.
	// uint32_t response, response level when sleeping
	case WAKE_H2D_SLEEP_START:
		agtx_lpw_save_time();
		if (len < 8) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}

		// need to check before time_set, or time may get set
		if (!(wake_set & 0x1)) {
			printf("[ERR] sleep_init has to be called first\n");
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
			break;
		}

		memcpy(&response, &data[4], 4);
		if (response > MAX_RESPONSE) {
			response = MAX_RESPONSE;
			memcpy(&data[4], &response, 4);
			printf("[WARN] response is higher than MAX_RESPONSE, set response as MAX_RESPONSE\n");
		}
		memcpy(&time, data, 4);
		if (time != 0) {
			wake_set |= (1 << 1);
		}

		if (!(wake_set & (1 << 1))) {
			printf("[ERR] no sleep_detect_en exists, can't go to sleep\n");
			agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		} else {
			wake_set = 0;
			gpio_id_adc = gpio_id_gpio = 0;
			// sleep start, no need to return
			agtx_cmd_send(CMD_WAKE_EVENT, WAKE_H2D_SLEEP_START, len, data);
		}
		break;
	// no payload
	case WAKE_H2D_GET_WAKE_BY:
		agtx_cmd_send(CMD_WAKE_EVENT, WAKE_H2D_GET_WAKE_BY, len, data);
		sem_wait(&g_wake_sem);
		agtx_req_send(CMD_WAKE_EVENT, WAKE_D2H_WAKE_BY, WAKE_BY_LEN, g_wake_by);
		break;
	default:
		printf("[ERR] unknown wake request\n");
		agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		break;
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
