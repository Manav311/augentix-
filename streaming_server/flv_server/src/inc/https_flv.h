#ifndef HTTPS_FLV_H
#define HTTPS_FLV_H

#include <openssl/ssl.h>
#include <openssl/err.h>

#define HTTP_BUF_SIZE 1024
typedef struct {
	int port;
	char *cert_file;
	char *key_file;
	char *ip_address;
	int client_socket;
	SSL_CTX *ctx;
	SSL *ssl;
} HttpsServerInfo;

SSL_CTX *TLS_createContext();

int TLS_configureContext(SSL_CTX *ctx, char *cert_path, char *key_path);

void TLS_initOpenssl();
void TLS_cleanupOpenssl();

#endif