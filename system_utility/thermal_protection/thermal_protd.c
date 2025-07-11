#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

#include <sys/file.h>

#define AR0331 1
#define OV4689 2

#include "autoconf.h"
#include "agtx_types.h"

#define APP_NAME "thermal_protd"
#define LOCK_NAME "/tmp/thermal_protd.lock"

/* Error checking */
#ifdef CONFIG_THERMAL_PROTECTION_SUPPORT
#if defined(CONFIG_THERMAL_PROTECTION_INFO_FROM_SENSOR) \
    && defined(CONFIG_THERMAL_PROTECTION_INFO_FROM_ADC)
#error "Obtaining thermal info from both interface is not supported!"
#else
/* FROM SENSOR */
#ifdef CONFIG_THERMAL_PROTECTION_INFO_FROM_SENSOR
/* Check I2C CHIP */
#ifndef CONFIG_THERMAL_PROTECTION_I2C_CHIP
#error "I2C_CHIP need to be set"
#endif
/* Check I2C DEV */
#ifndef CONFIG_THERMAL_PROTECTION_I2C_DEV
#error "I2C_DEV need to be set"
#endif
/* FROM ADC */
#elif defined(CONFIG_THERMAL_PROTECTION_INFO_FROM_ADC)
/* Check ADC CHANNEL */
#if !defined(CONFIG_THERMAL_PROTECTION_ADC_CHANNEL) \
    || (CONFIG_THERMAL_PROTECTION_ADC_CHANNEL < 0) \
    || (CONFIG_THERMAL_PROTECTION_ADC_CHANNEL > 2)
#error "ADC_CHANNEL should be in [0,1,2]"
#endif
/* FORMULA INIT */
#ifndef CONFIG_THERMAL_PROTECTION_ADC_FORMULA_MULTIPLE
#define CONFIG_THERMAL_PROTECTION_ADC_FORMULA_MULTIPLE 1
#endif
#ifndef CONFIG_THERMAL_PROTECTION_ADC_FORMULA_DIVIDE
#define CONFIG_THERMAL_PROTECTION_ADC_FORMULA_DIVIDE 1
#else
#if CONFIG_THERMAL_PROTECTION_ADC_FORMULA_DIVIDE == 0
#error "FORMULA_DEVIDE cannot be 0"
#endif
#endif
#ifndef CONFIG_THERMAL_PROTECTION_ADC_FORMULA_OFFSET
#define CONFIG_THERMAL_PROTECTION_ADC_FORMULA_OFFSET 0
#endif
/* None of I2C/ADC is set */
#else
#error "One of INFO_FROM_SENSOR or INFO_FROM_ADC need to be set"
#endif
#endif
/* No setting of Threshold Low/High */
#if !defined(CONFIG_THERMAL_PROTECTION_THRESHOLD_MEDIUM) \
	|| !defined(CONFIG_THERMAL_PROTECTION_THRESHOLD_HIGH)
#error "THRESHOLD_MEDIUM and THRESHOLD_HIGH need to be defined"
#else
#define THERMAL_THRESHOLD_MEDIUM CONFIG_THERMAL_PROTECTION_THRESHOLD_MEDIUM
#define THERMAL_THRESHOLD_HIGH CONFIG_THERMAL_PROTECTION_THRESHOLD_HIGH
#endif
/* Set default iir coefficient if not set */
#ifndef CONFIG_THERMAL_PROTECTION_IIR_COEFFICIENT
#define CONFIG_THERMAL_PROTECTION_IIR_COEFFICIENT 20
#elif CONFIG_THERMAL_PROTECTION_IIR_COEFFICIENT > 100
#error "IIR_COEFFICEINT must be [0-100]"
#endif
#endif /* CONFIG_THERMAL_PROTECTION_SUPPORT */

#define ADC_TO_TEMPERATURE(x) (x \
	    * CONFIG_THERMAL_PROTECTION_ADC_FORMULA_MULTIPLE \
	    / CONFIG_THERMAL_PROTECTION_ADC_FORMULA_DIVIDE \
	    + CONFIG_THERMAL_PROTECTION_ADC_FORMULA_OFFSET)

#define _ADC_FILE(x) "/sys/bus/iio/devices/iio:device0/in_voltage" #x "_raw"
#define ADC_FILE(x) _ADC_FILE(x)

#define _I2C_DEV(x) "/dev/i2c-" #x
#define I2C_DEV(x) _I2C_DEV(x)

#define _TO_STR(x) #x
#define TO_STR(x) _TO_STR(x)

#ifdef CONFIG_THERMAL_PROTECTION_SUPPORT
#ifdef CONFIG_THERMAL_PROTECTION_INFO_FROM_SENSOR
#include <linux/i2c-dev.h>
#include "sensor_params.h"
#if CONFIG_THERMAL_PROTECTION_I2C_CHIP == AR0331
#include "thermal_protd_ar0331.h"
#elif CONFIG_THERMAL_PROTECTION_I2C_CHIP == OV4689
#include "thermal_protd_ov4689.h"
#else
#error "I2C_CHIP unsupported"
#endif

#define I2C_BUF_SIZE 4

static int readI2c(int *temperature)
{
	int i2c_fd;
	int ret = 0;

	i2c_fd = open(I2C_DEV(CONFIG_THERMAL_PROTECTION_I2C_DEV), O_RDWR);
	if (i2c_fd < 0) {
		perror("open i2c");
		return i2c_fd;
	}

	ret = _readI2c(i2c_fd, temperature);

	close(i2c_fd);

	return ret;
}
#endif

#ifdef CONFIG_THERMAL_PROTECTION_INFO_FROM_ADC
static int readAdc(int *temperature)
{
	int fd = 0;
	char ch[4] = {0};
	const char *adc_file = ADC_FILE(CONFIG_THERMAL_PROTECTION_ADC_CHANNEL);

	fd = open(adc_file, O_RDONLY, 0x644);
	if (0 > fd) {
		printf("Cannot open file %s [%d]\n", adc_file, fd);
		return fd;
	}
	read(fd, ch, 4);
	close(fd);

	*temperature = atoi(ch);
	return 0;
}
#endif

/* Macro definition *
 * ---------------- *
 * getThermalInfo() *
 * convertToTemp()  */
#ifdef CONFIG_THERMAL_PROTECTION_INFO_FROM_SENSOR
#define getThermalInfo(t) readI2c(t)
#define convertToTemp(d,s) { d = s; }
#elif defined(CONFIG_THERMAL_PROTECTION_INFO_FROM_ADC)
#define getThermalInfo(t) readAdc(t)
#define convertToTemp(d,s) { d = ADC_TO_TEMPERATURE(s); }
#endif

static int checkThermal(int temperature)
{
	if (temperature > THERMAL_THRESHOLD_HIGH) {
		system("reboot");
		pause();
	} else if (temperature > THERMAL_THRESHOLD_MEDIUM) {
#define WARN() printf("[Warning]: temperature is high (%d°).\n", temperature)
		WARN();
	} else {
		/* temperature normal, do nothing */
	}
	return 0;
}
#endif

static int checkMultipleInstances(void)
{
	int lock_fd;
	char *lock_name = LOCK_NAME;

	/* Check multiple instances */
	lock_fd = open(lock_name, O_CREAT | O_RDWR, 0666);
	if (-1 == lock_fd) {
		perror("Cannot open lock file");
		return -EINVAL;
	}

	if (-1 == flock(lock_fd, LOCK_EX | LOCK_NB)) {
		printf("%s: Failed to lock %s. Multiple instance detected.\n",
				APP_NAME, lock_name);
		close(lock_fd);
		return -EBUSY;
	}

	return 0;
}

static void handleSigInt(int signo)
{
	AGTX_UNUSED(signo);

	exit(0);
}

int main()
{
	int ret = 0;

	signal(SIGINT, handleSigInt);

	/* check multiple instances of the program */
	ret = checkMultipleInstances();
	if (ret != 0) {
		return ret;
	}

#ifdef CONFIG_THERMAL_PROTECTION_SUPPORT
	int curr_temp = 0;
	int iir_temp = 0;
	while (1) {
		ret = getThermalInfo(&curr_temp);
		if (ret < 0) {
			goto wait_next;
		}
		convertToTemp(curr_temp, curr_temp);
		iir_temp = ((iir_temp * (100 - CONFIG_THERMAL_PROTECTION_IIR_COEFFICIENT))
			+ (curr_temp * CONFIG_THERMAL_PROTECTION_IIR_COEFFICIENT)) / 100;
		checkThermal(iir_temp);
wait_next:
		usleep(CONFIG_THERMAL_PROTECTION_SAMPLE_RATE_MS * 1000);
	}
#else
	printf("Thermal protection is not supported\n");
	pause();
#endif

	return 0;
}
