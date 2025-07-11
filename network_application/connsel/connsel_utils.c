#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <arpa/inet.h>

#include "connsel_def.h"
#include "connsel_log.h"
#include "connsel_common.h"


static void connsel_inet_ntoa(unsigned long haddr, char *buf)
{
	struct in_addr addr;

	addr.s_addr = haddr;

	strncpy(buf, inet_ntoa(addr), 16);

	return;
}

#if 0
void connsel_show_route_list(connsel_route_entries *data)
{
	int i;
	char net[16], gw[16], mask[16];
	connsel_route_entries *entry = NULL;
	connsel_route_entries *route_list = data;

	for (i = 0; i < CONNSEL_ROUTE_LIST_NUM; i++) {
		entry = route_list + i;

		if (strcmp(entry->name, "") != 0) {
			connsel_inet_ntoa(entry->net, net);
			connsel_inet_ntoa(entry->gw, gw);
			connsel_inet_ntoa(entry->mask, mask);

			connsel_info("[%d] %s - net %s gw %s netmask %s metric %d\n",
				     i, entry->name, net, gw, mask, entry->metric);
		}
	}
}
#endif

void connsel_save_route_list(char *iface, connsel_route_entries *data)
{
	FILE *fp;
	char devname[CONNSEL_IFNAME_LEN];
	unsigned long d, g, m;
	int flgs, ref, use, metric, mtu, win, ir;
	int i, r;

	connsel_route_entries *entry = NULL;
	connsel_route_entries *route_list = data;

	fp = fopen("/proc/net/route", "r");

	/* Skip the first line. */
	r = fscanf(fp, "%*[^\n]\n");
	if (r < 0) {
		/* Empty line, read error, or EOF. Yes, if routing table
		 * is completely empty, /proc/net/route has no header.
		 */
		goto ERROR;
	}

	i = 0;

	while (1) {

		r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
				devname, &d, &g, &flgs, &ref, &use, &metric, &m,
				&mtu, &win, &ir);
		if (r != 11) {
 ERROR:
			if ((r < 0) && feof(fp)) { /* EOF with no (nonspace) chars read. */
				break;
			}
			connsel_info("/proc/net/route may corrupted\n");
			break;
		}
#if 0
		if (!(flgs & CONNSEL_RTF_UP)) { /* Skip interfaces that are down. */
			continue;
		}
#endif
		if (strncmp(devname, iface, strlen(iface)) == 0) {
			entry = route_list + i;

			strncpy(entry->name, devname, strlen(devname));
			entry->net = d;
			entry->gw = g;
			entry->mask = m;
			entry->metric = metric;

			connsel_debug("add route list %d, iface %s net %lx gw %lx mask %lx metric %d\n",
				      i, devname, d, g, m, metric);
			i++;
		}
	}

	fclose(fp);

	connsel_debug("Save route list for %s\n", iface);
}

void connsel_clear_route_list(char *iface, connsel_route_entries *data)
{
	int i;
	connsel_route_entries *entry = NULL;
	connsel_route_entries *route_list = data;

	for (i = 0; i < CONNSEL_ROUTE_LIST_NUM; i++) {
		entry = route_list + i;

		if (strncmp(entry->name, iface, strlen(iface)) == 0) {
			memset(entry->name, 0, CONNSEL_IFNAME_LEN);
		}
	}

	connsel_debug("Clear route list for %s\n", iface);
}

void connsel_add_route(char *iface, connsel_route_entries *data)
{
	int i;
	char net[16], gw[16], mask[16];
	char buf[256];
	connsel_route_entries *entry = NULL;
	connsel_route_entries *route_list = data;

	for (i = CONNSEL_ROUTE_LIST_NUM - 1; i >= 0; i--) {
		entry = route_list + i;

		if (strncmp(entry->name, iface, strlen(iface)) == 0) {
			connsel_inet_ntoa(entry->net, net);
			connsel_inet_ntoa(entry->gw, gw);
			connsel_inet_ntoa(entry->mask, mask);

			if (strcmp(gw, "0.0.0.0") == 0) {
				sprintf(buf, "/sbin/route add -net %s netmask %s dev %s",
					net, mask, entry->name);
			} else {
				sprintf(buf, "/sbin/route add -net %s gw %s netmask %s dev %s",
					net, gw, mask, entry->name);
			}

			system(buf);

			connsel_info("Add route [%d] %s - net %s gw %s netmask %s\n",
				     i, entry->name, net, gw, mask);
		}
	}
}


void connsel_del_route(char *iface, connsel_route_entries *data)
{
	int i;
	char net[16], mask[16];
	char buf[256];
	connsel_route_entries *entry = NULL;
	connsel_route_entries *route_list = data;

	for (i = 0; i < CONNSEL_ROUTE_LIST_NUM; i++) {
		entry = route_list + i;

		if (strncmp(entry->name, iface, strlen(iface)) == 0) {
			connsel_inet_ntoa(entry->net, net);
			connsel_inet_ntoa(entry->mask, mask);

			sprintf(buf, "/sbin/route del -net %s netmask %s dev %s",
				net, mask, entry->name);

			system(buf);

			connsel_info("Delete route [%d] %s - net %s netmask %s\n",
				     i, entry->name, net, mask);
		}
	}
}


void connsel_update_metric(char *iface, connsel_route_entries *data, int met)
{
	int i;
	char net[16], gw[16], mask[16];
	char buf[256];
	connsel_route_entries *entry = NULL;
	connsel_route_entries *route_list = data;

	for (i = 0; i < CONNSEL_ROUTE_LIST_NUM; i++) {
		entry = route_list + i;

		if (strncmp(entry->name, iface, strlen(iface)) == 0 &&
		    entry->metric != met) {
			connsel_inet_ntoa(entry->net, net);
			connsel_inet_ntoa(entry->gw, gw);
			connsel_inet_ntoa(entry->mask, mask);

			sprintf(buf, "/sbin/route del -net %s netmask %s dev %s",
				net, mask, entry->name);

			system(buf);

			if (strcmp(gw, "0.0.0.0") == 0) {
				sprintf(buf, "/sbin/route add -net %s netmask %s metric %d dev %s",
					net, mask, met, entry->name);
			} else {
				sprintf(buf, "/sbin/route add -net %s gw %s netmask %s metric %d dev %s",
					net, gw, mask, met, entry->name);
			}

			system(buf);

			connsel_info("update metric [%d] %s - net %s gw %s netmask %s metric %d\n",
				     i, entry->name, net, gw, mask, met);
		}
	}
}

connsel_state connsel_get_iface_state(char *iface)
{
	int status = -1, dflags = 0;
	char buf[64], flags[8], carrier[4];
	FILE *pp = NULL;
	connsel_state state = CONNSEL_STATE_NONE;;

	sprintf(buf, "/bin/cat /sys/class/net/%s/flags", iface);

	pp = popen(buf, "r");
	if (pp == NULL) {
		connsel_info("Failed to run cat command\n");
		goto end;
	}

	if (fgets(flags, sizeof(flags), pp) == NULL) {
		connsel_info("Failed to read %s flags\n", iface);
		pclose(pp);
		goto end;
	}

	pclose(pp);

	dflags = (int)strtol(flags, NULL, 16);

	if (!(dflags & CONNSEL_RTF_UP)) {
		goto end;
	}

	sprintf(buf, "/bin/cat /sys/class/net/%s/carrier", iface);

	pp = popen(buf, "r");
	if (pp == NULL) {
		connsel_info("Failed to run cat command\n");
		goto end;
	}

	if (fgets(carrier, sizeof(carrier), pp) == NULL) {
		connsel_info("Failed to read %s carrier\n", iface);
		pclose(pp);
		goto end;
	}

	pclose(pp);

	status = atoi(carrier);

	if (status == 1) {
		state = CONNSEL_STATE_ON;
	} else if (status == 0) {
		state = CONNSEL_STATE_OFF;
	} else {
		state = CONNSEL_STATE_OFF;
	}

	connsel_debug("%s operstate: %s, carrier: %s, state: %d\n",
	              iface, operstate, carrier, state);
end:
	return state;
}

int connsel_get_inet_addr(const char *interface, char *ip)
{
#define MAX_RET_STR_SIZE (256)
#define MAX_HW_CMD_SIZE (256)

	FILE *fp;
	char str[MAX_RET_STR_SIZE] = { 0 };
	char cmd[MAX_HW_CMD_SIZE] = { 0 };

	sprintf(cmd, "ifconfig %s | awk '/inet addr/{print substr($2,6)}'", interface);

	/* execute command */
	fp = popen(cmd, "r");

	if (fp == NULL){
		pclose(fp);
		return -1;
	}

	if (fgets(str, MAX_RET_STR_SIZE, fp) != NULL) {
		sscanf(str, "%s", ip);
	} else {
		*ip = '\0';
	}

	pclose(fp);

	return 0;
}
