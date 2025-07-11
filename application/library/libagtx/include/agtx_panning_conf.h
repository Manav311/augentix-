#ifndef AGTX_PANNING_CONF_H_
#define AGTX_PANNING_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 center_offset_x;
	AGTX_INT32 center_offset_y;
	AGTX_INT32 enable;
	AGTX_INT32 hor_strength;
	AGTX_INT32 ldc_ratio;
	AGTX_INT32 radius;
	AGTX_INT32 ver_strength;
	AGTX_INT32 video_dev_idx;
} AGTX_PANNING_CONF_S;


#endif /* AGTX_PANNING_CONF_H_ */
