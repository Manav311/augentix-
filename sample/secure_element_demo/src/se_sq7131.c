#define _GNU_SOURCE

#include "se_sq7131.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "se_utils.h"
#include "log.h"
#include "otp-agtx.h"
#include <selib/Core_SELib.h>

/**
 * @brief store the device secret in the secure element
 * @details
 * @param[in] device_secret
 * @param[in] size
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int store_device_secret_in_se(uint8_t *device_secret, size_t size)
{
	int ret = 0;
	uint8_t buffer[SLOT_36_LEN] = { 0 };

	/* Boundary checking */
	if (size > SLOT_36_LEN) {
		printf("[SE][ERR] The size of the device_secret exceeds the size of the data slot.\n");
		return -EINVAL;
	}

	/* Write slot */
	memcpy(buffer, device_secret, size);
	ret = Core_write_zone(WRITE_MODE_DATA_32, SLOT_36, 0, buffer);
	if (ret) {
		printf("[SE][ERR] Core_write_zone failed.\n");
		return ret;
	}
	printf("[SE][INFO] Write slot [%d]:\n", SLOT_36);
	print_hex(device_secret, size);

	/* Verify value */
	ret = Core_read_slot_zone(READ_MODE_DATA_ZONE_32, SLOT_36, 0, buffer);
	printf("[SE][INFO] Data in Slot %d:\n", SLOT_36);
	msg_print(buffer, SLOT_36_LEN);

	for (size_t i = 0; i < size; i++) {
		if (buffer[i] != device_secret[i]) {
			printf("[SE][ERR] Wrong value in slot\n");
			return -EINVAL;
		}
	}

	for (size_t i = size; i < SLOT_36_LEN; i++) {
		if (buffer[i] != 0) {
			printf("[SE][ERR] Wrong value in slot\n");
			return -EINVAL;
		}
	}

	return ret;
}

/**
 * @brief calculate the username and the password
 * @details
 * @param[in] device_id
 * @param[in] product_id
 * @param[out] username
 * @param[out] username_size
 * @param[out] password
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int get_username_and_password(unsigned char *device_id, unsigned char *product_id, unsigned char **username,
                                     size_t *username_size, unsigned char *password)
{
	unsigned char *password_stuff = NULL;
	size_t slen = 0;

	/* Calculate username
	 * 1. Let device ID be the username 
	 * */
	*username_size = strlen((char *)device_id) + 1; // Include space for '\0'
	*username = malloc(*username_size);
	if (*username == NULL) {
		printf("[SE][ERR] Failed to allocate memory for username\n");
		return -ENOMEM;
	}
	memcpy(*username, device_id, *username_size);

	/* Calculate password
	 * 1. Concate password stuff 
	 * 2. Calculate 
	 * */
	slen = strlen((char *)device_id) + strlen((char *)product_id) + 20; // 20 for "deviceId=,productId="
	password_stuff = malloc(slen);
	if (password_stuff == NULL) {
		printf("[SE][ERR] Failed to allocate memory for password_stuff\n");
		free(*username);
		return -ENOMEM;
	}

	sprintf((char *)password_stuff, "deviceId=%s,productId=%s", device_id, product_id);
	printf("[SE][INFO] Password stuff:\n");
	print_hex(password_stuff, slen);
	DBG("[SE][DBG] Password stuff string:\n%s\n", password_stuff);

	Core_sha_hmac(password_stuff, slen, SLOT_36, password, SHA_MODE_TARGET_INTERNALKEY);

	return 0;
}

/**
 * @brief execute the provision of secure storage
 * @details
 * @param[in]
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int secure_storage_provision(void)
{
	int ret = 0;
	int offset = 0;
	unsigned char uid[UID_LEN] = { 0 };
	unsigned char io_protection_key[SLOT_22_LEN] = { 0 };
	unsigned char internal_key[32] = { 0 };
	unsigned char secret_key[32] = { 0 };
	unsigned char nonce_output[32] = { 0 };
	unsigned char random_output[32] = { 0 };
	unsigned char sha256_input[96] = { 0 };
	unsigned char sha256_output[SHA256_LEN] = { 0 };
	unsigned char write_input[64] = { 0 };
#ifdef DEBUG
	unsigned char aes_in[32] = { 0 };
	unsigned char aes_out[32] = { 0 };
#endif

	/* Step 1: Get chip UID */
	ret = get_se_uid(uid);
	if (ret) {
		printf("[SE][ERR] Cannot get the UID of SE.\n");
		return ret;
	}

	/* Step 2: Generate Secret Key from Master Key and UID */
	/* Noted: use hardcoded value in this example */
	memcpy(secret_key, SECRET_KEY_VECTOR, sizeof(secret_key));
	printf("[SE][INFO] Secret Key:\n");
	print_hex(secret_key, sizeof(secret_key));

	/* Step 3: Write IO protection Key into slot22 */
	/* Noted: IO protection key can be any of value,
	 * and will not be used in runtime, it is only used for provisioning
	 * use hardcoded value in this example 
	 * */
	memcpy(io_protection_key, IO_PROTECTION_KEY_VECTOR, sizeof(io_protection_key));
	ret = Core_write_zone(WRITE_MODE_DATA_32, SLOT_22, 0, io_protection_key);
	if (ret) {
		printf("[SE][ERR] Core_write_zone failed\n");
		return ret;
	}
	printf("[SE][INFO] IO Protection Key:\n");
	print_hex(io_protection_key, sizeof(io_protection_key));

	/* Step 4: Encrypted Write Secret Key into slot08 */
	/* Step 4.1: Nonce */
	ret = Core_random(random_output);
	if (ret) {
		printf("[SE][ERR] Core_random failed\n");
		return ret;
	}
	ret = Core_nonce_rand(random_output, nonce_output);
	if (ret) {
		printf("[SE][ERR] Core_nonce_rand failed\n");
		return ret;
	}
	memset(sha256_input, 0, sizeof(sha256_input));
	offset = 0;
	memcpy(sha256_input + offset, nonce_output, sizeof(nonce_output));
	offset += sizeof(nonce_output);
	memcpy(sha256_input + offset, random_output, 20);
	offset += 20;
	memcpy(sha256_input + offset, "\x16\x00\x00", 3);
	offset += 3;
	ret = sha256_openssl(sha256_input, offset, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}
	memset(internal_key, 0, sizeof(internal_key));
	memcpy(internal_key, sha256_output, sizeof(internal_key));

#ifdef DEBUG
	/* Use AES to check internal key value is as expected */
	DBG("[SE][DBG] Current Internal Key:\n");
	print_hex(internal_key, sizeof(internal_key));
	ret = Core_aes(AES_MODE_ECB_16, 0xFF, 0x04, aes_in, aes_out);
#endif

	/* Step 4.2: Gendig */
	ret = Core_gendig(GENDIG_ZONE_DATA, 22 << 8, NULL, 0);
	if (ret) {
		printf("[SE][ERR] Core_gendig failed\n");
		return ret;
	}
	offset = 0;
	memset(sha256_input, 0, sizeof(sha256_input));
	memcpy(sha256_input + offset, io_protection_key, sizeof(io_protection_key));
	offset += sizeof(io_protection_key);
	memcpy(sha256_input + offset, "\x15\x02\x16\x00", 4);
	offset += 4;
	sha256_input[offset++] = uid[3];
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	memset(sha256_input + offset, 0, 25);
	offset += 25;
	memcpy(sha256_input + offset, internal_key, sizeof(internal_key));
	offset += sizeof(internal_key);
	ret = sha256_openssl(sha256_input, offset, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}
	memset(internal_key, 0, sizeof(internal_key));
	memcpy(internal_key, sha256_output, sizeof(internal_key));

#ifdef DEBUG
	/* Use AES to check internal key value is as expected */
	DBG("[SE][DBG] Current Internal Key:\n");
	print_hex(internal_key, sizeof(internal_key));
	ret = Core_aes(AES_MODE_ECB_16, 0xFF, 0x04, aes_in, aes_out);
#endif

	/* Step 4.3: Encrypted Write Secret Key into slot08
	 * Noted: The secret key is encrypted with the internal key by xor-ing the secret key with the the internal key
	 * */
	/* Step 4.3.1: Calculate MAC of the data to be written */
	memset(write_input, 0, sizeof(write_input));
	offset = 0;
	memcpy(sha256_input + offset, internal_key, sizeof(internal_key));
	offset += sizeof(internal_key);
	memcpy(sha256_input + offset, "\x12\x12\x08\x00", 4);
	offset += 4;
	sha256_input[offset++] = uid[3];
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	memset(sha256_input + offset, 0, 25);
	offset += 25;
	memcpy(sha256_input + offset, secret_key, sizeof(secret_key));
	offset += sizeof(secret_key);
	ret = sha256_openssl(sha256_input, offset, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}

	/* Step 4.3.2: Write to slot08 */
	offset = 0;
	for (size_t i = 0; i < sizeof(secret_key); i++) {
		write_input[offset++] = secret_key[i] ^ internal_key[i];
	}
	memcpy(write_input + offset, sha256_output, sizeof(sha256_output));
	offset += sizeof(sha256_output);
	ret = Core_write(WRITE_MODE_DATA_32, 8 << 8, write_input, offset);
	if (ret) {
		printf("[SE][ERR] Core_write failed\n");
		return ret;
	}

	return 0;
}

/**
 * @brief execute secure storage
 * @details
 * @param[in] is_encrypt 1: encrypt 0: decrypt
 * @param[in] input
 * @param[in] input_size
 * @param[out] ouput
 * @param[out] output_size
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int secure_storage_runtime(int is_encrypt, unsigned char *input, int *input_size, unsigned char **output,
                                  int *output_size)
{
	int ret;
	uint8_t kdfmode = KDF_MODE_ALG_HKDF | KDF_MODE_SOURCE_SLOT | KDF_MODE_TARGET_OUTPUT_ENC;
	uint8_t source_input = 0x08;
	uint8_t source_output = 0x00;
	unsigned char io_protection_key[SLOT_22_LEN] = { 0 };
	unsigned char din[8] = { 0 }; // detail + salt
	unsigned char out_data[32] = { 0 };
	unsigned char out_nonce[32] = { 0 };
	unsigned char sha256_input[48] = { 0 }; // io_protection_key + out_nonce[:16]
	unsigned char sha256_output[SHA256_LEN] = { 0 };
	unsigned char aeskey[AES_KEY_LEN] = { 0 };
	unsigned char iv[IV_LEN] = "0123456789012345";
	unsigned char detail[4] = { 0x04, 0x00, 0x08, 0x02 };
	unsigned char salt[4] = { 0x01, 0x02, 0x03, 0x04 };

	/* Initialize io_protection_key and din */
	memcpy(io_protection_key, IO_PROTECTION_KEY_VECTOR, sizeof(io_protection_key));
	memcpy(din, detail, sizeof(detail));
	memcpy(din + 4, salt, sizeof(salt));

	/* Key Derivation Function */
	ret = Core_kdf(kdfmode, (source_output << 8) | source_input, din, out_data, out_nonce);
	if (ret) {
		printf("[SE][ERR] Core_kdf failed\n");
		return ret;
	}

	/* SHA-256 Calculation */
	memset(sha256_input, 0x00, sizeof(sha256_input));
	memcpy(sha256_input, io_protection_key, sizeof(io_protection_key));
	memcpy(sha256_input + 32, out_nonce, 16);

	ret = sha256_openssl(sha256_input, sizeof(sha256_input), sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}

	/* XOR calculation for AES key */
	for (size_t i = 0; i < sizeof(aeskey); i++) {
		aeskey[i] = out_data[i] ^ sha256_output[i];
	}
	printf("[SE][INFO] aeskey:\n");
	print_hex(aeskey, AES_KEY_LEN);

	/* Call appropriate function based on is_encrypt */
	if (is_encrypt) {
		return encrypt_data(input, input_size, output, output_size, aeskey, iv);
	} else {
		return decrypt_data(input, input_size, output, output_size, aeskey, iv);
	}
}

/**
 * @brief Read the value of the counter
 * @details
 * @param[in] counter_id
 * @param[out] counter_val
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int read_counter(const uint8_t counter_id, uint32_t *counter_val)
{
	int ret = 0;
	unsigned char out_counter[COUNTER_OUT_SIZE];
	uint8_t cid = (counter_id == 1 ? COUNTERS_ONE : COUNTERS_TWO);

	ret = Core_counter(COUNTER_READ, cid, out_counter);
	if (ret) {
		printf("[SE][ERR] Core_counter failed\n");
		return ret;
	}
#ifdef DEBUG
	DBG("[SE][DBG] out_counter: \n");
	msg_print(out_counter, sizeof(out_counter));
#endif

	for (int i = 0; i < COUNTER_OUT_SIZE; i++) {
		*counter_val |= (out_counter[i] << ((COUNTER_OUT_SIZE - 1 - i) * 8));
	}

	return 0;
}

/**
 * @brief Increase the value of the counter
 * @details
 * @param[in] counter_id
 * @param[in] desired_counter_val
 * @param[out] counter_val
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int increase_counter(const uint8_t counter_id, const uint32_t desired_counter_val, uint32_t *counter_val)
{
	int ret = 0;
	uint32_t increase_num = 0;
	uint32_t origin_counter_val = 0;
	unsigned char out_counter[COUNTER_OUT_SIZE];
	uint8_t cid = (counter_id == 1 ? COUNTERS_ONE : COUNTERS_TWO);

	ret = read_counter(counter_id, &origin_counter_val);
	if (ret) {
		printf("[SE][ERR] read_counter failed\n");
		return ret;
	}

	DBG("[SE][DBG] origin counter: %d\n", origin_counter_val);
	DBG("[SE][DBG] desired_counter_val: %d\n", desired_counter_val);
	if (desired_counter_val < origin_counter_val) {
		printf("[SE][ERR] Increase value should be positive\n");
		return -EINVAL;
	} else if (desired_counter_val == origin_counter_val) {
		printf("[SE][INFO] The counter value matches the desired value; no increment required\n");
		*counter_val = desired_counter_val;
		return 0;
	} else {
		increase_num = desired_counter_val - origin_counter_val;
	}

	DBG("[SE][DBG] increase_num: %d\n", increase_num);
	for (uint32_t i = 0; i < increase_num; i++) {
		ret = Core_counter(COUNTER_INCREASE, cid, out_counter);
		if (ret) {
			printf("[SE][ERR] Core_counter failed\n");
			return ret;
		}
	}
#ifdef DEBUG
	DBG("[SE][DBG] out_counter: \n");
	msg_print(out_counter, sizeof(out_counter));
#endif

	for (int i = 0; i < COUNTER_OUT_SIZE; i++) {
		*counter_val |= (out_counter[i] << ((COUNTER_OUT_SIZE - 1 - i) * 8));
	}

	return 0;
}

/**
 * @brief execute the demonstration of secure element
 * @details
 * @param[in] case_type
 * @param[in] meta_data
 * @arg
 * @see
 * @return result of the demonstration
 * @retval 0		demonstrate success
 * @retval not 0	demonstrate fail
 */
int SE_executeDemo(const CaseType case_type, const Meta_data meta_data)
{
	int ret = 0;
	int input_size = 0;
	int output_size = 0;
	size_t read_size = 0;
	uint8_t uid[UID_LEN] = { 0 };
	uint8_t key[SHA256_LEN] = { 0 };
	uint32_t current_counter_val = 0;
	unsigned char *product_id = NULL;
	unsigned char *device_id = NULL;
	unsigned char *encrypted_device_secret = NULL;
	unsigned char *device_secret = NULL;
	unsigned char *username = NULL;
	unsigned char *plaintext = NULL;
	unsigned char password[HMAC_SHA256_LEN];
	Slot_info slot_info = { 0 };

	/* Initialize libse */
	ret = securityLib_init();
	if (ret) {
		printf("[SE][ERR] Failed to initialize the secure element library.\n");
		return ret;
	}
	DBG("[SE][DBG] The secure element library is initialized.\n");

	/* Set I2C bus ID */
	ret = securityLib_setI2CBus(meta_data.i2c_bus_id);
	if (ret) {
		printf("[SE][ERR] Failed to set the I2C bus ID.\n");
		return ret;
	}
	printf("[SE][INFO] I2C bus ID = %d\n", meta_data.i2c_bus_id);

	DBG("[SE][DBG] Select case type = %d\n", case_type);
	switch (case_type) {
	case CASE_DEVICE_SECRET_FLASH:
		printf("[SE][INFO] Start to encrypt the device secret.\n");
		/* Check required file */
		if (meta_data.f_device_secret == NULL) {
			printf("[SE][ERR] File of device secret cannot be NULL.\n");
			return -ENOENT;
		}

		/* Prepare input */
		ret = read_file(meta_data.f_device_secret, &device_secret, &read_size);
		if (ret) {
			printf("[SE][ERR] Failed to read device secret file.\n");
			return ret;
		}
		DBG("[SE][DBG] Device secret: %s\n", device_secret);

		/* Store encrypted device secret in flash */
		input_size = (int)read_size;
		ret = secure_storage_runtime(1, device_secret, &input_size, &encrypted_device_secret, &output_size);
		if (ret) {
			printf("[SE][ERR] Failed to execute secure storage runtime\n");
			free(device_secret);
			return ret;
		}

		printf("[SE][INFO] Encrypted device secret:\n");
		print_hex(encrypted_device_secret, output_size);

		FILE *file = fopen(meta_data.f_encrypted_device_secret, "wb");
		if (file == NULL) {
			printf("[SE][ERR] Failed to open file\n");
			free(device_secret);
			return -ENFILE;
		}

		fwrite(encrypted_device_secret, 1, output_size, file);
		fclose(file);

		/* free resource */
		free(device_secret);

		printf("[SE][INFO] Encrypt the device secret end.\n");

		break;
	case CASE_DEVICE_SECRET_SE:
		printf("[SE][INFO] Start to store the device secret in the Secure Element slot %d.\n", SLOT_36);

		/* Check required file */
		if (meta_data.f_device_secret == NULL) {
			printf("[SE][ERR] File of device secret cannot be NULL.\n");
			return -ENOENT;
		}

		/* Prepare input */
		ret = read_file(meta_data.f_device_secret, &device_secret, &read_size);
		if (ret) {
			printf("[SE][ERR] Failed to read device secret file.\n");
			return -ENOENT;
		}
		DBG("[SE][DBG] Device secret string:\n%s\n", device_secret);

		/* Store device secret in secure element */
		ret = store_device_secret_in_se(device_secret, read_size);
		if (ret != 0) {
			printf("[SE][ERR] Failed to store the device secret in the secure element.\n");
			free(device_secret);
			return ret;
		}

		/* free resource */
		free(device_secret);

		printf("[SE][INFO] Store the device secret in the Secure Element end.\n");

		break;
	case CASE_DECRYPTED_DEVICE_SECRET_FLASH:
		printf("[SE][INFO] Start to decrypt the device secret.\n");

		/* Check required file */
		if (meta_data.f_encrypted_device_secret == NULL) {
			printf("[SE][INFO] Using device secret in secure element.\n");
		} else {
			printf("[SE][INFO] Using device secret in flash.\n");
			printf("[SE][INFO] Encrypted device secret file: %s to %s\n",
			       meta_data.f_encrypted_device_secret, meta_data.f_device_secret);
		}

		if (meta_data.f_encrypted_device_secret != NULL) {
			/* Read encrypted device secret file */
			ret = read_file(meta_data.f_encrypted_device_secret, &encrypted_device_secret, &read_size);
			if (ret != 0) {
				printf("[SE][ERR] Failed to read encrypted device secret file\n");
				free(product_id);
				free(device_id);
				return -ENOENT;
			}
			printf("[SE][INFO] Encrypted device secret:\n");
			print_hex(encrypted_device_secret, read_size);

			/* Decrypt device secret */
			input_size = (int)read_size;
			ret = secure_storage_runtime(0, encrypted_device_secret, &input_size, &device_secret,
			                             &output_size);
			if (ret) {
				printf("[SE][ERR] Failed to execute secure storage runtime\n");
				free(product_id);
				free(device_id);
				return ret;
			}
			DBG("[SE][DBG] Device secret string:\n%s\n", device_secret);

			file = fopen(meta_data.f_device_secret, "wb");
			if (file == NULL) {
				printf("[SE][ERR] Failed to open file\n");
				free(device_secret);
				return -ENFILE;
			}

			fwrite(device_secret, 1, output_size, file);
			fclose(file);

			/* free resource */
			free(device_secret);
			free(encrypted_device_secret);
		}

		printf("[SE][INFO] decrypt the device secret end.\n");
		break;
	case CASE_AUTHENTICATION:
		printf("[SE][INFO] Start to create the username and password.\n");

		/* Check required file */
		if (meta_data.f_product_id == NULL) {
			printf("[SE][ERR] File of product id cannot be NULL.\n");
			return -ENOENT;
		}
		printf("[SE][INFO] Product id file: %s\n", meta_data.f_product_id);

		if (meta_data.f_device_id == NULL) {
			printf("[SE][ERR] File of device id cannot be NULL.\n");
			return -ENOENT;
		}
		printf("[SE][INFO] Device id file: %s\n", meta_data.f_device_id);

		if (meta_data.f_encrypted_device_secret == NULL) {
			printf("[SE][INFO] Using device secret in secure element.\n");
		} else {
			printf("[SE][INFO] Using device secret in flash.\n");
			printf("[SE][INFO] Device secret file: %s\n", meta_data.f_encrypted_device_secret);
		}

		/* Prepare input
		 * 1. Product ID
		 * 2. Device ID 
		 * 3. Device secret from secure element or flash
		 * */
		ret = read_file(meta_data.f_product_id, &product_id, &read_size);
		if (ret) {
			printf("[SE][ERR] Failed to read product id file.\n");
			return -ENOENT;
		}
		printf("[SE][INFO] Product ID:\n");
		print_hex(product_id, read_size);

		ret = read_file(meta_data.f_device_id, &device_id, &read_size);
		if (ret) {
			printf("[SE][ERR] Failed to read device id file.\n");
			return -ENOENT;
		}
		printf("[SE][INFO] Device ID:\n");
		print_hex(device_id, read_size);

		if (meta_data.f_encrypted_device_secret != NULL) {
			/* Read encrypted device secret file */
			ret = read_file(meta_data.f_encrypted_device_secret, &encrypted_device_secret, &read_size);
			if (ret != 0) {
				printf("[SE][ERR] Failed to read encrypted device secret file\n");
				free(product_id);
				free(device_id);
				return -ENOENT;
			}
			printf("[SE][INFO] Encrypted device secret:\n");
			print_hex(encrypted_device_secret, read_size);

			/* Decrypt device secret */
			input_size = (int)read_size;
			ret = secure_storage_runtime(0, encrypted_device_secret, &input_size, &device_secret,
			                             &output_size);
			if (ret) {
				printf("[SE][ERR] Failed to execute secure storage runtime\n");
				free(product_id);
				free(device_id);
				return ret;
			}
			DBG("[SE][DBG] Device secret string:\n%s\n", device_secret);

			/* Store device secret in secure element */
			if (output_size > SLOT_36_LEN) {
				printf("[SE][ERR] Data length exceeds SLOT_36 space\n");
				free(product_id);
				free(device_id);
				free(device_secret);
				return -ENOMEM;
			}
			ret = store_device_secret_in_se(device_secret, output_size);
			if (ret != 0) {
				printf("[SE][ERR] Failed to store the device secret in the secure element.\n");
				free(product_id);
				free(device_id);
				free(device_secret);
				return ret;
			}
			printf("[SE][INFO] Device secret is stored in the secure element slot: %d\n", SLOT_36);
		}

		/* Get username and password */
		ret = get_username_and_password(device_id, product_id, &username, &read_size, password);
		if (ret != 0) {
			printf("[SE][ERR] Failed to get username and password.\n");
			free(product_id);
			free(device_id);
			free(device_secret);
			return ret;
		}

		/* Print username and password on screen */
		printf("\n[SE][NOTICE] For demonstration purposes only: passwords should not be printed under normal circumstances.\n");
		printf("[SE][INFO] Username:\n");
		print_hex(username, read_size);
		printf("[SE][INFO] Password:\n");
		print_hex(password, HMAC_SHA256_LEN);

		/* free resource */
		free(product_id);
		free(device_id);
		free(device_secret);
		free(username);

		printf("[SE][INFO] Create the username and password end.\n");

		break;
	case CASE_SECURE_STORAGE_PROVISION:
		printf("[SE][INFO] Start to store the IO Protection Key and Secret Key in the Secure Element.\n");
		ret = secure_storage_provision();
		if (ret) {
			printf("[SE][ERR] Failed to do the provision of secure storage\n");
			return ret;
		}
		printf("[SE][INFO] Store the IO Protection Key and Secret Key in the Secure Element end.\n");

		break;
	case CASE_UID:
		ret = get_se_uid(uid);
		if (ret) {
			printf("[SE][ERR] Cannot get the UID of SE\n");
			return ret;
		}
		printf("[SE][INFO] UID of the secure element:\n");
		print_hex(uid, sizeof(uid));
		break;
	case CASE_COUNTER_READ:
		ret = read_counter(meta_data.counter_id, &current_counter_val);

		/* NOTE: The following log is recognized by sysupd-anti-rollback-se and S000anti_rollback.
		 * Revise with caution if modifications are necessary. */
		if (ret) {
			printf("[SE][ERR] Cannot get the value of counter\n");
			return ret;
		} else {
			printf("[SE][INFO] Counter value: %d\n", current_counter_val);
		}
		break;
	case CASE_COUNTER_INCREASE:
		ret = increase_counter(meta_data.counter_id, meta_data.desired_counter_val, &current_counter_val);

		/* NOTE: The following log is recognized by sysupd-anti-rollback-se and S000anti_rollback.
		 * Revise with caution if modifications are necessary. */
		if (ret) {
			printf("[SE][ERR] Cannot increase the counter\n");
			return ret;
		} else {
			printf("[SE][INFO] Counter value: %d\n", current_counter_val);
		}
		break;
	case CASE_KEY_PROVISION:
		/* Check required file */
		if (meta_data.f_plaintext == NULL) {
			printf("[SE][ERR] APP magic num cannot be NULL.\n");
			return -ENOENT;
		}
		printf("[SE][INFO] APP magic num file: %s\n", meta_data.f_plaintext);

		/* Prepare input */
		ret = read_file(meta_data.f_plaintext, &plaintext, &read_size);
		if (ret) {
			printf("[SE][ERR] Failed to read APP magic num file.\n");
			return -ENOENT;
		}

		/* Get slot information */
		get_slot_info_by_id(meta_data.slot_id, &slot_info);

		if (slot_info.slot_type == NULL) {
			printf("[SE][ERR] The provided Slot %d is not recognized or supported in this example.\n",
			       meta_data.slot_id);
			return -ENOEXEC;
		} else {
			printf("[SE][INFO] Slot ID:%d Bytes:%d Writeable:%d Readable:%d Unlocked:%d Loackable:%d Type:%s\n",
			       meta_data.slot_id, slot_info.bytes, slot_info.is_writeable, slot_info.is_readable,
			       slot_info.is_unlocked, slot_info.is_lockable, slot_info.slot_type);
		}

		/* Key provision */
		ret = gen_io_protection_key(plaintext, read_size, key);
		if (ret) {
			printf("[SE][ERR] Failed to generate io protection key\n");
			return ret;
		}

		ret = write_key(meta_data.slot_id, key);
		if (ret) {
			printf("[SE][ERR] Failed to do key provision.\n");
			return ret;
		}
		printf("[SE][INFO] Key provisioning completed. The key has been securely stored in slot %d.\n",
		       meta_data.slot_id);
		break;
	case CASE_LOCK_SLOT:
		/* Get slot information */
		get_slot_info_by_id(meta_data.slot_id, &slot_info);

		if (slot_info.slot_type == NULL) {
			printf("[SE][ERR] The provided Slot %d is not recognized or supported in this example.\n",
			       meta_data.slot_id);
			return -ENOEXEC;
		} else {
			printf("[SE][INFO] Slot ID:%d Bytes:%d Writeable:%d Readable:%d Unlocked:%d Loackable:%d Type:%s\n",
			       meta_data.slot_id, slot_info.bytes, slot_info.is_writeable, slot_info.is_readable,
			       slot_info.is_unlocked, slot_info.is_lockable, slot_info.slot_type);
		}

		/* NOTE: To avoid unintentionally locking other slots, we limit the use to SLOT_37 (Genral Data) only. */
		if (meta_data.slot_id != SLOT_37) {
			printf("[SE][ERR] For demonstration purposes, only SLOT_37 (General Data) can be locked.\n");
			return -ENOEXEC;
		}

		/* Lock slot */
		if (slot_info.is_lockable == FALSE) {
			printf("[SE][ERR] Slot %d is not lockable. Stop lock slot.\n", meta_data.slot_id);
			return -ENOEXEC;
		}

		if (slot_info.is_unlocked == FALSE) {
			printf("[SE][ERR] Slot %d is already locked. Stop lock slot.\n", meta_data.slot_id);
			return -ENOEXEC;
		}

		ret = lock_slot_by_id(meta_data.slot_id);
		if (ret) {
			printf("[SE][ERR] Failed to lock the slot.\n");
			return ret;
		}
		printf("[SE][INFO] Slot %d is locked permanently and cannot be unlocked.\n", meta_data.slot_id);
		break;
	case CASE_SLOT_STATUS:
		/* Get slot information */
		get_slot_info_by_id(meta_data.slot_id, &slot_info);

		if (slot_info.slot_type == NULL) {
			printf("[SE][ERR] The provided Slot %d is not recognized or supported in this example.\n",
			       meta_data.slot_id);
			return -ENOEXEC;
		} else {
			printf("[SE][INFO] Slot ID:%d Bytes:%d Writeable:%d Readable:%d Unlocked:%d Loackable:%d Type:%s\n",
			       meta_data.slot_id, slot_info.bytes, slot_info.is_writeable, slot_info.is_readable,
			       slot_info.is_unlocked, slot_info.is_lockable, slot_info.slot_type);
		}

		/* Show slot value */
		if (slot_info.is_readable) {
			ret = print_slot(meta_data.slot_id, slot_info.bytes);
			if (ret) {
				printf("[SE][ERR] Failed to read slot %d\n", meta_data.slot_id);
				return ret;
			}
		}
		break;
	default:
		printf("[SE][ERR] Invalid option\n");
		break;
	}

	return ret;
}
