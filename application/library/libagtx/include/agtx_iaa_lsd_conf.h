#ifndef AGTX_IAA_LSD_CONF_H_
#define AGTX_IAA_LSD_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 audio_dev_idx;
	AGTX_FLOAT duration;
	AGTX_INT32 enabled;
	AGTX_FLOAT suppression;
	AGTX_INT32 volume;
} AGTX_IAA_LSD_CONF_S;


#endif /* AGTX_IAA_LSD_CONF_H_ */
