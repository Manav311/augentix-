#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include "augentix-earlyvideo.h"

int main(void)
{
	int fd;
	int err = -1;
	EarlyvideoStatus status = EARLYVIDEO_STATUS_NUM;

	fd = open("/dev/earlyvideo", O_RDWR);
	if (fd == -1) {
		return -1;
	}

	printf("before ioctl status = %d\n", status);

	err = ioctl(fd, EARLYVIDEO_IOCTL_GET_STATUS, &status);

	printf("after ioctl status = %d\n", status);

	return err;
}
