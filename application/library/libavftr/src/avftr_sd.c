/**
* @cond
*
* code fragment skipped by Doxygen.
*/

#include <errno.h>
#include <stdio.h>
#include <pthread.h>
#include "mpi_dev.h"
#include "avftr_log.h"

#include "avftr_sd.h"
#include "avftr.h"
#include "aftr_sd.h"

typedef struct {
	AFTR_SD_PARAM_S param;
	AFTR_SD_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} SD_CTX_S;

static SD_CTX_S g_sd_ctx[AVFTR_SD_MAX_SUPPORT_NUM] = { { { 0 } } };
#define SD_GET_CTX(idx) &g_sd_ctx[idx]

extern AVFTR_AUDIO_CTX_S *aftr_res_shm;

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in] idx           audio device index.
 * @param[in]  ctx          audio sound detection content.
 * @param[out] empty        global sound detection index.
 * @see AVFTR_SD_addInstance()
 * @see AVFTR_SD_deleteInstance()
 * @retval enable index.
 */
static int findSdCtx(const MPI_DEV idx, const AVFTR_SD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_SD_MAX_SUPPORT_NUM; i++) {
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
 * @param[in] sd_ctx        audio sound detection content.
 * @param[in] sd_stat       audio sound detection result.
 * @see AVFTR_SD_getRes()
 * @retval none.
 */
static void genSdAlarm(const AVFTR_SD_CTX_S *sd_ctx, const AFTR_SD_STATUS_S *sd_stat)
{
	if (sd_ctx->cb == NULL) {
		return;
	}

	if (sd_stat->alarm) {
		sd_ctx->cb();
	}
	return;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] none.
 * @see AVFTR_SD_enable()
 * @retval none.
 */
static void alarmEmptyCb()
{
	return;
}


/**
 * @brief Get enable status of sound detection.
 * @param[in] idx           audio device index.
 * @param[in] aftr_sd_ctx   audio sound detection content.
 * @see none.
 * @retval enable status of sound detection.
 */
int AVFTR_SD_getStat(const MPI_DEV idx, AVFTR_SD_CTX_S *aftr_sd_ctx)
{
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);

	return enable_idx < 0 ? 0 : aftr_sd_ctx[enable_idx].en;
}

/**
 * @brief Get result of sound detection.
 * @param[in]  idx          audio device index.
 * @param[in]  raw_buffer   raw audio bits.
 * @param[in]  size_of_raw  raw audio size.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          SD object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_SD_getRes(const MPI_DEV idx, const char *raw_buffer, const int size_of_raw, const int buf_idx)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	const int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);

	if (enable_idx < 0 || !aftr_sd_ctx[enable_idx].en) {
		return 0;
	}

	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("SD instance is NULL!");
		return -EFAULT;
	}

	AFTR_SD_STATUS_S *sd_result_shm = &aftr_sd_ctx[enable_idx].sd_res[buf_idx];

	pthread_mutex_lock(&ctx->lock);
	AFTR_SD_detect(ctx->instance, raw_buffer, size_of_raw, sd_result_shm);
	pthread_mutex_unlock(&ctx->lock);

	pthread_mutex_lock(&ctx->cb_lock);
	genSdAlarm(&aftr_sd_ctx[enable_idx], sd_result_shm);
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Add sound detection instance.
 * @param[in]  idx          audio device index.
 * @see AVFTR_SD_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create SD instance.
 * @retval -ENOMEM          No more space to register idx / malloc SD instance failed.
 */
int AVFTR_SD_addInstance(const MPI_DEV idx)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int empty_idx;
	int set_idx = findSdCtx(idx, aftr_sd_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("SD is registered on dev %u.", idx.dev);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered*/
		SD_CTX_S *ctx = SD_GET_CTX(empty_idx);
		ctx->instance = AFTR_SD_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create SD instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		ctx->cb_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		aftr_sd_ctx[empty_idx].idx = idx;
		aftr_sd_ctx[empty_idx].reg = 1;
		aftr_sd_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		aftr_sd_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);

	} else {
		/* No more space to register idx */
		avftr_log_err("Add SD instance failed on dev %u.", idx.dev);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete sound detection instance.
 * @param[in]  idx          audio device index.
 * @see AVFTR_SD_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_SD_deleteInstance(const MPI_DEV idx)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
		return 0;
	}
	if (aftr_sd_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("SD is still enable on dev %u, can not be deleted!", idx.dev);
		return -EAGAIN;
	}

	INT32 ret = 0;
	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = AFTR_SD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free SD instance failed! err: %d", ret);
		return ret;
	}
	aftr_sd_ctx[enable_idx].reg = 0;
	aftr_sd_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	aftr_sd_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Enable sound detection.
 * @param[in]  idx          audio device index.
 * @see AVFTR_SD_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_SD_enable(const MPI_DEV idx)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
		return -ENODEV;
	}

	if (aftr_sd_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("SD is enabled on dev %u, no need to enable again!", idx.dev);
		return 0;
	}

	INT32 ret = 0;
	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	if (aftr_sd_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("Sound detection alarm callback function is not registered on win %u.", idx.win);
		aftr_sd_ctx[enable_idx].cb = alarmEmptyCb;
	}
	pthread_mutex_unlock(&ctx->cb_lock);

	AFTR_SD_PARAM_S param = { .volume = 95, .duration = 1, .suppression = 2 };

	ret = AFTR_SD_setParam(ctx->instance, &param);
	if (ret != 0) {
		avftr_log_err("Sound detection failed to set default parameters on dev %u.", idx.dev);
		return ret;
	}

	aftr_sd_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable sound detection.
 * @param[in]  idx          audio device index.
 * @see AVFTR_SD_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SD_disable(const MPI_DEV idx)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
		return -ENODEV;
	}

	if (!aftr_sd_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("SD is not enabled on dev %u yet, no need to disable!", idx.dev);
		return 0;
	}

	aftr_sd_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of sound detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          audio device index.
 * @param[out] param        audio sound detection parameters.
 * @see AVFTR_SD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SD_getParam(const MPI_DEV idx, AFTR_SD_PARAM_S *param)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
		return -ENODEV;
	}

	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);

	*param = ctx->param;

	return 0;
}

/**
 * @brief set parameters of sound detection.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_SD_writeParam().
 * @param[in]  idx          audio device index.
 * @param[in]  param        audio sound detection parameters.
 * @see AVFTR_SD_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SD_setParam(const MPI_DEV idx, const AFTR_SD_PARAM_S *param)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
		return -ENODEV;
	}

	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);

	int ret;

	ret = AFTR_SD_checkParam(param);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to aftr_sd
	pthread_mutex_lock(&ctx->lock);
	ctx->param = *param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters to sound detection instance.
 * @param[in]  idx          audio device index.
 * @see AVFTR_SD_getParam()
 * @see AVFTR_SD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SD_writeParam(const MPI_DEV idx)
{
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	const int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);
	AFTR_SD_PARAM_S *param = &ctx->param;

	int ret;

	int s_flag = ctx->s_flag;
	if (s_flag == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = AFTR_SD_setParam(ctx->instance, param);
		ctx->s_flag = 0;
		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}

/**
 * @brief Register alarm callback function of sound detection.
 * @param[in]  idx              audio device index.
 * @param[in]  alarm_cb_fptr    function pointer of callback function.
 * @see none.
 * @return The execution result.
 * @retval 0                    success.
 * @retval -EFAULT              NULL pointer of cb function.
 * @retval -ENODEV              idx is not registered yet.
 */
int AVFTR_SD_regCallback(const MPI_DEV idx, const AVFTR_SD_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Args should not be NULL.");
		return -EFAULT;
	}

	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	int enable_idx = findSdCtx(idx, aftr_sd_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
		return -ENODEV;
	}

	SD_CTX_S *ctx = SD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	aftr_sd_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Reset results of sound detection.
 * @param[in] dev_idx       audio device index.
 * @see none.
 * @return The execution result.
 */
int AVFTR_SD_resetShm(const MPI_DEV idx __attribute__((unused)))
{
	int i;
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	AVFTR_AUDIO_BUF_INFO_S *buf_info = aftr_res_shm->buf_info;

	for (i = 0; i < AVFTR_SD_MAX_SUPPORT_NUM; i++) {
		if (aftr_sd_ctx[i].en) {
			aftr_sd_ctx[i].sd_res[buf_info[i].buf_cur_idx].alarm = 0;
		}
	}

	return 0;
}

int AVFTR_SD_resume(const MPI_DEV idx)
{
	int i;
	AVFTR_SD_CTX_S *aftr_sd_ctx = aftr_res_shm->sd_ctx;
	for (i = 0; i < AVFTR_SD_MAX_SUPPORT_NUM; i++) {
		if (aftr_sd_ctx[i].en) {
			int enable_idx = findSdCtx(aftr_sd_ctx[i].idx, aftr_sd_ctx, NULL);
			if (enable_idx < 0) {
				/* idx is not registered yet */
				avftr_log_err("SD is not registered on dev %u yet!", idx.dev);
				return -ENODEV;
			}
			SD_CTX_S *ctx = SD_GET_CTX(enable_idx);

			pthread_mutex_lock(&ctx->lock);
			AFTR_SD_reset(ctx->instance);
			pthread_mutex_unlock(&ctx->lock);
		}
	}

	return 0;
}
