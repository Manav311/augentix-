#include "agtx_lpw_cmd_common.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#if defined(LPW_HI3861L)
#include "hichannel_host.h"
#elif defined(LPW_AIW4211L)
#include "socchannel_host.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static int g_req_sock = 0;
static int g_loglevel = LOG_NOTICE;

void lpw_log_handler(uint8_t sub_type, uint16_t len, uint8_t *data);

void agtx_cmd_handler(unsigned char *buf, int length)
{
	if ((buf == NULL) || (length == 0)) {
		return;
	}
#ifdef DEBUG
	int i;
	printf("recv: [");
	for (i = 0; i < length; i++) {
		printf("%x ", buf[i]);
	}
	printf("]\n");
#endif
	switch (buf[0]) {
	case CMD_INFO:
		info_cmd_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_WIFI:
		wifi_cmd_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_PERIPHERAL:
		peripheral_cmd_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_TCP:
		tcp_cmd_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_WAKE_EVENT:
		wake_cmd_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_FW_UPG:
		fw_upg_cmd_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_LOG:
		lpw_log_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_ERR:
		printf("[err] cmd error\n");
		break;
	default:
		printf("[err] unknown cmd type\n");
	}
}

void lpw_log_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	char *log_type[] = { "DBG", "INFO", "NOTICE", "WARN", "ERROR" };
	if (sub_type >= g_loglevel && sub_type <= LOG_ERROR)
		printf("[LPW][%s] %*s", log_type[sub_type], len, (char *)data);
}

int agtx_cmd_send(uint8_t cmd_type, uint8_t sub_type, uint16_t len, uint8_t *data)
{
	int ret;
	unsigned char *pack;
	if (len > PAYLOAD_MAX_LEN)
		return -1;

	// todo: flowcontrol here
	usleep(10000);

	pack = malloc(CMD_MAX_LEN);
	if (pack == NULL) {
		printf("ERR: alloc fail for cmd send\n");
		return -1;
	}
	pack[0] = cmd_type;
	pack[1] = sub_type;
	pack[2] = len & 0xFF;
	pack[3] = (len >> 8) & 0xFF;
	memcpy(&pack[4], data, len);

#ifdef DEBUG
	int i;
	printf("cmd send(%d): [", len + 4);
	for (i = 0; i < len + 4; i++) {
		printf("%x ", pack[i]);
	}
	printf("]\n");
#endif

#if defined(LPW_HI3861L)
	ret = hi_channel_send_to_dev(pack, len + 4);
#elif defined(LPW_AIW4211L)
	ret = aich_channel_send_to_dev(pack, len + 4);
#endif

	free(pack);
	return ret;
}

void agtx_req_handler(unsigned char *buf, int length, int sock)
{
#ifdef DEBUG
	int i;
#endif
	if ((buf == NULL) || (length == 0)) {
		return;
	}

	g_req_sock = sock;

NEXT:
#ifdef DEBUG
	printf("req recv: [");
	for (i = 0; i < *(uint16_t *)&buf[2] + 4; i++) {
		printf("%x ", buf[i]);
	}
	printf("]\n");
#endif

	switch (buf[0]) {
	case CMD_INFO:
		info_req_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_WIFI:
		wifi_req_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_PERIPHERAL:
		peripheral_req_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_TCP:
		tcp_req_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_WAKE_EVENT:
		wake_req_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	case CMD_FW_UPG:
		fw_upg_req_handler(buf[1], *(uint16_t *)&buf[2], (uint8_t *)&buf[4]);
		break;
	default:
		printf("[err] unknown cmd type\n");
	}
	if (length > *(uint16_t *)&buf[2]) {
		length -= (*(uint16_t *)&buf[2] + 4);
		buf += (*(uint16_t *)&buf[2] + 4);
		if (length >= (*(uint16_t *)&buf[2] + 4))
			goto NEXT;
	}
	g_req_sock = 0;
}

int agtx_req_send(uint8_t cmd_type, uint8_t sub_type, uint16_t len, uint8_t *data)
{
	int ret;
	unsigned char *pack;
	if (len > PAYLOAD_MAX_LEN)
		return -1;
	pack = malloc(CMD_MAX_LEN);
	if (pack == NULL) {
		printf("ERR: alloc fail for cmd send\n");
		return -1;
	}
	pack[0] = cmd_type;
	pack[1] = sub_type;
	pack[2] = len & 0xFF;
	pack[3] = (len >> 8) & 0xFF;
	memcpy(&pack[4], data, len);

#ifdef DEBUG
	int i;
	printf("req send(%d): [", len + 4);
	for (i = 0; i < len + 4; i++) {
		printf("%x ", pack[i]);
	}
	printf("]\n");
#endif

	if (g_req_sock != 0) {
		if (write(g_req_sock, pack, len + 4) == -1) {
			printf("ERR: send req error!fd:%d\n", g_req_sock);
		}
	} else {
		//printf("ERR: send req response to unknown client\n");
	}

	free(pack);
	return ret;
}

void agtx_lpw_init(int argc, char **argv)
{
	info_init();
	peri_init();
	wake_init();
	wifi_init(argc, argv);
}

int agtx_lpw_sync(void)
{
	if (agtx_lpw_ver_check() < 0)
		return -1;
	agtx_lpw_restore_time();
	agtx_cmd_send(CMD_WIFI, WIFI_H2D_GET_STATUS, 0, NULL);
	gpio_dir_sync();
	return 0;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
