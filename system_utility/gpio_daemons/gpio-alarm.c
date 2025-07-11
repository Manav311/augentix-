/*
 * gpio_alarm.c: Augentix GPIO alarm detection application
 */
#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>

/*
 * Alarm trigger condition:
 * LVL_HIGH: High level triggered
 * LVL_LOW: Low level triggered
 * */
enum alarm_mode {
	LVL_LOW = 0,
	LVL_HIGH
	//EDGE_POS,
	//EDGE_NEG,
};

#define POLLING_PERIOD_SEC 1
#define ALARM_GPIO 57
#define ALARM_MODE LVL_HIGH

typedef struct {
	Gpio * gpio;
	int mode;
	int state;
} Alarm;

static Alarm g_alarm;

static void doTriggerAlarm(void)
{
	/* TODO: Alarm operation */
	syslog(LOG_NOTICE, "Alarm has been triggered!");
}

static int isAlarmTriggered(void)
{
	Value v = getGpioValue(g_alarm.gpio);
	return ( ((g_alarm.mode == LVL_LOW)  && (v == GPIO_LOW) )
	      || ((g_alarm.mode == LVL_HIGH) && (v == GPIO_HIGH)) );
}

int main()
{
	g_alarm.gpio = NULL;

	openlog("alarm_daemon", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Alarm daemon started.");

	g_alarm.gpio = initGpio(ALARM_GPIO);
	if (g_alarm.gpio == NULL) {
		syslog(LOG_ERR, "Failed to initialize GPIO alarm!");
		closelog();
		return -1;
	}

	syslog(LOG_NOTICE, "GPIO %d exported", ALARM_GPIO);
	setGpioDirection(g_alarm.gpio, "in");
	syslog(LOG_NOTICE, "GPIO %d set as input pin", ALARM_GPIO);
	g_alarm.mode = ALARM_MODE;

	while (1) {
		if (isAlarmTriggered()) {
			doTriggerAlarm();
		}
		sleep(POLLING_PERIOD_SEC);
	}

	closelog();
	return 0;
}
