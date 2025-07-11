#ifndef wpa_parse_h
#define wpa_parse_h

#include "agtx_lpw.h"

#define lpw_supp(fmt, ...) printf("[LPW_SUPP]" fmt, ##__VA_ARGS__)
#define supp_info(fmt, ...) lpw_supp("[INFO] " fmt, ##__VA_ARGS__)
#define supp_notice(fmt, ...) lpw_supp("[NOTICE] " fmt, ##__VA_ARGS__)
#define supp_warn(fmt, ...) lpw_supp("[WARN] " fmt, ##__VA_ARGS__)
#define supp_err(fmt, ...) lpw_supp("[ERR] " fmt, ##__VA_ARGS__)

#define WIFI_WEP64_KEY_LEN 5
#define WIFI_WEP128_KEY_LEN 13

typedef enum {
	CONF_BSSID,
	CONF_SCAN_SSID,
	CONF_SSID,
	CONF_PSK,
	CONF_KEY_MGMT,
	CONF_PAIRWISE,
	CONF_PRIORITY,
	CONF_WEP_KEY0
} wpa_item;

typedef struct {
	char ssid[WIFI_MAX_SSID_LEN + 1];
	wifi_auth_mode auth;
	char key[WIFI_MAX_KEY_LEN + 1];
	unsigned char bssid[WIFI_MAC_LEN];
	wifi_pairwise pairwise;
} wifi_assoc_request;

typedef struct {
	int priority;
	wifi_assoc_request req;
} wpa_conf;

wpa_conf *get_wpa_conf(void);
void print_network(int num);
int wpa_parse(char *conf_file);

#endif
