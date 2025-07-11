#ifndef AGTX_IVA_RMS_CONF_H_
#define AGTX_IVA_RMS_CONF_H_

#include "agtx_types.h"
struct json_object;


typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 sensitivity;
	AGTX_INT32 split_x;
	AGTX_INT32 split_y;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_RMS_CONF_S;


#endif /* AGTX_IVA_RMS_CONF_H_ */
