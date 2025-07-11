#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "vftr.h"
#include "vftr_ef.h"
#include "avftr_conn.h"
#include "vftr_shd.h"

#include "ef_demo.h"
#include "log.h"

#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

static int g_interval = 5;
extern int g_ef_running;

#ifdef CONFIG_APP_EF_SUPPORT_SEI
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

static int findEfCtx(const MPI_WIN idx, const AVFTR_EF_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_EF_MAX_SUPPORT_NUM; i++) {
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

static int clearEfCtx(MPI_WIN idx, AVFTR_EF_CTX_S *ctx)
{
	int ctx_idx = findEfCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The EF ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(AVFTR_EF_CTX_S));
	return 0;
}

static inline int findShdCtx(const MPI_WIN idx, const AVFTR_SHD_CTX_S *ctx, int *empty)
{
	int find_idx = -1;
	int emp_idx = -1;
	int i = 0;

	if (empty == NULL) {
		emp_idx = -2;
	}

	for (i = 0; i < AVFTR_SHD_MAX_SUPPORT_NUM; ++i) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].reg) {
			find_idx = i;
		}
		if (emp_idx == -1 && !ctx[i].reg) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int clearShdCtx(MPI_WIN idx, AVFTR_SHD_CTX_S *ctx)
{
	int ctx_idx = findShdCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The SHD ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(AVFTR_SHD_CTX_S));
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
int getObjList(MPI_WIN idx, UINT32 timestamp, VIDEO_FTR_OBJ_LIST_S *obj_list, VFTR_SHD_INSTANCE_S *shd)
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

#ifdef CONFIG_APP_SHD_SUPPORT
		VFTR_SHD_STATUS_S status = { .shake_rect = { { 0 } }, .shaking = { 0 } };
		VFTR_SHD_detectShake(shd, &obj_list->basic_list, &status);
		for (int i = 0; i < obj_cnt; i++) {
			obj_list->obj_attr[i].shaking = status.shaking[i];
		}
#endif
	}

	return MPI_SUCCESS;
err:
	return MPI_FAILURE;
}
#endif

int detectEfObject(MPI_WIN win_idx, MPI_SIZE_S *res, VFTR_EF_PARAM_S *ef_param, VFTR_SHD_PARAM_S *shd_attr,
                   VFTR_SHD_LONGTERM_LIST_S *shd_long_list)
{
	UINT32 timestamp = 0;
	INT32 timeout = 0;
	INT32 ret = 0;
	int frame_interval_cnt = 0;

	g_ef_running = 1;
	VFTR_SHD_INSTANCE_S *shd = NULL;
	VFTR_EF_INSTANCE_S *ef;
	MPI_IVA_OBJ_LIST_S *obj_list;
	(void)res; // unused parameter

#ifdef CONFIG_APP_EF_SUPPORT_SEI

#ifdef CONFIG_APP_SHD_SUPPORT
	int support_shd = 1;
#endif
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

	/* init EF ctx */
	set_idx = findEfCtx(win_idx, vftr_res_shm->ef_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("EF is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create EF instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	AVFTR_EF_CTX_S *ef_ctx = &vftr_res_shm->ef_ctx[empty_idx];
	ef_ctx->en = 1;
	ef_ctx->reg = 1;
	ef_ctx->idx = win_idx;
	ef_ctx->cb = NULL;

#ifdef CONFIG_APP_SHD_SUPPORT
	/* init SHD ctx */
	set_idx = findShdCtx(win_idx, vftr_res_shm->shd_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("SHD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create SHD instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	AVFTR_SHD_CTX_S *shd_ctx = &vftr_res_shm->shd_ctx[empty_idx];
	shd_ctx->en = 1;
	shd_ctx->reg = 1;
	shd_ctx->idx = win_idx;
#endif

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

	/* create EF instance */
	ef = VFTR_EF_newInstance();
	if (!ef) {
		log_err("Failed to create EF instance");
		return -EINVAL;
	}

	/* set parameter to ef */
	ret = VFTR_EF_setParam(ef, ef_param);
	if (ret) {
		log_err("Failed to set EF parameters. err: %d", ret);
		return -EIO;
	}

#ifdef CONFIG_APP_SHD_SUPPORT
	/* create SHD instance */
	shd = VFTR_SHD_newInstance();
	if (!shd) {
		log_err("Failed to create SHD instance.");
		return -EINVAL;
	}

	/* set parameter to shd */
	ret = VFTR_SHD_setParam(shd, shd_attr);
	if (ret) {
		log_err("Failed to set SHD parameters. err: %d", ret);
		return -EINVAL;
	}

	ret = VFTR_SHD_setUserLongTermList(shd, shd_long_list);
	if (ret) {
		log_err("Failed to set long-term parameters. err: %d", ret);
		return -EINVAL;
	}
#endif

	while (g_ef_running) {
		/* wait one video frame processed */
		if ((ret = MPI_DEV_waitWin(win_idx, &timestamp, timeout)) != MPI_SUCCESS) {
			log_err("Wait ISP statistics fail for win:0x%x. err: %d", win_idx.value, ret);
			continue;
		}

		/* do motion detection once every g_interval */
		if (frame_interval_cnt < g_interval) {
			++frame_interval_cnt;
			continue;
		} else {
			frame_interval_cnt = 0;
		}

#ifdef CONFIG_APP_EF_SUPPORT_SEI
		/* get buffer index, fill the result to shared memory
		 * it will be read and embedded as SEI data through RTSP streaming
		 * */
		int buf_idx = updateBufferIdx(buf_info, timestamp);

		VIDEO_FTR_OBJ_LIST_S *vftr_obj_list = &od_ctx->ol[buf_idx];

		/**
		 * get object list from MPI
		 */
		if (support_shd) {
			ret = getObjList(win_idx, timestamp, vftr_obj_list, shd);
			if (ret != MPI_SUCCESS) {
				continue;
			}
		} else {
			ret = getObjList(win_idx, timestamp, vftr_obj_list, NULL);
			if (ret != MPI_SUCCESS) {
				continue;
			}
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
		DBG("still alive %u\n", timestamp);
#endif

		VFTR_EF_STATUS_S *ef_res;
#ifdef CONFIG_APP_EF_SUPPORT_SEI
		ef_res = &ef_ctx->ef_res[buf_idx];
#else
		VFTR_EF_STATUS_S status;
		ef_res = &status;
#endif
		/**
		 * electronic fence detection
		 */
		ret = VFTR_EF_detect(ef, obj_list, ef_res);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to update efence. err: %d", ret);
			continue;
		}

		/* mark buffer ready for display */
#ifdef CONFIG_APP_EF_SUPPORT_SEI
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	/* delete EF instance */
	VFTR_EF_deleteInstance(&ef);

	VFTR_exit();

	clearOdCtx(win_idx, vftr_res_shm->od_ctx);
	clearEfCtx(win_idx, vftr_res_shm->ef_ctx);
	clearShdCtx(win_idx, vftr_res_shm->shd_ctx);
	clearVftrBufCtx(win_idx, vftr_res_shm->buf_info);

	return 0;
}
