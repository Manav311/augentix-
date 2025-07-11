#ifndef SE_UTILS_H_
#define SE_UTILS_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/pem.h>

#define DEBUG

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define UID_LEN 16
#define SHA256_LEN 32
#define AES_KEY_LEN 32
#define HMAC_SHA256_LEN 32
#define IV_LEN 16

#define DEVFILE "/dev/otp_agtx"

struct custom_field {
	uint32_t uuid[4];
};

typedef struct slot_info {
	uint16_t bytes;
	uint8_t is_writeable;
	uint8_t is_readable;
	uint8_t is_unlocked;
	uint8_t is_lockable;
	const char *slot_type;
} Slot_info;

typedef struct slot {
	uint8_t id;
	Slot_info slot_info;
} Slot;

void print_hex(const unsigned char *content, const size_t size);
int read_otp_uuid(struct custom_field *cf);
int read_file(const char *filename, unsigned char **content, size_t *read_size);
int get_se_uid(uint8_t *uid);
int sha256_openssl(const unsigned char *input, size_t input_len, unsigned char *output);
int encrypt_data(unsigned char *input, int *input_size, unsigned char **output, int *output_size, unsigned char *aeskey,
                 unsigned char *iv);
int decrypt_data(unsigned char *input, int *input_size, unsigned char **output, int *output_size, unsigned char *aeskey,
                 unsigned char *iv);
int lock_slot_by_id(const uint8_t slot_id);
int get_lock_status_by_id(const uint8_t slot_id, uint8_t *status);
void get_slot_info_by_id(const uint8_t slot_id, Slot_info *slot_info);
int print_slot(const uint8_t slot_id, const uint16_t bytes);
int gen_io_protection_key(const unsigned char *app_magic_num, const size_t app_magic_num_size, uint8_t *key);
int write_key(const int slot_id, uint8_t *key);
int gen_random(unsigned char *rand, const size_t rand_size);
int generate_der_file_from_internal_key(const unsigned char internal_publicKey[64], const char *output_file);
void generate_der_from_key(unsigned char *key, const char *output_file);

#endif /* SE_UTILS_H_ */
