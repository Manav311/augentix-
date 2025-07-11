#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "mpi_osd.h"
#include "aroi_demo.h"
#include "vftr_aroi.h"
#include "video_ptz.h"

#include "avftr_conn.h"
#include "log.h"

#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

extern int g_aroi_running;
extern int g_detect_interval;

#ifdef CONFIG_APP_AROI_SUPPORT_SEI
extern AVFTR_CTX_S *avftr_res_shm;
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

static int findOdCtx(MPI_WIN idx, VIDEO_OD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_OD_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].en) {
			find_idx = i;
		} else if (emp_idx == -1 && !(ctx[i].en || ctx[i].en_implicit)) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int clearOdCtx(MPI_WIN idx, VIDEO_OD_CTX_S *ctx)
{
	int ctx_idx = findOdCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The OD ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(VIDEO_OD_CTX_S));
	return 0;
}

static int findAroiCtx(const MPI_WIN idx, const AVFTR_AROI_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_AROI_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].reg) {
			find_idx = i;
		} else if (emp_idx == -1 && !ctx[i].reg) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int clearAroiCtx(MPI_WIN idx, AVFTR_AROI_CTX_S *ctx)
{
	int ctx_idx = findAroiCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The AROI ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(AVFTR_AROI_CTX_S));
	return 0;
}

static int findVftrBufCtx(MPI_WIN idx, const AVFTR_VIDEO_BUF_INFO_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_VIDEO_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].en) {
			find_idx = i;
		} else if (emp_idx == -1 && !ctx[i].en) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int clearVftrBufCtx(MPI_WIN idx, AVFTR_VIDEO_BUF_INFO_S *info)
{
	int info_idx = findVftrBufCtx(idx, info, NULL);
	if (info_idx < 0) {
		log_err("The VFTR buffer ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&info[info_idx], 0, sizeof(AVFTR_VIDEO_BUF_INFO_S));
	return 0;
}

static int updateBufferIdx(AVFTR_VIDEO_BUF_INFO_S *buf_info, UINT32 timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;
	return buf_info->buf_cur_idx;
}

int getMpiSize(const MPI_WIN idx, MPI_SIZE_S *res)
{
	INT32 ret;
	INT32 i;
	UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	UINT32 chn_idx = MPI_GET_VIDEO_CHN(idx);
	UINT32 win_idx = MPI_GET_VIDEO_WIN(idx);
	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);

	ret = MPI_DEV_queryChnState(chn, &chn_stat);
	if (ret != 0) {
		fprintf(stderr, "Query channel state on channel %d on device %d failed", chn_idx, dev_idx);
		return ret;
	}

	if (!MPI_STATE_IS_ADDED(chn_stat.status)) {
		fprintf(stderr, "Channel %d on device %d is not added", chn_idx, dev_idx);
		return ENODEV;
	}

	MPI_CHN_LAYOUT_S layout_attr;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != 0) {
		fprintf(stderr, "Get video channel %d layout attributes failed.", chn_idx);
		return ret;
	}

	/* FIXME: check window state */
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			res->width = layout_attr.window[i].width;
			res->height = layout_attr.window[i].height;
			break;
		}
	}
	if (i == layout_attr.window_num) {
		fprintf(stderr, "Invalid video window index %d from video channel %d", win_idx, chn_idx);
		return EINVAL;
	}

	return 0;
}

/**
 * @brief fill object context for display
 */
int getObjList(MPI_WIN idx, UINT32 timestamp, VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];
	vftr_od_ctx->en = 1;

	if (vftr_od_ctx->en) {
		int ret = 0;
		MPI_RECT_POINT_S *bd = &vftr_od_ctx->bdry;
		MPI_RECT_POINT_S *obj;
		MPI_RECT_POINT_S *final_obj;
		int obj_cnt = 0;

		ret = MPI_IVA_getBitStreamObjList(idx, timestamp, &obj_list->basic_list);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "MPI_IVA_getBitStreamObjList ret = %d\n", ret);
			goto err;
		}

		for (int i = 0; i < obj_list->basic_list.obj_num; i++) {
			/* Initialize object attr */
			obj_list->obj_attr[obj_cnt].cat[0] = '\0';
			obj_list->obj_attr[obj_cnt].shaking = 0;

			/* Limit OL boundary */
			obj = &obj_list->basic_list.obj[i].rect;
			final_obj = &obj_list->basic_list.obj[obj_cnt].rect;

			/* NOTICE: the following code remove the object that out of boundary
			 *         or crop the object to fit the output image */
			if (bd->sx != -1) {
				if (obj->ex < bd->sx) {
					continue;
				}
				final_obj->sx = obj->sx > bd->sx ? obj->sx : bd->sx;
			} else {
				final_obj->sx = obj->sx;
			}

			if (bd->sy != -1) {
				if (obj->ey < bd->sy) {
					continue;
				}
				final_obj->sy = obj->sy > bd->sy ? obj->sy : bd->sy;
			} else {
				final_obj->sy = obj->sy;
			}

			if (bd->ex != -1) {
				if (obj->sx > bd->ex) {
					continue;
				}
				final_obj->ex = obj->ex > bd->ex ? bd->ex : obj->ex;
			} else {
				final_obj->ex = obj->ex;
			}

			if (bd->ey != -1) {
				if (obj->sy > bd->ey) {
					continue;
				}
				final_obj->ey = obj->ey > bd->ey ? bd->ey : obj->ey;
			} else {
				final_obj->ey = obj->ey;
			}

			obj_list->basic_list.obj[obj_cnt].id = obj_list->basic_list.obj[i].id;
			obj_list->basic_list.obj[obj_cnt].life = obj_list->basic_list.obj[i].life;
			obj_list->basic_list.obj[obj_cnt].mv = obj_list->basic_list.obj[i].mv;

			/* increase the index */
			obj_cnt++;
		}

		obj_list->basic_list.obj_num = obj_cnt;
	}

	return MPI_SUCCESS;
err:
	return MPI_FAILURE;
}

#ifdef CONFIG_APP_PTZ_SUPPORT
static void ptz_roiConvertBoundary(MPI_RECT_POINT_S *roi, MPI_RECT_S *roi_corrd)
{
	/*transfer aroi point to rect*/
	roi_corrd->x = roi->sx;
	roi_corrd->y = roi->sy;
	roi_corrd->width = abs(roi->ex - roi->sx);
	roi_corrd->height = abs(roi->ey - roi->sy);
}

#endif /* CONFIG_APP_PTZ_SUPPORT */
#endif

int detectAroi(MPI_RECT_S *pWinRect __attribute__((unused)) /*in pixel*/,
               MPI_RECT_S *pWinRectRatio __attribute__((unused)) /*in [0-1024]*/, MPI_WIN win_idx, MPI_SIZE_S *res,
               VFTR_AROI_PARAM_S *aroi_attr, VIDEO_FTR_PTZ_PARAM_S *ptz_param __attribute__((unused)))
{
	UINT32 timestamp = 0;
	INT32 timeout = 0;
	INT32 ret = 0;
	int frame_interval_cnt = 0;
	VFTR_AROI_INSTANCE_S *aroi;
	MPI_IVA_OBJ_LIST_S *obj_list;

	/* bd = boundary, set boundary to filter object list */
#ifdef CONFIG_APP_AROI_SUPPORT_SEI
	/* init OD ctx */
	int empty_idx, set_idx;
	VIDEO_OD_CTX_S *od_ctx;
	set_idx = findOdCtx(win_idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("OD is already registered on win(%u, %u, %u) set_idx:%d.", win_idx.dev, win_idx.chn,
		         win_idx.win, set_idx);
		od_ctx = &vftr_res_shm->od_ctx[set_idx];
	} else {
		if (empty_idx < 0) {
			log_err("Failed to create OD on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
			return -ENOMEM;
		}

		od_ctx = &vftr_res_shm->od_ctx[empty_idx];
		od_ctx->en = 1;
		od_ctx->en_shake_det = 0;
		od_ctx->en_crop_outside_obj = 0;
		od_ctx->idx = win_idx;
		od_ctx->cb = NULL;
		od_ctx->bdry.sx = 0;
		od_ctx->bdry.sy = 0;
		od_ctx->bdry.ex = res->width - 1;
		od_ctx->bdry.ey = res->height - 1;
	}

	/* init AROI ctx */
	AVFTR_AROI_CTX_S *aroi_ctx;
	set_idx = findAroiCtx(win_idx, vftr_res_shm->aroi_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("AROI is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		aroi_ctx = &vftr_res_shm->aroi_ctx[set_idx];
		return 0;
	} else {
		if (empty_idx < 0) {
			log_err("Failed to create AROI instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn,
			        win_idx.win);
			return -ENOMEM;
		}

		aroi_ctx = &vftr_res_shm->aroi_ctx[empty_idx];
		aroi_ctx->en = 1;
		aroi_ctx->reg = 1;
		aroi_ctx->idx = win_idx;
		aroi_ctx->cb = NULL;
	}

	// init vftr buffer ctx
	int buf_info_idx;
	set_idx = findVftrBufCtx(win_idx, vftr_res_shm->buf_info, &empty_idx);
	if (set_idx >= 0) {
		buf_info_idx = set_idx;
	} else if (empty_idx >= 0) {
		buf_info_idx = empty_idx;
	} else {
		log_err("Failed to register vftr buffer ctx on win(%u, %u, %u).", win_idx.dev, win_idx.chn,
		        win_idx.win);
		return -ENOMEM;
	}

	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	buf_info->idx = win_idx;
	buf_info->en = 1;
#endif

	/* create AROI instance */
	aroi = VFTR_AROI_newInstance();
	if (!aroi) {
		fprintf(stderr, "Failed to create AROI instance\n");
		return -EINVAL;
	}

	/* set parameter to aroi */
	MPI_SIZE_S win_res = { .width = pWinRect->width, .height = pWinRect->height };
	getMpiSize(win_idx, &win_res);
	ret = VFTR_AROI_setParam(aroi, &win_res, aroi_attr);
	if (ret) {
		fprintf(stderr, "Failed to set AROI parameters.\n");
		return -EINVAL;
	}

	while (g_aroi_running) {
		/* wait one video frame processed */
		if (MPI_DEV_waitWin(win_idx, &timestamp, timeout) != MPI_SUCCESS) {
			fprintf(stderr, "Wait ISP statistics fail for win:0x%x\n", win_idx.value);
			continue;
		}

		/* do motion detection once every g_interval */
		if (frame_interval_cnt < g_detect_interval) {
			++frame_interval_cnt;
			continue;
		} else {
			frame_interval_cnt = 0;
		}

#ifdef CONFIG_APP_AROI_SUPPORT_SEI
		int buf_idx = updateBufferIdx(buf_info, timestamp);

		VIDEO_FTR_OBJ_LIST_S *vftr_obj_list = &od_ctx->ol[buf_idx];

		/**
		 * get object list from MPI
		 */
		ret = getObjList(win_idx, timestamp, vftr_obj_list);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to getBitStreamObjList.[LINE]: %d\n", __LINE__);
			continue;
		}

		obj_list = &vftr_obj_list->basic_list;
#else
		MPI_IVA_OBJ_LIST_S obj_instance;

		ret = MPI_IVA_getBitStreamObjList(win_idx, timestamp, &obj_instance);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "MPI_IVA_getBitStreamObjList ret = %d\n", ret);
			continue;
		}

		obj_list = &obj_instance;
		/* DBG("still alive %u\n", timestamp); */
#endif /* CONFIG_APP_AROI_SUPPORT_SEI */

		VFTR_AROI_STATUS_S *aroi_res;
#ifdef CONFIG_APP_AROI_SUPPORT_SEI
		aroi_res = &aroi_ctx->aroi_res[buf_idx];
#else
		VFTR_AROI_STATUS_S status;
		aroi_res = &status;
#endif /* CONFIG_APP_AROI_SUPPORT_SEI */

		/**
		 * detect aroi detection
		 */
		ret = VFTR_AROI_detectRoi(aroi, obj_list, aroi_res);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to AROI_detectMotion.[LINE]: %d\n", __LINE__);
			continue;
		}

#ifdef CONFIG_APP_PTZ_SUPPORT
		MPI_WIN apply_idx = MPI_VIDEO_WIN(0, ptz_param->win_id->chn, ptz_param->win_id->win);
		MPI_RECT_POINT_S p = (MPI_RECT_POINT_S)aroi_res->roi;
		MPI_RECT_POINT_S roi_tmp = { 0 };
		MPI_RECT_S roi_corrd = { 0 };

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

		roi_tmp.sx = MAX(p.sx * 1024 / MAX(win_res.width, 1), 0);
		roi_tmp.sy = MAX(p.sy * 1024 / MAX(win_res.height, 1), 0);
		roi_tmp.ex = MIN(MAX(p.ex * 1024 / MAX(win_res.width, 1), 0), 1024);
		roi_tmp.ey = MIN(MAX(p.ey * 1024 / MAX(win_res.height, 1), 0), 1024);

		ptz_roiConvertBoundary(&roi_tmp, &roi_corrd);

		for (int i = 0; i < ptz_param->win_num; i++) {
			ret = MPI_DEV_setWindowRoi(apply_idx, &roi_corrd);
			if (ret != MPI_SUCCESS) {
				fprintf(stderr, "Failed to set AROI_Window.[LINE]: %d\n", __LINE__);
				return -1;
			}
		}
#endif /* CONFIG_APP_PTZ_SUPPORT */

		/*
		 * write result to shared memory so that RTSP server can read
		 * */
		/* mark buffer ready for display */
#ifdef CONFIG_APP_AROI_SUPPORT_SEI
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	/* delete AROI instance */
	VFTR_AROI_deleteInstance(&aroi);

	clearOdCtx(win_idx, vftr_res_shm->od_ctx);
	clearAroiCtx(win_idx, vftr_res_shm->aroi_ctx);
	clearVftrBufCtx(win_idx, vftr_res_shm->buf_info);

	return 0;
}
