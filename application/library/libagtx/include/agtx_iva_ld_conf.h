#ifndef AGTX_IVA_LD_CONF_H_
#define AGTX_IVA_LD_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_LD_TRIG_COND_LIGHT_NONE,
	AGTX_LD_TRIG_COND_LIGHT_ON,
	AGTX_LD_TRIG_COND_LIGHT_OFF,
	AGTX_LD_TRIG_COND_BOTH
} AGTX_LD_TRIG_COND_E;


typedef struct {
	AGTX_INT32 end_x; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 end_y; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 start_x; /* Start coordinates of detection region. (inclusive) */
	AGTX_INT32 start_y; /* Start coordinates of detection region. (inclusive) */
} AGTX_IVA_LD_REGION_S;

typedef struct {
	AGTX_IVA_LD_REGION_S det_region;
	AGTX_INT32 enabled;
	AGTX_INT32 sensitivity;
	AGTX_LD_TRIG_COND_E trigger_cond;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_LD_CONF_S;


#endif /* AGTX_IVA_LD_CONF_H_ */
