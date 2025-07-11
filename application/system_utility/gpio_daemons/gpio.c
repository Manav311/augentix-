#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

const char * const g_gpio_direction[] = {"in", "out"};

#define MAX_FILEPATH 128
#define MAX_BUF 4

int exportGpio(Gpio * gpio);
void unexportGpio(Gpio * gpio);

Gpio * initGpio(int id)
{
	int ret;
	Gpio* gpio = malloc(sizeof(*gpio));
	gpio->id = id;
	gpio->direction = GPIO_IN;
	gpio->value = 0;
	ret = exportGpio(gpio);
	if (ret < 0) {
		free(gpio);
		gpio = NULL;
	}
	return gpio;
}

void releaseGpio(Gpio * gpio)
{
	unexportGpio(gpio);
	free(gpio);
	gpio = NULL;
}

int exportGpio(Gpio * gpio)
{
	char buf[MAX_BUF];
	int fd, ret = 0;
	sprintf(buf, "%d", gpio->id);

	//printf("Export GPIO %s\n", buf);
	fd = open("/sys/class/gpio/export", O_WRONLY);
	if (fd < 0) {
		printf("[Error]: Failed to open GPIO device node!\n");
		return fd;
	}

	if (write(fd, buf, strlen(buf)) < 0) {
		printf("[Error]: Failed to write to GPIO device node!\n");
		ret = -1;
	}

	close(fd);
	return ret;
}

void unexportGpio(Gpio * gpio)
{
	char buf[MAX_BUF];
	int fd;
	sprintf(buf, "%d", gpio->id);

	//printf("Unexport GPIO %s\n", buf);
	fd = open("/sys/class/gpio/unexport", O_WRONLY);
	if (write(fd, buf, strlen(buf)) < 0)
		printf("[Error]: Failed to write to GPIO device node!\n");
	close(fd);
}

void setGpioDirection(Gpio * gpio, char * direction)
{
	char filepath[MAX_FILEPATH];
	int fd;
	sprintf(filepath, "/sys/class/gpio/gpio%d/direction", gpio->id);

	if (!strncmp(direction, g_gpio_direction[GPIO_IN], MAX_BUF)) {
		gpio->direction = GPIO_IN;
	} else if (!strncmp(direction, g_gpio_direction[GPIO_OUT], MAX_BUF)) {
		gpio->direction = GPIO_OUT;
	} else {
		printf("[Error] GPIO %d: Invalid GPIO direction!\n", gpio->id);
		return;
	}

	/*
	 * printf("%s: Set GPIO %d direction to \"%s\"\n",
	 *   filepath, gpio->id, direction);
	 */
	fd = open(filepath, O_WRONLY);
	if (write(fd, direction, strlen(direction))< 0)
		printf("[Error]: Failed to write to GPIO device node!\n");
	close(fd);
}

Direction getGpioDirection(Gpio * gpio)
{
	/*return gpio->direction;*/
	char filepath[MAX_FILEPATH];
	char direction[MAX_BUF];
	int fd;
	sprintf(filepath, "/sys/class/gpio/gpio%d/direction", gpio->id);

	fd = open(filepath, O_RDONLY);
	if (read(fd, direction, MAX_BUF) < 0)
		printf("[Error]: Failed to read from GPIO device node %s!\n", filepath);
	close(fd);

	if (!strncmp(direction, g_gpio_direction[GPIO_IN], MAX_BUF)) {
		gpio->direction = GPIO_IN;
	} else if (!strncmp(direction, g_gpio_direction[GPIO_OUT], MAX_BUF)) {
		gpio->direction = GPIO_OUT;
	} else {
		printf("[Error] GPIO %d: Invalid GPIO direction!\n", gpio->id);
	}

	return gpio->direction;

}

void setGpioValue(Gpio * gpio, Value v)
{
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
	if (write(fd, &c, 1) < 0)
		printf("[Error]: Failed to write to GPIO device node!\n");

	close(fd);
}

Value getGpioValue(Gpio * gpio)
{
	/* return gpio->value; */
	char filepath[MAX_FILEPATH];
	char c;
	int fd;
	sprintf(filepath, "/sys/class/gpio/gpio%d/value", gpio->id);

	fd = open(filepath, O_RDONLY);
	if (read(fd, &c, 1) < 0)
		printf("[Error]: Failed to read from GPIO device node %s!\n", filepath);
	close(fd);

	gpio->value = (c == '1') ? GPIO_HIGH : GPIO_LOW;
	return gpio->value;
}
