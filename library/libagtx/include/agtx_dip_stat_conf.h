#ifndef AGTX_DIP_STAT_CONF_H_
#define AGTX_DIP_STAT_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_X_SIZE 5
#define MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_Y_SIZE 5
#define MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_SLOPE_SIZE 4
#define MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_TH_SIZE 4

typedef struct {
	AGTX_INT32 lum_max;
	AGTX_INT32 lum_min;
	AGTX_INT32 lum_slope;
	AGTX_INT32 rb_point_x[MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_X_SIZE];
	AGTX_INT32 rb_point_y[MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_Y_SIZE];
	AGTX_INT32 rb_rgn_slope[MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_SLOPE_SIZE];
	AGTX_INT32 rb_rgn_th[MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_TH_SIZE];
	AGTX_INT32 gwd_auto_lum_thd_enable;
	AGTX_INT32 gwd_auto_lum_max_degree;
	AGTX_UINT64 gwd_auto_indoor_ev_thd;
	AGTX_UINT64 gwd_auto_outdoor_ev_thd;
	AGTX_INT32 gwd_auto_indoor_lum_range;
	AGTX_INT32 gwd_auto_outdoor_lum_range;
	AGTX_INT32 gwd_auto_min_lum_bnd;
} AGTX_DIP_STAT_WB_CONF_S;

typedef struct {
	AGTX_INT32 video_dev_idx;
	AGTX_DIP_STAT_WB_CONF_S wb;
} AGTX_DIP_STAT_CONF_S;


#endif /* AGTX_DIP_STAT_CONF_H_ */
