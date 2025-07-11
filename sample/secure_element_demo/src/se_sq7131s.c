#define _GNU_SOURCE

#include "se_sq7131s.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "se_utils.h"
#include "log.h"
#include <selib/Core_SELib.h>

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
	int rc = 0;
	int all_ff = 1;
	uint8_t data_Out[32];

	memset(data_Out, 0x00, sizeof(data_Out));
	rc = Core_read_slot_zone(READ_MODE_DATA_ZONE_32, counter_id, 0, data_Out);
	if (rc) {
		printf("[SE][ERR] Core_read_slot_zone failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] counter value:");
	msg_print(data_Out, sizeof(data_Out));
#endif
	for (int i = 0; i < 32; i++) {
		if (data_Out[i] != 0xFF) {
			all_ff = 0;
			break;
		}
	}

	if (all_ff == 1) {
		*counter_val = 0;
		printf("[SE][INFO] Counter is in initial state (all bytes are 0xFF)\n");
	} else {
		*counter_val = (uint32_t)(data_Out[0]) | (uint32_t)(data_Out[1] << 8) | (uint32_t)(data_Out[2] << 16) |
		               (uint32_t)(data_Out[3] << 24);
	}

	return rc;
}

/**
 * @brief Update the value of the counter
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
static int update_counter(const uint8_t counter_id, const uint32_t desired_counter_val, uint32_t *counter_val)
{
	int rc = 0;
	int offset = 0;
	unsigned char uid[16];
	unsigned char internal_key[32];
	unsigned char inData[32];
	unsigned char ioKey[32];
	unsigned char app_magic_num[32];
	unsigned char nonce_output[32];
	unsigned char random_output[32];
	unsigned char sha256_input[96];
	unsigned char sha256_output[32];
	unsigned char write_input[64];
	unsigned char lock[4] = { 0 };
	unsigned char dataSlotZoneConfigState[2] = { 0 };
	uint8_t data_Out[32];

	// Update value
	memset(data_Out, 0x00, sizeof(data_Out));
	data_Out[0] = (uint8_t)(desired_counter_val & 0xFF);
	data_Out[1] = (uint8_t)((desired_counter_val >> 8) & 0xFF);
	data_Out[2] = (uint8_t)((desired_counter_val >> 16) & 0xFF);
	data_Out[3] = (uint8_t)((desired_counter_val >> 24) & 0xFF);

	// Check if IOProtectionKey is locked
	rc = Core_info_base(0x06, 0x1600, lock);
	if (rc) {
		printf("[SE][ERR] Core_info_base failed\n");
		return rc;
	}
	printf("[SE][INFO] lock: ");
	msg_print(lock + 3, 1);

	// Check if dataSlotZoneConfig is locked
	rc = Core_read_config_zone(READ_MODE_CONFIG_ZONE_2, 0x000d, dataSlotZoneConfigState);
	if (rc) {
		printf("[SE][ERR] Core_read_config_zone failed\n");
		return rc;
	}
	printf("[SE][INFO] dataSlotZoneConfigState: ");
	msg_print(dataSlotZoneConfigState + 1, 1);

	if (dataSlotZoneConfigState[1] == 0xFF) {
		rc = -1;
		printf("[SE][ERR] Please lock dataSlotZoneConfig failed\n");
		return rc;
	}

	// Get chip UID
	rc = Core_info_get_uid(uid);
	if (rc) {
		printf("[SE][ERR] Core_info_get_uid failed\n");
		return rc;
	}
	printf("[SE][INFO] UID: ");
	msg_print(uid, sizeof(uid));

	// Check if parameters are provided (counter_id, inputData, SoC IOProtectionKey)
	printf("[SE][INFO] counter_id: %d\n", counter_id);

	memset(inData, 0, sizeof(inData));
	memcpy(inData, data_Out, sizeof(data_Out));
	printf("[SE][INFO] inData: ");
	msg_print(inData, sizeof(inData));

	memcpy(app_magic_num, APP_MAGIC_NUM, sizeof(app_magic_num));
	rc = gen_io_protection_key(app_magic_num, sizeof(app_magic_num), ioKey);
	if (rc) {
		printf("[SE][ERR] gen_io_protectio_key failed\n");
		return rc;
	}
	printf("[SE][INFO] io_protection_key: ");
	msg_print(ioKey, 32);

	// Synchronize trng3 chip
	rc = Core_random(random_output);
	if (rc) {
		printf("[SE][ERR] Core_random failed\n");
		return rc;
	}
	rc = Core_nonce_rand(random_output, nonce_output);
	if (rc) {
		printf("[SE][ERR] Core_nonce_rand failed\n");
		return rc;
	}

	// Synchronize trng3 SoC
	memset(sha256_input, 0, sizeof(sha256_input));
	offset = 0;
	memcpy(sha256_input + offset, nonce_output, sizeof(nonce_output));
	offset += sizeof(nonce_output);
	memcpy(sha256_input + offset, random_output, 20);
	offset += 20;
	memcpy(sha256_input + offset, "\x16\x00\x00", 3);
	offset += 3;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		return rc;
	}
	memset(internal_key, 0, sizeof(internal_key));
	memcpy(internal_key, sha256_output, sizeof(internal_key));

	// Calculate trng4 chip
	rc = Core_gendig(GENDIG_ZONE_DATA, 22 << 8, NULL, 0);
	if (rc) {
		printf("[SE][ERR] Core_gendig failed\n");
		return rc;
	}

	// Calculate trng4 SoC
	offset = 0;
	memset(sha256_input, 0, sizeof(sha256_input));
	memcpy(sha256_input + offset, ioKey, 32);
	offset += 32;
	memcpy(sha256_input + offset, "\x15\x02\x16\x00", 4);
	offset += 4;
	sha256_input[offset++] = uid[3];
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	memset(sha256_input + offset, 0, 25);
	offset += 25;
	memcpy(sha256_input + offset, internal_key, sizeof(internal_key));
	offset += sizeof(internal_key);
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		return rc;
	}
	memset(internal_key, 0, sizeof(internal_key));
	memcpy(internal_key, sha256_output, sizeof(internal_key));

	// Calculate MAC for writing data
	memset(write_input, 0, sizeof(write_input));
	offset = 0;
	memcpy(sha256_input + offset, internal_key, sizeof(internal_key));
	offset += sizeof(internal_key);
	memcpy(sha256_input + offset, "\x12\x12", 2);
	offset += 2;
	sha256_input[offset++] = counter_id;
	memcpy(sha256_input + offset, "\00", 1);
	offset += 1;
	sha256_input[offset++] = uid[3];
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	memset(sha256_input + offset, 0, 25);
	offset += 25;
	memcpy(sha256_input + offset, inData, sizeof(inData));
	offset += sizeof(inData);
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		return rc;
	}

	// Calculate cipher for writing data
	offset = 0;
	for (size_t i = 0; i < sizeof(inData); i++) {
		write_input[offset++] = inData[i] ^ internal_key[i];
	}
	memcpy(write_input + offset, sha256_output, sizeof(sha256_output));
	offset += sizeof(sha256_output);

	// Write calculated CipherMAC to Slot
	rc = Core_write(WRITE_MODE_DATA_32, counter_id << 8, write_input, offset);
	if (rc) {
		printf("[SE][ERR] Core_write failed\n");
		return rc;
	}

	rc = read_counter(counter_id, counter_val);
	if (rc) {
		printf("[SE][ERR] read_counter failed\n");
		return rc;
	}
	if (*counter_val != desired_counter_val) {
		printf("[SE][ERR] Value mismatch\n");
		return -EINVAL;
	}

	return 0;
}

/**
 * @brief IO PROTECTION KEY provision which config is unlock
 * @details
 * @param[out] sharedKey
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int io_protection_key_provision_unlock(unsigned char **sharedKey)
{
	int rc = 0;
	int offset = 0;
	unsigned char uid[16];
	unsigned char testSharedKey[32];
	unsigned char soc_publicKey[64];
	unsigned char internal_publicKey[64];
	unsigned char chipOutMAC[32];
	unsigned char macchallenge[32];
	unsigned char sha256_input[96];
	unsigned char sha256_output[32];
	unsigned char lock[4] = { 0 };
	unsigned char dataSlotZoneConfigState[2];

	// Check if the IO Protection key is locked
	rc = Core_info_base(0x06, 0x1600, lock);
	if (rc) {
		printf("[SE][ERR] Core_info_base failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] lock: ");
	msg_print(lock + 3, 1);
#endif

	// Confirm if the data slot zone config is unlocked.
	// This example cannot be used if it is locked.
	memset(dataSlotZoneConfigState, 0x00, sizeof(dataSlotZoneConfigState));
	rc = Core_read_config_zone(READ_MODE_CONFIG_ZONE_2, 0x000d, dataSlotZoneConfigState);
	if (rc) {
		printf("[SE][ERR] Core_read_config_zone failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] dataSlotZoneConfigState: ");
	msg_print(dataSlotZoneConfigState + 1, 1);
#endif
	if (dataSlotZoneConfigState[1] != 0xFF) {
		rc = -1;
		printf("[SE][ERR] Please using unlock dataSlotZoneConfig\n");
		return rc;
	}

	// Get chip UID
	rc = get_se_uid(uid);
	if (rc) {
		printf("[SE][ERR] get_se_uid failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] UID: ");
	msg_print(uid, sizeof(uid));
#endif

	// generate SoC key pair
	int result = system(
	        "openssl genpkey -algorithm EC -pkeyopt ec_paramgen_curve:prime256v1 -out /tmp/soc_privateKey.der -outform DER");
	if (result == -1) {
		printf("Error executing system command.\n");
		return -1;
	}
	result = system(
	        "openssl pkey -in /tmp/soc_privateKey.der -inform DER -pubout -out /tmp/soc_publicKey.der -outform DER");
	if (result == -1) {
		printf("Error executing system command.\n");
		return -1;
	}

	FILE *file = fopen("/tmp/soc_publicKey.der", "rb");

	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	// Seek to the last 64 bytes
	fseek(file, -64, SEEK_END);

	// Read 64 bytes into the buffer
	size_t bytesRead = fread(soc_publicKey, 1, 64, file);
	fclose(file);

	if (bytesRead != 64) {
		printf("Failed to read 64 bytes (read %zu bytes)\n", bytesRead);
		return 1;
	}

#ifdef DEBUG
	DBG("[SE][DBG] soc_publicKey: ");
	msg_print(soc_publicKey, sizeof(soc_publicKey));
#endif

	// Generate a key pair (public and private) on the chip itself and store it in the internal key
	rc = Core_genkey(0xFF, CORE_ECCTARGET_P256, internal_publicKey);
	if (rc) {
		printf("[SE][ERR] Core_genkey internalkey failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] internal_publicKey: ");
	msg_print(internal_publicKey, sizeof(internal_publicKey));
#endif
	generate_der_file_from_internal_key(internal_publicKey, "/tmp/internal_publicKey.der");

	// Use the soc_publicKey and internal key to exchange and generate the shared key,
	// then store it in the IO Protection key (0x16)
	rc = Core_ecdh_base(ECDH_MODE_SPECIFIEDSLOT, 0x1600, soc_publicKey, NULL, NULL);
	if (rc) {
		printf("[SE][ERR] Core_ecdh_base store soc_publicKey failed\n");
		return rc;
	}

	// Use the IO Protection key on the chip to generate the MAC,
	// allowing the SoC to perform the verification
	memset(macchallenge, 0x01, 32);
	rc = Core_mac(MAC_MODE_CHALLENGE, 0x16, macchallenge, chipOutMAC);
	if (rc) {
		printf("[SE][ERR] Core_mac failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] chipOutMAC: ");
	msg_print(chipOutMAC, sizeof(chipOutMAC));
#endif

	// Get sharedKey
	result = system(
	        "openssl pkeyutl -derive -inkey /tmp/soc_privateKey.der -keyform DER -peerkey /tmp/internal_publicKey.der -peerform DER -out /usrdata/shared_secret.bin");
	if (result == -1) {
		printf("Error executing system command.\n");
		return -1;
	}

	file = fopen("/usrdata/shared_secret.bin", "rb");

	if (file == NULL) {
		perror("Error opening file");
		return 1;
	}

	fseek(file, 0, SEEK_SET);

	// Read 32 bytes into the buffer
	bytesRead = fread(testSharedKey, 1, 32, file);
	fclose(file);

	if (bytesRead != 32) {
		printf("Failed to read 64 bytes (read %zu bytes)\n", bytesRead);
		return 1;
	}

#ifdef DEBUG
	DBG("[SE][DBG] testSharedKey: ");
	msg_print(testSharedKey, sizeof(testSharedKey));
#endif

	// SoC calculates MAC
	memset(sha256_input, 0, sizeof(sha256_input));
	offset = 0;
	memcpy(sha256_input + offset, testSharedKey, 32);
	offset += sizeof(testSharedKey);
	memcpy(sha256_input + offset, macchallenge, sizeof(macchallenge));
	offset += sizeof(macchallenge);
	memcpy(sha256_input + offset, "\x08\x00\x16\x00", 4);
	offset += 15;
	sha256_input[offset++] = uid[3];
	offset += 4;
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	offset += 2;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		DBG("[SE][DBG] [SE][DBG] MAC failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] SoCMAC: ");
	msg_print(sha256_output, sizeof(sha256_output));
#endif

	// Compare SoC and SE MAC
	for (size_t i = 0; i < sizeof(sha256_output); i++) {
		if (sha256_output[i] != chipOutMAC[i]) {
			printf("[SE][ERR] MAC mismatch\n");
			return -EINVAL;
		}
	}

	printf("[SE][INFO] MAC OK!!!\n");

	*sharedKey = malloc(sizeof(testSharedKey));
	if (*sharedKey == NULL) {
		printf("[SE][ERR] Memory allocation failed\n");
		return -1;
	}
	memcpy(*sharedKey, testSharedKey, sizeof(testSharedKey));

	// Please make sure to perform Core_lock after generating the IOProtectionKey on your product side
	Core_lock(0x02, 0x0000); // DataSlotZoneConfig Lock 0x02 is the data slot zone config lock

	// Remove all key info
	result = system("rm -f /tmp/soc_privateKey.der");
	if (result == -1) {
		printf("Error executing system command: remove private key\n");
		return -1;
	}

	result = system("rm -f /tmp/soc_publicKey.der");
	if (result == -1) {
		printf("Error executing system command: remove public key\n");
		return -1;
	}

	result = system("rm -f /tmp/internal_publicKey.der");
	if (result == -1) {
		printf("Error executing system command: remove internal public key\n");
		return -1;
	}

	return 0;
}

/**
 * @brief IO PROTECTION KEY provision while the config is locked
 * @details NOTE: this should be executed right after io_protection_key_provision_unlock
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int io_protection_key_provision_lock(void)
{
	int rc = 0;
	int offset = 0;
	size_t size = 0;
	unsigned char io_protection_key_slot = 0x16;
	unsigned char *defaultecdh_io_protection_key;
	unsigned char uid[16];
	unsigned char app_magic_num[32];
	unsigned char internal_key[32];
	unsigned char new_ioKey[32];
	unsigned char nonce_output[32];
	unsigned char random_output[32];
	unsigned char sha256_input[96];
	unsigned char sha256_output[32];
	unsigned char write_input[64];
	unsigned char lock[4] = { 0 };
	unsigned char dataSlotZoneConfigState[2] = { 0 };

	// Check if IOProtectionKey is locked; it should be 00 if locked
	rc = Core_info_base(0x06, 0x1600, lock);
	if (rc) {
		printf("[SE][ERR] Core_info_base failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] lock: ");
	msg_print(lock + 3, 1);
#endif

	// Read data slot zone config lock state;
	// a value other than 0xff indicates it is properly locked, allowing further steps
	rc = Core_read_config_zone(READ_MODE_CONFIG_ZONE_2, 0x000d, dataSlotZoneConfigState);
	if (rc) {
		printf("[SE][ERR] Core_read_config_zone failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] dataSlotZoneConfigState: ");
	msg_print(dataSlotZoneConfigState + 1, 1);
#endif

	if (dataSlotZoneConfigState[1] == 0xFF) {
		rc = -1;
		printf("[SE][ERR] Please lock dataSlotZoneConfig\n");
		return rc;
	}

	// Get chip UID
	rc = get_se_uid(uid);
	if (rc) {
		printf("[SE][ERR] get_se_uid failed\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] UID: ");
	msg_print(uid, sizeof(uid));
#endif

	// Get app magic num
	memcpy(app_magic_num, APP_MAGIC_NUM, sizeof(app_magic_num));
#ifdef DEBUG
	DBG("[SE][DBG] APP MAGIC NUM: ");
	msg_print(app_magic_num, sizeof(app_magic_num));
#endif

	// new_ioKey should be generated by SoC, this is the actual IOProtectionKey to be written to the product
	rc = gen_io_protection_key(app_magic_num, sizeof(app_magic_num), new_ioKey);
	if (rc) {
		printf("[SE][ERR] gen_io_protectio_key failed\n");
		return rc;
	}

#ifdef DEBUG
	DBG("[SE][DBG] new_ioKey: ");
	msg_print(new_ioKey, sizeof(new_ioKey));
#endif

	// defaultecdh_io_protection_key comes from the SharedKey generated by SQ713xS/EncWrite_IO_Protection_key/Unlock_DataSlotConfig.
	rc = read_file("/usrdata/shared_secret.bin", &defaultecdh_io_protection_key, &size);
	if (rc) {
		printf("[SE][ERR] Failed to read sharedKey.\n");
		return rc;
	}
#ifdef DEBUG
	DBG("[SE][DBG] defaultecdh_io_protection_key: ");
	msg_print(defaultecdh_io_protection_key, 32);
#endif

	// Start the synchronization of trng3
	rc = Core_random(random_output);
	if (rc) {
		printf("[SE][ERR] Core_random failed\n");
		goto fail;
	}
	rc = Core_nonce_rand(random_output, nonce_output);
	if (rc) {
		printf("[SE][ERR] Core_nonce_rand failed\n");
		goto fail;
	}
	memset(sha256_input, 0, sizeof(sha256_input));
	offset = 0;
	memcpy(sha256_input + offset, nonce_output, sizeof(nonce_output));
	offset += sizeof(nonce_output);
	memcpy(sha256_input + offset, random_output, 20);
	offset += 20;
	memcpy(sha256_input + offset, "\x16\x00\x00", 3);
	offset += 3;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		goto fail;
	}
	memset(internal_key, 0, sizeof(internal_key));
	memcpy(internal_key, sha256_output, sizeof(internal_key));

	// Complete the synchronization and calculation of trng4 for chip
	rc = Core_gendig(GENDIG_ZONE_DATA, 22 << 8, NULL, 0);
	if (rc) {
		printf("[SE][ERR] Core_gendig failed\n");
		goto fail;
	}

	// Complete the synchronization and calculation of trng4 for SoC
	offset = 0;
	memset(sha256_input, 0, sizeof(sha256_input));
	memcpy(sha256_input + offset, defaultecdh_io_protection_key, 32);
	offset += 32;
	memcpy(sha256_input + offset, "\x15\x02\x16\x00", 4);
	offset += 4;
	sha256_input[offset++] = uid[3];
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	memset(sha256_input + offset, 0, 25);
	offset += 25;
	memcpy(sha256_input + offset, internal_key, sizeof(internal_key));
	offset += sizeof(internal_key);
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		goto fail;
	}
	memset(internal_key, 0, sizeof(internal_key));
	memcpy(internal_key, sha256_output, sizeof(internal_key));

	// Calculate the MAC to be written to the chip
	memset(write_input, 0, sizeof(write_input));
	offset = 0;
	memcpy(sha256_input + offset, internal_key, sizeof(internal_key));
	offset += sizeof(internal_key);
	memcpy(sha256_input + offset, "\x12\x12", 2);
	offset += 2;
	sha256_input[offset++] = io_protection_key_slot;
	memcpy(sha256_input + offset, "\00", 1);
	offset += 1;
	sha256_input[offset++] = uid[3];
	sha256_input[offset++] = uid[0];
	sha256_input[offset++] = uid[1];
	memset(sha256_input + offset, 0, 25);
	offset += 25;
	memcpy(sha256_input + offset, new_ioKey, sizeof(new_ioKey));
	offset += sizeof(new_ioKey);
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		goto fail;
	}
	// Prepare to calculate the IOProtectionKey for the chip
	offset = 0;
	for (size_t i = 0; i < sizeof(new_ioKey); i++) {
		write_input[offset++] = new_ioKey[i] ^ internal_key[i];
	}
	memcpy(write_input + offset, sha256_output, sizeof(sha256_output));
	offset += sizeof(sha256_output);

	// Use Core_write to perform encWrite and write the official IOProtectionKey
	rc = Core_write(WRITE_MODE_DATA_32, io_protection_key_slot << 8, write_input, offset);
	if (rc) {
		printf("[SE][ERR] Core_write failed\n");
		goto fail;
	}
	// In real development, make sure to lock the IOProtectionKey after replacing it
	// Core_lock_data_slot(io_protection_key_slot);
	printf("[SE][ERR] App_write_ioProtectionKey done\n");

fail:
	free(defaultecdh_io_protection_key);

	return rc;
}

/**
 * @brief Generate the secret key
 * @details
 * @param[in] source_output: secret key slot
 * @arg
 * @see
 * @return result
 * @retval 0		success
 * @retval not 0	fail
 */
static int secret_key_provision(const uint8_t source_output)
{
	int rc = 0;
	uint8_t kdfmode = KDF_MODE_ALG_HKDF | KDF_MODE_SOURCE_INTERNALKEY | KDF_MODE_TARGET_SLOT;
	uint8_t source_input = 0x00;
	// Detail salt din please follow the datasheet specifications freely
	unsigned char detial[4] = { 0x04, 0x00, 0x08, 0x02 }; // The first byte 0x04 corresponds to the salt length 0-31
	unsigned char salt[4] = {
		0x01, 0x02, 0x03, 0x04
	}; // Host side: different salts can generate different AES keys or be of different lengths
	unsigned char din[8]; // {detial + salt}

	unsigned char outData[32]; // This example does not use this
	unsigned char outNonce[32]; // This example does not use this

	unsigned char randin[32];
	unsigned char randout[32];

	memcpy(din, detial, 4); // Copy the 4 bytes of detial into the first 4 bytes of din
	memcpy(din + 4, salt, 4); // Copy the 4 bytes of salt into the last 4 bytes of din
	memset(outData, 0x00, 32);
	memset(outNonce, 0x00, 32);
	memset(randin, 0x00, 32);
	memset(randout, 0x00, 32);

	// Update trng1
	//rc = gen_random(randin, sizeof(randin));
	if (rc) {
		printf("[SE][ERR] Cannot generate TRNG 1\n");
		return rc;
	}

	// SoC provides trng1 -> randin for chip to perform calculations, then directly the chip generates the key for slot 0x08~0x13 through KDF
	rc = Core_nonce_rand(randin, randout);
	rc = Core_kdf(kdfmode, (source_output << 8) | source_input, din, outData, outNonce);
	if (rc) {
		printf("[SE][ERR] Core_kdf failed\n");
		return rc;
	}
	// In this example, Core_lock_data_slot(source_output) is commented out; in a real development environment, be sure to lock the slot after key generation
	// Core_lock_data_slot(source_output);
	return 0;
}

/**
 * @brief Decrypt the encoded data using the IO protection key and a random number
 * @details 
 * @param[in] enc: encoded data
 * @param[in] trng: random number
 * @param[in] io_protection_key
 * @param[out] outData
 * @arg
 * @see
 * @return result
 * @retval 0		success
 * @retval not 0	fail
 */
static int handleDecEncCipher(uint8_t *enc, uint8_t *trng, unsigned char *io_protection_key, uint8_t *outData,
                              uint8_t outsize)
{
	int rc, offset, i;
	unsigned char sha256_input[96];
	unsigned char sha256_output[32];

	// get aesout Cipher
	memset(sha256_input, 0, sizeof(sha256_input));
	offset = 0;
	memcpy(sha256_input + offset, io_protection_key, 32);
	offset += 32;
	memcpy(sha256_input + offset, trng, 16);
	offset += 16;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		return rc;
	}

	offset = 0;
	for (i = 0; i < outsize; i++) {
		outData[offset++] = enc[i] ^ sha256_output[i];
	}
	return 0;
}

/**
 * @brief Check if the Data Slot Zone configuration is locked
 * @details
 * @arg
 * @see
 * @return result
 * @retval 0		success
 * @retval not 0	fail
 */
static int checkDataSlotZoneConfig(void)
{
	int rc;
	unsigned char lock[4] = { 0 };
	unsigned char dataSlotZoneConfigState[2] = { 0 };

	rc = Core_info_base(0x06, 0x0800, lock);
	if (rc) {
		printf("[SE][ERR] Core_info_base failed\n");
		return rc;
	}
	printf("[SE][INFO] lock: ");
	msg_print(lock, sizeof(lock));

	rc = Core_read_config_zone(READ_MODE_CONFIG_ZONE_2, 0x000d, dataSlotZoneConfigState);
	if (rc) {
		printf("[SE][ERR] Core_read_config_zone failed\n");
		return rc;
	}
	printf("[SE][INFO] dataSlotZoneConfigState: ");
	msg_print(dataSlotZoneConfigState + 1, 1);

	if (dataSlotZoneConfigState[1] == 0xFF) {
		rc = -1;
		printf("[SE][ERR] Lock dataSlotZoneConfig failed\n");
		return rc;
	}

	return 0;
}

/**
 * @brief Encrypt data with IO protection key and randmon number
 * @details
 * @param[in] input
 * @param[in] input_size
 * @param[in] io_protection_key
 * @param[out] outData
 * @arg
 * @see
 * @return result
 * @retval 0		success
 * @retval not 0	fail
 */
static int handleInputData(uint8_t *input, size_t input_size, unsigned char *io_protection_key, uint8_t *outData)
{
	int rc, offset;
	unsigned char uid[16];
	unsigned char trng1[32];
	unsigned char trng2[32];
	unsigned char trng3[32];
	unsigned char sha256_input[64 + input_size];
	unsigned char sha256_output[32];
	unsigned char encryptionKey[32];
	unsigned char write_input[32 + input_size];

	/* Step 1: Get chip UID */
	rc = Core_info_get_uid(uid);
	if (rc) {
		printf("[SE][ERR] Core_info_get_uid failed\n");
		return rc;
	}

	/* Step 2: set inData [plaintext]*/

	// Update trng1
	rc = gen_random(trng1, sizeof(trng1));
	if (rc) {
		printf("[SE][ERR] Cannot generate TRNG 1\n");
		return rc;
	}

	// TRNG3 sha256(trng2 + trng1 first 20 bytes + 0x16 0x00 0x00)
	rc = Core_nonce_rand(trng1, trng2);
	if (rc) {
		printf("[SE][ERR] Core_nonce_rand failed\n");
		return rc;
	}

	memset(sha256_input, 0, sizeof(sha256_input));
	offset = 0;
	memcpy(sha256_input + offset, trng2, sizeof(trng2));
	offset += sizeof(trng2);
	memcpy(sha256_input + offset, trng1, 20);
	offset += 20;
	memcpy(sha256_input + offset, "\x16\x00\x00", 3);
	offset += 3;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] trng3 failed\n");
		return rc;
	}
	memset(trng3, 0, sizeof(trng3));
	memcpy(trng3, sha256_output, sizeof(trng3));

	//Encryption key SHA256 (I/O protection key + first 16 bytes of RNG3)
	offset = 0;
	memset(sha256_input, 0, sizeof(sha256_input));
	memcpy(sha256_input + offset, io_protection_key, 32);
	offset += 32;
	memcpy(sha256_input + offset, trng3, 16);
	offset += 16;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		return rc;
	}
	memset(encryptionKey, 0, sizeof(encryptionKey));
	memcpy(encryptionKey, sha256_output, sizeof(encryptionKey));
	// MAC SHA ( TRNG3 + I/O protection key + inData )
	memset(write_input, 0, sizeof(write_input));
	offset = 0;

	memcpy(sha256_input + offset, trng3, sizeof(trng3));
	offset += sizeof(trng3);
	memcpy(sha256_input + offset, io_protection_key, 32);
	offset += 32;
	memcpy(sha256_input + offset, input, input_size);
	offset += input_size;
	rc = se_sw_sha2_256(sha256_input, offset, sha256_output);
	if (rc) {
		printf("[SE][ERR] se_sw_sha2_256 failed\n");
		return rc;
	}

	// Cipher inData XOR Encryption Key
	offset = 0;
	for (size_t i = 0; i < input_size; i++) {
		write_input[offset++] = input[i] ^ encryptionKey[i % sizeof(encryptionKey)];
	}
	memcpy(write_input + offset, sha256_output, sizeof(sha256_output));
	offset += sizeof(sha256_output);
	memcpy(outData, write_input, offset);

	return 0;
}

/**
 * @brief Encrypt the sensitive data using AES in ECB mode with the secret key
 * @details Encrypt sensitive data, such as Wi-Fi credentials or P2P IP addresses, using AES in ECB mode with a secret key
 * @param[in] slot_id: secret key slot
 * @param[in] intput: sensitive data
 * @param[in] input_size: length of the input
 * @param[out] mac_path: path of MAC
 * @param[out] inverse_data_path: path of the cipher
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int encrypt_sensitive_data(const uint8_t slot_id, unsigned char *input, const size_t input_size,
                                  const char *mac_path, const char *inverse_data_path)
{
	int rc = 0;
	unsigned char trng4[32] = { 0 };
	unsigned char aes_out[64] = { 0 };
	unsigned char ioKey[32] = { 0 };
	unsigned char app_magic_num[32] = { 0 };
	unsigned char encCipher[32] = { 0 };
	size_t aligned_size = ((input_size + 31) / 32) * 32;
	unsigned char *aligned_input = malloc(aligned_size);
	unsigned char *write_input = malloc(aligned_size + 64);
	unsigned char *cipher = NULL; // Dynamically allocated buffer
	unsigned char mac[32] = { 0 };

	if (!aligned_input || !write_input) {
		printf("[SE][ERR] Failed to allocate memory\n");
		return -1;
	}
	printf("[SE][INFO] input: ");
	msg_print(input, input_size);

	memset(aligned_input, 0, aligned_size);
	memcpy(aligned_input, input, input_size);
	printf("[SE][INFO] Input aligned to 32 bytes (padded with zeros)\n");

	// generate IO PROTECTION KEY
	memcpy(app_magic_num, APP_MAGIC_NUM, sizeof(app_magic_num));
	rc = gen_io_protection_key(app_magic_num, sizeof(app_magic_num), ioKey);
	if (rc) {
		printf("[SE][ERR] gen_io_protectio_key failed\n");
		return rc;
	}
	printf("[SE][INFO] io_protection_key: ");
	msg_print(ioKey, 32);

	// Allocate cipher buffer with size aligned to 16-byte blocks
	size_t cipher_size = ((input_size + 31) / 32) * 32;
	cipher = (unsigned char *)malloc(cipher_size);
	if (!cipher) {
		printf("[SE][ERR] Failed to allocate memory for cipher\n");
		return -1;
	}
	memset(cipher, 0, cipher_size);

	for (size_t i = 0; i < aligned_size; i += 32) {
		rc = handleInputData(aligned_input + i, 32, ioKey, write_input);

		// Get EncCipher and TRNG4
		rc = Core_aes(AES_MODE_ECB_32 | AES_MODE_ENC_CIPHERMAC, slot_id, AES_TYPE256, write_input, aes_out);
		if (rc) {
			printf("[SE][ERR] Core_aes write failed %d \n", rc);
			free(cipher);
			return rc;
		}

		memcpy(encCipher, &aes_out[0], 32);
		memcpy(trng4, &aes_out[32], 32);

		rc = handleDecEncCipher(encCipher, trng4, ioKey, cipher + i, 32);
	}
	printf("[SE][INFO] cipher: ");
	msg_print(cipher, cipher_size);

	// Write file
	FILE *file = fopen(inverse_data_path, "wb");
	if (file == NULL) {
		printf("[SE][ERR] Failed to open file\n");
		return -ENFILE;
	}

	fwrite(cipher, 1, cipher_size, file);
	fclose(file);
	printf("[SE][INFO] EncCipher: %s\n", inverse_data_path);

	// MAC
	Core_mac(MAC_MODE_CHALLENGE, 0x16, cipher, mac);
	if (rc) {
		printf("[SE][ERR] Core_mac failed\n");
		goto free;
	}
	printf("[SE][INFO] MAC: ");
	msg_print(mac, sizeof(mac));

	file = fopen(mac_path, "wb");
	if (file == NULL) {
		printf("[SE][ERR] Failed to open file\n");
		return -ENFILE;
	}

	fwrite(mac, 1, 32, file);
	fclose(file);
	printf("[SE][INFO] MAC: %s\n", mac_path);

free:
	// Free dynamically allocated buffer
	free(cipher);
	return rc;
}

/**
 * @brief Decrypt the cipher of sensitive data using AES in ECB mode with the secret key
 * @details Decrypt the cipher of sensitive data, such as Wi-Fi credentials or P2P IP addresses, using AES in ECB mode with a secret key
 * @param[in] slot_id: secret key slot
 * @param[in] cipher: the cipher of the sensitive data
 * @param[in] cipher_size: length of the cipher
 * @param[in] mac: path of MAC
 * @param[out] inverse_data_path: path of the decrypted sensitive data
 * @arg
 * @see
 * @return result.
 * @retval 0            success.
 * @retval not 0        fail.
 */
static int decrypt_sensitive_data(const uint8_t slot_id, unsigned char *cipher, const size_t cipher_size,
                                  unsigned char *mac, const char *inverse_data_path)
{
	int rc = 0;
	unsigned char ioKey[32] = { 0 };
	unsigned char app_magic_num[32] = { 0 };
	unsigned char trng4[32] = { 0 };
	unsigned char aes_out[64] = { 0 };
	unsigned char encData[32] = { 0 };
	unsigned char *plaintext = NULL; // Dynamically allocated buffer
	size_t aligned_size = ((cipher_size + 31) / 32) * 32;
	unsigned char *aligned_cipher = malloc(aligned_size);
	unsigned char *write_input = malloc(aligned_size + 64);
	unsigned char tmp_mac[32] = { 0 };

	printf("[SE][INFO] cipher: ");
	msg_print(cipher, cipher_size);

	printf("[SE][INFO] mac: ");
	msg_print(mac, 32);

	if (!aligned_cipher || !write_input) {
		printf("[SE][ERR] Failed to allocate memory\n");
		return -1;
	}
	memset(aligned_cipher, 0, aligned_size);
	memcpy(aligned_cipher, cipher, cipher_size);
	printf("[SE][INFO] Cipher aligned to 32 bytes (padded with zeros)\n");

	// Check MAC
	Core_mac(MAC_MODE_CHALLENGE, 0x16, aligned_cipher, tmp_mac);
	if (rc) {
		printf("[SE][ERR] Core_mac failed\n");
		return -1;
	}
	for (size_t i = 0; i < 32; i++) {
		if (tmp_mac[i] != mac[i]) {
			printf("[SE][ERR] MAC mismatch\n");
			return -EINVAL;
		}
	}
	printf("[SE][INFO] Pass MAC checking\n");

	// Generate IO PROTECTION KEY
	memcpy(app_magic_num, APP_MAGIC_NUM, sizeof(app_magic_num));
	rc = gen_io_protection_key(app_magic_num, sizeof(app_magic_num), ioKey);
	if (rc) {
		printf("[SE][ERR] gen_io_protectio_key failed\n");
		return rc;
	}
	printf("[SE][INFO] io_protection_key: ");
	msg_print(ioKey, 32);

	// Allocate plaintext buffer with size aligned to 16-byte blocks
	size_t plaintext_size = ((cipher_size + 15) / 16) * 16;
	plaintext = (unsigned char *)malloc(plaintext_size);
	if (!plaintext) {
		printf("[SE][ERR] Failed to allocate memory for plaintext\n");
		return -1;
	}
	memset(plaintext, 0, plaintext_size);

	for (size_t i = 0; i < aligned_size; i += 32) {
		rc = handleInputData(aligned_cipher + i, 32, ioKey, write_input);

		// Get EncData and TRNG4
		rc = Core_aes(AES_MODE_ECB_32 | AES_MODE_DEC_CIPHERMAC, slot_id, AES_TYPE256, write_input, aes_out);
		if (rc) {
			printf("[SE][ERR] Core_aes write failed %d \n", rc);
			free(plaintext);
			return rc;
		}
		memcpy(encData, &aes_out[0], 32);
		memcpy(trng4, &aes_out[32], 32);

		rc = handleDecEncCipher(encData, trng4, ioKey, plaintext + i, 32);
	}
	printf("[SE][INFO] plaintext: ");
	msg_print(plaintext, plaintext_size);

	// Write file
	FILE *file = fopen(inverse_data_path, "wb");
	if (file == NULL) {
		printf("[SE][ERR] Failed to open file\n");
		return -ENFILE;
	}

	fwrite(plaintext, 1, plaintext_size, file);
	fclose(file);

	printf("[SE][INFO] Decrypted data: %s\n", inverse_data_path);

	// Free dynamically allocated buffer
	free(plaintext);
	free(aligned_cipher);

	return 0;
}

/**
 * @brief Encrypt sensitive data by hashing the original data and applying KDF using a secret key
 * @details Encrypt sensitive data, such as usernames and passwords, by hashing the original data and applying KDF using a secret key
 * @param[in] username
 * @param[in] username_size
 * @param[in] password
 * @param[in] password_size
 * @param[out] output_path
 * @arg
 * @see
 * @return result
 * @retval 0		success
 * @retval not 0	fail
 */
static int encrypt_sensitive_data_hash(unsigned char *username, size_t username_size, unsigned char *password,
                                       size_t password_size, const char *output_path)
{
	int ret;
	uint8_t kdfmode = KDF_MODE_ALG_HKDF | KDF_MODE_SOURCE_SLOT | KDF_MODE_TARGET_OUTPUT_ENC;
	uint8_t source_input = SE_SECRET_KEY_SLOT;
	uint8_t source_output = 0x00;
	unsigned char *udin = malloc(username_size + 4); // detail + salt
	unsigned char *pdin = malloc(password_size + 4);
	unsigned char out_data[32] = { 0 };
	unsigned char out_nonce[32] = { 0 };
	unsigned char detail[4] = { 0x20, 0x00, 0x08, 0x02 };
	unsigned char sha256_input[48] = { 0 }; // io_protection_key + out_nonce[:16]
	unsigned char sha256_output[SHA256_LEN] = { 0 };
	unsigned char aeskey[32] = { 0 };
	unsigned char app_magic_num[32];
	unsigned char ioKey[32];
	size_t path_len = strlen(output_path);
	size_t buffer_len = path_len + 50; // output_path + 50 bytes for the filename
	char *full_path = malloc(buffer_len);
	FILE *file = NULL;

	printf("[SE][INFO] username:");
	msg_print(username, username_size);

	if (udin == NULL) {
		printf("[SE][ERR] Cannot allocate username din\n");
		return -1;
	}

	printf("[SE][INFO] password:");
	msg_print(password, password_size);

	if (pdin == NULL) {
		printf("[SE][ERR] Cannot allocate password din\n");
		return -1;
	}

	memcpy(app_magic_num, APP_MAGIC_NUM, sizeof(app_magic_num));
	ret = gen_io_protection_key(app_magic_num, sizeof(app_magic_num), ioKey);
	if (ret) {
		printf("[SE][ERR] gen_io_protectio_key failed\n");
		return ret;
	}
	printf("[SE][INFO] io_protection_key: ");
	msg_print(ioKey, 32);

	// Calculate username
	memset(sha256_output, 0x00, sizeof(sha256_output));
	ret = sha256_openssl(username, username_size, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}

	memcpy(udin, detail, sizeof(detail));
	memcpy(udin + sizeof(detail), sha256_output, sizeof(sha256_output));

	memset(out_data, 0x00, sizeof(out_data));
	memset(out_nonce, 0x00, sizeof(out_nonce));
	ret = Core_kdf(kdfmode, (source_output << 8) | source_input, udin, out_data, out_nonce);
	if (ret) {
		printf("[SE][ERR] Core_kdf failed\n");
		return ret;
	}
	printf("[SE][INFO] outdata");
	msg_print(out_data, sizeof(out_data));

	/* SHA-256 Calculation */
	memset(sha256_input, 0x00, sizeof(sha256_input));
	memcpy(sha256_input, ioKey, sizeof(ioKey));
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
	printf("[SE][INFO] aeskey:");
	msg_print(aeskey, AES_KEY_LEN);

	// Write file
	if (full_path == NULL) {
		printf("[ERR] Memory allocation failed\n");
		return -1;
	}
	snprintf(full_path, buffer_len, "%s/valid_username.bin", output_path);

	file = fopen(full_path, "wb");
	if (file == NULL) {
		printf("[SE][ERR] Failed to open file\n");
		return -ENFILE;
	}

	fwrite(aeskey, 1, 32, file);
	fclose(file);

	printf("[SE][INFO] Valid password: %s\n", full_path);

	// Calulate password
	memset(sha256_output, 0x00, sizeof(sha256_output));
	ret = sha256_openssl(password, password_size, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}

	memcpy(pdin, detail, sizeof(detail));
	memcpy(pdin + sizeof(detail), sha256_output, sizeof(sha256_output));

	memset(out_data, 0x00, sizeof(out_data));
	memset(out_nonce, 0x00, sizeof(out_nonce));
	ret = Core_kdf(kdfmode, (source_output << 8) | source_input, pdin, out_data, out_nonce);
	if (ret) {
		printf("[SE][ERR] Core_kdf failed\n");
		return ret;
	}
	printf("[SE][INFO] outdata");
	msg_print(out_data, sizeof(out_data));

	/* SHA-256 Calculation */
	memset(sha256_input, 0x00, sizeof(sha256_input));
	memcpy(sha256_input, ioKey, sizeof(ioKey));
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
	printf("[SE][INFO] aeskey:");
	msg_print(aeskey, AES_KEY_LEN);

	// Write file
	if (full_path == NULL) {
		printf("[ERR] Memory allocation failed\n");
		return -1;
	}
	snprintf(full_path, buffer_len, "%s/valid_password.bin", output_path);

	file = fopen(full_path, "wb");
	if (file == NULL) {
		printf("[SE][ERR] Failed to open file\n");
		return -ENFILE;
	}

	fwrite(aeskey, 1, 32, file);
	fclose(file);

	printf("[SE][INFO] Valid username: %s\n", full_path);

	return ret;
}

/**
 * @brief check sensitive data in hash
 * @details
 * @param[in] username
 * @param[in] username_size
 * @param[in] password
 * @param[in] password_size
 * @arg
 * @see
 * @return result
 * @retval 0		success
 * @retval not 0	fail
 */
static int check_sensitive_data_hash(unsigned char *username, size_t username_size, unsigned char *password,
                                     size_t password_size)
{
	int ret;
	uint8_t kdfmode = KDF_MODE_ALG_HKDF | KDF_MODE_SOURCE_SLOT | KDF_MODE_TARGET_OUTPUT_ENC;
	uint8_t source_input = SE_SECRET_KEY_SLOT;
	uint8_t source_output = 0x00;
	unsigned char *udin = malloc(username_size + 4); // detail + salt
	unsigned char *pdin = malloc(password_size + 4);
	unsigned char out_data[32] = { 0 };
	unsigned char out_nonce[32] = { 0 };
	unsigned char detail[4] = { 0x20, 0x00, 0x08, 0x02 };
	unsigned char sha256_input[48] = { 0 }; // io_protection_key + out_nonce[:16]
	unsigned char sha256_output[SHA256_LEN] = { 0 };
	unsigned char aeskey[32] = { 0 };
	unsigned char app_magic_num[32];
	unsigned char ioKey[32];
	unsigned char *valid_username;
	unsigned char *valid_password;
	const unsigned char *output_path = (const unsigned char *)"/usrdata";
	size_t path_len = strlen((const char *)output_path);
	size_t buffer_len = path_len + 50; // output_path + 50 bytes for the filename
	size_t valid_username_size = 0;
	size_t valid_password_size = 0;
	char *full_path = malloc(buffer_len);

	printf("[SE][INFO] username:");
	msg_print(username, username_size);

	if (udin == NULL) {
		printf("[SE][ERR] Cannot allocate username din\n");
		return -1;
	}

	printf("[SE][INFO] password:");
	msg_print(password, password_size);

	if (pdin == NULL) {
		printf("[SE][ERR] Cannot allocate password din\n");
		return -1;
	}

	memcpy(app_magic_num, APP_MAGIC_NUM, sizeof(app_magic_num));
	ret = gen_io_protection_key(app_magic_num, sizeof(app_magic_num), ioKey);
	if (ret) {
		printf("[SE][ERR] gen_io_protectio_key failed\n");
		return ret;
	}
	printf("[SE][INFO] io_protection_key: ");
	msg_print(ioKey, 32);

	// Calculate username
	memset(sha256_output, 0x00, sizeof(sha256_output));
	ret = sha256_openssl(username, username_size, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}

	memcpy(udin, detail, sizeof(detail));
	memcpy(udin + sizeof(detail), sha256_output, sizeof(sha256_output));

	memset(out_data, 0x00, sizeof(out_data));
	memset(out_nonce, 0x00, sizeof(out_nonce));
	ret = Core_kdf(kdfmode, (source_output << 8) | source_input, udin, out_data, out_nonce);
	if (ret) {
		printf("[SE][ERR] Core_kdf failed\n");
		return ret;
	}
	printf("[SE][INFO] outdata:");
	msg_print(out_data, sizeof(out_data));

	/* SHA-256 Calculation */
	memset(sha256_input, 0x00, sizeof(sha256_input));
	memcpy(sha256_input, ioKey, sizeof(ioKey));
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
	printf("[SE][INFO] aeskey:");
	msg_print(aeskey, AES_KEY_LEN);

	snprintf(full_path, buffer_len, "%s/valid_username.bin", output_path);

	ret = read_file(full_path, &valid_username, &valid_username_size);
	if (ret) {
		printf("[SE][ERR] Failed to read valid username.\n");
		return ret;
	}

	printf("[SE][INFO] valid_username:");
	msg_print(valid_username, AES_KEY_LEN);

	for (size_t i = 0; i < 32; i++) {
		if (valid_username[i] != aeskey[i]) {
			printf("[SE][ERR] username failed\n");
			return -1;
		}
	}

	// Calulate password
	memset(sha256_output, 0x00, sizeof(sha256_output));
	ret = sha256_openssl(password, password_size, sha256_output);
	if (ret) {
		printf("[SE][ERR] sha256_openssl failed\n");
		return ret;
	}

	memcpy(pdin, detail, sizeof(detail));
	memcpy(pdin + sizeof(detail), sha256_output, sizeof(sha256_output));

	memset(out_data, 0x00, sizeof(out_data));
	memset(out_nonce, 0x00, sizeof(out_nonce));
	ret = Core_kdf(kdfmode, (source_output << 8) | source_input, pdin, out_data, out_nonce);
	if (ret) {
		printf("[SE][ERR] Core_kdf failed\n");
		return ret;
	}
	printf("[SE][INFO] outdata");
	msg_print(out_data, sizeof(out_data));

	/* SHA-256 Calculation */
	memset(sha256_input, 0x00, sizeof(sha256_input));
	memcpy(sha256_input, ioKey, sizeof(ioKey));
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
	printf("[SE][INFO] aeskey:");
	msg_print(aeskey, AES_KEY_LEN);

	snprintf(full_path, buffer_len, "%s/valid_password.bin", output_path);
	ret = read_file(full_path, &valid_password, &valid_password_size);
	if (ret) {
		printf("[SE][ERR] Failed to read valid password.\n");
		return ret;
	}

	printf("[SE][INFO] valid_password:");
	msg_print(valid_password, AES_KEY_LEN);

	for (size_t i = 0; i < 32; i++) {
		if (valid_password[i] != aeskey[i]) {
			printf("[SE][ERR] password failed\n");
			return -1;
		}
	}
	printf("Valid Username and Password\n");

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
	int is_initial = 0;
	uint8_t uid[UID_LEN];
	uint32_t current_counter_val = 0;
	unsigned char *io_protection_key = NULL;
	unsigned char *sharedKey = NULL;
	unsigned char *input = NULL;
	unsigned char *mac = NULL;
	unsigned char *username = NULL;
	unsigned char *password = NULL;
	size_t read_size = 0;
	size_t mac_size = 0;
	size_t username_size = 0;
	size_t password_size = 0;
	FILE *file = NULL;
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
	case CASE_UID:
		memset(uid, 0x00, UID_LEN);
		ret = get_se_uid(uid);
		if (ret) {
			printf("[SE][ERR] Cannot get the UID of SE\n");
			goto end;
		}
		printf("[SE][INFO] UID of the secure element:\n");
		print_hex(uid, sizeof(uid));
		break;
	case CASE_LOCK_SLOT:
		/* Get slot information */
		get_slot_info_by_id(meta_data.slot_id, &slot_info);

		if (slot_info.slot_type == NULL) {
			printf("[SE][ERR] The provided Slot %d is not recognized or supported in this example.\n",
			       meta_data.slot_id);
			ret = -ENOEXEC;
			goto end;
		} else {
			printf("[SE][INFO] Slot ID:%d Bytes:%d Writeable:%d Readable:%d Unlocked:%d Loackable:%d Type:%s\n",
			       meta_data.slot_id, slot_info.bytes, slot_info.is_writeable, slot_info.is_readable,
			       slot_info.is_unlocked, slot_info.is_lockable, slot_info.slot_type);
		}

		/* NOTE: To avoid unintentionally locking other slots, we limit the use to SLOT_37 (Genral Data) only. */
		if (meta_data.slot_id != LOCK_SLOT) {
			printf("[SE][ERR] For demonstration purposes, only SLOT_37 (General Data) can be locked.\n");
			ret = -ENOEXEC;
			goto end;
		}

		/* Lock slot */
		if (slot_info.is_lockable == FALSE) {
			printf("[SE][ERR] Slot %d is not lockable. Stop lock slot.\n", meta_data.slot_id);
			ret = -ENOEXEC;
			goto end;
		}

		if (slot_info.is_unlocked == FALSE) {
			printf("[SE][ERR] Slot %d is already locked. Stop lock slot.\n", meta_data.slot_id);
			ret = -ENOEXEC;
			goto end;
		}

		ret = lock_slot_by_id(meta_data.slot_id);
		if (ret) {
			printf("[SE][ERR] Failed to lock the slot.\n");
			goto end;
		}
		printf("[SE][INFO] Slot %d is locked permanently and cannot be unlocked.\n", meta_data.slot_id);
		break;
	case CASE_SLOT_STATUS:
		/* Get slot information */
		get_slot_info_by_id(meta_data.slot_id, &slot_info);

		if (slot_info.slot_type == NULL) {
			printf("[SE][ERR] The provided Slot %d is not recognized or supported in this example.\n",
			       meta_data.slot_id);
			ret = -ENOEXEC;
			goto end;
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
				goto end;
			}
		}
		break;
	case CASE_COUNTER_READ:
		if (meta_data.counter_id != COUNTER_SLOT) {
			printf("[SE][ERR] counter ID must be 36 in demo.\n");
			ret = -EINVAL;
			goto end;
		}
		ret = read_counter(meta_data.counter_id, &current_counter_val);
		/* NOTE: The following log is recognized by sysupd-anti-rollback-se and S000anti_rollback.
		 * Revise with caution if modifications are necessary. */
		if (ret) {
			printf("[SE][ERR] Cannot get the value of counter\n");
			goto end;
		} else {
			printf("[SE][INFO] Counter value: %d\n", current_counter_val);
		}

		break;
	case CASE_COUNTER_UPDATE:
		if (meta_data.counter_id != COUNTER_SLOT) {
			printf("[SE][ERR] counter ID must be %d in demo.\n", COUNTER_SLOT);
			ret = -EINVAL;
			goto end;
		}
		if (meta_data.desired_counter_val > 2097151) {
			printf("[SE][ERR] counter value should be 0 ~ 2097151\n");
			goto end;
		}

		ret = update_counter(meta_data.counter_id, meta_data.desired_counter_val, &current_counter_val);

		/* NOTE: The following log is recognized by sysupd-anti-rollback-se and S000anti_rollback.
		 * Revise with caution if modifications are necessary. */
		if (ret) {
			printf("[SE][ERR] Cannot increase the counter\n");
			goto end;
		} else {
			printf("[SE][INFO] Counter value: %d\n", current_counter_val);
		}

		break;
	case CASE_IO_PROTECTION_KEY:
		/* Prepare input */
		file = fopen("/usrdata/shared_secret.bin", "rb");
		if (file != NULL) {
			// File exists, skip the io_protection_key_provision_unlock process
			printf("[INFO] /usrdata/shared_secret.bin already exists, skipping key generation.\n");
			fclose(file);
		} else {
			ret = io_protection_key_provision_unlock(&sharedKey);
			if (ret) {
				printf("[SE][ERR] Failed to get shared ECC key\n");
				goto end;
			}
			is_initial = 1;
		}

		ret = io_protection_key_provision_lock();
		if (ret) {
			printf("[SE][ERR] Failed to store IO PROTECTION KEY\n");
			goto end;
		}
		printf("[SE][INFO] Store IO PROTECTION KEY done\n");

		ret = system("rm -f /usrdata/shared_secret.bin");
		if (ret == -1) {
			printf("Error executing system command: remove shared secret key\n");
			return -1;
		}
		break;
	case CASE_SECRET_KEY:
		ret = secret_key_provision(SE_SECRET_KEY_SLOT);
		if (ret) {
			printf("[SE][ERR] Failed to store SECRET KEY\n");
			goto end;
		}

		printf("[SE][INFO] Generate SECRET KEY in SLOT[13]\n");

		break;
	case CASE_SENSITIVE_DATA:
		if (meta_data.username == NULL && meta_data.password == NULL) {
			ret = checkDataSlotZoneConfig();
			if (ret)
				goto end;

			if (meta_data.data != NULL) { // Encypt sensitive data (Set)
				/* Prepare input */
				ret = read_file(meta_data.data, &input, &read_size);
				if (ret) {
					printf("[SE][ERR] Failed to read input data.\n");
					goto end;
				}

				ret = encrypt_sensitive_data(SE_SECRET_KEY_SLOT, input, read_size, meta_data.mac,
				                             meta_data.inverse_data);
			} else { // Decryt sensitive data (Get)
				ret = read_file(meta_data.mac, &mac, &mac_size);
				if (ret) {
					printf("[SE][ERR] Failed to read MAC.\n");
					goto end;
				}
				/* Prepare input */
				ret = read_file(meta_data.input, &input, &read_size);
				if (ret) {
					printf("[SE][ERR] Failed to read input data.\n");
					goto end;
				}

				ret = decrypt_sensitive_data(SE_SECRET_KEY_SLOT, input, read_size, mac,
				                             meta_data.inverse_data);
			}
			if (ret)
				goto end;
		} else if (meta_data.username != NULL && meta_data.password != NULL) {
			/* Prepare input */
			ret = read_file(meta_data.username, &username, &username_size);
			if (ret) {
				printf("[SE][ERR] Failed to read username.\n");
				goto end;
			}
			ret = read_file(meta_data.password, &password, &password_size);
			if (ret) {
				printf("[SE][ERR] Failed to read password.\n");
				goto end;
			}

			if (meta_data.output_path != NULL) { // Encypt sensitive data in hash
				ret = encrypt_sensitive_data_hash(username, username_size, password, password_size,
				                                  meta_data.output_path);
			} else { // Compare sensitive data in hash
				ret = check_sensitive_data_hash(username, username_size, password, password_size);
			}
			if (ret)
				goto end;
		} else {
			printf("[SE][ERR] Unknown senstive data use case\n");
			ret = -1;
		}
		break;
	default:
		printf("[SE][ERR] Invalid option\n");
		break;
	}

end:
	free(io_protection_key);
	free(sharedKey);
	free(input);

	return ret;
}
