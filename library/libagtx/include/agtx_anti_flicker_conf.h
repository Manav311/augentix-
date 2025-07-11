#ifndef AGTX_ANTI_FLICKER_CONF_H_
#define AGTX_ANTI_FLICKER_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_ANTI_FLICKER_CONF_S_FREQUENCY_LIST_SIZE 2

typedef struct {
	AGTX_FLOAT fps;
	AGTX_INT32 frequency;
} AGTX_ANTI_FLICKER_LIST_S;

typedef struct {
	AGTX_INT32 enable;
	AGTX_INT32 frequency_idx;
	AGTX_ANTI_FLICKER_LIST_S frequency_list[MAX_AGTX_ANTI_FLICKER_CONF_S_FREQUENCY_LIST_SIZE];
} AGTX_ANTI_FLICKER_CONF_S;


#endif /* AGTX_ANTI_FLICKER_CONF_H_ */
