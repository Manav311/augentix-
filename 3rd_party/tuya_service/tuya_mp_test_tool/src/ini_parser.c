#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "ini_parser.h"
#include "iniparser.h"


int init_cfg(MpttCfg * cfg)
{
	cfg->ini = NULL;
	return 0;
}

void release_cfg(MpttCfg * cfg)
{
	if (cfg->ini != NULL)
		iniparser_freedict(cfg->ini);
}

void dump_cfg(MpttCfg *cfg)
{
	printf("ssid: %s\n", cfg->ssid);
	printf("passwd: %s\n", cfg->passwd);
	printf("ipaddr: %s\n", cfg->ipaddr);
	printf("netmask: %s\n", cfg->netmask);
	printf("interface: %s\n", cfg->interface);
	printf("usb_eth_ipaddr: %s\n", cfg->usb_eth_ipaddr);
	printf("pc_ipaddr: %s\n", cfg->pc_ipaddr);
}

int parse_ini_file(const char * file, MpttCfg * cfg)
{
	cfg->ini = iniparser_load(file);
	if (cfg->ini == NULL) {
		fprintf(stderr, "Failed to parse file \"%s\"\n", file);
		return -1;
	}

	/* SSID */
	cfg->ssid = iniparser_getstring(cfg->ini, "wifi:ssid", "undefined");
	/* passwd */
	cfg->passwd = iniparser_getstring(cfg->ini, "wifi:password", "undefined");
	/* DUT ipaddr */
	cfg->ipaddr = iniparser_getstring(cfg->ini, "wifi:ipaddr", "0.0.0.0");
	/* netmask */
	cfg->netmask = iniparser_getstring(cfg->ini, "wifi:netmask", "255.255.255.0");
	/* interface */
	cfg->interface = iniparser_getstring(cfg->ini, "wifi:interface", "wlan0");
	/* USB Eth ipaddr*/
	cfg->usb_eth_ipaddr = iniparser_getstring(cfg->ini, "wifi:usbethernet", "192.168.2.100");
	/* pc ipaddr */
	cfg->pc_ipaddr = iniparser_getstring(cfg->ini, "wifi:pc", "0.0.0.0");

	return 0;
}
