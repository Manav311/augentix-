#ifndef AGTX_IVA_SHD_CONF_H_
#define AGTX_IVA_SHD_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_IVA_SHD_CONF_S_LONGTERM_LIST_SIZE 10

typedef struct {
	AGTX_INT32 end_x; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 end_y; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 start_x; /* Start coordinates of detection region. (inclusive) */
	AGTX_INT32 start_y; /* Start coordinates of detection region. (inclusive) */
} AGTX_IVA_SHD_LT_LIST_S;

typedef struct {
	AGTX_INT32 enabled; /* (Dis/En)able IVA shaking object detection. */
	AGTX_INT32 instance_duration; /* Define of instance object duration(0.1sec). */
	AGTX_INT32 longterm_dec_period; /* Define of shaking object update duration(1sec). */
	AGTX_INT32 longterm_life_th; /* The minimum life threshold to activate registered longterm item */
	AGTX_IVA_SHD_LT_LIST_S longterm_list[MAX_AGTX_IVA_SHD_CONF_S_LONGTERM_LIST_SIZE];
	AGTX_INT32 longterm_num; /* Number of longterm item. */
	AGTX_INT32 obj_life_th; /* Minimum life threshold for object to be considered for detection */
	AGTX_INT32 quality; /* The frequency of SHD to do checking and update internal data. */
	AGTX_INT32 sensitivity;
	AGTX_INT32 shaking_update_duration; /* Define of shaking object update duration(1sec). */
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_SHD_CONF_S;


#endif /* AGTX_IVA_SHD_CONF_H_ */
