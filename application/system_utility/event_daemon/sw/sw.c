#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <syslog.h>

#include "sw.h"

int createServerListen(const char *socket_path)
{
	openlog("SW", LOG_PID, LOG_USER);
	size_t path_len;
	int fd;
	struct sockaddr_un server_un;
	int ret;

	path_len = strlen(socket_path);

	if (path_len == 0) {
		syslog(LOG_ERR, "Socket path can't be empty!\n");
		return SW_FAILURE;
	}

	memset(&server_un, 0, sizeof(server_un));
	server_un.sun_family = AF_UNIX;
	strcpy(server_un.sun_path, socket_path);

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd < 0) {
		syslog(LOG_ERR, "Socket() failure!\n");
		return SW_FAILURE;
	}

	unlink(socket_path);

	ret = bind(fd, (struct sockaddr *)&server_un, sizeof(server_un));

	if (ret < 0) {
		syslog(LOG_ERR, "Bind() failure!\n");
		close(fd);
		return SW_FAILURE;
	}

	ret = listen(fd, 0);

	if (ret < 0) {
		syslog(LOG_ERR, "Listen() failure!\n");
		close(fd);
		return SW_FAILURE;
	}

	closelog();
	return fd;
}

int waitServerAccept(int listen_fd)
{
	openlog("SW", LOG_PID, LOG_USER);
	int child_fd;
	struct sockaddr_un income_un;
	socklen_t addrlen;

	addrlen = sizeof(income_un);

	child_fd = accept(listen_fd, (struct sockaddr *)&income_un, &addrlen);

	if (child_fd < 0) {
		syslog(LOG_ERR, "Accept() failure!\n");
		return SW_FAILURE;
	}

	closelog();
	return child_fd;
}

int startClientConnect(const char *socket_path)
{
	openlog("SW", LOG_PID, LOG_USER);
	size_t path_len;
	int fd;
	struct sockaddr_un client_un;
	int ret;

	path_len = strlen(socket_path);

	if (path_len == 0) {
		syslog(LOG_ERR, "Socket path can't be empty!\n");
		return SW_FAILURE;
	}

	fd = socket(AF_UNIX, SOCK_STREAM, 0);

	if (fd < 0) {
		syslog(LOG_ERR, "Socket() failure!\n");
		return SW_FAILURE;
	}

	memset(&client_un, 0, sizeof(client_un));
	client_un.sun_family = AF_UNIX;
	strcpy(client_un.sun_path, socket_path);

	ret = connect(fd, (struct sockaddr *)&client_un, sizeof(client_un));

	if (ret < 0) {
		syslog(LOG_ERR, "Connect() failure!\n");
		close(fd);
		return SW_FAILURE;
	}

	closelog();
	return fd;
}