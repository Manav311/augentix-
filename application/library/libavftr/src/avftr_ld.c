#include "avftr_ld.h"

#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "avftr_log.h"
#include "mpi_dev.h"

#include "avftr.h"
#include "avftr_common.h"

typedef struct {
	VFTR_LD_PARAM_S param;
	VFTR_LD_INSTANCE_S *instance;
	unsigned int is_write;
	unsigned int is_reset;
	unsigned int is_setRoi;

	UINT8 mcvp_avg_y_cfg_idx;

	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} LD_CTX_S;

static LD_CTX_S g_ld_ctx[AVFTR_LD_MAX_SUPPORT_NUM] = { { { 0 } } };

#define LD_GET_CTX(idx) &g_ld_ctx[idx]

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video light detection content.
 * @param[out] empty        global light detection index.
 * @see AVFTR_LD_addInstance()
 * @see AVFTR_LD_deleteInstance()
 * @retval enable index.
 */
static int findLdCtx(const MPI_WIN idx, const AVFTR_LD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_LD_MAX_SUPPORT_NUM; i++) {
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
 * @brief Invoke callback function when alarm condition is satisfied.
 * @param[in] vftr_ld_ctx   video light detection content.
 * @param[in] trig_cond     video light detection alarm trigger condition.
 * @see AVFTR_LD_getRes()
 * @retval none.
 */
static void genLdAlarm(const AVFTR_LD_CTX_S *vftr_ld_ctx, const VFTR_LD_COND_E trig_cond)
{
	if (vftr_ld_ctx->cb == NULL) {
		return;
	}

	if (trig_cond != VFTR_LD_LIGHT_NONE) {
		vftr_ld_ctx->cb();
		return;
	}
}

static void getLdResOffset(const MPI_WIN idx, AVFTR_RECT_POINT_S *roi)
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint8_t i;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	int ret;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Cannot get channel layout for chn:%d", chn.chn);
		return;
	}
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			break;
		}
	}
	if (i == layout_attr.window_num) {
		avftr_log_err("Window %d does not exist in channel %d", idx.win, idx.chn);
		return;
	}

	x = layout_attr.window[i].x;
	y = layout_attr.window[i].y;
	roi->sx += x;
	roi->sy += y;
	roi->ex += x;
	roi->ey += y;

	return;
}

/**
 * @brief Get predefined formatted LD result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  ld_stat      video light detection result.
 * @param[out] str          formatted LD result string buffer.
 * @see AVFTR_LD_getRes()
 * @retval length of formatted LD result.
 */
static int getLdMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                     const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                     const AVFTR_LD_STATUS_S *ld_stat, char *str)
{
	int offset = 0;

	AVFTR_RECT_POINT_S roi = ld_stat->roi;

	if (src_idx.value != dst_idx.value) {
		rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &roi);
	}

	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML
	                  "<LD><ROI RECT=\"%d %d %d %d\" ALARM=\"%d\"/></LD>"
#else /* IVA_FORMAT_JSON */
	                  "\"ld\":{\"roi\":{\"rect\":[%d,%d,%d,%d],\"alarm\":%d}},"
#endif /*! !IVA_FORMAT_XML */
	                  ,
	                  roi.sx, roi.sy, roi.ex, roi.ey, ld_stat->status.trig_cond);
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] none.
 * @see AVFTR_LD_enable()
 * @retval none.
 */
static void alarmEmptyCb()
{
	return;
}

/**
 * @brief Get enable status of light detection.
 * @param[in]  idx          video window index.
 * @param[in]  vftr_ld_ctx  video light detection content.
 * @see none.
 * @retval enable status of light detection.
 */
int AVFTR_LD_getStat(const MPI_WIN idx, AVFTR_LD_CTX_S *vftr_ld_ctx)
{
	const int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_ld_ctx[enable_idx].en;
}

/**
 * @brief Get result of light detection.
 * @param[in]  idx          video window index.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          LD object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_LD_getRes(const MPI_WIN idx, const int buf_idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	const int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	int ret = 0;

	if (enable_idx < 0 || !vftr_ld_ctx[enable_idx].en) {
		return 0;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("LD object is NULL!");
		return -EFAULT;
	}

	VFTR_LD_STATUS_S *ld_status = &vftr_ld_ctx[enable_idx].ld_res[buf_idx].status;

	VFTR_LD_PARAM_S param;
	ret = AVFTR_LD_getParam(idx, &param);
	if (ret != 0) {
		avftr_log_err("Get light detection ROI on win %d failed.", idx.win);
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_LD_detect(ctx->instance, ctx->mcvp_avg_y_cfg_idx, ld_status);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Failed to run light detection! err: %d", ret);
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	genLdAlarm(&vftr_ld_ctx[enable_idx], ld_status->trig_cond);
	pthread_mutex_unlock(&ctx->cb_lock);

	getLdResOffset(idx, &param.roi);

	return 0;
}

/**
 * @brief Translate result of light detection.
 * @param[in]  vftr_ld_ctx      video light detection result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted LD result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_LD_getRes()
 * @retval length of light detection result.
 */
int AVFTR_LD_transRes(AVFTR_LD_CTX_S *vftr_ld_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findLdCtx(src_idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0 || !vftr_ld_ctx[enable_idx].en)
		return 0;

	return getLdMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                 &vftr_ld_ctx[enable_idx].ld_res[buf_idx], str);
}

/**
 * @brief Add light detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_LD_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create LD instance.
 * @retval -ENOMEM          No more space to register idx / malloc LD instance failed.
 */
int AVFTR_LD_addInstance(const MPI_WIN idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int empty_idx;
	int set_idx = findLdCtx(idx, vftr_ld_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("LD is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered*/
		LD_CTX_S *ctx = LD_GET_CTX(empty_idx);
		ctx->instance = VFTR_LD_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create LD instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		ctx->cb_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_ld_ctx[empty_idx].idx = idx;
		vftr_ld_ctx[empty_idx].reg = 1;
		vftr_ld_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		vftr_ld_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);

	} else {
		/* No more space to register idx */
		avftr_log_err("Add LD instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete light detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_LD_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_LD_deleteInstance(const MPI_WIN idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_ld_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("LD is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_LD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free light detection instance failed! err: %d", ret);
		return ret;
	}
	vftr_ld_ctx[enable_idx].reg = 0;
	vftr_ld_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_ld_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);
	return 0;
}

/**
 * @brief Enable light detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_LD_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_LD_enable(const MPI_WIN idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_ld_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("LD is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);

	INT32 ret = 0;
	ret = vftrYAvgResDec();
	if (ret != 0) {
		avftr_log_err("All MPI Yavg ROI Resource are being used!");
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	if (vftr_ld_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("LD alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_ld_ctx[enable_idx].cb = alarmEmptyCb;
	}
	pthread_mutex_unlock(&ctx->cb_lock);

	vftr_ld_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable light detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_LD_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_LD_disable(const MPI_WIN idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_ld_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("LD is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	vftrYAvgResInc();
	vftr_ld_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of light detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video light detection parameters.
 * @see AVFTR_LD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_LD_getParam(const MPI_WIN idx, VFTR_LD_PARAM_S *param)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);
	int ret;

	// Use this to check win state
	MPI_WIN_ATTR_S win_attr;
	ret = MPI_DEV_getWindowAttr(idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get window %u attributes failed. err: %d", idx.value, ret);
		return ret;
	}
	*param = ctx->param;

	return 0;
}

/**
 * @brief set parameters of light detection.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_LD_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video light detection parameters.
 * @see AVFTR_LD_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_LD_setParam(const MPI_WIN idx, const VFTR_LD_PARAM_S *param)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);

	int ret;

	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	ret = VFTR_LD_checkParam(&res, param);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to vftr_ld
	pthread_mutex_lock(&ctx->lock);
	ctx->param = *param;
	ctx->is_write = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of light detection instance.
 * @param[in] idx           video window index.
 * @see AVFTR_LD_getParam()
 * @see AVFTR_LD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_LD_writeParam(const MPI_WIN idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	const int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);
	VFTR_LD_PARAM_S *param = &ctx->param;

	int ret;
	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	// Use this to check win state
	MPI_WIN_ATTR_S win_attr;
	ret = MPI_DEV_getWindowAttr(idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get window %u attributes failed. err: %d", idx.value, ret);
		return ret;
	}

	int is_write = ctx->is_write;
	if (is_write == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_LD_setParam(ctx->instance, &res, param);
		ctx->is_write = 0;
		// avftr_log_err("[reader] write LD param success, sensitivity is %d", param->ld_param.sensitivity);
		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Register alarm callback function of light detection.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none.
 * @retval 0                   success.
 * @retval -EFAULT             NULL pointer of cb function.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_LD_regCallback(const MPI_WIN idx, const AVFTR_LD_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to light detection alarm callback function should not be NULL.");
		return -EFAULT;
	}

	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_ld_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Reset results of light detection.
 * @param[in]  idx             video window index.
 * @see none.
 * @retval execution result.
 */
int AVFTR_LD_resetShm(const MPI_WIN idx __attribute__((unused)))
{
	/*  FIXME: by win used
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);

	if (enable_idx < 0) {
		return -1;
	}

	if (vftr_ld_ctx[enable_idx].en) {
		vftr_ld_ctx[enable_idx].ld_res.alarm = 0;
	}
	*/
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	AVFTR_VIDEO_BUF_INFO_S *buf_info = vftr_res_shm->buf_info;
	int i;

	for (i = 0; i < AVFTR_LD_MAX_SUPPORT_NUM; i++) {
		if (vftr_ld_ctx[i].en) {
			vftr_ld_ctx[i].ld_res[buf_info[i].buf_cur_idx].status.trig_cond = VFTR_LD_LIGHT_NONE;
		}
	}

	return 0;
}

int AVFTR_LD_resume(const MPI_WIN idx __attribute__((unused)))
{
#if (0) /* Do nothing */
	int i;
	for (i = 0; i < AVFTR_LD_MAX_SUPPORT_NUM; i++) {
		if (vftr_ld_ctx[i].en) {
			// do nothing now
		}
	}
#endif
	return 0;
}

int AVFTR_LD_regMpiInfo(const MPI_WIN idx, const MPI_RECT_POINT_S *roi)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);

	int ret;

	if (ctx->is_setRoi == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = MPI_DEV_rmIspYAvgCfg(idx, ctx->mcvp_avg_y_cfg_idx);
		pthread_mutex_unlock(&ctx->lock);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Release y avg configuration %d on win %u failed.", ctx->mcvp_avg_y_cfg_idx,
			              idx.value);
			return -ENODEV;
		}
		ctx->is_setRoi = 0;
	}

	MPI_ISP_Y_AVG_CFG_S y_avg_cfg = { .roi = *roi, .diff_thr = 0 };

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspYAvgCfg(idx, &y_avg_cfg, &ctx->mcvp_avg_y_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != MPI_SUCCESS) {
		avftr_log_err("Add luma avg configuration failed on win %u. err: %d", idx.value, ret);
		return ret;
	}

	ctx->is_setRoi = 1;
	return 0;
}

int AVFTR_LD_releaseMpiInfo(const MPI_WIN idx)
{
	AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	int enable_idx = findLdCtx(idx, vftr_ld_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_ld_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("LD is still enable on win(%u, %u, %u), ROI can not be deleted!", idx.dev, idx.chn,
		              idx.win);
		return -EAGAIN;
	}

	LD_CTX_S *ctx = LD_GET_CTX(enable_idx);

	int ret;

	if (ctx->is_setRoi == 0) {
		avftr_log_err("ROI config is not set yet.");
		return 0;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_rmIspYAvgCfg(idx, ctx->mcvp_avg_y_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Release y avg configuration %d on win %u failed", ctx->mcvp_avg_y_cfg_idx, idx.value);
		return -ENODEV;
	}

	ctx->is_setRoi = 0;
	return 0;
}

int AVFTR_LD_updateMpiInfo(const MPI_WIN idx __attribute__((unused)))
{
	// AVFTR_LD_CTX_S *vftr_ld_ctx = vftr_res_shm->ld_ctx;
	// int enable_idx = findBmCtx(idx, vftr_ld_ctx, NULL);
	// if (enable_idx < 0) {
	// 	/* idx is not registered yet */
	// 	avftr_log_err("LD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
	// 	return -ENODEV;
	// }

	// if (!vftr_ld_ctx[enable_idx].en) {
	// 	/* idx is not enabled */
	// 	avftr_log_err("idx:%d is not enabled yet, no need to get mpi info", idx.value);
	// 	return 0;
	// }

	// LD_CTX_S *ctx = LD_GET_CTX(enable_idx);
	// INT32 ret = 0;

	// ret = MPI_DEV_getIspYAvg(idx, ctx->y_avg_cfg_idx, &ctx->mpi_input.y_shp_avg);
	// if (ret != 0) {
	// 	avftr_log_err("Get luma avg on win %u failed", idx.value);
	// 	return ret;
	// }

	return 0;
}
