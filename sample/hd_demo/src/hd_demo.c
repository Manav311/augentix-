#include <errno.h>
#include <unistd.h>
#include <stdio.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "hd_demo.h"
#include "log.h"

#ifdef CONFIG_APP_HD_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include "video_od.h"

//#define _DEBUG

#ifdef _DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

extern MPI_RECT_POINT_S g_chn_bdry;
extern MPI_IVA_OD_PARAM_S g_od_param;
extern int g_hd_running;
extern HD_SCENE_PARAM_S g_hd_scene_param;

#ifdef CONFIG_APP_HD_SUPPORT_SEI
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

/**
 * @brief update ring buffer for iva sei display
 */
static int updateVftrBufferInfo(AVFTR_VIDEO_BUF_INFO_S *buf_info, uint32_t timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;
	return buf_info->buf_cur_idx;
}

/**
 * @brief fillin data for od with hd result for rtsp iva display
 */
static int GetOdRes(MPI_WIN idx __attribute__((unused)), const MPI_IVA_OBJ_LIST_S *ol, const EAIF_STATUS_S *status, int buf_idx)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];
	VIDEO_FTR_OBJ_LIST_S *obj_list;
	const EAIF_OBJ_ATTR_S *eaif_obj = NULL;

	int i;
	unsigned int j;
	int id;

	if (vftr_od_ctx->en) {
		obj_list = &vftr_od_ctx->ol[buf_idx];
		obj_list->basic_list = *ol;
		for (i = 0; i < ol->obj_num; i++) {
			id = obj_list->basic_list.obj[i].id;
			eaif_obj = NULL;
			for (j = 0; j < status->obj_cnt; j++) {
				if (id == status->obj_attr[j].id) {
					eaif_obj = &status->obj_attr[j];
					break;
				}
			}

			if (eaif_obj && eaif_obj->label_num) {
				strncpy(obj_list->obj_attr[i].cat, eaif_obj->category[0], VFTR_OBJ_CAT_LEN - 1);
				strncpy(obj_list->obj_attr[i].conf, eaif_obj->prob[0], VFTR_OBJ_CAT_LEN - 1);
			} else {
				obj_list->obj_attr[i].cat[0] = 0;
				obj_list->obj_attr[i].conf[0] = 0;
			}
		}
	}

	return MPI_SUCCESS;
}

/**
 * @brief fill object context
 */
static int GetObjList(MPI_WIN idx, UINT32 timestamp, MPI_IVA_OBJ_LIST_S *obj_list)
{
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	VIDEO_OD_CTX_S *vftr_od_ctx = &vftr_res_shm->od_ctx[od_idx];

	if (vftr_od_ctx->en) {
		int ret = 0;
		MPI_RECT_POINT_S *bd = &vftr_od_ctx->bdry;
		MPI_RECT_POINT_S *obj;
		MPI_RECT_POINT_S *final_obj;
		int obj_cnt = 0;

		ret = MPI_IVA_getBitStreamObjList(idx, timestamp, obj_list);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to get object list. err: %d", ret);
			goto err;
		}

		/* Below segment is to remove out of boundary object list */
		for (int i = 0; i < obj_list->obj_num; i++) {
			/* Limit OL boundary */
			obj = &obj_list->obj[i].rect;
			final_obj = &obj_list->obj[obj_cnt].rect;

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

			obj_list->obj[obj_cnt].id = obj_list->obj[i].id;
			obj_list->obj[obj_cnt].life = obj_list->obj[i].life;
			obj_list->obj[obj_cnt].mv = obj_list->obj[i].mv;

			/* increase the index */
			obj_cnt++;
		}

		obj_list->obj_num = obj_cnt;
	}

	return MPI_SUCCESS;
err:
	return MPI_FAILURE;
}

#else /* CONFIG_APP_HD_SUPPORT_SEI */

static int GetObjList(MPI_WIN idx, UINT32 timestamp, MPI_IVA_OBJ_LIST_S *obj_list)
{
	int ret = MPI_IVA_getBitStreamObjList(idx, timestamp, obj_list);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get object list. err: %d", ret);
		return MPI_FAILURE;
	}

	return 0;
}

#endif

/**
 * @brief declaration of od enable in enable OD ()
 */
int VIDEO_FTR_enableOd(MPI_WIN idx)
{
	int ret = 0;

	MPI_IVA_OD_PARAM_S *od = &g_od_param;

	ret = MPI_IVA_setObjParam(idx, od);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to set OD param. err: %d", ret);
		goto error;
	}

	ret = MPI_IVA_enableObjDet(idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to enable OD. err: %d", ret);
		goto error;
	}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
	/* init OD ctx */
	int empty_idx, set_idx;
	VIDEO_OD_CTX_S *od_ctx;
	set_idx = findOdCtx(idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("OD is already registered on win(%u, %u, %u) set_idx:%d.", idx.dev, idx.chn, idx.win, set_idx);
		od_ctx = &vftr_res_shm->od_ctx[set_idx];
	} else {
		if (empty_idx < 0) {
			log_err("Failed to create OD on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
			return -ENOMEM;
		}

		od_ctx = &vftr_res_shm->od_ctx[empty_idx];
		od_ctx->en = 1;
		od_ctx->en_shake_det = 0;
		od_ctx->en_crop_outside_obj = 0;
		od_ctx->idx = idx;
		od_ctx->cb = NULL;
		od_ctx->bdry = g_chn_bdry;
	}
#endif

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}

/**
 * @brief declaration of od disable
 */
int VIDEO_FTR_disableOd(MPI_WIN idx)
{
	int ret = 0;

	ret = MPI_IVA_disableObjDet(idx);
	if (ret != MPI_SUCCESS) {
		log_err("Disable OD on win %u failed.", idx.win);
		goto error;
	}

	clearOdCtx(idx, vftr_res_shm->od_ctx);

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}

/**@brief filter object box by pre-defined roi attributes(coordinate, max size, min size)
  *@      foreach object
  *@         get the object center position and dimension
  *@         set copy_flag to true
  *@         for each roi
  *@             if object center position is not within roi area
  *@                   continue
  *@             if object dimension > roi max-object-size
  *@                   break and set copy_flag = false
  *@             if object dimension < roi min-object-size
  *@                   break and set copy_flag = false
  *@         if copy_flag is true
  *@             copy object to dst_list
  *@param[in] param struct HD_SCENE_PARAM_S param
  *@param[in] src_list source object list
  *@param[in/out] dst_list destinate object list
  */
void HD_runSceneRoiFilter(const HD_SCENE_PARAM_S *param, const MPI_IVA_OBJ_LIST_S *src_list,
                          MPI_IVA_OBJ_LIST_S *dst_list)
{
	int obj_cnt = 0;
	dst_list->obj_num = 0;
	dst_list->timestamp = src_list->timestamp;

	if (!param->size) {
		*dst_list = *src_list;
		return;
	}

	for (int i = 0; i < src_list->obj_num; i++) {
		const MPI_IVA_OBJ_ATTR_S *obj = &src_list->obj[i];
		int copy_obj = 1;
		int cx = (obj->rect.sx + obj->rect.ex + 1) / 2;
		int cy = (obj->rect.sy + obj->rect.ey + 1) / 2;
		int obj_w = obj->rect.ex - obj->rect.sx + 1;
		int obj_h = obj->rect.ey - obj->rect.sy + 1;

		for (int j = 0; j < param->size; j++) {
			const HD_ROI_FILTER_S *roi = &param->rois[j];

			if (cx < roi->rect.sx || cx > roi->rect.ex || cy < roi->rect.sy || cy > roi->rect.ey)
				continue;

			if (obj_w > roi->max.width || obj_h > roi->max.height) {
				copy_obj = 0;
				break;
			}
			if (obj_w < roi->min.width || obj_h < roi->min.height) {
				copy_obj = 0;
				break;
			}
		}
		if (copy_obj) {
			dst_list->obj[obj_cnt] = *obj;
			obj_cnt++;
		}
	}
	dst_list->obj_num = obj_cnt;
}

int runHumanDetection(MPI_WIN win_idx, EAIF_PARAM_S *hd_param)
{
	const MPI_WIN idx = MPI_VIDEO_WIN(win_idx.dev, win_idx.chn, win_idx.win);
	const int timeout = 0;
	EAIF_INSTANCE_S *instance;
	uint32_t timestamp = 0;
	int ret = 0;

#ifdef CONFIG_APP_HD_SUPPORT_SEI
	// init vftr buffer ctx
	int empty_idx, set_idx, buf_info_idx;
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

	if ((ret = VIDEO_FTR_enableOd(idx))) {
		return -EINVAL;
	}

	instance = EAIF_newInstance(idx);
	if (!instance) {
		log_err("Cannot add hd instance!");
		return -EINVAL;
	}

	DBG("[INFO] Add EAIF instance!\n");

	ret = EAIF_setParam(instance, hd_param);
	if (ret) {
		log_err("Cannot set HD parameter! err: %d", ret);
		return -EINVAL;
	}
	DBG("[INFO] Set Human detection parameter!\n");

	// enable eaif
	ret = EAIF_activate(instance);
	if (ret) {
		log_err("Failed to activate EAIF. err: %d", ret);
		return -EINVAL;
	}
	DBG("[INFO] Enabled EAIF!\n");
	g_hd_running = 1;

//#define SEI_DRAW_FILTER_ROI
#ifdef SEI_DRAW_FILTER_ROI
	VIDEO_VDBG_CTX_S *vdbg_ctx = &vftr_res_shm->vdbg_ctx;
	vdbg_ctx->en = 1;
	vdbg_ctx->ctx |= VIDEO_VDBG_DEBUG;
#endif
	// while running
	while (g_hd_running) {
		MPI_IVA_OBJ_LIST_S obj_list_original = { 0 };
		MPI_IVA_OBJ_LIST_S obj_list_filtered = { 0 };
		MPI_IVA_OBJ_LIST_S *obj_list = &obj_list_original;
		EAIF_STATUS_S status = { 0 };

		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(idx, &timestamp, timeout);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", idx.dev, idx.chn, idx.win, ret);
			continue;
		}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
		int buf_idx = updateVftrBufferInfo(buf_info, timestamp);
#endif
		GetObjList(idx, timestamp, &obj_list_original);

		if (g_hd_scene_param.size) {
			HD_runSceneRoiFilter(&g_hd_scene_param, &obj_list_original, &obj_list_filtered);
			obj_list = &obj_list_filtered;
		}

		// get classification result from eaif client module
		EAIF_testRequestV2(instance, obj_list, &status);

		if (obj_list->obj_num) {
			DBG("[DEB] HD result timestamp: %u\n", timestamp);
			for (int i = 0; i < obj_list->obj_num; i++) {
				DBG("[DEB] HD idx:%d life:%3d [%d,%d,%d,%d] class:%s\n", obj_list->obj[i].id,
				    obj_list->obj[i].life, obj_list->obj[i].rect.sx, obj_list->obj[i].rect.sy,
				    obj_list->obj[i].rect.ex, obj_list->obj[i].rect.ey, status.obj_attr[i].category[0]);
			}
			DBG("\n");
		}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
		// copy to object list result to share memory
		GetOdRes(idx, obj_list, &status, buf_idx);

#ifdef SEI_DRAW_FILTER_ROI
		char *vdbg_buff = &vdbg_ctx->data[buf_idx][0];
		if (g_hd_scene_param.size) {
			int data_size = 0;
			for (int j = 0; j < g_hd_scene_param.size; j++) {
				const HD_ROI_FILTER_S *roi = &g_hd_scene_param.rois[j];
				data_size += sprintf(&vdbg_buff[data_size],
				                     "{\"rect\":{\"rect\":[%d,%d,%d,%d],\"sz\":2,\"lncr\":[0,0,255]}},",
				                     roi->rect.sx, roi->rect.sy, roi->rect.ex, roi->rect.ey);
			}
			vdbg_buff[data_size] = 0;
			vdbg_ctx->data_len[buf_idx] = data_size;
		}
#endif // SEI_DRAW_FILTER_ROI
		buf_info->buf_ready[buf_idx] = 1;

#endif /* CONFIG_APP_HD_SUPPORT_SEI */
	}

	DBG("[INFO] Exiting running loop!\n");

	ret = EAIF_deactivate(instance);
	if (ret) {
		log_err("Failed to deactivate EAIF. err: %d", ret);
		return -EINVAL;
	}

	DBG("[INFO] Disabled EAIF!\n");

	ret = EAIF_deleteInstance(&instance);
	if (ret && instance) {
		log_err("Failed to delete EAIF instance. err: %d", ret);
		return -EINVAL;
	}

	DBG("[INFO] Deleted EAIF!\n");

	ret = VIDEO_FTR_disableOd(idx);
	if (ret) {
		log_err("Failed to disable OD!");
		return -EINVAL;
	}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
	clearVftrBufCtx(idx, vftr_res_shm->buf_info);
#endif

	return 0;
}
