#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>

#include <linux/sockios.h>
#include <linux/ethtool.h>

#include "augentix.h"

int SYS_Gethostname(char *str)
{
	return gethostname(str, HOSTNAME_LEN);
}

int SYS_Getgateway(unsigned int *p)
{
	FILE *fp;
	char buf[256];
	char iface[16];
	unsigned long dest_addr, gate_addr;

	*p = INADDR_NONE;

	fp = fopen("/proc/net/route", "r");
	if (fp == NULL) {
		return -1;
	}

	/* Skip title line */
	fgets(buf, sizeof(buf), fp);

	while (fgets(buf, sizeof(buf), fp)) {
		if (sscanf(buf, "%s\t%lX\t%lX", iface, &dest_addr, &gate_addr) != 3 || dest_addr != 0)
			continue;
		*p = gate_addr;
		break;
	}

	fclose(fp);

	return 0;
}

char *SYS_Getipaddr(char *name, char *str)
{
	int fd;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFADDR, &ifr)) {
		printf("Get SIOCGIFADDR fail\n");
		close(fd);
		return str;
	}

	close(fd);

	/* display result */
	//printf("%s\n", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	strcpy(str, inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr));

	return str;
}

char *SYS_Getmacaddr(char *name, char *str)
{
	int fd;
	char *mac;
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	if (ioctl(fd, SIOCGIFHWADDR, &ifr)) {
		printf("Get SIOCGIFHWADDR fail\n");
		close(fd);
		return str;
	}

	close(fd);

	mac = ((struct sockaddr *)&ifr.ifr_addr)->sa_data;
	//sprintf(str,"%x:%x:%x:%x:%x:%x",mac[0],mac[1],mac[2],mac[3],mac[4],mac[5]);
	memcpy(str, mac, 6);

	return str;
}

int SYS_Getadminsettings(char *name, unsigned char *autoneg, unsigned short *speed, unsigned char *duplex)
{
	int fd;
	struct ethtool_cmd cmd = { 0 };
	struct ifreq ifr;

	fd = socket(AF_INET, SOCK_DGRAM, 0);

	ifr.ifr_addr.sa_family = AF_INET;

	strncpy(ifr.ifr_name, name, IFNAMSIZ - 1);

	ifr.ifr_data = (void *)&cmd;
	cmd.cmd = ETHTOOL_GSET; /* "Get settings" */
	if (ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
		/* Unknown */
		printf("Get SIOCETHTOOL fail %d(%m)\n", errno);
		speed = 0;
		duplex = 0;
		autoneg = 0;
		close(fd);
	} else {
		*autoneg = cmd.autoneg; //AutoNegotiation
		*speed = ethtool_cmd_speed(&cmd);
		*duplex = cmd.duplex;
	}

	close(fd);

	return 0;
}
