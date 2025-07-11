#include "alarmout.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void help()
{
	printf("alarmoutc <gpio#> <polarity> <sec>\n");
	printf("\t<gpio#>   : [0~%d]\n", GPIO_MAX);
	printf("\t<polarity>: [0~%d]\n", POLARITY_MAX);
	printf("\t<sec>     : [0~%d]\n", SEC_MAX);
}

int main(int argc, char **argv)
{
	int sockfd;
	struct sockaddr_un addr;
	struct alarm_out_rule *almo_rule;
	char *buffer;

	if (argc < 4) {
		help();
		return 0;
	}

	buffer = malloc(sizeof(struct alarm_out_rule));
	memset(buffer, 0, sizeof(struct alarm_out_rule));

	almo_rule = (struct alarm_out_rule *)buffer;
	if (atoi(argv[1]) <= GPIO_MAX && atoi(argv[1]) >= 0) {
		almo_rule->gpio = atoi(argv[1]);
	} else {
		help();
		goto failed;
	}
	if (atoi(argv[2]) <= POLARITY_MAX && atoi(argv[2]) >= 0) {
		almo_rule->polarity = atoi(argv[2]);
	} else {
		help();
		goto failed;
	}
	if (atoi(argv[3]) <= SEC_MAX && atoi(argv[3]) >= 0) {
		almo_rule->duration.tv_sec = atoi(argv[3]);
	} else {
		help();
		goto failed;
	}

	memset(&addr, 0, sizeof(addr));

	sockfd = socket(PF_UNIX, SOCK_STREAM, 0);

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, ALARM_OUT_SOCKET);

	connect(sockfd, (struct sockaddr*)&addr, sizeof(addr));
	send(sockfd, buffer, sizeof(struct alarm_out_rule), 0);

	close(sockfd);

failed:
	free(buffer);
	return 0;
}
