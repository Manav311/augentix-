#ifndef AGTX_DIP_AE_CONF_H_
#define AGTX_DIP_AE_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_AE_ZONE_WEIGHT_TABLE_MO_AVG,
	AGTX_AE_ZONE_WEIGHT_TABLE_MO_CENTRAL,
	AGTX_AE_ZONE_WEIGHT_TABLE_MO_SPOT,
	AGTX_AE_ZONE_WEIGHT_TABLE_MO_MANUAL
} AGTX_AE_ZONE_WEIGHT_TABLE_MODE;

#define MAX_AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S_MANUAL_TABLE_SIZE 64
typedef enum { AGTX_FPS_MO_VARIABLE, AGTX_FPS_MO_FIXED } AGTX_FPS_MODE;

typedef enum {
	AGTX_EXP_STRATE_NORMAL,
	AGTX_EXP_STRATE_HI_LIGHT_SUPPRES
} AGTX_EXP_STRATEGY;

typedef struct {
	AGTX_INT32 manual_table[MAX_AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S_MANUAL_TABLE_SIZE];
	AGTX_AE_ZONE_WEIGHT_TABLE_MODE mode;
} AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S;

typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 exp_value;
	AGTX_INT32 flag;
	AGTX_INT32 inttime;
	AGTX_UINT32 isp_gain;
	AGTX_UINT32 sensor_gain;
	AGTX_UINT32 sys_gain;
} AGTX_DIP_AE_MANUAL_CONF_S;

typedef struct {
	AGTX_INT32 enable;
	AGTX_INT32 frequency;
	AGTX_INT32 luma_delta;
} AGTX_DIP_AE_ANTI_FLICKER_CONF_S;

typedef struct {
	AGTX_DIP_AE_ANTI_FLICKER_CONF_S anti_flicker;
	AGTX_INT32 black_delay_frame;
	AGTX_INT32 black_speed_bias;
	AGTX_INT32 brightness;
	AGTX_EXP_STRATEGY exp_strategy;
	AGTX_INT32 exp_strength;
	AGTX_FPS_MODE fps_mode;
	AGTX_FLOAT frame_rate;
	AGTX_UINT32 gain_thr_down;
	AGTX_UINT32 gain_thr_up;
	AGTX_INT32 interval;
	AGTX_DIP_AE_MANUAL_CONF_S manual;
	AGTX_UINT32 max_isp_gain;
	AGTX_UINT32 max_sensor_gain;
	AGTX_UINT32 max_sys_gain;
	AGTX_UINT32 min_isp_gain;
	AGTX_UINT32 min_sensor_gain;
	AGTX_UINT32 min_sys_gain;
	AGTX_INT32 roi_awb_weight;
	AGTX_INT32 roi_luma_weight;
	AGTX_INT32 roi_zone_lum_avg_weight;
	AGTX_FLOAT slow_frame_rate;
	AGTX_INT32 speed;
	AGTX_INT32 tolerance;
	AGTX_INT32 video_dev_idx;
	AGTX_INT32 white_delay_frame;
	AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S zone_weight;
	AGTX_INT32 max_inttime;
	AGTX_INT32 min_inttime;
} AGTX_DIP_AE_CONF_S;

#endif /* AGTX_DIP_AE_CONF_H_ */
