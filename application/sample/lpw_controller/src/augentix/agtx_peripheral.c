#include "agtx_lpw_cmd_common.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define GPIO_AMOUNT 15
#define ADC_AMOUNT 7
#define VALID_GPIO_GROUP              \
	{                             \
		2, 5, 6, 7, 9, 10, 14 \
	}
#define VALID_ADC_CH    \
	{               \
		2, 3, 4 \
	}
#define ADC_PINS                       \
	{                              \
		12, 4, 5, 7, 9, 11, 13 \
	}
static sem_t g_peri_sem;
// 0:input, 1:output/low, 2:output/high
static uint8_t g_gpio_dir[GPIO_AMOUNT];
static uint8_t g_data_buf[4];

void peri_init(void)
{
	sem_init(&g_peri_sem, 0, 0);
}

void adc_ch_to_gpio(uint16_t adc_ch, uint8_t *gpio_id)
{
	//different wifi-module must check this condition
	uint8_t adc_ch_to_gpio[] = ADC_PINS;
	*gpio_id = adc_ch_to_gpio[adc_ch];
}

int adc_is_not_valid(uint8_t adc_ch)
{
	uint8_t valid_adc_ch[] = VALID_ADC_CH;
	//different wifi-module must check this condition
	uint8_t i;
	for (i = 0; i < sizeof(valid_adc_ch) / sizeof(valid_adc_ch[0]); i++) {
		if (adc_ch == valid_adc_ch[i]) {
			return 0;
		}
	}
	return 1;
}

int gpio_is_not_valid(uint8_t gpio_id)
{
	//3,4:UART, burn
	//8: interrupt
	//11~13: SDIO
	//different wifi-module must check this condition
	uint8_t valid_gpio[] = VALID_GPIO_GROUP;
	uint8_t i;
	if (gpio_id < GPIO_AMOUNT) {
		for (i = 0; i < sizeof(valid_gpio) / sizeof(valid_gpio[0]); i++) {
			if (gpio_id == valid_gpio[i])
				return 0;
		}
	}
	return 1;
}

int get_gpio_dir(uint8_t gpio_id, uint8_t *gpio_dir)
{
	if (!gpio_is_not_valid(gpio_id)) {
		*gpio_dir = g_gpio_dir[gpio_id];
		return 0;
	}

	return -1;
}
void gpio_dir_sync(void)
{
	uint8_t valid_gpio[] = VALID_GPIO_GROUP;
	uint8_t i;
	uint8_t j;
	for (i = 0; i < GPIO_AMOUNT; i++) {
		for (j = 0; j < sizeof(valid_gpio) / sizeof(valid_gpio[0]); j++) {
			if (i == valid_gpio[j]) {
				agtx_cmd_send(CMD_PERIPHERAL, PERI_H2D_GPIO_GET_DIR, 1, &i);
				sem_wait(&g_peri_sem);
			}
		}
	}
}

void peripheral_cmd_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	switch (sub_type) {
	// payload 2 byte, { uint8_t goio_id, uint8_t config: 0: input, 1: output low, 2: output high }
	case PERI_D2H_GPIO_DIR:
		if (len < 2) {
			printf("[ERR][cmd] len:%u is too low\n", len);
		} else {
			g_gpio_dir[data[0]] = data[1];
			sem_post(&g_peri_sem);
		}
		break;
	case PERI_D2H_GPIO_INPUT:
		if (len < 1) {
			printf("[ERR][cmd] len:%u is too low\n", len);
		} else {
			memcpy(g_data_buf, data, 2);
			sem_post(&g_peri_sem);
		}
		break;
	case PERI_D2H_ADC_VAL:
		if (len < 4) {
			printf("[ERR][cmd] len:%u is too low\n", len);
		} else {
			memcpy(g_data_buf, data, 4);
			sem_post(&g_peri_sem);
		}
		break;
	default:
		printf("[ERR] unknown peri cmd\n");
		break;
	}
}
void peripheral_req_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	uint8_t gpio_id;
	uint8_t config;
	uint8_t gpio_val;
	uint16_t adc_ch;

	switch (sub_type) {
	// payload 2 byte, { uint8_t goio_id, uint8_t config: 0: input, 1: output low, 2: output high }
	case PERI_H2D_GPIO_SET_DIR:
		if (len < 2) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		gpio_id = data[0];
		config = data[1];
		if (gpio_is_not_valid(gpio_id)) {
			printf("[ERR] invalid gpio id:%u\n", gpio_id);
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (config > 2) {
			printf("[ERR] invalid config:%u\n", config);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		} else {
			agtx_cmd_send(CMD_PERIPHERAL, PERI_H2D_GPIO_SET_DIR, len, data);
			g_gpio_dir[gpio_id] = config;
			agtx_req_send(CMD_ERR, 0, 0, NULL);
		}
		break;
	// payload 1 byte, gpio_id
	case PERI_H2D_GPIO_GET_DIR:
		if (len < 1) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		gpio_id = data[0];
		if (gpio_is_not_valid(gpio_id)) {
			printf("[ERR] invalid gpio id:%u\n", gpio_id);
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else {
			// payload 2 byte, { uint8_t goio_id, uint8_t config: 0: input, 1: output low, 2: output high }
			g_data_buf[0] = gpio_id;
			g_data_buf[1] = g_gpio_dir[gpio_id];
			agtx_req_send(CMD_PERIPHERAL, PERI_D2H_GPIO_DIR, 2, g_data_buf);
		}
		break;
	// payload 2 byte, { uint8_t goio_id, uint8_t output_value }
	case PERI_H2D_GPIO_SET_OUTPUT:
		if (len < 2) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		gpio_id = data[0];
		gpio_val = data[1];
		if (gpio_is_not_valid(gpio_id)) {
			printf("[ERR] invalid gpio id:%u\n", gpio_id);
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (gpio_val > 1) {
			printf("[ERR] invalid output val:%u\n", gpio_val);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
		} else if (!g_gpio_dir[gpio_id]) {
			printf("[ERR] gpio%u dir is not output\n", gpio_id);
			agtx_req_send(CMD_ERR, EIO, 0, NULL);
		} else {
			agtx_cmd_send(CMD_PERIPHERAL, PERI_H2D_GPIO_SET_OUTPUT, len, data);
			g_gpio_dir[gpio_id] = 1 + gpio_val;
			agtx_req_send(CMD_ERR, 0, 0, NULL);
		}
		break;
	// payload 1 byte, gpio_id
	case PERI_H2D_GPIO_GET_INPUT:
		if (len < 1) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		gpio_id = data[0];
		if (gpio_is_not_valid(gpio_id)) {
			printf("[ERR] invalid gpio id:%u\n", gpio_id);
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (g_gpio_dir[gpio_id]) {
			printf("[ERR] gpio%u dir is not input\n", gpio_id);
			agtx_req_send(CMD_ERR, EIO, 0, NULL);
		} else {
			agtx_cmd_send(CMD_PERIPHERAL, PERI_H2D_GPIO_GET_INPUT, len, data);
			sem_wait(&g_peri_sem);
			// payload 2 byte, { uint8_t gpio_id, uint8_t input_value }
			agtx_req_send(CMD_PERIPHERAL, PERI_D2H_GPIO_INPUT, 2, g_data_buf);
		}
		break;
	// payload 2 byte, adc_ch
	case PERI_H2D_ADC_GET:
		if (len < 2) {
			printf("[ERR] len:%u is too low\n", len);
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
		memcpy(&adc_ch, data, 2);
		adc_ch_to_gpio(adc_ch, &gpio_id);
		if (adc_is_not_valid(adc_ch)) {
			printf("[ERR] invalid adc channel:%u\n", adc_ch);
			agtx_req_send(CMD_ERR, ENXIO, 0, NULL);
		} else if (g_gpio_dir[gpio_id]) {
			printf("[ERR] adc channel%u is not gpio/input\n", adc_ch);
			agtx_req_send(CMD_ERR, EIO, 0, NULL);
		} else {
			agtx_cmd_send(CMD_PERIPHERAL, PERI_H2D_ADC_GET, len, data);
			sem_wait(&g_peri_sem);
			// payload 4 byte, { uint16_t adc_ch, uint16_t adc_value }
			agtx_req_send(CMD_PERIPHERAL, PERI_D2H_ADC_VAL, 4, g_data_buf);
		}
		break;
	default:
		printf("[ERR] unknown peri req\n");
		agtx_req_send(CMD_ERR, EACCES, 0, NULL);
		break;
	}
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
