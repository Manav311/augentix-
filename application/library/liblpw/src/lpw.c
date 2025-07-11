#include "agtx_lpw.h"

#include <pthread.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <assert.h>

#include "agtx_lpw_cmd_common.h"

#define SOCK_BUF_MAX 1500

#define SERVER_SOCK_FILE "/tmp/lpw_server.sock"
#define CLIENT_SOCK_FILE "/tmp/lpw_client_%d.sock"

#define lpw_log(fmt, ...) printf("[liblpw][%s]" fmt, __func__, ##__VA_ARGS__)

#define UNUSED(x) (void)(x) /**< Definition of unused. */

typedef struct {
	struct sockaddr_un addr;
	struct sockaddr_un server_addr;
	int sock;
	pthread_mutex_t mut;
	union { // no concurrent sending and receiving for now
		uint8_t buf_out[SOCK_BUF_MAX];
		uint8_t buf_in[SOCK_BUF_MAX];
	};
} lpw_private_t;

typedef struct {
	uint8_t cmd_type;
	uint8_t sub_type;
	uint16_t len;
	uint8_t payload[];
} lpw_message_t;

static void send_to_lpw_controller(lpw_private_t *lpw_pri, uint8_t cmd_type, uint8_t sub_type, uint16_t len,
                                   unsigned char *payload)
{
	if (len > SOCK_BUF_MAX - 4) {
		lpw_log("[ERR] unable to send data length of %d\n", len);
	}

	lpw_pri->buf_out[0] = cmd_type;
	lpw_pri->buf_out[1] = sub_type;
	lpw_pri->buf_out[2] = len & 0xFF;
	lpw_pri->buf_out[3] = (len >> 8) & 0xFF;

	memcpy(&lpw_pri->buf_out[4], payload, len);

	write(lpw_pri->sock, lpw_pri->buf_out, len + 4);
}

static lpw_message_t *recv_from_lpw_controller(lpw_private_t *lpw_pri)
{
	lpw_message_t *ret = (lpw_message_t *)lpw_pri->buf_in;
	if (read(lpw_pri->sock, lpw_pri->buf_in, sizeof(lpw_pri->buf_in)) < 0) {
		lpw_log("[ERR] unable to recv data\n");
		return NULL;
	}
	return ret;
}

lpw_handle lpw_open(void)
{
	lpw_private_t *lpw_pri = malloc(sizeof(lpw_private_t));
	if (lpw_pri == NULL) {
		lpw_log("[ERR] alloc fail\n");
		return (lpw_handle)NULL;
	}
	memset(lpw_pri, 0, sizeof(lpw_private_t));

	lpw_pri->sock = socket(AF_UNIX, SOCK_STREAM, 0);
	if (lpw_pri->sock == -1) {
		lpw_log("[ERR] create sock fail\n");
		free(lpw_pri);
		return (lpw_handle)NULL;
	}

	lpw_pri->server_addr.sun_family = AF_UNIX;
	strcpy(lpw_pri->server_addr.sun_path, SERVER_SOCK_FILE);

	if (connect(lpw_pri->sock, (const struct sockaddr *)&lpw_pri->server_addr, sizeof(struct sockaddr_un)) < 0) {
		// lpw_controller is not ready
		close(lpw_pri->sock);
		free(lpw_pri);
		return (lpw_handle)NULL;
	}

	pthread_mutex_init(&lpw_pri->mut, 0);

	return (lpw_handle)lpw_pri;
}

void lpw_close(lpw_handle hd)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;

	pthread_mutex_destroy(&lpw_pri->mut);

	unlink(lpw_pri->addr.sun_path);
	close(lpw_pri->sock);

	free(lpw_pri);
}

void lpw_module_get_version(lpw_handle hd, lpw_fw_ver_t *ver)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_INFO, INFO_H2D_GET_FW_VER, 0, NULL);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv && recv->cmd_type == CMD_INFO && recv->sub_type == INFO_D2H_FW_VER) {
		memcpy(ver, recv->payload, 3);
	} else {
		lpw_log("[ERR] get module FW version fail\n");
	}

	pthread_mutex_unlock(&lpw_pri->mut);
}

int lpw_wifi_get_status(lpw_handle hd, lpw_wifi_state_t *st)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WIFI, WIFI_H2D_GET_STATUS, 0, NULL);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_WIFI) {
		memcpy(st, recv->payload, sizeof(lpw_wifi_state_t));
		ret = 0;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_wifi_set_sta_mode(lpw_handle hd)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int size = 1;
	unsigned char payload[size];

	payload[0] = 0;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WIFI, WIFI_H2D_SET_MODE, size, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_wifi_scan(lpw_handle hd, lpw_wifi_scan_t *scan, lpw_wifi_scan_result_t *result)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int ssid_len = (scan->ssid == NULL) ? 1 : (strlen(scan->ssid) + 1);
	int size = 8 + ssid_len;
	unsigned char payload[size];

	memset(payload, 0, size);
	memcpy(payload, scan->bssid, WIFI_MAC_LEN);
	payload[6] = scan->channel;
	payload[7] = scan->scan_type;
	if (scan->ssid != NULL) {
		memcpy(payload + 8, scan->ssid, ssid_len);
	} else {
		payload[size - 1] = '\0';
	}

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WIFI, WIFI_H2D_SCAN, size, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_WIFI) {
		memcpy(result, recv->payload, sizeof(lpw_wifi_scan_result_t));
		if (result->num == 0) {
			/* ret = -ENODEV means taget doesn't  exist */
			ret = -ENODEV;
		} else {
			/* ret = 0 means target exist */
			ret = 0;
		}
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_wifi_connect_to(lpw_handle hd, lpw_wifi_conn_t *conn)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int ssid_len = (conn->ssid == NULL) ? 1 : (strlen(conn->ssid) + 1);
	int key_len = (conn->key == NULL) ? 1 : (strlen(conn->key) + 1);
	int size = 8 + ssid_len + key_len;
	unsigned char payload[size];

	memset(payload, 0, size);
	memcpy(payload, conn->bssid, WIFI_MAC_LEN);
	payload[6] = conn->auth;
	payload[7] = conn->pairwise;
	if (conn->ssid != NULL) {
		memcpy(payload + 8, conn->ssid, ssid_len);
	} else {
		payload[8 + ssid_len] = '\0';
	}
	if (conn->key != NULL) {
		memcpy(payload + 8 + ssid_len, conn->key, key_len);
	} else {
		payload[8 + ssid_len + key_len] = '\0';
	}
	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WIFI, WIFI_H2D_CONN_AP, size, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_WIFI) {
		ret = recv->payload[0];
		if (ret == 1) {
			/* ret = 0 means wifi is connected */
			ret = 0;
		} else if (ret == 0) {
			/* ret = -EACCES means wifi is unconnected */
			ret = -EACCES;
		}
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_wifi_set_ap_mode(lpw_handle hd, lpw_wifi_ap_config_t *config)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int ssid_len = (config->ssid == NULL) ? 1 : (strlen(config->ssid) + 1);
	int key_len = (config->key == NULL) ? 1 : (strlen(config->key) + 1);
	int size = 1 + 3 + ssid_len + key_len;
	unsigned char payload[size];

	memset(payload, 0, size);
	/* Set AP */
	payload[0] = 1;
	payload[1] = config->channel_num;
	payload[2] = config->authmode;
	payload[3] = config->pairwise;
	memcpy(payload + 4, config->ssid, ssid_len);
	memcpy(payload + 4 + ssid_len, config->key, key_len);

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WIFI, WIFI_H2D_SET_MODE, size, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_gpio_set_dir(lpw_handle hd, int gpio_id, int dir, int out)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 2;
	unsigned char payload[len];

	assert(out == 0 || out == 1);
	payload[0] = (unsigned char)gpio_id;
	if (dir == 0) {
		payload[1] = (unsigned char)0;
	} else {
		payload[1] = (unsigned char)(1 + out);
	}

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_PERIPHERAL, PERI_H2D_GPIO_SET_DIR, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	//if get failed, is it possible that recv !=NULL && recv->cmd_type != (CMD_ERR & CMD_PERI)?
	return ret;
}

int lpw_gpio_get_dir(lpw_handle hd, int gpio_id)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 1;
	unsigned char payload[len];

	payload[0] = (unsigned char)gpio_id;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_PERIPHERAL, PERI_H2D_GPIO_GET_DIR, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_PERIPHERAL && recv->sub_type == PERI_D2H_GPIO_DIR) {
		ret = recv->payload[1];
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_gpio_set_output(lpw_handle hd, int gpio_id, int out)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 2;
	unsigned char payload[len];

	assert(out == 0 || out == 1);
	payload[0] = (unsigned char)gpio_id;
	payload[1] = (unsigned char)out;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_PERIPHERAL, PERI_H2D_GPIO_SET_OUTPUT, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_gpio_get_input(lpw_handle hd, int gpio_id)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 1;
	unsigned char payload[len];

	payload[0] = (unsigned char)gpio_id;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_PERIPHERAL, PERI_H2D_GPIO_GET_INPUT, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_PERIPHERAL && recv->sub_type == PERI_D2H_GPIO_INPUT && recv->payload[1] < 2) {
		ret = recv->payload[1];
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_adc_get(lpw_handle hd, int adc_ch)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret = 0;
	int len = 2;
	unsigned char payload[len];

	pthread_mutex_lock(&lpw_pri->mut);

	memcpy(payload, &adc_ch, 2);
	send_to_lpw_controller(lpw_pri, CMD_PERIPHERAL, PERI_H2D_ADC_GET, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_PERIPHERAL && recv->sub_type == PERI_D2H_ADC_VAL) {
		memcpy(&ret, &(recv->payload[2]), 2);
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_fw_upg(lpw_handle hd, unsigned char *new_fw)
{
#define FWUPG_H2D_INIT_PAYLOAD_BYTES 8
#define STATUS_PAYLOAD_BYTES 1
#define STATUS_FAIL 0
#define STATUS_SUCCESS 1
//#define PRINT_PROGRESS_BAR
#ifdef PRINT_PROGRESS_BAR
#define PROGRESS_INTERVAL 100
#endif

	/* initialize variable */
	int32_t ret = 0;
	uint32_t new_fw_len = 0;
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	uint8_t status[STATUS_PAYLOAD_BYTES] = { 0 };
	unsigned char init_payload[FWUPG_H2D_INIT_PAYLOAD_BYTES] = { 0 };
	unsigned char download_slice[PAYLOAD_MAX_LEN] = { 0 };
	FILE *fptr = NULL;
#ifdef PRINT_PROGRESS_BAR
	int32_t percentage = 0;
	char buf[102] = { 0 };
#endif

	/* lock using mutex */
	pthread_mutex_lock(&lpw_pri->mut);

	/* read firmware file length */
	if (new_fw == NULL) {
		lpw_log("[ERR] the path of the new firmware file is null.\n");
		return -ENOENT;
	}

	fptr = fopen((const char *)new_fw, "rb");
	if (fptr == NULL) {
		lpw_log("[ERR] fails to open the new firmware file.\n");
		return -ENOENT;
	}

	if (fseek(fptr, 0L, SEEK_END) != 0) {
		lpw_log("[ERR] fails to seek the end of the file.\n");
		return -EIO;
	}

	new_fw_len = ftell(fptr);
	if (new_fw_len <= 0) {
		lpw_log("[ERR] fails to get the size of the file.\n");
		return -EIO;
	}

	fseek(fptr, 0L, SEEK_SET);

	/**
	 * send initialize request to lpw_controller with payload:
	 * 
	 * 1. uint32_t new_fw_size
	 * 2. uint32_t crc
	 *    NOTE: currently, CRC is stored at the header of the firmware upgrade file of Hi3861
	 *          therefore, no need to pass the CRC via hichannel
	 */

	/* transfer uint32 new_fw_len into uint8 array */
	init_payload[0] = ((uint8_t)new_fw_len) & 0xff;
	init_payload[1] = ((uint8_t)(new_fw_len >> 8)) & 0xff;
	init_payload[2] = ((uint8_t)(new_fw_len >> 16)) & 0xff;
	init_payload[3] = ((uint8_t)(new_fw_len >> 24)) & 0xff;

	send_to_lpw_controller(lpw_pri, CMD_FW_UPG, FWUPG_H2D_INIT, FWUPG_H2D_INIT_PAYLOAD_BYTES, init_payload);

	/* receive INIT status */
	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		lpw_log("[ERR] receive null.\n");
		return -EBUSY;
	} else if (recv->cmd_type == CMD_FW_UPG && recv->sub_type == FWUPG_D2H_INIT_DONE) {
		memcpy(status, recv->payload, STATUS_PAYLOAD_BYTES);
		if (status[0] == STATUS_FAIL) {
			lpw_log("[ERR] init fail.\n");
			return -EILSEQ;
		} else if (status[0] == STATUS_SUCCESS) {
			lpw_log("[INFO] init success.\n");
		} else {
			lpw_log("[ERR] unknown init result.\n");
			return -EILSEQ;
		}
	} else if (recv->cmd_type == CMD_ERR) {
		lpw_log("[ERR] receive CMD_ERR.\n");
		return -recv->sub_type;
	} else {
		lpw_log("[ERR] receive unknown command.\n");
		return -EPIPE;
	}

	/**
	 * send download fw request to lpw_controller with payload:
	 * 
	 * 1. uint8_t new_fw
	 * NOTE: first package length must exceed 96 bytes
	 */
	for (uint32_t i = 0, slice_len = PAYLOAD_MAX_LEN; i < new_fw_len;) {
		/* update slice length */
		if ((i + PAYLOAD_MAX_LEN) > new_fw_len) {
			slice_len = new_fw_len - i;
		}

		if ((unsigned int)fread(download_slice, sizeof(unsigned char), slice_len, fptr) != slice_len) {
			lpw_log("[ERR] fails to read the slice of file.\n");
			fclose(fptr);
			return -EPERM;
		}

		/* send DOWNLOAD command */
		send_to_lpw_controller(lpw_pri, CMD_FW_UPG, FWUPG_H2D_DOWNLOAD, slice_len, download_slice);
		if (memset(download_slice, 0, PAYLOAD_MAX_LEN) == NULL) {
			lpw_log("[ERR] memset for download_sub_payload fail.\n");
			return -EPERM;
		}

		/* receive VERIFY status */
		recv = recv_from_lpw_controller(lpw_pri);
		if (recv == NULL) {
			lpw_log("[ERR] receive null.\n");
			return -EBUSY;
		} else if (recv->cmd_type == CMD_FW_UPG && recv->sub_type == FWUPG_D2H_VERIFY) {
			memcpy(status, recv->payload, STATUS_PAYLOAD_BYTES);
			if (status[0] == STATUS_FAIL) {
				lpw_log("[ERR] verify firmware fail.\n");
				return -EILSEQ;
			} else if (status[0] == STATUS_SUCCESS) {
				i += slice_len;
			} else {
				lpw_log("[ERR] unknown verify result.\n");
				return -EILSEQ;
			}
		} else if (recv->cmd_type == CMD_ERR) {
			lpw_log("[ERR] receive CMD_ERR.\n");
			return -recv->sub_type;
		} else {
			lpw_log("[ERR] receive unknown command. recv: %x.\n", recv->cmd_type);
			return -EPIPE;
		}

#ifdef PRINT_PROGRESS_BAR
		/* print progress bar */
		if ((i % PROGRESS_INTERVAL) == 0 || i == new_fw_len) {
			percentage = (i * 100) / new_fw_len;
			buf[percentage] = '=';
			printf("Download Progress [%-101s][%d%%]", buf, percentage);
			if (i != new_fw_len) {
				printf("\r");
			} else {
				printf("\n");
			}
			fflush(stdout);
		}
#endif
	}
	fclose(fptr);

	/* send start fw upgrade request to lpw_controller with no payload */
	send_to_lpw_controller(lpw_pri, CMD_FW_UPG, FWUPG_H2D_START, 0, NULL);

	/* unlock the mutex */
	pthread_mutex_unlock(&lpw_pri->mut);


	return ret;
}

int lpw_tcp_connect_to(lpw_handle hd, in_addr_t ip, in_port_t port)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	in_port_t port_ns = htons(port);
	int size = sizeof(in_addr_t) + sizeof(in_port_t);
	unsigned char payload[size];

	memcpy(payload, &ip, sizeof(in_addr_t));
	memcpy(payload + sizeof(in_addr_t), &port_ns, sizeof(in_port_t));

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_TCP, TCP_H2D_CONN_TO, size, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_TCP) {
		ret = recv->payload[0];
		if (ret == 1) {
			/* ret = 0 means tcp is connected */
			ret = 0;
		} else if (ret == 0) {
			/* ret = -EACCES means tcp is unconnected */
			ret = -EACCES;
		}
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

void lpw_tcp_disconnect(lpw_handle hd)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_TCP, TCP_H2D_DISCONN, 0, NULL);

	pthread_mutex_unlock(&lpw_pri->mut);
}

int lpw_tcp_send(lpw_handle hd, unsigned char *data, int len)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret = 0;
	int i;
	int send_bytes = 0;
	int packet = len / PAYLOAD_MAX_LEN;
	packet = packet + ((len % PAYLOAD_MAX_LEN) > 0 ? 1 : 0);

	assert(data);

	pthread_mutex_lock(&lpw_pri->mut);

	for (i = 0; i < packet; i++) {
		if (i < packet - 1) {
			send_to_lpw_controller(lpw_pri, CMD_TCP, TCP_H2D_SEND, PAYLOAD_MAX_LEN, data);
		} else {
			send_to_lpw_controller(lpw_pri, CMD_TCP, TCP_H2D_SEND, len % PAYLOAD_MAX_LEN, data);
		}

		recv = recv_from_lpw_controller(lpw_pri);

		if (recv == NULL) {
			ret = -EBUSY;
			break;
		} else if (recv->cmd_type == CMD_ERR) {
			ret = -recv->sub_type;
			break;
		} else if (recv->cmd_type == CMD_TCP) {
			memcpy(&send_bytes, recv->payload, 4);
			ret += send_bytes;
		} else {
			ret = -EPIPE;
			break;
		}
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_tcp_recv(lpw_handle hd, unsigned char *buf, int len)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;

	assert(buf);

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_TCP, TCP_H2D_RECV, sizeof(int), (unsigned char *)&len);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else if (recv->cmd_type == CMD_TCP) {
		memcpy(buf, recv->payload, recv->len);
		ret = recv->len;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_pm_init(lpw_handle hd, int gpio_id, int active_level, int adc_ch, int off, int on)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 8;
	unsigned char payload[len];
	uint8_t id = gpio_id;
	uint8_t level = active_level;
	uint16_t ch = adc_ch;
	uint16_t off_thre = off;
	uint16_t on_thre = on;

	memset(payload, 0, len);

	memcpy(payload, &id, 1);
	memcpy(payload + 1, &level, 1);
	memcpy(payload + 2, &ch, 2);
	memcpy(payload + 4, &off_thre, 2);
	memcpy(payload + 6, &on_thre, 2);

	pthread_mutex_lock(&lpw_pri->mut);
	//WAKE_H2D_SOURCE use 2 bytes
	send_to_lpw_controller(lpw_pri, CMD_WAKE_EVENT, WAKE_H2D_PM, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_sleep_detect_gpio_en(lpw_handle hd, int gpio_id, gpio_event_t event)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 2;
	unsigned char payload[len];

	assert(event > 0);
	payload[0] = (unsigned char)gpio_id;
	payload[1] = (unsigned char)event;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WAKE_EVENT, WAKE_H2D_GPIO, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_sleep_detect_adc_en(lpw_handle hd, int adc_ch, int threshold, int above_under)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 6;
	unsigned char payload[len];
	uint16_t ch = adc_ch;
	uint16_t thre = threshold;
	uint16_t ab_un = above_under;

	assert(adc_ch >= 0);
	assert(threshold >= 0);
	assert(above_under == 0 || above_under == 1);

	memcpy(payload, &ch, sizeof(ch));
	memcpy(payload + sizeof(ch), &ab_un, sizeof(ab_un));
	memcpy(payload + sizeof(ch) + sizeof(ab_un), &thre, sizeof(thre));

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WAKE_EVENT, WAKE_H2D_ADC, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_sleep_tcp_wake_en(lpw_handle hd, tcp_wake_t *pattern)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret;
	int len = 8 + pattern->keep_alive_pack_len + pattern->wake_pack_len; //7 for pervious struct member
	unsigned char payload[len];

	pthread_mutex_lock(&lpw_pri->mut);

	memcpy(payload, &pattern->keep_alive_pack_len, 2);
	memcpy(payload + 2, &pattern->keep_alive_period, 2);
	memcpy(payload + 4, &pattern->wake_pack_len, 2);
	memcpy(payload + 6, &pattern->bad_conn_handle, 2);
	memcpy(payload + 8, pattern->keep_alive_pack, pattern->keep_alive_pack_len);
	memcpy(payload + 8 + pattern->keep_alive_pack_len, pattern->wake_pack, pattern->wake_pack_len);

	send_to_lpw_controller(lpw_pri, CMD_WAKE_EVENT, WAKE_H2D_TCP, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_sleep_start(lpw_handle hd, int timeout, int response)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret = 0;
	int len = 8;
	unsigned char payload[len];
	uint32_t time = timeout;
	uint32_t res = response;

	pthread_mutex_lock(&lpw_pri->mut);

	assert(timeout >= 0);
	assert(response > 0);
	memcpy(payload, &time, sizeof(time));
	memcpy(payload + 4, &res, sizeof(res));

	send_to_lpw_controller(lpw_pri, CMD_WAKE_EVENT, WAKE_H2D_SLEEP_START, len, payload);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_ERR) {
		ret = -recv->sub_type;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}

int lpw_sleep_get_wake_event(lpw_handle hd, wake_event_t *event)
{
	lpw_private_t *lpw_pri = (lpw_private_t *)hd;
	lpw_message_t *recv;
	int ret = 0;

	pthread_mutex_lock(&lpw_pri->mut);

	send_to_lpw_controller(lpw_pri, CMD_WAKE_EVENT, WAKE_H2D_GET_WAKE_BY, 0, NULL);

	recv = recv_from_lpw_controller(lpw_pri);
	if (recv == NULL) {
		ret = -EBUSY;
	} else if (recv->cmd_type == CMD_WAKE_EVENT && recv->sub_type == WAKE_D2H_WAKE_BY) {
		memcpy(event, recv->payload, 8);
		ret = 0;
	} else {
		ret = -EPIPE;
	}

	pthread_mutex_unlock(&lpw_pri->mut);

	return ret;
}
