#ifndef AGTX_DIP_NR_WIN_CONF_H_
#define AGTX_DIP_NR_WIN_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_2D_LIST_SIZE 11
#define MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_3D_LIST_SIZE 11
#define MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE 11
#define MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_2D_LIST_SIZE 11
#define MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_3D_LIST_SIZE 11
#define MAX_AGTX_NR_STRM_PARAM_S_WINDOW_ARRAY_SIZE 9
#define MAX_AGTX_DIP_NR_WIN_CONF_S_VIDEO_STRM_SIZE 4

typedef struct {
	AGTX_INT32 auto_c_level_2d_list[MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_2D_LIST_SIZE];
	AGTX_INT32 auto_c_level_3d_list[MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_3D_LIST_SIZE];
	AGTX_INT32 auto_fss_y_level_3d_list[MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE];
	AGTX_INT32 auto_y_level_2d_list[MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_2D_LIST_SIZE];
	AGTX_INT32 auto_y_level_3d_list[MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_3D_LIST_SIZE];
	AGTX_INT32 fss_ratio_max;
	AGTX_INT32 fss_ratio_min;
	AGTX_INT32 ghost_remove;
	AGTX_NR_LUT_TYPE_E lut_type;
	AGTX_INT32 ma_c_strength;
	AGTX_INT32 ma_y_strength;
	AGTX_INT32 manual_c_level_2d; /* This is a description. */
	AGTX_INT32 manual_c_level_3d;
	AGTX_INT32 manual_fss_y_level_3d;
	AGTX_INT32 manual_y_level_2d;
	AGTX_INT32 manual_y_level_3d;
	AGTX_INT32 mc_y_level_offset;
	AGTX_INT32 mc_y_strength;
	AGTX_INT32 me_frame_fallback_en;
	AGTX_INT32 mode;
	AGTX_INT32 motion_comp;
	AGTX_INT32 ratio_3d;
	AGTX_INT32 trail_suppress;
	AGTX_INT32 window_idx;
} AGTX_NR_WINDOW_PARAM_S;

typedef struct {
	AGTX_INT32 video_strm_idx;
	AGTX_NR_WINDOW_PARAM_S window_array[MAX_AGTX_NR_STRM_PARAM_S_WINDOW_ARRAY_SIZE];
	AGTX_INT32 window_num;
} AGTX_NR_STRM_PARAM_S;

typedef struct {
	AGTX_INT32 strm_num;
	AGTX_INT32 video_dev_idx;
	AGTX_NR_STRM_PARAM_S video_strm[MAX_AGTX_DIP_NR_WIN_CONF_S_VIDEO_STRM_SIZE];
	AGTX_INT32 win_nr_en;
} AGTX_DIP_NR_WIN_CONF_S;


#endif /* AGTX_DIP_NR_WIN_CONF_H_ */
