#include "agtx_lpw_cmd_common.h"

#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <sys/time.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define unused(x) ((x) = (x))

typedef struct {
	uint8_t fw_ver[3];
	uint8_t protocol_ver[3];
} info_t;

static sem_t g_info_cmd_sem;
static info_t g_module_info;
static uint32_t g_time;

void info_init(void)
{
	sem_init(&g_info_cmd_sem, 0, 0);
}

int agtx_lpw_ver_check(void)
{
	int ret = 0;
	agtx_cmd_send(CMD_INFO, INFO_H2D_GET_FW_VER, 0, NULL);
	sem_wait(&g_info_cmd_sem);
	agtx_cmd_send(CMD_INFO, INFO_H2D_GET_PROTOCOL_VER, 0, NULL);
	sem_wait(&g_info_cmd_sem);

	//FW version checking
	if (!(g_module_info.fw_ver[0] == FW_VER_X && g_module_info.fw_ver[1] == FW_VER_Y)) {
		if (g_module_info.fw_ver[0] == 1 && g_module_info.fw_ver[1] == 0) {
			ret = -1;
			printf("[ERR] LPW module FW needs to be updated. Current version:%u.%u.%u.\n",
			       g_module_info.fw_ver[0], g_module_info.fw_ver[1], g_module_info.fw_ver[2]);
		} else {
			printf("[WARN] Library FW version:%u.%u.%u doesn't match LPW FW version:%u.%u.%u.\n", FW_VER_X,
			       FW_VER_Y, FW_VER_Z, g_module_info.fw_ver[0], g_module_info.fw_ver[1],
			       g_module_info.fw_ver[2]);
		}
	}
	return ret;
}

void agtx_lpw_restore_time(void)
{
	struct timeval tv;
	agtx_cmd_send(CMD_INFO, INFO_H2D_GET_TIME, 0, NULL);
	sem_wait(&g_info_cmd_sem);
	tv.tv_sec = g_time;
	tv.tv_usec = 0;
	settimeofday(&tv, NULL);
	printf("[LPW] Restore wall time:%d\n", g_time);
}

void agtx_lpw_save_time(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	g_time = tv.tv_sec;
	agtx_cmd_send(CMD_INFO, INFO_H2D_SET_TIME, 4, (uint8_t *)&g_time);
	sem_wait(&g_info_cmd_sem);
	printf("[LPW] Save wall time:%d\n", g_time);
}

void info_cmd_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	unused(len);
	switch (sub_type) {
	case INFO_D2H_FW_VER:
		g_module_info.fw_ver[0] = data[0];
		g_module_info.fw_ver[1] = data[1];
		g_module_info.fw_ver[2] = data[2];
		sem_post(&g_info_cmd_sem);
		break;
	case INFO_D2H_PROTOCOL_VER:
		g_module_info.protocol_ver[0] = data[0];
		g_module_info.protocol_ver[1] = data[1];
		g_module_info.protocol_ver[2] = data[2];
		sem_post(&g_info_cmd_sem);
		break;
	case INFO_D2H_TIME:
		memcpy(&g_time, data, 4);
		sem_post(&g_info_cmd_sem);
		break;
	default:
		printf("[ERR] unknown info cmd\n");
		break;
	}
}

void info_req_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	unused(len);
	unused(data);
	switch (sub_type) {
	case INFO_H2D_GET_FW_VER:
		agtx_req_send(CMD_INFO, INFO_D2H_FW_VER, 3, g_module_info.fw_ver);
		break;
	case INFO_H2D_GET_PROTOCOL_VER:
		agtx_req_send(CMD_INFO, INFO_D2H_PROTOCOL_VER, 3, g_module_info.protocol_ver);
		break;
	default:
		printf("[ERR] unknown info req\n");
		break;
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
