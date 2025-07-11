#include "netinfo.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#define JSON_BUF_STR_SIZE 8192

char *get_ip()
{
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFADDR, &ifr);
	close(fd);
	return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

char *get_netmask()
{
	int fd;
	struct ifreq ifr;
	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ - 1);
	ioctl(fd, SIOCGIFNETMASK, &ifr);
	close(fd);
	return inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr);
}

char *get_mac_addr()
{
	FILE *fp = fopen("/sys/class/net/eth0/address", "r");
	if (fp == NULL) {
		syslog(LOG_ERR, "Cannot get mac address info.");
		return NULL;
	}
	char *mac_addr = (char *)malloc(100);
	fgets(mac_addr, 100, fp);
	fclose(fp);

	// Remove last ctrl char
	int n = strlen(mac_addr) - 1;
	if (iscntrl(mac_addr[n])) {
		mac_addr[n] = 0;
	}
	return mac_addr;
}

char *get_net_settings()
{
	char buf[100];
	char buf1[50];
	char ip[16] = { 0 };
	char netmask[16] = { 0 };
	char gateway[16] = { 0 };
	char dns1[16] = { 0 };
	char dns2[16] = { 0 };
	int dhcp_status = 0;

	FILE *fp = fopen("/etc/network/interfaces", "r");
	fgets(buf, sizeof(buf), fp); // Pass 1st line: `auto lo`
	fgets(buf, sizeof(buf), fp); // Pass 2nd line: `iface lo inet loopback`

	while (fgets(buf, sizeof(buf), fp) != NULL) {
		if (sscanf(buf, "iface eth0 inet %s", buf1) == 1) {
			if (strcmp(buf1, "static") != 0) {
				dhcp_status = DHCP_ENABLE;
			}
		} else if (sscanf(buf, "address %s", ip) == 1) {
		} else if (sscanf(buf, "netmask %s", netmask) == 1) {
		} else if (sscanf(buf, "gateway %s", gateway) == 1) {
		} else if (sscanf(buf, "dns-nameservers %s %s", dns1, dns2) > 1) {
		}
	}
	char *net_settings = (char *)malloc(JSON_BUF_STR_SIZE);
	sprintf(net_settings,
	        "{\"dhcp_status\":%d,\"ip\":\"%s\",\"netmask\":\"%s\",\"gateway\":\"%s\",\"dns1\":\"%s\",\"dns2\":\"%s\"}",
	        dhcp_status, ip, netmask, gateway, dns1, dns2);
	return net_settings;
}

int set_net_setting(int dhcp_status, char *ip, char *netmask, char *gateway, char *dns1, char *dns2)
{
	char arg_dhcp[10] = { 0 };
	sprintf(arg_dhcp, "-s %s", (dhcp_status != DHCP_DISABLE) ? "dhcp" : "static");
	char arg_ip[20] = { 0 };
	if (ip != NULL) {
		sprintf(arg_ip, "-i %s", ip);
	} else if (dhcp_status == DHCP_DISABLE) {
		syslog(LOG_ERR, "IP address undefined.");
		return -1;
	}
	char arg_netmask[20] = { 0 };
	if (netmask != NULL) {
		sprintf(arg_netmask, "-m %s", netmask);
	} else if (dhcp_status == DHCP_DISABLE) {
		syslog(LOG_ERR, "Netmask undefined.");
		return -1;
	}
	char arg_gateway[20] = { 0 };
	if (gateway != NULL) {
		sprintf(arg_gateway, "-g %s", gateway);
	} else if (dhcp_status == DHCP_DISABLE) {
		syslog(LOG_ERR, "Gateway undefined.");
		return -1;
	}
	char arg_dns[36] = { 0 };
	if (dns1 != NULL) {
		if (dns2 != NULL) {
			sprintf(arg_dns, "-d \"%s %s\"", dns1, dns2);
		} else {
			sprintf(arg_dns, "-d %s", dns1);
		}
	} else if (dns2 != NULL) {
		sprintf(arg_dns, "-d %s", dns2);
	} else if (dhcp_status == DHCP_DISABLE) {
		syslog(LOG_WARNING, "DNS undefined, use gateway IP instead.");
		sprintf(arg_dns, "-d %s", gateway);
	}
	char cmd[128] = { 0 };
	sprintf(cmd, "ip_assign %s %s %s %s %s", arg_dhcp, arg_ip, arg_netmask, arg_gateway, arg_dns);
	int err = system(cmd);
	if (err != 0) {
		syslog(LOG_ERR, "Call ip_assign failed.");
		return -1;
	}
	return 0;
}
