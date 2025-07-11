#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <fcgi_stdio.h>

#define JSON_BUF_STR_SIZE 16384
void error(const char *);

char buf[JSON_BUF_STR_SIZE];

char cmd1[] = "{\"master_id\":0, \"cmd_id\":1048577, \"cmd_type\":\"ctrl\", "
              "\"name\":\"CGI\"}";
char cmd2[] = "{\"master_id\":0, \"cmd_id\":1048578, \"cmd_type\":\"ctrl\"}";

int main(void)
{
	int sockfd, servlen, n, post_len = 0;
	struct sockaddr_un serv_addr;
	char buffer[JSON_BUF_STR_SIZE];
	char *str_post_len = NULL;

	memset((char *)&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sun_family = AF_UNIX;
	strcpy(serv_addr.sun_path, "/tmp/ccUnxSkt");
	servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

	if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
		error("Creating socket");
	}

	if (connect(sockfd, (struct sockaddr *)&serv_addr, servlen) < 0) {
		error("Connecting");
	}

	write(sockfd, cmd1, strlen(cmd1));
	n = read(sockfd, buffer, JSON_BUF_STR_SIZE - 2);
	write(sockfd, cmd2, strlen(cmd2));
	n = read(sockfd, buffer, JSON_BUF_STR_SIZE - 2);

	while (FCGI_Accept() >= 0) {
		str_post_len = getenv("CONTENT_LENGTH");
		if ((str_post_len == NULL) || (sscanf(str_post_len, "%d", &post_len) != 1) || (post_len <= 0) ||
		    (post_len >= JSON_BUF_STR_SIZE)) {
			printf("Status: 400\r\n"
			       "Content-type: text/html\r\n\r\n"
			       "Error: No data exist!\n");
			continue;
		}

		int ch = 0;
		for (int i = 0; i < post_len; i++) {
			if ((ch = getchar()) < 0) {
				break;
			}
			buf[i] = (char)ch;
		}
		if (ch < 0) {
			printf("Status: 500\r\n"
			       "Content-type: text/html\r\n\r\n"
			       "Error: Not enough bytes received!\n");
			continue;
		}
		buf[post_len] = '\0';

		write(sockfd, buf, strlen(buf));
		n = read(sockfd, buffer, JSON_BUF_STR_SIZE - 2);
		buffer[n] = '\0';
		printf("Content-type: application/json\r\n\r\n");
		printf(buffer);
	}

	return 0;
}
