#include "se_utils.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <openssl/rand.h>

#include "log.h"
#include "otp-agtx.h"
#include <selib/Core_SELib.h>

/* Slot table */
Slot slot_table[53] = { { 0, { 0, 0, 0, 0, 0, NULL } },
	                { 1, { 0, 0, 0, 0, 0, NULL } },
	                { 2, { 0, 0, 0, 0, 0, NULL } },
	                { 3, { 0, 0, 0, 0, 0, NULL } },
	                { 4, { 0, 0, 0, 0, 0, NULL } },
	                { 5, { 0, 0, 0, 0, 0, NULL } },
	                { 6, { 0, 0, 0, 0, 0, NULL } },
	                { 7, { 0, 0, 0, 0, 0, NULL } },
	                { 8, { 32, TRUE, FALSE, TRUE, TRUE, "Secret Key0" } },
	                { 9, { 32, TRUE, FALSE, TRUE, TRUE, "Secret Key1" } },
	                { 10, { 32, TRUE, FALSE, TRUE, TRUE, "Secret Key2" } },
	                { 11, { 32, TRUE, FALSE, TRUE, TRUE, "Secret Key3" } },
	                { 12, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key0" } },
	                { 13, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key1" } },
	                { 14, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key2" } },
	                { 15, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key3" } },
	                { 16, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key4" } },
	                { 17, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key5" } },
	                { 18, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key6" } },
	                { 19, { 32, TRUE, FALSE, TRUE, FALSE, "AES Key7" } },
	                { 20, { 0, 0, 0, 0, 0, NULL } },
	                { 21, { 0, 0, 0, 0, 0, NULL } },
	                { 22, { 32, TRUE, FALSE, TRUE, TRUE, "IO Protection Key" } },
	                { 23, { 0, 0, 0, 0, 0, NULL } },
	                { 24, { 0, 0, 0, 0, 0, NULL } },
	                { 25, { 0, 0, 0, 0, 0, NULL } },
	                { 26, { 0, 0, 0, 0, 0, NULL } },
	                { 27, { 0, 0, 0, 0, 0, NULL } },
	                { 28, { 0, 0, 0, 0, 0, NULL } },
	                { 29, { 0, 0, 0, 0, 0, NULL } },
	                { 30, { 0, 0, 0, 0, 0, NULL } },
	                { 31, { 0, 0, 0, 0, 0, NULL } },
	                { 32, { 0, 0, 0, 0, 0, NULL } },
	                { 33, { 0, 0, 0, 0, 0, NULL } },
	                { 34, { 0, 0, 0, 0, 0, NULL } },
	                { 35, { 0, 0, 0, 0, 0, NULL } },
	                { 36, { 32, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 37, { 32, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 38, { 32, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 39, { 32, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 40, { 32, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 41, { 32, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 42, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 43, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 44, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 45, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 46, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 47, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 48, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 49, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 50, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 51, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } },
	                { 52, { 512, TRUE, TRUE, TRUE, TRUE, "General Data" } } };

/**
 * @brief print hexadecimal data
 * @details
 * @param[in] content
 * @param[in] size
 * @arg
 * @see
 * @return
 * @retval 
 */
void print_hex(const unsigned char *content, const size_t size)
{
	for (size_t i = 0; i < size; i++) {
		printf("%02x", content[i]);
		if ((i + 1) % 2 == 0) {
			printf(" ");
		}
		if ((i + 1) % 16 == 0) {
			printf("\n");
		}
	}
	if (size % 16 != 0) {
		printf("\n");
	}
}

/**
 * @brief read OTP area of UUID
 * @details
 * @param[in] cf
 * @arg
 * @see
 * @retval 0            success.
 * @retval not 0        fail.
 */
int read_otp_uuid(struct custom_field *cf)
{
	int fd, ret;

	fd = open(DEVFILE, O_RDWR);
	if (fd == -1) {
		perror("open");
		return -1;
	}

	/* Read otp area of UUID */
	memset(cf, 0, sizeof((struct otp_user *)cf));

	ret = ioctl(fd, OTP_READ_USER, (struct otp_user *)cf);
	if (ret == -1) {
		perror("ioctl");
		return -1;
	}

	return 0;
}

/**
 * @brief read file
 * @details
 * @param[in] filename
 * @param[in] content
 * @param[in] read_size
 * @arg
 * @see
 * @return read result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int read_file(const char *filename, unsigned char **content, size_t *read_size)
{
	int ret = 0;
	long file_size = 0;
	FILE *file = NULL;

	file = fopen(filename, "rb");
	if (!file) {
		printf("[SE][ERR] Failed to open file\n");
		return -EACCES;
	}

	fseek(file, 0, SEEK_END);
	file_size = ftell(file);
	if (file_size < 0) {
		printf("[SE][ERR] Failed to get the file size\n");
		fclose(file);
		return -EBADF;
	}
	rewind(file);

	*content = malloc(file_size + 1); // Include space for '\0'
	if (*content == NULL) {
		printf("[SE][ERR] Failed to allocate memory\n");
		return -ENOMEM;
	}

	*read_size = fread(*content, 1, file_size, file);
	if (*read_size != (size_t)file_size) {
		printf("[SE][ERR] Failed to read the complete file\n");
		free(*content);
		fclose(file);
		return -EBADF;
	}
	(*content)[file_size] = '\0';

	fclose(file);

	return ret;
}

/**
 * @brief get secure element ID
 * @details
 * @param[in] uid
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int get_se_uid(uint8_t *uid)
{
	int ret = 0;

	ret = Core_info_get_uid(uid);
	if (ret) {
		printf("[SE][ERR] Core_info_get_uid failed.\n");
	}
	return ret;
}

/**
 * @brief SHA256 Openssl
 * @details
 * @param[in] input
 * @param[in] input_len
 * @param[in] output
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int sha256_openssl(const unsigned char *input, size_t input_len, unsigned char *output)
{
	EVP_MD_CTX *mdctx = NULL;
	unsigned int output_len = 0;

	if ((mdctx = EVP_MD_CTX_new()) == NULL) {
		return -1;
	}

	if (1 != EVP_DigestInit_ex(mdctx, EVP_sha256(), NULL)) {
		EVP_MD_CTX_free(mdctx);
		return -1;
	}

	if (1 != EVP_DigestUpdate(mdctx, input, input_len)) {
		EVP_MD_CTX_free(mdctx);
		return -1;
	}

	if (1 != EVP_DigestFinal_ex(mdctx, output, &output_len)) {
		EVP_MD_CTX_free(mdctx);
		return -1;
	}

	/* Free resource */
	EVP_MD_CTX_free(mdctx);

	return 0;
}

/**
 * @brief data encryption
 * @details
 * @param[in] input
 * @param[in] input_size
 * @param[in] ouput
 * @param[in] output_size
 * @param[in] aeskey
 * @param[in] iv
 * @arg
 * @see
 * @return execution result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int encrypt_data(unsigned char *input, int *input_size, unsigned char **output, int *output_size, unsigned char *aeskey,
                 unsigned char *iv)
{
	EVP_CIPHER_CTX *ctx;
	int block_size;
	int ciphertext_len = 0;
	int len = 0;

	/* Initialize context */
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		printf("[SE][ERR] EVP_CIPHER_CTX initialization error\n");
		return -1;
	}

	/* Calculate the size needed for ciphertext */
	block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
	*output_size = (*input_size + block_size) / block_size * block_size; // Padding size
	*output = malloc(*output_size + block_size); // Extra space for padding
	if (*output == NULL) {
		printf("[SE][ERR] Failed to allocate memory for output\n");
		EVP_CIPHER_CTX_free(ctx);
		return -ENOMEM;
	}

	/* Perform encryption */
	if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aeskey, iv) ||
	    1 != EVP_EncryptUpdate(ctx, *output, &ciphertext_len, input, *input_size) ||
	    1 != EVP_EncryptFinal_ex(ctx, *output + ciphertext_len, &len)) {
		printf("[SE][ERR] Encryption error\n");
		free(*output);
		EVP_CIPHER_CTX_free(ctx);
		return -1;
	} else {
		ciphertext_len += len;
		*output_size = ciphertext_len; // Update the output size
	}

	/* Free resources */
	EVP_CIPHER_CTX_free(ctx);

	return 0;
}

/**
 * @brief data decryption
 * @details
 * @param[in] input
 * @param[in] input_size
 * @param[in] ouput
 * @param[in] output_size
 * @param[in] aeskey
 * @param[in] iv
 * @arg
 * @see
 * @return execution result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int decrypt_data(unsigned char *input, int *input_size, unsigned char **output, int *output_size, unsigned char *aeskey,
                 unsigned char *iv)
{
	EVP_CIPHER_CTX *ctx;
	int block_size;
	int decryptedtext_len = 0;
	int len = 0;

	/* Initialize context */
	ctx = EVP_CIPHER_CTX_new();
	if (!ctx) {
		printf("[SE][ERR] EVP_CIPHER_CTX initialization error\n");
		return -EINVAL;
	}

	/* Allocate memory for decrypted text */
	block_size = EVP_CIPHER_block_size(EVP_aes_256_cbc());
	*output_size = (*input_size + block_size) / block_size * block_size; // Padding size
	*output = malloc(*output_size + block_size); // Extra space for padding
	if (*output == NULL) {
		printf("[SE][ERR] Failed to allocate memory for output\n");
		EVP_CIPHER_CTX_free(ctx);
		return -ENOMEM;
	}

	/* Perform decryption */
	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, aeskey, iv) ||
	    1 != EVP_DecryptUpdate(ctx, *output, &decryptedtext_len, input, *input_size) ||
	    1 != EVP_DecryptFinal_ex(ctx, *output + decryptedtext_len, &len)) {
		printf("[SE][ERR] Decryption error\n");
		free(*output);
		EVP_CIPHER_CTX_free(ctx);
		return -ENOMEM;
	} else {
		decryptedtext_len += len;
		*output_size = decryptedtext_len;
		(*output)[*output_size] = '\0'; // Null-terminate
	}

	/* Free resources */
	EVP_CIPHER_CTX_free(ctx);

	return 0;
}

/**
 * @brief execute slot lock
 * @details
 * @param[in] slot_id
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int lock_slot_by_id(const uint8_t slot_id)
{
	int ret = 0;

	ret = Core_lock_data_slot(slot_id);
	if (ret) {
		printf("[SE][ERR] Core_lock_data_slot failed\n");
		return ret;
	}

	return ret;
}

/**
 * @brief check lock status of slot
 * @details
 * @param[in] slot_id
 * @param[in] status
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
int get_lock_status_by_id(const uint8_t slot_id, uint8_t *status)
{
	int ret = 0;
	uint8_t all_status[7] = { 0 };
	const int max_index = sizeof(all_status) - 1;

	if (slot_id < 1 || slot_id > 52) {
		printf("[SE][ERR] Slot ID %d is out of range\n", slot_id);
		return -1;
	}

	int byte_index = max_index - (slot_id / 8);
	int bit_index = slot_id % 8;

	ret = Core_info_get_keyLock(0xFF, all_status);
	if (ret) {
		printf("[SE][ERR] Core_info_get_keyLock failed with error code: %d\n", ret);
		return ret;
	}

	DBG("[SE][DBG] All key lock status: ");
	print_hex(all_status, sizeof(all_status));

	*status = (all_status[byte_index] >> bit_index) & 1;
	DBG("[SE][DBG] Slot %d status: %d\n", slot_id, *status);

	return 0;
}

/**
 * @brief get slot information from slot table
 * @details
 * @param[in] slot_id
 * @param[in] slot_info
 * @arg
 * @see
 * @return result of get slot information execution
 * @retval 0		execution success
 * @retval not 0	execution fail
 */
void get_slot_info_by_id(const uint8_t slot_id, Slot_info *slot_info)
{
	int ret = 0;

	if (slot_id < 1 || slot_id > 52) {
		printf("[SE][INFO] slot_id out of range: %d\n", slot_id);
		slot_info = NULL;
		return;
	}

	if (slot_table[slot_id].id != slot_id) {
		printf("[SE][ERR] slot_id %d does not match the id in the slot table[%d]\n", slot_id,
		       slot_table[slot_id].id);
		slot_info = NULL;
		return;
	}

	DBG("[SE][DBG] Slot ID: %d\n", slot_table[slot_id].id);
	ret = get_lock_status_by_id(slot_table[slot_id].id, &slot_table[slot_id].slot_info.is_unlocked);
	if (ret) {
		printf("[SE][ERR] Failed to get lock status.\n");
		slot_info = NULL;
		return;
	}

	if (slot_table[slot_id].slot_info.is_unlocked == FALSE) { // if the slot is locked
		slot_table[slot_id].slot_info.is_writeable = FALSE;
	}
	*slot_info = slot_table[slot_id].slot_info;
}

/**
 * @brief print slot data
 * @details
 * @param[in] slot_id
 * @param[in] bytes
 * @arg
 * @see
 * @return result of the print
 * @retval 0		print success
 * @retval not 0	print fail
 */
int print_slot(const uint8_t slot_id, const uint16_t bytes)
{
	int ret = 0;
	unsigned char buffer[512] = { 0 };

	for (int i = 0; i < (bytes / 32); i++) {
		ret = Core_read_slot_zone(READ_MODE_DATA_ZONE_32, slot_id, i, buffer + (i * 32));
		if (ret) {
			printf("[SE][ERR] Slot read error\n");
			return ret;
		}
	}
	printf("[SE][INFO] Data in Slot %d:\n", slot_id);
	msg_print(buffer, bytes);

	return 0;
}

/**
 * @brief generate IO PROTECTION KEY
 * @details
 * @param[in] app_magic_num
 * @param[in] app_magic_num_size
 * @param[out] key
 * @arg
 * @see
 * @return result of the generation
 * @retval 0		success
 * @retval not 0	fail
 */
int gen_io_protection_key(const unsigned char *app_magic_num, const size_t app_magic_num_size, uint8_t *key)
{
	int ret;
	struct otp_user otp_user;
	struct custom_field *cf = (struct custom_field *)&otp_user;
	uint8_t se_uid[UID_LEN] = { 0 };
	uint8_t soc_uuid_hash[SHA256_LEN] = { 0 };
	size_t concat_len = sizeof(se_uid) + sizeof(soc_uuid_hash) + app_magic_num_size;
	size_t offset = 0;
	uint8_t concatenated[concat_len];

	/* Get SE_UID */
	ret = get_se_uid(se_uid);
	if (ret) {
		printf("[SE][ERR] Cannot get the UID of SE.\n");
		return ret;
	}
#ifdef DEBUG
	DBG("[SE][DBG] SE_UID:\n");
	msg_print(se_uid, sizeof(se_uid));
#endif

	/* Get SOC_UUID_HASH */
	ret = read_otp_uuid(cf);
	if (ret) {
		printf("[SE][ERR] Failed to read the UUID of SOC. Please insmod otp-agtx.ko.\n");
		return ret;
	}

#ifdef DEBUG
	for (size_t i = 0; i < sizeof(cf->uuid) / sizeof(int); i++) {
		DBG("[SE][DBG] cf->uuid[%d]: 0x%08x\n", i, cf->uuid[i]);
	}

	DBG("[SE][DBG] SOC_UUID:\n");
	print_hex((const unsigned char *)cf->uuid, UID_LEN);
#endif

	ret = sha256_openssl((const unsigned char *)cf->uuid, UID_LEN, soc_uuid_hash);
	if (ret != 0) {
		printf("[SE][ERR] SHA256 computation failed.\n");
		return ret;
	}

#ifdef DEBUG
	DBG("[SE][DBG] SOC_UUID_HASH:");
	print_hex(soc_uuid_hash, sizeof(soc_uuid_hash));

	/* Get APP_MAGIC_NUM */
	DBG("[SE][DBG] APP_MAGIC_NUM:\n");
	print_hex(app_magic_num, app_magic_num_size);
#endif

	/* Concatenate se_uid, soc_uuid_hash, and app_magic_num */
	memcpy(concatenated + offset, se_uid, sizeof(se_uid));
	offset += sizeof(se_uid);

	memcpy(concatenated + offset, soc_uuid_hash, sizeof(soc_uuid_hash));
	offset += sizeof(soc_uuid_hash);

	memcpy(concatenated + offset, app_magic_num, app_magic_num_size);

#ifdef DEBUG
	DBG("[SE][DBG] Concatenated data:\n");
	msg_print(concatenated, sizeof(concatenated));
#endif

	/* Compute the SHA256 hash of the concatenated values */
	ret = sha256_openssl(concatenated, concat_len, key);
	if (ret != 0) {
		printf("[SE][ERR] SHA256 computation failed.\n");
		return ret;
	}

#ifdef DEBUG
	DBG("[SE][DBG] SHA256 result:\n");
	msg_print(key, SHA256_LEN);
#endif

	return 0;
}

/**
 * @brief store IO PROTECTION KEY
 * @details
 * @param[in] slot ID
 * @param[in] key
 * @arg
 * @see
 * @return result of write
 * @retval 0		success
 * @retval not 0	fail
 */
int write_key(const int slot_id, uint8_t *key)
{
	int ret;

	/* Store the IO_PROTECTION_KEY_FOR_ENCRYPTED_CHANNEL in the slot */
	ret = Core_write_zone(WRITE_MODE_DATA_32, slot_id, 0, key);
	if (ret) {
		printf("[SE][ERR] Core_write_zone failed.\n");
		return ret;
	}
	DBG("[SE][DBG] Write slot [%d].\n", slot_id);

	return 0;
}

/**
 * @brief generate the random number
 * @details
 * @param[out] rand
 * @param[in] rand_size
 * @arg
 * @see
 * @return result of write
 * @retval 0		success
 * @retval not 0	fail
 */
int gen_random(unsigned char *rand, const size_t rand_size)
{
	if (RAND_bytes(rand, rand_size) != 1) {
		printf("[SE][ERR] Error generating random bytes\n");
		return -EPERM;
	}
	return 0;
}

/**
 * @brief generate the internal (SE) public key in DER
 * @details
 * @param[in] internal_publicKey
 * @param[in] output_file
 * @arg
 * @see
 * @return result of the generation
 * @retval 0		success
 * @retval not 0	fail
 */
int generate_der_file_from_internal_key(const unsigned char internal_publicKey[64], const char *output_file)
{
	// Define the curve OID and other constants
	const char *curve_oid = "06082A8648CE3D030107"; // Curve OID for prime256v1
	const char *id_ecpublickey = "06072A8648CE3D0201"; // id-ecPublicKey OID

	// Construct the public key in uncompressed format: 04 | x | y
	char uncompressed_key[130]; // 2 hex chars per byte (64 + 64 + 1 for the prefix)
	snprintf(uncompressed_key, sizeof(uncompressed_key), "04");

	// Convert x and y to hex and append to uncompressed_key
	for (int i = 0; i < 32; i++) {
		snprintf(uncompressed_key + 2 + i * 2, 3, "%02x", internal_publicKey[i]);
	}
	for (int i = 0; i < 32; i++) {
		snprintf(uncompressed_key + 66 + i * 2, 3, "%02x", internal_publicKey[32 + i]);
	}

	// Construct the ASN.1 structure manually (in hex)
	// SEQUENCE {
	//   SEQUENCE {
	//     OBJECT IDENTIFIER: id-ecPublicKey (1.2.840.10045.2.1)
	//     OBJECT IDENTIFIER: prime256v1 (1.2.840.10045.3.1.7)
	//   }
	//   BIT STRING: 04 | x | y
	// }

	// Components of the DER structure
	const char *header = "3059"; // Top-level SEQUENCE, length 57 (0x39 bytes)
	const char *algorithm_id = "3013"; // SEQUENCE, length 19 (0x13 bytes)
	const char *public_key_bitstring_prefix = "03"; // BIT STRING prefix
	const char *pad = "4200"; // Padding for BIT STRING

	// Create the final DER hex string
	char der_hex[1025]; // A buffer to hold the final DER hex string
	snprintf(der_hex, sizeof(der_hex), "%s%s%s%s%s%s%s", header, algorithm_id, id_ecpublickey, curve_oid,
	         public_key_bitstring_prefix, pad, uncompressed_key);

	// Print the DER hex string for debugging
	DBG("[SE][DBG] DER Hex String: %s\n", der_hex);

	// Convert the hex string to binary data
	size_t hex_len = strlen(der_hex);
	size_t bin_len = hex_len / 2;
	unsigned char *binary_data = (unsigned char *)malloc(bin_len);
	if (binary_data == NULL) {
		printf("[SE][ERR] Memory allocation failed.\n");
		return -1;
	}

	// Convert the hex string to binary data
	for (size_t i = 0; i < bin_len; i++) {
		sscanf(der_hex + 2 * i, "%2hhx", &binary_data[i]);
	}

	// Open the output file for writing binary data
	FILE *file = fopen(output_file, "wb");
	if (file == NULL) {
		printf("[SE][ERR] Error opening file for writing.\n");
		free(binary_data);
		return -1;
	}

	// Write the binary data to the file
	fwrite(binary_data, 1, bin_len, file);

	// Close the file
	fclose(file);

	// Clean up
	free(binary_data);

	// Confirmation
	printf("[SE][INFO] SE public DER file created: %s\n", output_file);
	return 0;
}
