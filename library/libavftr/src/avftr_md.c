#include "avftr_md.h"

#include "mpi_base_types.h"
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <pthread.h>

#include "video_od.h"
#include "avftr.h"
#include "avftr_common.h"
#include "avftr_log.h"
#include "mpi_dev.h"

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

typedef struct {
	uint32_t start_time; /**< Param to store alarm start time */
	AVFTR_MD_PARAM_S param;
	VFTR_MD_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} MD_CTX_S;

static MD_CTX_S g_md_ctx[AVFTR_MD_MAX_SUPPORT_NUM] = { { 0 } };

#define MD_GET_CTX(idx) &g_md_ctx[idx]

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video motion detection content.
 * @param[out] empty        global motion detection index.
 * @see AVFTR_MD_addInstance()
 * @see AVFTR_MD_deleteInstance()
 * @retval enable index.
 */
static int findMdCtx(const MPI_WIN idx, const AVFTR_MD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_MD_MAX_SUPPORT_NUM; i++) {
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

static void getMdResOffset(const MPI_WIN idx, VFTR_MD_STATUS_S *stat)
{
	uint32_t x = 0;
	uint32_t y = 0;
	int32_t i;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	int ret;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Cannot get channel layout for chn%d, ret: %d", chn.chn, ret);
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

	for (i = 0; i < stat->region_num; i++) {
		stat->attr[i].pts.sx += x;
		stat->attr[i].pts.sy += y;
		stat->attr[i].pts.ex += x;
		stat->attr[i].pts.ey += y;
	}

	return;
}

static void mdProcShakeObjectList(VIDEO_FTR_OBJ_LIST_S *vol, const VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	int i;
	int obj_cnt = 0;
	MPI_IVA_OBJ_LIST_S *ol = &vol->basic_list;
	for (i = 0; i < obj_list->basic_list.obj_num; i++) {
		if (obj_list->obj_attr[i].shaking) {
			/* skip shaking objects */
			//memcpy(&ol->obj[i].rect, &obj_list->obj_attr[i].shake_rect, sizeof(MPI_RECT_POINT_S));
		} else {
			ol->obj[obj_cnt] = obj_list->basic_list.obj[i];
			strcpy(vol->obj_attr[obj_cnt].cat, obj_list->obj_attr[i].cat);
			vol->obj_attr[obj_cnt].shaking = obj_list->obj_attr[i].shaking;
			obj_cnt++;
		}
	}
	ol->obj_num = obj_cnt;
	return;
}

static void mdProcPdObjectList(VIDEO_FTR_OBJ_LIST_S *vol, const VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	int i;
	int obj_cnt = 0;
	MPI_IVA_OBJ_LIST_S *ol = &vol->basic_list;
	for (i = 0; i < obj_list->basic_list.obj_num; i++) {
		if (!strcmp(obj_list->obj_attr[i].cat, AVFTR_PD_UNKNOWN)) {
			/* skip shaking objects */
			//memcpy(&ol->obj[i].rect, &obj_list->obj_attr[i].shake_rect, sizeof(MPI_RECT_POINT_S));
		} else {
			ol->obj[obj_cnt] = obj_list->basic_list.obj[i];
			strcpy(vol->obj_attr[obj_cnt].cat, obj_list->obj_attr[i].cat);
			vol->obj_attr[obj_cnt].shaking = obj_list->obj_attr[i].shaking;
			obj_cnt++;
		}
	}
	ol->obj_num = obj_cnt;
	return;
}

/**
 * @brief Invoke callback function when alarm condition is satisfied.
 * @param[in] vftr_md_ctx   video motion detection content.
 * @param[in] md_stat       video motion detection result.
 * @param[in,out] alarm_ctx video motion detection alarm content.
 * @param[in] obj_list      object list.
 * @see AVFTR_MD_getRes()
 * @retval none.
 */
static void genMdAlarm(AVFTR_MD_CTX_S *vftr_md_ctx, const VFTR_MD_STATUS_S *md_stat, MD_CTX_S *alarm_ctx,
                       const VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	if (vftr_md_ctx->cb == NULL) {
		return;
	}

	int i;
	uint32_t timestamp = obj_list->basic_list.timestamp;
	uint32_t diff;
	for (i = 0; i < md_stat->region_num; i++) {
		if (md_stat->stat[i].alarm == VFTR_MD_ALARM_TRUE) {
			if (alarm_ctx->start_time) {
				/* Alarm buffer duration checking */
				diff = timestamp - alarm_ctx->start_time;
				if (diff > alarm_ctx->param.duration) {
					vftr_md_ctx->total_alarm = 1;
					vftr_md_ctx->cb(1);
				}
			} else {
				/* Register alarm start time */
				alarm_ctx->start_time = timestamp;
			}
			return;
		}
	}
	if (vftr_md_ctx->total_alarm) {
		vftr_md_ctx->cb(0);
		vftr_md_ctx->total_alarm = 0;
	}
	/* Reset alarm buffer */
	alarm_ctx->start_time = 0;
	return;
}

/**
 * @brief Get predefined formatted MD result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  md_stat      video motion detection result.
 * @param[out] str          formatted MD result string buffer.
 * @see AVFTR_MD_getRes()
 * @retval length of formatted MD result.
 */
static int getMdMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                     const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                     const VFTR_MD_STATUS_S *md_stat, char *str)
{
	int offset = 0;
	int i = 0;
	VFTR_MD_STATUS_S dst_stat = *md_stat;

	if (src_idx.value != dst_idx.value) {
		for (int i = 0; i < dst_stat.region_num; i++) {
			rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_stat.attr[i].pts);
		}
	}

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "<MD>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "\"md\":[ ");
#endif /* !IVA_FORMAT_XML */
	for (i = 0; i < dst_stat.region_num; i++) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<REG ID=\"%d\" ALARM=\"%d\" RECT=\"%d %d %d %d\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"reg\":{\"id\":%d,\"alarm\":%d,\"rect\":[%d,%d,%d,%d]}},",
#endif /* !IVA_FORMAT_XML */
		                  dst_stat.attr[i].id, dst_stat.stat[i].alarm, dst_stat.attr[i].pts.sx,
		                  dst_stat.attr[i].pts.sy, dst_stat.attr[i].pts.ex, dst_stat.attr[i].pts.ey);
	}
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "</MD>");
#else /* IVA_FORMAT_JSON */
	offset += (sprintf(&str[offset - 1], "],") - 1);
#endif /* !IVA_FORMAT_XML */
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] alarm     alarm
 * @see AVFTR_MD_enable()
 * @retval none.
 */
static void alarmEmptyCb(const uint8_t alarm __attribute__((unused)))
{
	// avftr_log_err("Please register MD alarm callback function.");
	return;
}

/**
 * @brief Get enable status of motion detection.
 * @param[in]  idx          video window index.
 * @param[in]  vftr_dk_ctx  video motion detection content.
 * @see none.
 * @retval enable status of motion detection.
 */
int AVFTR_MD_getStat(const MPI_WIN idx, AVFTR_MD_CTX_S *vftr_md_ctx)
{
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_md_ctx[enable_idx].en;
}

/**
 * @brief Get result of motion detection.
 * @param[in]  idx          video window index.
 * @param[in]  obj_list     object list.
 * @param[in]  buf_idx      vftr buffer index.
 * @see none.
 * @retval 0                success.
 * @retval -EFAULT          MD object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_MD_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx)
{
	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);
	int ret = 0;

	// Skip all actions if motion detection is disabled.
	if (enable_idx < 0 || !vftr_md_ctx[enable_idx].en) {
		return 0;
	}

	VIDEO_FTR_OBJ_LIST_S ol, ol1;
	const VIDEO_FTR_OBJ_LIST_S *ol_ptr, *ol_ptr1;

	/* idx is registered */
	MD_CTX_S *ctx = MD_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("MD instance is NULL!");
		return -EFAULT;
	}

	VFTR_MD_STATUS_S *md_result_shm = &vftr_md_ctx[enable_idx].md_res[buf_idx];

	ol_ptr1 = obj_list;
	if (ctx->param.en_skip_shake) {
		mdProcShakeObjectList(&ol, obj_list);
		ol_ptr1 = &ol;
	}
	ol_ptr = ol_ptr1;
	if (ctx->param.en_skip_pd) {
		mdProcPdObjectList(&ol1, ol_ptr1);
		ol_ptr = &ol1;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_MD_detectMotion(ctx->instance, &ol_ptr->basic_list, md_result_shm);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Detect motion failed. err: %d", ret);
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	genMdAlarm(&vftr_md_ctx[enable_idx], md_result_shm, ctx, obj_list);
	pthread_mutex_unlock(&ctx->cb_lock);

	getMdResOffset(idx, md_result_shm);

	return 0;
}

/**
 * @brief Translate result of motion detection.
 * @param[in]  vftr_md_ctx      door keeper result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted DK result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_MD_getRes()
 * @retval length of door keeper result.
 */
int AVFTR_MD_transRes(AVFTR_MD_CTX_S *vftr_md_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                      const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                      const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findMdCtx(src_idx, vftr_md_ctx, NULL);
	if (enable_idx < 0 || !vftr_md_ctx[enable_idx].en) {
		/* idx is not registered yet */
		return 0;
	}

	return getMdMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                 &vftr_md_ctx[enable_idx].md_res[buf_idx], str);
}

/**
 * @brief Add motion detection instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_MD_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create MD instance.
 * @retval -ENOMEM          No more space to register idx / malloc MD instance failed.

 */
int AVFTR_MD_addInstance(const MPI_WIN idx)
{
	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	int empty_idx;
	int set_idx = findMdCtx(idx, vftr_md_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("MD is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered */
		MD_CTX_S *ctx = MD_GET_CTX(empty_idx);
		ctx->instance = VFTR_MD_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create MD instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		ctx->cb_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_md_ctx[empty_idx].idx = idx;
		vftr_md_ctx[empty_idx].reg = 1;
		vftr_md_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		vftr_md_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);

	} else {
		/* No more space to register idx */
		avftr_log_err("Add MD instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete motion detection instance.
 * @param[in]  idx          video window index.
 * @return The execution result.
 * @retval 0                success.
 * @retval -EAGAIN          motion detection is enabled, not remove.
 * @see AVFTR_MD_addInstance()
 */
int AVFTR_MD_deleteInstance(const MPI_WIN idx)
{
	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("MD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_md_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("MD is still enable on win(%u, %u, %u), can not be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	MD_CTX_S *ctx = MD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_MD_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Free md instance failed!");
		return ret;
	}
	vftr_md_ctx[enable_idx].reg = 0;
	vftr_md_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_md_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Enable motion detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_MD_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_MD_enable(const MPI_WIN idx)
{
	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("MD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_md_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("MD is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	int ret;
	MD_CTX_S *ctx = MD_GET_CTX(enable_idx);
	ret = VIDEO_FTR_enableOd_implicit(idx);
	if (ret != 0) {
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	if (vftr_md_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("MD alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_md_ctx[enable_idx].cb = alarmEmptyCb;
	}
	pthread_mutex_unlock(&ctx->cb_lock);

	pthread_mutex_lock(&ctx->lock);
	ctx->param.duration = AVFTR_MD_ALRAM_BUF_DEFAULT * AVFTR_VIDEO_JIF_HZ;
	pthread_mutex_unlock(&ctx->lock);
	vftr_md_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable motion detection.
 * @param[in]  idx          video window index.
 * @see AVFTR_MD_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_MD_disable(const MPI_WIN idx)
{
	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;

	int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("MD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_md_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("MD is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_disableOd_implicit(idx);
	if (ret != 0) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}
	vftr_md_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of motion detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        video motion detection parameters.
 * @see AVFTR_MD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_MD_getParam(MPI_WIN idx, AVFTR_MD_PARAM_S *param)
{
	const int ret = checkMpiDevValid(idx);
	if (ret != 0) {
		return ret;
	}

	const AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("MD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	const MD_CTX_S *ctx = MD_GET_CTX(enable_idx);
	*param = ctx->param;

	return 0;
}

/**
 * @brief set parameters of motion detection.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_MD_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        video motion detection parameters.
 * @see AVFTR_MD_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 * @retval -EFAULT          input parameter is null.
 * @retval -EINVAL          invalid MD parameter.
 */
int AVFTR_MD_setParam(MPI_WIN idx, const AVFTR_MD_PARAM_S *param)
{
	if (param == NULL) {
		avftr_log_err("Pointer to the MD parameter should not be NULL.");
		return -EFAULT;
	}

	if (param->duration > (AVFTR_MD_ALARM_BUF_MAX * AVFTR_VIDEO_JIF_HZ)) {
		avftr_log_err("MD alarm buffer duration exceeds maximum (%d seconds). recv: %d",
					  AVFTR_MD_ALARM_BUF_MAX, param->duration / AVFTR_VIDEO_JIF_HZ);
		return -EINVAL;
	}

	const AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("MD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	MD_CTX_S *ctx = MD_GET_CTX(enable_idx);
	MPI_SIZE_S res = {};
	int ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	ret = VFTR_MD_checkParam(&param->md_param, &res);
	if (ret != 0) {
		return ret;
	}

	// Store config to queue,
	// After thread avftr_iva releases the lock, apply the setting to MD instance.
	pthread_mutex_lock(&ctx->lock);
	ctx->param = *param;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of motion detection instance.
 * @param[in] idx           video window index.
 * @see AVFTR_MD_getParam()
 * @see AVFTR_MD_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_MD_writeParam(const MPI_WIN idx)
{
	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	const int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	MD_CTX_S *ctx = MD_GET_CTX(enable_idx);

	int ret = 0;

	AVFTR_MD_PARAM_S *param = &ctx->param;

	int s_flag = ctx->s_flag;
	if (s_flag == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_MD_setParam(ctx->instance, &param->md_param);
		ctx->s_flag = 0;
		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Register alarm callback function of motion detection.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none.
 * @retval 0                   success.
 * @retval -EFAULT             NULL pointer of cb function.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_MD_regCallback(const MPI_WIN idx, const AVFTR_MD_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to motion detection alarm callback function should not be NULL.");
		return -EFAULT;
	}

	AVFTR_MD_CTX_S *vftr_md_ctx = vftr_res_shm->md_ctx;
	int enable_idx = findMdCtx(idx, vftr_md_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("MD is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	MD_CTX_S *ctx = MD_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_md_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}
