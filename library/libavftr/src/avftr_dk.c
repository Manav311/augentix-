#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <pthread.h>
#include "avftr_log.h"

#include "vftr_dk.h"
#include "avftr_common.h"
#include "avftr_dk.h"
#include "avftr.h"

//#define AVFTR_DK_DEBUG_API
//#define AVFTR_DK_SHOW_OD_REPORT

#ifdef AVFTR_DK_DEBUG_API
#define AVFTR_DK_API_INFO(fmt, args...) printf("[DK] " fmt, ##args)
#else
#define AVFTR_DK_API_INFO(fmt, args...)
#endif

typedef struct {
	VFTR_DK_STATUS_S dk_res;
	AVFTR_DK_PARAM_S param;
	VFTR_DK_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
} DK_CTX_S;

static DK_CTX_S g_dk_ctx[AVFTR_DK_MAX_SUPPORT_NUM] = { { { 0 } } };
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

#define DK_GET_CTX(idx) &g_dk_ctx[idx]

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video door keeper content.
 * @param[out] empty        global door keeper index.
 * @see AVFTR_DK_addInstance()
 * @see AVFTR_DK_deleteInstance()
 * @retval enable index.
 */
static int findDkCtx(const MPI_WIN idx, const AVFTR_DK_CTX_S *ctx, int *empty)
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

static void determineDkRes(const VFTR_DK_STATUS_S *dk_status, const AVFTR_DK_PARAM_S *param,
                           const MPI_IVA_OBJ_LIST_S *obj_list, AVFTR_DK_STATUS_S *dk_res_shm)
{
	dk_res_shm->dk_status = *dk_status;
	dk_res_shm->roi = param->dk_param.roi_pts;
#ifdef AVFTR_DK_SHOW_OD_REPORT
	copy_obj_list(obj_list, &dk_res_shm->obj_list);
#endif
	return;
}

/**
 * @brief Get predefined formatted DK result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  dk_res       video door keeper result.
 * @param[out] str          formatted DK result string buffer.
 * @see AVFTR_DK_getRes()
 * @retval length of formatted DK result.
 */
static int getDkMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                     const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                     const AVFTR_DK_STATUS_S *dk_res, char *str)
{
	int offset = 0;
	MPI_RECT_POINT_S local_roi = dk_res->roi;
#ifdef AVFTR_DK_SHOW_OD_REPORT
	MPI_IVA_OBJ_LIST_S dst_list;
	copy_obj_list(&dk_res->obj_list, &dst_list);
#endif

	if (src_idx.value != dst_idx.value) {
		rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &local_roi);
#ifdef AVFTR_DK_SHOW_OD_REPORT
		for (int i = 0; i < dst_list.obj_num; i++) {
			rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_list.obj[i].rect);
		}
#endif
	}

	offset += print_meta(&str[offset],
	                     "<DK>"
	                     "<ROI RECT=\"%d %d %d %d\" EVT=\"%d\"/>",
	                     "\"dk\":{"
	                     "\"roi\":{\"rect\":[%d,%d,%d,%d],\"evt\":%d}",
	                     local_roi.sx, local_roi.sy, local_roi.ex, local_roi.ey, dk_res->dk_status.result);

#ifdef AVFTR_DK_SHOW_OD_REPORT
	offset += print_meta(&str[offset], "<OD>", ",\"od\":[");

	for (int i = 0; i < dst_list.obj_num; i++) {
		offset += print_meta(&str[offset], "<OBJ ID=\"%d\" RECT=\"%d %d %d %d\" DURATION=\"%d\" EVT=\"%d\"/>",
		                     "{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"duration\":%d,\"evt\":%d}},",
		                     dst_list.obj[i].id, dst_list.obj[i].rect.sx, dst_list.obj[i].rect.sy,
		                     dst_list.obj[i].rect.ex, dst_list.obj[i].rect.ey,
		                     dk_res->dk_status.stat[i].duration, dk_res->dk_status.stat[i].result);
	}

#ifndef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	if (dst_list.obj_num)
		offset--;
#endif
	offset += print_meta(&str[offset], "</OD>", "]");

#endif /* AVFTR_DK_SHOW_OD_REPORT */

	offset += print_meta(&str[offset], "</DK>", "},");
	return offset;
}

/**
 * @brief Get enable status of door keeper.
 * @param[in]  idx          video window index.
 * @param[in]  vftr_dk_ctx  video door keeper content.
 * @see none.
 * @retval enable status of door keeper.
 */
int AVFTR_DK_getStat(const MPI_WIN idx, AVFTR_DK_CTX_S *vftr_dk_ctx)
{
	const int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);
	return enable_idx < 0 ? 0 : vftr_dk_ctx[enable_idx].en;
}

/**
 * @brief Get result of door keeper.
 * @param[in]  idx          video window index.
 * @param[in]  obj_list     object list.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          DK object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_DK_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	const int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);
	int ret = 0;

	if (enable_idx < 0 || !vftr_dk_ctx[enable_idx].en) {
		return 0;
	}

	DK_CTX_S *ctx = DK_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("DK instance is NULL!");
		return -EFAULT;
	}

	AVFTR_DK_STATUS_S *dk_result_shm = &vftr_dk_ctx[enable_idx].dk_res[buf_idx];
	VFTR_DK_STATUS_S *dk_status = &ctx->dk_res;

	AVFTR_DK_PARAM_S *param = &ctx->param;

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_DK_detect(ctx->instance, &obj_list->basic_list, dk_status);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != 0) {
		avftr_log_err("Failed to run door keeper! err: %d", ret);
		return ret;
	}
	determineDkRes(dk_status, param, &obj_list->basic_list, dk_result_shm);

	return 0;
}

/**
 * @brief Translate result of door keeper.
 * @param[in]  vftr_dk_ctx      video door keeper result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted DK result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_Dk_getRes()
 * @retval length of door keeper result.
 */
// This function should be invoked by AVFTR client routine only.
int AVFTR_DK_transRes(AVFTR_DK_CTX_S *vftr_dk_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findDkCtx(src_idx, vftr_dk_ctx, NULL);
	if (enable_idx < 0 || !vftr_dk_ctx[enable_idx].en) {
		/* idx is not registered yet */
		return 0;
	}

	/* idx is registered and enable */
	return getDkMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                 &vftr_dk_ctx[enable_idx].dk_res[buf_idx], str);
}

/**
 * @brief Add door keeper instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_DK_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create DK instance.
 * @retval -ENOMEM          No more space to register idx / malloc DK instance failed.
 */
int AVFTR_DK_addInstance(const MPI_WIN idx)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	int empty_idx;
	int set_idx = findDkCtx(idx, vftr_dk_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("DK is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered */
		DK_CTX_S *ctx = DK_GET_CTX(empty_idx);
		ctx->instance = VFTR_DK_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create DK instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_dk_ctx[empty_idx].idx = idx;
		vftr_dk_ctx[empty_idx].reg = 1;
		vftr_dk_ctx[empty_idx].en = 0;
	} else {
		avftr_log_err("No more space to add DK instance for win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete door keeper instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_DK_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_DK_deleteInstance(const MPI_WIN idx)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("DK is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_dk_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("DK is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	DK_CTX_S *ctx = DK_GET_CTX(enable_idx);
	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_DK_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != 0) {
		avftr_log_err("Free dk instance failed! err: %d", ret);
		return ret;
	}
	vftr_dk_ctx[enable_idx].reg = 0;
	vftr_dk_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Enable door keeper.
 * @param[in]  idx          video window index.
 * @see AVFTR_DK_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_DK_enable(const MPI_WIN idx)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("DK is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_dk_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("DK is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_enableOd_implicit(idx);
	if (ret != 0) {
		return ret;
	}

	vftr_dk_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable door keeper.
 * @param[in]  idx          video window index.
 * @see AVFTR_DK_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_DK_disable(const MPI_WIN idx)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("DK is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_dk_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("DK is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_disableOd_implicit(idx);
	if (ret != 0) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}
	vftr_dk_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of door keeper.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video door keeper parameters.
 * @see AVFTR_DK_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_DK_getParam(const MPI_WIN idx, AVFTR_DK_PARAM_S *param)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("DK is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	DK_CTX_S *ctx = DK_GET_CTX(enable_idx);
	int ret;

	ret = checkMpiDevValid(idx);
	if (ret != 0) {
		return ret;
	}

	param->dk_param = ctx->param.dk_param;

	return 0;
}

/**
 * @brief set parameters of door keeper.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_DK_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video door keeper parameters.
 * @see AVFTR_DK_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_DK_setParam(const MPI_WIN idx, const AVFTR_DK_PARAM_S *param)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("DK is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	DK_CTX_S *ctx = DK_GET_CTX(enable_idx);

	int ret;

	ret = VFTR_DK_checkParam(&param->dk_param);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to vftr_dk
	pthread_mutex_lock(&ctx->lock);
	ctx->param.dk_param = param->dk_param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of door keeper instance.
 * @param[in] idx           video window index.
 * @see AVFTR_DK_getParam()
 * @see AVFTR_DK_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_DK_writeParam(const MPI_WIN idx)
{
	AVFTR_DK_CTX_S *vftr_dk_ctx = vftr_res_shm->dk_ctx;
	const int enable_idx = findDkCtx(idx, vftr_dk_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	DK_CTX_S *ctx = DK_GET_CTX(enable_idx);
	AVFTR_DK_PARAM_S *param = &ctx->param;

	int ret;

	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	int s_flag = ctx->s_flag;
	if (s_flag == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_DK_setParam(ctx->instance, &res, &param->dk_param);
		ctx->s_flag = 0;
		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}
