#include "agtx_lpw_cmd_common.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <sys/timeb.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

//#define DEBUG

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define NEW_FW_LEN_BYTES 4
#define NEW_FW_CRC_BYTES 4
#define FWUPG_H2D_INIT_PAYLOAD_BYTES 8
#define STATUS_PAYLOAD_BYTES 1

struct fw_upg_info {
	uint8_t new_fw_crc[NEW_FW_CRC_BYTES];
	uint32_t new_fw_len;
	uint32_t cum_fw_len;
	uint8_t *new_fw;
};

static struct fw_upg_info g_fw_upg_info;
static sem_t g_fw_upg_sem;
static uint8_t g_status[STATUS_PAYLOAD_BYTES];
#ifdef DEBUG
#define TIME_ELE_CNT 9
int32_t g_time[TIME_ELE_CNT] = { 0 };
struct timeb g_s_time;
struct timeb g_e_time;
#endif

void fw_upg_init(void)
{
	sem_init(&g_fw_upg_sem, 0, 0);
}

void fw_upg_cmd_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	uint32_t ret;
#ifdef DEBUG
	struct timeb s_time;
	struct timeb e_time;
#endif

	switch (sub_type) {
	case FWUPG_D2H_INIT_DONE:
#ifdef DEBUG
		ftime(&s_time);
#endif

		DBG("[INFO] fw_upg_cmd_handler: start FWUPG_D2H_INIT_DONE ...\n");
		/**
		 * check the length of payload and payload which contains:
		 * 1. uint32_t success
		 */
		if (len != STATUS_PAYLOAD_BYTES) {
			printf("[ERR] fw_upg_cmd_handler: payload len. of INIT_DONE error.\n");
			break;
		}

		/* copy verify success status */
		if (memcpy(g_status, data, STATUS_PAYLOAD_BYTES) == NULL) {
			printf("[ERR] fw_upg_cmd_handler: memcpy of status is fail.\n");
			break;
		}
		DBG("[INFO] fw_upg_cmd_handler: g_status: %d.\n", g_status[0]);

		/* unlock semaphore: activating FWUPG_D2H_INIT */
		ret = sem_post(&g_fw_upg_sem);
		if (ret != 0) {
			printf("[ERR] fw_upg_cmd_handler: fail to unlock semaphore.\n");
			break;
		}
#ifdef DEBUG
		ftime(&e_time);
		g_time[3] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
#endif

		break;
	case FWUPG_D2H_VERIFY:
#ifdef DEBUG
		ftime(&s_time);
#endif
		DBG("[INFO] fw_upg_cmd_handler: start FWUPG_D2H_VERIFY ...\n");
		/**
		 * check the length of payload and payload which contains:
		 * 1. uint32_t success
		 */
		if (len != STATUS_PAYLOAD_BYTES) {
			printf("[ERR] fw_upg_cmd_handler: payload len. of VERIFY error.\n");
			break;
		}

		/* copy verify success status */
		if (memcpy(g_status, data, STATUS_PAYLOAD_BYTES) == NULL) {
			printf("[ERR] fw_upg_cmd_handler: memcpy of status is fail.\n");
			break;
		}
		DBG("[INFO] fw_upg_cmd_handler: g_status: %d.\n", g_status[0]);

		/* unlock semaphore: activating FWUPG_D2H_DOWNLOAD */
		ret = sem_post(&g_fw_upg_sem);
		if (ret != 0) {
			printf("[ERR] fw_upg_cmd_handler: fail to unlock semaphore.\n");
			break;
		}
#ifdef DEBUG
		ftime(&e_time);
		g_time[7] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
#endif
		break;
	default:
		printf("[ERR] fw_upg_cmd_handler: unknown fw upg command.\n");
		break;
	}
}

void fw_upg_req_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	int ret = 0;
#ifdef DEBUG
	struct timeb s_time;
	struct timeb e_time;
#endif

	switch (sub_type) {
	case FWUPG_H2D_INIT:
#ifdef DEBUG
		ftime(&g_s_time);
		ftime(&s_time);
#endif
		/**
		 * check the length of payload and payload which contains:
		 * 1. uint32_t new_fw_size
		 * 2. uint32_t crc
		 * NOTE: currently, CRC is stored at the header of the firmware upgrade file of Hi3861
		 *       therefore, no need to pass the CRC to hi3861 module
		 */
		DBG("[INFO] fw_upg_req_handler: FWUPG_H2D_INIT.\n");

		if (len != FWUPG_H2D_INIT_PAYLOAD_BYTES) {
			printf("[ERR] fw_upg_req_handler: payload len. of INIT error.\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}

		if (data == NULL) {
			printf("[ERR] fw_upg_req_handler: payload of INIT is null.\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}

		/* get new_fw_size */
		memcpy(&g_fw_upg_info.new_fw_len, data, NEW_FW_LEN_BYTES);
		if (g_fw_upg_info.new_fw_len <= 0) {
			printf("[ERR] fw_upg_req_handler: new_fw_size <= 0.\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}

		/** 
		 * get crc 
		 * NOTE: this is currently not used. 
		 */

		/* send command */
		agtx_cmd_send(CMD_FW_UPG, FWUPG_H2D_INIT, len, data);
#ifdef DEBUG
		ftime(&e_time);
		g_time[0] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
		ftime(&s_time);
#endif

		/* lock a semaphore: waiting for FWUPG_H2D_INIT_DONE */
		ret = sem_wait(&g_fw_upg_sem);
		if (ret != 0) {
			printf("[ERR] fw_upg_req_handler: fail to lock a semaphore.\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
#ifdef DEBUG
		ftime(&e_time);
		g_time[1] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
		ftime(&s_time);
#endif

		/* send request */
		agtx_req_send(CMD_FW_UPG, FWUPG_D2H_INIT_DONE, STATUS_PAYLOAD_BYTES, g_status);
#ifdef DEBUG
		ftime(&e_time);
		g_time[2] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
#endif

		DBG("[INFO] fw_upg_req_handler: send FWUPG_H2D_INIT done. new_fw_len: %d.\n", g_fw_upg_info.new_fw_len);
		break;
	case FWUPG_H2D_DOWNLOAD:
#ifdef DEBUG
		ftime(&s_time);
#endif
		/**
		 * check the length of payload and payload which contains:
		 * 1. uint8_t new_fw
		 */
		DBG("[INFO] fw_upg_req_handler: FWUPG_H2D_DOWNLOAD. new_fw_len: %d. len of payload: %d.\n",
		    g_fw_upg_info.new_fw_len, len);

		if (data == NULL) {
			printf("[ERR] fw_upg_req_handler: payload of DOWNLOAD is null.\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}

		/* send command */
		agtx_cmd_send(CMD_FW_UPG, FWUPG_H2D_DOWNLOAD, len, data);
		g_fw_upg_info.cum_fw_len += (uint32_t)len;
#ifdef DEBUG
		ftime(&e_time);
		g_time[4] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
		ftime(&s_time);
#endif
		/* lock a semaphore: waiting for FWUPG_D2H_VERIFY */
		ret = sem_wait(&g_fw_upg_sem);
		if (ret != 0) {
			printf("[ERR] fw_upg_req_handler: fail to lock a semaphore.\n");
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			break;
		}
#ifdef DEBUG
		ftime(&e_time);
		g_time[5] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
		ftime(&s_time);
#endif

		/* send request */
		agtx_req_send(CMD_FW_UPG, FWUPG_D2H_VERIFY, STATUS_PAYLOAD_BYTES, g_status);
		DBG("[INFO] fw_upg_req_handler: FWUPG_H2D_DOWNLOAD done. g_status = %d.\n", g_status[0]);
#ifdef DEBUG
		ftime(&e_time);
		g_time[6] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);
#endif
		break;
	case FWUPG_H2D_START:
#ifdef DEBUG
		ftime(&s_time);
#endif
		/* send command */
		DBG("[INFO] fw_upg_req_handler: FWUPG_H2D_START.\n");

		agtx_cmd_send(CMD_FW_UPG, FWUPG_H2D_START, len, data);

		DBG("[INFO] fw_upg_req_handler: send FWUPG_H2D_START done.\n");
#ifdef DEBUG
		ftime(&g_e_time);
		ftime(&e_time);
		g_time[8] += (e_time.time - s_time.time) * 1000 + (e_time.millitm - s_time.millitm);

		DBG("[INFO] upgrade finish. Using %ld ms.\n",
		    ((g_e_time.time - g_s_time.time) * 1000 + (g_e_time.millitm - g_s_time.millitm)));
		DBG("[INFO] send init: %ld ms; wait sem: %ld ms; send init_done: %ld ms; init_done: %ld ms.\n",
		    g_time[0], g_time[1], g_time[2], g_time[3]);
		DBG("[INFO] send download: %ld ms; wait sem: %ld ms; send verify: %ld ms; verify: %ld ms.\n", g_time[4],
		    g_time[5], g_time[6], g_time[7]);
		DBG("[INFO] start: %ld ms.\n");
#endif
		break;
	default:
		printf("[ERR] fw_upg_req_handler: unknown fw upg request. sub_type: %d.\n", sub_type);
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
