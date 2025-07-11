/*
 * gpio_reset.c: Augentix GPIO reset detection application
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
 * Reset trigger condition:
 * LVL_HIGH: High level triggered
 * LVL_LOW: Low level triggered
 * */
enum reset_mode {
	LVL_LOW = 0,
	LVL_HIGH
	//EDGE_POS,
	//EDGE_NEG,
};

#define POLLING_PERIOD_SEC 1
#define RESET_GPIO 46
#define RESET_MODE LVL_LOW

typedef struct {
	Gpio * gpio;
	int mode;
	int state;
} Reset;

static Reset g_reset;

static void doTriggerReset(void)
{
	/* TODO: Reset operation */
	syslog(LOG_NOTICE, "Reset has been triggered!");
}

static int isResetTriggered(void)
{
	Value v = getGpioValue(g_reset.gpio);
	return ( ((g_reset.mode == LVL_LOW)  && (v == GPIO_LOW) )
	      || ((g_reset.mode == LVL_HIGH) && (v == GPIO_HIGH)) );
}

int main()
{
	g_reset.gpio = NULL;

	openlog("reset_daemon", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "Reset daemon started.");

	g_reset.gpio = initGpio(RESET_GPIO);
	if (g_reset.gpio == NULL) {
		syslog(LOG_ERR, "Failed to initialize GPIO reset!");
		closelog();
		return -1;
	}

	syslog(LOG_NOTICE, "GPIO %d exported", RESET_GPIO);
	setGpioDirection(g_reset.gpio, "in");
	syslog(LOG_NOTICE, "GPIO %d set as input pin", RESET_GPIO);
	g_reset.mode = RESET_MODE;

	while (1) {
		if (isResetTriggered())
			doTriggerReset();
		sleep(POLLING_PERIOD_SEC);
	}

	closelog();
	return 0;
}
