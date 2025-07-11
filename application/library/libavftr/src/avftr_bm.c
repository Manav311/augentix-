#include "avftr_bm.h"

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdint.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "vftr_fgd.h"

#include "avftr_log.h"
#include "avftr_common.h"
#include "avftr.h"

typedef struct {
	AVFTR_BM_STATUS_S bm_res;
	AVFTR_BM_PARAM_S param;
	VFTR_FGD_INSTANCE_S *instance;
	UINT8 y_avg_cfg_idx;
	MPI_ISP_Y_AVG y_avg;
	int s_flag;
	int is_write;
	pthread_mutex_t mutex;
} BM_CTX_S;

static BM_CTX_S g_bm_ctx[VIDEO_BM_MAX_SUPPORT_NUM] = {
	[0 ... VIDEO_BM_MAX_SUPPORT_NUM - 1] = {
		.mutex = PTHREAD_MUTEX_INITIALIZER
	}
};
#define BM_GET_CTX(idx) (&g_bm_ctx[idx])

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static int findBmCtx(MPI_WIN idx, const AVFTR_BM_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_BM_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].reg) {
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

// This function is duplicated by bm_demo.
// It'd better to follow bm_demo's implement.
static void determineBmRes(const VFTR_FGD_STATUS_S *fd_res, const AVFTR_BM_PARAM_S *param, AVFTR_BM_STATUS_S *bm_res)
{

#define BM_ACTIVE_TH (8)
#define BM_AWAKE_TH (4)
	const MPI_RECT_POINT_S *roi = &param->fgd_param.roi;

	bm_res->fgd_stat = *fd_res;
	bm_res->roi = *roi;
	switch (fd_res->event) {
		case VFTR_FGD_OBJECT_ABSENT:
			bm_res->current_event = AVFTR_BM_EVENT_ABSENT;
			break;
		case VFTR_FGD_OBJECT_BOUNDARY:
			bm_res->current_event = AVFTR_BM_EVENT_BOUNDARY;
			bm_res->duration_active++;
			break;
		case VFTR_FGD_OBJECT_ENTERING:
			bm_res->current_event = AVFTR_BM_EVENT_ENTERING;
			break;
		case VFTR_FGD_OBJECT_LEAVING:
			bm_res->current_event = AVFTR_BM_EVENT_LEAVING;
			break;
		case VFTR_FGD_OBJECT_PRESENT:
			if (fd_res->motion_level >= BM_ACTIVE_TH) {
				bm_res->current_event = AVFTR_BM_EVENT_ACTIVE;
				bm_res->duration_active++;
			} else if (fd_res->motion_level >= BM_AWAKE_TH) {
				bm_res->current_event = AVFTR_BM_EVENT_AWAKE;
				bm_res->duration_awake++;
			} else {
				bm_res->current_event = AVFTR_BM_EVENT_SLEEP;
				bm_res->duration_sleep++;
			}
			break;
		default:
			assert(0);
	}

	return;
}

static int getBmMeta(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                     const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const AVFTR_BM_STATUS_S *bm_res, char *str)
{
	int offset = 0;
	MPI_RECT_POINT_S copy_roi = bm_res->roi;

	if (src_idx.value != dst_idx.value) {
		rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &copy_roi);
	}

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "<BM>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "\"bm\":{");
#endif /* !IVA_FORMAT_XML */
	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<ROI RECT=\"%d %d %d %d\" EVT=\"%d\"/>"
                          "<REPORT ACTIVE=\"%d\" AWAKE=\"%d\" SLEEP=\"%d\"/>"
#ifdef BM_SHOW_VFTR_FGD_REPORT
                          "<FD MOT_LVL=\"%d\" FG=\"%d\" BND_EVT=\"%d\"/>"
#endif /* BM_SHOW_VFTR_FGD_REPORT */
#else /* IVA_FORMAT_JSON */
		                  "\"roi\":{\"rect\":[%d,%d,%d,%d],\"evt\":%d},"
                          "\"report\":{\"active\":%d,\"awake\":%d,\"sleep\":%d}"
#ifdef BM_SHOW_VFTR_FGD_REPORT
                          ",\"fd\":{\"mot_lvl\":%d,\"fg\":%d,\"bnd_evt\":%d}"
#endif /* BM_SHOW_VFTR_FGD_REPORT */
#endif /* !IVA_FORMAT_XML */
		                 ,copy_roi.sx, copy_roi.sy, copy_roi.ex, copy_roi.ey, bm_res->current_event,
		                 bm_res->duration_active, bm_res->duration_awake, bm_res->duration_sleep
#ifdef BM_SHOW_VFTR_FGD_REPORT
                         ,bm_res->fgd_stat.motion_level, bm_res->fgd_stat.fg_object, bm_res->fgd_stat.boundary_event
#endif /* BM_SHOW_VFTR_FGD_REPORT */
                         );
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "</BM>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "},");
#endif /* !IVA_FORMAT_XML */
	return offset;
}


/**
 * @brief Get enable status of baby monitor.
 * @param[in]  idx         video window index.
 * @param[in]  vftr_bm_ctx video baby monitor control.
 * @see none
 * @retval enable status of baby monitor.
 */
int AVFTR_BM_getStat(MPI_WIN idx, const AVFTR_BM_CTX_S *vftr_bm_ctx)
{
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	return enable_idx < 0 ? 0 : vftr_bm_ctx[enable_idx].en;
}

// This function should be invoked by AVFTR server routine only.
int AVFTR_BM_getRes(MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, int buf_idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	int ret;

	if (enable_idx < 0 || !vftr_bm_ctx[enable_idx].en) {
		return 0;
	}

	BM_CTX_S *ctx = BM_GET_CTX(enable_idx);

	if (ctx->instance == NULL) {
		avftr_log_err("BM instance is NULL!");
		return -EFAULT;
	}

	AVFTR_BM_STATUS_S *bm_result_shm = &vftr_bm_ctx[enable_idx].bm_res[buf_idx];
	AVFTR_BM_STATUS_S *bm_result = &ctx->bm_res;
	AVFTR_BM_PARAM_S *param = &ctx->param;
	VFTR_FGD_INPUT_S fd_input = { .y_avg = ctx->y_avg, .obj_list = &obj_list->basic_list };
	VFTR_FGD_STATUS_S fd_result = { 0 };

	pthread_mutex_lock(&ctx->mutex);
	ret = VFTR_FGD_detect(ctx->instance, &fd_input, &fd_result);
	pthread_mutex_unlock(&ctx->mutex);

	if (ret) {
		avftr_log_err("Failed to get foreground detection result.");
		return ret;
	}

	determineBmRes(&fd_result, param, bm_result);
	bm_result_shm->fgd_stat = fd_result;
	*bm_result_shm = *bm_result;

	return 0;
}

// This function should be invoked by AVFTR client routine only.
int AVFTR_BM_transRes(AVFTR_BM_CTX_S *vftr_bm_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                      const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str,
                      int buf_idx)
{
	const int enable_idx = findBmCtx(src_idx, vftr_bm_ctx, NULL);
	if (enable_idx < 0 || !vftr_bm_ctx[enable_idx].en) {
		return 0;
	}

	return getBmMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                 &vftr_bm_ctx[enable_idx].bm_res[buf_idx], str);
}

int AVFTR_BM_addInstance(MPI_WIN idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	BM_CTX_S *ctx;
	int empty_idx;
	const int set_id = findBmCtx(idx, vftr_bm_ctx, &empty_idx);

	// Avoid instance double construction.
	if (set_id >= 0) {
		avftr_log_warn("BM is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	}

	// The slots are all in-use.
	if (empty_idx < 0) {
		avftr_log_err("Failed to create BM instance of win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	ctx = BM_GET_CTX(empty_idx);
	ctx->instance = VFTR_FGD_newInstance();
	if (!ctx->instance) {
		return -ENOMEM;
	}

	vftr_bm_ctx[empty_idx].idx = idx;
	vftr_bm_ctx[empty_idx].reg = 1;
	vftr_bm_ctx[empty_idx].en = 0;

	return 0;
}

int AVFTR_BM_deleteInstance(MPI_WIN idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	int ret;

	if (enable_idx) {
		return 0;
	}

	if (vftr_bm_ctx[enable_idx].en) {
		avftr_log_err("BM is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	BM_CTX_S *ctx = BM_GET_CTX(enable_idx);
	pthread_mutex_lock(&ctx->mutex);
	ret = VFTR_FGD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->mutex);

	if (ret != 0) {
		return ret;
	}

	vftr_bm_ctx[enable_idx].reg = 0;
	vftr_bm_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Enable baby monitor.
 * @param[in] idx    video window index.
 * @retval 0            success.
 * @retval otherwise    unexpected fail.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_BM_enable(MPI_WIN idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	BM_CTX_S *ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	int ret;

	if (enable_idx < 0) {
		avftr_log_err("Baby monitor is not registered on WIN(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_bm_ctx[enable_idx].en) {
		avftr_log_warn("Baby monitor is already enabled on WIN(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	}

	if ((ret = vftrYAvgResDec())) {
		return -1;
	}

	if ((ret = VIDEO_FTR_enableOd_implicit(idx))) {
		vftrYAvgResInc();
		return -1;
	}

	ctx = BM_GET_CTX(enable_idx);
	MPI_ISP_Y_AVG_CFG_S y_avg_cfg = (MPI_ISP_Y_AVG_CFG_S){
		.roi = ctx->param.fgd_param.roi, .diff_thr = 0
	};

	pthread_mutex_lock(&ctx->mutex);
	ret = MPI_DEV_addIspYAvgCfg(idx, &y_avg_cfg, &ctx->y_avg_cfg_idx);
	pthread_mutex_unlock(&ctx->mutex);

	if (ret != MPI_SUCCESS) {
		avftr_log_err("Failed to add Y avg configuration on win %u. err: %d", idx.value, ret);
		return ret;
	}

	vftr_bm_ctx[enable_idx].en = 1;
	vftr_bm_ctx[enable_idx].resource_registered = 1;

	return 0;
}

/**
 * @brief Disable baby monitor.
 * @details
 * @param[in] idx    video window index.
 * @see VIDEO_FTR_enableTd
 * @retval 0     success.
 * @retval -1     unexpected fail.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_BM_disable(MPI_WIN idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	BM_CTX_S *ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	int ret;

	if (enable_idx < 0) {
		avftr_log_err("BM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_bm_ctx[enable_idx].en) {
		// av_main2 invokes this function in the starting flow.
		// avftr_log_warn("Baby monitor is already disabled on WIN(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	}

	vftrYAvgResInc();

	ctx = BM_GET_CTX(enable_idx);
	pthread_mutex_lock(&ctx->mutex);
	ret = MPI_DEV_rmIspYAvgCfg(idx, ctx->y_avg_cfg_idx);
	pthread_mutex_unlock(&ctx->mutex);

	if (ret != MPI_SUCCESS) {
		avftr_log_err("Failed to release Y avg config on win(%u, %u, %u). err: %d", idx.dev, idx.chn, idx.win, ret);
		return ret;
	}

	if ((ret = VIDEO_FTR_disableOd_implicit(idx))) {
		return ret;
	}

	vftr_bm_ctx[enable_idx].en = 0;
	vftr_bm_ctx[enable_idx].resource_registered = 0;

	return 0;
}

/**
 * @brief Get BM parameters from static memory.
 * @param[in]  idx      Video window index.
 * @param[out] param    Pointer to parameter.
 * @return The execution result.
 * @retval 0          Success.
 * @retval -ENODEV    Baby monitor instance is not created yet.
 */
int AVFTR_BM_getParam(MPI_WIN idx, AVFTR_BM_PARAM_S *param)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);

	if (enable_idx < 0) {
		avftr_log_err("BM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	*param = BM_GET_CTX(enable_idx)->param;
	return 0;
}

/**
 * @brief Set BM parameters.
 * @param[in] idx       Video window index.
 * @param[in] param     Baby monitor parameters.
 * @retval 0            Success.
 * @retval otherwise    Unexpected error
 */
int AVFTR_BM_setParam(MPI_WIN idx, const AVFTR_BM_PARAM_S *param)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	VFTR_FGD_PARAM_S fgd_param;
	BM_CTX_S *ctx;

	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	int ret;

	if (enable_idx < 0) {
		avftr_log_err("BM is not registered on win(%u, %u, %u) yet.", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = BM_GET_CTX(enable_idx);

	// Clone parameter and hardcode event type as `Object monitor`
	fgd_param = param->fgd_param;
	fgd_param.event_type = VFTR_FGD_OBJECT_MONITOR;
	if ((ret = VFTR_FGD_checkParam(&fgd_param))) {
		return ret;
	}

	pthread_mutex_lock(&ctx->mutex);
	ctx->param.fgd_param = fgd_param;
	ctx->is_write = 1;
	pthread_mutex_unlock(&ctx->mutex);

	return 0;
}

// Assumes that this function called by thread only.
// We does not deal with race condition yet.
int AVFTR_BM_writeParam(MPI_WIN idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);
	int ret;

	if (enable_idx < 0) {
		return -ENODEV;
	}

	BM_CTX_S *ctx = BM_GET_CTX(enable_idx);
	if (!ctx->is_write) {
		return 0;
	}

	ctx->is_write = 0;
	MPI_ISP_Y_AVG_CFG_S y_avg_cfg = {
		.roi = ctx->param.fgd_param.roi, .diff_thr = 0,
	};

	ret = VFTR_FGD_setParam(ctx->instance, &ctx->param.fgd_param);
	if (ret) {
		avftr_log_err("Write BM parameters failed. err: %d", ret);
		return ret;
	}

	ret = MPI_DEV_rmIspYAvgCfg(idx, ctx->y_avg_cfg_idx);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("BM failed to remove Y avg config. err: %d", ret);
		return ret;
	}

	ret = MPI_DEV_addIspYAvgCfg(idx, &y_avg_cfg, &ctx->y_avg_cfg_idx);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("BM failed to add Y avg config. err: %d", ret);
		return ret;
	}

	return ret;
}

// Reset counter
int AVFTR_BM_resetData(MPI_WIN idx __attribute__((unused)))
{
	// AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	// const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);

	// if (enable_idx < 0) {
	// 	return 0;
	// }

	// BM_CTX_S *ctx = BM_GET_CTX(enable_idx);

	// pthread_mutex_lock(&ctx->mutex);
	// ctx->bm_res.duration_active = 0;
	// ctx->bm_res.duration_awake = 0;
	// ctx->bm_res.duration_sleep = 0;
	// ctx->bm_res.current_event = 0;
	// pthread_mutex_unlock(&ctx->mutex);

	return 0;
}

int AVFTR_BM_regMpiInfo(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

int AVFTR_BM_releaseMpiInfo(MPI_WIN idx __attribute__((unused)))
{
	return 0;
}

// Store Y-average to static memory
// This function should be invoked by AVFTR server routine only.
int AVFTR_BM_updateMpiInfo(MPI_WIN idx)
{
	AVFTR_BM_CTX_S *vftr_bm_ctx = vftr_res_shm->bm_ctx;
	const int enable_idx = findBmCtx(idx, vftr_bm_ctx, NULL);

	if (enable_idx < 0 || !vftr_bm_ctx[enable_idx].en) {
		return 0;
	}

	BM_CTX_S *ctx = BM_GET_CTX(enable_idx);
	int ret;

	if (!vftr_bm_ctx[enable_idx].resource_registered) {
		avftr_log_err("MPI resources for BM has not been registered yet.");
		return -ENODEV;
	}

	ret = MPI_DEV_getIspYAvg(idx, ctx->y_avg_cfg_idx, &ctx->y_avg);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get Y average on win (%u, %u, %u) failed. err: %d", idx.dev, idx.chn, idx.win, ret);
		return ret;
	}

	return 0;
}

/**
 * @brief Reset share memory of baby monitor.
 * @param[in]  idx             video window index.
 * @see none
 * @retval 0          success.
  */
__attribute__ ((deprecated)) int VIDEO_FTR_resetBmShm(MPI_WIN idx __attribute__((unused)))
{
	// Not decided what action does this API should do yet.
	return 0;
}

__attribute__ ((deprecated)) int VIDEO_FTR_ctrlBmData(MPI_WIN idx __attribute__((unused)), const char *data_path __attribute__((unused)),
                         AVFTR_BM_DATA_CTRL_E ctrl __attribute__((unused)))
{
	// Not decided what action does this API should do yet.
	return 0;
}
