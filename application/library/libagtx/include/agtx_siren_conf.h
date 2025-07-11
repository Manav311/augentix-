#ifndef AGTX_SIREN_CONF_H_
#define AGTX_SIREN_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 duration; /* Duration of siren once starts playing. (unit: second) */
	AGTX_INT32 enabled; /* Enable siren for audio output. */
	AGTX_INT32 hold_time; /* Time until another source can use audio output. (unit: second) */
	AGTX_INT32 priority; /* Priority of audio output. The smaller the highest. */
	AGTX_INT32 volume; /* Volume. */
} AGTX_SIREN_CONF_S;


#endif /* AGTX_SIREN_CONF_H_ */
