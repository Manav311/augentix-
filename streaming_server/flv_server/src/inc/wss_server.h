#ifndef WSS_SERVER_H_
#define WSS_SERVER_H_

#include "libwebsockets.h"
#include <stdlib.h>
#include <string.h>

#define WS_PORT (7681)
#define MAX_PAYLOAD_SIZE ((8 * 3 * 4 * 1024) >> 1)

typedef enum { INIT, START, AVAIL, END } ServerStat;

typedef struct {
	int port;
	char *cert_file;
	char *key_file;
} WssServerInfo;

typedef struct {
	int msg_count;
	unsigned char buf[LWS_PRE + MAX_PAYLOAD_SIZE];
	unsigned int len;
	int bin;
	int fin;
} SessionData;

typedef struct {
	int client_num;
	struct lws *wsi;
} ServingClient;

int wsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static const struct lws_protocols protocols[] = {
	{
	        .name = "ws",
	        .callback = wsCallback,
	        .per_session_data_size = sizeof(SessionData),
	        .rx_buffer_size = MAX_PAYLOAD_SIZE,
	        .id = 0,
	        .user = NULL,
	        .tx_packet_size = 0,
	},
	{
	        .name = NULL,
	        .callback = NULL,
	        .per_session_data_size = 0,
	        .rx_buffer_size = 0,
	        .id = 0,
	        .user = NULL,
	        .tx_packet_size = 0,
	} /* terminator */
};

static const struct lws_extension extensions[] = { { "permessage-deflate", lws_extension_callback_pm_deflate,
	                                             "permessage-deflate"
	                                             "; client_no_context_takeover"
	                                             "; client_max_window_bits" },
	                                           { NULL, NULL, NULL /* terminator */ } };

void *runWssServerListenThread(void *data);

#endif
