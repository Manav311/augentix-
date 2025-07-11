#ifndef AGTX_DIP_DHZ_CONF_H_
#define AGTX_DIP_DHZ_CONF_H_

#include "agtx_types.h"

#define MAX_AGTX_DIP_DHZ_CONF_S_AUTO_Y_GAIN_MAX_LIST_SIZE 11
#define MAX_AGTX_DIP_DHZ_CONF_S_AUTO_C_GAIN_MAX_LIST_SIZE 11

struct json_object;

typedef struct {
	AGTX_INT32 auto_y_gain_max_list[MAX_AGTX_DIP_DHZ_CONF_S_AUTO_Y_GAIN_MAX_LIST_SIZE];
	AGTX_INT32 auto_c_gain_max_list[MAX_AGTX_DIP_DHZ_CONF_S_AUTO_C_GAIN_MAX_LIST_SIZE];
	AGTX_INT32 manual_y_gain_max;
	AGTX_INT32 manual_c_gain_max;
	AGTX_INT32 dc_iir_weight;
	AGTX_INT32 gain_step_th;
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_DHZ_CONF_S;

#endif /* AGTX_DIP_DHZ_CONF_H_ */
