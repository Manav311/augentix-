#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h> // for write
#include <pthread.h> // for threading, link with lpthread
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <getopt.h>

#include "log_define.h"
#include "http_flv_server.h"

#ifdef SUPPORT_HTTPS
#include "https_flv_server.h"
#include "https_flv.h"
#endif // SUPPORT_HTTPS

#ifdef WEBSOCKET_AUDIO_PLAYBACK
#include "wss_server.h"
#endif // WEBSOCKET_AUDIO_PLAYBACK

#include "video.h"
#include "audio.h"

#define SLEEP_INTERVAL (2)

int g_run_flag = 0;

static void handleSigInt(int signo)
{
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGPIPE) {
		printf("Caught SIGPIPE!\n");
		return;
	} else {
		perror("Unexpected signal!\n");
	}
	g_run_flag = 0;
}

void help()
{
	printf("usage:\r\n"
	       "-t --http Run http flv server\r\n"
#ifdef SUPPORT_HTTPS
	       "-s --https Run https flv server\r\n"
#endif // SUPPORT_HTTPS
#ifdef WEBSOCKET_AUDIO_PLAYBACK
	       "-w --wss  Run wss 2-way audio server\r\n"
#endif // WEBSOCKET_AUDIO_PLAYBACK
	       "-k <key_path>       TLS key file path\r\n"
	       "-c <cert_path>      TLS certificate file path\r\n"
	       "-a <ip_address>     Set the IP address the server will listen on\r\n"
	       "-h help()\r\n");
}

int main(int argc, char *argv[])
{
	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGTERM!\n");
		exit(1);
	}

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGPIPE!\n");
		exit(1);
	}

	const char *optstring = "hwtsc:k:a:";
	int c;
	struct option opts[] = { { "http", 0, NULL, 't' },
#ifdef SUPPORT_HTTPS
		                 { "https", 0, NULL, 's' },
#endif // SUPPORT_HTTPS
#ifdef WEBSOCKET_AUDIO_PLAYBACK
		                 { "wss", 0, NULL, 'w' },
#endif // WEBSOCKET_AUDIO_PLAYBACK
		                 { "cert", 1, NULL, 'c' },  { "key", 1, NULL, 'k' }, { "address", 1, NULL, 'a' } };

	bool has_wss = false;
	bool has_http = false;
	bool has_https = false;
	char *cert_file = NULL;
	char *key_file = NULL;
	char *ip_address = NULL;
#ifdef SUPPORT_HTTPS
	HttpsServerInfo https_info;
#endif // SUPPORT_HTTPS
#ifdef WEBSOCKET_AUDIO_PLAYBACK
	WssServerInfo wss_info;
#endif // WEBSOCKET_AUDIO_PLAYBACK

	while ((c = getopt_long(argc, argv, optstring, opts, NULL)) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'w':
			has_wss = true;
			break;
		case 't':
			has_http = true;
			break;
		case 's':
			has_https = true;
			break;
		case 'c':
			cert_file = optarg;
			break;
		case 'k':
			key_file = optarg;
			break;
		case 'a':
			ip_address = optarg;
			break;
		default:
			help();
			exit(1);
		}
	}

	flv_server_log_debug("has_wss: %d, has_http: %d, has_https: %d.cert path: %s.key path: %s.", has_wss, has_http,
	                     has_https, cert_file, key_file);

	if (((has_https) || (has_wss)) && ((access(cert_file, F_OK) == -1) || (access(key_file, F_OK) == -1))) {
		flv_server_log_err("failed to open TLS file: %s, %s.", cert_file, key_file);
		return -EACCES;
	}

	g_run_flag = 1;
	int ret = 0;

	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to MPI_SYS_init!, ret: %d", ret);
		return -EPERM;
	}

	ret = MPI_initBitStreamSystem(); /*only once, first clients,  destroy if no client*/
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("MPI_initBitStreamSystem_failed, ret: %d", ret);
		g_run_flag = 0;
		return -EPERM;
	}

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (has_http) {
		pthread_t t0;
		ret = pthread_create(&t0, NULL, runHttpServerListenThread, (void *)ip_address);
		if (ret != 0) {
			flv_server_log_err("failed to create http thread. ret:%d", ret);
		}
		if (pthread_setname_np(t0, "http-flv") != 0) {
			flv_server_log_debug("failed to set thread name.");
		}
	}

#ifdef SUPPORT_HTTPS
	if (has_https) {
		pthread_t t0;
		https_info.port = 8443;
		https_info.cert_file = cert_file;
		https_info.key_file = key_file;
		https_info.ip_address = ip_address;

		ret = pthread_create(&t0, NULL, runHttpsServerListenThread, (void *)&https_info);
		if (ret != 0) {
			flv_server_log_err("failed to create https thread, ret:%d", ret);
		}
		if (pthread_setname_np(t0, "https-flv") != 0) {
			flv_server_log_debug("failed to set thread name");
		}
	}
#endif // SUPPORT_HTTPS

#ifdef WEBSOCKET_AUDIO_PLAYBACK
	if (has_wss) {
		pthread_t t0;
		wss_info.port = WS_PORT;
		wss_info.cert_file = cert_file;
		wss_info.key_file = key_file;

		ret = pthread_create(&t0, NULL, runWssServerListenThread, (void *)&wss_info);
		if (ret != 0) {
			flv_server_log_err("failed to create wss thread, ret: %d", ret);
		}
		if (pthread_setname_np(t0, "wss-server") != 0) {
			flv_server_log_debug("failed to set thread name");
		}
	}
#endif // WEBSOCKET_AUDIO_PLAYBACK

	pthread_attr_destroy(&attr);

	while (g_run_flag) {
		sleep(SLEEP_INTERVAL);
	}

	ret = MPI_exitBitStreamSystem();
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to MPI_exitBitStreamSystem!, ret: %d", ret);
	}

	ret = MPI_SYS_exit();
	if (ret != MPI_SUCCESS) {
		flv_server_log_err("Failed to MPI_SYS_exit!, ret: %d", ret);
	}

	return 0;
}