#ifndef AGTX_SURROUND_CONF_H_
#define AGTX_SURROUND_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_SURR_ROTATE_0,
	AGTX_SURR_ROTATE_90,
	AGTX_SURR_ROTATE_180,
	AGTX_SURR_ROTATE_270
} AGTX_SURR_ROTATE_E;


typedef struct {
	AGTX_INT32 center_offset_x;
	AGTX_INT32 center_offset_y;
	AGTX_INT32 enable;
	AGTX_INT32 ldc_ratio;
	AGTX_INT32 max_radius;
	AGTX_INT32 min_radius;
	AGTX_SURR_ROTATE_E rotate;
	AGTX_INT32 video_dev_idx;
} AGTX_SURROUND_CONF_S;


#endif /* AGTX_SURROUND_CONF_H_ */
