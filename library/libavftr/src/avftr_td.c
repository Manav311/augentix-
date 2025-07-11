#include "avftr_td.h"

#include "mpi_base_types.h"
#include <pthread.h>
#include <errno.h>
#include <stdio.h>

#include "avftr_log.h"
#include "mpi_dev.h"

#include "avftr.h"
#include "avftr_common.h"

typedef struct {
	AVFTR_TD_PARAM_S param;
	VFTR_TD_MPI_INPUT_S mpi_input;
	VFTR_TD_INSTANCE_S *instance;
	unsigned int is_write;
	unsigned int is_reset;

	MPI_PATH path;
	UINT8 mv_hist_cfg_idx;
	UINT8 var_cfg_idx;

	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} TD_CTX_S;

static TD_CTX_S g_td_ctx[AVFTR_TD_MAX_SUPPORT_NUM] = { { { { 0 } } } };

#define TD_GET_CTX(idx) &g_td_ctx[idx]

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video tamper detection content.
 * @param[out] empty        global tamper detection index.
 * @see AVFTR_TD_addInstance()
 * @see AVFTR_TD_deleteInstance()
 * @retval enable index.
 */
static int findTdCtx(const MPI_WIN idx, const AVFTR_TD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_TD_MAX_SUPPORT_NUM; i++) {
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
 * @brief Invoke callback function when tamper detection alarm condition is satisfied.
 * @param[in] tk_ctx        video tamper detection content.
 * @param[in] tamper_alarm  video tamper detection alarm condition.
 * @see AVFTR_TD_getRes()
 * @retval none.
 */
static void genTdAlarm(const AVFTR_TD_CTX_S *td_ctx, const int tamper_alarm)
{
	if (td_ctx->cb == NULL) {
		return;
	}

	if (tamper_alarm) {
		td_ctx->cb();
		return;
	}
	/*if (tamper_alarm & (1 << TD_ALARM_MULTIPLE)) {
	} else if (tamper_alarm & (1 << TD_ALARM_BLOCK)) {
	} else if (tamper_alarm & (1 << TD_ALARM_DEFOCUSING)) {
	} else if (tamper_alarm & (1 << TD_ALARM_REDIRECTING)) {
	}*/
	return;
}

/**
 * @brief Get predefined formatted tamper detection result string for Multiplayer.
 * @param[in] tamper_alarm  tamper detection result.
 * @param[out] str          formatted tamper detection result string buffer.
 * @see AVFTR_TD_getRes()
 * @retval length of formatted TD result.
 */
static int getTdMeta(const int tamper_alarm, char *str)
{
	int offset = 0;
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "<TD>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "\"td\":{");
#endif /* !IVA_FORMAT_XML */
	if (tamper_alarm & (1 << VFTR_TD_ALARM_BLOCK)) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<TYPE>block</TYPE>");
#else /* IVA_FORMAT_JSON */
		                  "\"type\":\"block tamper\"");
#endif /* !IVA_FORMAT_XML */
	} else if (tamper_alarm & (1 << VFTR_TD_ALARM_REDIRECT)) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<TYPE>redirect</TYPE>");
#else /* IVA_FORMAT_JSON */
		                  "\"type\":\"redirect tamper\"");
#endif /* !IVA_FORMAT_XML */
	} else if (tamper_alarm & (1 << VFTR_TD_ALARM_NUM)) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<TYPE>both</TYPE>");
#else /* IVA_FORMAT_JSON */
		                  "\"type\":\"both tamper\"");
#endif /* !IVA_FORMAT_XML */
	}

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "</TD>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "},");
#endif /* !IVA_FORMAT_XML */
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] none.
 * @see AVFTR_TD_enable()
 * @retval none.
 */
static void alarmEmptyCb()
{
	// avftr_log_err("Please register tamper detection alarm callback function.");
	return;
}

/**
 * @brief Get enable status of tamper detection.
 * @param[in]  idx          video window index.
 * @param[in]  vftr_td_ctx  video tamper detection content.
 * @see none.
 * @retval enable status of tamper detection.
 */
int AVFTR_TD_getStat(const MPI_WIN idx, AVFTR_TD_CTX_S *vftr_td_ctx)
{
	const int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_td_ctx[enable_idx].en;
}

/**
 * @brief Get result of tamper detection.
 * @param[in]  idx          video window index.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          TD object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_TD_getRes(const MPI_WIN idx, const int buf_idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	const int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	int ret = 0;

	if (enable_idx < 0 || !vftr_td_ctx[enable_idx].en) {
		return 0;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("TD object is NULL!");
		return -EFAULT;
	}

	VFTR_TD_STATUS_S *td_status = &vftr_td_ctx[enable_idx].td_res[buf_idx];

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_TD_detect(ctx->instance, &ctx->mpi_input, td_status);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Failed to run tamper detection!");
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	genTdAlarm(&vftr_td_ctx[enable_idx], td_status->alarm);
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Translate result of tamper detection.
 * @param[in]  vftr_td_ctx      video tamper detection result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted TD result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_TD_getRes()
 * @retval length of tamper detection result.
 */
int AVFTR_TD_transRes(AVFTR_TD_CTX_S *vftr_td_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx __attribute__((unused)),
                      const MPI_RECT_S *src_rect __attribute__((unused)),
                      const MPI_RECT_S *dst_rect __attribute__((unused)),
                      const MPI_RECT_S *src_roi __attribute__((unused)),
                      const MPI_RECT_S *dst_roi __attribute__((unused)), char *str, const int buf_idx)
{
	const int enable_idx = findTdCtx(src_idx, vftr_td_ctx, NULL);
	if (enable_idx < 0 || !vftr_td_ctx[enable_idx].en) {
		return 0;
	}

	return getTdMeta(vftr_td_ctx[enable_idx].td_res[buf_idx].alarm, str);
}

/**
 * @brief Add tamper detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_TD_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create TD instance.
 * @retval -ENOMEM          No more space to register idx / malloc TD instance failed.
 */
int AVFTR_TD_addInstance(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int empty_idx;
	int set_idx = findTdCtx(idx, vftr_td_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("TD is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered*/
		TD_CTX_S *ctx = TD_GET_CTX(empty_idx);
		ctx->instance = VFTR_TD_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create TD instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		ctx->cb_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_td_ctx[empty_idx].idx = idx;
		vftr_td_ctx[empty_idx].reg = 1;
		vftr_td_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		vftr_td_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);

	} else {
		/* No more space to register idx */
		avftr_log_err("Add TD instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete tamper detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_TD_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_TD_deleteInstance(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_td_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("TD is still enable on win(%u, %u, %u), can not be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_TD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free td instance failed!\n");
		return ret;
	}
	vftr_td_ctx[enable_idx].reg = 0;
	vftr_td_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_td_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Enable tamper detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_TD_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_TD_enable(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_td_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("TD is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = vftrYAvgResDec();
	if (ret != 0) {
		avftr_log_err("All MPI YAVG ROI Resource are being used!");
		return ret;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ctx->is_reset = 1;
	//avftr_log_err("[writer] enable TD success");
	pthread_mutex_unlock(&ctx->lock);

	pthread_mutex_lock(&ctx->cb_lock);
	if (vftr_td_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("TD alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_td_ctx[enable_idx].cb = alarmEmptyCb;
	}
	pthread_mutex_unlock(&ctx->cb_lock);

	vftr_td_ctx[enable_idx].en = 1;
	vftr_td_ctx[enable_idx].resource_registered = 0;

	return 0;
}

/**
 * @brief Disable tamper detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_TD_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_TD_disable(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_td_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("TD is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	vftrYAvgResInc();
	vftr_td_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of tamper detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video tamper detection parameters.
 * @see AVFTR_TD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_TD_getParam(const MPI_WIN idx, AVFTR_TD_PARAM_S *param)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);
	int ret;

	// Use this to check win state
	MPI_WIN_ATTR_S win_attr;
	ret = MPI_DEV_getWindowAttr(idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get window %u attributes failed. err: %d", idx.value, ret);
		return ret;
	}
	param->td_param = ctx->param.td_param;

	return 0;
}

/**
 * @brief set parameters of tamper detection.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_TD_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video tamper detection parameters.
 * @see AVFTR_TD_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_TD_setParam(const MPI_WIN idx, const AVFTR_TD_PARAM_S *param)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	int ret;

	ret = VFTR_TD_checkParam(&param->td_param);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to vftr_td
	pthread_mutex_lock(&ctx->lock);
	ctx->param.td_param = param->td_param;
	ctx->is_write = 1;
	//avftr_log_err("[AVFTR_TD] set TD param success");
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of tamper detection instance.
 * @param[in] idx           video window index.
 * @see AVFTR_TD_getParam()
 * @see AVFTR_TD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_TD_writeParam(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	const int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);
	AVFTR_TD_PARAM_S *param = &ctx->param;

	int ret;

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
		ret = VFTR_TD_setParam(ctx->instance, &param->td_param);
		ctx->is_write = 0;

		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}

	/* reset TD internal status */
	int is_reset = ctx->is_reset;
	if (is_reset == 1) {
		/* get input resolution & luma roi for TD */
		MPI_RECT_S roi;
		MPI_ROI_ATTR_S roi_attr;
		MPI_PATH_ATTR_S path_attr;

		ret = MPI_DEV_getPathAttr(ctx->path, &path_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get path attr on window %u, err: %d", idx.value, ret);
			return ret;
		}

		ret = MPI_getRoiAttr(ctx->path, &roi_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get roi attr on window %u, err: %d", idx.value, ret);
			return ret;
		}

		roi.width = (roi_attr.luma_roi.ex - roi_attr.luma_roi.sx) * path_attr.res.width / 1024;
		roi.height = (roi_attr.luma_roi.ey - roi_attr.luma_roi.sy) * path_attr.res.height / 1024;

		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_TD_init(ctx->instance, &roi, (int)win_attr.fps);
		pthread_mutex_unlock(&ctx->lock);

		if (ret != 0) {
			avftr_log_err("Init TD failed! err: %d", ret);
			return ret;
		}

		pthread_mutex_lock(&ctx->lock);
		ctx->is_reset = 0;
		pthread_mutex_unlock(&ctx->lock);
	}

	return 0;
}

/**
 * @brief Register alarm callback function of tamper detection.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none.
 * @retval 0                   success.
 * @retval -EFAULT             NULL pointer of cb function.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_TD_regCallback(const MPI_WIN idx, const AVFTR_TD_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to tamper detection alarm callback function should not be NULL.");
		return -EFAULT;
	}

	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_td_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Reset registered model of tamper detection.
 * @param[in]  idx             video window index.
 * @see none
 * @retval 0                   success.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_TD_reset(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ctx->is_reset = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

int AVFTR_TD_resetShm(const MPI_WIN idx __attribute__((unused)))
{
	/*  FIXME: by win used
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);

	if (enable_idx < 0) {
		return -1;
	}

	if (vftr_td_ctx[enable_idx].en) {
		vftr_td_ctx[enable_idx].td_res.alarm = 0;
	}
	*/
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int i, j;

	for (i = 0; i < AVFTR_TD_MAX_SUPPORT_NUM; i++) {
		if (vftr_td_ctx[i].en) {
			for (j = 0; j < AVFTR_VIDEO_RING_BUF_SIZE; j++)
				vftr_td_ctx[i].td_res[j].alarm = 0;
		}
	}

	return 0;
}

int AVFTR_TD_resume(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;

	/* FIXME: not support by win for notify
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);

	if (enable_idx < 0) {
		return -1;
	}

	if (vftr_td_ctx[enable_idx].en) {
		TD_resetData(vftr_td_ctx[enable_idx].idx);
	}
	*/

	int i;
	for (i = 0; i < AVFTR_TD_MAX_SUPPORT_NUM; i++) {
		if (vftr_td_ctx[i].en) {
			int enable_idx = findTdCtx(vftr_td_ctx[i].idx, vftr_td_ctx, NULL);
			if (enable_idx < 0) {
				/* idx is not registered yet */
				avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn,
				              idx.win);
				return -ENODEV;
			}

			TD_CTX_S *ctx = TD_GET_CTX(enable_idx);
			pthread_mutex_lock(&ctx->lock);
			ctx->is_reset = 1;
			//avftr_log_err("[AVFTR_TD] set reset TD success");
			pthread_mutex_unlock(&ctx->lock);
		}
	}

	return 0;
}

int AVFTR_TD_regMpiInfo(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_td_ctx[enable_idx].resource_registered) {
		avftr_log_err("Resource of TD has been registered.");
		return 0;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	int ret;

	MPI_CHN_LAYOUT_S chn_layout;
	MPI_CHN chn = (MPI_CHN){ .value = idx.value };
	ret = MPI_DEV_getChnLayout(chn, &chn_layout);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get video channel %u layout failed. err: %d", chn.value, ret);
		return -1;
	}
	int i = 0;
	for (i = 0; i < chn_layout.window_num; i++) {
		if (idx.value == chn_layout.win_id[i].value) {
			break;
		}
	}
	UINT32 win_idx = MPI_GET_VIDEO_WIN(idx);
	if (i == chn_layout.window_num) {
		avftr_log_err("Invalid video window index %d from video channel %d", win_idx, idx.chn);
		return -1;
	}

	MPI_RECT_POINT_S roi = { .sx = chn_layout.window[i].x,
		                 .sy = chn_layout.window[i].y,
		                 .ex = (chn_layout.window[i].x + chn_layout.window[i].width - 1),
		                 .ey = (chn_layout.window[i].y + chn_layout.window[i].height - 1) };

	MPI_ISP_MV_HIST_CFG_S mv_hist_cfg = { .roi = roi };
	MPI_ISP_VAR_CFG_S var_cfg = { .roi = roi };

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspMvHistCfg(idx, &mv_hist_cfg, &ctx->mv_hist_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Add Mv hist configuration failed on win %u, err: %d", idx.value, ret);
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspVarCfg(idx, &var_cfg, &ctx->var_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Add variance configuration failed on win %u, err: %d", idx.value, ret);
		return ret;
	}

	UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	MPI_WIN_ATTR_S win_attr;
	ret = MPI_DEV_getWindowAttr(idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get window attributes failed. err: %d", ret);
		return -ENODEV;
	}

	int path_idx;
	if (win_attr.path.bit.path0_en) {
		path_idx = 0;
	} else if (win_attr.path.bit.path1_en) {
		path_idx = 1;
	} else if (win_attr.path.bit.path2_en) {
		path_idx = 2;
	} else if (win_attr.path.bit.path3_en) {
		path_idx = 3;
	} else {
		avftr_log_err("Wrong path bmp %d setting", win_attr.path.bmp);
		return -EINVAL;
	}

	ctx->path = MPI_INPUT_PATH(dev_idx, path_idx);
	vftr_td_ctx[enable_idx].resource_registered = 1;
	return 0;
}

int AVFTR_TD_releaseMpiInfo(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("TD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_td_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("TD is enabled on win(%u, %u, %u), ROI can not be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);

	if (!vftr_td_ctx[enable_idx].resource_registered) {
		//avftr_log_err("Resource of TD has not been registered.");
		return 0;
	}

	int ret = 0;

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_rmIspMvHistCfg(idx, ctx->mv_hist_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Release mv hist configuration %d on win %u failed, err: %d", ctx->mv_hist_cfg_idx, idx.value, ret);
		return -ENODEV;
	}

	// Temporary do not remove var config, otherwise leads video pipeline hanging
	// pthread_mutex_lock(&ctx->lock);
	// ret = MPI_DEV_rmIspVarCfg(idx, ctx->var_cfg_idx);
	// pthread_mutex_unlock(&ctx->lock);
	// if (ret != 0) {
	// 	avftr_log_err("Release var configuration %d on win %u failed", ctx->var_cfg_idx, idx.value);
	// 	return ENODEV;
	// }

	vftr_td_ctx[enable_idx].resource_registered = 0;

	return 0;
}

int AVFTR_TD_updateMpiInfo(const MPI_WIN idx)
{
	AVFTR_TD_CTX_S *vftr_td_ctx = vftr_res_shm->td_ctx;
	const int enable_idx = findTdCtx(idx, vftr_td_ctx, NULL);
	if (enable_idx < 0) {
		return -ENODEV;
	}

	if (!vftr_td_ctx[enable_idx].en) {
		return 0;
	}

	if (!vftr_td_ctx[enable_idx].resource_registered) {
		avftr_log_err("MPI Resource of TD has not been registered yet!");
		return 0;
	}

	TD_CTX_S *ctx = TD_GET_CTX(enable_idx);
	INT32 ret = 0;

	ret = MPI_getStatistics(ctx->path, &ctx->mpi_input.dip_stat);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get statistics on path %d failed. err: %d", ctx->path.path, ret);
		return ret;
	}

	ret = MPI_DEV_getIspMvHist(idx, ctx->mv_hist_cfg_idx, &ctx->mpi_input.mv_hist);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get MV histogram on win %u failed. err: %d", idx.value, ret);
		return ret;
	}

	ret = MPI_DEV_getIspVar(idx, ctx->var_cfg_idx, &ctx->mpi_input.var);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get variance on win %u failed. err: %d", idx.value, ret);
		return ret;
	}

	return 0;
}
