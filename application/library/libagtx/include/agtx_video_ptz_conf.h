#ifndef AGTX_VIDEO_PTZ_CONF_H_
#define AGTX_VIDEO_PTZ_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_SUBWIN_DISP_S_WIN_SIZE 4
typedef enum {
	AGTX_VIDEO_PTZ_MODE_AUTO,
	AGTX_VIDEO_PTZ_MODE_MANUAL,
	AGTX_VIDEO_PTZ_MODE_SCAN
} AGTX_VIDEO_PTZ_MODE_E;


typedef struct {
	AGTX_INT32 chn_idx;
	AGTX_INT32 win_idx;
} AGTX_SUBWIN_PARAM_S;

typedef struct {
	AGTX_SUBWIN_PARAM_S win[MAX_AGTX_SUBWIN_DISP_S_WIN_SIZE];
	AGTX_INT32 win_num;
} AGTX_SUBWIN_DISP_S;

typedef struct {
	AGTX_INT32 enabled;
	AGTX_VIDEO_PTZ_MODE_E mode;
	AGTX_INT32 roi_height;
	AGTX_INT32 roi_width;
	AGTX_INT32 speed_x; /* Moving position speed x. */
	AGTX_INT32 speed_y; /* Moving position speed y. */
	AGTX_SUBWIN_DISP_S subwindow_disp;
	AGTX_INT32 win_pos_x; /* Target window position x. */
	AGTX_INT32 win_pos_y; /* Target window position y. */
	AGTX_INT32 win_size_limit_max; /* Window size maximum limit [1-1024] */
	AGTX_INT32 win_size_limit_min; /* Window size minimum limit [1-1024] */
	AGTX_INT32 win_speed_x;
	AGTX_INT32 win_speed_y;
	AGTX_INT32 zoom_change; /* Zoom Change [-1 0 1] */
	AGTX_INT32 zoom_level; /* Zoom level */
	AGTX_INT32 zoom_speed_height;
	AGTX_INT32 zoom_speed_width;
} AGTX_VIDEO_PTZ_CONF_S;


#endif /* AGTX_VIDEO_PTZ_CONF_H_ */
