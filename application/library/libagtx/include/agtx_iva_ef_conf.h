#ifndef AGTX_IVA_EF_CONF_H_
#define AGTX_IVA_EF_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_IVA_EF_MODE_DIR_NONE,
	AGTX_IVA_EF_MODE_DIR_POS,
	AGTX_IVA_EF_MODE_DIR_NEG,
	AGTX_IVA_EF_MODE_DIR_BOTH
} AGTX_IVA_EF_MODE_E;

#define MAX_AGTX_IVA_EF_CONF_S_ACTIVE_CELL_SIZE 512
#define MAX_AGTX_IVA_EF_CONF_S_LINE_LIST_SIZE 16

typedef struct {
	AGTX_INT32 end_x; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 end_y; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 id;
	AGTX_IVA_EF_MODE_E mode;
	AGTX_INT32 obj_area;
	AGTX_INT32 obj_life_th;
	AGTX_INT32 obj_max_h;
	AGTX_INT32 obj_max_w;
	AGTX_INT32 obj_min_h;
	AGTX_INT32 obj_min_w;
	AGTX_INT32 obj_v_th;
	AGTX_INT32 start_x; /* Start coordinates of detection region. (inclusive) */
	AGTX_INT32 start_y; /* Start coordinates of detection region. (inclusive) */
} AGTX_IVA_EF_LINE_S;

typedef struct {
	AGTX_UINT8 active_cell[MAX_AGTX_IVA_EF_CONF_S_ACTIVE_CELL_SIZE];
	AGTX_INT32 enabled;
	AGTX_INT32 line_cnt;
	AGTX_IVA_EF_LINE_S line_list[MAX_AGTX_IVA_EF_CONF_S_LINE_LIST_SIZE];
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_EF_CONF_S;


#endif /* AGTX_IVA_EF_CONF_H_ */
