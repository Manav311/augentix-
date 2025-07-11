#ifndef AGTX_IVA_PD_CONF_H_
#define AGTX_IVA_PD_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 max_aspect_ratio_h;
	AGTX_INT32 max_aspect_ratio_w;
	AGTX_INT32 max_size;
	AGTX_INT32 min_aspect_ratio_h;
	AGTX_INT32 min_aspect_ratio_w;
	AGTX_INT32 min_size;
	AGTX_INT32 obj_life_th;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_PD_CONF_S;


#endif /* AGTX_IVA_PD_CONF_H_ */
