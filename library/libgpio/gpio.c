#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>

const char * const g_gpio_direction[] = {"in", "out", "high", "low"};

#define MAX_FILEPATH 128
#define MAX_BUF 4

Gpio * initGpio(int id)
{
    int ret;
    Gpio* gpio = malloc(sizeof(*gpio));
    gpio->id = id;
    gpio->direction = GPIO_IN;
    gpio->value = 0;
    ret = exportGpio(gpio->id);
    if (ret < 0)
		printf("Warning: GPIO %d exported\n", id);
    return gpio;
}

void releaseGpio(Gpio * gpio)
{
    unexportGpio(gpio->value);
    free(gpio);
    gpio = NULL;
}

int exportGpio(Value v)
{
    openlog("GPIO", LOG_PID, LOG_USER);
    char buf[MAX_BUF];
    int fd, ret = 0;
    sprintf(buf, "%d", v);

    //printf("Export GPIO %s\n", buf);
    fd = open("/sys/class/gpio/export", O_WRONLY);

    if (fd < 0) {
        syslog(LOG_ERR, "Failed to open GPIO device node!\n");
        return fd;
    }

    if (write(fd, buf, strlen(buf)) < 0) {
        syslog(LOG_ERR, "Failed to write to GPIO device node!\n");
        ret = -1;
    }

    close(fd);
    closelog();
    return ret;
}

int unexportGpio(Value v)
{
    openlog("GPIO", LOG_PID, LOG_USER);
    char buf[MAX_BUF];
    int fd, ret = 0;
    sprintf(buf, "%d", v);

    //printf("Unexport GPIO %s\n", buf);
    fd = open("/sys/class/gpio/unexport", O_WRONLY);

    if (write(fd, buf, strlen(buf)) < 0) {
        syslog(LOG_ERR, "Failed to write to GPIO device node!\n");
        ret = -1;
    }

    close(fd);
    closelog();
    return ret;
}

void setGpioDirection(Gpio * gpio, char * direction)
{
    openlog("GPIO", LOG_PID, LOG_USER);
    char filepath[MAX_FILEPATH];
    int fd;
    sprintf(filepath, "/sys/class/gpio/gpio%d/direction", gpio->id);

    if (!strncmp(direction, g_gpio_direction[GPIO_IN], MAX_BUF)) {
        gpio->direction = GPIO_IN;
    } else if (!strncmp(direction, g_gpio_direction[GPIO_OUT], MAX_BUF)) {
        gpio->direction = GPIO_OUT;
    } else if (!strncmp(direction, g_gpio_direction[GPIO_OUT_HIGH], MAX_BUF)) {
        gpio->direction = GPIO_OUT_HIGH;
    } else if (!strncmp(direction, g_gpio_direction[GPIO_OUT_LOW], MAX_BUF)) {
        gpio->direction = GPIO_OUT_LOW;
    } else {
        syslog(LOG_ERR, "GPIO %d: Invalid GPIO direction!\n", gpio->id);
        return;
    }

    /*
        * printf("%s: Set GPIO %d direction to \"%s\"\n",
        *   filepath, gpio->id, direction);
        */
    fd = open(filepath, O_WRONLY);

    if (write(fd, direction, strlen(direction))< 0) {
        syslog(LOG_ERR, "Failed to write to GPIO device node!\n");
    }

    close(fd);
    closelog();
}

Direction getGpioDirection(Gpio * gpio)
{
    openlog("GPIO", LOG_PID, LOG_USER);
    /*return gpio->direction;*/
    char filepath[MAX_FILEPATH];
    char direction[MAX_BUF];
    int fd;
    sprintf(filepath, "/sys/class/gpio/gpio%d/direction", gpio->id);

    fd = open(filepath, O_RDONLY);

    if (read(fd, direction, MAX_BUF) < 0) {
        syslog(LOG_ERR, "Failed to read from GPIO device node %s!\n", filepath);
    }

    close(fd);

    if (!strncmp(direction, g_gpio_direction[GPIO_IN], MAX_BUF)) {
        gpio->direction = GPIO_IN;
    } else if (!strncmp(direction, g_gpio_direction[GPIO_OUT], MAX_BUF)) {
        gpio->direction = GPIO_OUT;
    } else {
        syslog(LOG_ERR, "GPIO %d: Invalid GPIO direction!\n", gpio->id);
    }

    closelog();
    return gpio->direction;
}

void setGpioValue(Gpio * gpio, Value v)
{
    openlog("GPIO", LOG_PID, LOG_USER);
    char filepath[MAX_FILEPATH];
    char c;
    int fd;
    sprintf(filepath, "/sys/class/gpio/gpio%d/value", gpio->id);
    gpio->value = v;
    c = (gpio->value == GPIO_LOW) ? '0' : '1';
    /*
        * printf("%s: Set GPIO %d value to \"%d\"\n",
        *   filepath, gpio->id, gpio->value);
        */
    fd = open(filepath, O_WRONLY);

    if (write(fd, &c, 1) < 0) {
        syslog(LOG_ERR, "Failed to write to GPIO device node!\n");
    }

    close(fd);
    closelog();
}

Value getGpioValue(Gpio * gpio)
{
    openlog("GPIO", LOG_PID, LOG_USER);
    /* return gpio->value; */
    char filepath[MAX_FILEPATH];
    char c;
    int fd;
    sprintf(filepath, "/sys/class/gpio/gpio%d/value", gpio->id);

    fd = open(filepath, O_RDONLY);

    if (read(fd, &c, 1) < 0) {
        syslog(LOG_ERR, "Failed to read from GPIO device node %s!\n", filepath);
    }

    close(fd);

    gpio->value = (c == '1') ? GPIO_HIGH : GPIO_LOW;
    closelog();
    return gpio->value;
}
