#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <syslog.h>

#include "auth.h"
#include "datetime.h"
#include "decrypt.h"
#include "netinfo.h"
#include "sysinfo.h"
#include "agtx_types.h"

#define MAX_CLIENTS_NUM 64
#define BUFLEN 16384 * 2 // 2048
#define PORT "7803" // double quotes for getaddrinfo()

#define DELIMITER '@'

char g_connect_port[5] = PORT;

socklen_t addrlen;

static void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in *)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

static int get_client_idx(int clientfd, int clients[], int num_clients)
{
	for (int i = 0; i < num_clients; ++i) {
		if (clients[i] == clientfd)
			return i;
	}
	return -1;
}

static int handle_action(int clientfd, unsigned char *data, int data_len)
{
	int err;
	char resp_ok[] = { "ok" };
	char resp_failed[] = { "failed" };
	unsigned char *dec_data = (unsigned char *)malloc(data_len);
	int n = decrypt(data, data_len, dec_data);
	if (n < 0) {
		syslog(LOG_ERR, "Cannot decrypt data.");
		return send(clientfd, resp_failed, strlen(resp_failed), 0);
	}

	// Get sys info
	if (strcmp((const char *)dec_data, "get_sys_info") == 0) {
		char *sys_info = get_sys_info();
		return send(clientfd, sys_info, strlen(sys_info) + 1, 0);
	}
	// Get net settings
	if (strcmp((const char *)dec_data, "get_net_settings") == 0) {
		char *net_settings = get_net_settings();
		return send(clientfd, net_settings, strlen(net_settings) + 1, 0);
	}

	const char *comma = ",";
	char *action = strtok((char *)dec_data, comma);
	if (action == NULL) {
		syslog(LOG_ERR, "Invalid data format.");
		return send(clientfd, resp_failed, strlen(resp_failed), 0);
	} else if (strcmp(action, "set_auth") == 0) {
		// Set WebUI auth
		char *uname = strtok(NULL, comma);
		char *passwd = strtok(NULL, "\0");
		err = set_auth(uname, passwd);
		return send(clientfd, (err == 0) ? resp_ok : resp_failed, strlen((err == 0) ? resp_ok : resp_failed), 0);
	} else if (strcmp(action, "set_time") == 0) {
		// Set DateTime
		char *datetime = strtok(NULL, "\0");
		err = set_date_time(datetime);
		return send(clientfd, (err == 0) ? resp_ok : resp_failed, strlen((err == 0) ? resp_ok : resp_failed), 0);
	} else if (strcmp(action, "set_net_settings") == 0) {
		// Set net settings
		int dhcp_status = atoi(strtok(NULL, comma));
		char *ip = strtok(NULL, comma);
		char *netmask = strtok(NULL, comma);
		char *gateway = strtok(NULL, comma);
		char *dns1 = strtok(NULL, comma);
		char *dns2 = strtok(NULL, "\0");
		err = set_net_setting(dhcp_status, ip, netmask, gateway, dns1, dns2);
		int n = send(clientfd, (err == 0) ? resp_ok : resp_failed, strlen((err == 0) ? resp_ok : resp_failed), 0);
		char cmd[36] = { 0 };
		char *mode = get_mode();
		sprintf(cmd, "/etc/init.d/%s/S40eth restart", mode);
		err = system(cmd);
		if (err != 0) {
			syslog(LOG_ERR, "Network restart failed.");
		}
		return n;
	}
	return send(clientfd, resp_failed, strlen(resp_failed), 0);
}

static void start_service()
{
	fd_set master; // master file descriptor list
	fd_set read_fds; // temp file descriptor list for select()
	int fdmax; // maximum file descriptor number

	int listener; // listening socket descriptor
	int newfd; // newly accepted socket descriptor
	struct sockaddr_storage remoteaddr; // client address
	unsigned char buf[BUFLEN]; // buffer for client data
	int nbytes; // number of bytes involved during I/O
	char remoteIP[INET6_ADDRSTRLEN];

	int yes = 1; // for setsockopt() SO_REUSEADDR
	int rv, client_idx, num_clients = 0;
	struct addrinfo hints, *listener_address;
	int client_fd_list[MAX_CLIENTS_NUM] = { 0 }; // list of clients' fd

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	// get a socket and bind it
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, g_connect_port, &hints, &listener_address)) != 0) {
		fprintf(stderr, "stamen_server: %s\n", gai_strerror(rv));
		exit(1);
	}

	struct addrinfo *p;
	for (p = listener_address; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0) {
			continue;
		}
		// get rid of "address already in use" error message
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			close(listener);
			syslog(LOG_INFO, "setsock close!\n");
			continue;
		} else {
			syslog(LOG_INFO, "setsock listening!\n");
		}
		break;
	}

	if (p == NULL) {
		syslog(LOG_INFO, "WTF happened!\n");
		fprintf(stderr, "stamen_server: failed to bind\n");
		exit(2);
	}
	freeaddrinfo(listener_address);

	if (listen(listener, MAX_CLIENTS_NUM) == -1) {
		syslog(LOG_ERR, "listen");
		exit(3);
	}
	syslog(LOG_INFO, "stamen waiting for connections...\n");
	FD_SET(listener, &master);
	fdmax = listener;

	syslog(LOG_INFO, "Waiting for client connection...\n");
	while (true) {
		read_fds = master;
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			syslog(LOG_ERR, "select");
			exit(4);
		}
		syslog(LOG_INFO, "#--- Selected, max fd: %d ---#\n", fdmax);

		// run through existing connections looking for data to read
		for (int i = 0; i <= fdmax; i++) {
			bool ret_isset = FD_ISSET(i, &read_fds);
			if (!ret_isset) {
				continue;
			}
			if (i == listener) { // handle new connection
				addrlen = sizeof(remoteaddr);
				newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
				if (newfd == -1) {
					syslog(LOG_ERR, "accept");
				} else {
					FD_SET(newfd, &master);
					if (newfd > fdmax) {
						fdmax = newfd;
					}
					syslog(LOG_INFO, "New connection from socket %s %d, fdmax = %d\n",
					       inet_ntop(remoteaddr.ss_family,
					                 get_in_addr((struct sockaddr *)&remoteaddr), remoteIP,
					                 INET6_ADDRSTRLEN),
					       newfd, fdmax);

					int real_newfd_idx = newfd - 1;
					if (real_newfd_idx >= MAX_CLIENTS_NUM) {
						syslog(LOG_ERR, "newfd: %2d (> %d)\n", newfd, MAX_CLIENTS_NUM);
						exit(5);
					}
					client_fd_list[real_newfd_idx] = newfd;

					if (num_clients >= (MAX_CLIENTS_NUM - 4)) {
						syslog(LOG_ERR, "Number of clients: %2d (> %d)\n", num_clients, MAX_CLIENTS_NUM - 4);
					}
					num_clients++;
					syslog(LOG_INFO, "Number of clients: %2d\n", num_clients);
				}
			} else { // handle data from existing client
				memset(&buf, '\0', sizeof(buf));
				int offs = 0;
				int n = recv(i, buf, sizeof(buf), 0);
				if (n > 0) {
					if (buf[n - 1] != DELIMITER) {
						offs += n;
						// until recv DELIMITER
						do {
							n = recv(i, buf, sizeof(buf), 0);
							if (n <= 0) {
								nbytes = offs;
								break;
							}
							offs += n;
							if (buf[offs - 1] == DELIMITER) {
								nbytes = offs;
								break;
							}
						} while (1);
					} else {
						nbytes = n;
					}
				} else {
					nbytes = 0;
				}
				syslog(LOG_INFO, "\t Recive %d bytes of data\n", nbytes);
				syslog(LOG_INFO, "\t from client %2d\n", i);
				if (nbytes <= 0) {
					// got error or connection closed by client
					if (nbytes == 0) {
						syslog(LOG_INFO, "Client %d hung up\n", i);
					} else {
						syslog(LOG_ERR, "recv");
					}
					close(i);
					FD_CLR(i, &master);
					num_clients--;
				} else { // we got some data from a client
					for (int j = 0; j <= fdmax; j++) {
						if (!FD_ISSET(j, &master) || i != j) {
							continue;
						}
						syslog(LOG_INFO, "Handle action for client %2d, fdmax = %d ...\n", j, fdmax);
						client_idx = get_client_idx(j, client_fd_list, fdmax);
						if (client_idx < 0) {
							syslog(LOG_ERR, "Client not found: client_idx = %2d\n", client_idx);
							exit(6);
						}

						// Remove DELIMITER on last
						buf[nbytes - 1] = 0;
						nbytes--;

						handle_action(client_fd_list[client_idx], buf, nbytes);
					}
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	// TODO Usage
	// const char *basename = argv[0];
	// if (argc < 2) {
	// 	help(basename);
	// 	return EXIT_FAILURE;
	// }
	AGTX_UNUSED(argc);
	AGTX_UNUSED(argv);

	openlog("stamen", LOG_PID, LOG_DAEMON);
	syslog(LOG_NOTICE, "stamen daemon ready to serve! Port: %s\n", g_connect_port);
	start_service();
	closelog();

	return EXIT_SUCCESS;
}
