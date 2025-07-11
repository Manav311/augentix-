#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/time.h>
#include <fcntl.h>
#include <linux/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <pthread.h>

#include "tuya_test_frame.h"
#include "tuya_cmd.h"
#include "tuya_mptt_common.h"
#include "ini_parser.h"

char g_client_addr[32];

static int generate_wpa_supp_cfg(const char *file, const MpttCfg *cfg)
{
	FILE *fp = NULL;
	printf("Generating %s ...\n", file);
	fp = fopen(file, "w");
	if (fp == NULL) {
		fprintf(stderr, "Failed to generate %s\n", file);
		return -1;
	}

	fprintf(fp, "ctrl_interface=/var/run/wpa_supplicant\nupdate_config=1\n");
	fprintf(fp, "network={\n");
	fprintf(fp, "\tssid=\"%s\"\n", cfg->ssid);
	fprintf(fp, "\tkey_mgmt=WPA-PSK\n");
	fprintf(fp, "\tpairwise=CCMP TKIP\n");
	fprintf(fp, "\tgroup=CCMP TKIP\n");
	fprintf(fp, "\tproto=WPA RSN\n");
	fprintf(fp, "\tpsk=\"%s\"\n", cfg->passwd);
	fprintf(fp, "\tpriority=1\n}\n");

	fclose(fp);
	return 0;
}

static int init_network_if(const MpttCfg *cfg)
{
	char cmd_buf[256];
	if (strcmp((char *)cfg->interface, "wlan0") == 0) {
		snprintf(cmd_buf, sizeof(cmd_buf), "/system/script/wifi_on.sh %s", WPA_SUPP_FILE);
		printf("Command: %s\n", cmd_buf);
		system(cmd_buf);
	}

	memset(cmd_buf, 0, sizeof(cmd_buf));
	snprintf(cmd_buf, sizeof(cmd_buf), "ifconfig %s inet %s netmask %s", cfg->interface, cfg->ipaddr, cfg->netmask);
	printf("Command: %s\n", cmd_buf);
	system(cmd_buf);

	return 0;
}

static int create_tcp_server_socket(const MpttCfg *cfg, int port)
{
	int sock_fd;
	struct sockaddr_in server; /* server's address information */

	if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket() error. Failed to initiate a socket");
		return -1;
	}

	/* set socket option */
	int on = 1;
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
		perror("setsockopt() error. Failed to set a socket");
		close(sock_fd);
		sock_fd = -1;
		return -1;
	}

	struct timeval timeout = { 6, 0 };
	setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sock_fd, (struct sockaddr *)&server, sizeof(struct sockaddr)) < 0) {
		perror("bind() error");
		close(sock_fd);
		return -1;
	}

	if (listen(sock_fd, 1) == -1) {
		perror("listen() error");
		close(sock_fd);
		return -1;
	}

	return sock_fd;
}

int get_local_ip_addr(const char *netif, char *addr)
{
	int sock_fd = -1;
	struct ifreq ifr;

	if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket() error. Failed to initiate a socket");
		return -1;
	}

	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, netif, 31);
	ioctl(sock_fd, SIOCGIFADDR, &ifr);
	inet_ntop(AF_INET, &((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr.s_addr, addr, 32);

	close(sock_fd);
	return 0;
}

int main(void)
{
	MpttCfg cfg;
	char local_addr[32];
	int listen_fd = -1, com_fd = -1;
	struct sockaddr_in client;
	socklen_t addrlen;
	int ret = 0;
	fd_set fds;
	struct timeval timeout = {
		.tv_sec = 3, .tv_usec = 0,
	};

	printf("Tuya MP test tool\n");

	if (init_cfg(&cfg)) {
		printf("Error: Failed to init config\n");
		return -1;
	}

	parse_ini_file(CONFIG_FILE, &cfg);
	dump_cfg(&cfg);

	if (generate_wpa_supp_cfg(WPA_SUPP_FILE, &cfg)) {
		printf("Error: Failed to write wpa_supplicant.conf\n");
		goto network_fail;
	}

	if (init_network_if(&cfg)) {
		printf("Error: Failed to write wpa_supplicant.conf\n");
		goto network_fail;
	}

	listen_fd = create_tcp_server_socket(&cfg, TUYA_MQTT_SOCKET_PORT);
	if (listen_fd < 0) {
		printf("Error: Failed to open TCP socket\n");
		return -1;
	}

	ret = get_local_ip_addr(cfg.interface, local_addr);
	if (ret < 0) {
		printf("Error: Failed to get IP address from %s\n", cfg.interface);
		return -1;
	}
	printf("IP address: %s\n", local_addr);
	printf("Waiting for client connection...\n");
	while (1) {
		if (com_fd <= 0) { /* First connection */
			addrlen = sizeof(client);
			memset(g_client_addr, 0, sizeof(g_client_addr));
			com_fd = accept(listen_fd, (struct sockaddr *)&client, &addrlen);
			if (com_fd < 0) {
				if (errno != EAGAIN)
					perror("Fail to create client socket port\n");
				usleep(30000);
				continue;
			}
			g_start_flag = false;
			inet_ntop(AF_INET, &client.sin_addr.s_addr, g_client_addr, 32);
			printf("Client \"%s\" is connected\n", g_client_addr);
			printf("Waiting for command\n");
		}
		FD_ZERO(&fds);
		FD_SET(com_fd, &fds);
		ret = select(com_fd + 1, &fds, NULL, NULL, &timeout);
		if (ret < 0) {
			perror("select() error\n");
			close(com_fd);
			com_fd = -1;
		} else if (ret == 0) {
			//printf("select() timeout\n");
		} else {
			if (FD_ISSET(com_fd, &fds)) {
				DBG("socket fd %d is readable \n", com_fd);
				TuyaTestFrame *frame = tuya_get_frame(&com_fd);
				if (frame) {
					memcpy(frame->ip_addr, local_addr, sizeof(local_addr));
					tuya_dispatch_cmd(com_fd, frame);
					tuya_free_frame(frame);
				}
			} else {
				printf("FD_ISSET error\n");
			}
		}
	}

	close(com_fd);
	com_fd = -1;

	close(listen_fd);
	listen_fd = -1;

network_fail:

	release_cfg(&cfg);

	return 0;
}
