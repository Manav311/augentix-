#include "avftr_fld.h"

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "vftr_fld.h"
#include "avftr_log.h"

#include "avftr_common.h"
#include "avftr.h"

//#define AVFTR_FLD_DEBUG_API
#define FLD_SHOW_OD_REPORT

#ifdef AVFTR_FLD_DEBUG_API
#define AVFTR_FLD_API_INFO(fmt, args...) printf("[FLD] " fmt, ##args)
#else
#define AVFTR_FLD_API_INFO(fmt, args...)
#endif

typedef struct {
	VFTR_FLD_STATUS_S fld_res;
	AVFTR_FLD_PARAM_S param;
	VFTR_FLD_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} FLD_CTX_S;

static FLD_CTX_S g_fld_ctx[AVFTR_FLD_MAX_SUPPORT_NUM] = { { { 0 } } };
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

#define FLD_GET_CTX(idx) &g_fld_ctx[idx]

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video fall detection content.
 * @param[out] empty        global fall detection index.
 * @see AVFTR_FLD_addInstance()
 * @see AVFTR_FLD_deleteInstance()
 * @retval enable index.
 */
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

/**
 * @brief Invoke callback function when fall detection event is satisfied.
 * @param[in] fld_ctx       video fall detection content.
 * @param[in] fld_evt       video fall detection event.
 * @param[in] fld_param     video fall_detection parameters.
 * @see AVFTR_FLD_getRes()
 * @retval none.
 */
static void genFldAlarm(const AVFTR_FLD_CTX_S *fld_ctx, const int fld_evt, const AVFTR_FLD_PARAM_S *fld_param)
{
	if (fld_ctx->cb == NULL) {
		return;
	}
	fld_ctx->cb(fld_ctx->idx, fld_evt, fld_param);
	return;
}

static void determineFldRes(const VFTR_FLD_STATUS_S *fld_status, const MPI_IVA_OBJ_LIST_S *obj_list,
                            AVFTR_FLD_STATUS_S *fld_stat_shm)
{
	fld_stat_shm->fld_status = *fld_status;
	copy_obj_list(obj_list, &fld_stat_shm->obj_list);
}

/**
 * @brief Get predefined formatted FLD result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  fld_stat     fall detection result.
 * @param[out] str          formatted FLD result string buffer.
 * @see AVFTR_FLD_getRes()
 * @retval length of formatted FLD result.
 */
static int getFldMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                      const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                      const AVFTR_FLD_STATUS_S *fld_stat, char *str)
{
	int offset = 0;
#ifdef FLD_SHOW_OD_REPORT
	MPI_IVA_OBJ_LIST_S dst_list;
	copy_obj_list(&fld_stat->obj_list, &dst_list);
#endif

	if (src_idx.value != dst_idx.value) {
#ifdef FLD_SHOW_OD_REPORT
		for (int i = 0; i < dst_list.obj_num; i++) {
			rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_list.obj[i].rect);
		}
#endif
	}

	offset += print_meta(&str[offset],
	                     "<FLD>"
	                     "<EVT>%d</EVT>",
	                     "\"fld\":{"
	                     "\"evt\":%d",
	                     fld_stat->fld_status.result);

#ifdef FLD_SHOW_OD_REPORT
	offset += print_meta(&str[offset], "<OD>", ",\"od\":[");

	for (int i = 0; i < dst_list.obj_num; i++) {
		offset += print_meta(&str[offset], "<OBJ ID=\"%d\" RECT=\"%d %d %d %d\" EVT=\"%d\"/>",
		                     "{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"evt\":%d}},", dst_list.obj[i].id,
		                     dst_list.obj[i].rect.sx, dst_list.obj[i].rect.sy, dst_list.obj[i].rect.ex,
		                     dst_list.obj[i].rect.ey, fld_stat->fld_status.obj_stat_list[i].result);
	}

#ifndef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	if (dst_list.obj_num)
		offset--;
#endif
	offset += print_meta(&str[offset], "</OD>", "]");

#endif /* FLD_SHOW_OD_REPORT */

	offset += print_meta(&str[offset], "</FLD>", "},");
	return offset;
}

static void emptyFldCallback(const MPI_WIN idx __attribute__((unused)),
                             const VFTR_FLD_RESULT_E evt __attribute__((unused)),
                             const AVFTR_FLD_PARAM_S *param __attribute__((unused)))
{
	return;
}

/**
 * @brief Get enable status of fall detection.
 * @param[in]  idx          video window index.
 * @param[in]  vftr_fld_ctx video fall detection content.
 * @see none.
 * @retval enable status of fall detection.
 */
int AVFTR_FLD_getStat(const MPI_WIN idx, AVFTR_FLD_CTX_S *vftr_fld_ctx)
{
	const int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);
	return enable_idx < 0 ? 0 : vftr_fld_ctx[enable_idx].en;
}

/**
 * @brief Get result of fall detection.
 * @param[in]  idx          video window index.
 * @param[in]  obj_list     object list.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          FLD object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_FLD_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	const int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);
	int ret = 0;

	if (enable_idx < 0 || !vftr_fld_ctx[enable_idx].en) {
		return 0;
	}

	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("FLD instance is NULL!");
		return -EFAULT;
	}

	AVFTR_FLD_STATUS_S *fld_result_shm = &vftr_fld_ctx[enable_idx].fld_res[buf_idx];
	VFTR_FLD_STATUS_S *fld_status = &ctx->fld_res;

	AVFTR_FLD_PARAM_S *param = &ctx->param;

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_FLD_detect(ctx->instance, &obj_list->basic_list, fld_status);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Failed to run FLD!");
		return ret;
	}
	determineFldRes(fld_status, &obj_list->basic_list, fld_result_shm);

	pthread_mutex_lock(&ctx->cb_lock);
	genFldAlarm(&vftr_fld_ctx[enable_idx], fld_status->result, param);
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Translate result of fall detection.
 * @param[in]  vftr_fld_ctx     video fall detection result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted FLD result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_FLD_getRes()
 * @retval length of fall detection result.
 */
// This function should be invoked by AVFTR client routine only.
int AVFTR_FLD_transRes(AVFTR_FLD_CTX_S *vftr_fld_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                       const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                       const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findFldCtx(src_idx, vftr_fld_ctx, NULL);
	if (enable_idx < 0 || !vftr_fld_ctx[enable_idx].en) {
		return 0;
	}

	return getFldMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                  &vftr_fld_ctx[enable_idx].fld_res[buf_idx], str);
}

/**
 * @brief Add fall detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_FLD_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create FLD instance.
 * @retval -ENOMEM          No more space to register idx / malloc FLD instance failed.
 */
int AVFTR_FLD_addInstance(const MPI_WIN idx)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int empty_idx;
	int set_idx = findFldCtx(idx, vftr_fld_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("FLD is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered*/
		FLD_CTX_S *ctx = FLD_GET_CTX(empty_idx);
		ctx->instance = VFTR_FLD_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create FLD instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_fld_ctx[empty_idx].idx = idx;
		vftr_fld_ctx[empty_idx].reg = 1;
		vftr_fld_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		vftr_fld_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);

	} else {
		/* No more space to register idx */
		avftr_log_err("Add FLD instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete fall detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_FLD_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_FLD_deleteInstance(const MPI_WIN idx)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("FLD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_fld_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("FLD is still enable on win(%u, %u, %u), can not be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_FLD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free fld instance failed!");
		return ret;
	}
	vftr_fld_ctx[enable_idx].reg = 0;
	vftr_fld_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_fld_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Enable fall detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_FLD_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_FLD_enable(const MPI_WIN idx)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("FLD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_fld_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("FLD is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);

	INT32 ret = 0;
	ret = VIDEO_FTR_enableOd_implicit(idx);
	if (ret != 0) {
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	if (vftr_fld_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("FLD alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_fld_ctx[enable_idx].cb = emptyFldCallback;
	}
	pthread_mutex_unlock(&ctx->cb_lock);

	vftr_fld_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable fall detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_FLD_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_FLD_disable(const MPI_WIN idx)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("FLD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_fld_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("FLD is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_disableOd_implicit(idx);
	if (ret != 0) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}
	vftr_fld_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of fall detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video fall detection parameters.
 * @see AVFTR_FLD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_FLD_getParam(const MPI_WIN idx, AVFTR_FLD_PARAM_S *param)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("FLD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);
	int ret;

	ret = checkMpiDevValid(idx);
	if (ret != 0) {
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	param->fld_param = ctx->param.fld_param;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief set parameters of fall detection.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_FLD_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video fall detection parameters.
 * @see AVFTR_FLD_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_FLD_setParam(const MPI_WIN idx, const AVFTR_FLD_PARAM_S *param)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("FLD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);

	int ret;

	ret = VFTR_FLD_checkParam(&param->fld_param);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to vftr_fld
	pthread_mutex_lock(&ctx->lock);
	ctx->param.fld_param = param->fld_param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of fall detection instance.
 * @param[in] idx           video window index.
 * @see AVFTR_FLD_getParam()
 * @see AVFTR_FLD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_FLD_writeParam(const MPI_WIN idx)
{
	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	const int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);
	AVFTR_FLD_PARAM_S *param = &ctx->param;

	int ret = 0;

	pthread_mutex_lock(&ctx->lock);
	if (ctx->s_flag == 1) {
		ret = VFTR_FLD_setParam(ctx->instance, &param->fld_param);
		ctx->s_flag = 0;
	}
	pthread_mutex_unlock(&ctx->lock);

	return ret;
}

/**
 * @brief Register alarm callback function of fall detection.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none.
 * @retval 0                   success.
 * @retval -EFAULT             NULL pointer of cb function.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_FLD_regCallback(const MPI_WIN idx, const AVFTR_FLD_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to FLD event callback function should not be NULL.");
		return -EFAULT;
	}

	AVFTR_FLD_CTX_S *vftr_fld_ctx = vftr_res_shm->fld_ctx;
	int enable_idx = findFldCtx(idx, vftr_fld_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("FLD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	FLD_CTX_S *ctx = FLD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_fld_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}
