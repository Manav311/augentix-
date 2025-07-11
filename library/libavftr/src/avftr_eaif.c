#include "avftr_eaif.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "avftr_log.h"


#include "mpi_index.h"
#include "mpi_sys.h"
#include "mpi_base_types.h"
#include "mpi_errno.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_iva.h"

#include "avftr.h"
#include "avftr_common.h"

#include "eaif.h"

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

typedef struct {
	pthread_mutex_t lock;
	AVFTR_EAIF_PARAM_S param;
	EAIF_INSTANCE_S *instance;
} EAIF_CTX_S;

static EAIF_CTX_S g_eaif_ctx[AVFTR_EAIF_MAX_SUPPORT_NUM] = { { { { 0 } } } };
#define EAIF_GET_CTX(idx) &g_eaif_ctx[idx]

static int g_replace_od = 0;

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video edge AI assisted feature content.
 * @param[out] empty        global edge AI assisted feature index.
 * @see AVFTR_EAIF_addInstance()
 * @see AVFTR_EAIF_deleteInstance()
 * @retval enable index.
 */
static int findEaifCtx(const MPI_WIN idx, const AVFTR_EAIF_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}
	for (i = 0; i < AVFTR_EAIF_MAX_SUPPORT_NUM; i++) {
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

static void genAvftrEaifDetect(const EAIF_STATUS_S *status, VIDEO_FTR_OBJ_LIST_S *ol)
{
	int i;
	VIDEO_FTR_OBJ_ATTR_S *obj = NULL;
	MPI_IVA_OBJ_ATTR_S *od = NULL;
	const EAIF_OBJ_ATTR_S *eobj = NULL;

	ol->basic_list.obj_num = status->obj_cnt;
	for (i = 0; (UINT32)i < status->obj_cnt; i++) {
		eobj = &status->obj_attr[i];
		obj = &ol->obj_attr[i];
		od = &ol->basic_list.obj[i];

		od->id = eobj->id; // TBD how to maintain detection id ?
		od->mv = (MPI_MOTION_VEC_S){ 0, 0 }; // TBD how to get detection motion vector ?
		od->life = EAIF_MAX_OBJ_LIFE_TH; // TBD how to determine detection life ?
		od->rect = eobj->rect;
		obj->shaking = 0;
		strncpy(obj->cat, eobj->category[0], VFTR_OBJ_CAT_LEN);
		strncpy(obj->conf, eobj->prob[0], VFTR_OBJ_CONF_LEN);
	}
	return;
}

static void genAvftrEaifClassify(const EAIF_STATUS_S *status, VIDEO_FTR_OBJ_LIST_S *ol)
{
	int i, j, k, set;
	int datasize;
	int range;
	VIDEO_FTR_OBJ_ATTR_S *obj = NULL;
	MPI_IVA_OBJ_ATTR_S *od = NULL;
	const EAIF_OBJ_ATTR_S *eobj = NULL;

	for (i = 0; i < ol->basic_list.obj_num; i++) {
		obj = &ol->obj_attr[i];
		od = &ol->basic_list.obj[i];
		set = 0;
		for (j = 0; (UINT32)j < status->obj_cnt; j++) {
			eobj = &status->obj_attr[j];
			if (od->id == eobj->id) {
				datasize = 0;
				int conf_size = 0;
				range = VFTR_OBJ_CAT_LEN;
				for (k = 0; k < eobj->label_num && range >= 0; ++k) {
					datasize += snprintf(obj->cat + datasize, range, "%s ", eobj->category[k]);
					range = VFTR_OBJ_CAT_LEN - datasize;
					conf_size += snprintf(&obj->conf[conf_size], sizeof(obj->conf) - conf_size,
					                      "%s ", eobj->prob[k]);
				}
				set = 1;
				break;
			}
		}
		if (set == 0) {
			strcpy(obj->cat, "NoMatch");
		}
	}

	return;
}

static void genAvftrEaifRes(const AVFTR_EAIF_PARAM_S *p, const EAIF_STATUS_S *status, VIDEO_FTR_OBJ_LIST_S *ol)
{
	int i;
	VIDEO_FTR_OBJ_ATTR_S *obj = NULL;
	if (status->server_reachable == EAIF_URL_REACHABLE) {
		/* TODO: Modify OD attr to support prob or other attri */
		if (p->api == EAIF_API_FACERECO || p->api == EAIF_API_CLASSIFY || p->api == EAIF_API_CLASSIFY_CV ||
		    p->api == EAIF_API_HUMAN_CLASSIFY) {
			genAvftrEaifClassify(status, ol);
		} else if (p->api == EAIF_API_DETECT) {
			if (g_replace_od)
				genAvftrEaifDetect(status, ol);
		}
	} else {
		for (i = 0; i < ol->basic_list.obj_num; i++) {
			obj = &ol->obj_attr[i];
			obj->cat[0] = '\0';
		}
	}
	return;
}

// toAvoid extra function definition without using avftr_common
static void rescaleEaifMpiRectPoint(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                                    const MPI_RECT_S *dst_roi, MPI_RECT_POINT_S *roi)
{
	INT32 roi_sx = roi->sx - src_rect->x;
	INT32 roi_sy = roi->sy - src_rect->y;
	INT32 roi_ex = roi->ex - src_rect->x;
	INT32 roi_ey = roi->ey - src_rect->y;

	INT32 src_roi_width = src_roi->width;
	INT32 src_roi_height = src_roi->height;
	INT32 src_roi_x = src_roi->x;
	INT32 src_roi_y = src_roi->y;

	INT32 dst_roi_width = dst_roi->width;
	INT32 dst_roi_height = dst_roi->height;
	INT32 dst_roi_x = dst_roi->x;
	INT32 dst_roi_y = dst_roi->y;

	INT32 src_rect_width = src_rect->width;
	INT32 src_rect_height = src_rect->height;

	INT32 dst_rect_width = dst_rect->width;
	INT32 dst_rect_height = dst_rect->height;
	INT32 dst_rect_x = dst_rect->x;
	INT32 dst_rect_y = dst_rect->y;

	roi_sx = (roi_sx * src_roi_width + (src_rect_width >> 1)) / src_rect_width + src_roi_x - dst_roi_x;
	roi_sx = (roi_sx * dst_rect_width + (dst_roi_width >> 1)) / dst_roi_width;
	roi_ex = (roi_ex * src_roi_width + (src_rect_width >> 1)) / src_rect_width + src_roi_x - dst_roi_x;
	roi_ex = (roi_ex * dst_rect_width + (dst_roi_width >> 1)) / dst_roi_width;

	roi_sy = (roi_sy * src_roi_height + (src_rect_height >> 1)) / src_rect_height + src_roi_y - dst_roi_y;
	roi_sy = (roi_sy * dst_rect_height + (dst_roi_height >> 1)) / dst_roi_height;
	roi_ey = (roi_ey * src_roi_height + (src_rect_height >> 1)) / src_rect_height + src_roi_y - dst_roi_y;
	roi_ey = (roi_ey * dst_rect_height + (dst_roi_height >> 1)) / dst_roi_height;

	roi_sx += dst_rect_x;
	roi_sy += dst_rect_y;
	roi_ex += dst_rect_x;
	roi_ey += dst_rect_y;

	roi->sx = CLAMP(roi_sx, SHRT_MIN, SHRT_MAX);
	roi->sy = CLAMP(roi_sy, SHRT_MIN, SHRT_MAX);
	roi->ex = CLAMP(roi_ex, SHRT_MIN, SHRT_MAX);
	roi->ey = CLAMP(roi_ey, SHRT_MIN, SHRT_MAX);

	return;
}

/**
 * @brief Get predefined formatted EAIF result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  eaif_stat    video edge AI assisted feature result.
 * @param[out] str          formatted EAIF result string buffer.
 * @see AVFTR_EAIF_getRes()
 * @retval length of formatted EAIF result.
 */
static int getEaifMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                       const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                       const EAIF_STATUS_S *eaif_stat, char *str)
{
	int offset = 0;
	int i, j;
	if (eaif_stat->obj_attr[0].rect.sx == EAIF_RECT_INIT)
		return 0;

	EAIF_STATUS_S dst_stat = *eaif_stat;
	EAIF_OBJ_ATTR_S *attr = NULL;

	if (src_idx.value != dst_idx.value) {
		for (int i = 0; (UINT32)i < dst_stat.obj_cnt; i++) {
			rescaleEaifMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_stat.obj_attr[i].rect);
		}
	}

	offset += print_meta(&str[offset], "<EAIF>", "\"eaif\":[");
	for (i = 0; (UINT32)i < dst_stat.obj_cnt; i++) {
		attr = &dst_stat.obj_attr[i];
		char categories[256] = {};
		int str_size = 0;
		for (j = 0; j < attr->label_num; j++)
			str_size += sprintf(&categories[str_size], "%s ", attr->category[j]);
		if (attr->label_num)
			categories[str_size - 1] = 0;

		offset += print_meta(&str[offset], "<OBJ ID=\"%d\" RECT=\"%d %d %d %d\" CAT=\"%s\"/>",
		                     "{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"cat\":\"%s\"}},", attr->id,
		                     attr->rect.sx, attr->rect.sy, attr->rect.ex, attr->rect.ey, categories);
	}

#ifndef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	if (dst_stat.obj_cnt)
		offset--;
#endif
	offset += print_meta(&str[offset], "</EAIF>", "],");
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @see AVFTR_EAIF_enable()
 */
static void alarmEmptyCb(const MPI_WIN idx __attribute__((unused)), void *args __attribute__((unused)))
{
	return;
}

/**
 * @brief Translate result of edge AI assisted feature.
 * @param[in]  vftr_eaif_ctx    video edge AI assisted feature result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted EAIF result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_EAIF_getRes()
 * @retval length of edge AI assisted feature result.
 */
// This function should be invoked by AVFTR client routine only.
int AVFTR_EAIF_transRes(AVFTR_EAIF_CTX_S *vftr_eaif_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                        const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                        const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findEaifCtx(src_idx, vftr_eaif_ctx, NULL);
	if (enable_idx < 0 || !vftr_eaif_ctx[enable_idx].en) {
		return 0;
	}

	return getEaifMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                   &vftr_eaif_ctx[enable_idx].stat[buf_idx], str);
}

/**
 * @brief Add edge AI assisted feature instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_EAIF_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create EAIF instance.
 * @retval -ENOMEM          No more space to register idx / malloc EAIF instance failed.
 */
int AVFTR_EAIF_addInstance(const MPI_WIN idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	int empty_idx;
	const int set_idx = findEaifCtx(idx, vftr_eaif_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("EAIF is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered */
		EAIF_CTX_S *ctx = EAIF_GET_CTX(empty_idx);

		ctx->instance = EAIF_newInstance(idx);
		if (!ctx->instance) {
			avftr_log_err("Failed to create EAIF instance on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
			return -EFAULT;
		}

		vftr_eaif_ctx[empty_idx].idx = idx;
		vftr_eaif_ctx[empty_idx].reg = 1;
		vftr_eaif_ctx[empty_idx].en = 0;
		vftr_eaif_ctx[empty_idx].cb = NULL;
	} else {
		/* No more space to register idx */
		avftr_log_err("Add EAIF instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete edge AI assisted feature instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_EAIF_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 * @retval -EFAULT          EAIF instance is NULL.
 */
int AVFTR_EAIF_deleteInstance(const MPI_WIN idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	const int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);

	if (enable_idx < 0) {
		return 0;
	}

	if (vftr_eaif_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("EAIF is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn,
		              idx.win);
		return -EAGAIN;
	}

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);

	/* Free EAIF instance*/
	if (ctx->instance == NULL) {
		avftr_log_err("Pointer to the EAIF instance should not be NULL.");
		return -EFAULT;
	}

	if (ctx->instance->algo_status == NULL) {
		avftr_log_err("Pointer to the EAIF algo instance should not be NULL.");
		return -EFAULT;
	}

	pthread_mutex_lock(&ctx->lock);
	EAIF_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ctx->instance) {
		avftr_log_err("Cannot delete eaif instance.");
		return -EFAULT;
	}

	/* !Free EAIF object*/
	vftr_eaif_ctx[enable_idx].reg = 0;
	vftr_eaif_ctx[enable_idx].en = 0;
	vftr_eaif_ctx[enable_idx].cb = NULL;

	return 0;
}

/**
 * @brief Enable edge AI assisted feature (including OD).
 * @param[in]  idx          video window index.
 * @see AVFTR_EAIF_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_EAIF_enable(const MPI_WIN idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("EAIF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_eaif_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("EAIF is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_enableOd_implicit(idx);
	if (ret != 0) {
		return ret;
	}

	if (vftr_eaif_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("EAIF alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_eaif_ctx[enable_idx].cb = alarmEmptyCb;
	}

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);

	//EAIF_ENABLE
	pthread_mutex_lock(&ctx->lock);
	ret = EAIF_activate(ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		return ret;
	}

	vftr_eaif_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Enable edge AI assisted feature.
 * @param[in]  idx          video window index.
 * @see AVFTR_EAIF_disableV2()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_EAIF_enableV2(const MPI_WIN idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	const int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("EAIF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_eaif_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("EAIF is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	if (vftr_eaif_ctx[enable_idx].cb == NULL) {
		//avftr_log_err("Door Keerper alarm callback function is not registered on win %u.", idx.win);
		vftr_eaif_ctx[enable_idx].cb = alarmEmptyCb;
	}

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);

	//EAIF_ENABLE
	int ret = EAIF_activate(ctx->instance);
	if (ret != 0) {
		return ret;
	}

	vftr_eaif_ctx[enable_idx].en = 1;
	return 0;
}

/**
 * @brief Disable edge AI assisted feature (including OD).
 * @param[in]  idx          video window index.
 * @see AVFTR_EAIF_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EAIF_disable(const MPI_WIN idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EAIF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_eaif_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("EAIF is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	int ret = 0;

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);

	//Disable EAIF
	pthread_mutex_lock(&ctx->lock);
	ret = EAIF_deactivate(ctx->instance);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != 0) {
		return ret;
	}

	ret = VIDEO_FTR_disableOd_implicit(idx);
	if (ret != 0) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}
	vftr_eaif_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Disable edge AI assisted feature.
 * @param[in]  idx          video window index.
 * @see AVFTR_EAIF_enableV2()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EAIF_disableV2(const MPI_WIN idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	const int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EAIF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_eaif_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("EAIF is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	int ret = 0;

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);

	//Disable EAIF
	ret = EAIF_deactivate(ctx->instance);
	if (ret != 0) {
		return ret;
	}

	vftr_eaif_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of edge AI assisted feature.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video edge AI assisted feature parameters.
 * @see AVFTR_EAIF_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EAIF_getParam(const MPI_WIN idx, AVFTR_EAIF_PARAM_S *param)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EAIF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);
	pthread_mutex_lock(&ctx->lock);
	int ret = EAIF_getParam(ctx->instance, param);
	pthread_mutex_unlock(&ctx->lock);
	if (ret) {
		avftr_log_err("Cannot get eaif param!");
		return ret;
	}

	return 0;
}

/**
 * @brief set parameters of edge AI assisted feature.
 * @details Set parameters to buffer.
 * @param[in]  idx          video window index.
 * @param[in]  param        video edge AI assisted feature parameters.
 * @see AVFTR_EAIF_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 * @retval -EFAULT          input parameter is null.
 */
int AVFTR_EAIF_setParam(const MPI_WIN idx, const AVFTR_EAIF_PARAM_S *param)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	const int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);
	int ret;

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EAIF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!param) {
		avftr_log_err("Args should not be NULL.");
		return -EFAULT;
	}

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);

	ret = EAIF_checkParam(param);
	if (ret) {
		avftr_log_err("Invalid EAIF input param!");
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = EAIF_setParam(ctx->instance, param);
	pthread_mutex_unlock(&ctx->lock);
	if (ret) {
		avftr_log_err("Fail to set EAIF param!");
		return ret;
	}

	if (vftr_eaif_ctx[enable_idx].en && param->inf_utils.cmd != EAIF_INF_NONE && param->api == EAIF_API_FACERECO &&
	    !strcmp(param->url, EAIF_INFERENCE_INAPP_STR)) {
		ret = EAIF_applyFaceUtils(ctx->instance, param);
		if (ret) {
			avftr_log_err("Fail to apply face utils!");
			return ret;
		}
	}

	pthread_mutex_lock(&ctx->lock);
	ctx->param = *param;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Get result of edge AI assisted feature.
 * @param[in]  idx          video window index.
 * @param[in]  obj_list     object list.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          EAIF object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_EAIF_getRes(const MPI_WIN idx, VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx)
{
	AVFTR_EAIF_CTX_S *vftr_eaif_ctx = vftr_res_shm->eaif_ctx;
	const int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);

	if (enable_idx < 0 || !vftr_eaif_ctx[enable_idx].en) {
		return 0;
	}

	EAIF_CTX_S *ctx = EAIF_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("EAIF instance is NULL!");
		return -EFAULT;
	}

	EAIF_STATUS_S *status = &vftr_eaif_ctx[enable_idx].stat[buf_idx];
	EAIF_testRequest(ctx->instance, &obj_list->basic_list, status);

	pthread_mutex_lock(&ctx->lock);
	genAvftrEaifRes(&ctx->param, status, obj_list);
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Get enable status of edge AI assisted feature.
 * @param[in]  idx              video window index.
 * @param[in]  vftr_eaif_ctx    video edge AI assisted feature content.
 * @see none.
 * @retval enable status of edge AI assisted feature.
 */
int AVFTR_EAIF_getStat(const MPI_WIN idx, AVFTR_EAIF_CTX_S *vftr_eaif_ctx)
{
	const int enable_idx = findEaifCtx(idx, vftr_eaif_ctx, NULL);
	return enable_idx < 0 ? 0 : vftr_eaif_ctx[enable_idx].en;
}

/**
 * @brief Check edge AI assisted feature parameter.
 * @param[in]  param        video edge AI assisted feature parameters.
 * @see none.
 * @retval 0                input parameter is valid.
 * @retval -EFAULT          input parameter is null.
 * @retval -EINVAL          input parameter is invalid.
 */
int AVFTR_EAIF_checkParam(const AVFTR_EAIF_PARAM_S *param)
{
	if (!param) {
		return -EFAULT;
	}

	return -EAIF_checkParam(param);
}
