#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "agtx_lpw.h"
#include "wpa_parse.h"

#define DEFAULT_WPA_CONFIG_FILE "/etc/wpa_supplicant.conf"

wpa_conf *g_wpa_conf;

static void try_wpa_list(lpw_handle hd, int num)
{
	int cnt = 0;
	unsigned char empty_bssid[] = { 0, 0, 0, 0, 0, 0 };
	int scan_status[num];
	int conn_status[num];
	int scan_ret = -1;
	int conn_ret = -1;
	int sleep_interval = 30;

	for (cnt = 0; cnt < num; cnt++)
		conn_status[cnt] = scan_status[cnt] = -1;
	cnt = 0;

	/* scan target and result */
	lpw_wifi_scan_t sc;
	memset(&sc, 0, sizeof(lpw_wifi_scan_t));
	sc.ssid = calloc(WIFI_MAX_SSID_LEN + 1, sizeof(char));
	lpw_wifi_scan_result_t result;

	/* connect info */
	lpw_wifi_conn_t conn;
	memset(&conn, 0, sizeof(lpw_wifi_conn_t));
	conn.ssid = calloc(WIFI_MAX_SSID_LEN + 1, sizeof(char));
	conn.key = calloc(WIFI_MAX_KEY_LEN + 1, sizeof(char));

	supp_notice("Retry connection every %u seconds\n", sleep_interval);
	while (1) {
		if (g_wpa_conf[cnt].req.ssid[0] != '\0') {
			/* Scan SSID */
			strcpy(sc.ssid, g_wpa_conf[cnt].req.ssid);
			/* Indicate scan type */
			sc.scan_type = WIFI_SSID_SCAN;
			if (scan_status[cnt] == -1 || scan_ret != scan_status[cnt])
				supp_info("scan AP: %s", g_wpa_conf[cnt].req.ssid);
		} else if (memcmp(g_wpa_conf[cnt].req.bssid, empty_bssid, 6)) {
			memcpy(sc.bssid, g_wpa_conf[cnt].req.bssid, 6);
			sc.scan_type = WIFI_BSSID_SCAN;
			if (scan_status[cnt] == -1 || scan_ret != scan_status[cnt])
				supp_info("scan AP: %d:%d:%d...", g_wpa_conf[cnt].req.bssid[0],
				          g_wpa_conf[cnt].req.bssid[1], g_wpa_conf[cnt].req.bssid[2]);
		} else {
			goto NEXT;
		}
		scan_status[cnt] = scan_ret;
		scan_ret = lpw_wifi_scan(hd, &sc, &result);

		if (scan_status[cnt] == -1 || scan_ret != scan_status[cnt]) {
			/* sc_ret = 0 means target exist */
			if (scan_ret == 0) {
				printf("...present\n");
			} else if (scan_ret == -ENODEV) {
				printf("...doesn't exist\n");
			} else {
				printf("...error = %d\n", scan_ret);
				break;
			}
		}
		if (scan_ret == 0) {
			/* Fill conn structure */
			strncpy(conn.ssid, g_wpa_conf[cnt].req.ssid, WIFI_MAX_SSID_LEN);
			strncpy(conn.key, g_wpa_conf[cnt].req.key, WIFI_MAX_KEY_LEN);
			conn.auth = g_wpa_conf[cnt].req.auth;
			conn.pairwise = g_wpa_conf[cnt].req.pairwise;
			memcpy(conn.bssid, g_wpa_conf[cnt].req.bssid, WIFI_MAC_LEN);
			/* Connect AP */
			conn_status[cnt] = conn_ret;
			conn_ret = lpw_wifi_connect_to(hd, &conn);
			if (conn_status[cnt] == -1 || conn_ret != conn_status[cnt]) {
				if (conn_ret == 0) {
					/* if connect successful then break while loop */
					supp_info("connect ssid = \"%s\" successful\n", g_wpa_conf[cnt].req.ssid);
					break;
				} else if (conn_ret == -EACCES) {
					/* return -EACCES means wifi is unconnected */
					supp_notice("connect ssid = \"%s\" fail\n", g_wpa_conf[cnt].req.ssid);
				} else {
					supp_err("connect ssid = \"%s\" error = %d\n", g_wpa_conf[cnt].req.ssid,
					         conn_status[cnt]);
				}
			}
		}
	NEXT:
		cnt++;
		if (cnt == num) {
			sleep(sleep_interval);
			cnt = 0;
		}
	}
	free(conn.ssid);
	free(conn.key);
	free(sc.ssid);
}

int main(int argc, char *argv[])
{
	int network_num = 0;
	g_wpa_conf = get_wpa_conf();
	int cnt = 0;
	int ret;
	int retry = 5;
	/* wifi status */
	lpw_wifi_state_t st;
	lpw_handle hd = (lpw_handle)NULL;

	if (argc >= 2) {
		if (strcmp(argv[1], "-c") == 0) {
			network_num = wpa_parse(argv[2]);
			if (network_num <= 0) {
				supp_notice("parse %s file failed, use %s instead\n", argv[2], DEFAULT_WPA_CONFIG_FILE);
			} else {
				supp_notice("parse %s success, get %d network from file\n", argv[2], network_num);
			}
		} else {
			supp_notice("incorrect input argument, correct format is lpw_supplicant -c <file> "
			            "use %s instead\n",
			            DEFAULT_WPA_CONFIG_FILE);
		}
	}
	if (network_num <= 0) {
		network_num = wpa_parse(DEFAULT_WPA_CONFIG_FILE);
		if (network_num <= 0) {
			supp_err("parse %s file failed, terminate lpw_supplicant\n", DEFAULT_WPA_CONFIG_FILE);
			return -1;
		} else {
			supp_notice("parse %s success, get %d network from file\n", DEFAULT_WPA_CONFIG_FILE,
			            network_num);
		}
	}

	/* retry when lpw controller is not ready yet */
	do {
		usleep(50000);
		retry--;
		if (retry == 0) {
			supp_err("open lpw device fail, terminate\n");
			return -1;
		}
		hd = lpw_open();
	} while (hd == (lpw_handle)NULL);

	if (hd == (lpw_handle)NULL) {
		supp_err("open lpw device success\n");
	}

	while (1) {
		/* get wifi status */
		ret = lpw_wifi_get_status(hd, &st);
		if (ret < 0) {
			/* set sta mode */
			lpw_wifi_set_sta_mode(hd);
			/* try wpa list until wifi is connected */
			try_wpa_list(hd, network_num);
			/* update status */
			lpw_wifi_get_status(hd, &st);
		}
		/* If it's connected already, check if SSID is valid */
		if (st.conn == 1) {
			for (cnt = 0; cnt < network_num; cnt++) {
				if (strcmp(st.ssid, g_wpa_conf[cnt].req.ssid) == 0) {
					break;
				}
			}
		}
		/* If wifi is unconnected or SSID is not valid, try wpa list */
		if (st.conn == 0 || cnt == network_num) {
			try_wpa_list(hd, network_num);
		}
		sleep(3);
	}

	lpw_close(hd);

	return 0;
}
