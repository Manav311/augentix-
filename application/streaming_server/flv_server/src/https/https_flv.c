#include "https_flv.h"

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
#include <pthread.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include "log_define.h"
#include "http_parser.h"
#include "http_flv_server.h"

pthread_mutex_t *ssl_mutex = NULL;
#if 0
static void ssl_locking_cb(int mode, int type, const char *file, int line)
{
	if (mode & CRYPTO_LOCK)
		pthread_mutex_lock(&ssl_mutex[type]);
	else
		pthread_mutex_unlock(&ssl_mutex[type]);
}

static unsigned long ssl_id_cb(void)
{
	return (unsigned long)pthread_self();
}
#endif
SSL_CTX *TLS_createContext()
{
	const SSL_METHOD *method;
	SSL_CTX *ctx;

	method = SSLv23_server_method();

	ctx = SSL_CTX_new(method);
	if (!ctx) {
		flv_server_log_err("Unable to create SSL context");
		ERR_print_errors_fp(stderr);
		return (SSL_CTX *)NULL;
	}

	return ctx;
}

int TLS_configureContext(SSL_CTX *ctx, char *cert_path, char *key_path)
{
	if ((cert_path == NULL) || (key_path == NULL)) {
		flv_server_log_err("failed to get .pem");
		return -EACCES;
	}

	/* Set the key and cert */
	if (SSL_CTX_use_certificate_file(ctx, cert_path, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		flv_server_log_err("Failed use cert file");
		return -EACCES;
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, key_path, SSL_FILETYPE_PEM) <= 0) {
		ERR_print_errors_fp(stderr);
		flv_server_log_err("Failed use key file");
		return -EACCES;
	}

	return 0;
}

void TLS_initOpenssl()
{
	int i;

	/* The number of lock we need is getting from CRYPTO_num_locks() */
	if ((ssl_mutex = malloc(sizeof(pthread_mutex_t) * CRYPTO_num_locks())) == NULL) {
		flv_server_log_err("malloc() failed.");
		return;
	}

	/* Init. mutex. */
	for (i = 0; i < CRYPTO_num_locks(); i++) {
		pthread_mutex_init(&ssl_mutex[i], NULL);
	}

	/* Set up locking function */
	CRYPTO_set_locking_callback(ssl_locking_cb);
	CRYPTO_set_id_callback(ssl_id_cb);

	SSL_library_init();
	ERR_load_crypto_strings();
	SSL_load_error_strings();
	OpenSSL_add_ssl_algorithms();
}

void TLS_cleanupOpenssl()
{
	int i;
	CRYPTO_set_locking_callback(NULL);
	// Destroy the locks
	for (i = 0; i < CRYPTO_num_locks(); i++) {
		pthread_mutex_destroy(&(ssl_mutex[i]));
	}
	OPENSSL_free(ssl_mutex);
	ssl_mutex = NULL;

	EVP_cleanup();
}
