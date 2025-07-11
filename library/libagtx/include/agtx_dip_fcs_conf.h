#ifndef AGTX_DIP_FCS_CONF_H_
#define AGTX_DIP_FCS_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_DIP_FCS_CONF_S_AUTO_OFFSET_LIST_SIZE 11
#define MAX_AGTX_DIP_FCS_CONF_S_AUTO_STRENGTH_LIST_SIZE 11
#define MAX_AGTX_DIP_FCS_CONF_S_AUTO_THRESHOLD_LIST_SIZE 11

typedef struct {
	AGTX_INT32 auto_offset_list[MAX_AGTX_DIP_FCS_CONF_S_AUTO_OFFSET_LIST_SIZE];
	AGTX_INT32 auto_strength_list[MAX_AGTX_DIP_FCS_CONF_S_AUTO_STRENGTH_LIST_SIZE];
	AGTX_INT32 auto_threshold_list[MAX_AGTX_DIP_FCS_CONF_S_AUTO_THRESHOLD_LIST_SIZE];
	AGTX_INT32 manual_offset;
	AGTX_INT32 manual_strength;
	AGTX_INT32 manual_threshold;
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_FCS_CONF_S;

#endif /* AGTX_DIP_FCS_CONF_H_ */
