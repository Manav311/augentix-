#ifndef AGTX_IVA_BM_CONF_H_
#define AGTX_IVA_BM_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_IVA_FD_CTRL_NONE,
	AGTX_IVA_FD_CTRL_SAVE,
	AGTX_IVA_FD_CTRL_LOAD
} AGTX_IVA_FD_CTRL_E;


typedef struct {
	AGTX_INT32 end_x; /* End coordinates of monitor region. (inclusive) */
	AGTX_INT32 end_y; /* End coordinates of monitor region. (inclusive) */
	AGTX_INT32 start_x; /* Start coordinates of monitor region. (inclusive) */
	AGTX_INT32 start_y; /* Start coordinates of monitor region. (inclusive) */
} AGTX_IVA_BM_REGION_S;

typedef struct {
	AGTX_INT32 boundary_thickness; /* Border thickness of monitor region. */
	AGTX_IVA_FD_CTRL_E data_ctrl; /* Save or Load foreground detection data. */
	AGTX_INT32 enabled;
	AGTX_INT32 quality; /* quality range [0,255]. */
	AGTX_INT32 reset; /* Reset baby monitor status. */
	AGTX_IVA_BM_REGION_S roi;
	AGTX_INT32 sensitivity; /* sensitivity range [0,255]. */
	AGTX_INT32 suppression; /* suppression time(seconds). */
	AGTX_INT32 time_buffer; /* Request time buffer size(seconds). */
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_BM_CONF_S;


#endif /* AGTX_IVA_BM_CONF_H_ */
