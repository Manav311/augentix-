#ifndef AGTX_FLOODLIGHT_CONF_H_
#define AGTX_FLOODLIGHT_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 bright_mode; /* Lightness condition. 0 is manual, 1 is auto. */
	AGTX_INT32 enabled;
	AGTX_INT32 lightness;
	AGTX_INT32 warn_switch;
	AGTX_INT32 warn_time;
} AGTX_FLOODLIGHT_CONF_S;


#endif /* AGTX_FLOODLIGHT_CONF_H_ */
