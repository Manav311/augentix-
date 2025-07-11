#ifndef SE_SQ7131S_H_
#define SE_SQ7131S_H_

#include <stdint.h>

#define APP_MAGIC_NUM \
	"\x58\xeb\x20\xf2\xe5\x8a\xa3\xa4\xca\xff\x3f\x22\x3a\xbc\xb2\xf8\xb6\xc2\x72\xb2\x88\xbe\x95\x7d\x74\xf4\x26\xea\xb2\x24\x23\x80"

#define SE_SECRET_KEY_SLOT 0x0D // slot 13
#define COUNTER_SLOT 0x24 // slot 36
#define LOCK_SLOT 0x25 // slot 37

typedef struct meta_data {
	uint8_t i2c_bus_id;
	uint8_t slot_id;
	uint8_t counter_id;
	uint32_t desired_counter_val;
	char *input;
	char *output_path;
	char *mac;
	char *username;
	char *password;
	char *data;
	char *inverse_data;
} Meta_data;

typedef enum {
	CASE_UID,
	CASE_LOCK_SLOT,
	CASE_SLOT_STATUS,
	CASE_COUNTER_READ,
	CASE_COUNTER_UPDATE,
	CASE_IO_PROTECTION_KEY,
	CASE_SECRET_KEY,
	CASE_SENSITIVE_DATA,
	CASE_NUM
} CaseType;

int SE_executeDemo(const CaseType case_type, const Meta_data meta_data);

#endif /* SE_SQ7131S_H_ */
