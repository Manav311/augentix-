#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

#include "agtx_types.h"
#include "pwm.h"

#define MAX_FILEPATH 128
#define MAX_BUF 8

int exportPWM(Value v)
{
	openlog("PWM", LOG_PID, LOG_USER);
	char buf[MAX_BUF];
	int fd, ret = 0;
	sprintf(buf, "%d", v);

	// printf("Export PMW %s\n", buf);
	fd = open("/sys/class/pwm/pwmchip0/export", O_WRONLY);

	if (fd < 0) {
		syslog(LOG_ERR, "Failed to open PWM device node!\n");
		return fd;
	}

	if (write(fd, buf, strlen(buf)) < 0) {
		syslog(LOG_ERR, "Failed to export to PWM device node!\n");
		ret = -1;
	}

	close(fd);
	closelog();
	return ret;
}

int enabledPWM(PWM *pwm, Value v)
{
	openlog("PWM", LOG_PID, LOG_USER);
	char buf[MAX_FILEPATH];
	char enabled[MAX_BUF];
	int fd, ret = 0;
	sprintf(enabled, "%d", v);

	sprintf(buf, "/sys/class/pwm/pwmchip0/pwm%d/enable", pwm->id);
	fd = open(buf, O_WRONLY);

	if (fd < 0) {
		syslog(LOG_ERR, "Failed to open PWM device node!\n");
		return fd;
	}

	if (write(fd, enabled, strlen(enabled)) < 0) {
		syslog(LOG_ERR, "Failed to enable to PWM device node!\n");
		ret = -1;
	}

	close(fd);
	closelog();
	return ret;
}

int unexportPWM(Value v)
{
	openlog("PWM", LOG_PID, LOG_USER);
	char buf[MAX_BUF];
	int fd, ret = 0;
	sprintf(buf, "%d", v);

	// printf("Unexport PWM %s\n", buf);
	fd = open("/sys/class/pwm/pwmchip0/unexport", O_WRONLY);

	if (write(fd, buf, strlen(buf)) < 0) {
		syslog(LOG_ERR, "Failed to unexport to PWM device node!\n");
		ret = -1;
	}

	close(fd);
	closelog();
	return ret;
}

void setPWMPeriod(PWM *pwm, int period)
{
	char filepath[MAX_FILEPATH];
	char buf[MAX_BUF];
	int fd;
	sprintf(buf, "%d", period);
	sprintf(filepath, "/sys/class/pwm/pwmchip0/pwm%d/period", pwm->id);

	fd = open(filepath, O_WRONLY);

	if (write(fd, buf, strlen(buf)) < 0) {
		syslog(LOG_ERR, "Failed to write to PWM period!\n");
	}

	close(fd);
	closelog();
}

void setPWMDutyCycle(PWM *pwm, int duty_cycle)
{
	char filepath[MAX_FILEPATH];
	char buf[MAX_BUF];
	int fd;
	sprintf(buf, "%d", duty_cycle);
	sprintf(filepath, "/sys/class/pwm/pwmchip0/pwm%d/duty_cycle", pwm->id);

	fd = open(filepath, O_WRONLY);

	if (write(fd, buf, strlen(buf)) < 0) {
		syslog(LOG_ERR, "Failed to write to PWM duty cycle!\n");
	}

	close(fd);
	closelog();
}