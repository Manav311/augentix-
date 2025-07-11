#ifndef AGTX_DIP_PTA_CONF_H_
#define AGTX_DIP_PTA_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_DIP_PTA_CONF_S_AUTO_TONE_TABLE_SIZE 11
#define MAX_AGTX_DIP_PTA_CONF_S_CURVE_SIZE 33

typedef struct {
	AGTX_INT32 auto_tone_table[MAX_AGTX_DIP_PTA_CONF_S_AUTO_TONE_TABLE_SIZE];
	AGTX_INT32 break_point;
	AGTX_INT32 brightness;
	AGTX_INT32 contrast;
	AGTX_INT32 curve[MAX_AGTX_DIP_PTA_CONF_S_CURVE_SIZE];
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_PTA_CONF_S;


#endif /* AGTX_DIP_PTA_CONF_H_ */
