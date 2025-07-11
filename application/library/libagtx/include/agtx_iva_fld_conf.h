#ifndef AGTX_IVA_FLD_CONF_H_
#define AGTX_IVA_FLD_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef struct {
	AGTX_INT32 demo_level; /* demo level of detail. Range: [0: simple, 1:detailed] */
	AGTX_INT32 down_period_th; /* Down period threshold (10ms). range [0, 60*100] */
	AGTX_INT32 enabled;
	AGTX_INT32 fallen_period_th; /* Fallen period threshold (10ms). range [0, 60*100] */
	AGTX_INT32 falling_period_th; /* Falling period threshold (10ms). range [0, 60*100] */
	AGTX_INT32 obj_falling_mv_th; /* Object falling mv threshold range [0, 120]. */
	AGTX_INT32 obj_high_ratio_th; /* Object high to high history ratio threshold. range [0,100] */
	AGTX_INT32 obj_life_th; /* Object confidence threshold range [0,160]. */
	AGTX_INT32 obj_stop_mv_th; /* Object stop mv threshold range [0, 120]. */
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_FLD_CONF_S;

#endif /* AGTX_IVA_FLD_CONF_H_ */
