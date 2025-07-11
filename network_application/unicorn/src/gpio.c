#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <json.h>

#include "gpio.h"
#include "utils.h"
#include "unicorn_debug.h"

GpioFrame g_gpio;
AdcFrame g_adc;

void gpio_dispatch_cmd(char *jstr, char *cmd);
void adc_dispatch_cmd(char *jstr, char *cmd);

int parseGpio(char *jstr_in)
{
	int i = 0, array_len = 0;
	char *jstr, *cmd;

	array_len = unicorn_json_get_array_length(jstr_in, "gpio_alias", strlen(jstr_in));
	for (i = 0; i < array_len; i++) {
		jstr = unicorn_json_get_object_from_array(jstr_in, "gpio_alias", i, strlen(jstr_in));
		if (jstr == NULL) {
			DBG_MED("Error: Unable to parse gpio_alias, %s\n", jstr_in);
			continue;
		}
		cmd = unicorn_json_get_string(jstr, "name", strlen(jstr));
		if (cmd == NULL) {
			DBG_MED("Error: Unable to parse naem, %s\n", jstr);
		} else {
			gpio_dispatch_cmd(jstr, cmd);
		}

		if (jstr) {
			free(jstr);
		}
		if (cmd) {
			free(cmd);
		}
	}
	g_gpio.flag = 1;

	return 0;
}

int parseAdc(char *jstr_in)
{
	int i = 0, array_len = 0;
	char *jstr, *jstr_adc, *cmd;

	array_len = unicorn_json_get_array_length(jstr_in, "event", strlen(jstr_in));
	for (i = 0; i < array_len; i++) {
		jstr = unicorn_json_get_object_from_array(jstr_in, "event", i, strlen(jstr_in));
		if (jstr == NULL) {
			DBG_MED("Error: Unable to parse event, %s\n", jstr_in);
			continue;
		}
		cmd = unicorn_json_get_string(jstr, "name", strlen(jstr));
		jstr_adc = unicorn_json_get_object(jstr, "adc", strlen(jstr));
		if (cmd == NULL || jstr_adc == NULL) {
			DBG_MED("Error: Unable to parse ADC, %s\n", jstr_in);
		} else {
			adc_dispatch_cmd(jstr_adc, cmd);
		}

		if (jstr) {
			free(jstr);
		}
		if (cmd) {
			free(cmd);
		}
		if (jstr_adc) {
			free(jstr_adc);
		}
	}

	g_adc.flag = 1;

	return 0;
}

void gpio_dispatch_cmd(char *jstr, char *cmd)
{
	char *name = unicorn_json_get_string(jstr, "name", strlen(jstr));
	char *dir = unicorn_json_get_string(jstr, "dir", strlen(jstr));
	int pin_num = unicorn_json_get_int(jstr, "pin_num", strlen(jstr));
	int value = unicorn_json_get_int(jstr, "value", strlen(jstr));

	if (name == NULL || dir == NULL) {
		DBG_MED("Error: Unable to get name or dir, %s\n", jstr);
	} else if (strcmp(cmd, "PUSH_BUTTON_IN") == 0) {
		sprintf(g_gpio.button.name, "%s", name);
		sprintf(g_gpio.button.dir, "%s", dir);
		g_gpio.button.pin_num = pin_num;
		g_gpio.button.value = value;
	} else if (strcmp(cmd, "LIGHT_SENSOR_IN") == 0) {
		sprintf(g_gpio.light_sensor.name, "%s", name);
		sprintf(g_gpio.light_sensor.dir, "%s", dir);
		g_gpio.light_sensor.pin_num = pin_num;
		g_gpio.light_sensor.value = value;
	} else if (strcmp(cmd, "PIR_IN") == 0) {
		sprintf(g_gpio.pir.name, "%s", name);
		sprintf(g_gpio.pir.dir, "%s", dir);
		g_gpio.pir.pin_num = pin_num;
		g_gpio.pir.value = value;
	} else if (strcmp(cmd, "SD_CARD_IN") == 0) {
		sprintf(g_gpio.sd_card.name, "%s", name);
		sprintf(g_gpio.sd_card.dir, "%s", dir);
		g_gpio.sd_card.pin_num = pin_num;
		g_gpio.sd_card.value = value;
	} else if (strcmp(cmd, "IRCUT0_OUT") == 0) {
		sprintf(g_gpio.ir_cut[0].name, "%s", name);
		sprintf(g_gpio.ir_cut[0].dir, "%s", dir);
		g_gpio.ir_cut[0].pin_num = pin_num;
		g_gpio.ir_cut[0].value = value;
	} else if (strcmp(cmd, "IRCUT1_OUT") == 0) {
		sprintf(g_gpio.ir_cut[1].name, "%s", name);
		sprintf(g_gpio.ir_cut[1].dir, "%s", dir);
		g_gpio.ir_cut[1].pin_num = pin_num;
		g_gpio.ir_cut[1].value = value;
	} else if (strcmp(cmd, "ALARM_OUT") == 0) {
		sprintf(g_gpio.alarm.name, "%s", name);
		sprintf(g_gpio.alarm.dir, "%s", dir);
		g_gpio.alarm.pin_num = pin_num;
		g_gpio.alarm.value = value;
	} else if (strcmp(cmd, "IRLED_OUT") == 0) {
		sprintf(g_gpio.ir_led.name, "%s", name);
		sprintf(g_gpio.ir_led.dir, "%s", dir);
		g_gpio.ir_led.pin_num = pin_num;
		g_gpio.ir_led.value = value;
	} else if (strcmp(cmd, "LED0_OUT") == 0) {
		sprintf(g_gpio.led[0].name, "%s", name);
		sprintf(g_gpio.led[0].dir, "%s", dir);
		g_gpio.led[0].pin_num = pin_num;
		g_gpio.led[0].value = value;
	} else if (strcmp(cmd, "LED1_OUT") == 0) {
		sprintf(g_gpio.led[1].name, "%s", name);
		sprintf(g_gpio.led[1].dir, "%s", dir);
		g_gpio.led[1].pin_num = pin_num;
		g_gpio.led[1].value = value;
	} else {
		DBG_MED("Error: Unknown command %s\n", cmd);
	}

	if (name) {
		free(name);
	}
	if (dir) {
		free(dir);
	}
}

void adc_dispatch_cmd(char *jstr, char *cmd)
{
	char *name = cmd;
	int chn = unicorn_json_get_int(jstr, "chn", strlen(jstr));

	if (strcmp(cmd, "LIGHT_SENSOR_ADC") == 0) {
		sprintf(g_adc.light_sensor.name, "%s", name);
		g_adc.light_sensor.chn = chn;
	} else {
		DBG_MED("Error: Unknown command %s\n", cmd);
	}
}
