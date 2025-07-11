#ifndef AGTX_IVA_DK_CONF_H_
#define AGTX_IVA_DK_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef struct {
	AGTX_INT32 end_x; /* End coordinates of monitor region. (inclusive) */
	AGTX_INT32 end_y; /* End coordinates of monitor region. (inclusive) */
	AGTX_INT32 start_x; /* Start coordinates of monitor region. (inclusive) */
	AGTX_INT32 start_y; /* Start coordinates of monitor region. (inclusive) */
} AGTX_IVA_DK_REGION_S;

typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 obj_life_th; /* object confidence threshold range [0,160]. */
	AGTX_INT32 overlap_ratio_th; /* Object to ROI overlap ratio threshold. range [0,100] */
	AGTX_INT32 loiter_count_th; /* Count threshold for prowling status. */
	AGTX_INT32 loiter_period_th; /* loitering period threshold (10ms). */
	AGTX_IVA_DK_REGION_S roi;
	AGTX_INT32 video_chn_idx;
	AGTX_INT32 visit_count_th; /* Count threshold for visiting status. */
	AGTX_INT32 visit_period_th; /* visit period threshold (10ms). */
} AGTX_IVA_DK_CONF_S;

#endif /* AGTX_IVA_DK_CONF_H_ */
