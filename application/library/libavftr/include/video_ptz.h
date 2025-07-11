#ifndef VIDEO_PTZ_H_
#define VIDEO_PTZ_H_

#include <pthread.h>

#include "mpi_dev.h"
#include "mpi_base_types.h"

#include "avftr_common.h"

#define VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE 4 /**< Max number of subwindow */
#define VIDEO_FTR_PTZ_MAX_WAIT_TIME (5 * AVFTR_VIDEO_JIF_HZ) /**< Max wait time duration for AUTO-MANUAL MODE */
#define VIDEO_FTR_PTZ_TIMEOUT \
	(30 * AVFTR_VIDEO_JIF_HZ) /**< Max time out duration(force release) for AUTO-MANUAL MODE */

#define VIDEO_FTR_PTZ_ZOOM_LVL_MIN 1 /**< Min zoom level in fractional number VIDEO_FTR_PTZ_ZOOM_PREC */
#define VIDEO_FTR_PTZ_ZOOM_LVL_MIN_BND 2 /**< Min hard zoom level in fractional number VIDEO_FTR_PTZ_ZOOM_PREC */
#define VIDEO_FTR_PTZ_ZOOM_LVL_MAX 4 /**< Max zoom level in fractional number VIDEO_FTR_PTZ_ZOOM_PREC */
#define VIDEO_FTR_PTZ_ZOOM_CHANGE_MIN 0 /**< Min zoom change in fractional number VIDEO_FTR_PTZ_ZOOM_PREC */
#define VIDEO_FTR_PTZ_ZOOM_CHANGE_MAX 4 /**< Max zoom change in fractional number VIDEO_FTR_PTZ_ZOOM_PREC */
#define VIDEO_FTR_PTZ_ZOOM_PREC 10
#define VIDEO_FTR_PTZ_ZOOM_UNIT (1 << VIDEO_FTR_PTZ_ZOOM_PREC)
#define VIDEO_FTR_PTZ_WIN_SIZE_MIN 1
#define VIDEO_FTR_PTZ_WIN_SIZE_MAX 1024

#define VIDEO_PTZ_AR_FRACTIONAL_BIT (8)
#define VIDEO_PTZ_AR_FRACTIONAL_INV_BIT (2)

#define VIDEO_PTZ_MV_XY_BYPASS (32767) /* BY PASS */
#define VIDEO_PTZ_MV_POS_STEADY (32767) /* No change */
#define VIDEO_PTZ_MV_POS_PROBE (32766) /* PROBE probe api need to be used with same ptz mode*/ 
#define VIDEO_PTZ_MV_POS_MIN (0)
#define VIDEO_PTZ_MAX_RES_W (1024) /**< Max width for PTZ */
#define VIDEO_PTZ_MAX_RES_H (1024) /**< Max height for PTZ */

#define VIDEO_PTZ_RET_TYPE_BS 27
#define VIDEO_PTZ_RET_TYPE_ZLVL 1
#define VIDEO_PTZ_RET_TYPE_COOR 2
#define VIDEO_PTZ_RET_TYPE_ERRON 3

#define VIDEO_PTZ_RET_X_BS 0
#define VIDEO_PTZ_RET_Y_BS 10
#define VIDEO_PTZ_RET_ZLVL_BS 0
#define VIDEO_PTZ_RET_ERRON_BS 0

typedef enum {
	VIDEO_FTR_PTZ_REACH_MIN_ZOOM_LVL = 1,
	VIDEO_FTR_PTZ_REACH_MAX_ZOOM_LVL,
} VIDEO_FTR_PTZ_ERRON_E;

/**
 * @brief Enum of video pan-tilt-zoom modes.
 */
typedef enum {
	VIDEO_FTR_PTZ_MODE_AUTO = 0,
	VIDEO_FTR_PTZ_MODE_MANUAL,
	VIDEO_FTR_PTZ_MODE_SCAN,
	VIDEO_FTR_PTZ_MODE_NUM,
} VIDEO_FTR_PTZ_MODE_E;

/**
 * @brief Structure of video pan-tilt-zoom attributes.
 */
typedef struct {
	UINT8 win_num;
	INT32 zoom_change; /**< Zoom level change [-ve, 0(no change), +ve] in fractional number*/
	UINT32 zoom_lvl; /**< Zoom level in fractional number */
	VIDEO_FTR_PTZ_MODE_E mode; /**< PTZ mode */
	MPI_MOTION_VEC_S mv; /**< Manual panning */
	MPI_MOTION_VEC_S zoom_v; /**< Manual zooming speed*/
	MPI_MOTION_VEC_S speed; /**< Speed of moving w.r.t. mv_pos */
	MPI_POINT_S mv_pos; /**< Absolute motion position request */
	MPI_SIZE_S roi_bd; /**< Default ROI size */
	MPI_RANGE_S win_size_limit; /**< NOTE. customized window size limit constraint */
	MPI_WIN win_id[VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE];
} VIDEO_FTR_PTZ_PARAM_S;

int VIDEO_FTR_enablePtz(void);
int VIDEO_FTR_disablePtz(void);
int VIDEO_FTR_getPtzStat(void);
int VIDEO_FTR_setPtzParam(const VIDEO_FTR_PTZ_PARAM_S *param);
int VIDEO_FTR_getPtzParam(VIDEO_FTR_PTZ_PARAM_S *param);
int VIDEO_FTR_updateAutoPtz(MPI_RECT_POINT_S *roi);
int VIDEO_FTR_setPtzResult(UINT32 timestamp);

#endif
