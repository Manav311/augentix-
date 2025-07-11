#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "vftr.h"
#include "vftr_fld.h"
#include "avftr_conn.h"

#include "log.h"
#include "fld_demo.h"

#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))

static int g_interval = 5;
extern int g_fld_running;

#ifdef CONFIG_APP_FLD_SUPPORT_SEI
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

static int findFldCtx(const MPI_WIN idx, const AVFTR_FLD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_FLD_MAX_SUPPORT_NUM; i++) {
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

static int clearFldCtx(MPI_WIN idx, AVFTR_FLD_CTX_S *ctx)
{
	int ctx_idx = findFldCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The FLD ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(AVFTR_FLD_CTX_S));
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
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1) {
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
			log_err("Failed to get object list. err: %d", ret);
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
#endif

int detectFldObject(MPI_WIN win_idx, MPI_SIZE_S *res, VFTR_FLD_PARAM_S *fld_param)
{
	UINT32 timestamp = 0;
	const INT32 timeout = 0;
	INT32 ret = 0;
	int frame_interval_cnt = 0;
	g_fld_running = 1;
	VFTR_FLD_INSTANCE_S *instance;
	MPI_IVA_OBJ_LIST_S *obj_list;

#ifdef CONFIG_APP_FLD_SUPPORT_SEI
	/* init OD ctx */
	int empty_idx, set_idx;
	set_idx = findOdCtx(win_idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_err("OD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -EINVAL;
	}
	if (empty_idx < 0) {
		log_err("Failed to create OD on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	VIDEO_OD_CTX_S *od_ctx = &vftr_res_shm->od_ctx[empty_idx];
	od_ctx->en = 1;
	od_ctx->en_shake_det = 0;
	od_ctx->en_crop_outside_obj = 0;
	od_ctx->idx = win_idx;
	od_ctx->cb = NULL;
	od_ctx->bdry.sx = 0;
	od_ctx->bdry.sy = 0;
	od_ctx->bdry.ex = res->width - 1;
	od_ctx->bdry.ey = res->height - 1;

	/* init FLD ctx */
	set_idx = findFldCtx(win_idx, vftr_res_shm->fld_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("FLD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create FLD instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	AVFTR_FLD_CTX_S *fld_ctx = &vftr_res_shm->fld_ctx[empty_idx];
	fld_ctx->en = 1;
	fld_ctx->reg = 1;
	fld_ctx->idx = win_idx;
	fld_ctx->cb = NULL;

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
#endif
	VFTR_init(NULL);

	/* create FLD instance */
	instance = VFTR_FLD_newInstance();
	if (!instance) {
		log_err("Failed to create FLD instance");
		return -EINVAL;
	}

	/* set parameter to fld */
	ret = VFTR_FLD_setParam(instance, fld_param);
	if (ret) {
		log_err("Failed to set FLD parameters. err: %d", ret);
		return -EIO;
	}

	while (g_fld_running) {
		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(win_idx, &timestamp, timeout);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", win_idx.dev, win_idx.chn, win_idx.win, ret);
			continue;
		}

		/* do motion detection once every g_interval */
		if (frame_interval_cnt < g_interval) {
			++frame_interval_cnt;
			continue;
		} else {
			frame_interval_cnt = 0;
		}

#ifdef CONFIG_APP_FLD_SUPPORT_SEI
		/* get buffer index, fill the result to shared memory
		 * it will be read and embedded as SEI data through RTSP streaming
		 * */
		int buf_idx = updateBufferIdx(buf_info, timestamp);

		VIDEO_FTR_OBJ_LIST_S *vftr_obj_list = &od_ctx->ol[buf_idx];

		ret = getObjList(win_idx, timestamp, vftr_obj_list);
		if (ret != MPI_SUCCESS) {
			continue;
		}

		obj_list = &vftr_obj_list->basic_list;
#else
		MPI_IVA_OBJ_LIST_S obj_instance;

		ret = MPI_IVA_getBitStreamObjList(win_idx, timestamp, &obj_instance);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to get object list. err: %d", ret);
			continue;
		}

		obj_list = &obj_instance;

#endif

		VFTR_FLD_STATUS_S *fld_res;
#ifdef CONFIG_APP_FLD_SUPPORT_SEI
		fld_res = &fld_ctx->fld_res[buf_idx].fld_status;
#else
		VFTR_FLD_STATUS_S status;
		fld_res = &status;
#endif
		// Fall detection
		ret = VFTR_FLD_detect(instance, obj_list, fld_res);
		if (ret) {
			log_err("Failed to detect FLD. err: %d", ret);
			continue;
		}

		/* mark buffer ready for display */
#ifdef CONFIG_APP_FLD_SUPPORT_SEI
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	/* delete FLD instance */
	VFTR_FLD_deleteInstance(&instance);

	VFTR_exit();

	clearOdCtx(win_idx, vftr_res_shm->od_ctx);
	clearFldCtx(win_idx, vftr_res_shm->fld_ctx);
	clearVftrBufCtx(win_idx, vftr_res_shm->buf_info);

	return 0;
}
