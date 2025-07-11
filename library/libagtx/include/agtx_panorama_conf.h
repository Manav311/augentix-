#ifndef AGTX_PANORAMA_CONF_H_
#define AGTX_PANORAMA_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 center_offset_x;
	AGTX_INT32 center_offset_y;
	AGTX_INT32 curvature;
	AGTX_INT32 enable;
	AGTX_INT32 ldc_ratio;
	AGTX_INT32 radius;
	AGTX_INT32 straighten;
	AGTX_INT32 video_dev_idx;
} AGTX_PANORAMA_CONF_S;


#endif /* AGTX_PANORAMA_CONF_H_ */
