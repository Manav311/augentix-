#include "gpio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <linux/limits.h>
#include <errno.h>

#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>
#include "utlist.h"

const char *const g_gpio_direction[] = { "in", "out", "high", "low" };

#define MAX_BUF 16

static inline void sstrip(char *str);
static int exportGpio(int id);
static int unexportGpio(int id);

static void sstrip(char *str)
{
	char *start;
	char *end;
	int i = 0;
	if (str == NULL) {
		return;
	}

	start = str;
	end = str + strlen(str) - 1;

	// strip front
	while (start <= end) {
		if (isgraph(*start)) {
			break;
		}
		start++;
	}

	// strip back
	while (start <= end) {
		if (isgraph(*end)) {
			break;
		}
		end--;
	}

	// generate new string
	while (start <= end) {
		str[i] = *start;
		start++;
		i++;
	}
	str[i] = '\0';
}

/**
 * @brief Export the GPIO control driver to user space.
 *
 * @param[in] id ID of the GPIO
 * @return Either the operation is successful.
 * @retval 0 Succeeded.
 * @retval negative number Failed.
 */
static int exportGpio(int id)
{
	char gpio_id[MAX_BUF];
	char gpio_sys_file[PATH_MAX];
	int fd;
	snprintf(gpio_id, MAX_BUF, "%d", id);
	snprintf(gpio_sys_file, PATH_MAX, "/sys/class/gpio/gpio%d", id);

	if (access(gpio_sys_file, X_OK) != 0) {
		fd = open("/sys/class/gpio/export", O_WRONLY);

		if (fd < 0) {
			fprintf(stderr, "Failed to open GPIO export node.\n");
			return -EINVAL;
		}

		if (write(fd, gpio_id, strlen(gpio_id)) < 0) {
			fprintf(stderr, "Failed to write to GPIO export node.\n");
			close(fd);
			return -EINVAL;
		}

		close(fd);

		if (access(gpio_sys_file, X_OK) != 0) {
			fprintf(stderr, "Unable to export GPIO %d.\n", id);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Unexport the GPIO control driver from user space.
 * @param[in] id ID of the GPIO
 * @return Either the operation is successful.
 * @retval 0 Succeeded.
 * @retval negative number Failed.
 */
static int unexportGpio(int id)
{
	char gpio_id[MAX_BUF];
	char gpio_sys_file[PATH_MAX];
	int fd;

	/* generate file strings */
	snprintf(gpio_id, MAX_BUF, "%d", id);
	snprintf(gpio_sys_file, PATH_MAX, "/sys/class/gpio/gpio%d", id);

	if (access(gpio_sys_file, X_OK) == 0) {
		fd = open("/sys/class/gpio/unexport", O_WRONLY);

		if (fd < 0) {
			fprintf(stderr, "Failed to open GPIO unexport node.\n");
			return -EINVAL;
		}

		if (write(fd, gpio_id, strlen(gpio_id)) < 0) {
			fprintf(stderr, "Failed to write to GPIO unexport node.\n");
			close(fd);
			return -EINVAL;
		}

		close(fd);

		if (access(gpio_sys_file, X_OK) == 0) {
			fprintf(stderr, "Unable to unexport GPIO %d.\n", id);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * @brief Initialize the gpio with a given id.
 * @param[in] gpio Ptr of the GPIO, please see the wiki page about GPIO for the
 * mapping table.
 * @return 0 for successfily export & get GPIO direction & value, -EINVAL for
 * failed.
 */
int GPIO_initGpio(Gpio *gpio)
{
	int ret;

	if (gpio->id < 0) {
		fprintf(stderr, "Invalid GPIO ID %d.\n", gpio->id);
		return -EINVAL;
	}

	ret = exportGpio(gpio->id);
	if (ret != 0) {
		return -EINVAL;
	}

	GPIO_getGpioDirection(gpio);
	GPIO_getGpioValue(gpio);
	return 0;
}

/**
 * @brief Release  the gpio.
 * @param[in] gpio The Gpio instance to unexport.
 */
void GPIO_releaseGpio(Gpio *gpio)
{
	if (gpio == NULL) {
		fprintf(stderr, "Invalid argument.\n");
		return;
	}
	if (gpio->id < 0) {
		fprintf(stderr, "Invalid GPIO ID %d.\n", gpio->id);
		return;
	}

	unexportGpio(gpio->id);
}

/**
 * @brief Set the direction of the GPIO.
 * @param[in] gpio The information of the GPIO, including the preferred
 * direction.
 * @return Either the operation is succeeded.
 * @retval 0 Succeeded.
 * @retval negative number Failed.
 */
int GPIO_setGpioDirection(Gpio *gpio)
{
	char filepath[PATH_MAX];
	const char *dir_text;
	int fd;

	if (gpio == NULL) {
		fprintf(stderr, "Invalid argument.\n");
		return -EINVAL;
	}
	if (gpio->id < 0) {
		fprintf(stderr, "Invalid GPIO ID %d.\n", gpio->id);
		return -EINVAL;
	}

	snprintf(filepath, PATH_MAX, "/sys/class/gpio/gpio%d/direction", gpio->id);

	if (gpio->direction >= GPIO_DIR_NUM) {
		fprintf(stderr, "Invalid GPIO direction %d.\n", gpio->direction);
		return -EINVAL;
	}

	fd = open(filepath, O_WRONLY);

	if (fd < 0) {
		fprintf(stderr, "Failed to open GPIO%d direction node '%s'.\n", gpio->id, filepath);
		return -EINVAL;
	}

	dir_text = g_gpio_direction[gpio->direction];

	if (write(fd, dir_text, strlen(dir_text)) < 0) {
		fprintf(stderr, "Failed to write to GPIO%d direction node.\n", gpio->id);
		close(fd);
		return -EINVAL;
	}

	close(fd);

	if (gpio->direction == GPIO_OUT_HIGH) {
		gpio->direction = GPIO_OUT;
		gpio->value = 1;
	} else if (gpio->direction == GPIO_OUT_LOW) {
		gpio->direction = GPIO_OUT;
		gpio->value = 0;
	}

	return 0;
}

/**
 * @brief Get the direction of the GPIO.
 * @param[in] gpio The information of the GPIO. The direction information will
 * also be stored here.
 * @return Either the operation is succeeded.
 * @retval 0 Succeeded.
 * @retval negative number Failed.
 */
int GPIO_getGpioDirection(Gpio *gpio)
{
	/*return gpio->direction;*/
	char filepath[PATH_MAX];
	char direction[MAX_BUF] = {};
	int fd;
	int ret;

	if (gpio == NULL) {
		fprintf(stderr, "Invalid argument.\n");
		return -EINVAL;
	}
	if (gpio->id < 0) {
		fprintf(stderr, "Invalid GPIO ID %d.\n", gpio->id);
		return -EINVAL;
	}

	snprintf(filepath, PATH_MAX, "/sys/class/gpio/gpio%d/direction", gpio->id);
	fd = open(filepath, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Failed to open GPIO%d direction node '%s'.", gpio->id, filepath);
		return -EINVAL;
	}

	ret = read(fd, direction, MAX_BUF);

	if (ret < 0) {
		fprintf(stderr, "Failed to read from GPIO%d direction node '%s'.", gpio->id, filepath);
		close(fd);
		return -EINVAL;
	}

	close(fd);
	sstrip(direction);

	if (strncmp(direction, g_gpio_direction[GPIO_IN], MAX_BUF) == 0) {
		gpio->direction = GPIO_IN;
	} else if (strncmp(direction, g_gpio_direction[GPIO_OUT], MAX_BUF) == 0) {
		gpio->direction = GPIO_OUT;
	} else {
		fprintf(stderr, "Invalid GPIO%d direction '%s'.", gpio->id, direction);
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief Set the value of the GPIO.
 * @note This function does not check the current GPIO state, but should only be
 * called when GPIO is set to output.<br> Otherwise it will always fail.
 * @param[in] gpio The information of the GPIO, including the preferred GPIO
 * value.
 * @return Either the operation is succeeded.
 * @retval 0 Succeeded.
 * @retval negative number Failed.
 */
int GPIO_setGpioValue(Gpio *gpio)
{
	char filepath[PATH_MAX];
	char c = 0;
	int fd;

	if (gpio == NULL) {
		fprintf(stderr, "Invalid argument.\n");
		return -EINVAL;
	}

	if (gpio->id < 0) {
		fprintf(stderr, "Invalid GPIO ID %d.\n", gpio->id);
		return -EINVAL;
	}

	sprintf(filepath, "/sys/class/gpio/gpio%d/value", gpio->id);
	switch (gpio->value) {
	case GPIO_VAL_LOW:
		c = '0';
		break;
	case GPIO_VAL_HIGH:
		c = '1';
		break;
	default:
		fprintf(stderr, "Invalid GPIO value %d.\n", gpio->value);
		return -EINVAL;
	}

	fd = open(filepath, O_WRONLY);

	if (fd < 0) {
		fprintf(stderr, "Failed to open GPIO%d value node '%s'.\n", gpio->id, filepath);
		return -EINVAL;
	}

	if (write(fd, &c, 1) < 0) {
		fprintf(stderr, "Failed to write to GPIO%d value node.\n", gpio->id);
		close(fd);
		return -EINVAL;
	}

	close(fd);

	return 0;
}

/**
 * @brief Get the value of the GPIO.
 * @param[in] gpio The information of the GPIO, including the preferred GPIO
 * value.
 * @return Either the operation is succeeded.
 * @retval 0 Succeeded.
 * @retval negative number Failed.
 */
int GPIO_getGpioValue(Gpio *gpio)
{
	char filepath[PATH_MAX];
	char c;
	int fd;

	if (gpio == NULL) {
		fprintf(stderr, "Invalid argument.\n");
		return -EINVAL;
	}

	if (gpio->id < 0) {
		fprintf(stderr, "Invalid GPIO ID %d.\n", gpio->id);
		return -EINVAL;
	}

	sprintf(filepath, "/sys/class/gpio/gpio%d/value", gpio->id);
	fd = open(filepath, O_RDONLY);

	if (fd < 0) {
		fprintf(stderr, "Failed to open GPIO%d value node '%s'.\n", gpio->id, filepath);
		return -EINVAL;
	}

	if (read(fd, &c, 1) < 0) {
		fprintf(stderr, "Failed to read from GPIO%d value node %s.\n", gpio->id, filepath);
		close(fd);
		return -EINVAL;
	}

	close(fd);
	gpio->value = (c == '1') ? GPIO_VAL_HIGH : GPIO_VAL_LOW;

	return 0;
}
