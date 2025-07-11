#ifndef AGTX_IVA_PFM_CONF_H_
#define AGTX_IVA_PFM_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_IVA_PFM_CONF_S_SCHEDULE_SIZE 10

typedef struct {
	AGTX_INT32 end_x; /* End coordinates of monitor region. (inclusive) */
	AGTX_INT32 end_y; /* End coordinates of monitor region. (inclusive) */
	AGTX_INT32 start_x; /* Start coordinates of monitor region. (inclusive) */
	AGTX_INT32 start_y; /* Start coordinates of monitor region. (inclusive) */
} AGTX_IVA_PFM_REGION_S;

typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 endurance; /* Time needed to determine the finish signal. */
	AGTX_INT32 regis_to_feeding_interval; /* The duration between background registration and feeding notification. */
	AGTX_INT32 register_scene; /* 0-Do nothing, 1-Register scene, 2-Feeding notification. */
	AGTX_IVA_PFM_REGION_S roi;
	AGTX_INT32 schedule[MAX_AGTX_IVA_PFM_CONF_S_SCHEDULE_SIZE];
	AGTX_INT32 sensitivity;
	AGTX_INT32 time_number; /* Number of feeding time in the schedule. */
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_PFM_CONF_S;


#endif /* AGTX_IVA_PFM_CONF_H_ */
