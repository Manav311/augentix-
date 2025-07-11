#include "dk_demo.h"

#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "vftr.h"
#include "vftr_dk.h"
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

extern int g_dk_running;

#ifdef CONFIG_APP_DK_SUPPORT_SEI
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

static int findDkCtx(MPI_WIN idx, AVFTR_DK_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_DK_MAX_SUPPORT_NUM; i++) {
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

int detectDkObject(MPI_WIN win_idx, MPI_SIZE_S *res, VFTR_DK_PARAM_S *dk_param)
{
	UINT32 timestamp = 0;
	const INT32 timeout = 0;
	INT32 ret = 0;
	VFTR_DK_INSTANCE_S *instance;
	MPI_IVA_OBJ_LIST_S *obj_list;

	g_dk_running = 1;

#ifdef CONFIG_APP_DK_SUPPORT_SEI
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
	od_ctx->idx = win_idx;
	od_ctx->cb = NULL;
	od_ctx->bdry.sx = 0;
	od_ctx->bdry.sy = 0;
	od_ctx->bdry.ex = res->width - 1;
	od_ctx->bdry.ey = res->height - 1;

	/* init DK ctx */
	set_idx = findDkCtx(win_idx, vftr_res_shm->dk_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("DK is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create DK instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	AVFTR_DK_CTX_S *dk_ctx = &vftr_res_shm->dk_ctx[empty_idx];
	dk_ctx->en = 1;
	dk_ctx->reg = 1;
	dk_ctx->idx = win_idx;

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

	/* create DK instance */
	instance = VFTR_DK_newInstance();
	if (!instance) {
		log_err("Failed to create DK instance.");
		return -EINVAL;
	}

	/* set parameter to dk */
	ret = VFTR_DK_setParam(instance, res, dk_param);
	if (ret) {
		log_err("Failed to set DK parameters. err: %d", ret);
		return -EINVAL;
	}

	while (g_dk_running) {
		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(win_idx, &timestamp, timeout);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", win_idx.dev, win_idx.chn,
			        win_idx.win, ret);
			continue;
		}

#ifdef CONFIG_APP_DK_SUPPORT_SEI
		/* get buffer index, fill the result to shared memory
		 * it will be read and embedded as SEI data through RTSP streaming
		 * */
		int buf_idx = updateBufferIdx(buf_info, timestamp);

		VIDEO_FTR_OBJ_LIST_S *vftr_obj_list = &od_ctx->ol[buf_idx];

		// Continuously get detection result from MPI.
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

		VFTR_DK_STATUS_S *dk_res = NULL;
#ifdef CONFIG_APP_DK_SUPPORT_SEI
		dk_res = &dk_ctx->dk_res[buf_idx].dk_status;
		dk_ctx->dk_res[buf_idx].roi = dk_param->roi_pts;
#else
		VFTR_DK_STATUS_S status;
		dk_res = &status;
#endif
		// Send video statistics to DK algorithm module.
		ret = VFTR_DK_detect(instance, obj_list, dk_res);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to detect DK. err: %d", ret);
			continue;
		}

		/* mark buffer ready for display */
#ifdef CONFIG_APP_DK_SUPPORT_SEI
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	/* delete DK instance */
	VFTR_DK_deleteInstance(&instance);

	VFTR_exit();

	return 0;
}
