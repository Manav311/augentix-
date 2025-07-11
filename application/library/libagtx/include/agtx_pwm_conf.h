#ifndef AGTX_PWM_CONF_H_
#define AGTX_PWM_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_PWM_ALIAS_S_NAME_SIZE 129
#define MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE 6

typedef struct {
	AGTX_INT32 duty_cycle;
	AGTX_INT32 enabled; /* PWM enabled. Set 1/0 to open/close the initial PWM status. */
	AGTX_UINT8 name[MAX_AGTX_PWM_ALIAS_S_NAME_SIZE]; /* PWM event name. */
	AGTX_INT32 period;
	AGTX_INT32 pin_num; /* PWM pin number. Set -1 to close the PWM module. */
} AGTX_PWM_ALIAS_S;

typedef struct {
	AGTX_PWM_ALIAS_S pwm_alias[MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE];
} AGTX_PWM_CONF_S;


#endif /* AGTX_PWM_CONF_H_ */
