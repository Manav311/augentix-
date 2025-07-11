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

#include "ledevt.h"
#include <sys/wait.h>

int forkIndependentProcWatch(char *prog, char **arg_list)
{
	pid_t child;

	if ((child = fork()) < 0) {
		/* parent: check if fork failed */
		syslog(LOG_ERR, "parent: fork error\n");
	} else if (child == 0) {
		/* 1st level child: fork again */
		if ((child = fork()) < 0) {
			syslog(LOG_ERR, "1st level child: fork error\n");
		} else if (child > 0) {
			/* 1st level child: terminate itself to make init process the parent of 2nd level child */
			exit(0);
		} else {
			/* 2nd level child: execute program and will become child of init once 1st level child exits */
			execvp(prog, arg_list);
			syslog(LOG_ERR, "execvp error\n");
			exit(0);
		}
	}

	/* parent: wait for 1st level child ends */
	waitpid(child, NULL, 0);

	return child;
}

int main()
{
	FILE *fp;
	char buffer[128];
	char retry_flag = 0x00;
	char *tuya_service_argv[] = { "tuya_service", "-m 1 -E 1 -G 1", NULL };

	while (1) {
		//Detect tuya status
		fp = popen("ps|grep -v \"grep\"|grep \"tuya_service\"", "r");
		if (fp == NULL) {
			syslog(LOG_INFO, "tuya_service: Failed to run command\n");
			exit(1);
		}

		if (fgets(buffer, sizeof(buffer) - 1, fp) == NULL) {
			if (retry_flag < 0x03) {
				forkIndependentProcWatch("/system/bin/tuya_service", tuya_service_argv);
				retry_flag++;
			} else {
				setLEDInform("Critical_Error", 1);
				sleep(10);
				system("reboot");
			}

		} else {
			if (retry_flag > 0x00) {
				retry_flag = 0x00;
			}
			syslog(LOG_INFO, "tuya_service: %s", buffer);
		}
		pclose(fp);

		sleep(10);
	}
	return 0;
}
