#include "decrypt.h"

#include <string.h>
#include <openssl/evp.h>
#include <syslog.h>

const char *k_key = "RX-78GP03 Stamen";
const char *k_iv = "1234567890987654";

int decrypt(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext)
{
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		syslog(LOG_ERR, "EVP_CIPHER_CTX_new");
		return -1;
	}
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (const unsigned char *)k_key, (const unsigned char *)k_iv)) {
		syslog(LOG_ERR, "EVP_DecryptInit_ex");
		return -1;
	}
	int len, plaintext_len;
	if (1 != EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len)) {
		syslog(LOG_ERR, "EVP_DecryptUpdate");
		return -1;
	}
	plaintext_len = len;
	if (1 != EVP_DecryptFinal_ex(ctx, plaintext + len, &len)) {
		syslog(LOG_ERR, "EVP_DecryptFinal_ex");
		return -1;
	}
	plaintext_len += len;

	EVP_CIPHER_CTX_free(ctx);
	plaintext[plaintext_len] = 0;
	return plaintext_len;
}

int encrypt(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext)
{
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		syslog(LOG_ERR, "EVP_CIPHER_CTX_new");
		return -1;
	}
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_cbc(), NULL, (const unsigned char *)k_key, (const unsigned char *)k_iv)) {
		syslog(LOG_ERR, "EVP_EncryptInit_ex");
		return -1;
	}
	int len, ciphertext_len;
	if (1 != EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len)) {
		syslog(LOG_ERR, "EVP_EncryptUpdate");
		return -1;
	}
	ciphertext_len = len;
	if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + len, &len)) {
		syslog(LOG_ERR, "EVP_EncryptFinal_ex");
		return -1;
	}
	ciphertext_len += len;

	EVP_CIPHER_CTX_free(ctx);
	return ciphertext_len;
}
