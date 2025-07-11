#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h>
#include <eirq-pir-hc18xx.h>
#include <time.h>
#include <sys/time.h>
#include <utime.h>

int main()
{
	int fd;
	int *stat = calloc(sizeof(int), 4);
	int timeout = 500;
	int sensitivity = 100;

	printf("pir unit test start!\n");

	fd = open("/dev/eint_pir", O_RDWR);
	if (fd < 0) {
		perror("Can't not open /dev/eint_pir.");
		return -1;
	}

	if (ioctl(fd, IOC_PIR_SETTIMEOUT, &timeout) < 0) {
		perror("failed to set timeout.");
	}

	if (ioctl(fd, IOC_PIR_SENSITIVITY, &sensitivity) < 0) {
		perror("failed to set timeout.");
	}

	while (1) {
		if (read(fd, stat, sizeof(stat)) < 0) {
			perror("failed to get pir status.");
		}

		printf("stat = %d\n", *stat);
	}

	close(fd);

	return 0;
}
