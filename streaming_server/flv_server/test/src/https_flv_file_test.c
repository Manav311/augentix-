#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include <getopt.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include "https_flv.h"
#include "http_flv.h"

#define DEBUG
#ifdef DEBUG
#define LOG(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);
#else
#define LOG(format, args...)
#endif
#define ERR(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);
#define INFO(format, args...) fprintf(stderr, "[%s:%d]" format, __func__, __LINE__, ##args);

int run_flag = 0;
int srv_soc = 0, acpt_soc = 0;
#define HTTP_BUF_SIZE 1024
#define HTTP_FILENAME_LEN 256
#define FILE_NAME_LEN (64)

void handleSignal(int signo)
{
	if (signo == SIGINT) {
		ERR("received SIGINT\n");
		run_flag = 0;
		ERR("close port :%d , %d\n", srv_soc, acpt_soc);
	}

	return;
}

void *runHttpsServer(void *argv)
{
	HttpsServerInfo *info = (HttpsServerInfo *)argv;

	LOG("Assigned port %d, src:%s\r\ncert path:%s\r\nkey path:%s\r\n", info->port, info->src_file, info->cert_file,
	    info->key_file);

	SSL_CTX *ctx;

	struct sockaddr_in serv_addr;
	struct sockaddr_in from_addr;
	char recv_buf[HTTP_BUF_SIZE];
	char read_buf[HTTP_BUF_SIZE];
	socklen_t from_len = sizeof(from_addr);
	int result = 0, recv_len;

	TLS_initOpenssl();
	ctx = TLS_createContext();
	TLS_configureContext(ctx, &info->cert_file[0], &info->key_file[0]);
	srv_soc = HTTP_setlistenPort(info->port, 0);

	while (run_flag) {
		SSL *ssl;

		acpt_soc = accept(srv_soc, (struct sockaddr *)&from_addr, &from_len);
		if (acpt_soc < 0) {
			perror("Unable to accept");
			break;
		}

		INFO("[Web] Accepted address:[%s], port:[%d]\n", inet_ntoa(from_addr.sin_addr),
		     ntohs(from_addr.sin_port));

		ssl = SSL_new(ctx);
		SSL_set_fd(ssl, acpt_soc);

		if (SSL_accept(ssl) <= 0) {
			ERR_print_errors_fp(stderr);
		}

		/*seperate to single thread*/
		recv_len = SSL_read(ssl, recv_buf, HTTP_BUF_SIZE);
		if (recv_len == -1) {
			close(acpt_soc);
			SSL_shutdown(ssl);
			SSL_free(ssl);
			ERR("[Web] Fail to recv, error = %d\n", recv_len);
			break;
		}

		LOG("%s", recv_buf);
		recv_buf[recv_len] = 0;

		char http_res_hdr_tmpl[] = "HTTP/1.1 200 OK\r\n"
		                           "Server: Augentix <0.1>\r\n"
		                           "Content-Type:  video/x-flv\r\n"
		                           "Connection: keep-alive\r\n"
		                           "Expires: -1\r\n"
		                           "Access-Control-Allow-Origin: *\r\n"
		                           "Access-Control-Allow-Credentials: true\r\n\r\n";
		SSL_write(ssl, http_res_hdr_tmpl, strlen(http_res_hdr_tmpl));

		FILE *fp = fopen(info->src_file, "rb+");
		fseek(fp, 0, SEEK_END);
		int file_len = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		int read_len = 0;
		int send_len = 0;
		do {
			read_len = fread(read_buf, sizeof(char), HTTP_BUF_SIZE, fp);
			if (read_len > 0) {
				send_len = SSL_write(ssl, read_buf, read_len);

				if (send_len == -1) {
					ERR("[Web] Fail to send, error = %d\n", send_len);
					break;
				}
				file_len -= read_len;
			}
		} while (((read_len > 0) && (file_len > 0)) && (run_flag));

		fclose(fp);
		LOG("[Web] send finish\n");

		SSL_shutdown(ssl);
		SSL_free(ssl);
		close(acpt_soc);
	}

	SSL_CTX_free(ctx);
	TLS_cleanupOpenssl();

	return NULL;
}

void help()
{
	printf("Usage\r\n");
	printf("-p port number\r\n");
	printf("-s flv format src file\r\n");
	printf("-c cert.pem file\r\n");
	printf("-k private key file\r\n");
	printf("-h help");
}

int main(int argc, char **argv)
{
	if (signal(SIGINT, handleSignal) == SIG_ERR) {
		ERR("\ncan't catch SIGINT\n");
	}
	char src_file[64];
	char cert_file[64];
	char key_file[64];
	int port = 8443;
	int c = 0;

	while ((c = getopt(argc, argv, "hc:k:s:p:")) != -1) {
		switch (c) {
		case 'h':
			help();
			exit(1);
			break;
		case 'p':
			port = atoi(argv[optind - 1]);
			break;
		case 'c':
			snprintf(&cert_file[0], 64, "%s", argv[optind - 1]);
			break;
		case 'k':
			snprintf(&key_file[0], 64, "%s", argv[optind - 1]);
			break;
		case 's':
			snprintf(&src_file[0], 64, "%s", argv[optind - 1]);
			break;
		default:
			help();
			exit(1);
		}
	}

	HttpsServerInfo server_info;
	server_info.port = port;
	server_info.cert_file = malloc(FILE_NAME_LEN);
	server_info.key_file = malloc(FILE_NAME_LEN);
	server_info.src_file = malloc(FILE_NAME_LEN);

	memcpy(server_info.cert_file, &cert_file[0], sizeof(cert_file));
	memcpy(server_info.key_file, &key_file[0], sizeof(key_file));
	memcpy(server_info.src_file, &src_file[0], sizeof(src_file));

	if ((access(src_file, F_OK) == -1) || (access(key_file, F_OK) == -1) || (access(cert_file, F_OK) == -1)) {
		ERR("File not exist\r\n");
		free(server_info.cert_file);
		free(server_info.key_file);
		free(server_info.src_file);
		return -EPERM;
	}

	run_flag = 1;
	pthread_t t0;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if (pthread_create(&t0, &attr, runHttpsServer, (void *)&server_info) != 0) {
		ERR("failed to create thread\r\n");
	}
	pthread_attr_destroy(&attr);

	if (pthread_setname_np(t0, "https-flv") != 0) {
		ERR("failed to set thread name\r\n");
	}

	while (run_flag) {
		sleep(2);
	}

	free(server_info.cert_file);
	free(server_info.key_file);
	free(server_info.src_file);


	return 0;
}
