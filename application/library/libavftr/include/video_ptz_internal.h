#ifndef VIDEO_PTZ_INTERNAL_H_
#define VIDEO_PTZ_INTERNAL_H_

#include <pthread.h>

#include "mpi_dev.h"
#include "mpi_base_types.h"
#include "mpi_base_types.h"

#include "avftr_conn.h"

#define VIDEO_PTZ_ADJ_ROI (4)
#define VIDEO_PTZ_DELTA_LIM (4)

#define VIDEO_FTR_PTZ_ROUTINE_STEPS_MAX 10

#define PTZ_MAX_UPDATE_RATIO (64)
#define PTZ_IIR_UPDATE_RATIO                                                                                           \
	(((32 * (1 << VIDEO_PTZ_AR_FRACTIONAL_BIT)) + (PTZ_MAX_UPDATE_RATIO >> 1)) / PTZ_MAX_UPDATE_RATIO)

#define PTZ_POS(a, b)                                                                                                  \
	(MPI_POINT_S)                                                                                                  \
	{                                                                                                              \
		.y = (a), .x = (b)                                                                                     \
	}

typedef struct {
	UINT8 steps_cur;
	UINT8 steps_num;
	MPI_POINT_S steps[VIDEO_FTR_PTZ_ROUTINE_STEPS_MAX];
	//MPI_RECT_POINT_S step_rect[VIDEO_FTR_PTZ_ROUTINE_MAX]; /* Prefined target step */
} VIDEO_FTR_PTZ_ROUTINE_S;

typedef struct {
	INT32 width;
	INT32 height;
} VIDEO_FTR_PTZ_SIZE_S;

typedef enum {
	VIDEO_FTR_PTZ_POS_TRACK,
	VIDEO_FTR_PTZ_POS_DIR,
} VIDEO_FTR_PTZ_POS_STATE;

typedef enum {
	VIDEO_FTR_PTZ_AUTO_MANUAL, /* Init Manual state in Auto mode */
	VIDEO_FTR_PTZ_AUTO_MANUAL_RUN, /* State to track timeout */
	VIDEO_FTR_PTZ_AUTO_RUN, /* State to track wait time */
} VIDEO_FTR_PTZ_AUTO_MODE_STATE;

typedef struct {
	union {
		struct {
			int sx;
			int sy;
			int ex;
			int ey;
		};
		int coord[4];
	};
} VFTR_RECT_POINT_S;

typedef struct {
	union {
		int val;
		struct {
			int x;
			int y;
		};
		int coord[2];
	};
} VFTR_POINT_S;

typedef struct {
	union {
		UINT32 flag;
		struct {
			union {
				UINT16 pos;
				struct {
					UINT8 pos_zoom; /* motion flag for zoom changes */
					UINT8 pos_mv; /* motion flag for mv_pos */
				};
			};
			UINT16 mv; /* motion flag for zoom_v and mv */
		};
	};
} PTZ_MOTION_FLAG;

typedef struct {
	UINT8 enabled;
	UINT8 motion_apply;
	UINT32 zoom_lvl_target; /**< Current Zoom level in fractional number */
	UINT32 timer;
	INT32 motion_sta;
	UINT32 retval;
	UINT8 roi_set[VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE];
	pthread_mutex_t lock;
	MPI_RECT_POINT_S pos; /* Current ePTZ position */
	MPI_RECT_POINT_S auto_pos; /* Current AROI position */
	MPI_RECT_S roi_default[VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE]; /* default roi when initialize PTZ */
	MPI_RECT_S roi_boundary; /* Sensor roi boundary */
	MPI_SIZE_S roi_bd;
	VFTR_POINT_S mv_pos; /* translated motion position */
	MPI_SIZE_S roi_bd_target; /* translated roi_bd */
	VIDEO_FTR_PTZ_POS_STATE state;
	VIDEO_FTR_PTZ_AUTO_MODE_STATE auto_sta;
	VIDEO_FTR_PTZ_ROUTINE_S routine;
	VIDEO_FTR_PTZ_PARAM_S param;
} VIDEO_FTR_PTZ_CTX_S;

#endif /* !VIDEO_PTZ_INTERNAL_H_ */