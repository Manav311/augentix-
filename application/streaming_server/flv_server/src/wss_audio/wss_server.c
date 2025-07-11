#include "wss_server.h"

#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <pthread.h>

#include <time.h>
#include <json.h>
#include "log_define.h"
#include "wss_msg.h"

snd_pcm_t *g_capture;

ServerStat g_stat = INIT;
MimeType g_mime = ALAW;
ServingClient g_client_info = { .client_num = 0, .wsi = NULL };

char *g_jsonBuf;
int g_buf_size;
extern int g_run_flag;

int WSS_pcmInitbyMimetype(MimeType type)
{
	/*snd_pcm init*/
	snd_pcm_uframes_t frame = 256;
	unsigned int channel = 1;
	unsigned int rate = 8000;
	unsigned int gain = 25;
	unsigned int stream = SND_PCM_STREAM_PLAYBACK;
	unsigned int format = SND_PCM_FORMAT_A_LAW; /*need parse correct format*/
	if (type == MULAW) {
		flv_server_log_info("format MULAW");
		format = SND_PCM_FORMAT_MU_LAW;
	}

	int err = 0;
	flv_server_log_info("pcm init format:%d", format);
	err = agtxPcmInit(&g_capture, "default", stream, format, frame, rate, channel);
	if (err < 0) {
		flv_server_log_err("pcm init failed");
		return -EIO;
	}

	err = agtxSetGain(gain);
	if (err < 0) {
		flv_server_log_err("set_gain failed: %s", snd_strerror(err));
	}

	return err;
}

int WSS_pcmDeinit()
{
	int err = agtxPcmUninit(g_capture);
	return err;
}

int WSS_pcmPlay(char *data, int len)
{
	int buf_size = 256;
	int remain_size = len;
	int idx = 0;
	int err = 0;
	while (remain_size > 0) {
		err = snd_pcm_writei(g_capture, data + idx, buf_size);
		if (err == -EPIPE) {
			/* EPIPE means xrun, 0 data */
			flv_server_log_info("Underrun occrred: %s", snd_strerror(err));
			snd_pcm_prepare(g_capture);
		} else if (err < 0) {
			flv_server_log_err("error from write: %s", snd_strerror(err));
		} else if (err != buf_size) {
			flv_server_log_err("Short write (expected %d, wrote %d)", (int)buf_size, (int)err);
		}

		remain_size -= buf_size;
		idx += buf_size;
	}

	return 0;
}

int wsCallback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
	int ret = 0;
	PcmMessage msg = { 0 };
	SessionData *data = (SessionData *)user;

	switch (reason) {
	case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
		flv_server_log_info("ask need to close old client?");
		flv_server_log_info("enter client[%d]", g_client_info.client_num);
		g_client_info.client_num += 1;
		if (g_client_info.client_num == 2) {
			ServingClient *old_client = &g_client_info;
			lws_set_timeout(old_client->wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE, LWS_TO_KILL_ASYNC);
		}
		g_client_info.wsi = wsi;
	case LWS_CALLBACK_ESTABLISHED:
		flv_server_log_info("Client connected.");
		lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE, 10);
		break;
	case LWS_CALLBACK_RECEIVE:
		data->fin = lws_is_final_fragment(wsi);
		data->bin = lws_frame_is_binary(wsi);
		lws_rx_flow_control(wsi, 0);

		if (g_jsonBuf == NULL) {
			g_buf_size = 0;
			g_jsonBuf = malloc(MAX_PAYLOAD_SIZE);
			memcpy(g_jsonBuf + g_buf_size, in, len);
			g_buf_size += len;
		} else if ((unsigned)g_buf_size < (MAX_PAYLOAD_SIZE - len - 1)) {
			memcpy(g_jsonBuf + g_buf_size, in, len);
			g_buf_size += len;
		}

		if (data->fin == 1) {
			flv_server_log_info("sum recv: %d", g_buf_size);

			/*parse msg when fin*/
			ret = WSS_parseWssPcmMessage(g_jsonBuf, &msg);
			g_buf_size = 0;

			if (ret != 0) {
				flv_server_log_err("failed to parse");
				free(g_jsonBuf);
				g_jsonBuf = NULL;
				ServingClient *old_client = &g_client_info;
				lws_set_timeout(old_client->wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE,
				                LWS_TO_KILL_ASYNC);
				break;
			}

			if (g_jsonBuf != NULL) {
				free(g_jsonBuf);
				g_jsonBuf = NULL;
				flv_server_log_info("free g_jsonBuf");
			}

			if (0 == strcmp(msg.msg_type, "ping")) {
				sprintf((char *)&data->buf[LWS_PRE], "{\"id\":%lu,\"type\":\"pong\"}",
				        (unsigned long)time(NULL));
				data->len = LWS_PRE + strlen("{\"id\":1634210417470,\"type\":\"pong\"}");
				flv_server_log_info("send pong, len:%d", data->len);
			} else if (0 == strcmp(msg.msg_type, "pong")) {
				/*do nothing*/
			} else if (0 == strcmp(msg.msg_type, "call_start")) {
				flv_server_log_info("call start");
				if ((g_stat == END) || (g_stat == INIT)) {
					g_stat = START;
				} else {
					flv_server_log_err("unavailable stat:%d", g_stat);
				}

				lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE, 10);
			} else if (0 == strcmp(msg.msg_type, "call_end")) {
				flv_server_log_info("call end");
				if (g_stat == AVAIL) {
					flv_server_log_info("do snd pcm deinit");
					g_stat = END;
				} else {
					flv_server_log_err("unavailable stat:%d", g_stat);
				}

				lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE, 1);

			} else if (0 == strcmp(msg.msg_type, "data_available")) {
				flv_server_log_info("data_available");
				if (g_stat == START) {
					flv_server_log_info("do snd pcm init");
					WSS_pcmInitbyMimetype(msg.type);
					WSS_pcmPlay(msg.data, msg.length);
					g_stat = AVAIL;
				} else if (g_stat == AVAIL) {
					/*check mimetype change*/
					WSS_pcmPlay(msg.data, msg.length);
				} else {
					flv_server_log_err("unavailable stat:%d", g_stat);
				}

				lws_set_timeout(wsi, PENDING_TIMEOUT_AWAITING_PROXY_RESPONSE, 10);
			}

			if (msg.data != NULL) {
				free(msg.data);
			}
		}

		lws_callback_on_writable(wsi);
		break;
	case LWS_CALLBACK_SERVER_WRITEABLE:
		if (data->len > 0) {
			lws_write(wsi, &data->buf[LWS_PRE], data->len, LWS_WRITE_TEXT);
		}
		data->len = 0;
		lws_rx_flow_control(wsi, 1);
		break;
	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE:
		flv_server_log_err("client end an unsolicited Close WS packet.");
		break;
	case LWS_CALLBACK_CLOSED:
		flv_server_log_notice("Client disconnected.");
		WSS_pcmDeinit();

		if (msg.data != NULL) {
			free(msg.data);
		}
		if (g_jsonBuf != NULL) {
			free(g_jsonBuf);
			g_jsonBuf = NULL;
			flv_server_log_info("free g_jsonBuf");
		}
		g_client_info.client_num -= 1;
		g_stat = END;
		break;
	default:
		/* other reasons */
		break;
	}
	return EXIT_SUCCESS;
}

void *runWssServerListenThread(void *data)
{
	const WssServerInfo *info = (const WssServerInfo *)data;

#ifdef FLV_DEBUG
	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG;
#else
	int logs = LLL_USER | LLL_ERR | LLL_WARN;
#endif
	/*ws init*/
	lws_set_log_level(logs, NULL);
	lwsl_user("wss_server | visit http://localhost:%d\n", WS_PORT);

	struct lws_context_creation_info context_info = { 0 };
	context_info.port = info->port;
	context_info.protocols = protocols;
	context_info.extensions = extensions;
	context_info.ssl_cert_filepath = info->cert_file;
	context_info.ssl_private_key_filepath = info->key_file;
	context_info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	struct lws_context *context = lws_create_context(&context_info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return NULL;
	}

	while (g_run_flag) {
		lws_service(context, 0);
	}

	lws_context_destroy(context);
	pthread_detach(pthread_self());
	return NULL;
}
