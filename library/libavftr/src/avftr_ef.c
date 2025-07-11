#include "avftr_ef.h"

#include "mpi_base_types.h"
#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "avftr_log.h"
#include "mpi_dev.h"

#include "video_od.h"
#include "avftr.h"
#include "avftr_common.h"

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

typedef struct {
	VFTR_EF_PARAM_S param;
	VFTR_EF_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} EF_CTX_S;

static EF_CTX_S g_ef_ctx[AVFTR_EF_MAX_SUPPORT_NUM] = { { { 0 } } };

#define EF_GET_CTX(idx) &g_ef_ctx[idx]

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video electronic fence content.
 * @param[out] empty        global electronic fence index.
 * @see AVFTR_EF_addInstance()
 * @see AVFTR_EF_deleteInstance()
 * @retval enable index.
 */
static int findEfCtx(const MPI_WIN idx, const AVFTR_EF_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_EF_MAX_SUPPORT_NUM; i++) {
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
 * @param[in] vftr_ef_ctx   video electronic fence content.
 * @param[in] status        video electronic fence detected result.
 * @see AVFTR_EF_getRes()
 * @retval none.
 */
static void genEfAlarm(AVFTR_EF_CTX_S *vftr_ef_ctx, const VFTR_EF_STATUS_S *status __attribute__((unused)))
{
	if (vftr_ef_ctx->cb == NULL) {
		return;
	}
	vftr_ef_ctx->cb();
	return;
}

/**
 * @brief Get predefined formatted EF result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  ef_stat      video electronic fence result.
 * @param[out] str          formatted EF result string buffer.
 * @see AVFTR_EF_getRes()
 * @retval length of formatted EF result.
 */
static int getEfMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                     const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                     const VFTR_EF_STATUS_S *ef_stat, char *str)
{
	int offset = 0;
	int i = 0;
	VFTR_EF_STATUS_S dst_stat;

	for (i = 0; i < ef_stat->fence_num; i++) {
		dst_stat.attr[i].line = ef_stat->attr[i].line;
	}

	if (src_idx.value != dst_idx.value) {
		for (i = 0; i < ef_stat->fence_num; i++) {
			rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_stat.attr[i].line);
		}
	}
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "<EF>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "\"ef\":[ ");
#endif /* !IVA_FORMAT_XML */
	for (i = 0; i < ef_stat->fence_num; i++) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<VL ID=\"%d\" LINE=\"%d %d %d %d\" MODE=\"%d\" "
		                  "ALARM=\"%d\" NAME=\"%s\" CNTP=\"%d\" CNTN=\"%d\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"vl\":{\"id\":%d,\"line\":[%d,%d,%d,%d],\"mode\":%d,"
		                  "\"alarm\":%d,\"name\":\"%s\",\"cntp\":%d,\"cntn\":%d}},",
#endif /* !IVA_FORMAT_XML */
		                  ef_stat->attr[i].id, dst_stat.attr[i].line.sx, dst_stat.attr[i].line.sy,
		                  dst_stat.attr[i].line.ex, dst_stat.attr[i].line.ey, ef_stat->attr[i].mode,
		                  ef_stat->stat[i].alarm, "", /* FIXME: add name in fence struct */
		                  ef_stat->stat[i].cnt[0], ef_stat->stat[i].cnt[1]);
	}
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "</EF>");
#else /* IVA_FORMAT_JSON */
	offset += (sprintf(&str[offset - 1], "],") - 1);
#endif /* !IVA_FORMAT_XML */
	return offset;
}

static void getEfResOffset(const MPI_WIN idx, VFTR_EF_STATUS_S *ef_stat)
{
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t i;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	int ret;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Cannot get channel layout for chn: %d, err: %d", chn.chn, ret);
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

	for (i = 0; i < ef_stat->fence_num; i++) {
		ef_stat->attr[i].line.sx += x;
		ef_stat->attr[i].line.sy += y;
		ef_stat->attr[i].line.ex += x;
		ef_stat->attr[i].line.ey += y;
	}

	return;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] none.
 * @see AVFTR_EF_enable()
 * @retval none.
 */
static void alarmEmptyCb()
{
	// avftr_log_err("Please register electronic fence alarm callback function.\n");
	return;
}

/**
 * @brief Get enable status of electronic fence.
 * @param[in]  idx              video window index.
 * @param[in]  vftr_ef_ctx      video electronic fence content.
 * @see none.
 * @retval enable status of electronic fence.
 */
int AVFTR_EF_getStat(MPI_WIN idx, AVFTR_EF_CTX_S *vftr_ef_ctx)
{
	int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_ef_ctx[enable_idx].en;
}

/**
 * @brief Get results of electronic fence.
 * @param[in]  idx          video window index.
 * @param[in]  obj_list     object list.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          EF object is NULL.
 */
int AVFTR_EF_getRes(MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, int buf_idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);
	int ret = 0;

	if (enable_idx < 0 || !vftr_ef_ctx[enable_idx].en) {
		return 0;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("Ef instance is NULL!");
		return -EFAULT;
	}

	VFTR_EF_STATUS_S *list = &vftr_ef_ctx[enable_idx].ef_res[buf_idx];

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_EF_detect(ctx->instance, &obj_list->basic_list, list);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Failed to run EF detection! err: %d", ret);
		return ret;
	}

	/* Invoke callback function when alarm condition is satisfied. */
	pthread_mutex_lock(&ctx->cb_lock);
	genEfAlarm(&vftr_ef_ctx[enable_idx], list);
	pthread_mutex_unlock(&ctx->cb_lock);
	getEfResOffset(idx, list);

	return 0;
}

/**
 * @brief Translate result of electronic fence.
 * @param[in]  vftr_ef_ctx      video electronic fence result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted EF result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_EF_getRes()
 * @retval length of electronic fence result.
 */
// This function should be invoked by AVFTR client routine only.
int AVFTR_EF_transRes(AVFTR_EF_CTX_S *vftr_ef_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findEfCtx(src_idx, vftr_ef_ctx, NULL);
	if (enable_idx < 0 || !vftr_ef_ctx[enable_idx].en) {
		return 0;
	}

	/* idx is registered and enable */
	return getEfMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                 &vftr_ef_ctx[enable_idx].ef_res[buf_idx], str);
}

/**
 * @brief Add electronic fence instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_EF_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create EF instance.
 * @retval -ENOMEM          No more space to register idx / malloc EF instance failed.
 */
int AVFTR_EF_addInstance(MPI_WIN idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	int empty_idx;
	int set_idx = findEfCtx(idx, vftr_ef_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("EF is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered */
		EF_CTX_S *ctx = EF_GET_CTX(empty_idx);
		ctx->instance = VFTR_EF_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create EF instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		ctx->cb_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_ef_ctx[empty_idx].idx = idx;
		vftr_ef_ctx[empty_idx].reg = 1;
		vftr_ef_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		vftr_ef_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);
	} else {
		/* No more space to register idx */
		avftr_log_err("Add EF instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete electronic fence instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_EF_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_EF_deleteInstance(MPI_WIN idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);

	if (enable_idx < 0) {
		return 0;
	}

	if (vftr_ef_ctx[enable_idx].en) {
		avftr_log_err("EF is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_EF_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free EF instance failed! err: %d", ret);
		return ret;
	}

	vftr_ef_ctx[enable_idx].reg = 0;
	vftr_ef_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_ef_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Enable electronic fence.
 * @param[in]  idx          video window index.
 * @see AVFTR_EF_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_EF_enable(MPI_WIN idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);
	int ret;

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_ef_ctx[enable_idx].en) {
		return 0;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	ret = VIDEO_FTR_enableOd_implicit(idx);
	if (ret != 0) {
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	if (vftr_ef_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("EF alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_ef_ctx[enable_idx].cb = alarmEmptyCb;
	}
	pthread_mutex_unlock(&ctx->cb_lock);
	vftr_ef_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable electronic fence.
 * @param[in]  idx          video window index.
 * @see AVFTR_EF_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EF_disable(MPI_WIN idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);

	if (enable_idx < 0) {
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_ef_ctx[enable_idx].en) {
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_disableOd_implicit(idx);
	if (ret != 0) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}
	vftr_ef_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of electronic fence.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video electronic fence parameters.
 * @see AVFTR_EF_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EF_getParam(MPI_WIN idx, VFTR_EF_PARAM_S *param)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	int ret = 0;

	ret = checkMpiDevValid(idx);
	if (ret != 0) {
		return ret;
	}

	*param = ctx->param;

	return 0;
}

/**
 * @brief set parameters of electronic fence.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_EF_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video electronic fence parameters.
 * @see AVFTR_EF_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 * @retval -EFAULT          input parameter is null.
 */
int AVFTR_EF_setParam(MPI_WIN idx, const VFTR_EF_PARAM_S *param)
{
	if (param == NULL) {
		avftr_log_err("Pointer to the ef parameter should not be NULL.");
		return -EFAULT;
	}

	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	int ret;

	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	ret = VFTR_EF_checkParam(param, &res);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to vftr_ef
	pthread_mutex_lock(&ctx->lock);
	ctx->param = *param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of electronic fence instance.
 * @param[in] idx           video window index.
 * @see AVFTR_EF_getParam()
 * @see AVFTR_EF_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EF_writeParam(MPI_WIN idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	int ret;

	int s_flag = ctx->s_flag;
	if (s_flag == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_EF_setParam(ctx->instance, &ctx->param);
		ctx->s_flag = 0;
		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}
	return 0;
}

/**
 * @brief Check electronic fence parameter.
 * @param[in] idx           video window index.
 * @see none.
 * @retval 0                parameter is valid.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_EF_checkParam(MPI_WIN idx)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	int ret;

	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	ret = VFTR_EF_checkParam(&ctx->param, &res);
	if (ret != 0) {
		return ret;
	}
	return 0;
}

/**
 * @brief Add extra virtual line.
 * @details
 * Add extra virtual line to target window.
 *
 * If success, fence->id will be set to the index of the virtual line. User
 * can use this index to remove the virtual line later.
 *
 * Otherwise, fence->id will be set to 0, other things will not be changed.
 * @param[in]     idx           target video window index.
 * @param[in,out] fence         electronic fence parameters.
 * @see AVFTR_EF_rmVl()
 * @return The execution result.
 * @retval 0                success.
 * @retval -EFAULT          input parameter is null.
 * @retval -ENODEV          ef instance is not registered to target window yet.
 * @retval -EINVAL          input argument is invalid.
 * @retval -EAGAIN		    maximum fence num is reached.
 */
int AVFTR_EF_addVl(MPI_WIN idx, AVFTR_EF_VL_ATTR_S *fence)
{
	if (fence == NULL) {
		avftr_log_err("Pointer to the efence should not be NULL.");
		return -EFAULT;
	}

	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	const UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	const UINT32 chn_idx = MPI_GET_VIDEO_CHN(idx);
	const UINT32 win_idx = MPI_GET_VIDEO_DEV(idx);

	if (dev_idx >= MPI_MAX_VIDEO_DEV_NUM || chn_idx >= MPI_MAX_VIDEO_CHN_NUM || win_idx >= MPI_MAX_VIDEO_WIN_NUM) {
		avftr_log_err("recv invalid window index (%d, %d, %d)", dev_idx, chn_idx, win_idx);
		return -EINVAL;
	}

	VFTR_EF_PARAM_S param = { 0 };

	pthread_mutex_lock(&ctx->lock);
	param = ctx->param;
	pthread_mutex_unlock(&ctx->lock);

	// Search for free slot to insert fence.
	//
	// This method will distribute smallest available fence id to user.
	// Expected in range [1, VFTR_EF_MAX_FENCE_NUM].
	fence->id = 0;
	for (int i = 0; i < VFTR_EF_MAX_FENCE_NUM; i++) {
		if (param.attr[i].id == 0) {
			fence->id = i + 1;
			param.fence_num++;
			memcpy(&param.attr[i], fence, sizeof(AVFTR_EF_VL_ATTR_S));
			break;
		}
	}

	// Exception: There are no free slots in VFTR_EF_INSTANCE_S to insert fence.
	if (fence->id == 0) {
		avftr_log_err("Maximum fence num is reached.");
		return -EAGAIN;
	}

	pthread_mutex_lock(&ctx->lock);
	ctx->param = param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Remove specific virtual line.
 * @details
 * Remove specific virtual line from target window.
 * @param[in] idx           target video window index.
 * @param[in] fence_id      virtual line id.
 * @see AVFTR_EF_addVl()
 * @return The execution result.
 * @retval 0                success.
 * @retval -ENODEV          ef instance is not registered to target window yet.
 * @retval -EINVAL          input argument is invalid.
 */
int AVFTR_EF_rmVl(MPI_WIN idx, INT32 fence_id)
{
	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	const int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	const UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	const UINT32 chn_idx = MPI_GET_VIDEO_CHN(idx);
	const UINT32 win_idx = MPI_GET_VIDEO_DEV(idx);

	if (fence_id < VFTR_EF_VL_MIN_FENCE_ID || fence_id > VFTR_EF_VL_MAX_FENCE_ID) {
		avftr_log_err("Fence index %d exceeds boundary.", fence_id);
		return -EINVAL;
	}

	if (dev_idx >= MPI_MAX_VIDEO_DEV_NUM || chn_idx >= MPI_MAX_VIDEO_CHN_NUM || win_idx >= MPI_MAX_VIDEO_WIN_NUM) {
		avftr_log_err("recv invalid window index (%d, %d, %d)", dev_idx, chn_idx, win_idx);
		return -EINVAL;
	}

	VFTR_EF_PARAM_S param = { 0 };

	pthread_mutex_lock(&ctx->lock);
	param = ctx->param;
	pthread_mutex_unlock(&ctx->lock);

	// Remove target virtual line.
	// Based on the ID distribution method in AVFTR_EF_addVl(), we can
	// simply check whether the target slot is in use or not.
	if (param.attr[fence_id - 1].id == 0) {
		avftr_log_err("Fence id %d does not exist.", fence_id);
		return -EINVAL;
	}

	param.fence_num--;
	param.attr[fence_id - 1].id = 0;

	pthread_mutex_lock(&ctx->lock);
	ctx->param = param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Register alarm callback function of electronic fence.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none.
 * @retval 0                   success.
 * @retval -EFAULT             NULL pointer of cb function.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_EF_regCallback(MPI_WIN idx, const AVFTR_EF_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to electric fence alarm callback function should not be NULL.");
		return -EFAULT;
	}

	AVFTR_EF_CTX_S *vftr_ef_ctx = vftr_res_shm->ef_ctx;
	int enable_idx = findEfCtx(idx, vftr_ef_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("EF is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	EF_CTX_S *ctx = EF_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_ef_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}
