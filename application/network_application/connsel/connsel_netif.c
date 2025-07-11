#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "connsel_def.h"
#include "connsel_log.h"
#include "connsel_common.h"
#include "connsel_utils.h"


#define ETH0_NAME                     "eth0"
#define WLAN0_NAME                    "wlan0"
#define WLAN0_METRIC                  (20)
#define WLAN0_METRIC_UPDATE_PERIOD    (20000) /* in unit of ms */
#define WLAN0_METRIC_UPDATE_CNT       (WLAN0_METRIC_UPDATE_PERIOD / CONNSEL_THREAD_PERIOD)

typedef struct {
	connsel_state state;
} connsel_eth0_ctrl;

connsel_eth0_ctrl g_eth0;

connsel_route_entries eth0_route_list[CONNSEL_ROUTE_LIST_NUM];

void eth0_init(void)
{
	connsel_state state;
	connsel_eth0_ctrl *p = &g_eth0;

	state = connsel_get_iface_state(ETH0_NAME);

	connsel_info("Interface %s init state %d\n", ETH0_NAME, state);

	p->state = state;

	return;
}

void eth0_deinit(void)
{
	connsel_eth0_ctrl *p = &g_eth0;

	p->state = CONNSEL_STATE_NONE;

	connsel_info("Interface %s deinit\n", ETH0_NAME);

	return;
}

void eth0_run(void)
{
	connsel_eth0_ctrl *p = &g_eth0;
	connsel_state curr_state = connsel_get_iface_state(ETH0_NAME);
	char ip[256];

	switch (p->state) {
	case CONNSEL_STATE_ON:
		if (curr_state == CONNSEL_STATE_OFF) {
			connsel_info("%s state on -> off\n", ETH0_NAME);

			connsel_save_route_list(ETH0_NAME, eth0_route_list);
			connsel_del_route(ETH0_NAME, eth0_route_list);
		}
		break;

	case CONNSEL_STATE_OFF:
		if (curr_state == CONNSEL_STATE_ON) {
			connsel_info("%s state off -> on\n", ETH0_NAME);

			connsel_add_route(ETH0_NAME, eth0_route_list);
			connsel_clear_route_list(ETH0_NAME, eth0_route_list);
			system("/sbin/udhcpc -n -i eth0 -R -t 3 -T 3 -p /var/run/udhcpc_eth0.pid -S");

			sleep(9); // based on -t 3 -T 3

			connsel_get_inet_addr(ETH0_NAME, ip);

			if (strcmp(ip, "\0") == 0) {
				connsel_info("set static ip\n");
				system("ifconfig eth0 192.168.1.100 netmask 255.255.255.0");
				sleep(1);
			}

			connsel_get_inet_addr(ETH0_NAME, ip);
			connsel_info("@@@ ip = %s @@@\n", ip);
		}
		break;

	case CONNSEL_STATE_NONE:
		if (curr_state == CONNSEL_STATE_ON) {
			connsel_info("%s state none -> on\n", ETH0_NAME);
			/* Do nothing since route table will be modified by
			 * other applications */
		}
		break;

	default:
		break;
	}

	p->state = curr_state;
}


typedef struct {
	connsel_state state;
	unsigned int metric_cnt;
} connsel_wlan0_ctrl;

connsel_wlan0_ctrl g_wlan0;

connsel_route_entries wlan0_route_list[CONNSEL_ROUTE_LIST_NUM];

void wlan0_init(void)
{
	connsel_wlan0_ctrl *p = &g_wlan0;
	connsel_state state = connsel_get_iface_state(WLAN0_NAME);

	p->state = state;
	p->metric_cnt = 0;

	connsel_info("Interface %s init state %d\n", WLAN0_NAME, state);

	return;
}

void wlan0_deinit(void)
{
	connsel_wlan0_ctrl *p = &g_wlan0;

	p->state = CONNSEL_STATE_NONE;
	p->metric_cnt = 0;

	connsel_info("Interface %s deinit\n", WLAN0_NAME);

	return;
}



void wlan0_run(void)
{
	connsel_wlan0_ctrl *p = &g_wlan0;
	connsel_state curr_state = connsel_get_iface_state(WLAN0_NAME);

	switch (p->state) {
	case CONNSEL_STATE_ON:
		if (curr_state == CONNSEL_STATE_ON) {
			if (p->metric_cnt % WLAN0_METRIC_UPDATE_CNT == 0) {
				p->metric_cnt = 0;

				connsel_save_route_list(WLAN0_NAME, wlan0_route_list);
				connsel_update_metric(WLAN0_NAME, wlan0_route_list, WLAN0_METRIC);
				connsel_clear_route_list(WLAN0_NAME, wlan0_route_list);
			}

			p->metric_cnt++;
		} else if (curr_state == CONNSEL_STATE_OFF) {
			connsel_info("%s state on -> off\n", WLAN0_NAME);
		}
		break;

	case CONNSEL_STATE_OFF:
		if (curr_state == CONNSEL_STATE_ON) {
			p->metric_cnt = 0;
			connsel_info("%s state off -> on\n", WLAN0_NAME);
		}
		break;

	case CONNSEL_STATE_NONE:
		if (curr_state == CONNSEL_STATE_ON) {
			p->metric_cnt = 0;
			connsel_info("%s state none -> on\n", WLAN0_NAME);
			/* Do nothing since route table will be modified by
			 * other applications */
		}
		break;

	default:
		break;
	}

	p->state = curr_state;
}


const connsel_iface_entries connsel_iface_table[] = {
	{
		.name = ETH0_NAME,
		.data = &g_eth0,
		.init = eth0_init,
		.deinit = eth0_deinit,
		.run = eth0_run,
	},
	{
		.name = WLAN0_NAME,
		.data = &g_wlan0,
		.init = wlan0_init,
		.deinit = wlan0_deinit,
		.run = wlan0_run,
	},
};

const int connsel_iface_table_size = sizeof(connsel_iface_table) / sizeof(connsel_iface_table[0]);

