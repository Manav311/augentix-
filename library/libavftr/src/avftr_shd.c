#include "avftr_shd.h"

#include <errno.h>
#include <pthread.h>

#include "avftr_log.h"

#include "avftr.h"

typedef struct {
	AVFTR_SHD_PARAM_S param;
	AVFTR_SHD_LONGTERM_LIST_S lt_list;
	VFTR_SHD_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
} SHD_CTX_S;

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static SHD_CTX_S g_shd_ctx[AVFTR_SHD_MAX_SUPPORT_NUM] = { { { 0 } } };

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video shaking object detection content.
 * @param[out] empty        global shaking object detection index.
 * @see AVFTR_SHD_addInstance()
 * @see AVFTR_SHD_deleteInstance()
 * @retval enable index.
 */
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

/**
 * @brief Add shaking object detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_SHD_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create SHD instance.
 * @retval -ENOMEM          No more space to register idx / malloc SHD instance failed.
 */
int AVFTR_SHD_addInstance(const MPI_WIN idx)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	SHD_CTX_S *ctx;
	int empty_idx;
	int enable_idx;

	enable_idx = findShdCtx(idx, vftr_shd_ctx, &empty_idx);
	if (enable_idx >= 0) {
		avftr_log_err("SHD is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	}

	if (empty_idx >= 0) {
		ctx = &g_shd_ctx[empty_idx];
		ctx->instance = VFTR_SHD_newInstance();
		if (ctx->instance == NULL) {
			avftr_log_err("Failed to create SHD instance.");
			return -EFAULT;
		}
		ctx->s_flag = 0;
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_shd_ctx[empty_idx] = (AVFTR_SHD_CTX_S){ .idx = idx, .reg = 1, .en = 0 };

		return 0;
	}

	/* No more space to register idx */
	avftr_log_err("Add SHD instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);

	return -ENOMEM;
}

/**
 * @brief Delete shaking object detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_SHD_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_SHD_deleteInstance(const MPI_WIN idx)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	SHD_CTX_S *ctx;
	int ret;
	int enable_idx;

	enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	if (vftr_shd_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("SHD is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	ctx = &g_shd_ctx[enable_idx];

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_SHD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free SHD obj failed.\n");
		return ret;
	}

	vftr_shd_ctx[enable_idx].reg = 0;
	vftr_shd_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get enable status of shaking object detection.
 * @param[in]  idx          video window index.
 * @param[in]  vftr_shd_ctx  video shaking object detection content.
 * @see none.
 * @retval enable status of shaking object detection.
 */
int AVFTR_SHD_getStat(const MPI_WIN idx, const AVFTR_SHD_CTX_S *vftr_shd_ctx)
{
	if (vftr_shd_ctx == NULL) {
		avftr_log_err("Input parameter cannot be NULL.");
		return -EFAULT;
	}

	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_shd_ctx[enable_idx].en;
}

/**
 * @brief Get detect result.
 */
int AVFTR_SHD_detectShake(const MPI_WIN idx, const MPI_IVA_OBJ_LIST_S *obj_list, AVFTR_SHD_STATUS_S *status)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;
	int ret;

	if (enable_idx < 0) {
		// avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = &g_shd_ctx[enable_idx];
	if (!vftr_shd_ctx[enable_idx].en) {
		// avftr_log_err("SHD is not enabled on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_SHD_detectShake(ctx->instance, obj_list, status);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		return ret;
	}

	return 0;
}

/**
 * @brief Enable shaking object detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_SHD_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_SHD_enable(const MPI_WIN idx)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_shd_ctx[enable_idx].en) {
		// avftr_log_err("SHD is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	vftr_shd_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable shaking object detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_SHD_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SHD_disable(const MPI_WIN idx)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_shd_ctx[enable_idx].en) {
		// avftr_log_err("SHD is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	vftr_shd_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief set parameters of shaking object detection.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_SHD_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video shaking object detection parameters.
 * @see AVFTR_SHD_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SHD_setParam(const MPI_WIN idx, const AVFTR_SHD_PARAM_S *param)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (param == NULL) {
		avftr_log_err("Input parameter cannot be NULL.");
		return -EFAULT;
	}

	ctx = &g_shd_ctx[enable_idx];

	pthread_mutex_lock(&ctx->lock);
	memcpy(&ctx->param, param, sizeof(AVFTR_SHD_PARAM_S));
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Get parameters of shaking object detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video shaking object detection parameters.
 * @see AVFTR_SHD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SHD_getParam(const MPI_WIN idx, AVFTR_SHD_PARAM_S *param)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (param == NULL) {
		avftr_log_err("Input parameter cannot be NULL.");
		return -EFAULT;
	}

	ctx = &g_shd_ctx[enable_idx];
	pthread_mutex_lock(&ctx->lock);
	memcpy(param, &ctx->param, sizeof(AVFTR_SHD_PARAM_S));
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of shaking object detection.
 * @param[in] idx           video window index.
 * @see AVFTR_SHD_getParam()
 * @see AVFTR_SHD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_SHD_writeParam(const MPI_WIN idx)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;
	int ret = 0;

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = &g_shd_ctx[enable_idx];
	if (ctx->s_flag == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_SHD_setParam(ctx->instance, &ctx->param);
		ctx->s_flag = 0;
		pthread_mutex_unlock(&ctx->lock);
	}

	return ret;
}

int AVFTR_SHD_setUsrList(const MPI_WIN idx, const AVFTR_SHD_LONGTERM_LIST_S *lt_list)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (lt_list == NULL) {
		avftr_log_err("Input parameter cannot be NULL.\n");
		return -EFAULT;
	}

	ctx = &g_shd_ctx[enable_idx];
	pthread_mutex_lock(&ctx->lock);
	memcpy(&ctx->lt_list, lt_list, sizeof(AVFTR_SHD_LONGTERM_LIST_S));
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

int AVFTR_SHD_getUsrList(const MPI_WIN idx, AVFTR_SHD_LONGTERM_LIST_S *lt_list)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (lt_list == NULL) {
		avftr_log_err("Input parameter cannot be NULL.");
		return -EFAULT;
	}

	ctx = &g_shd_ctx[enable_idx];
	pthread_mutex_lock(&ctx->lock);
	memcpy(lt_list, &ctx->lt_list, sizeof(AVFTR_SHD_LONGTERM_LIST_S));
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

int AVFTR_SHD_writeUsrList(const MPI_WIN idx)
{
	AVFTR_SHD_CTX_S *vftr_shd_ctx = vftr_res_shm->shd_ctx;
	int enable_idx = findShdCtx(idx, vftr_shd_ctx, NULL);
	SHD_CTX_S *ctx;
	int ret = 0;

	if (enable_idx < 0) {
		avftr_log_err("SHD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = &g_shd_ctx[enable_idx];
	pthread_mutex_lock(&ctx->lock);
	if ((ret = VFTR_SHD_setUserLongTermList(g_shd_ctx[enable_idx].instance, &ctx->lt_list))) {
		avftr_log_err("VFTR_SHD_setUserLongTermList failed. err: %d.", ret);
	}
	pthread_mutex_unlock(&ctx->lock);

	return ret;
}
