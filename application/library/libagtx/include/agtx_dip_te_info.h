#ifndef AGTX_DIP_TE_INFO_H_
#define AGTX_DIP_TE_INFO_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_DIP_TE_INFO_S_TM_CURVE_SIZE 60

typedef struct {
	AGTX_INT32 tm_curve[MAX_AGTX_DIP_TE_INFO_S_TM_CURVE_SIZE];
	AGTX_INT32 tm_enable;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_TE_INFO_S;

#endif /* AGTX_DIP_TE_INFO_H_ */
