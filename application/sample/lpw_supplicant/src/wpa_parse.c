#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#include "wpa_parse.h"

#define MAX_NETWORK 20
#define WIFI_MAC_CHAR_LEN (WIFI_MAC_LEN * 2 + 5)

/* global variable */
static char *pattern[] = { "bssid", "scan_ssid", "ssid", "psk", "key_mgmt", "pairwise", "priority", "wep_key0" };
static wpa_conf g_sta_conf[MAX_NETWORK];
static int network_num = 0;

wpa_conf *get_wpa_conf(void)
{
	return g_sta_conf;
}

void print_network(int num)
{
	int i;
	int cnt;
	for (cnt = 0; cnt < num; cnt++) {
		printf("Network%d:\n", cnt);
		printf("ssid = %s\n", g_sta_conf[cnt].req.ssid);
		printf("bssid = ");
		for (i = 0; i < WIFI_MAC_LEN; i++)
			printf("%d ", g_sta_conf[cnt].req.bssid[i]);
		printf("\n");
		printf("pairwise = %d\n", g_sta_conf[cnt].req.pairwise);
		printf("priority = %d\n", g_sta_conf[cnt].priority);
	}
}

static int newline_terminated(const char *buf, size_t buflen)
{
	size_t len = strlen(buf);
	if (len == 0)
		return 0;
	if (len == buflen - 1 && buf[buflen - 1] != '\r' && buf[len - 1] != '\n')
		return 0;
	return 1;
}

static void skip_line_end(FILE *stream)
{
	char buf[100];
	while (fgets(buf, sizeof(buf), stream)) {
		buf[sizeof(buf) - 1] = '\0';
		if (newline_terminated(buf, sizeof(buf)))
			return;
	}
}

/**
 * wpa_config_get_line - Read the next configuration file line
 * @s: Buffer for the line
 * @size: The buffer length
 * @stream: File stream to read from
 * @line: Pointer to a variable storing the file line number
 * @_pos: Buffer for the pointer to the beginning of data on the text line or
 * %NULL if not needed (returned value used instead)
 * Returns: Pointer to the beginning of data on the text line or %NULL if no
 * more text lines are available.
 *
 * This function reads the next non-empty line from the configuration file and
 * removes comments. The returned string is guaranteed to be null-terminated.
 */
static char *wpa_config_get_line(char *s, int size, FILE *stream, int *line, char **_pos)
{
	char *pos, *end, *sstart;

	while (fgets(s, size, stream)) {
		(*line)++;
		s[size - 1] = '\0';
		if (!newline_terminated(s, size)) {
			/*
			 * The line was truncated - skip rest of it to avoid
			 * confusing error messages.
			 */
			skip_line_end(stream);
		}
		pos = s;

		/* Skip white space from the beginning of line. */
		while (*pos == ' ' || *pos == '\t' || *pos == '\r')
			pos++;

		/* Skip comment lines and empty lines */
		if (*pos == '#' || *pos == '\n' || *pos == '\0')
			continue;

		/*
		 * Remove # comments unless they are within a double quoted
		 * string.
		 */
		sstart = strchr(pos, '"');
		if (sstart)
			sstart = strrchr(sstart + 1, '"');
		if (!sstart)
			sstart = pos;
		end = strchr(sstart, '#');
		if (end)
			*end-- = '\0';
		else
			end = pos + strlen(pos) - 1;

		/* Remove trailing white space. */
		while (end > pos && (*end == '\n' || *end == ' ' || *end == '\t' || *end == '\r'))
			*end-- = '\0';

		if (*pos == '\0')
			continue;

		if (_pos)
			*_pos = pos;
		return pos;
	}

	if (_pos)
		*_pos = NULL;
	return NULL;
}

static int get_quote_content(char *buf, char *line, int length)
{
	char *left;
	char *right;
	int size;
	/* prevent left = NULL cause strchr() access NULL+1 with segmentation fault */
	left = strchr(line, '"');
	if (left)
		right = strchr(left + 1, '"');
	if (left != NULL && right != NULL) {
		size = right - left - 1;
		if (size != 0 && size <= length) {
			memcpy(buf, left + 1, size);
			return size;
		} else if (size > length) {
			memcpy(buf, left + 1, length);
			return -1;
		}
	}

	return 0;
}

static void scan_pattern(char *pos, int num, int line)
{
	int i;
	int len;
	int cnt;
	unsigned char hex;
	char *right;
	char *left;
	char *ptr;
	int key_mgmt_flag = 0;

	/* Check '=' symbol */
	if (strchr(pos, '=') == NULL) {
		supp_err("Line %d: format is incorrect\n", line);
		return;
	}

	/* Scan pattern */
	for (i = 0; i < (int)(sizeof(pattern) / sizeof(char *)); i++) {
		ptr = strstr(pos, pattern[i]);
		if (ptr != NULL) {
			break;
		}
	}
	if (ptr == NULL) {
		//supp_warn("Line %d: doesn't match any pattern\n", line);
		return;
	}

	switch (i) {
	case CONF_BSSID:
		left = strchr(pos, ':');
		if (left == NULL) {
			supp_err("Line %d: bssid format is incorrect\n", line);
			return;
		}
		/* Check format by ':' offset */
		for (cnt = 0; cnt < 5; cnt++) {
			if (*(left + cnt * 3) != ':') {
				supp_err("Line %d: bssid format is incorrect\n", line);
				return;
			}
		}
		left -= 2;
		right = strchr(pos, '\0');
		/* MAC string is 17 bytes, which contains ":" character also */
		if (right - left == WIFI_MAC_CHAR_LEN) {
			for (cnt = 0; cnt < 6; cnt++, left += 3) {
				hex = strtol(left, NULL, 16);
				g_sta_conf[num].req.bssid[cnt] = hex;
			}
		} else {
			supp_err("Line %d: bssid format is incorrect\n", line);
		}
		break;
	case CONF_SCAN_SSID:
		/* do nothing */
		break;
	case CONF_SSID:
		len = get_quote_content(g_sta_conf[num].req.ssid, pos, WIFI_MAX_SSID_LEN);
		if (len > 0) {
			g_sta_conf[num].req.ssid[len + 1] = '\n';
		} else if (len < 0) {
			supp_warn("Line %d: content of ssid is over limitation, copy %d character only\n", line,
			          WIFI_MAX_SSID_LEN);
		} else {
			supp_warn("Line %d: doesn't detect ssid value\n", line);
		}
		break;
	case CONF_PSK:
		len = get_quote_content(g_sta_conf[num].req.key, pos, WIFI_MAX_KEY_LEN);
		if (len > 0) {
			g_sta_conf[num].req.key[len + 1] = '\n';
		} else if (len < 0) {
			supp_warn("Line %d: content of psk is over limitation, copy %d character only\n", line,
			          WIFI_MAX_KEY_LEN);
		} else {
			supp_warn("Line %d: doesn't detect psk value\n", line);
		}
		break;
	case CONF_KEY_MGMT:
		if (strstr(pos, "NONE")) {
			g_sta_conf[num].req.auth = WIFI_SECURITY_OPEN;
			key_mgmt_flag = 1;
		}
		if (strstr(pos, "WPA-PSK")) {
			g_sta_conf[num].req.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX;
			key_mgmt_flag = 1;
		}
		if (strstr(pos, "SAE")) {
			g_sta_conf[num].req.auth = WIFI_SECURITY_WPA3_WPA2_PSK_MIX;
			key_mgmt_flag = 1;
		}
		if (key_mgmt_flag == 0) {
			g_sta_conf[num].req.auth = WIFI_SECURITY_WPAPSK_WPA2PSK_MIX;
			supp_warn("Line %d: doesn't detect key_mgmt value, use default value WPA-PSK\n", line);
		}
		break;
	case CONF_PAIRWISE:
		g_sta_conf[num].req.pairwise = WIFI_PAIRWISE_UNKNOWN;
		if (strstr(pos, "TKIP"))
			g_sta_conf[num].req.pairwise = WIFI_PAIRWISE_TKIP;
		if (strstr(pos, "CCMP"))
			g_sta_conf[num].req.pairwise |= WIFI_PAIRWISE_AES;
		if (g_sta_conf[num].req.pairwise == WIFI_PAIRWISE_UNKNOWN) {
			g_sta_conf[num].req.pairwise = WIFI_PAIRWISE_TKIP_AES_MIX;
			supp_warn("Line %d: doesn't detect pairwise value, use default value CCMP TKIP\n", line);
		}
		break;
	case CONF_PRIORITY:
		left = strchr(pos, '=') + 1;
		if (strchr(left, ' '))
			left = strrchr(left, ' ') + 1;
		g_sta_conf[num].priority = strtol(left, NULL, 10);
		break;
	case CONF_WEP_KEY0:
		g_sta_conf[num].req.auth = WIFI_SECURITY_WEP;
		len = get_quote_content(g_sta_conf[num].req.key, pos, WIFI_WEP128_KEY_LEN);
		if (len > 0) {
			g_sta_conf[num].req.key[len + 1] = '\n';
		} else if (len < 0) {
			supp_warn("Line %d: content of wep_key0 is over limitation, copy %d character only\n", line,
			          WIFI_WEP128_KEY_LEN);
		} else {
			supp_warn("Line %d: doesn't detect wep_key0 value\n", line);
		}
		break;
	}
}

int wpa_parse(char *conf_file)
{
	FILE *f;
	char *pos;
	int line = 0;
	char buf[512];

	/* reset network record when wpa_parse() be called */
	network_num = 0;
	memset(g_sta_conf, 0, sizeof(g_sta_conf));

	/* open config file */
	f = fopen(conf_file, "r");
	if (f == NULL) {
		supp_err("%s doesn't exist\n", conf_file);
		return -1;
	}

	/* wpa_config_get_line() will preprocess format of line and save result to buf */
	/* "line" will be moved to next line number after function called */
	/* "pos" will be set to the start of string */
	while (wpa_config_get_line(buf, sizeof(buf), f, &line, &pos)) {
		if (strcmp(pos, "network={") == 0) {
			while (wpa_config_get_line(buf, sizeof(buf), f, &line, &pos)) {
				/* break when encounter '}' */
				if (strcmp(pos, "}") == 0) {
					break;
				}
				scan_pattern(pos, network_num, line);
			}
			if (strlen(g_sta_conf[network_num].req.ssid) ||
			    strlen((char *)g_sta_conf[network_num].req.bssid)) {
				network_num++;
			} else {
				supp_err("There's a conf without ssid & bssid\n");
			}
		}
	}
	fclose(f);

	return network_num;
}
