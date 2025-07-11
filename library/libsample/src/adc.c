#include "adc.h"

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

#define ADC_PATH "/sys/bus/iio/devices/iio:device0/"

/**
 * @brief Initialize the adc with a given id.
 * @param[in] adc Ptr of the GPIO, please see the wiki page about adc for the
 * mapping table.
 * @return 0 for successfily export & get adc direction & value, -EINVAL for
 * failed.
 */
int ADC_initAdc(Adc *adc)
{
	int ret;

	if (adc->id < 0) {
		fprintf(stderr, "Invalid adc ID %d.\n", adc->id);
		return -EINVAL;
	}

	ret = ADC_getAdcValue(adc);

	if (ret < 0) {
		fprintf(stderr, "adc%d is invalid, initilaize failed.\n", adc->id);
		return -EINVAL;
	}
	return 0;
}

int ADC_getAdcValue(Adc *adc)
{
	char filepath[PATH_MAX];
	int fd;
	char buff[4];

	if (adc == NULL) {
		fprintf(stderr, "[Adc_getAdcValue]Invalid adc argument\n");
		return -EINVAL;
	}

	if (adc->id < 0) {
		fprintf(stderr, "[Adc_getAdcValue]Invalid adc id %d\n", adc->id);
		return -EINVAL;
	}
	sprintf(filepath, "%sin_voltage%d_raw", ADC_PATH, adc->id);

	fd = open(filepath, O_RDONLY);
	if (fd < 0) {
		fprintf(stderr, "[Adc_getAdcValue]Failed to open adc%d path %s\n", adc->id, filepath);
	}

	if (read(fd, &buff, sizeof(buff)) < 0) {
		fprintf(stderr, "[Adc_getAdcValue]Failed to read from adc%d value node %s.\n", adc->id, filepath);
		close(fd);
		return -EINVAL;
	}
	close(fd);

	adc->value = (unsigned short int)atoi(buff);
	adc->adc_hl = (adc->value > adc->threshold_hl) ? ADC_VAL_HIGH : ADC_VAL_LOW;
	return 0;
}

void Adc_releaseAdc(Adc *adc)
{
	memset(adc, 0, sizeof(Adc));
	fprintf(stdout, "release adc\n");
}
