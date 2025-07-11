#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "agtx_types.h"
#include "event.h"
#include "gpio.h"
#include "led.h"
#include "ledevt.h"

extern int setLEDInform(char *led_event, int enable);

int getLEDInform(char **LEDInform)
{
	char s[4] = " ";
	char *info;
	char *en;
	int enabled;

	info = strtok(*LEDInform, s);
	en = strtok(NULL, s);
	enabled = atoi(en);
	strncpy(*LEDInform, info, LED_CLIENT_LENGTH);

	if (enabled >= 0) {
		enabled = (enabled != 0) ? 1 : 0;
		return enabled;
	} else {
		return -1;
	}
}

int getTimeTriger(struct timeval *pre_trig_tv, struct timeval *curr_trig_tv, long flash_period)
{
	gettimeofday(curr_trig_tv, NULL);
	if (curr_trig_tv->tv_sec >= (pre_trig_tv->tv_sec + flash_period)) {
		return 0;
	}
	return -1;
}

int handleButtonState(int currtime, int resettime, char *curr_client, char *prev_client, int button_reset)
{
	if (strcmp(prev_client, "OTA") != 0 || strcmp(prev_client, "Card_Upgrade") != 0) {
		if (button_reset != -1) {
			if ((currtime >= RESET_TRIGGER_SLOW) && (currtime < RESET_TRIGGER_FAST)) {
				setLEDInform("Reset_INFO_Slow", 1);
			} else if ((currtime >= RESET_TRIGGER_FAST) && (currtime < resettime)) {
				setLEDInform("Reset_INFO_Fast", 1);
			} else if (currtime < RESET_TRIGGER_SLOW) {
				if (strcmp(curr_client, "Reset_INFO_Fast") == 0) {
					setLEDInform("LED_OFF", 1);
					setLEDInform("Reset_INFO_Slow", 0);
					setLEDInform("Reset_INFO_Fast", 0);
				} else {
					setLEDInform("Reset_INFO_Slow", 0);
				}
				return AGTX_BUTTON_PRESS_TYPE_NONE;
			}
			return AGTX_BUTTON_PRESS_TYPE_PRESSING;
		} else {
			if (strcmp(curr_client, "Reset_INFO_Fast") == 0) {
				setLEDInform("LED_OFF", 1);
				setLEDInform("Reset_INFO_Slow", 0);
				setLEDInform("Reset_INFO_Fast", 0);
			} else {
				setLEDInform("Reset_INFO_Slow", 0);
			}
			return AGTX_BUTTON_PRESS_TYPE_RELEASE;
		}
	}
	return AGTX_BUTTON_PRESS_TYPE_NONE;
}

void *handleLEDaction(void *rsv)
{
	struct LED_EVENT_TABLE_S *levt;
	struct LED_EVENT_TABLE_S pre_levt;
	struct timeval curr_trig_tv;
	struct timeval pre_trig_tv;

	levt = (struct LED_EVENT_TABLE_S *)rsv;
	Gpio *gpio1 = malloc(sizeof(*gpio1));
	Gpio *gpio2 = malloc(sizeof(*gpio1));
	int r_enabled;
	int ret = 1;
	int trig_timeout_flag = 0;
	int led_switch_clear_flag = 0;
	/*Define led trigger type*/
	int dn_enabled = !levt->led_init_list.trigger_type_level;
	memset(&curr_trig_tv, 0, sizeof(struct timeval));
	memset(&pre_trig_tv, 0, sizeof(struct timeval));
	memset(&pre_levt, 0, sizeof(struct LED_EVENT_TABLE_S));
	memcpy(&pre_levt, levt, sizeof(*levt));

	/*Reset init led status*/
	gpio1->id = levt->led_init_list.pin;
	gpio1->direction = GPIO_OUT;
	gpio1->value = dn_enabled;
	if (levt->led_init_list.pin >= 0) {
		setGpioValue(gpio1, dn_enabled);
	}
	gpio2->id = -1;
	gpio2->direction = GPIO_OUT;
	gpio2->value = dn_enabled;

	while (ret) {
		pthread_mutex_lock(&levt->lock);
		if ((*pre_levt.led_switch_enabled != *levt->led_switch_enabled) || (*levt->led_switch_enabled != 0)) {
			led_switch_clear_flag = 0;
		}
		if (levt->clear_flag == 0) {
			memcpy(&pre_levt, levt, sizeof(*levt));
		}
		pthread_mutex_unlock(&levt->lock);

		/*When client change or led switch off, reset the gpio value.*/
		if (levt->clear_flag != 0 || *pre_levt.led_switch_enabled == 0) {
			if (led_switch_clear_flag == 0) {
				if (gpio1->id >= 0) {
					setGpioValue(gpio1, dn_enabled);
					EVT_TRACE("In handle: gpio1 ready to clear=%d\n", gpio1->id);
				}
				if (gpio2->id >= 0) {
					setGpioValue(gpio2, dn_enabled);
					EVT_TRACE("In handle: gpio2 ready to clear=%d\n", gpio2->id);
				}
				led_switch_clear_flag = 1;
			}
			levt->clear_flag = 0;
			trig_timeout_flag = 0;

			if (*pre_levt.led_switch_enabled == 0) {
				usleep(pre_levt.polling_period_usec);
			}

			continue;
		}

		/*Define LED switch, 0 is open, 1 is close*/
		if (pre_levt.enabled) {
			r_enabled = !dn_enabled;
		} else {
			r_enabled = dn_enabled;
		}
		gpio1->id = pre_levt.pin[0];
		gpio1->direction = GPIO_OUT;
		gpio1->value = r_enabled;
		if (pre_levt.pin[1] >= 0) {
			gpio2->id = pre_levt.pin[1];
			gpio2->direction = GPIO_OUT;
			gpio2->value = r_enabled;
		}
		if (pre_levt.enabled) {
			if (strcmp(pre_levt.flash_type, "PTRN_FLASH") == 0) {
				setGpioValue(gpio1, r_enabled);
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, r_enabled);
				}
				usleep(pre_levt.flash_period);
				setGpioValue(gpio1, dn_enabled);
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, dn_enabled);
				}
				usleep(pre_levt.flash_period);
				continue;
			} else if (strcmp(pre_levt.flash_type, "PTRN_ON") == 0) {
				setGpioValue(gpio1, r_enabled);
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, r_enabled);
				}
				usleep(pre_levt.polling_period_usec);
				continue;
			} else if (strcmp(pre_levt.flash_type, "PTRN_OFF") == 0) {
				if (pre_levt.pin[0] >= 0) {
					setGpioValue(gpio1, dn_enabled);
				}
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, dn_enabled);
				}
				usleep(pre_levt.polling_period_usec);
				continue;
			} else if (strcmp(pre_levt.flash_type, "PTRN_TEMP") == 0) {
				if (pre_levt.pin[0] >= 0) {
					setGpioValue(gpio1, r_enabled);
				}
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, r_enabled);
				}
				if (trig_timeout_flag == 0) {
					gettimeofday(&curr_trig_tv, NULL);
					memcpy(&pre_trig_tv, &curr_trig_tv, sizeof(curr_trig_tv));
					trig_timeout_flag = 1;
					continue;
				}
				trig_timeout_flag = getTimeTriger(&pre_trig_tv, &curr_trig_tv, pre_levt.flash_period);
				if (trig_timeout_flag == 0) {
					setLEDInform(pre_levt.curr_client, 0);
					pre_levt.enabled = 0;
				}
				usleep(500000);
				continue;
			} else if (strcmp(pre_levt.flash_type, "PTRN_FLASH_ALT") == 0) {
				setGpioValue(gpio1, r_enabled);
				usleep(pre_levt.flash_period);
				setGpioValue(gpio1, dn_enabled);
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, r_enabled);
				}
				usleep(pre_levt.flash_period);
				if (pre_levt.pin[1] >= 0) {
					setGpioValue(gpio2, dn_enabled);
				}
				usleep(pre_levt.flash_period);
				continue;
			}
		} else {
			usleep(pre_levt.polling_period_usec);
		}
	}
	return NULL;
}