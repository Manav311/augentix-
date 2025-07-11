#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#include "led.h"
#include "ledevt.h"

int setLEDInform(char *LED_Client, int Enabled)
{
	char *server_path = LED_SERVER_SOCKET_PATH;
	struct sockaddr_un serun;
	int len;
	char buf[100];
	int sockfd;

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		perror("client socket error");
		return -1;
	}

	memset(&serun, 0, sizeof(serun));
	serun.sun_family = AF_UNIX;
	strcpy(serun.sun_path, server_path);
	len = offsetof(struct sockaddr_un, sun_path) + strlen(serun.sun_path);
	if (connect(sockfd, (struct sockaddr *)&serun, len) < 0) {
		perror("connect error");
		close(sockfd);
		return -1;
	}

	Enabled = (Enabled != 0) ? 1 : 0;
	sprintf(buf, "%s %d", LED_Client, Enabled);
	write(sockfd, buf, strlen(buf));
	close(sockfd);

	return 0;
}