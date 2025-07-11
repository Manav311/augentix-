#include "video_ptz.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avftr_log.h"

#include "video_ptz_internal.h"

//#define VFTR_PTZ_API_DEBUG
#ifdef VFTR_PTZ_API_DEBUG
#define PTZ_FMT(fmt) "[PTZ] %s:%d " fmt
#define PTZ_API_DEBUG(fmt, args...) printf(PTZ_FMT(fmt), __func__, __LINE__, ##args)
#else
#define PTZ_API_DEBUG(fmt, args...)
#endif

/* static function template */
static inline int clamp(int x, int low, int high);
static inline void ptz_decodeRoi(const MPI_RECT_S *roi, const MPI_RECT_S *roi_boundary, MPI_RECT_S *o_roi);
static inline void ptz_decodeRoiPt(const VFTR_POINT_S *pt, const MPI_RECT_S *roi_corr, VFTR_POINT_S *pt_corr);
static inline INT32 ptz_checkMotionApply(const VIDEO_FTR_PTZ_PARAM_S *src_p);
static INT32 ptz_checkParamAndConvert(VIDEO_FTR_PTZ_PARAM_S *o_p);
static INT32 ptz_setParamToCtx(const VIDEO_FTR_PTZ_PARAM_S *o_p, const MPI_RECT_S *roi, VIDEO_FTR_PTZ_CTX_S *ctx);
static inline INT32 ptz_setDefaultWinBoundary(VIDEO_FTR_PTZ_CTX_S *ctx);
static inline INT32 ptz_resetSubWinROI(VIDEO_FTR_PTZ_CTX_S *ctx, VIDEO_FTR_PTZ_PARAM_S *param, int flag);
static void ptz_roiConvertBoundary(const MPI_RECT_S *roi, const MPI_RECT_S *roi_win, MPI_RECT_S *roi_corr);
static inline void ptz_iir(const MPI_RECT_POINT_S *pre_sta, VFTR_RECT_POINT_S *cur_sta_iir,
                           const MPI_RECT_POINT_S *cur_sta, INT32 update_ratio);
static void alignBoundary(MPI_RECT_POINT_S *rect, MPI_SIZE_S *res);
static inline void getBoxFromBox(const MPI_RECT_POINT_S *cur, const MPI_SIZE_S *sz, MPI_RECT_POINT_S *result);
static inline void getBoxSize(const VFTR_POINT_S *ctr, const MPI_SIZE_S *sz, MPI_RECT_POINT_S *result);
static inline void getBox(const MPI_RECT_S *cur, MPI_RECT_POINT_S *result);
static void alignDelta(const MPI_MOTION_VEC_S *speed, const MPI_RECT_POINT_S *pre_sta, MPI_RECT_POINT_S *curr_sta,
                       MPI_SIZE_S *win_res);
static void trackTargetPos(const MPI_MOTION_VEC_S *speed, const MPI_RECT_POINT_S *prev, const MPI_RECT_POINT_S *target,
                           MPI_RECT_POINT_S *result, MPI_SIZE_S *roi_bd);
static void ptz_setRoutineTarget(int step_target, const VIDEO_FTR_PTZ_ROUTINE_S *routine, const MPI_SIZE_S *size,
                                 MPI_RECT_POINT_S *target);
static int ptz_setRoutineCheck(const MPI_RECT_POINT_S *cur, const MPI_RECT_POINT_S *target);
static void ptz_setRoutineStep(VIDEO_FTR_PTZ_CTX_S *ctx, MPI_RECT_POINT_S *p, MPI_RECT_POINT_S *target);
static void resetPtz(VIDEO_FTR_PTZ_CTX_S *ctx);
static void ptz_setRoutine(VIDEO_FTR_PTZ_ROUTINE_S *routine);
static inline void ptz_setParamMotionOff(VIDEO_FTR_PTZ_PARAM_S *p, VIDEO_FTR_PTZ_CTX_S *ctx);
static void ptz_printParam(const VIDEO_FTR_PTZ_PARAM_S *p);

static VIDEO_FTR_PTZ_CTX_S g_ptz_ctx;

static inline int clamp(int x, int low, int high)
{
	return (x < low) ? low : ((x > high) ? high : x);
}

static void ptz_printParam(const VIDEO_FTR_PTZ_PARAM_S *p __attribute__((unused)))
{
	PTZ_API_DEBUG("PTZ param-> win num:%d, zm_ch:%d, zm_lvl:%d, mode:%d, mv.xy:%d %d, zm_v.xy:%d %d, spd.xy:%d %d"
	              "mv_pos:%d %d, roi_bd:%d %d, win_id[0]: 0%x win_id[1]: 0%x\n",
	              p->win_num, p->zoom_change, p->zoom_lvl, p->mode, p->mv.x, p->mv.y, p->zoom_v.x, p->zoom_v.y,
	              p->speed.x, p->speed.y, p->mv_pos.x, p->mv_pos.y, p->roi_bd.width, p->roi_bd.height,
	              p->win_id[0].value, p->win_id[1].value);
}

/**
 *@brief Convert Coordinate w.r.t. ROI boundary
 */
static inline void ptz_decodeRoi(const MPI_RECT_S *roi, const MPI_RECT_S *roi_boundary, MPI_RECT_S *o_roi)
{
	o_roi->x =
	        (((roi->x - roi_boundary->x) * VIDEO_PTZ_MAX_RES_W + (roi_boundary->width >> 1)) / roi_boundary->width);
	o_roi->y = (((roi->y - roi_boundary->y) * VIDEO_PTZ_MAX_RES_W + (roi_boundary->height >> 1)) /
	            roi_boundary->height);
	o_roi->width = (roi->width * VIDEO_PTZ_MAX_RES_W + (roi_boundary->width >> 1)) / roi_boundary->width;
	o_roi->height = (roi->height * VIDEO_PTZ_MAX_RES_H + (roi_boundary->height >> 1)) / roi_boundary->height;
}

/**
 *@brief Convert Coordinate w.r.t. input roi resolution
 */
static inline void ptz_decodeRoiPt(const VFTR_POINT_S *pt, const MPI_RECT_S *roi_corr, VFTR_POINT_S *pt_corr)
{
	int tmp = (VIDEO_PTZ_MAX_RES_W - roi_corr->width);
	if (tmp != 0)
		pt_corr->x = (((pt->x - ((roi_corr->width + 1) / 2)) * 1023) + (tmp / 2)) / tmp;
	else
		pt_corr->x = pt->x;
	tmp = (VIDEO_PTZ_MAX_RES_H - roi_corr->height);
	if (tmp != 0)
		pt_corr->y = (((pt->y - ((roi_corr->height + 1) / 2)) * 1023) + (tmp / 2)) / tmp;
	else
		pt_corr->y = pt->y;
}

static inline INT32 ptz_checkMotionApply(const VIDEO_FTR_PTZ_PARAM_S *p)
{
	return (p->zoom_lvl != 1 << VIDEO_FTR_PTZ_ZOOM_PREC) || (p->mv.x != VIDEO_PTZ_MV_XY_BYPASS) ||
	       (p->mv.y != VIDEO_PTZ_MV_XY_BYPASS) || (p->zoom_v.x != 0) || (p->zoom_v.x != 0) ||
	       (p->mv_pos.x != VIDEO_PTZ_MV_POS_STEADY) || (p->mv_pos.y != VIDEO_PTZ_MV_POS_STEADY) ||
	       (p->zoom_change != 1 << VIDEO_FTR_PTZ_ZOOM_PREC);
}

static inline void ptz_setParamMotionOff(VIDEO_FTR_PTZ_PARAM_S *p, VIDEO_FTR_PTZ_CTX_S *ctx)
{
	p->zoom_lvl = 1 << VIDEO_FTR_PTZ_ZOOM_PREC;
	p->mv.x = VIDEO_PTZ_MV_XY_BYPASS;
	p->mv.y = VIDEO_PTZ_MV_XY_BYPASS;
	p->zoom_v.x = 0;
	p->zoom_v.y = 0;
	p->mv_pos.x = VIDEO_PTZ_MV_POS_STEADY;
	p->mv_pos.y = VIDEO_PTZ_MV_POS_STEADY;
	p->zoom_change = 1 << VIDEO_FTR_PTZ_ZOOM_PREC;
	ctx->mv_pos.x = VIDEO_PTZ_MV_POS_STEADY;
	ctx->mv_pos.y = VIDEO_PTZ_MV_POS_STEADY;
}

static INT32 ptz_checkParamAndConvert(VIDEO_FTR_PTZ_PARAM_S *o_p)
{
	/* return fail */
	if ((o_p->mode < 0) || (o_p->mode >= VIDEO_FTR_PTZ_MODE_NUM)) {
		avftr_log_warn("PTZ Mode requested is not supported. Force Auto mode.");
		o_p->mode = VIDEO_FTR_PTZ_MODE_AUTO;
	}

	if (o_p->win_size_limit.min < VIDEO_FTR_PTZ_WIN_SIZE_MIN) {
		avftr_log_warn("Window size limit exceeds minimum threshold(%d), Force clipped.",
		               o_p->win_size_limit.min);
		o_p->win_size_limit.min = VIDEO_FTR_PTZ_WIN_SIZE_MIN;
	}

	if (o_p->win_size_limit.max > VIDEO_FTR_PTZ_WIN_SIZE_MAX) {
		avftr_log_warn("Window size limit exceeds maximum threshold(%d), Force clipped.",
		               o_p->win_size_limit.max);
		o_p->win_size_limit.max = VIDEO_FTR_PTZ_WIN_SIZE_MAX;
	}

	if ((o_p->mv_pos.x != VIDEO_PTZ_MV_POS_STEADY && o_p->mv_pos.x != VIDEO_PTZ_MV_POS_PROBE) &&
		(o_p->mv_pos.y != VIDEO_PTZ_MV_POS_STEADY && o_p->mv_pos.y != VIDEO_PTZ_MV_POS_PROBE)) {
		if ((o_p->mv_pos.x >= VIDEO_PTZ_MAX_RES_W) || (o_p->mv_pos.y >= VIDEO_PTZ_MAX_RES_H)) {
			avftr_log_warn("Motion pos(%d,%d) exceed boundary(%d,%d,%d,%d). Force clipped.",
			               o_p->mv_pos.x, o_p->mv_pos.y, VIDEO_PTZ_MV_POS_MIN, VIDEO_PTZ_MV_POS_MIN,
			               VIDEO_PTZ_MAX_RES_W, VIDEO_PTZ_MAX_RES_H);
			o_p->mv_pos.x = MIN(o_p->mv_pos.x, VIDEO_PTZ_MAX_RES_W - 1);
			o_p->mv_pos.y = MIN(o_p->mv_pos.y, VIDEO_PTZ_MAX_RES_H - 1);
		}
	}
	if (o_p->zoom_lvl < (VIDEO_FTR_PTZ_ZOOM_LVL_MIN << VIDEO_FTR_PTZ_ZOOM_PREC) ||
	    o_p->zoom_lvl > (VIDEO_FTR_PTZ_ZOOM_LVL_MAX << VIDEO_FTR_PTZ_ZOOM_PREC)) {
		avftr_log_warn("Zoom lvl(%d) exceed boundary(%d,%d). Force clipped.", o_p->zoom_lvl,
		               VIDEO_FTR_PTZ_ZOOM_LVL_MIN << VIDEO_FTR_PTZ_ZOOM_PREC,
		               VIDEO_FTR_PTZ_ZOOM_LVL_MAX << VIDEO_FTR_PTZ_ZOOM_PREC);
		o_p->zoom_lvl = MIN(o_p->zoom_lvl, VIDEO_FTR_PTZ_ZOOM_LVL_MAX << VIDEO_FTR_PTZ_ZOOM_PREC);
		o_p->zoom_lvl = MAX(o_p->zoom_lvl, VIDEO_FTR_PTZ_ZOOM_LVL_MIN << VIDEO_FTR_PTZ_ZOOM_PREC);
	}
	if (o_p->win_num > VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE) {
		avftr_log_warn("Window number(%d) exceed maximum value(%d). Force clipped.", o_p->win_num,
		               VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE);
		o_p->win_num = VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE;
	}

	if (o_p->zoom_change <= (VIDEO_FTR_PTZ_ZOOM_CHANGE_MIN << VIDEO_FTR_PTZ_ZOOM_PREC) ||
	    o_p->zoom_change > (VIDEO_FTR_PTZ_ZOOM_CHANGE_MAX << VIDEO_FTR_PTZ_ZOOM_PREC)) {
		avftr_log_warn("Zoom_change(%d) exceed boundary[%d %d]). Force clipped.", o_p->zoom_change,
		               VIDEO_FTR_PTZ_ZOOM_CHANGE_MIN << VIDEO_FTR_PTZ_ZOOM_PREC,
		               VIDEO_FTR_PTZ_ZOOM_CHANGE_MAX << VIDEO_FTR_PTZ_ZOOM_PREC);
		o_p->zoom_change = MIN(o_p->zoom_change, VIDEO_FTR_PTZ_ZOOM_CHANGE_MAX << VIDEO_FTR_PTZ_ZOOM_PREC);
		o_p->zoom_change =
		        MAX(o_p->zoom_change, (VIDEO_FTR_PTZ_ZOOM_CHANGE_MIN << VIDEO_FTR_PTZ_ZOOM_PREC) + 1);
	}

	int mv_dir = (o_p->mv.x != VIDEO_PTZ_MV_XY_BYPASS || o_p->mv.y != VIDEO_PTZ_MV_XY_BYPASS);
	int mv_zoom = (o_p->zoom_v.x || o_p->zoom_v.y);

	int mv_pos = ((o_p->mv_pos.x != VIDEO_PTZ_MV_POS_STEADY && o_p->mv_pos.x != VIDEO_PTZ_MV_POS_PROBE) ||
	              (o_p->mv_pos.y != VIDEO_PTZ_MV_POS_STEADY && o_p->mv_pos.y != VIDEO_PTZ_MV_POS_PROBE));
	int mv_zoom_lvl = (o_p->zoom_lvl > (1 << VIDEO_FTR_PTZ_ZOOM_PREC));
	int mv_zoom_ch = (o_p->zoom_change != (1 << VIDEO_FTR_PTZ_ZOOM_PREC));

	if ((mv_dir || mv_zoom) && (mv_pos || mv_zoom_lvl || mv_zoom_ch)) {
		avftr_log_warn("PTZ can only control one mode (mv,zoom_v),(zoom_change,mv_pos) at a time."
		               " Force Cancel (mv,zoom_v) motion");
		o_p->mv.x = VIDEO_PTZ_MV_XY_BYPASS;
		o_p->mv.y = VIDEO_PTZ_MV_XY_BYPASS;
		o_p->zoom_v.x = 0;
		o_p->zoom_v.y = 0;
	}

	if (mv_zoom_lvl && mv_zoom_ch) {
		avftr_log_warn("Conflict zoom request for zoom level(%d) and zoom change(%d)."
		               "Force cancel zoom_level request.",
		               o_p->zoom_lvl, o_p->zoom_change);
		o_p->zoom_lvl = 1 << VIDEO_FTR_PTZ_ZOOM_PREC;
	}

#if 0 /* check validity of input window */
	int i, j;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn;
	for (i = 0; i < o_p->win_num; i++) {
		chn = MPI_VIDEO_CHN(0,o_p->win_id[i].chn);
		if (MPI_DEV_getChnLayout(chn, &layout_attr) != MPI_SUCCESS) {
			ld_warn("Get video channel %d attributes failed.\n", chn_idx);
			return -1;
		}
		for (j = 0; i < layout_attr.window_num; j++) {
			if (o_p->win_id[i].value == layout_attr.win_id[j].value) {
				break;
			}
		}
		if (j == layout_attr.window_num) {
			ld_warn("Invalid video window index %d from video channel %d", win_idx, chn_idx);
			return -1;
		}
	}
#endif
	return 0;
}

static INT32 ptz_setParamToCtx(const VIDEO_FTR_PTZ_PARAM_S *o_p, const MPI_RECT_S *roi, VIDEO_FTR_PTZ_CTX_S *ctx)
{
	ptz_printParam(o_p);
	VFTR_POINT_S *mv_pos = &ctx->mv_pos;
	MPI_SIZE_S *roi_bd_target = &ctx->roi_bd_target;
	MPI_RECT_S roi_corr = { 0 };
	VFTR_POINT_S pt = { { 0 } };
	VFTR_POINT_S pt_corr = { { 0 } };
	ctx->retval = 0;

	int ptz_min_roi_val = o_p->win_size_limit.min;
	int ptz_max_roi_val = o_p->win_size_limit.max;

	int zoom_res, zoom_h, zoom_w;
	UINT32 ratio;
	int zoom_res_min;
	int zoom_res_val_h, zoom_res_val_l;
	int zoom_lvl_cur;
	int zoom_tmp = 0;
	int zoom_lvl_target = 0;

	/* Should not enter here */
	if ((roi->width == 0 && roi->height == 0) || o_p->win_num == 0) {
		roi_bd_target->width = 512;
		roi_bd_target->height = 512;
		mv_pos->x = VIDEO_PTZ_MV_POS_STEADY;
		mv_pos->y = VIDEO_PTZ_MV_POS_STEADY;
		return 0;
	}
	ptz_decodeRoi(roi, &ctx->roi_boundary, &roi_corr);
	pt.x = (roi_corr.x + (roi_corr.x + roi_corr.width) + 1) / 2;
	pt.y = (roi_corr.y + (roi_corr.y + roi_corr.height) + 1) / 2;
	ptz_decodeRoiPt(&pt, &roi_corr, &pt_corr);

	/* Motion flag checking */
	int ck_zoom_lvl = o_p->zoom_lvl > (1 << VIDEO_FTR_PTZ_ZOOM_PREC);
	int ck_zoom_ch = o_p->zoom_change != (1 << VIDEO_FTR_PTZ_ZOOM_PREC);
	int ck_pos_mv = (o_p->mv_pos.x != VIDEO_PTZ_MV_POS_STEADY) || (o_p->mv_pos.y != VIDEO_PTZ_MV_POS_STEADY);
	int ck_mv_xy = (o_p->mv.x != VIDEO_PTZ_MV_XY_BYPASS || o_p->mv.y != VIDEO_PTZ_MV_XY_BYPASS);
	int ck_zoom_xy = (o_p->zoom_v.x != 0 || o_p->zoom_v.y != 0);

	int pos_x, pos_y;

	zoom_res_val_h = (roi_corr.width > roi_corr.height) ? roi_corr.width : roi_corr.height;
	zoom_res_val_l = (roi_corr.width < roi_corr.height) ? roi_corr.width : roi_corr.height;

	zoom_lvl_cur = (VIDEO_PTZ_MAX_RES_W << VIDEO_FTR_PTZ_ZOOM_PREC) / zoom_res_val_l;
	if ((zoom_lvl_cur >= (VIDEO_FTR_PTZ_ZOOM_LVL_MAX << VIDEO_FTR_PTZ_ZOOM_PREC)) &&
	    (o_p->zoom_change > (1 << VIDEO_FTR_PTZ_ZOOM_PREC))) {
		ctx->retval = VIDEO_PTZ_RET_TYPE_ERRON << VIDEO_PTZ_RET_TYPE_BS;
		ctx->retval |= VIDEO_FTR_PTZ_REACH_MAX_ZOOM_LVL;
		return ctx->retval;
	}
	zoom_lvl_cur = (VIDEO_PTZ_MAX_RES_W << VIDEO_FTR_PTZ_ZOOM_PREC) / zoom_res_val_h;
	if ((zoom_lvl_cur <= (VIDEO_FTR_PTZ_ZOOM_LVL_MIN_BND << VIDEO_FTR_PTZ_ZOOM_PREC)) &&
	    (o_p->zoom_change < (1 << VIDEO_FTR_PTZ_ZOOM_PREC))) {
		ctx->retval = VIDEO_PTZ_RET_TYPE_ERRON << VIDEO_PTZ_RET_TYPE_BS;
		ctx->retval |= VIDEO_FTR_PTZ_REACH_MIN_ZOOM_LVL;
		return ctx->retval;
	}

	mv_pos->x = VIDEO_PTZ_MV_POS_STEADY;
	mv_pos->y = VIDEO_PTZ_MV_POS_STEADY;

	if (ck_zoom_lvl == 0 && ck_pos_mv == 0 && ck_zoom_ch == 0 && ck_mv_xy == 0 && ck_zoom_xy == 0) {
		return 0;
	}

	if (ck_zoom_xy) {
		zoom_h = roi_corr.height + 2 * o_p->zoom_v.y;
		zoom_w = roi_corr.width + 2 * o_p->zoom_v.x;
		roi_bd_target->width = clamp(zoom_w, ptz_min_roi_val, ptz_max_roi_val);
		roi_bd_target->height = clamp(zoom_h, ptz_min_roi_val, ptz_max_roi_val);
	} else {
		ratio = ((roi_corr.width << VIDEO_FTR_PTZ_ZOOM_PREC) + (roi_corr.height >> 1)) / roi_corr.height;
		if (roi_corr.width > roi_corr.height) {
			zoom_res_min = ((ptz_min_roi_val * ratio) + (1 << (VIDEO_FTR_PTZ_ZOOM_PREC - 1))) >>
			               VIDEO_FTR_PTZ_ZOOM_PREC;
		} else {
			zoom_res_min = ((ptz_min_roi_val << VIDEO_FTR_PTZ_ZOOM_PREC) + (ratio >> 1)) / ratio;
		}
		if (ck_zoom_lvl) {
			zoom_res = (VIDEO_PTZ_MAX_RES_W << VIDEO_FTR_PTZ_ZOOM_PREC) / o_p->zoom_lvl;
		} else if (ck_zoom_ch) {
			if (o_p->zoom_change > (1 << VIDEO_FTR_PTZ_ZOOM_PREC)) { /* Zoom in */
				zoom_tmp = zoom_lvl_cur + o_p->zoom_change - (1 << VIDEO_FTR_PTZ_ZOOM_PREC);
			} else { /* Zoom out */
				zoom_tmp = zoom_lvl_cur - (((1 << (VIDEO_FTR_PTZ_ZOOM_PREC * 2)) / o_p->zoom_change) -
				                           (1 << (VIDEO_FTR_PTZ_ZOOM_PREC)));
			}
			zoom_tmp = (zoom_tmp == 0) ? 1 : zoom_tmp;
			zoom_res = ((VIDEO_PTZ_MAX_RES_W << VIDEO_FTR_PTZ_ZOOM_PREC) + (zoom_tmp >> 1)) / zoom_tmp;
		} else {
			zoom_res = MAX(roi_corr.width, roi_corr.height);
		}
		zoom_res = clamp(zoom_res, zoom_res_min, ptz_max_roi_val);
		//printf("curres:%d zoom_res:%d curlvl:%d tar:%d zoom minres:%d, ratio:%d\n", MAX(roi->width, roi->height),,
		//zoom_res, zoom_lvl_cur, zoom_tmp, zoom_res_min, ratio);

		if (roi_corr.width > roi_corr.height) {
			roi_bd_target->width = zoom_res;
			roi_bd_target->height = (UINT32)((zoom_res << VIDEO_FTR_PTZ_ZOOM_PREC) + (ratio >> 1)) / ratio;
		} else {
			roi_bd_target->height = zoom_res;
			roi_bd_target->width =
			        ((zoom_res * ratio) + (1 << (VIDEO_FTR_PTZ_ZOOM_PREC - 1))) >> VIDEO_FTR_PTZ_ZOOM_PREC;
		}
	}
	//printf("final:%d (%d %d) \n", ctx->zoom_lvl_target, roi_bd_target->height , roi_bd_target->width);

	if (ck_pos_mv) {
		pos_x = o_p->mv_pos.x;
		pos_y = o_p->mv_pos.y;
		mv_pos->x = ((roi_bd_target->width + 1) / 2) + ((pos_x * (1024 - roi_bd_target->width)) / 1023);
		mv_pos->y = ((roi_bd_target->height + 1) / 2) + ((pos_y * (1024 - roi_bd_target->height)) / 1023);
	} else if (ck_mv_xy) {
		pos_x = pt_corr.x + o_p->mv.x;
		pos_y = pt_corr.y + o_p->mv.y;
		pos_x = ((roi_bd_target->width + 1) / 2) + (((pos_x * (1024 - roi_bd_target->width)) + 0) / 1023);
		pos_y = ((roi_bd_target->height + 1) / 2) + (((pos_y * (1024 - roi_bd_target->height)) + 0) / 1023);
		mv_pos->x = pos_x;
		mv_pos->y = pos_y;
	} else {
		pos_x = (roi_corr.x + (roi_corr.x + roi_corr.width) + 1) / 2;
		pos_y = (roi_corr.y + (roi_corr.y + roi_corr.height) + 1) / 2;
		mv_pos->x = pos_x;
		mv_pos->y = pos_y;
	}

	zoom_lvl_target = (VIDEO_PTZ_MAX_RES_W << VIDEO_FTR_PTZ_ZOOM_PREC) / roi_bd_target->width;
	if (ck_zoom_lvl || ck_zoom_ch || ck_zoom_xy) {
		ctx->retval = VIDEO_PTZ_RET_TYPE_ZLVL << VIDEO_PTZ_RET_TYPE_BS;
		ctx->retval |= (zoom_lvl_target & 0x3ffffff);
	} else if (ck_pos_mv || ck_mv_xy) {
		ctx->retval = VIDEO_PTZ_RET_TYPE_COOR << VIDEO_PTZ_RET_TYPE_BS;
		ctx->retval |= pt_corr.x;
		ctx->retval |= (pt_corr.y) << VIDEO_PTZ_RET_Y_BS;
	}

	ctx->zoom_lvl_target = zoom_lvl_target;
#if 0

	printf("%s %d: %d%d%d%d%d, o_p->xy(%d %d), roi:(%d %d) > (%d %d) cur:(%d %d) pos:(%d %d), mv_pos->xy(%d %d %d %d)\n",
	__func__, __LINE__, ck_zoom_lvl, ck_pos_mv, ck_zoom_ch, ck_mv_xy, ck_zoom_xy,
	o_p->mv.x, o_p->mv.y, roi->x, roi->y, pt_corr.x, pt_corr.y, roi_corr.width, roi_corr.height,
	pos_x, pos_y, mv_pos->x, mv_pos->y, roi_bd_target->width, roi_bd_target->height);
	printf("%s %d: pos_x:  %d %d  roi:(%d %d %d %d) + bnd(%d %d %d %d) => deco(%d %d %d %d)\n", __func__, __LINE__, pos_x, pos_y,
	roi->x, roi->y, roi->width, roi->height, ctx->roi_boundary.x, ctx->roi_boundary.y,
	ctx->roi_boundary.width, ctx->roi_boundary.height, roi_corr.x, roi_corr.y,
	roi_corr.width, roi_corr.height);
#endif
	return 0;
}

static inline INT32 ptz_setDefaultWinBoundary(VIDEO_FTR_PTZ_CTX_S *ctx)
{
	MPI_WIN bnd_win = MPI_VIDEO_WIN(0, 0, 0);
	MPI_RECT_S roi = { 0 };
	int ret = MPI_DEV_getWindowRoi(bnd_win, &roi);
	if (ret != MPI_SUCCESS) {
		return -1;
	}
	ctx->roi_boundary = roi;
	return 0;
}

static inline INT32 ptz_resetSubWinROI(VIDEO_FTR_PTZ_CTX_S *ctx, VIDEO_FTR_PTZ_PARAM_S *param, int flag)
{
	int i, ret = 0;
	for (i = 0; i < param->win_num; i++) {
		ret = MPI_DEV_setWindowRoi(param->win_id[i], &ctx->roi_default[i]);
		if (ret != MPI_SUCCESS) {
			return -1;
		}
		ctx->roi_set[i] = flag;
	}
	return ret;
}

static void ptz_roiConvertBoundary(const MPI_RECT_S *roi, const MPI_RECT_S *roi_win, MPI_RECT_S *roi_corr)
{
	roi_corr->x = roi_win->x + ((roi_win->width * roi->x + 512) / 1024);
	roi_corr->y = roi_win->y + ((roi_win->height * roi->y + 512) / 1024);
	roi_corr->width = ((roi_win->width * roi->width + 512) / 1024);
	roi_corr->height = ((roi_win->height * roi->height + 512) / 1024);
#if 0
	printf("%s %d [roi] (%d, %d, %d, %d) :: [winroi] (%d %d %d %d) => [roi_out] (%d %d %d %d)",
	__func__, __LINE__, roi->x, roi->y, roi->width, roi->height,
	roi_win->x, roi_win->y, roi_win->width, roi_win->height,
	roi_corr->x, roi_corr->y, roi_corr->width, roi_corr->height);
#endif
}

#define PTZ_IIR_PREC(prev, curr, ur) (((prev) << VIDEO_PTZ_AR_FRACTIONAL_BIT) + ((ur) * ((curr) - (prev))))

static inline void ptz_iir(const MPI_RECT_POINT_S *pre_sta, VFTR_RECT_POINT_S *cur_sta_iir,
                           const MPI_RECT_POINT_S *cur_sta, INT32 update_ratio)
{
	/* First Order IIR y[n] = y[n-1] + k ( x[n] - y[n-1] ) */

	cur_sta_iir->sx = PTZ_IIR_PREC(pre_sta->sx, cur_sta->sx, update_ratio);
	cur_sta_iir->sy = PTZ_IIR_PREC(pre_sta->sy, cur_sta->sy, update_ratio);
	cur_sta_iir->ex = PTZ_IIR_PREC(pre_sta->ex, cur_sta->ex, update_ratio);
	cur_sta_iir->ey = PTZ_IIR_PREC(pre_sta->ey, cur_sta->ey, update_ratio);
}

static void alignBoundary(MPI_RECT_POINT_S *rect, MPI_SIZE_S *res)
{
	if (rect->sx < 0) {
		rect->ex -= rect->sx;
		rect->sx = 0;
	}
	if (rect->sy < 0) {
		rect->ey -= rect->sy;
		rect->sy = 0;
	}
	if (rect->ex > (res->width - 1)) {
		rect->sx -= (rect->ex - res->width + 1);
		rect->ex = res->width - 1;
	}
	if (rect->ey > (res->height - 1)) {
		rect->sy -= (rect->ey - res->height + 1);
		rect->ey = res->height - 1;
	}
}

static inline void getBoxFromBox(const MPI_RECT_POINT_S *cur, const MPI_SIZE_S *sz, MPI_RECT_POINT_S *result)
{
	int cen_x = (cur->sx + cur->ex + 1) / 2;
	int cen_y = (cur->sy + cur->ey + 1) / 2;
	result->sx = cen_x - ((sz->width + 1) >> 1);
	result->sy = cen_y - ((sz->height + 1) >> 1);
	result->ex = cen_x + ((sz->width + 1) >> 1) - 1;
	result->ey = cen_y + ((sz->height + 1) >> 1) - 1;
}

static inline void getBoxSize(const VFTR_POINT_S *ctr, const MPI_SIZE_S *sz, MPI_RECT_POINT_S *result)
{
	result->sx = ctr->x - ((sz->width + 1) >> 1);
	result->sy = ctr->y - ((sz->height + 1) >> 1);
	result->ex = ctr->x + ((sz->width + 1) >> 1) - 1;
	result->ey = ctr->y + ((sz->height + 1) >> 1) - 1;
}

static inline void getBox(const MPI_RECT_S *cur, MPI_RECT_POINT_S *result)
{
	result->sx = cur->x - ((cur->width + 1) >> 1);
	result->sy = cur->y - ((cur->height + 1) >> 1);
	result->ex = cur->x + ((cur->width + 1) >> 1) - 1;
	result->ey = cur->y + ((cur->height + 1) >> 1) - 1;
}

static void getBoxWithAspectRatio(VFTR_RECT_POINT_S *cur_sta_iir, MPI_RECT_POINT_S *cur_sta, MPI_SIZE_S *roi_bd)
{
	MPI_RECT_S cur_box;

	cur_box.x = (cur_sta_iir->sx + cur_sta_iir->ex + (1 << VIDEO_PTZ_AR_FRACTIONAL_BIT)) >>
	            (1 + VIDEO_PTZ_AR_FRACTIONAL_BIT);
	cur_box.y = (cur_sta_iir->sy + cur_sta_iir->ey + (1 << VIDEO_PTZ_AR_FRACTIONAL_BIT)) >>
	            (1 + VIDEO_PTZ_AR_FRACTIONAL_BIT);

	cur_box.width = roi_bd->width;
	cur_box.height = roi_bd->height;

	getBox(&cur_box, cur_sta);
}

static void calDeltaPan(const MPI_RECT_POINT_S *pre_sta, const MPI_RECT_POINT_S *cur_sta, const MPI_SIZE_S *win_res,
                        const MPI_MOTION_VEC_S *speed, INT32 *new_delta)
{
	INT32 delta[4], diff, temp_delta;
	INT32 delta_limit_x, delta_limit_y;

	delta_limit_x = speed->x;
	delta_limit_y = speed->y;

	delta[0] = (pre_sta->ex + pre_sta->sx + 1) >> 1;
	delta[2] = (cur_sta->ex + cur_sta->sx + 1) >> 1;
	delta[1] = (pre_sta->ey + pre_sta->sy + 1) >> 1;
	delta[3] = (cur_sta->ey + cur_sta->sy + 1) >> 1;

	diff = delta[2] - delta[0];

	temp_delta = delta_limit_x - MAX(abs(new_delta[0]), abs(new_delta[2]));
	temp_delta = (temp_delta > abs(diff)) ? abs(diff) : temp_delta;
	temp_delta = SIGN(diff) * temp_delta;
	new_delta[0] += temp_delta;
	new_delta[2] += temp_delta;

	diff = delta[3] - delta[1];
	temp_delta = delta_limit_y - MAX(abs(new_delta[1]), abs(new_delta[3]));
	temp_delta = (temp_delta > abs(diff)) ? abs(diff) : temp_delta;
	temp_delta = SIGN(diff) * temp_delta;
	new_delta[1] += temp_delta;
	new_delta[3] += temp_delta;

	delta[0] = (INT32)pre_sta->sx + new_delta[0];
	delta[1] = (INT32)pre_sta->sy + new_delta[1];
	delta[2] = (INT32)pre_sta->ex + new_delta[2];
	delta[3] = (INT32)pre_sta->ey + new_delta[3];

	if (delta[0] < 0) {
		new_delta[0] -= delta[0];
		new_delta[2] -= delta[0];
	}

	if (delta[1] < 0) {
		new_delta[1] -= delta[1];
		new_delta[3] -= delta[1];
	}

	if (delta[2] > win_res->width - 1) {
		new_delta[0] -= (delta[2] - win_res->width + 1);
		new_delta[2] -= (delta[2] - win_res->width + 1);
	}

	if (delta[3] > win_res->height - 1) {
		new_delta[1] -= (delta[3] - win_res->height + 1);
		new_delta[3] -= (delta[3] - win_res->height + 1);
	}
}

static INT32 calDeltaScale(const MPI_RECT_POINT_S *pre_sta, const MPI_RECT_POINT_S *cur_sta,
                           const MPI_MOTION_VEC_S *speed, INT32 *new_delta)
{
	static UINT8 update = 0;

	INT32 delta_limit_x, delta_limit_y;

	delta_limit_x = speed->x;
	delta_limit_y = speed->y;

	INT32 delta[4];
	INT32 max = 0;
	INT32 max_x = 0, max_y = 0;
	INT32 i = 0;

	delta[0] = cur_sta->sx - (INT32)pre_sta->sx;
	delta[1] = cur_sta->sy - (INT32)pre_sta->sy;
	delta[2] = cur_sta->ex - (INT32)pre_sta->ex;
	delta[3] = cur_sta->ey - (INT32)pre_sta->ey;

	max_x = MAX(abs(delta[0]), abs(delta[2]));
	max_y = MAX(abs(delta[1]), abs(delta[3]));
	max = MAX(max_x, max_y);

	/* zero prevention */
	if (max == 0) {
		for (i = 0; i < 4; i++) {
			new_delta[i] = 0;
		}
		return update;
	}
	/* skip for small delta */
	if ((abs(delta[0]) <= delta_limit_x) && (abs(delta[1]) <= delta_limit_y) && (abs(delta[2]) <= delta_limit_x) &&
	    (abs(delta[3]) <= delta_limit_y)) {
		for (i = 0; i < 4; i++) {
			new_delta[i] = delta[i];
		}
		return update;
	}

	/* cal roi scaling within delta boundary */
	/* FIXME large delta limit induce overflow */
	if (delta_limit_x >= delta_limit_y) {
		for (i = 0; i < 4; i++) {
			new_delta[i] = (((delta[i] << (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2)) * delta_limit_x + (max >> 1)) /
			                max) >>
			               (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2);
		}

		if ((MAX(abs(new_delta[0]), abs(new_delta[2])) > delta_limit_x) ||
		    (MAX(abs(new_delta[1]), abs(new_delta[3])) > delta_limit_y)) {
			INT32 temp_delta = ((delta_limit_y * max)) / max_y;
			for (i = 0; i < 4; i++) {
				new_delta[i] =
				        (((delta[i] << (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2)) * temp_delta + (max >> 1)) /
				         max) >>
				        (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2);
			}
		}
	} else {
		for (i = 0; i < 4; i++) {
			new_delta[i] = (((delta[i] << (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2)) * delta_limit_y + (max >> 1)) /
			                max) >>
			               (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2);
		}

		if ((MAX(abs(new_delta[0]), abs(new_delta[2])) > delta_limit_x) ||
		    (MAX(abs(new_delta[1]), abs(new_delta[3])) > delta_limit_y)) {
			INT32 temp_delta = (delta_limit_x * max) / max_x;
			for (i = 0; i < 4; i++) {
				new_delta[i] =
				        (((delta[i] << (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2)) * temp_delta + (max >> 1)) /
				         max) >>
				        (VIDEO_PTZ_AR_FRACTIONAL_BIT / 2);
			}
		}
	}
	return !update;
}

static void alignDelta(const MPI_MOTION_VEC_S *speed, const MPI_RECT_POINT_S *pre_sta, MPI_RECT_POINT_S *curr_sta,
                       MPI_SIZE_S *win_res)
{
	/* [0]:dsx [1]:dsy [2]:dex [3]:dey */
	INT32 new_delta[4];

	/* cal detla for roi scaling */
	if (calDeltaScale(pre_sta, curr_sta, speed, new_delta)) {
		/* paning roi by reference centers within boundary */
		calDeltaPan(pre_sta, curr_sta, win_res, speed, new_delta);
	}

	curr_sta->sx = pre_sta->sx + new_delta[0];
	curr_sta->sy = pre_sta->sy + new_delta[1];
	curr_sta->ex = pre_sta->ex + new_delta[2];
	curr_sta->ey = pre_sta->ey + new_delta[3];
}

static void trackTargetPos(const MPI_MOTION_VEC_S *speed, const MPI_RECT_POINT_S *prev, const MPI_RECT_POINT_S *target,
                           MPI_RECT_POINT_S *result, MPI_SIZE_S *roi_bd)
{
	VFTR_RECT_POINT_S cur_sta_iir;
	MPI_SIZE_S win_size = { 1024, 1024 };
	//fprintf(stderr, "track prev = {%d, %d, %d, %d} >>>>>\n", prev->sx, prev->sy, prev->ex, prev->ey);
	//fprintf(stderr, "track target = {%d, %d, %d, %d}\n", target->sx, target->sy, target->ex, target->ey);
	ptz_iir(prev, &cur_sta_iir, target, PTZ_IIR_UPDATE_RATIO);
	//fprintf(stderr, "ptz_iir target = {%d, %d, %d, %d}\n", cur_sta_iir.sx, cur_sta_iir.sy, cur_sta_iir.ex, cur_sta_iir.ey);
	getBoxWithAspectRatio(&cur_sta_iir, result, roi_bd);

	//fprintf(stderr, "getBoxWithAspectRatio target = {%d, %d, %d, %d}\n", result->sx, result->sy, result->ex, result->ey);
	alignBoundary(result, &win_size);

	//fprintf(stderr, "alignBoundary target = {%d, %d, %d, %d}\n", result->sx, result->sy, result->ex, result->ey);
	alignDelta(speed, prev, result, &win_size);
	//fprintf(stderr, "alignDelta target = {%d, %d, %d, %d} <<<<<<\n", result->sx, result->sy, result->ex, result->ey);
}

static void ptz_setRoutineTarget(int step_target, const VIDEO_FTR_PTZ_ROUTINE_S *routine, const MPI_SIZE_S *size,
                                 MPI_RECT_POINT_S *target)
{
	MPI_POINT_S pos = routine->steps[step_target];
	MPI_RECT_S rect = {.x = 0, .y = 0, .width = size->width, .height = size->height };
	MPI_SIZE_S res = {.width = VIDEO_PTZ_MAX_RES_W, .height = VIDEO_PTZ_MAX_RES_H };

	rect.x = pos.x;
	rect.y = pos.y;
	getBox(&rect, target);
	alignBoundary(target, &res);
}

/**
 * @brief Check whether the routine meet the next step for PTZ Scan mode .
 * @param[in]  cur        current roi of PTZ.
 * @param[in]  target     next step of PTZ routine in scan mode.
 * @see VIDEO_FTR_setPtzResult().
 * @retval execution result.
 */
static int ptz_setRoutineCheck(const MPI_RECT_POINT_S *cur, const MPI_RECT_POINT_S *target)
{
	int reach_target = 0;
	if ((abs(cur->sx - target->sx) <= VIDEO_PTZ_ADJ_ROI / 2) &&
	    (abs(cur->ex - target->ex) <= VIDEO_PTZ_ADJ_ROI / 2) &&
	    (abs(cur->sy - target->sy) <= VIDEO_PTZ_ADJ_ROI / 2) &&
	    (abs(cur->ey - target->ey) <= VIDEO_PTZ_ADJ_ROI / 2)) {
		reach_target = 1;
	}
	return reach_target;
}

/**
 * @brief Set next routine step for PTZ scan mode.
 * @param[in]  ctx        structure of VIDEO_FTR_PTZ_CTX_S.
 * @param[in]  p          current roi of PTZ.
 * @param[in]  target     target roi of PTZ.
 * @see VIDEO_FTR_setPtzResult().
 * @retval execution result.
 */
static void ptz_setRoutineStep(VIDEO_FTR_PTZ_CTX_S *ctx, MPI_RECT_POINT_S *p, MPI_RECT_POINT_S *target)
{
	VIDEO_FTR_PTZ_PARAM_S *param = &ctx->param;
	VIDEO_FTR_PTZ_ROUTINE_S *routine = &ctx->routine;
	MPI_SIZE_S *roi_bd = &param->roi_bd;
	int step_target = (routine->steps_cur + 1) % routine->steps_num;
	ptz_setRoutineTarget(step_target, routine, roi_bd, target);
	if (ptz_setRoutineCheck(p, target)) {
		routine->steps_cur = step_target;
		step_target = (routine->steps_cur + 1) % routine->steps_num;
		ptz_setRoutineTarget(step_target, routine, roi_bd, target);
	}
}

/**
 * @brief Set static routine steps for scan mode.
 * @param[in]  routine        structure of VIDEO_FTR_PTZ_ROUTINE_S.
 * @see VIDEO_FTR_setPtzParam
 * @retval execution result.
 */
static void ptz_setRoutine(VIDEO_FTR_PTZ_ROUTINE_S *routine)
{
	routine->steps_num = 8;
	/* Snake-like routine */
	routine->steps[0] = PTZ_POS(16, 16); /* TOP LEFT */
	routine->steps[1] = PTZ_POS(16, 1008); /* TOP RIGHT */
	routine->steps[2] = PTZ_POS(512, 1008); /* MIDDLE RIGHT */
	routine->steps[3] = PTZ_POS(512, 16); /* MIDDLE LEFT */
	routine->steps[4] = PTZ_POS(1008, 16); /* BOTTOM LEFT */
	routine->steps[5] = PTZ_POS(1008, 1008); /* BOTTOM RIGHT */
	routine->steps[6] = PTZ_POS(512, 1008); /* MIDDLE RIGHT */
	routine->steps[7] = PTZ_POS(512, 16); /* MIDDLE LEFT */
	routine->steps_cur = routine->steps_num - 1;
}

/**
 * @brief Reset Pan-tilt-zoom context and parameters motion values.
 * @param[in]  ctx        structure of VIDEO_FTR_PTZ_CTX_S.
 * @see VIDEO_FTR_disablePtz(void)
 * @retval execution result.
 */
static void resetPtz(VIDEO_FTR_PTZ_CTX_S *ctx)
{
	int i, ret = 0;
	VIDEO_FTR_PTZ_PARAM_S *param = &ctx->param;
	MPI_RECT_POINT_S *pos = &ctx->pos;
	MPI_RECT_POINT_S *auto_pos = &ctx->auto_pos;
	ctx->enabled = 0;
	ctx->timer = 0;
	ctx->motion_sta = 0;
	ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_RUN;
	*pos = (MPI_RECT_POINT_S){ 0, 0, 1023, 1023 };
	*auto_pos = (MPI_RECT_POINT_S){ 0, 0, 1023, 1023 };
	param->mv = (MPI_MOTION_VEC_S){.x = VIDEO_PTZ_MV_XY_BYPASS, .y = VIDEO_PTZ_MV_XY_BYPASS };
	param->zoom_v = (MPI_MOTION_VEC_S){.x = 0, .y = 0 };
	param->zoom_lvl = 1 << VIDEO_FTR_PTZ_ZOOM_PREC;
	param->zoom_change = 1 << VIDEO_FTR_PTZ_ZOOM_PREC;
	param->mv_pos = (MPI_POINT_S){.x = VIDEO_PTZ_MV_POS_STEADY, .y = VIDEO_PTZ_MV_POS_STEADY };
	param->speed = (MPI_MOTION_VEC_S){.x = VIDEO_PTZ_DELTA_LIM, .y = VIDEO_PTZ_DELTA_LIM };
	for (i = 0; i < VIDEO_FTR_PTZ_MAX_SUBWIN_DISP_S_WIN_SIZE; i++) {
		if (ctx->roi_set[i]) {
			ctx->roi_set[i] = 0;
			if (i < param->win_num) {
				ret = MPI_DEV_setWindowRoi(param->win_id[i], &ctx->roi_default[i]);
				if (ret != MPI_SUCCESS) {
					/* FIXME: Error Handle */
					avftr_log_err("Cannot reset Window ROI for 0x%x", param->win_id[i].value);
				}
				param->win_id[i].win = 0;
				param->win_id[i].chn = 0;
			}
		}
	}
	//param->win_num = 0;
}


/**
 * @brief Set Pan-tilt-zoom result.
 * @param[in]  timestamp        timestamp in Jiffy Unit.
 * @see VIDEO_FTR_setPtzParam
 * @retval 0                                 success.
 * @retval -1                                unexpected fail.
 */
int VIDEO_FTR_setPtzResult(UINT32 timestamp)
{
	VIDEO_FTR_PTZ_CTX_S *ctx = &g_ptz_ctx;
	VIDEO_FTR_PTZ_PARAM_S *param = &ctx->param;
	MPI_RECT_POINT_S *p = &g_ptz_ctx.pos; /* Actual position of PTZ */
	MPI_RECT_S *roi_def = &ctx->roi_boundary;

	/* Output after param mode switching */
	MPI_RECT_POINT_S tmp_p; /* x[t](target) pos for tracking */
	MPI_RECT_POINT_S *target = NULL; /* x[t](target) pos for tracking ptr, usually set to tmp_p */

	MPI_SIZE_S tmp_roi_bd; /* Target resolution */
	MPI_SIZE_S *roi_bd = NULL; /* Target resolution for tracking usually set to tmp_roi_bd */

	int i;
	PTZ_MOTION_FLAG motion = { { 0 } };
	int delta_time = 0;
	int tmp_sta = 0; /* set state to VIDEO_FTR_PTZ_POS_DIR VIDEO_FTR_PTZ_AUTO_MANUAL_RUN */

	if (ctx->enabled == 0) {
		//printf("PTZ is not enabled");
		return 0;
	}

	pthread_mutex_lock(&ctx->lock);
	if (param->mv_pos.x == VIDEO_PTZ_MV_POS_PROBE) {
		ctx->timer = timestamp;
		param->mv_pos.x = VIDEO_PTZ_MV_POS_STEADY;
	}

	motion.mv = (param->mv.x != VIDEO_PTZ_MV_XY_BYPASS || param->mv.y != VIDEO_PTZ_MV_XY_BYPASS ||
		             param->zoom_v.x || param->zoom_v.y);
	motion.pos_zoom = (param->zoom_lvl > (1 << VIDEO_FTR_PTZ_ZOOM_PREC)) ||
		                  (param->zoom_change != (1 << VIDEO_FTR_PTZ_ZOOM_PREC));
	motion.pos_mv =
		        param->mv_pos.x != VIDEO_PTZ_MV_POS_STEADY || param->mv_pos.y != VIDEO_PTZ_MV_POS_STEADY;

	switch (param->mode) {
	case VIDEO_FTR_PTZ_MODE_AUTO:
		delta_time = (ctx->timer) ? (timestamp - ctx->timer) : 0;

		if (motion.flag) {
			if (ctx->auto_sta == VIDEO_FTR_PTZ_AUTO_MANUAL_RUN) {
				if (delta_time > VIDEO_FTR_PTZ_TIMEOUT) {
					/* FIXME: review force release, it might not sync to conf in tiger app */
					ptz_setParamMotionOff(param, ctx);
					ctx->timer = 0;
					ctx->motion_sta = 0;
					ctx->state = VIDEO_FTR_PTZ_POS_TRACK;
					ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_RUN;
					tmp_sta = 1;
				}
			} else {
				/* set roi_bd to cur aroi res */
				if (ctx->state == VIDEO_FTR_PTZ_POS_TRACK ||
				    ctx->auto_sta == VIDEO_FTR_PTZ_AUTO_MANUAL) {
					target = p;
				} else {
					target = &g_ptz_ctx.auto_pos;
				}
				param->roi_bd.width = target->ex - target->sx + 1;
				param->roi_bd.height = target->ey - target->sy + 1;
				ctx->timer = timestamp;
			}
			if (tmp_sta == 0) {
				ctx->motion_sta = motion.flag;
				ctx->state = VIDEO_FTR_PTZ_POS_DIR;
				ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_MANUAL_RUN;
			}
		} else if (ctx->auto_sta != VIDEO_FTR_PTZ_AUTO_RUN) {
			if (ctx->auto_sta == VIDEO_FTR_PTZ_AUTO_MANUAL_RUN) {
				ctx->timer = timestamp;
				ctx->motion_sta = motion.flag;
				ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_MANUAL;
			} else {
				if (delta_time > VIDEO_FTR_PTZ_MAX_WAIT_TIME) {
					ctx->timer = 0;
					ctx->motion_sta = motion.flag;
					ctx->state = VIDEO_FTR_PTZ_POS_TRACK;
					ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_RUN;
				}
			}
		}
		if (ctx->auto_sta == VIDEO_FTR_PTZ_AUTO_RUN) {
			/* auto mode flow */
			target = &g_ptz_ctx.auto_pos;
			tmp_roi_bd.width = target->ex - target->sx + 1;
			tmp_roi_bd.height = target->ey - target->sy + 1;
			roi_bd = &tmp_roi_bd;
			break;
		}
	case VIDEO_FTR_PTZ_MODE_MANUAL:
		if (motion.flag) {
			/* Add PTZ time out here for motion flag */
			if (ctx->timer == 0 || ctx->motion_apply) {
				ctx->timer = timestamp;
				if (ctx->motion_apply)
					ctx->motion_apply = 0;
			} else if (ctx->timer != 0 && delta_time > VIDEO_FTR_PTZ_TIMEOUT) {
				ctx->timer = 0;
				ptz_setParamMotionOff(param, ctx);
			}
		}
		if (motion.flag) {
#if 0 /* Track AROI when zoom ratio when there is no position input */
			if (motion.pos_zoom && !motion.pos_mv && ctx->auto_sta == VIDEO_FTR_PTZ_AUTO_MANUAL_RUN) {
				getBoxFromBox(&g_ptz_ctx.auto_pos, &ctx->roi_bd_target, &tmp_p);
			} else {
				getBoxSize(&ctx->mv_pos, &ctx->roi_bd_target, &tmp_p);
			}
#else /* Always output current position with zoom input */
			getBoxSize(&ctx->mv_pos, &ctx->roi_bd_target, &tmp_p);
#endif
			roi_bd = &ctx->roi_bd_target;
		} else {
			tmp_p = *p;
		}
		target = &tmp_p;
		break;
	case VIDEO_FTR_PTZ_MODE_SCAN:
		ptz_setRoutineStep(ctx, p, &tmp_p);
		target = &tmp_p;
		roi_bd = &param->roi_bd;
		break;
	default:
		printf("Unkown ptz mode\n");
		break;
	}
	MPI_RECT_POINT_S tmp_res;
	if (ctx->state == VIDEO_FTR_PTZ_POS_TRACK || param->mode != VIDEO_FTR_PTZ_MODE_AUTO ||
	    ctx->auto_sta != VIDEO_FTR_PTZ_AUTO_RUN) {
		if (motion.flag || (param->mode == VIDEO_FTR_PTZ_MODE_AUTO && ctx->state == VIDEO_FTR_PTZ_POS_TRACK) ||
		    (param->mode == VIDEO_FTR_PTZ_MODE_SCAN)) {
			trackTargetPos(&param->speed, p, target, &tmp_res, roi_bd);
		} else {
			tmp_res = *target;
		}
		if ((abs(tmp_res.sx - target->sx) <= VIDEO_PTZ_ADJ_ROI / 2) &&
		    (abs(tmp_res.ex - target->ex) <= VIDEO_PTZ_ADJ_ROI / 2) &&
		    (abs(tmp_res.sy - target->sy) <= VIDEO_PTZ_ADJ_ROI / 2) &&
		    (abs(tmp_res.ey - target->ey) <= VIDEO_PTZ_ADJ_ROI / 2)) {
			ctx->state = VIDEO_FTR_PTZ_POS_DIR;
			if (motion.pos && param->mode == VIDEO_FTR_PTZ_MODE_AUTO) {
				ptz_setParamMotionOff(param, ctx);
				ctx->timer = timestamp;
				ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_MANUAL;
			}
			//printf("%s %d motion:%d %d p(mv:%d-%d zmv:%d-%d lvl:%d pos:%d-%d ch:%d)\n", __func__, __LINE__, motion.mv, motion.pos,
			//param->mv.x, param->mv.y, param->zoom_v.x,param->zoom_v.y,param->zoom_lvl, param->mv_pos.x,
			//param->mv_pos.y, param->zoom_change);
		}
	} else {
		tmp_res.sx = target->sx;
		tmp_res.ex = target->ex;
		tmp_res.sy = target->sy;
		tmp_res.ey = target->ey;
	}

	// If new pos is the same with the prev one, no need to update
	if (memcmp(&tmp_res, p, sizeof(MPI_RECT_POINT_S)) == 0) {
		pthread_mutex_unlock(&ctx->lock);
		return 0;
	}

	INT32 ret;
	MPI_RECT_S roi = { 0 };
	MPI_RECT_S roi_corr = { 0 };
	*p = tmp_res;

	roi.x = p->sx;
	roi.y = p->sy;
	roi.width = p->ex - p->sx + 1;
	roi.height = p->ey - p->sy + 1;
#if 0
	printf("Set POS = {%d, %d, %d, %d}\n", p->sx, p->sy, p->ex, p->ey);
	//printf("WIN roi_x = %d, roi_y = %d, roi_width = %d, roi_height = %d\n", roi.x, roi.y, roi.width, roi.height);
	//printf("width=%d, height=%d\n", cb_data->res.width, cb_data->res.height);
#endif
	for (i = 0; i < param->win_num; i++) {
		ptz_roiConvertBoundary(&roi, roi_def, &roi_corr);
		ret = MPI_DEV_setWindowRoi(param->win_id[i], &roi_corr);
		if (ret != MPI_SUCCESS) {
			pthread_mutex_unlock(&ctx->lock);
			printf("PTZ set win roi failed\n");
			return -1;
		}
	}
	pthread_mutex_unlock(&ctx->lock);
	return 0;
}

/**
 * @brief Get the status of pan-tilt-zoom video feature.
 * @param[in]            none.
 * @see VIDEO_FTR_setPtzParam()
 * @retval 0             pan-tilt-zoom video feature is disabled.
 * @retval 1             pan-tilt-zoom video feature is enabled.
 */
int VIDEO_FTR_getPtzStat(void)
{
	//FIXME
	//need to check win?
	return g_ptz_ctx.enabled;
}

/**
 * @brief Update the AROI position for pan-tilt-zoom video feature.
 * @param[in] roi        structure of MPI_RECT_POINT_S.
 * @see VIDEO_FTR_setPtzParam()
 * @retval 0             success.
 */
int VIDEO_FTR_updateAutoPtz(MPI_RECT_POINT_S *roi)
{
	g_ptz_ctx.auto_pos = (MPI_RECT_POINT_S){.sx = roi->sx, .ex = roi->ex, .sy = roi->sy, .ey = roi->ey };
	//printf("%s: auto_pos = {%d, %d, %d, %d}\n", __func__, g_ptz_ctx.auto_pos.sx, g_ptz_ctx.auto_pos.sy,
	//                                     g_ptz_ctx.auto_pos.ex, g_ptz_ctx.auto_pos.ey);
	return 0;
}


int ptz_probePos(const MPI_RECT_S *roi, const MPI_RECT_S *roi_boundary)
{
	int ret = 0;
	MPI_RECT_S roi_corr;
	VFTR_POINT_S pt;
	VFTR_POINT_S pt_corr;
	ptz_decodeRoi(roi, roi_boundary, &roi_corr);
	pt.x = (roi_corr.x + (roi_corr.x + roi_corr.width) + 1) / 2;
	pt.y = (roi_corr.y + (roi_corr.y + roi_corr.height) + 1) / 2;
	ptz_decodeRoiPt(&pt, &roi_corr, &pt_corr);
	ret = VIDEO_PTZ_RET_TYPE_COOR << VIDEO_PTZ_RET_TYPE_BS;
	ret |= pt_corr.x;
	ret |= (pt_corr.y) << VIDEO_PTZ_RET_Y_BS;
	return ret;
}

void ptz_StopMotion(VIDEO_FTR_PTZ_CTX_S *ctx)
{
	VIDEO_FTR_PTZ_PARAM_S *param = &ctx->param;
	if (param->mode == VIDEO_FTR_PTZ_MODE_AUTO && ctx->auto_sta == VIDEO_FTR_PTZ_AUTO_RUN) {
		ptz_setParamMotionOff(param, ctx);
		param->mv_pos.x = VIDEO_PTZ_MV_POS_PROBE; // make motion flag up
		ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_MANUAL; // goto waiting mode
		ctx->state = VIDEO_FTR_PTZ_POS_DIR;
	}
}

int g_ptz_init = 1;

/**
 * @brief Set Pan-tile-zoom video feature parameters
 * @param[in] param        Video feature pan-tilt-zoom parameters.
 * @see VIDEO_FTR_getPtzParam()
 * @retval 0             success.
 * @retval ((VIDEO_PTZ_RET_TYPE_COOR << VIDEO_PTZ_RET_TYPE_BS) | coordinate xy)       success.
 * @retval ((VIDEO_PTZ_RET_TYPE_ZLVL << VIDEO_PTZ_RET_TYPE_BS) | zoon level target)   success.
 * @retval -1            unexpected fail.
 * @retval VIDEO_FTR_PTZ_REACH_MIN_ZOOM_LVL  Reach min zoom level, fail to set param.
 * @retval VIDEO_FTR_PTZ_REACH_MAX_ZOOM_LVL  Reach max zoom level, fail to set param.
 * @retval ((VIDEO_PTZ_RET_TYPE_ERRON << VIDEO_PTZ_RET_TYPE_BS) | VIDEO_FTR_PTZ_REACH_MIN_ZOOM_LVL)  Reach min zoom level, fail to set param.
 * @retval ((VIDEO_PTZ_RET_TYPE_ERRON << VIDEO_PTZ_RET_TYPE_BS) | VIDEO_FTR_PTZ_REACH_MAX_ZOOM_LVL)  Reach max zoom level, fail to set param.
 */
int VIDEO_FTR_setPtzParam(const VIDEO_FTR_PTZ_PARAM_S *param)
{
	PTZ_API_DEBUG("enter%s\n", "");

	VIDEO_FTR_PTZ_CTX_S *ctx = &g_ptz_ctx;
	VIDEO_FTR_PTZ_PARAM_S *dest = &ctx->param;
	VIDEO_FTR_PTZ_ROUTINE_S *routine = &ctx->routine;
	VIDEO_FTR_PTZ_PARAM_S oparam = *param;
	MPI_RECT_S roi = { 0 };
	int i, ret = 0;

	ret = ptz_checkParamAndConvert(&oparam);
	if (ret != MPI_SUCCESS) {
		return -1;
	}

	if (oparam.win_num != 0) {
		ret = MPI_DEV_getWindowRoi(oparam.win_id[0], &roi);
		if (ret != MPI_SUCCESS) {
			return -1;
		}
	}

	pthread_mutex_lock(&ctx->lock);
	if (((oparam.win_num != dest->win_num) && (oparam.win_num != 0)) ||
	    (oparam.win_id[0].value != dest->win_id[0].value)) {
		MPI_RECT_POINT_S *pos = &ctx->pos;

		ptz_resetSubWinROI(ctx, dest, 0);

		/* Set default roi if win num is different*/
		pos->sx = roi.x;
		pos->sy = roi.y;
		pos->ex = roi.x + roi.width - 1;
		pos->ey = roi.y + roi.height - 1;

		/* FIXME: Find another way to store ROI boundary instead of WIN(0,0,0) */
		ret = ptz_setDefaultWinBoundary(ctx);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Cannot Set PTZ window default boundary");
			pthread_mutex_unlock(&ctx->lock);
			return -1;
		}

		for (i = 0; i < oparam.win_num; i++) {
			ctx->roi_set[i] = 1;
			ctx->roi_default[i] = roi;
		}
	}

	if ((dest->mode != oparam.mode) && (oparam.mode == VIDEO_FTR_PTZ_MODE_AUTO)) {
		ctx->state = VIDEO_FTR_PTZ_POS_TRACK;
	}
	if ((dest->mode != oparam.mode) && (oparam.mode == VIDEO_FTR_PTZ_MODE_SCAN)) {
		routine->steps_cur = routine->steps_num - 1;
	}

	if (param->mv_pos.x == VIDEO_PTZ_MV_POS_PROBE || param->mv_pos.y == VIDEO_PTZ_MV_POS_PROBE) {
		ptz_StopMotion(ctx);
		ret = ptz_probePos(&roi, &ctx->roi_boundary);
		if (g_ptz_init == 1) { // always set param when init ptz
			g_ptz_init = 0;
			goto set_ptz_param;
		}
		pthread_mutex_unlock(&ctx->lock);
		return ret;
	}

set_ptz_param:
	ret = ptz_setParamToCtx(&oparam, &roi, ctx);
	if (ret != 0) {
		pthread_mutex_unlock(&ctx->lock);
		return ret;
	}

#if 0
	printf("[PTZ] Param: zoomlvl:%d zoomCh:%d winNum:%d [%d,%d] mode:%d mv:[%d,%d] zm_v:[%d,%d] spd:[%d,%d] pos:[%d,%d] roibd:[%d,%d]\n",
	oparam.zoom_lvl, oparam.zoom_change, oparam.win_num, oparam.win_id[0].win, oparam.win_id[1].win, oparam.mode,
	oparam.mv.x, oparam.mv.x, oparam.zoom_v.x, oparam.zoom_v.y, oparam.speed.x, oparam.speed.y,
	oparam.mv_pos.x, oparam.mv_pos.y, oparam.roi_bd.width, oparam.roi_bd.height);

	printf("[PTZ] Ctx: mv_pos:[%d,%d] roi_bd_target:[%d,%d] curroi:[%d,%d,%d,%d]\n", ctx->mv_pos.x, ctx->mv_pos.y,
	ctx->roi_bd_target.width, ctx->roi_bd_target.height, roi.x, roi.y, roi.width, roi.height);
#endif

	ctx->motion_apply = ptz_checkMotionApply(&oparam);

	memcpy(dest, &oparam, sizeof(VIDEO_FTR_PTZ_PARAM_S));
	pthread_mutex_unlock(&ctx->lock);
	PTZ_API_DEBUG("exit%s\n", "");
	return ctx->retval;
}

/**
 * @brief Get Pan-tile-zoom video feature parameters.
 * @param[in] param      input PTZ parameter.
 * @see VIDEO_FTR_setPtzParam()
 * @retval 0             success.
 */
int VIDEO_FTR_getPtzParam(VIDEO_FTR_PTZ_PARAM_S *param)
{
	VIDEO_FTR_PTZ_CTX_S *ctx = &g_ptz_ctx;
	VIDEO_FTR_PTZ_PARAM_S *src = &ctx->param;
	pthread_mutex_lock(&ctx->lock);
	memcpy(param, src, sizeof(VIDEO_FTR_PTZ_PARAM_S));
	pthread_mutex_unlock(&ctx->lock);
	return 0;
}

/**
 * @brief Enable Pan-tile-zoom video feature
 * @see VIDEO_FTR_disablePtz()
 * @retval 0             success.
 * @retval -1            unexpected fail.
 */
int VIDEO_FTR_enablePtz(void)
{
	VIDEO_FTR_PTZ_CTX_S *ctx = &g_ptz_ctx;
	MPI_RECT_POINT_S *pos = &ctx->pos;
	MPI_RECT_POINT_S *auto_pos = &ctx->auto_pos;
	VIDEO_FTR_PTZ_PARAM_S *param = &ctx->param;
	VIDEO_FTR_PTZ_ROUTINE_S *routine = &ctx->routine;
	MPI_RECT_S roi;
	int i, ret = 0;

	if (ctx->enabled) {
		avftr_log_warn("PTZ has already enabled!");
		return 0;
	}

	ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	pthread_mutex_lock(&ctx->lock);
	if (param->win_num > 0) {
		/* FIXME: Find another way to store ROI boundary instead of WIN(0,0,0) */
		ret = ptz_setDefaultWinBoundary(ctx);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Cannot Set PTZ window default boundary");
			goto error;
		};
		for (i = 0; i < param->win_num; i++) {
			/* Only get Deafult Window ROI once when Enable ROI */
			int ret = MPI_DEV_getWindowRoi(param->win_id[i], &roi);
			if (ret != MPI_SUCCESS) {
				avftr_log_err("Cannot get window roi on video window 0x%x", param->win_id[i].value);
				goto error;
			}
			ctx->roi_set[i] = 1;
			ctx->roi_default[i] = roi;
			if (i == 0) {
				pos->sx = roi.x;
				pos->sy = roi.y;
				pos->ex = roi.x + roi.width - 1;
				pos->ey = roi.y + roi.height - 1;

				auto_pos->sx = roi.x;
				auto_pos->sy = roi.y;
				auto_pos->ex = roi.x + roi.width - 1;
				auto_pos->ey = roi.y + roi.height - 1;
			}
		}
	}

	ptz_setRoutine(routine);
	ctx->enabled = 1;
	ctx->state = VIDEO_FTR_PTZ_POS_DIR;
	ctx->auto_sta = VIDEO_FTR_PTZ_AUTO_RUN;
	param->mv = (MPI_MOTION_VEC_S){.x = 0, .y = 0 };
	param->zoom_v = (MPI_MOTION_VEC_S){.x = 0, .y = 0 };
	pthread_mutex_unlock(&ctx->lock);
	return 0;

error:
	resetPtz(ctx);
	pthread_mutex_unlock(&ctx->lock);
	return -1;
}

/**
 * @brief Disable Pan-tile-zoom video feature
 * @see VIDEO_FTR_enablePtz()
 * @retval 0             success.
 */
int VIDEO_FTR_disablePtz(void)
{
	VIDEO_FTR_PTZ_CTX_S *ctx = &g_ptz_ctx;
	pthread_mutex_lock(&ctx->lock);
	resetPtz(ctx);
	pthread_mutex_unlock(&ctx->lock);
	return 0;
}
