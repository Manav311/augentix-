#include "adc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <syslog.h>

int getAdcValue(const char *adc_file_path, int *adc_value)
{
	openlog("ADC", LOG_PID, LOG_USER);
	int fd = 0;
	char ch[4] = { 0 };

	fd = open(adc_file_path, O_RDONLY, 0x644);
	if (fd < 0) {
		syslog(LOG_ERR, "Cannot open ADC file path %s!\n", adc_file_path);
		return fd;
	}
	read(fd, ch, 4);
	close(fd);

	*adc_value = atoi(ch);
	closelog();
	return 0;
}