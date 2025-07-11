#include "sysinfo.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <syslog.h>
#include <unistd.h>

#include "json.h"

#include "netinfo.h"

#define JSON_BUF_STR_SIZE 8192

char *getDevModel()
{
	int err, n;
	struct sockaddr_un ccserv_addr;
	char buffer[JSON_BUF_STR_SIZE];

	memset((char *)&ccserv_addr, 0, sizeof(ccserv_addr));
	ccserv_addr.sun_family = AF_UNIX;
	strcpy(ccserv_addr.sun_path, "/tmp/ccUnxSkt");
	int servlen = strlen(ccserv_addr.sun_path) + sizeof(ccserv_addr.sun_family);

	int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sockfd < 0) {
		syslog(LOG_ERR, "Get dev name failed: ccserver not exist.");
		return NULL;
	}

	err = connect(sockfd, (struct sockaddr *)&ccserv_addr, servlen);
	if (err != 0) {
		syslog(LOG_ERR, "Get dev name failed: cannot connect ccserver.");
		return NULL;
	}
	char cmd_start[] = "{\"master_id\":0,\"cmd_id\":1048577,\"cmd_type\":\"ctrl\"}"; // AGTX_CMD_SESS_START
	write(sockfd, cmd_start, strlen(cmd_start));
	n = read(sockfd, buffer, JSON_BUF_STR_SIZE - 2);
	if (n < 0) {
		syslog(LOG_ERR, "Get dev name failed: cannot start ccserver session.");
		return NULL;
	}

	char cmd[] = "{\"master_id\":1,\"cmd_id\":1048579,\"cmd_type\":\"get\"}"; // AGTX_CMD_SYS_INFO
	write(sockfd, cmd, strlen(cmd));
	n = read(sockfd, buffer, JSON_BUF_STR_SIZE - 2);
	if (n < 0) {
		syslog(LOG_ERR, "Get dev name failed: cannot recieved response from ccserver.");
		return NULL;
	}
	buffer[n] = 0;

	json_object *json_obj, *tmp_obj;
	struct json_tokener *jtok = json_tokener_new();
	json_obj = json_tokener_parse_ex(jtok, buffer, n);
	if (!(json_object_object_get_ex(json_obj, "dev_name", &tmp_obj)) ||
	    (json_object_get_type(tmp_obj) != json_type_string)) {
		syslog(LOG_ERR, "Get dev name failed: cannot get dev name from ccserver.");
		return NULL;
	}
	return (char *)json_object_get_string(tmp_obj);
}

char *getDevName()
{
	char *hostname = (char *)malloc(128);
	gethostname(hostname, 127);
	return hostname;
}

char *getHwVer()
{
	return (char *)"1.0";
}

char *getFwVer()
{
	FILE *fp = fopen("/etc/sw-version", "r");
	if (fp == NULL) {
		return (char *)"0.0.1";
	}
	char *ver = (char *)malloc(100);
	fgets(ver, 100, fp);
	fclose(fp);

	// Remove last ctrl char
	int n = strlen(ver) - 1;
	if (iscntrl(ver[n])) {
		ver[n] = 0;
	}
	return ver;
}

char *get_sys_info()
{
	char *model = getDevModel();
	if (model == NULL) {
		return NULL;
	}
	char *name = getDevName();

	char ip[16] = { 0 };
	strcpy(ip, get_ip());
	char netmask[16] = { 0 };
	strcpy(netmask, get_netmask());
	char *mac_addr = get_mac_addr();
	if (mac_addr == NULL) {
		return NULL;
	}

	char *hw_ver = getHwVer();
	char *fw_ver = getFwVer();

	char *sys_info = (char *)malloc(JSON_BUF_STR_SIZE);
	sprintf(sys_info,
	        "{\"model\":\"%s\",\"name\":\"%s\",\"ip\":\"%s\",\"netmask\":\"%s\",\"mac_addr\":\"%s\",\"hw_ver\":\"%s\",\"fw_ver\":\"%s\"}",
	        model, name, ip, netmask, mac_addr, hw_ver, fw_ver);
	return sys_info;
}

char *get_mode()
{
	FILE *fp = fopen("/usrdata/mode", "r");
	if (fp == NULL) {
		syslog(LOG_WARNING, "Cannot get device mode.");
		return NULL;
	}
	char *mode = (char *)malloc(8);
	fgets(mode, 8, fp);
	fclose(fp);

	// Remove last ctrl char
	int n = strlen(mode) - 1;
	if (iscntrl(mode[n])) {
		mode[n] = 0;
	}
	return mode;
}
