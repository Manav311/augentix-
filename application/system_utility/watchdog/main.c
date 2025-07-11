#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <linux/watchdog.h>
#include <syslog.h>
#include <signal.h>

#define MEM_PATH "/proc/meminfo"

int main(){
	int fd;
	int MemAvailable = 0;
	char buffer[128];

	fd = open("/dev/watchdog", O_RDONLY);
	if (fd < 0){
		perror("Can't not open /dev/watchdog.");
		return -1;
	}

	while(1){
		ioctl(fd, WDIOC_KEEPALIVE, 0);

		//Detect memory available
		FILE *fp_MEM;
		fp_MEM = fopen(MEM_PATH, "r");
		if(fp_MEM) {
			while(fgets(buffer, 128, fp_MEM)!=NULL) {
				if( sscanf(buffer, "MemAvailable:%skB", buffer)!= 0 ) {
					MemAvailable = atoi(buffer);
				}
			}
		}
		if (MemAvailable < 2048) {
			syslog(LOG_ERR, "Memory available is too low!");
			break;
		}
		fclose(fp_MEM);

		sleep(20);
		//syslog(LOG_INFO, "Feed watchdog.");
	}

	close(fd);

	return 0;
}
