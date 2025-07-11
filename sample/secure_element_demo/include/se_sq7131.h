#ifndef SE_SQ7131_H_
#define SE_SQ7131_H_

#include <stdint.h>

#define IO_PROTECTION_KEY_VECTOR \
	"\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x0F\x0E\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00"
#define SECRET_KEY_VECTOR \
	"\x0F\x0E\x0D\x0C\x0B\x0A\x09\x08\x07\x06\x05\x04\x03\x02\x01\x00\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F"

#define SLOT_22 22 // IO protection key, write only
#define SLOT_22_LEN 32
#define SLOT_36 36 // 32 bytes, general data
#define SLOT_36_LEN 32
#define SLOT_37 37 // 32 bytes, general data

#define COUNTER_INCREASE ((uint8_t)0x01)
#define COUNTER_READ ((uint8_t)0x00)
#define COUNTERS_ONE ((uint8_t)0x00)
#define COUNTERS_TWO ((uint8_t)0x01)
#define COUNTER_OUT_SIZE 4

typedef struct meta_data {
	uint8_t i2c_bus_id;
	uint8_t counter_id;
	uint8_t slot_id;
	uint32_t desired_counter_val;
	char *f_product_id;
	char *f_device_id;
	char *f_device_secret;
	char *f_encrypted_device_secret;
	char *f_plaintext;
} Meta_data;

typedef enum {
	CASE_DEVICE_SECRET_FLASH = 0,
	CASE_DECRYPTED_DEVICE_SECRET_FLASH,
	CASE_DEVICE_SECRET_SE,
	CASE_AUTHENTICATION,
	CASE_SECURE_STORAGE_PROVISION,
	CASE_UID,
	CASE_COUNTER_READ,
	CASE_COUNTER_INCREASE,
	CASE_KEY_PROVISION,
	CASE_LOCK_SLOT,
	CASE_SLOT_STATUS,
	CASE_NUM
} CaseType;

int SE_executeDemo(const CaseType case_type, const Meta_data meta_data);

#endif /* SE_SQ7131_H_ */
