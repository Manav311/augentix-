#ifndef AGTX_IVA_AROI_CONF_H_
#define AGTX_IVA_AROI_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 aspect_ratio_height;
	AGTX_INT32 aspect_ratio_width;
	AGTX_INT32 en_skip_shake;
	AGTX_INT32 enabled;
	AGTX_INT32 max_roi_height;
	AGTX_INT32 max_roi_width;
	AGTX_INT32 min_roi_height;
	AGTX_INT32 min_roi_width;
	AGTX_INT32 return_speed;
	AGTX_INT32 track_speed;
	AGTX_INT32 obj_life_th;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_AROI_CONF_S;


#endif /* AGTX_IVA_AROI_CONF_H_ */
