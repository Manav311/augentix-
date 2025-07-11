#ifndef AGTX_DIP_SHP_WIN_CONF_H_
#define AGTX_DIP_SHP_WIN_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_SHP_STRM_PARAM_S_WINDOW_ARRAY_SIZE 9
#define MAX_AGTX_DIP_SHP_WIN_CONF_S_VIDEO_STRM_SIZE 4

typedef struct {
	AGTX_INT32 strength;
	AGTX_INT32 window_idx;
} AGTX_SHP_WINDOW_PARAM_S;

typedef struct {
	AGTX_INT32 video_strm_idx;
	AGTX_SHP_WINDOW_PARAM_S window_array[MAX_AGTX_SHP_STRM_PARAM_S_WINDOW_ARRAY_SIZE];
	AGTX_INT32 window_num;
} AGTX_SHP_STRM_PARAM_S;

typedef struct {
	AGTX_INT32 strm_num;
	AGTX_INT32 video_dev_idx;
	AGTX_SHP_STRM_PARAM_S video_strm[MAX_AGTX_DIP_SHP_WIN_CONF_S_VIDEO_STRM_SIZE];
	AGTX_INT32 win_shp_en;
} AGTX_DIP_SHP_WIN_CONF_S;


#endif /* AGTX_DIP_SHP_WIN_CONF_H_ */
