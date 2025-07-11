#define _GNU_SOURCE

#include "secure_openssl_demo.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/core_dispatch.h>

#include "log.h"
#include <selib/Core_SELib.h>

#define DEBUG

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define MAX_BUF 1024

typedef struct {
	int (*cb)(unsigned char *out, size_t outlen, const char *name, EVP_RAND_CTX *ctx);
	int state;
	const char *name;
	EVP_RAND_CTX *ctx;
} CUSTOM_RAND;

void print_openssl_error()
{
	unsigned long err;
	while ((err = ERR_get_error()) != 0) {
		char *str = ERR_error_string(err, NULL);
		fprintf(stderr, "OpenSSL Error: %s\n", str);
	}
}

void handle_error(const char *msg)
{
	perror(msg);
	ERR_print_errors_fp(stderr);
	exit(EXIT_FAILURE);
}

int custom_rand_bytes(unsigned char *buf, size_t num, void *userdata)
{
	unsigned char random_output[32] = { 0 };
	size_t remaining_bytes = num, offset = 0;
	int ret;
	do {
		ret = Core_random(random_output);
		if (ret) {
			return 0;
		}
		if (remaining_bytes >= 32) {
			memcpy(buf + offset, random_output, 32);
			offset += 32;
			remaining_bytes -= 32;
		} else {
			memcpy(buf + offset, random_output, remaining_bytes);
			remaining_bytes = 0;
		}
	} while (remaining_bytes > 0);
	return 1;
}

static OSSL_FUNC_rand_newctx_fn custom_rand_newctx;
static OSSL_FUNC_rand_freectx_fn custom_rand_freectx;
static OSSL_FUNC_rand_instantiate_fn custom_rand_instantiate;
static OSSL_FUNC_rand_uninstantiate_fn custom_rand_uninstantiate;
static OSSL_FUNC_rand_generate_fn custom_rand_generate;
static OSSL_FUNC_rand_enable_locking_fn custom_rand_enable_locking;
static OSSL_FUNC_rand_gettable_ctx_params_fn custom_rand_gettable_ctx_params;
static OSSL_FUNC_rand_get_ctx_params_fn custom_rand_get_ctx_params;

static void *custom_rand_newctx(void *provctx, void *parent, const OSSL_DISPATCH *parent_dispatch)
{
	CUSTOM_RAND *r = OPENSSL_zalloc(sizeof(*r));
	if (r != NULL)
		r->state = EVP_RAND_STATE_UNINITIALISED;
	return r;
}

static void custom_rand_freectx(void *vrng)
{
	OPENSSL_free(vrng);
}

static int custom_rand_instantiate(void *vrng, unsigned int strength, int prediction_resistance,
                                   const unsigned char *pstr, size_t pstr_len, const OSSL_PARAM params[])
{
	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	rng->state = EVP_RAND_STATE_READY;
	return 1;
}

static int custom_rand_uninstantiate(void *vrng)
{
	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	rng->state = EVP_RAND_STATE_UNINITIALISED;
	return 1;
}

static int custom_rand_generate(void *vrng, unsigned char *out, size_t outlen, unsigned int strength,
                                int prediction_resistance, const unsigned char *adin, size_t adinlen)
{
	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	if (rng->cb != NULL)
		return (*rng->cb)(out, outlen, rng->name, rng->ctx);
	if (!custom_rand_bytes(out, outlen, NULL)) {
		ERR_raise(ERR_LIB_PROV, ERR_R_INTERNAL_ERROR);
		return 0;
	}
	return 1;
}

static int custom_rand_enable_locking(void *ctx)
{
	return 1;
}

static const OSSL_PARAM *custom_rand_gettable_ctx_params(void *vrng, void *provctx)
{
	static const OSSL_PARAM known_gettable_ctx_params[] = { OSSL_PARAM_int(OSSL_RAND_PARAM_STATE, NULL),
		                                                OSSL_PARAM_uint(OSSL_RAND_PARAM_STRENGTH, NULL),
		                                                OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
		                                                OSSL_PARAM_END };
	return known_gettable_ctx_params;
}

static int custom_rand_get_ctx_params(void *vrng, OSSL_PARAM params[])
{
	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	OSSL_PARAM *p;

	if ((p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STATE)) != NULL && !OSSL_PARAM_set_int(p, rng->state))
		return 0;
	if ((p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_STRENGTH)) != NULL && !OSSL_PARAM_set_uint(p, 256))
		return 0;
	if ((p = OSSL_PARAM_locate(params, OSSL_RAND_PARAM_MAX_REQUEST)) != NULL && !OSSL_PARAM_set_size_t(p, INT_MAX))
		return 0;
	return 1;
}

static const OSSL_DISPATCH custom_rand_functions[] = {
	{ OSSL_FUNC_RAND_NEWCTX, (void (*)(void))custom_rand_newctx },
	{ OSSL_FUNC_RAND_FREECTX, (void (*)(void))custom_rand_freectx },
	{ OSSL_FUNC_RAND_INSTANTIATE, (void (*)(void))custom_rand_instantiate },
	{ OSSL_FUNC_RAND_UNINSTANTIATE, (void (*)(void))custom_rand_uninstantiate },
	{ OSSL_FUNC_RAND_GENERATE, (void (*)(void))custom_rand_generate },
	{ OSSL_FUNC_RAND_ENABLE_LOCKING, (void (*)(void))custom_rand_enable_locking },
	{ OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS, (void (*)(void))custom_rand_gettable_ctx_params },
	{ OSSL_FUNC_RAND_GET_CTX_PARAMS, (void (*)(void))custom_rand_get_ctx_params },
	{ 0, NULL }
};

static const OSSL_ALGORITHM custom_rand_algs[] = { { "CUSTOM", "provider=custom", custom_rand_functions },
	                                           { NULL, NULL, NULL } };

static const OSSL_ALGORITHM *custom_query(void *provctx, int operation_id, int *no_cache)
{
	*no_cache = 0;
	switch (operation_id) {
	case OSSL_OP_RAND:
		return custom_rand_algs;
	default:
		return NULL;
	}
}

static const OSSL_DISPATCH custom_dispatch[] = { { OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)(void))OSSL_LIB_CTX_free },
	                                         { OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void))custom_query },
	                                         { 0, NULL } };

static int custom_provider_init(const OSSL_CORE_HANDLE *handle, const OSSL_DISPATCH *in, const OSSL_DISPATCH **out,
                                void **provctx)
{
	OSSL_LIB_CTX *libctx = OSSL_LIB_CTX_new();
	if (libctx == NULL) {
		return 0;
	}
	*provctx = libctx;
	*out = custom_dispatch;
	return 1;
}

OSSL_PROVIDER *custom_rand_start(OSSL_LIB_CTX *libctx)
{
	OSSL_PROVIDER *p;

	if (!OSSL_PROVIDER_add_builtin(libctx, "custom-rand", custom_provider_init)) {
		return NULL;
	}

	if (!RAND_set_DRBG_type(libctx, "custom", NULL, NULL, NULL)) {
		return NULL;
	}

	p = OSSL_PROVIDER_try_load(libctx, "custom-rand", 1);
	if (p == NULL) {
		return NULL;
	}

	return p;
}

int tls_client_hw(const char *hostname, const int port, const char *cert_file, const char *key_file,
                  const char *ca_file)
{
	SSL_CTX *ctx;
	SSL *ssl;
	BIO *bio;
	char buf[MAX_BUF];
	int ret;
	OSSL_PROVIDER *custom_provider = NULL;
	OSSL_PROVIDER *default_provider = NULL;
	OSSL_LIB_CTX *lib_ctx = NULL;

	SSL_library_init();
	OpenSSL_add_all_algorithms();
	SSL_load_error_strings();

	lib_ctx = OSSL_LIB_CTX_new();
	if (!lib_ctx)
		handle_error("Error creating library context");

	printf("[%s] L%d Start to generate the custom RNG via the secure element.\n", __func__, __LINE__);
	custom_provider = custom_rand_start(lib_ctx);
	if (!custom_provider)
		handle_error("Error starting custom RNG");
	printf("[%s] L%d Successfully generated the custom RNG via the secure element.\n", __func__, __LINE__);

	default_provider = OSSL_PROVIDER_load(lib_ctx, "default");
	if (!default_provider)
		handle_error("Error loading default provider");

	ctx = SSL_CTX_new_ex(lib_ctx, "provider=default", TLS_client_method());
	if (!ctx) {
		ERR_print_errors_fp(stderr);
		handle_error("Error creating SSL context");
	}

	if (SSL_CTX_use_certificate_file(ctx, cert_file, SSL_FILETYPE_PEM) <= 0) {
		fprintf(stderr, "Error loading client certificate from %s\n", cert_file);
		handle_error("Error loading client certificate");
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, key_file, SSL_FILETYPE_PEM) <= 0) {
		fprintf(stderr, "Error loading client private key from %s\n", key_file);
		handle_error("Error loading client private key");
	}

	if (!SSL_CTX_load_verify_locations(ctx, ca_file, NULL)) {
		fprintf(stderr, "Error loading CA certificate from %s\n", ca_file);
		handle_error("Error loading CA certificate");
	}

	SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);

	bio = BIO_new_ssl_connect(ctx);
	if (!bio)
		handle_error("Error creating BIO chain");

	BIO_get_ssl(bio, &ssl);
	if (!ssl)
		handle_error("Error getting SSL object");

	if (SSL_set_tlsext_host_name(ssl, hostname) != 1) {
		handle_error("Error setting hostname for SNI");
	}

	char connection_string[256];
	snprintf(connection_string, sizeof(connection_string), "%s:%d", hostname, port);
	if (BIO_set_conn_hostname(bio, connection_string) <= 0) {
		handle_error("Error setting connection hostname");
	}

	if (BIO_do_connect(bio) <= 0) {
		fprintf(stderr, "Error connecting to %s\n", connection_string);
		handle_error("Error connecting to server");
	}
	printf("[%s] L%d Connected to %s\n", __func__, __LINE__, connection_string);

	if (BIO_do_handshake(bio) <= 0) {
		fprintf(stderr, "Error during SSL handshake\n");
		handle_error("Error performing SSL handshake");
	}

	if (SSL_get_verify_result(ssl) != X509_V_OK)
		handle_error("Certificate verification error");

	printf("[%s] L%d Connected with %s encryption\n", __func__, __LINE__, SSL_get_cipher(ssl));

	const char *message = "Hello, server!";
	ret = BIO_write(bio, message, strlen(message));
	if (ret <= 0)
		handle_error("Error writing to server");

	printf("[%s] L%d Read from the host...\n", __func__, __LINE__);
	ret = BIO_read(bio, buf, sizeof(buf) - 1);
	if (ret > 0) {
		buf[ret] = '\0';
		printf("Received: %s\n", buf);
	} else if (ret == 0) {
		printf("Connection closed\n");
	} else {
		handle_error("Error reading from server");
	}

	BIO_free_all(bio);
	SSL_CTX_free(ctx);
	OSSL_PROVIDER_unload(custom_provider);
	OSSL_PROVIDER_unload(default_provider);
	OSSL_LIB_CTX_free(lib_ctx);
	EVP_cleanup();

	return 0;
}

/**
 * @brief execute the demonstration of secure OpenSSL
 * @details
 * @param[in]
 * @arg
 * @see
 * @return result of the demonstration
 * @retval 0		demonstrate success
 * @retval not 0	demonstrate fail
 */
int SEOPENSSL_executeDemo(const int i2c_bus_id, const char *hostname, const int port, const char *cert_file,
                          const char *key_file, const char *ca_file)
{
	int ret = 0;

	/* Initialize libse */
	ret = securityLib_init();
	if (ret) {
		log_err("Fail to initialize the secure element library.\n");
		return ret;
	}
	DBG("[%s] L%d The secure element library is initialized.\n", __func__, __LINE__);

	/* Set I2C bus ID */
	ret = securityLib_setI2CBus(i2c_bus_id);
	if (ret) {
		log_err("Fail to set the I2C bus ID.\n");
		return ret;
	}
	printf("[%s] L%d I2C bus ID = %d\n", __func__, __LINE__, i2c_bus_id);

	/* TLS client HW */
	ret = tls_client_hw(hostname, port, cert_file, key_file, ca_file);
	if (ret) {
		log_err("Fail to execute TLS client HW demo.\n");
		return ret;
	}

	return ret;
}
