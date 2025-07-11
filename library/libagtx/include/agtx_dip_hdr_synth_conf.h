#ifndef AGTX_DIP_HDR_SYNTH_CONF_H_
#define AGTX_DIP_HDR_SYNTH_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 local_fb_th;
	AGTX_INT32 video_dev_idx;
	AGTX_INT32 weight_le_weight_max;
	AGTX_INT32 weight_le_weight_min;
	AGTX_INT32 weight_le_weight_slope;
	AGTX_INT32 weight_le_weight_th_max;
	AGTX_INT32 weight_se_weight_max;
	AGTX_INT32 weight_se_weight_min;
	AGTX_INT32 weight_se_weight_slope;
	AGTX_INT32 weight_se_weight_th_min;
	AGTX_INT32 frame_fb_strength;
} AGTX_DIP_HDR_SYNTH_CONF_S;


#endif /* AGTX_DIP_HDR_SYNTH_CONF_H_ */
