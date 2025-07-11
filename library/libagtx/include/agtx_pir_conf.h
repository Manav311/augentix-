#ifndef AGTX_PIR_CONF_H_
#define AGTX_PIR_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_PIR_ENABLED_OFF,
	AGTX_PIR_ENABLED_ON,
	AGTX_PIR_ENABLED_NUM
} AGTX_PIR_ENABLED_E;

#define MAX_AGTX_PIR_GROUP_S_NAME_SIZE 129
#define MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE 3

typedef struct {
	AGTX_INT32 enabled; /* Enable PIR GPIO detection. 1 is On, 0 is Off. */
	AGTX_UINT8 name[MAX_AGTX_PIR_GROUP_S_NAME_SIZE]; /* PIR event name. */
} AGTX_PIR_GROUP_S;

typedef struct {
	AGTX_INT32 duty_cycle; /* Sensitivity of pir distance detected. */
	AGTX_INT32 period;
	AGTX_PIR_GROUP_S pir_alias[MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE];
} AGTX_PIR_CONF_S;


#endif /* AGTX_PIR_CONF_H_ */
