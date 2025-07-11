#include "agtx_lpw_cmd_common.h"

#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <semaphore.h>

#if defined(LPW_HI3861L)
#include "hichannel_host.h"
#elif defined(LPW_AIW4211L)
#include "socchannel_host.h"
#endif

#define WIFI_MAX_SSID_LEN 32
#define WIFI_MAC_LEN 6
#define WIFI_AP_MAX_KEY_LEN 64
#define WIFI_AP_MIN_KEY_LEN 8
#define WIFI_STA_MAX_KEY_LEN 64
#define WIFI_STA_MIN_KEY_LEN 8
#define WIFI_WEP64_KEY_LEN 5
#define WIFI_WEP128_KEY_LEN 13

#define lpw_wifi(fmt, ...) printf("[LPW_WIFI]" fmt, ##__VA_ARGS__)
#define wifi_info(fmt, ...) lpw_wifi("[INFO] " fmt, ##__VA_ARGS__)
#define wifi_notice(fmt, ...) lpw_wifi("[NOTICE] " fmt, ##__VA_ARGS__)
#define wifi_warn(fmt, ...) lpw_wifi("[WARN] " fmt, ##__VA_ARGS__)
#define wifi_err(fmt, ...) lpw_wifi("[ERR] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef enum {
	WIFI_DISCONNECTED,
	WIFI_CONNECTED,
} wifi_conn_status;

typedef enum {
	MODE_STA,
	MODE_AP,
	MODE_DEFAULT,
} module_mode;

typedef enum {
	WIFI_CHANNEL_SCAN = 1,
	WIFI_SSID_SCAN,
	WIFI_SSID_PREFIX_SCAN,
	WIFI_BSSID_SCAN,
} wifi_scan_type;

typedef enum {
	WIFI_SECURITY_OPEN,
	WIFI_SECURITY_WEP,
	WIFI_SECURITY_WPA2PSK,
	WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	WIFI_SECURITY_WPAPSK,
	WIFI_SECURITY_WPA,
	WIFI_SECURITY_WPA2,
	WIFI_SECURITY_SAE,
	WIFI_SECURITY_WPA3_WPA2_PSK_MIX,
	WIFI_SECURITY_UNKNOWN,
} wifi_auth_mode;

typedef enum {
	WIFI_PARIWISE_UNKNOWN,
	WIFI_PAIRWISE_AES,
	WIFI_PAIRWISE_TKIP,
	WIFI_PAIRWISE_TKIP_AES_MIX,
} wifi_pairwise;

typedef struct {
	char channel_num;
	char authmode;
	char pairwise;
	/* <SSID> '\0' <key> '\0' */
	char data[];
} softap_info;

typedef struct {
	char mac[WIFI_MAC_LEN];
	char ip[4];
	char mask[4];
	char gw[4];
	char cc[4]; // country code
	unsigned int mode;
	signed char conn; /* conn is valid only when mode = MODE_STA */
	char ssid[WIFI_MAX_SSID_LEN + 1];
} status_info;

typedef struct {
	char bssid[WIFI_MAC_LEN];
	char channel;
	char scan_type;
	char ssid[];
} scan_info;

typedef struct {
	char bssid[WIFI_MAC_LEN];
	char channel;
	char auth;
	char ssid[WIFI_MAX_SSID_LEN + 1];
} scan_result;

typedef struct {
	signed char num;
	scan_result data[];
} scan_result_head;

typedef struct {
	scan_result_head head;
	scan_result result;
} scan_result_pack;

typedef struct {
	char bssid[WIFI_MAC_LEN];
	char auth;
	char pairwise;
	/* <SSID> '\0' <key> '\0' */
	char data[];
} conn_info;

/* global variable */
static int ap_auth_mode[] = {
	WIFI_SECURITY_OPEN,
	WIFI_SECURITY_WPA2PSK,
	WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	WIFI_SECURITY_SAE,
	WIFI_SECURITY_WPA3_WPA2_PSK_MIX
};
static int sta_auth_mode[] = {
	WIFI_SECURITY_OPEN,
	WIFI_SECURITY_WEP,
	WIFI_SECURITY_WPA2PSK,
	WIFI_SECURITY_WPAPSK_WPA2PSK_MIX,
	WIFI_SECURITY_WPA3_WPA2_PSK_MIX,
	WIFI_SECURITY_SAE
};
/* country code support list */
static char *valid_cc[] = { "AF", "AL", "DZ", "AS", "AO", "AI", "AG", "AR", "AM", "AW", "AU", "AT", "AZ", "BS", "BH",
	                    "BD", "BY", "BE", "BZ", "BM", "BO", "BA", "BR", "BN", "BG", "KH", "CA", "KY", "CL", "CN",
	                    "CO", "CR", "HR", "CU", "CY", "CZ", "DK", "DO", "EC", "EG", "SV", "EE", "ET", "FI", "FR",
	                    "GF", "GE", "DE", "GR", "GL", "GD", "GU", "GT", "VA", "HN", "HK", "HU", "IS", "IN", "ID",
	                    "IR", "IQ", "IE", "IL", "IT", "JM", "JP", "JO", "KZ", "KE", "KP", "KR", "KW", "LA", "LV",
	                    "LB", "LS", "LI", "LT", "LU", "MO", "MK", "MW", "MY", "MV", "MT", "MH", "MR", "MU", "YT",
	                    "MX", "MD", "MC", "MN", "ME", "MA", "NP", "NL", "NZ", "NI", "NG", "NO", "OM", "PK", "PA",
	                    "PG", "PY", "PE", "PH", "PL", "PT", "PR", "QA", "RE", "RO", "RU", "BL", "SA", "SN", "RS",
	                    "SG", "SK", "SI", "ZA", "ES", "LK", "SD", "SE", "CH", "SY", "TW", "TH", "TT", "TN", "TR",
	                    "UG", "UA", "AE", "GB", "US", "UY", "UZ", "VE", "VN", "VI", "YE", "ZM", "ZW" };
static char *default_cc = "AU";
static char g_cc[4] = { 0 };
static sem_t g_wifi_sem;
static int g_sem_flag = 0;
static module_mode g_current_mode = MODE_DEFAULT;
static status_info g_status;
static scan_result_pack g_scan_result_pack;
static int g_status_sync = 0;

void wifi_init(int argc, char **argv)
{
	unsigned int i;
	int ret;
	char *cc = NULL;

	/* Init semaphore */
	sem_init(&g_wifi_sem, 0, 0);

	/* Find parameter "-c" of country code */
	if (argc > 1) {
		for (i = 1; i < (unsigned int)argc; i++) {
			if (strcmp(argv[i], "-c") == 0) {
				cc = argv[i + 1];
				break;
			}
		}
	}

	/* Check country code is valid or not */
	if (cc == NULL) {
		wifi_warn("No country code found, use default value %s\n", default_cc);
		cc = default_cc;
	} else {
		/* Check if country code is in support list */
		for (i = 0; i < sizeof(valid_cc) / sizeof(char *); i++) {
			ret = strcmp(cc, valid_cc[i]);
			if (ret == 0) {
				break;
			}
		}
		if (i == sizeof(valid_cc) / sizeof(char *)) {
			wifi_warn("lpw_controller doesn't support %s, use default value %s\n", cc, default_cc);
			cc = default_cc;
		}
	}

	/* Save country code */
	strcpy(g_cc, cc);
}

static void set_wifi_country_code(void)
{
	/* Note: after setting country code, it need to call set sta/ap mode to take effect */
	/* Set country code if wifi module setting is not same as we require */
	if (strcmp(g_cc, g_status.cc) != 0) {
		wifi_info("set module country code as %s\n", g_cc);
		agtx_cmd_send(CMD_WIFI, WIFI_H2D_SET_CC, sizeof(g_cc), (uint8_t *)g_cc);
	}
}

int get_wifi_conn_status(void)
{
	return g_status.conn;
}

static int ifdown_interface(void)
{
	int ret;
	char cmd[SYSTEM_CMD_SIZE] = { 0 };

	memset(cmd, 0, SYSTEM_CMD_SIZE);
	if (snprintf(cmd, SYSTEM_CMD_SIZE - 1, "ifconfig %s down", SYSTEM_NETDEV_NAME) == -1) {
		goto ifdown_fail;
	}
	ret = system(cmd);
	if (ret != 0) {
		goto ifdown_fail;
	}
	return 0;

ifdown_fail:
	wifi_err("%s ifdown fail\n", SYSTEM_NETDEV_NAME);
	return -1;
}

static void wlan_init_gateway(char *gw)
{
	int ret;
	char cmd[SYSTEM_CMD_SIZE] = { 0 };

	memset(cmd, 0, SYSTEM_CMD_SIZE);
	if (snprintf(cmd, SYSTEM_CMD_SIZE - 1, "route add default gw %d.%d.%d.%d dev %s", gw[0], gw[1], gw[2], gw[3],
	             SYSTEM_NETDEV_NAME) == -1) {
		goto init_gw_fail;
	}
	ret = system(cmd);
	if (ret != 0) {
		goto init_gw_fail;
	}

	wifi_notice("%s add default gateway success\n", SYSTEM_NETDEV_NAME);
	return;

init_gw_fail:
	wifi_err("%s add default gateway fail\n", SYSTEM_NETDEV_NAME);
}

static void wlan_init_network(char *ip, char *mask, char *mac, char *gw)
{
	int ret;
	char cmd[SYSTEM_CMD_SIZE] = { 0 };

	/* need to ifconfig down interface before setting mac address */
	ret = ifdown_interface();
	if (ret == -1) {
		goto init_network_fail;
	}

	/* ifconfig <ip> will bring up interface automatically */
	memset(cmd, 0, SYSTEM_CMD_SIZE);
	if (snprintf(cmd, SYSTEM_CMD_SIZE - 1,
	             "ifconfig %s "
	             "hw ether %x:%x:%x:%x:%x:%x %d.%d.%d.%d "
	             "netmask %d.%d.%d.%d",
	             SYSTEM_NETDEV_NAME, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], ip[0], ip[1], ip[2], ip[3],
	             mask[0], mask[1], mask[2], mask[3]) == -1) {
		goto init_network_fail;
	}
	ret = system(cmd);
	if (ret != 0) {
		goto init_network_fail;
	}

	wifi_notice("%s set ip/netmask/mac success\n", SYSTEM_NETDEV_NAME);

	/* After ifconfig down, default gateway will be removed automatically, so we need to add it back */
	wlan_init_gateway(gw);
	return;

init_network_fail:
	wifi_err("%s set ip/netmask/mac fail\n", SYSTEM_NETDEV_NAME);
}

static void wlan_deinit_gateway(char *gw)
{
	int ret;
	char cmd[SYSTEM_CMD_SIZE] = { 0 };

	memset(cmd, 0, SYSTEM_CMD_SIZE);
	if (snprintf(cmd, SYSTEM_CMD_SIZE - 1, "route del default gw %d.%d.%d.%d dev %s", gw[0], gw[1], gw[2], gw[3],
	             SYSTEM_NETDEV_NAME) == -1) {
		goto deinit_gw_fail;
	}
	ret = system(cmd);
	/* delete gateway return error code(not -1) when it can't find specific rule */
	if (ret != 0) {
		goto deinit_gw_fail;
	}

	wifi_notice("%s delete default gateway success\n", SYSTEM_NETDEV_NAME);
	return;

deinit_gw_fail:
	wifi_err("%s delete default gateway fail\n", SYSTEM_NETDEV_NAME);
}

void get_wifi_status(status_info *info)
{
	int size;
	int change = 0;

	/* Wake up from sleep will reset g_current_mode, so we need to set it back
	 * when lpw controller sync wifi status */
	g_current_mode = info->mode;
	memcpy(g_status.cc, info->cc, sizeof(g_status.cc));

	if (info->mode == MODE_STA) {
		if (g_status.conn == WIFI_CONNECTED && info->conn == WIFI_DISCONNECTED) {
			/* Delete wlan0 default gateway when wifi disconnected from connected state */
			wifi_notice("get wifi status is disconnected, delete corresponding default gateway\n");
			wlan_deinit_gateway(g_status.gw);
		} else if (info->conn == WIFI_CONNECTED) {
			if (memcmp(g_status.ip, info->ip, sizeof(g_status.ip))) {
				memcpy(g_status.ip, info->ip, sizeof(g_status.ip));
				change = 1;
			}
			if (memcmp(g_status.mask, info->mask, sizeof(g_status.mask))) {
				memcpy(g_status.mask, info->mask, sizeof(g_status.mask));
				change = 1;
			}
			if (memcmp(g_status.gw, info->gw, sizeof(g_status.gw))) {
				memcpy(g_status.gw, info->gw, sizeof(g_status.gw));
				change = 1;
			}
			if (memcmp(g_status.mac, info->mac, sizeof(g_status.mac))) {
				memcpy(g_status.mac, info->mac, sizeof(g_status.mac));
				change = 1;
			}
			if (strcmp(g_status.ssid, info->ssid)) {
				size = strlen(info->ssid);
				memcpy(g_status.ssid, info->ssid, size + 1);
			}
			/* If wifi is connected and ip/netmask/gateway/mac is different from
			 * current status then update status and reinit interface */
			if (change == 1) {
				/* Set IP, Subnetmask, Gateway, MAC */
				wifi_notice(
				        "get wifi status is connected and status is changed, init ip/netmask/mac/gatway/mac\n");
				wlan_init_network(g_status.ip, g_status.mask, g_status.mac, g_status.gw);
			} else if (g_status.conn == WIFI_DISCONNECTED) {
				/* reinit gateway if network status is same and current wifi status is disconnected */
				wlan_init_gateway(g_status.gw);
			}
		}
	} else if (info->mode == MODE_AP) {
		wifi_notice("Init ip/netmask/mac/gateway of AP mode\n");
		memcpy(g_status.ip, info->ip, sizeof(g_status.ip));
		memcpy(g_status.mask, info->mask, sizeof(g_status.mask));
		memcpy(g_status.mac, info->mac, sizeof(g_status.mac));
		memcpy(g_status.gw, info->gw, sizeof(g_status.gw));
		wlan_init_network(g_status.ip, g_status.mask, g_status.mac, g_status.gw);
	}

	/* info.conn may be an error code */
	g_status.conn = info->conn;
}

static void get_scan_result(scan_result_head *info)
{
	scan_result *data;

	/* Clear old data */
	memset(&g_scan_result_pack, 0, sizeof(scan_result_pack));

	g_scan_result_pack.head.num = info->num;

	/* scan failed */
	if (info->num == 0) {
		return;
	}

	data = info->data;
	/* get first scan_result */
	memcpy(g_scan_result_pack.result.bssid, data->bssid, WIFI_MAC_LEN);
	g_scan_result_pack.result.channel = data->channel;
	g_scan_result_pack.result.auth = data->auth;
	memcpy(g_scan_result_pack.result.ssid, data->ssid, strlen(data->ssid) + 1);
}

void wifi_cmd_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	status_info *wifi_status;
	scan_result_head *result;

	// TODO: Fix by Ken
	len = len;

	switch (sub_type) {
	case WIFI_D2H_STATUS:
		wifi_status = (status_info *)data;
		get_wifi_status(wifi_status);
		if (g_status_sync == 0) {
			g_status_sync = 1;
			/* Set country code */
			set_wifi_country_code();
		}
		if (g_sem_flag == 1) {
			g_sem_flag = 0;
			sem_post(&g_wifi_sem);
		}
		break;
	case WIFI_D2H_SCAN_RESULT:
		result = (scan_result_head *)data;
		get_scan_result(result);
		sem_post(&g_wifi_sem);
		break;
	}
}

void wifi_req_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	scan_info *sc;
	softap_info *sfap;
	conn_info *conn;
	char *key;
	unsigned int cnt;

	switch (sub_type) {
	case WIFI_H2D_SET_MODE:
		/* 0: sta mode, 1: ap mode */
		if (data[0] >= MODE_DEFAULT) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			wifi_err("set mode fail, invalid mode\n");
		} else {
			if (data[0] == MODE_STA) {
				/* Only send first byte, ingore exceeded data */
				agtx_cmd_send(CMD_WIFI, WIFI_H2D_SET_MODE, 1, data);
				g_current_mode = data[0];
				/* g_status.conn should set as disconnected after set sta mode  */
				/* so WIFI_H2D_GET_STATUS can get correct connection state */
				g_status.conn = WIFI_DISCONNECTED;
				agtx_req_send(CMD_ERR, 0, 0, NULL);
			} else if (data[0] == MODE_AP) {
				sfap = (softap_info *)(data + 1);
				for (cnt = 0; cnt < sizeof(ap_auth_mode) / sizeof(int); cnt++) {
					if (sfap->authmode == ap_auth_mode[cnt]) {
						break;
					}
				}
				if (cnt == sizeof(ap_auth_mode) / sizeof(int)) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("set ap mode fail, invalid auth mode\n");
					return;
				}
				if (sfap->channel_num < 1 || sfap->channel_num > 14) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("set ap mode fail, invalid channel\n");
					return;
				}
				if (sfap->pairwise > WIFI_PAIRWISE_TKIP_AES_MIX) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("set ap mode fail, invalid pairwise\n");
					return;
				}
				if (strlen(sfap->data) > WIFI_MAX_SSID_LEN || strlen(sfap->data) == 0) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("set ap mode fail, invalid ssid length\n");
					return;
				}
				key = sfap->data + strlen(sfap->data) + 1;
				if (sfap->authmode != WIFI_SECURITY_OPEN) {
					if (strlen(key) > WIFI_AP_MAX_KEY_LEN || strlen(key) < WIFI_AP_MIN_KEY_LEN) {
						agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
						wifi_err("set ap mode fail, invalid key length\n");
						return;
					}
				}
				g_sem_flag = 1;
				agtx_cmd_send(CMD_WIFI, WIFI_H2D_SET_MODE, len, data);
				sem_wait(&g_wifi_sem);
				/* Clear g_status.conn */
				g_status.conn = WIFI_DISCONNECTED;
				agtx_req_send(CMD_ERR, 0, 0, NULL);
			}
		}
		break;
	case WIFI_H2D_GET_STATUS:
		if (g_status_sync == 0) {
			g_sem_flag = 1;
			sem_wait(&g_wifi_sem);
		}
		if (g_current_mode != MODE_STA) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			wifi_err("get wifi status fail, not in sta mode\n");
		} else {
			/* Get wifi status from cache in lpw controller */
			if (g_status.conn == -EIO) {
				agtx_req_send(CMD_ERR, EIO, 0, NULL);
				wifi_err("get wifi status failed\n");
			} else if (g_status.conn == WIFI_CONNECTED || g_status.conn == WIFI_DISCONNECTED) {
				agtx_req_send(CMD_WIFI, WIFI_D2H_STATUS, sizeof(g_status), (uint8_t *)&g_status);
			}
		}
		break;
	case WIFI_H2D_SCAN:
		if (g_current_mode != MODE_STA) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			wifi_err("wifi scanf fail, need to set sta mode first\n");
		} else {
			sc = (scan_info *)data;
			if (sc->scan_type > WIFI_BSSID_SCAN || sc->scan_type < WIFI_CHANNEL_SCAN) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				wifi_err("wifi scan fail, invalid scan type\n");
				return;
			}
			switch (sc->scan_type) {
			case WIFI_CHANNEL_SCAN:
				if (sc->channel < 1 || sc->channel > 14) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("wifi scan fail, invalid channel\n");
					return;
				}
				break;
			case WIFI_SSID_SCAN:
			case WIFI_SSID_PREFIX_SCAN:
				if (strlen(sc->ssid) > WIFI_MAX_SSID_LEN || strlen(sc->ssid) == 0) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("wifi scan fail, invalid ssid length\n");
					return;
				}
				break;
			case WIFI_BSSID_SCAN:
				if (sc->bssid[0] == 0 && sc->bssid[1] == 0 && sc->bssid[2] == 0 && sc->bssid[3] == 0 &&
				    sc->bssid[4] == 0 && sc->bssid[5] == 0) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("wifi scan fail, invalid bssid\n");
					return;
				}
				break;
			}
			agtx_cmd_send(CMD_WIFI, WIFI_H2D_SCAN, len, data);
			/* Wait for scan result */
			sem_wait(&g_wifi_sem);
			if (g_scan_result_pack.head.num == -EIO) {
				agtx_req_send(CMD_ERR, EIO, 0, NULL);
			} else {
				agtx_req_send(CMD_WIFI, WIFI_D2H_SCAN_RESULT, sizeof(g_scan_result_pack),
				              (uint8_t *)&g_scan_result_pack);
			}
		}
		break;
	case WIFI_H2D_CONN_AP:
		conn = (conn_info *)data;
		if (g_current_mode != MODE_STA) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			wifi_err("connect ap fail, need to set sta mode first\n");
		} else {
			conn = (conn_info *)data;
			for (cnt = 0; cnt < sizeof(sta_auth_mode) / sizeof(int); cnt++) {
				if (conn->auth == sta_auth_mode[cnt]) {
					break;
				}
			}
			if (cnt == sizeof(sta_auth_mode) / sizeof(int)) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				wifi_err("connect ap fail, invalid auth mode\n");
				return;
			}
			if (conn->pairwise > WIFI_PAIRWISE_TKIP_AES_MIX) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				wifi_err("connect ap fail, invalid pairwise\n");
				return;
			}
			if (strlen(conn->data) > WIFI_MAX_SSID_LEN) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				wifi_err("connect ap fail, invalid ssid length\n");
				return;
			}
			if (strlen(conn->data) == 0 &&
			    (conn->bssid[0] == 0 && conn->bssid[1] == 0 && conn->bssid[2] == 0 && conn->bssid[3] == 0 &&
			     conn->bssid[4] == 0 && conn->bssid[5] == 0)) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				wifi_err("connect ap fail, ssid and bssid can't be empty in same time\n");
				return;
			}
			key = conn->data + strlen(conn->data) + 1;
			if (conn->auth != WIFI_SECURITY_OPEN && conn->auth != WIFI_SECURITY_WEP) {
				if (strlen(key) < WIFI_STA_MIN_KEY_LEN || strlen(key) > WIFI_STA_MAX_KEY_LEN) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("connect ap fail, invalid key length\n");
					return;
				}
			} else if (conn->auth == WIFI_SECURITY_WEP) {
				if (strlen(key) != WIFI_WEP64_KEY_LEN && strlen(key) != WIFI_WEP128_KEY_LEN) {
					agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
					wifi_err("connect ap fail, invalid wep key length, should be %d or %d\n",
					         WIFI_WEP64_KEY_LEN, WIFI_WEP128_KEY_LEN);
					return;
				}
			}
			g_sem_flag = 1;
			agtx_cmd_send(CMD_WIFI, WIFI_H2D_CONN_AP, len, data);
			/* Wait for connection result */
			sem_wait(&g_wifi_sem);
			if (g_status.conn == WIFI_CONNECTED || g_status.conn == WIFI_DISCONNECTED) {
				agtx_req_send(CMD_WIFI, WIFI_D2H_STATUS, 1, (uint8_t *)&g_status.conn);
			} else if (g_status.conn == -EIO) {
				agtx_req_send(CMD_ERR, EIO, 0, NULL);
				wifi_err("connect ap, get wifi status fail\n");
			}
		}
		break;
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
