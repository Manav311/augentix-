#define _GNU_SOURCE

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/provider.h>
#include <openssl/core_names.h>
#include <openssl/crypto.h>
#include <openssl/core_dispatch.h>

#include <selib/Core_SELib.h>

#define I2C_BUS_ID (1)

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
	(void)(userdata);

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

static void *SQ713X_rand_newctx(void *provctx, void *parent, const OSSL_DISPATCH *parent_dispatch)
{
	(void)(provctx);
	(void)(parent);
	(void)(parent_dispatch);

	CUSTOM_RAND *r = OPENSSL_zalloc(sizeof(*r));
	if (r != NULL)
		r->state = EVP_RAND_STATE_UNINITIALISED;
	return r;
}

static void SQ713X_rand_freectx(void *vrng)
{
	OPENSSL_free(vrng);
}

static int SQ713X_rand_instantiate(void *vrng, unsigned int strength, int prediction_resistance,
                                   const unsigned char *pstr, size_t pstr_len, const OSSL_PARAM params[])
{
	(void)(strength);
	(void)(prediction_resistance);
	(void)(pstr);
	(void)(pstr_len);
	(void)(params);

	int ret = 0;
	/* Initialize libse */
	ret = securityLib_init();
	if (ret) {
		ERR_raise(ERR_LIB_PROV, ERR_R_INIT_FAIL);
		return 0;
	}

	/* Set I2C bus ID */
	ret = securityLib_setI2CBus(I2C_BUS_ID);
	if (ret) {
		ERR_raise(ERR_LIB_PROV, ERR_R_INIT_FAIL);
		return 0;
	}

	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	rng->state = EVP_RAND_STATE_READY;
	return 1;
}

static int SQ713X_rand_uninstantiate(void *vrng)
{
	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	rng->state = EVP_RAND_STATE_UNINITIALISED;
	return 1;
}

static int SQ713X_rand_generate(void *vrng, unsigned char *out, size_t outlen, unsigned int strength,
                                int prediction_resistance, const unsigned char *adin, size_t adinlen)
{
	(void)(strength);
	(void)(prediction_resistance);
	(void)(adin);
	(void)(adinlen);

	CUSTOM_RAND *rng = (CUSTOM_RAND *)vrng;
	if (rng->cb != NULL)
		return (*rng->cb)(out, outlen, rng->name, rng->ctx);
	if (!custom_rand_bytes(out, outlen, NULL)) {
		ERR_raise(ERR_LIB_PROV, ERR_R_INTERNAL_ERROR);
		return 0;
	}
	return 1;
}

static int SQ713X_rand_enable_locking(void *ctx)
{
	(void)(ctx);

	return 1;
}

static const OSSL_PARAM *SQ713X_rand_gettable_ctx_params(void *vrng, void *provctx)
{
	(void)(vrng);
	(void)(provctx);

	static const OSSL_PARAM known_gettable_ctx_params[] = { OSSL_PARAM_int(OSSL_RAND_PARAM_STATE, NULL),
		                                                OSSL_PARAM_uint(OSSL_RAND_PARAM_STRENGTH, NULL),
		                                                OSSL_PARAM_size_t(OSSL_RAND_PARAM_MAX_REQUEST, NULL),
		                                                OSSL_PARAM_END };
	return known_gettable_ctx_params;
}

static int SQ713X_rand_get_ctx_params(void *vrng, OSSL_PARAM params[])
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

static const OSSL_DISPATCH se_trng_rand_functions[] = {
	{ OSSL_FUNC_RAND_NEWCTX, (void (*)(void))SQ713X_rand_newctx },
	{ OSSL_FUNC_RAND_FREECTX, (void (*)(void))SQ713X_rand_freectx },
	{ OSSL_FUNC_RAND_INSTANTIATE, (void (*)(void))SQ713X_rand_instantiate },
	{ OSSL_FUNC_RAND_UNINSTANTIATE, (void (*)(void))SQ713X_rand_uninstantiate },
	{ OSSL_FUNC_RAND_GENERATE, (void (*)(void))SQ713X_rand_generate },
	{ OSSL_FUNC_RAND_ENABLE_LOCKING, (void (*)(void))SQ713X_rand_enable_locking },
	{ OSSL_FUNC_RAND_GETTABLE_CTX_PARAMS, (void (*)(void))SQ713X_rand_gettable_ctx_params },
	{ OSSL_FUNC_RAND_GET_CTX_PARAMS, (void (*)(void))SQ713X_rand_get_ctx_params },
	{ 0, NULL }
};

static const OSSL_ALGORITHM se_trng_rand_algs[] = { { "RNG", "provider=se_trng", se_trng_rand_functions, NULL },
	                                            { "CTR-DRBG", "provider=se_trng", se_trng_rand_functions, NULL },
	                                            { NULL, NULL, NULL, NULL } };

static const OSSL_PARAM provider_gettable_params[] = { OSSL_PARAM_utf8_ptr(OSSL_PROV_PARAM_NAME, NULL, 0),
	                                               OSSL_PARAM_utf8_ptr(OSSL_PROV_PARAM_VERSION, NULL, 0),
	                                               OSSL_PARAM_utf8_ptr(OSSL_PROV_PARAM_BUILDINFO, NULL, 0),
	                                               OSSL_PARAM_END };

static const OSSL_PARAM *se_trng_gettable_params(void *provctx)
{
	(void)(provctx);

	return provider_gettable_params;
}

static int se_trng_get_params(OSSL_PARAM params[])
{
	OSSL_PARAM *p;

	p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_NAME);
	if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, "se_trng")) {
		return 0;
	}

	p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_VERSION);
	if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, "1.0")) {
		return 0;
	}

	p = OSSL_PARAM_locate(params, OSSL_PROV_PARAM_BUILDINFO);
	if (p != NULL && !OSSL_PARAM_set_utf8_ptr(p, "Secure Element TRNG Provider Build")) {
		return 0;
	}

	return 1;
}

static const OSSL_ALGORITHM *custom_query(void *provctx, int operation_id, int *no_cache)
{
	(void)(provctx);

	*no_cache = 0;
	switch (operation_id) {
	case OSSL_OP_RAND:
		return se_trng_rand_algs;
	default:
		return NULL;
	}
}

static const OSSL_DISPATCH custom_dispatch[] = { { OSSL_FUNC_PROVIDER_TEARDOWN, (void (*)(void))OSSL_LIB_CTX_free },
	                                         { OSSL_FUNC_PROVIDER_QUERY_OPERATION, (void (*)(void))custom_query },
	                                         { OSSL_FUNC_PROVIDER_GETTABLE_PARAMS,
	                                           (void (*)(void))se_trng_gettable_params },
	                                         { OSSL_FUNC_PROVIDER_GET_PARAMS, (void (*)(void))se_trng_get_params },
	                                         { 0, NULL } };

static int custom_rng_provider_init(const OSSL_CORE_HANDLE *handle, const OSSL_DISPATCH *in, const OSSL_DISPATCH **out,
                                    void **provctx)
{
	(void)(handle);
	(void)(in);

	OSSL_LIB_CTX *libctx = OSSL_LIB_CTX_new();
	if (libctx == NULL) {
		return 0;
	}
	*provctx = libctx;
	*out = custom_dispatch;
	return 1;
}

int OSSL_provider_init(const OSSL_CORE_HANDLE *handle, const OSSL_DISPATCH *in, const OSSL_DISPATCH **out,
                       void **provctx)
{
	return custom_rng_provider_init(handle, in, out, provctx);
}