#ifndef AGTX_IVA_MD_CONF_H_
#define AGTX_IVA_MD_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_IVA_MD_MODE_AREA,
	AGTX_IVA_MD_MODE_ENERGY
} AGTX_IVA_MD_MODE_E;

typedef enum {
	AGTX_IVA_MD_DET_NORMAL,
	AGTX_IVA_MD_DET_SUBTRACT
} AGTX_IVA_MD_DET_E;

#define MAX_AGTX_IVA_MD_CONF_S_ACTIVE_CELL_SIZE 513
#define MAX_AGTX_IVA_MD_CONF_S_RGN_LIST_SIZE 64

typedef struct {
	AGTX_IVA_MD_DET_E det;
	AGTX_INT32 ex; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 ey; /* End coordinates of detection region. (inclusive) */
	AGTX_INT32 id;
	AGTX_INT32 max_spd;
	AGTX_INT32 min_spd;
	AGTX_IVA_MD_MODE_E mode;
	AGTX_INT32 obj_life_th;
	AGTX_INT32 sens;
	AGTX_INT32 sx; /* Start coordinates of detection region. (inclusive) */
	AGTX_INT32 sy; /* Start coordinates of detection region. (inclusive) */
} AGTX_IVA_MD_REGION_S;

typedef struct {
	AGTX_FLOAT alarm_buffer;
	AGTX_INT32 alarm_switch_on_time;
	AGTX_IVA_MD_DET_E det;
	AGTX_INT32 en_rgn; /* Enable region selection */
	AGTX_INT32 en_skip_pd;
	AGTX_INT32 en_skip_shake;
	AGTX_INT32 enabled;
	AGTX_INT32 max_spd;
	AGTX_INT32 min_spd;
	AGTX_IVA_MD_MODE_E mode;
	AGTX_INT32 obj_life_th;
	AGTX_INT32 rgn_cnt;
	AGTX_UINT8 active_cell[MAX_AGTX_IVA_MD_CONF_S_ACTIVE_CELL_SIZE];
	AGTX_IVA_MD_REGION_S rgn_list[MAX_AGTX_IVA_MD_CONF_S_RGN_LIST_SIZE];
	AGTX_INT32 sens;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_MD_CONF_S;


#endif /* AGTX_IVA_MD_CONF_H_ */
