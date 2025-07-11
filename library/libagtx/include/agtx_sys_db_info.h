#ifndef AGTX_SYS_DB_INFO_H_
#define AGTX_SYS_DB_INFO_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_UPDATE_RULE_NONE,
	AGTX_UPDATE_RULE_OVERWRITE
} AGTX_UPDATE_RULE_E;

#define MAX_AGTX_FW_SETTING_PARAM_S_NAME_SIZE 65
#define MAX_AGTX_SYS_DB_INFO_S_DB_VER_SIZE 129
#define MAX_AGTX_SYS_DB_INFO_S_FW_SETTING_LIST_SIZE 128

typedef struct {
	AGTX_INT32 gpio_pin0;
	AGTX_INT32 gpio_pin1;
	AGTX_INT32 inverse_pin0;
	AGTX_INT32 inverse_pin1;
	AGTX_INT32 period_msec;
	AGTX_INT32 duty_cycle;
	AGTX_INT32 interleave;
} AGTX_LED_INFO_S;

typedef struct {
	AGTX_UINT8 name[MAX_AGTX_FW_SETTING_PARAM_S_NAME_SIZE];
	AGTX_UPDATE_RULE_E update_rule;
} AGTX_FW_SETTING_PARAM_S;

typedef struct {
	AGTX_UINT8 db_ver[MAX_AGTX_SYS_DB_INFO_S_DB_VER_SIZE];
	AGTX_FW_SETTING_PARAM_S fw_setting_list[MAX_AGTX_SYS_DB_INFO_S_FW_SETTING_LIST_SIZE];
	AGTX_LED_INFO_S led_info;
} AGTX_SYS_DB_INFO_S;


#endif /* AGTX_SYS_DB_INFO_H_ */
