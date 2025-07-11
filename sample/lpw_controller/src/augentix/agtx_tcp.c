#include "agtx_lpw_cmd_common.h"

#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define lpw_tcp(fmt, ...) printf("[LPW_TCP]" fmt, ##__VA_ARGS__)
#define tcp_info(fmt, ...) lpw_tcp("[INFO] " fmt, ##__VA_ARGS__)
#define tcp_notice(fmt, ...) lpw_tcp("[NOTICE] " fmt, ##__VA_ARGS__)
#define tcp_warn(fmt, ...) lpw_tcp("[WARN] " fmt, ##__VA_ARGS__)
#define tcp_err(fmt, ...) lpw_tcp("[ERR] " fmt, ##__VA_ARGS__)

#define MEM_LEVEL_HI 8000
#define MEM_LEVEL_LO 4000

typedef struct recv_node {
	uint8_t *data;
	uint8_t *index;
	uint16_t size;
	uint16_t len;
	struct recv_node *next;
} recv_node;

typedef struct {
	recv_node *head;
	recv_node *end;
	int size;
} list;

typedef struct {
	uint8_t ip[4];
	uint16_t port;
} tcp_target;

enum { TCP_DISCONNECTED,
       TCP_CONNECTED,
};

/* external function */
extern int get_wifi_conn_status(void);

/* global variable */
static int g_suspend_flag = 0;
static int g_sem_flag;
static int g_tcp_status = TCP_DISCONNECTED;
static sem_t g_tcp_sem;
static list g_recv_list = { NULL, NULL, 0 };
static int g_send_bytes = 0;

int is_tcp_connected(void)
{
	return g_tcp_status;
}

void tcp_init(void)
{
	sem_init(&g_tcp_sem, 0, 0);
}

static int is_list_empty(list *li)
{
	if (li->head == NULL) {
		return 1;
	} else {
		return 0;
	}
}

/* enqueue node of list */
static int enqueue_node(list *li, uint8_t *data, uint16_t len)
{
	uint8_t *buff;
	recv_node *node;

	if (data == NULL || len == 0) {
		return -1;
	}

	/* Allocate data space */
	buff = (uint8_t *)malloc(len * sizeof(uint8_t));
	if (buff == NULL) {
		return -1;
	}
	memcpy(buff, data, len);

	node = (recv_node *)malloc(sizeof(recv_node));
	if (node == NULL) {
		free(buff);
		return -1;
	}
	/* Initialize node */
	node->next = NULL;
	node->data = buff;
	node->index = buff;
	node->size = node->len = len;

	if (li->head == NULL) {
		li->head = li->end = node;
	} else {
		/* Add new node to the end of list */
		li->end->next = node;
		/* point "end" to new node */
		li->end = node;
	}
	/* record total size of data in list */
	li->size += len;
	/* if total size if over than MEM_LEVEL_HI, notify module suspend receive */
	if (li->size > MEM_LEVEL_HI && g_suspend_flag == 0) {
		agtx_cmd_send(CMD_TCP, TCP_H2D_RECV_SUSPEND, 0, NULL);
		g_suspend_flag = 1;
		tcp_notice("TCP buffered %d bytes, suspend receive\n", li->size);
	}

	tcp_info("enqueue node successful\n");
	return 0;
}

/* dequeue node of list */
static int dequeue_node(list *li)
{
	recv_node *temp;

	if (li->head == NULL) {
		return -1;
	}

	/* free data space */
	free(li->head->data);
	/* move head to next node */
	temp = li->head;
	if (li->head != li->end) {
		/* If li->head is not the last node, move it to next node */
		li->head = li->head->next;
	} else {
		/* If li->head is last node, free li->head and reset to NULL */
		li->head = li->end = NULL;
	}
	/* recaculate total size of data in list */
	li->size -= temp->size;
	/* if total size if lower than MEM_LEVEL_LO, notify module resume receive */
	if (li->size < MEM_LEVEL_LO && g_suspend_flag == 1) {
		agtx_cmd_send(CMD_TCP, TCP_H2D_RECV_RESUME, 0, NULL);
		g_suspend_flag = 0;
		tcp_notice("TCP buffered %d bytes, resume receive\n", li->size);
	}

	/* free original head node */
	free(temp);
	tcp_info("dequeue node successful\n");
	return 0;
}

static uint16_t read_data(list *li, uint8_t *buff, uint16_t len)
{
	uint16_t left_len = len;
	uint16_t offset = 0;

	if (is_list_empty(li)) {
		return 0;
	}

	while (left_len > 0) {
		/* If request length is large than current node's remaining data length then dequeue node
		 * after read node's data */
		/* head->len is node's remaining data length which haven't been read */
		if (left_len >= li->head->len) {
			memcpy(buff + offset, li->head->index, li->head->len);
			offset += li->head->len;
			left_len -= li->head->len;
			/* dequeue node of list */
			dequeue_node(li);
			/* If list is empty, break while loop */
			if (is_list_empty(li)) {
				break;
			}
		} else {
			memcpy(buff + offset, li->head->index, left_len);
			offset += left_len;
			/* move index for next read_data() */
			li->head->index += left_len;
			/* update data length haven't been read */
			li->head->len -= left_len;
			/* achieve request length, set 0 to leave while loop */
			left_len = 0;
		}
	}
	return offset;
}

static int is_ip_valid(uint8_t *ip)
{
	if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
		return -1;
	}
	if (ip[0] == 255 && ip[1] == 255 && ip[2] == 255 && ip[3] == 255) {
		return -1;
	}
	return 0;
}

static int is_port_valid(uint16_t port)
{
	if (port == 0) {
		return -1;
	} else {
		return 0;
	}
}

void tcp_cmd_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	int ret;

	switch (sub_type) {
	case TCP_D2H_CONN_STATUS:
		g_tcp_status = *data;
		if (g_sem_flag == 1) {
			g_sem_flag = 0;
			sem_post(&g_tcp_sem);
		}
		/* If tcp disconnected, then dequeue all receive node in list */
		if (g_tcp_status == TCP_DISCONNECTED) {
			/* during cleaning buffered data, g_suspend_flag would be set to 0 also */
			while (dequeue_node(&g_recv_list) != -1)
				;
			tcp_notice("get tcp status is disconnected, clear tcp buffered data\n");
		}
		break;
	case TCP_D2H_RECV:
		ret = enqueue_node(&g_recv_list, data, len);
		if (ret != 0) {
			tcp_warn("enqueue node failed, RX TCP data dropped\n");
		}
		break;
	case TCP_D2H_SEND:
		g_send_bytes = *(int *)data;
		sem_post(&g_tcp_sem);
		break;
	}
}

void tcp_req_handler(uint8_t sub_type, uint16_t len, uint8_t *data)
{
	int recv_size;
	uint8_t *buff;
	uint16_t ret = 0;
	tcp_target *tg;

	switch (sub_type) {
	case TCP_H2D_CONN_TO:
		if (g_tcp_status == TCP_CONNECTED) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			tcp_warn("tcp is connected already, need to disconnect first\n");
		} else if (g_tcp_status == TCP_DISCONNECTED && get_wifi_conn_status() == 1) {
			tg = (tcp_target *)data;
			if (is_ip_valid(tg->ip) != 0) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				tcp_err("connect tcp fail, ip is invalid\n");
				return;
			}
			if (is_port_valid(tg->port) != 0) {
				agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
				tcp_err("connect tcp fail, port is invalid\n");
				return;
			}
			tcp_info("connect tcp\n");
			g_sem_flag = 1;
			agtx_cmd_send(CMD_TCP, TCP_H2D_CONN_TO, len, data);
			/* Wait for connection result */
			sem_wait(&g_tcp_sem);
			agtx_req_send(CMD_TCP, TCP_D2H_CONN_STATUS, 1, (uint8_t *)&g_tcp_status);
		} else {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			tcp_err("connect tcp fail, need to connect wifi first\n");
		}
		break;
	case TCP_H2D_TLS_CONN_TO:
		break;
	case TCP_H2D_DISCONN:
		if (g_tcp_status == TCP_CONNECTED) {
			tcp_info("disconnect tcp\n");
			agtx_cmd_send(CMD_TCP, TCP_H2D_DISCONN, 0, NULL);
		} else if (g_tcp_status == TCP_DISCONNECTED) {
			tcp_warn("tcp is disconnected already\n");
		}
		break;
	case TCP_H2D_SEND:
		if (g_tcp_status == TCP_CONNECTED) {
			tcp_info("send tcp data\n");
			g_sem_flag = 1;
			agtx_cmd_send(CMD_TCP, TCP_H2D_SEND, len, data);
			/* Wait for send bytes result */
			sem_wait(&g_tcp_sem);
			if (g_send_bytes != -EIO) {
				agtx_req_send(CMD_TCP, TCP_D2H_SEND, sizeof(g_send_bytes), (uint8_t *)&g_send_bytes);
			} else if (g_send_bytes == -EIO) {
				agtx_req_send(CMD_ERR, EIO, 0, NULL);
			}
		} else if (g_tcp_status == TCP_DISCONNECTED) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			tcp_err("tcp send fail, need to connect tcp first\n");
		}
		break;
	case TCP_H2D_RECV:
		if (g_tcp_status == TCP_CONNECTED) {
			if (is_list_empty(&g_recv_list)) {
				agtx_req_send(CMD_TCP, TCP_D2H_RECV, 0, NULL);
				tcp_warn("no data can read\n");
			} else {
				recv_size = *(int *)data;
				buff = (uint8_t *)malloc(recv_size * sizeof(uint8_t));
				if (buff == NULL) {
					agtx_req_send(CMD_ERR, ENOMEM, 0, NULL);
					tcp_err("out of memory, read data failed\n");
				} else {
					ret = read_data(&g_recv_list, buff, recv_size);
					agtx_req_send(CMD_TCP, TCP_D2H_RECV, ret, buff);
					/* Release space */
					free(buff);
				}
			}
		} else if (g_tcp_status == TCP_DISCONNECTED) {
			agtx_req_send(CMD_ERR, EINVAL, 0, NULL);
			tcp_err("tcp recv fail, need to connect tcp first\n");
		}
		break;
	}
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif
