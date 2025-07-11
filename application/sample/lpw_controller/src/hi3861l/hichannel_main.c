/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: sample cli file.
 * Author: Hisilicon
 * Create: 2020-09-09
 */
/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <linux/sockios.h>
#include <linux/if.h>
#include <linux/wireless.h>

#include "hi_base.h"
#include "hichannel_host.h"
#include "hichannel_host_comm.h"
#include "augentix/agtx_handle.h"

/*****************************************************************************
  2 宏定义、全局变量
*****************************************************************************/
static hi_s32 sample_exit_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg);
static hi_s32 sample_help_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg);
static hi_s32 raw_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg);
static hi_s32 req_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg);

static const sample_cmd_entry_stru g_sample_cmd[] = {
	{ "help", sample_help_cmd_process },
	{ "exit", sample_exit_cmd_process },
	{ "raw", raw_cmd_process },
	{ "req", req_cmd_process },
};

#define MAX_SOCK_CLIENT 5
#define MAX_CMD_LEN 20
#define MAX_IPV4_LEN 16
#define WIFI_MAC_LEN 6
#define SLEEP_TIME 10
#define SAMPLE_CMD_NUM (sizeof(g_sample_cmd) / sizeof(g_sample_cmd[0]))

#define SERVER_SOCK_DBG_FILE "/tmp/lpw_server_dbg.sock"
#define SERVER_SOCK_FILE "/tmp/lpw_server.sock"
/*****************************************************************************
  3 枚举、结构体定义
*****************************************************************************/
typedef enum {
	/* commands */
	SAMPLE_CMD_NONE,
	SAMPLE_CMD_HELP,
	SAMPLE_CMD_EXIT,
	SAMPLE_CMD_RAW,
	SAMPLE_CMD_REQ,
} sample_cmd_e;

/* command/event information */
typedef struct {
	sample_cmd_e what;
	hi_u32 len;
	hi_u8 obj[CMD_MAX_LEN];
	int req_sock;
	struct sockaddr_un req_addr;
} sample_message_s;

struct snode {
	sample_message_s message;
	struct snode *next;
};

struct squeue {
	struct snode *front;
	struct snode *rear;
};

typedef struct {
	pthread_mutex_t mut;
	pthread_cond_t cond;
	struct squeue cmd_queue;
	pthread_t sock_thread;
	hi_s32 sockfd_listen;
	hi_s32 sockfd_cli[MAX_SOCK_CLIENT];
#ifdef DEBUG
	pthread_t sock_thread_dbg;
	hi_s32 sockfd_dbg_listen;
	hi_s32 sockfd_dbg_cli[MAX_SOCK_CLIENT];
#endif
} sample_link_s;

static hi_bool g_terminate = HI_FALSE;
static sample_link_s *g_sample_link = HI_NULL;
// static hi_char host_cmd[][MAX_CMD_LEN] = { "cmd_get_mac", "cmd_get_ip", "cmd_set_filter" };
/*****************************************************************************
  4 函数实现
*****************************************************************************/
static hi_void sample_usage(hi_void)
{
	printf("\nUsage:\n");
	printf("\tsample_cli  quit          quit sample_ap\n");
	printf("\tsample_cli  help          show this message\n");
	printf("\tsample_cli  raw,00,11...          send raw package to module\n");
	printf("\tsample_cli  req,00,11...          send raw package to req_handler\n");
}

hi_s32 sample_str_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg)
{
	sample_unused(wdata);
	sample_message_s *msg = (sample_message_s *)pmsg;
	msg->what = SAMPLE_CMD_HELP;
	msg->len = len;
	memcpy(msg->obj, param, len);
	msg->obj[CMD_MAX_LEN - 1] = '\0';
	return HI_SUCCESS;
}

hi_s32 sample_help_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg)
{
	sample_unused(wdata);
	sample_unused(param);
	sample_unused(len);
	sample_message_s *msg = (sample_message_s *)pmsg;
	msg->what = SAMPLE_CMD_HELP;
	return HI_SUCCESS;
}

hi_s32 raw_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg)
{
	sample_unused(wdata);
	sample_unused(len);
	sample_message_s *msg = (sample_message_s *)pmsg;
	msg->what = SAMPLE_CMD_RAW;
	hi_char *tmp = param;
	hi_char *data = (hi_char *)msg->obj;
	hi_u32 data_len = 0;
	hi_u32 i;

	do {
		if (*tmp == ',')
			tmp++;
		data[data_len] = strtol(tmp, &tmp, 16);
		data_len++;
	} while (*tmp == ',');
	msg->len = data_len;

	printf("raw cmd send [");
	for (i = 0; i < data_len; i++) {
		printf("%02x ", data[i]);
	}
	printf("]\n");

	return HI_SUCCESS;
}

hi_s32 req_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg)
{
	sample_unused(wdata);
	sample_unused(len);
	sample_message_s *msg = (sample_message_s *)pmsg;
	msg->what = SAMPLE_CMD_REQ;
	hi_char *tmp = param;
	hi_char *data = (hi_char *)msg->obj;
	hi_u32 data_len = 0;

	do {
		if (*tmp == ',')
			tmp++;
		data[data_len] = strtol(tmp, &tmp, 16);
		data_len++;
	} while (*tmp == ',');
	msg->len = data_len;

	return HI_SUCCESS;
}

static hi_s32 sample_exit_cmd_process(hi_void *wdata, hi_char *param, hi_u32 len, hi_void *pmsg)
{
	sample_unused(wdata);
	sample_unused(param);
	sample_unused(len);
	sample_message_s *msg = (sample_message_s *)pmsg;
	msg->what = SAMPLE_CMD_EXIT;
	return HI_SUCCESS;
}

static hi_void sample_cleanup(hi_void)
{
#ifdef DEBUG
	if (g_sample_link->sock_thread_dbg) {
		pthread_cancel(g_sample_link->sock_thread_dbg);
		pthread_join(g_sample_link->sock_thread_dbg, HI_NULL);
	}
#endif

	pthread_mutex_destroy(&g_sample_link->mut);
	pthread_cond_destroy(&g_sample_link->cond);

#ifdef DEBUG
	if (g_sample_link->sockfd_dbg_listen != -1) {
		close(g_sample_link->sockfd_dbg_listen);
		unlink(SERVER_SOCK_DBG_FILE);
	}
#endif

	if (g_sample_link->sockfd_listen != -1) {
		close(g_sample_link->sockfd_listen);
		unlink(SERVER_SOCK_FILE);
	}

	if (g_sample_link != HI_NULL) {
		free(g_sample_link);
		g_sample_link = HI_NULL;
	}
}

static hi_void sample_terminate(hi_s32 sig)
{
	sample_unused(sig);
	sample_cleanup();
	g_terminate = HI_TRUE;
	_exit(0);
}

static hi_void sample_power(hi_s32 sig)
{
	sample_unused(sig);
}

static hi_s32 sample_wlan_init_up(hi_void)
{
	hi_s32 ret;
	hi_char cmd[SYSTEM_CMD_SIZE] = { 0 };

	memset(cmd, 0, SYSTEM_CMD_SIZE);
	if (snprintf(cmd, SYSTEM_CMD_SIZE - 1, "ifconfig %s up", SYSTEM_NETDEV_NAME) == -1) {
		sample_log_print("snprintf_s fail\n");
		return HI_FAILURE;
	}
	ret = system(cmd);
	if (ret == -1) {
		sample_log_print("%s up error\n", SYSTEM_NETDEV_NAME);
		return HI_FAILURE;
	}

	sample_log_print("net device up success\n");
	return HI_SUCCESS;
}

static hi_void set_lo_ipaddr(hi_void)
{
	hi_s32 results;
	hi_char cmd[SYSTEM_CMD_SIZE] = { 0 }; /* system Temporary variables */
	hi_char *spawn_args[] = { "ifconfig", "lo", "127.0.0.1", HI_NULL };

	results = sprintf(cmd, "%s %s %s", spawn_args[0], /* spawn_args[0]:ifconfig */
	                  spawn_args[1], spawn_args[2]); /* spawn_args[1]:lo,spawn_args[2]:ipaddr */
	if (results < 0) {
		sample_log_print("SAMPLE_STA: set lo ipaddr sprintf_s err!\n");
		return;
	}

	results = system(cmd);
}

static hi_s32 sample_enqueue(struct squeue *pqueue, const sample_message_s *element)
{
	struct snode *pnew = HI_NULL;

	if (pqueue == HI_NULL || element == HI_NULL) {
		return -1;
	}
	/* Create a new node */
	pnew = malloc(sizeof(struct snode));
	if (pnew == HI_NULL) {
		return -1;
	}

	pnew->message = *element;
	pnew->next = HI_NULL;

	if (pqueue->rear == HI_NULL) {
		/* queue is empty, set front and rear points to new node */
		pqueue->front = pqueue->rear = pnew;
	} else {
		/* queue is not empty, set rear points to the new node */
		pqueue->rear = pqueue->rear->next = pnew;
	}

	return HI_SUCCESS;
}

static hi_s32 sample_dequeue(struct squeue *pqueue, sample_message_s *element)
{
	struct snode *p = HI_NULL;

	if (pqueue == HI_NULL || element == HI_NULL) {
		return HI_FAILURE;
	}

	if (pqueue->front == HI_NULL) {
		return HI_FAILURE;
	}

	*element = pqueue->front->message;
	p = pqueue->front;
	pqueue->front = p->next;
	/* if the queue is empty, set rear = NULL */
	if (pqueue->front == HI_NULL) {
		pqueue->rear = HI_NULL;
	}

	free(p);
	return HI_SUCCESS;
}

#ifdef DEBUG
static hi_void *sample_sock_thread_dbg(hi_void *args)
{
	hi_char link_buf[SOCK_BUF_MAX];
	fd_set readfds;
	ssize_t recvbytes;
	int max_sd;
	int i;
	int active_count;
	hi_s32 sock_listen;
	hi_s32 *sock_cli;
	hi_s32 new_sock;
	sample_message_s message;
	struct sockaddr_un addr;
	socklen_t addrlen = sizeof(addr);
	sample_unused(args);

	memset(&message, 0, sizeof(message));

	sock_cli = g_sample_link->sockfd_dbg_cli;

	sock_listen = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_listen < 0) {
		goto ERR;
	}
	g_sample_link->sockfd_dbg_listen = sock_listen;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SERVER_SOCK_DBG_FILE);
	unlink(SERVER_SOCK_DBG_FILE);

	if (bind(sock_listen, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		goto ERR;
	}

	if (listen(sock_listen, MAX_SOCK_CLIENT) < 0) {
		goto ERR;
	}

	while (1) {
		FD_ZERO(&readfds);

		FD_SET(sock_listen, &readfds);

		max_sd = sock_listen;

		active_count = 0;

		for (i = 0; i < MAX_SOCK_CLIENT; i++) {
			if (sock_cli[i] > 0) {
				active_count++;
				FD_SET(sock_cli[i], &readfds);

				if (sock_cli[i] > max_sd)
					max_sd = sock_cli[i];
			}
		}

		if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
			goto ERR;
		}

		if (FD_ISSET(sock_listen, &readfds) && active_count < MAX_SOCK_CLIENT) {
			if ((new_sock = accept(sock_listen, (struct sockaddr *)&addr, (socklen_t *)&addrlen)) < 0) {
				goto ERR;
			}
			for (i = 0; i < MAX_SOCK_CLIENT; i++) {
				if (sock_cli[i] == 0) {
					sock_cli[i] = new_sock;
					break;
				}
			}
		}

		for (i = 0; i < MAX_SOCK_CLIENT; i++) {
			if (FD_ISSET(sock_cli[i], &readfds)) {
				recvbytes = read(sock_cli[i], link_buf, sizeof(link_buf));
				if (recvbytes <= 0) {
					close(sock_cli[i]);
					sock_cli[i] = 0;
				} else {
					if (sample_sock_cmd_entry(g_sample_link, link_buf, recvbytes,
					                          (hi_void *)&message) != HI_SUCCESS) {
						sample_log_print("sample_str_cmd_process entry\n");
						sample_str_cmd_process(g_sample_link, link_buf, recvbytes,
						                       (hi_void *)&message);
					}

					if (write(sock_cli[i], "OK", strlen("OK")) == -1) {
						sample_log_print("send error!fd:%d\n", sock_cli[i]);
					}

					pthread_mutex_lock(&g_sample_link->mut);
					if (sample_enqueue(&g_sample_link->cmd_queue, &message) == HI_SUCCESS) {
						pthread_cond_signal(&g_sample_link->cond);
					}
					pthread_mutex_unlock(&g_sample_link->mut);
				}
			}
		}
	}
	return (hi_void *)HI_SUCCESS;

ERR:
	sample_log_print("error:%s", strerror(errno));
	return (hi_void *)HI_FAILURE;
}
#endif

static hi_void *sample_sock_thread(hi_void *args)
{
	hi_char link_buf[SOCK_BUF_MAX];
	fd_set readfds;
	ssize_t recvbytes;
	int max_sd;
	int i;
	int active_count;
	hi_s32 sock_listen;
	hi_s32 *sock_cli;
	hi_s32 new_sock;
	sample_message_s message;
	struct sockaddr_un addr;
	socklen_t addrlen = sizeof(addr);
	sample_unused(args);

	memset(&message, 0, sizeof(message));

	sock_cli = g_sample_link->sockfd_cli;

	sock_listen = socket(AF_UNIX, SOCK_STREAM, 0);
	if (sock_listen < 0) {
		goto ERR;
	}
	g_sample_link->sockfd_listen = sock_listen;

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, SERVER_SOCK_FILE);
	unlink(SERVER_SOCK_DBG_FILE);

	if (bind(sock_listen, (const struct sockaddr *)&addr, sizeof(addr)) < 0) {
		goto ERR;
	}

	if (listen(sock_listen, MAX_SOCK_CLIENT) < 0) {
		goto ERR;
	}

	while (1) {
		FD_ZERO(&readfds);

		FD_SET(sock_listen, &readfds);

		max_sd = sock_listen;

		active_count = 0;

		for (i = 0; i < MAX_SOCK_CLIENT; i++) {
			if (sock_cli[i] > 0) {
				active_count++;
				FD_SET(sock_cli[i], &readfds);

				if (sock_cli[i] > max_sd)
					max_sd = sock_cli[i];
			}
		}

		if (select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
			goto ERR;
		}

		if (FD_ISSET(sock_listen, &readfds) && active_count < MAX_SOCK_CLIENT) {
			if ((new_sock = accept(sock_listen, (struct sockaddr *)&addr, (socklen_t *)&addrlen)) < 0) {
				goto ERR;
			}
			for (i = 0; i < MAX_SOCK_CLIENT; i++) {
				if (sock_cli[i] == 0) {
					sock_cli[i] = new_sock;
					break;
				}
			}
		}

		for (i = 0; i < MAX_SOCK_CLIENT; i++) {
			if (FD_ISSET(sock_cli[i], &readfds)) {
				recvbytes = read(sock_cli[i], link_buf, sizeof(link_buf));
				if (recvbytes <= 0) {
					close(sock_cli[i]);
					sock_cli[i] = 0;
				} else {
					message.what = SAMPLE_CMD_REQ;
					memcpy(message.obj, link_buf, recvbytes);
					message.len = recvbytes;
					message.req_sock = sock_cli[i];

					pthread_mutex_lock(&g_sample_link->mut);
					if (sample_enqueue(&g_sample_link->cmd_queue, &message) == HI_SUCCESS) {
						pthread_cond_signal(&g_sample_link->cond);
					}
					pthread_mutex_unlock(&g_sample_link->mut);
				}
			}
		}
	}
	return (hi_void *)HI_SUCCESS;

ERR:
	sample_log_print("error:%s", strerror(errno));
	return (hi_void *)HI_FAILURE;
}

void main_process(void)
{
	/* main loop */
	while (!g_terminate) {
		sample_message_s message;
		pthread_mutex_lock(&g_sample_link->mut);
		while (sample_dequeue(&g_sample_link->cmd_queue, &message) != HI_SUCCESS) {
			pthread_cond_wait(&g_sample_link->cond, &g_sample_link->mut);
		}
		pthread_mutex_unlock(&g_sample_link->mut);

		fflush(stdout);

		switch (message.what) {
		case SAMPLE_CMD_HELP:
			sample_usage();
			break;
		case SAMPLE_CMD_EXIT:
			g_terminate = HI_TRUE;
			break;
		case SAMPLE_CMD_RAW:
			if (hi_channel_send_to_dev(message.obj, message.len) != HI_SUCCESS) {
				sample_log_print("sample_iwpriv_cmd send fail\n");
			}
			break;
		case SAMPLE_CMD_REQ:
			agtx_req_handler(message.obj, message.len, message.req_sock);
			break;
		default:
			break;
		}
	}
}

int main(int argc, char *argv[])
{
	hi_s32 ret;
	set_lo_ipaddr();

	signal(SIGINT, sample_terminate);
	signal(SIGTERM, sample_terminate);
	signal(SIGPWR, sample_power);

	g_sample_link = (sample_link_s *)malloc(sizeof(sample_link_s));
	if (g_sample_link == HI_NULL) {
		return -1;
	}

	memset(g_sample_link, 0, sizeof(sample_link_s));
	pthread_mutex_init(&g_sample_link->mut, HI_NULL);
	pthread_cond_init(&g_sample_link->cond, HI_NULL);

	if (sample_wlan_init_up() != HI_SUCCESS) {
		sample_log_print("sample_wlan_init_up is fail\n");
		goto link_out;
	}

	if (hi_channel_init() != HI_SUCCESS) {
		sample_log_print("hi_channel_init is fail\n");
		goto link_out;
	}

	hi_channel_register_rx_cb(agtx_cmd_handler);

	if (sample_register_cmd((sample_cmd_entry_stru *)&g_sample_cmd, SAMPLE_CMD_NUM) != HI_SUCCESS) {
		sample_log_print("register wlan cmd is fail\n");
		goto link_out;
	}

#ifdef DEBUG
	ret = pthread_create(&g_sample_link->sock_thread_dbg, HI_NULL, sample_sock_thread_dbg, HI_NULL);
	if (ret != HI_SUCCESS) {
		sample_log_print("create sock DBG thread is fail\n");
		goto link_out;
	}
#endif

	ret = pthread_create(&g_sample_link->sock_thread, HI_NULL, sample_sock_thread, HI_NULL);
	if (ret != HI_SUCCESS) {
		sample_log_print("create sock DBG thread is fail\n");
		goto link_out;
	}

	agtx_lpw_init(argc, argv);
	if (agtx_lpw_sync() < 0)
		goto link_out;

	main_process();

link_out:
	sample_cleanup();
	sample_log_print("terminate\n");
	return -1;
}
