#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "event.h"
#include "led.h"
#include "agtx_types.h"

int AGTX_EVENT_execCmd(void *action_args, void *rsv)
{
	AGTX_UNUSED(rsv);

	openlog("EXEC_CMD", LOG_PID, LOG_USER);
	int status;
	char buf[128];

	sprintf(buf, "%s", (char *)action_args);
	status = system(buf);

	if (status == -1) {
		EVT_ERR("System error!\n");
	} else {
		if (WIFEXITED(status)) {
			if (WEXITSTATUS(status) == 0) {
				syslog(LOG_INFO, "(%s) be excuted successfully.\n", buf);
			} else {
				EVT_ERR("Run cmd fail and exit code is %d (%s)!\n", WEXITSTATUS(status), buf);
			}
		} else {
			EVT_ERR("exit status is %d (%s)!\n", WEXITSTATUS(status), buf);
		}
	}

	closelog();
	return status;
}

int AGTX_EVENT_print(void *action_args, void *rsv)
{
	AGTX_UNUSED(rsv);

	char buf[128];
	sprintf(buf, "%s", (char *)action_args);

	EVT_INFO("%s\n", buf);

	return 0;
}

int AGTX_EVENT_parseString(void *action_args, void *rsv)
{
	struct LED_EVENT_TABLE_S *levt;
	levt = (struct LED_EVENT_TABLE_S *)rsv;
	char buf[128];
	int i;
	int k = 0;
	char s[4] = " ";
	char *led_num;
	char *led_1;
	char *led_2;
	char *flash_type;
	char *flash_period;
	int led_number;
	AGTX_INT32 flash_period_time;

	/* "action_args": " led_num led_1 led_2 flash_type flash_period " */
	sprintf(buf, "%s", (char *)action_args);
	led_num = strtok(buf, s);
	led_number = atoi(led_num);

	led_1 = strtok(NULL, s);
	if (led_number > 1) {
		led_2 = strtok(NULL, s);
	}
	flash_type = strtok(NULL, s);
	flash_period = strtok(NULL, s);
	flash_period_time = atoi(flash_period);

	levt->led_num = led_number;
	strncpy(levt->flash_type, flash_type, LED_FLASH_LENGTH);
	levt->flash_period = flash_period_time;

	for (i = 0; i < LED_PIN_NUM_SIZE; i++) {
		if ((strcmp(levt->led[i].pin_name, led_1) == 0)) {
			levt->pin[k] = levt->led[i].pin;
			k++;
		}
	}
	if (led_number > 1) {
		for (i = 0; i < LED_PIN_NUM_SIZE; i++) {
			if ((strcmp(levt->led[i].pin_name, led_2) == 0)) {
				levt->pin[k] = levt->led[i].pin;
				k++;
			}
		}
	}
	for (k = k; k < LED_PIN_NUM_SIZE; k++) {
		levt->pin[k] = -1;
	}

	return 0;
}
