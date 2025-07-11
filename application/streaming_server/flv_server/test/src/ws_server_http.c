#include <libwebsockets.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#include <time.h>
#include <json.h>

#include "log_define.h"
#include "audio.h"

#include "wss_msg.h"
#include "wss_server.h"

static int interrupted = 0;

void sigintHandler(int sig)
{
	interrupted = 1;
}

int main(void)
{
	signal(SIGINT, sigintHandler);

	/*ws init*/
#ifdef HTTP_FLV_DEBUG
	int logs = LLL_USER | LLL_ERR | LLL_WARN | LLL_NOTICE | LLL_DEBUG;
#define FLV_DEBUG
#else
	int logs = LLL_USER | LLL_ERR | LLL_WARN;
#endif
	lws_set_log_level(logs, NULL);
	lwsl_user("ws_server | visit http://localhost:%d\n", WS_PORT);

	struct lws_context_creation_info info = { 0 };
	info.port = WS_PORT;
	info.protocols = protocols;
	info.extensions = extensions;
	info.ssl_cert_filepath = "/etc/nginx/ssl/cert.pem";
	info.ssl_private_key_filepath = "/etc/nginx/ssl/key.pem";

	info.options = LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;

	struct lws_context *context = lws_create_context(&info);
	if (!context) {
		lwsl_err("lws init failed\n");
		return -EPERM;
	}

	while (!interrupted) {
		lws_service(context, 0);
	}
	lws_context_destroy(context);

	return 0;
}
