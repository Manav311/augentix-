#include "avftr_aroi.h"

#include <errno.h>
#include <stdio.h>
#include <pthread.h>

#include "avftr_log.h"
#include "mpi_dev.h"

#include "avftr.h"
#include "avftr_common.h"

typedef struct {
	VFTR_AROI_STATUS_S aroi_res;
	AVFTR_AROI_PARAM_S param;
	VFTR_AROI_INSTANCE_S *instance;
	int s_flag;
	pthread_mutex_t lock;
	pthread_mutex_t cb_lock;
} AROI_CTX_S;

#define abs(a)                          \
	({                              \
		__typeof__(a) _a = (a); \
		_a < 0 ? -_a : _a;      \
	})

static AROI_CTX_S g_aroi_ctx[AVFTR_AROI_MAX_SUPPORT_NUM] = { { { { 0 } } } };
#define AROI_GET_CTX(idx) &g_aroi_ctx[idx]

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video automatic region of interest content.
 * @param[out] empty        global automatic region of interest index.
 * @see AVFTR_AROI_addInstance()
 * @see AVFTR_AROI_deleteInstance()
 * @retval enable index.
 */
static int findAroiCtx(const MPI_WIN idx, const AVFTR_AROI_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_AROI_MAX_SUPPORT_NUM; i++) {
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
 * @param[in] vftr_aroi_ctx video automatic region of interest content.
 * @param[in] aroi_stat     video automatic region of interest result.
 * @param[in] obj_list      object list.
 * @param[in] rect          video layout rectangle.
 * @see AVFTR_AROI_getRes()
 * @retval none.
 */
static void genAroiAlarm(const AVFTR_AROI_CTX_S *vftr_aroi_ctx, const VFTR_AROI_STATUS_S *aroi_stat,
                         const VIDEO_FTR_OBJ_LIST_S *obj_list, const MPI_RECT_S *rect)
{
	if (vftr_aroi_ctx->cb == NULL) {
		return;
	}

	vftr_aroi_ctx->cb(vftr_aroi_ctx->idx, aroi_stat, obj_list);
	MPI_SIZE_S sz = { .width = rect->width, .height = rect->height };
	AVFTR_AROI_updateMotionEvt(vftr_aroi_ctx->idx, aroi_stat, obj_list, &sz);
	return;
	//FIXME: add vftr_aroi_ctx[idx].cb() if alarm is needed
}

static void getWinInfo(const MPI_WIN idx, MPI_RECT_S *rect)
{
	int i, ret;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);

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

	rect->x = layout_attr.window[i].x;
	rect->y = layout_attr.window[i].y;
	rect->width = layout_attr.window[i].width;
	rect->height = layout_attr.window[i].height;
	return;
}

static void getAroiResOffset(VFTR_AROI_STATUS_S *aroi_stat, const MPI_RECT_S *rect)
{
	aroi_stat->roi.sx += rect->x;
	aroi_stat->roi.sy += rect->y;
	aroi_stat->roi.ex += rect->x;
	aroi_stat->roi.ey += rect->y;
	return;
}

static void aroiProcShakeObjectList(MPI_IVA_OBJ_LIST_S *ol, const VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	int i;
	int obj_cnt = 0;
	for (i = 0; i < obj_list->basic_list.obj_num; i++) {
		if (obj_list->obj_attr[i].shaking) {
			/* skip shaking objects */
			//memcpy(&ol->obj[i].rect, &obj_list->obj_attr[i].shake_rect, sizeof(MPI_RECT_POINT_S));
		} else {
			ol->obj[obj_cnt] = obj_list->basic_list.obj[i];
			obj_cnt++;
		}
	}
	ol->obj_num = obj_cnt;
	return;
}

/**
 * @brief Get predefined formatted AROI result string for Multiplayer.
 * @param[in]  src_idx      source video window index.
 * @param[in]  dst_idx      dest video window index.
 * @param[in]  src_rect     source video window layout in rect.
 * @param[in]  dst_rect     dest video window layout in rect.
 * @param[in]  src_roi      source video window roi.
 * @param[in]  dst_roi      dest video window roi.
 * @param[in]  aroi_stat    video automatic region of interest result.
 * @param[out] str          formatted AROI result string buffer.
 * @see AVFTR_AROI_getRes()
 * @retval length of formatted AROI result.
 */
static int getAroiMeta(const MPI_WIN src_idx, const MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                       const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi,
                       const VFTR_AROI_STATUS_S *aroi_stat, char *str)
{
	int offset = 0;
	VFTR_AROI_STATUS_S dst_aroi_stat = *aroi_stat;

	if (src_idx.value != dst_idx.value) {
		rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_aroi_stat.roi);
	}

	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	                  "<AROI><ROI ID=\"0\" RECT=\"%d %d %d %d\"/></AROI>",
#else /* IVA_FORMAT_JSON */
	                  "\"aroi\":{\"roi\":{\"rect\":[%d,%d,%d,%d]}},",
#endif /* IVA_FORMAT_XML */
	                  dst_aroi_stat.roi.sx, dst_aroi_stat.roi.sy, dst_aroi_stat.roi.ex, dst_aroi_stat.roi.ey);
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @see AVFTR_AROI_enable()
 */
static void alarmEmptyCb(const MPI_WIN idx __attribute__((unused)),
                         const VFTR_AROI_STATUS_S *status __attribute__((unused)),
                         const VIDEO_FTR_OBJ_LIST_S *obj_list __attribute__((unused)))
{
	return;
}

static inline int overlapCheck(const MPI_RECT_POINT_S *rect0, const MPI_RECT_POINT_S *rect1)
{
#define NO_OVERLAP (0)
#define OVERLAP (1)
	int overlap_sx, overlap_sy, overlap_ex, overlap_ey;
	overlap_sx = MAX(rect0->sx, rect1->sx);
	if (overlap_sx > rect0->ex)
		return NO_OVERLAP;
	overlap_sy = MAX(rect0->sy, rect1->sy);
	if (overlap_sy > rect0->ey)
		return NO_OVERLAP;
	overlap_ex = MIN(rect0->ex, rect1->ex);
	if (overlap_ex < rect0->sx)
		return NO_OVERLAP;
	overlap_ey = MIN(rect0->ey, rect1->ey);
	if (overlap_ey < rect0->sy)
		return NO_OVERLAP;
	return OVERLAP;
}

#define UPDATE_MOTION_EVT_EN

int AVFTR_AROI_updateMotionEvt(const MPI_WIN idx, const VFTR_AROI_STATUS_S *status,
                               const VIDEO_FTR_OBJ_LIST_S *obj_list, const MPI_SIZE_S *sz)
{
#ifdef UPDATE_MOTION_EVT_EN
#define MOTION_DIR_TH (20)
#define MOTION_DIR_QUIET_TH (5)

	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	static MPI_RECT_POINT_S prev_rect;
	static int start = 0;
	//int update = 1;
	uint32_t w, h, mid_w;
	int cent_tar_x, cent_prev_x, cent_cur_x, i;
	int region_state = 0, dir_state = 0, diff, diff1;
	MPI_RECT_POINT_S left, right, target;
	const MPI_IVA_OBJ_LIST_S *ol = NULL;

	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_aroi_ctx[enable_idx].en) {
		/* idx is not enabled yet */
		avftr_log_err("AROI is not enabled on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	/* Calc motion reg */
	w = sz->width;
	h = sz->height;
	mid_w = w >> 2;
	left = (MPI_RECT_POINT_S){ .sx = 0, .sy = 0, .ex = mid_w - 1, .ey = h };
	right = (MPI_RECT_POINT_S){ .sx = mid_w, .sy = 0, .ex = w - 1, .ey = h };
	ol = &obj_list->basic_list;
	for (i = 0; i < ol->obj_num; i++) {
		if (obj_list->obj_attr[i].shaking) {
			/* Skip shaking object */
			continue;
		}
		region_state |= (overlapCheck(&ol->obj[i].rect, &right) << 1) | (overlapCheck(&ol->obj[i].rect, &left));
		if (region_state == AVFTR_MOTION_BOTH) {
			/* Do nothing if both region detected */
			//update = 0;
			break;
		}
	}
	//if (update) {
	vftr_aroi_ctx[enable_idx].motion_reg = region_state;
	//}
	/* Calc motion direction */
	/* initialize check */
	if (start == 0) {
		start = 1;
	} else {
		AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);
		if (ctx->instance == NULL) {
			avftr_log_err("AROI instance is NULL!");
			return -EFAULT;
		}

		pthread_mutex_lock(&ctx->lock);
		VFTR_AROI_getTarget(ctx->instance, &target);
		pthread_mutex_unlock(&ctx->lock);

		cent_tar_x = (target.sx + target.ex + 1) >> 1;
		cent_prev_x = (prev_rect.sx + prev_rect.ex + 1) >> 1;
		cent_cur_x = (status->roi.sx + status->roi.ex + 1) >> 1;
		diff = (cent_tar_x - cent_prev_x);
		if (abs(diff) > MOTION_DIR_TH) {
			dir_state = (diff < 0) ? 0b10 : 0b01;
		} else {
			dir_state = 0;
		}
		if (dir_state) {
			diff1 = (cent_cur_x - cent_prev_x);
			if (abs(diff1) < MOTION_DIR_QUIET_TH) {
				dir_state = 0;
			} else {
				dir_state &= (diff1 < 0) ? 0b10 : 0b01;
			}
		}
	}
	prev_rect = status->roi;
	vftr_aroi_ctx[enable_idx].motion_dir = dir_state;
#if (0)
	int tmp = region_state << 2 | dir_state;
	if (tmp)
		printf("%s:%d motion dir:(%d:%d) %d %d %d\n", __func__, __LINE__, vftr_aroi_ctx[enable_idx].motion_reg,
		       region_state, dir_state, diff, diff1);
#endif

#endif /* !UPDATE_MOTION_EVT_EN */
	return 0;
}

/**
 * @brief Get enable status of automatic region of interest.
 * @param[in]  idx              video window index.
 * @param[in]  vftr_aroi_ctx    video automatic region of interest content.
 * @see none.
 * @retval enable status of automatic region of interest.
 */
int AVFTR_AROI_getStat(const MPI_WIN idx, AVFTR_AROI_CTX_S *vftr_aroi_ctx)
{
	const int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_aroi_ctx[enable_idx].en;
}

/**
 * @brief Get results of automatic region of interest.
 * @param[in]  idx          video window index.
 * @param[in]  obj_list     object list.
 * @param[in]  buf_idx      vftr buffer index.
 * @retval 0                success.
 * @retval -EFAULT          AROI object is NULL.
 */
// This function should be invoked by AVFTR server routine only.
int AVFTR_AROI_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	const int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	if (enable_idx < 0 || !vftr_aroi_ctx[enable_idx].en) {
		return 0;
	}

	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("AROI instance for WIN(%d, %d, %d) is NULL!", idx.dev, idx.chn, idx.win);
		return -EFAULT;
	}

	MPI_IVA_OBJ_LIST_S ol;
	const MPI_IVA_OBJ_LIST_S *ol_ptr;
	MPI_RECT_S rect = { 0 };

	VFTR_AROI_STATUS_S *aroi_result_shm = &vftr_aroi_ctx[enable_idx].aroi_res[buf_idx];
	MPI_RECT_POINT_S *aroi_tar = &vftr_aroi_ctx[enable_idx].tar[buf_idx];

	if (ctx->param.en_skip_shake) {
		aroiProcShakeObjectList(&ol, obj_list);
		ol_ptr = &ol;
	} else {
		ol_ptr = &obj_list->basic_list;
	}
	getWinInfo(idx, &rect);

	pthread_mutex_lock(&ctx->lock);
	VFTR_AROI_detectRoi(ctx->instance, ol_ptr, aroi_result_shm);
	VFTR_AROI_getTarget(ctx->instance, aroi_tar);
	pthread_mutex_unlock(&ctx->lock);

	pthread_mutex_lock(&ctx->cb_lock);
	genAroiAlarm(&vftr_aroi_ctx[enable_idx], aroi_result_shm, obj_list, &rect);
	pthread_mutex_unlock(&ctx->cb_lock);

	getAroiResOffset(aroi_result_shm, &rect);

	return 0;
}

/**
 * @brief Translate result of automatic region of interest.
 * @param[in]  vftr_aroi_ctx    video automatic region of interest result.
 * @param[in]  src_idx          source video window index.
 * @param[in]  dst_idx          dest video window index.
 * @param[in]  src_rect         source video window layout in rect.
 * @param[in]  dst_rect         dest video window layout in rect.
 * @param[in]  src_roi          source video window roi.
 * @param[in]  dst_roi          dest video window roi.
 * @param[out] str              formatted AROI result string buffer.
 * @param[in]  buf_idx          vftr buffer index.
 * @see AVFTR_AROI_getRes()
 * @retval length of automatic region of interest result.
 */
// This function should be invoked by AVFTR client routine only.
int AVFTR_AROI_transRes(AVFTR_AROI_CTX_S *vftr_aroi_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                        const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                        const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findAroiCtx(src_idx, vftr_aroi_ctx, NULL);
	if (enable_idx < 0 || !vftr_aroi_ctx[enable_idx].en) {
		return 0;
	}

	return getAroiMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                   &vftr_aroi_ctx[enable_idx].aroi_res[buf_idx], str);
}

/**
 * @brief Add automatic region of interest instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_AROI_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create AROI instance.
 * @retval -ENOMEM          No more space to register idx / malloc AROI instance failed.
 */
int AVFTR_AROI_addInstance(const MPI_WIN idx)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int empty_idx;
	int set_idx = findAroiCtx(idx, vftr_aroi_ctx, &empty_idx);

	if (set_idx >= 0) {
		/* idx is registered */
		avftr_log_err("AROI is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	} else if (set_idx < 0 && empty_idx >= 0) {
		/* idx is not registered yet but there is empty space to be registered */
		AROI_CTX_S *ctx = AROI_GET_CTX(empty_idx);
		ctx->instance = VFTR_AROI_newInstance();
		if (!ctx->instance) {
			avftr_log_err("Failed to create AROI instance.");
			return -EFAULT;
		}
		ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
		ctx->cb_lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

		vftr_aroi_ctx[empty_idx].idx = idx;
		vftr_aroi_ctx[empty_idx].reg = 1;
		vftr_aroi_ctx[empty_idx].en = 0;

		pthread_mutex_lock(&ctx->cb_lock);
		vftr_aroi_ctx[empty_idx].cb = NULL;
		pthread_mutex_unlock(&ctx->cb_lock);

	} else {
		/* No more space to register idx */
		avftr_log_err("Add AROI instance failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	return 0;
}

/**
 * @brief Delete automatic region of interest instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_AROI_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_AROI_deleteInstance(const MPI_WIN idx)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered */
		// avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return 0;
	}
	if (vftr_aroi_ctx[enable_idx].en) {
		/* idx is enabled */
		avftr_log_err("AROI is still enable on win(%u, %u, %u), cannot be deleted!", idx.dev, idx.chn,
		              idx.win);
		return -EAGAIN;
	}

	INT32 ret = 0;
	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_AROI_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != 0) {
		avftr_log_err("Free aroi instance failed! err: %d", ret);
		return ret;
	}
	vftr_aroi_ctx[enable_idx].reg = 0;
	vftr_aroi_ctx[enable_idx].en = 0;

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_aroi_ctx[enable_idx].cb = NULL;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}

/**
 * @brief Enable automatic region of interest.
 * @param[in]  idx          video window index.
 * @see AVFTR_AROI_disable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered.
 */
int AVFTR_AROI_enable(const MPI_WIN idx)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered */
		avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_aroi_ctx[enable_idx].en) {
		/* idx is enabled */
		// avftr_log_err("AROI is enabled on win(%u, %u, %u), no need to enable again!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);

	INT32 ret = 0;
	ret = VIDEO_FTR_enableOd_implicit(idx);
	if (ret != 0) {
		return ret;
	}

	pthread_mutex_lock(&ctx->cb_lock);
	if (vftr_aroi_ctx[enable_idx].cb == NULL) {
		// avftr_log_err("AROI alarm callback function is not registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		vftr_aroi_ctx[enable_idx].cb = alarmEmptyCb;
	}
	pthread_mutex_unlock(&ctx->cb_lock);

	vftr_aroi_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable AROI.
 * @param[in]  idx          video window index.
 * @see AVFTR_AROI_enable()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_AROI_disable(const MPI_WIN idx)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_aroi_ctx[enable_idx].en) {
		/* idx is not enabled */
		// avftr_log_err("AROI is not enabled on win(%u, %u, %u) yet, no need to disable!", idx.dev, idx.chn, idx.win);
		return 0;
	}

	INT32 ret = 0;
	ret = VIDEO_FTR_disableOd_implicit(idx);
	if (ret != 0) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}
	vftr_aroi_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of AROI.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @param[out] param        AROI parameters.
 * @see AVFTR_AROI_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_AROI_getParam(const MPI_WIN idx, AVFTR_AROI_PARAM_S *param)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);
	int ret;

	ret = checkMpiDevValid(idx);
	if (ret != 0) {
		return ret;
	}

	param->aroi_param = ctx->param.aroi_param;

	return 0;
}

/**
 * @brief set parameters of AROI.
 * @details Set parameters to buffer, then it can be updated actually by AVFTR_AROI_writeParam().
 * @param[in]  idx          video window index.
 * @param[in]  param        AROI parameters.
 * @see AVFTR_AROI_getParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_AROI_setParam(const MPI_WIN idx, const AVFTR_AROI_PARAM_S *param)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);
	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);

	int ret;

	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	ret = VFTR_AROI_checkParam(&param->aroi_param, &res);
	if (ret != 0) {
		return ret;
	}

	// Copy param to temp buffer and prepare to set to vftr_aroi
	pthread_mutex_lock(&ctx->lock);
	ctx->param.aroi_param = param->aroi_param;
	ctx->param.en_skip_shake = param->en_skip_shake;
	ctx->s_flag = 1;
	pthread_mutex_unlock(&ctx->lock);

	return ret;
}

/**
 * @brief Write parameters of automatic region of interest instance.
 * @param[in] idx           video window index.
 * @see AVFTR_AROI_getParam()
 * @see AVFTR_AROI_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_AROI_writeParam(const MPI_WIN idx)
{
	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	const int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	if (enable_idx < 0) {
		return -ENODEV;
	}

	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);
	AVFTR_AROI_PARAM_S *param = &ctx->param;

	int ret = 0;

	MPI_SIZE_S res;
	memset(&res, 0, sizeof(MPI_SIZE_S));

	ret = getMpiSize(idx, &res);
	if (ret != 0) {
		return ret;
	}

	int s_flag = ctx->s_flag;
	if (s_flag == 1) {
		pthread_mutex_lock(&ctx->lock);
		ret = VFTR_AROI_setParam(ctx->instance, &res, &param->aroi_param);
		ctx->s_flag = 0;
		pthread_mutex_unlock(&ctx->lock);
		if (ret != 0) {
			return ret;
		}
	}

	return 0;
}

/**
 * @brief Register alarm callback function of automatic region of interest.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none.
 * @retval 0                   success.
 * @retval -EFAULT             NULL pointer of cb function.
 * @retval -ENODEV             idx is not registered yet.
 */
int AVFTR_AROI_regCallback(const MPI_WIN idx, const AVFTR_AROI_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to AROI alarm callback function should not be NULL.");
		return -EFAULT;
	}

	AVFTR_AROI_CTX_S *vftr_aroi_ctx = vftr_res_shm->aroi_ctx;
	int enable_idx = findAroiCtx(idx, vftr_aroi_ctx, NULL);

	if (enable_idx < 0) {
		/* idx is not registered yet */
		avftr_log_err("AROI is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	AROI_CTX_S *ctx = AROI_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->cb_lock);
	vftr_aroi_ctx[enable_idx].cb = alarm_cb_fptr;
	pthread_mutex_unlock(&ctx->cb_lock);

	return 0;
}
