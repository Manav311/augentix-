#include "avftr_pfm.h"

#include "mpi_base_types.h"
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>

#include "mpi_dev.h"

#include "avftr_log.h"
#include "video_od.h"
#include "avftr.h"

#define VFTR_PFM_TIME_INIT 999999

typedef struct {
	AVFTR_PFM_PARAM_S param;
	VFTR_PFM_INPUT_S mpi_input;
	VFTR_PFM_INSTANCE_S *instance;
	unsigned int is_write;
	unsigned int is_reset;

	MPI_PATH path;
	UINT8 y_avg_cfg_idx;
	UINT8 var_cfg_idx;

	uint32_t set_time; /**< Param to store alarm start time */
	uint32_t hist_time;

	pthread_mutex_t lock;
} PFM_CTX_S;

static PFM_CTX_S g_pfm_ctx[VIDEO_PFM_MAX_SUPPORT_NUM] = { { { { 0 } } } };

#define PFM_GET_CTX(idx) (&g_pfm_ctx[idx])

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static inline int align16(int x)
{
	return ((x / 16) * 16);
}

/**
 * @brief Find CTX with specified WIN_IDX.
 * @details This function also sets empty_idx if target WIN_IDX is not found.
 * @param[in]  idx          video window index.
 * @param[in]  ctx          video pet feeding monitor content.
 * @param[out] empty        global pet feeding monitor index.
 * @see AVFTR_PFM_addInstance()
 * @see AVFTR_PFM_deleteInstance()
 * @retval enable index.
 */
static int findPfmCtx(MPI_WIN idx, AVFTR_PFM_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_PFM_MAX_SUPPORT_NUM; i++) {
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

/**
 * @brief Get predefined metadata format for Multiplayer.
 * @param[in]  src_idx         mpi win index of source window
 * @param[in]  dst_idx         mpi win index of destination window
 * @param[in]  src_rect        source window
 * @param[in]  dst_rect        destination window
 * @param[in] res         Pet feeding monitor result.
 * @param[in] str         metadata string buffer.
 * @see VIDEO_FTR_getLdRes()
 * @retval length of metadata.
 */
static int getPfmMeta(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                      const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const AVFTR_PFM_STATUS_S *status, char *str)
{
	int offset = 0;

	MPI_RECT_POINT_S roi = status->roi;

	if (src_idx.value != dst_idx.value) {
		rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &roi);
	}

	offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML
	                  "<PFM><ROI RECT=\"%d %d %d %d\" EVT=\"%d\" RMDR=\"%d\" /></PFM>"
#else /* IVA_FORMAT_JSON */
	                  "\"pfm\":{\"roi\":{\"rect\":[%d,%d,%d,%d],\"evt\":%d},\"rmdr\":%d},"
#endif /*! !IVA_FORMAT_XML */
	                  ,
	                  roi.sx, roi.sy, roi.ex, roi.ey, status->data.event, status->data.remainder);
	return offset;
}

/**
 * @brief Get enable status of pet feeding monitor.
 * @param[in] idx     video window index.
 * @see none
 * @retval enable status of pet feeding monitor.
 */
int AVFTR_PFM_getStat(const MPI_WIN idx, AVFTR_PFM_CTX_S *vftr_pfm_ctx)
{
	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	return enable_idx < 0 ? 0 : vftr_pfm_ctx[enable_idx].en;
}

/**
 * @brief Get results of pet feeding monitor.
 * @param[in]  idx         video window index.
 * @param[in]  obj_list    object list.
 * @param[out] str         metadata string buffer.
 * @see none
 * @retval length of metadata.
 */
int AVFTR_PFM_getRes(const MPI_WIN idx, const VIDEO_FTR_OBJ_LIST_S *obj_list, const int buf_idx)
{
	int ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);

	if (enable_idx < 0 || !vftr_pfm_ctx[enable_idx].en) {
		return 0;
	}

	ctx = PFM_GET_CTX(enable_idx);
	if (ctx->instance == NULL) {
		avftr_log_err("PFM instance is NULL!");
		return -EFAULT;
	}

	VFTR_PFM_STATUS_S *pfm_status = &vftr_pfm_ctx[enable_idx].stat[buf_idx].data;

	pthread_mutex_lock(&ctx->lock);
	ctx->mpi_input.obj_list = &obj_list->basic_list;
	ret = VFTR_PFM_detect(ctx->instance, &ctx->mpi_input, pfm_status);

	vftr_pfm_ctx[enable_idx].stat[buf_idx].roi = ctx->param.pfm_param.roi;
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Failed to run pet feeding monitor!");
		return ret;
	}

	return 0;
}

/**
 * @brief Get predefined metadata format for Multiplayer.
 * @param[in] src_idx  mpi win index of source window
 * @param[in] dst_idx  mpi win index of destination window
 * @param[in] src_rect source window
 * @param[in] dst_rect destination window
 * @param[in] res         light detection result.
 * @param[in] str         metadata string buffer.
 * @see VIDEO_FTR_getLdRes()
 * @retval length of metadata.
 */
int AVFTR_PFM_transRes(AVFTR_PFM_CTX_S *vftr_pfm_ctx, const MPI_WIN src_idx, const MPI_WIN dst_idx,
                       const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                       const MPI_RECT_S *dst_roi, char *str, const int buf_idx)
{
	const int enable_idx = findPfmCtx(src_idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0 || !vftr_pfm_ctx[enable_idx].en) {
		return 0;
	}

	return getPfmMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
	                  &vftr_pfm_ctx[enable_idx].stat[buf_idx], str);
}

/**
 * @brief Add pet feeding monitor instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_PFM_deleteInstance()
 * @retval 0                success.
 * @retval -EFAULT          Failed to create PFM instance.
 * @retval -ENOMEM          No more space to register idx / malloc PFM instance failed.
 */
int AVFTR_PFM_addInstance(const MPI_WIN idx)
{
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	int empty_idx;
	const int set_idx = findPfmCtx(idx, vftr_pfm_ctx, &empty_idx);

	// Avoid instance double construction.
	if (set_idx >= 0) {
		avftr_log_warn("PFM is registered on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return 0;
	}

	// The slots are all in-use.
	if (empty_idx < 0) {
		avftr_log_err("Failed to create PFM instance on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		return -ENOMEM;
	}

	ctx = PFM_GET_CTX(empty_idx);
	ctx->instance = VFTR_PFM_newInstance();
	if (!ctx->instance) {
		avftr_log_err("Failed to create PFM instance.");
		return -ENOMEM;
	}

	vftr_pfm_ctx[empty_idx].idx = idx;
	vftr_pfm_ctx[empty_idx].reg = 1;
	vftr_pfm_ctx[empty_idx].en = 0;

	ctx->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	return 0;
}

/**
 * @brief Delete pet feeding monitor instance.
 * @param[in]  idx          video window index.
 * @see AVFTR_PFM_addInstance()
 * @retval 0                success.
 * @retval -EAGAIN          idx is enabled, not to remove.
 */
int AVFTR_PFM_deleteInstance(const MPI_WIN idx)
{
	int ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);

	if (enable_idx < 0) {
		return 0;
	}

	if (vftr_pfm_ctx[enable_idx].en) {
		avftr_log_err("PFM is still enable on win(%u, %u, %u), can not be deleted!", idx.dev, idx.chn, idx.win);
		return -EAGAIN;
	}

	ctx = PFM_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_PFM_deleteInstance(&ctx->instance);
	pthread_mutex_unlock(&ctx->lock);

	if (ret != 0) {
		avftr_log_err("Delete pfm instance failed!");
		return ret;
	}

	vftr_pfm_ctx[enable_idx].en = 0;
	vftr_pfm_ctx[enable_idx].reg = 0;

	return 0;
}

/**
 * @brief Enable pet feeding monitor.
 * @param[in]  idx            video window index.
 * @see AVFTR_PFM_disable
 * @retval 0                  success.
 * @retval -ENODEV            idx is not registered.
 */
int AVFTR_PFM_enable(const MPI_WIN idx)
{
	int ret = 0;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_pfm_ctx[enable_idx].en) {
		return 0;
	}

	if ((ret = VIDEO_FTR_enableOd_implicit(idx))) {
		return ret;
	}

	vftr_pfm_ctx[enable_idx].en = 1;

	return 0;
}

/**
 * @brief Disable pet feeding monitor.
 * @param[in]  idx        video window index.
 * @see AVFTR_PFM_enable
 * @retval 0                 success.
 * @retval -1                 unexpected fail.
 */
int AVFTR_PFM_disable(const MPI_WIN idx)
{
	int ret = 0;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet!\n", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_pfm_ctx[enable_idx].en) {
		return 0;
	}

	if ((ret = VIDEO_FTR_disableOd_implicit(idx))) {
		avftr_log_err("Disable object detection on win %d failed!", idx.win);
		return ret;
	}

	vftr_pfm_ctx[enable_idx].en = 0;

	return 0;
}

/**
 * @brief Get parameters of pet feeding monitor.
 * @param[in]  idx        video window index.
 * @param[out] param      pet feeding monitor parameters.
 * @see AVFTR_PFM_setParam
 * @retval 0             success.
 * @retval MPI_ERR_NULL_POINTER    input pointer is NULL.
 * @retval MPI_ERR_DEV_INVALID_WIN invalid video window index.
 * @retval MPI_ERR_DEV_INVALID_CHN invalid video channel index.
 * @retval MPI_ERR_DEV_INVALID_DEV invalid device index.
 * @retval MPI_ERR_NOT_EXIST       device/channel doesn't exist.
 * @retval -1             unexpected fail.
 */
int AVFTR_PFM_getParam(const MPI_WIN idx, AVFTR_PFM_PARAM_S *param)
{
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = PFM_GET_CTX(enable_idx);

	param->pfm_param = ctx->param.pfm_param;

	return 0;
}

/**
 * @brief Set parameters of pet feeding monitor.
 * @param[in] idx          video window index.
 * @param[in] param        pet feeding monitor parameters.
 * @see AVFTR_PFM_getParam
 * @retval 0             success.
 * @retval MPI_ERR_NULL_POINTER    input pointer is NULL.
 * @retval MPI_ERR_DEV_INVALID_WIN invalid video window index.
 * @retval MPI_ERR_DEV_INVALID_CHN invalid video channel index.
 * @retval MPI_ERR_DEV_INVALID_DEV invalid device index.
 * @retval MPI_ERR_NOT_EXIST       device/channel doesn't exist.
 * @retval MPI_ERR_INVALID_PARAM   invalid parameters.
 * @retval -1             unexpected fail.
 */
int AVFTR_PFM_setParam(const MPI_WIN idx, const AVFTR_PFM_PARAM_S *param)
{
	int ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = PFM_GET_CTX(enable_idx);

	ret = VFTR_PFM_checkParam(&param->pfm_param);
	if (ret != 0) {
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ctx->param.pfm_param = param->pfm_param;
	ctx->is_write = 1;
	pthread_mutex_unlock(&ctx->lock);

	return 0;
}

/**
 * @brief Write parameters of pet feeding monitor instance.
 * @param[in] idx           video window index.
 * @see AVFTR_PFM_getParam()
 * @see AVFTR_PFM_setParam()
 * @retval 0                success.
 * @retval -ENODEV          idx is not registered yet.
 */
int AVFTR_PFM_writeParam(const MPI_WIN idx)
{
	int ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;
	MPI_ISP_Y_AVG_CFG_S y_avg_cfg;
	MPI_ISP_VAR_CFG_S var_cfg;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	ctx = PFM_GET_CTX(enable_idx);
	if (!ctx->is_write) {
		return 0;
	}

	ctx->is_write = 0;
	y_avg_cfg = (MPI_ISP_Y_AVG_CFG_S){ .roi = ctx->param.pfm_param.roi, .diff_thr = 0 };
	var_cfg = (MPI_ISP_VAR_CFG_S){ .roi = ctx->param.pfm_param.roi };

	pthread_mutex_lock(&ctx->lock);
	ret = VFTR_PFM_setParam(ctx->instance, &ctx->param.pfm_param);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != 0) {
		avftr_log_err("Write PFM parameters failed. err: %d", ret);
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_rmIspYAvgCfg(idx, ctx->y_avg_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("PFM failed to remove Y avg config. err: %d", ret);
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspYAvgCfg(idx, &y_avg_cfg, &ctx->y_avg_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("PFM failed to add Y avg config. err: %d", ret);
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_rmIspVarCfg(idx, ctx->var_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Release var configuration %d on win %u failed.", ctx->var_cfg_idx, idx.value);
		return -ENODEV;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspVarCfg(idx, &var_cfg, &ctx->var_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Add variance configuration failed on win %u", idx.value);
		return ret;
	}

	return 0;
}

int AVFTR_PFM_regMpiInfo(const MPI_WIN idx __attribute__((unused)))
{
	int ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;
	MPI_ISP_Y_AVG_CFG_S y_avg_cfg;
	MPI_ISP_VAR_CFG_S var_cfg;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet!", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (vftr_pfm_ctx[enable_idx].resource_registered) {
		return 0;
	}

	ctx = PFM_GET_CTX(enable_idx);

	y_avg_cfg = (MPI_ISP_Y_AVG_CFG_S){ .roi = ctx->param.pfm_param.roi, .diff_thr = 0 };
	var_cfg = (MPI_ISP_VAR_CFG_S){ .roi = ctx->param.pfm_param.roi };

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspYAvgCfg(idx, &y_avg_cfg, &ctx->y_avg_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Add y average configuration failed on win %u", idx.value);
		return ret;
	}

	if ((ret = vftrYAvgResDec())) {
		return ret;
	}

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_addIspVarCfg(idx, &var_cfg, &ctx->var_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Add variance configuration failed on win %u", idx.value);
		return ret;
	}

	// get path for dip_stat
	UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	MPI_WIN_ATTR_S win_attr;
	ret = MPI_DEV_getWindowAttr(idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get window attributes failed.");
		return -ENODEV;
	}

	if (win_attr.path.bit.path0_en) {
		ctx->path = MPI_INPUT_PATH(dev_idx, 0);
	} else if (win_attr.path.bit.path1_en) {
		ctx->path = MPI_INPUT_PATH(dev_idx, 1);
	} else if (win_attr.path.bit.path2_en) {
		ctx->path = MPI_INPUT_PATH(dev_idx, 2);
	} else if (win_attr.path.bit.path3_en) {
		ctx->path = MPI_INPUT_PATH(dev_idx, 3);
	} else {
		avftr_log_err("Wrong path bmp %d setting.", win_attr.path.bmp);
		return -EINVAL;
	}

	vftr_pfm_ctx[enable_idx].resource_registered = 1;

	return 0;
}

int AVFTR_PFM_releaseMpiInfo(const MPI_WIN idx __attribute__((unused)))
{
	int ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0) {
		avftr_log_err("PFM is not registered on win(%u, %u, %u) yet.", idx.dev, idx.chn, idx.win);
		return -ENODEV;
	}

	if (!vftr_pfm_ctx[enable_idx].resource_registered) {
		avftr_log_warn("Resource of PFM has been released.");
		return 0;
	}

	ctx = PFM_GET_CTX(enable_idx);

	pthread_mutex_lock(&ctx->lock);
	ret = MPI_DEV_rmIspYAvgCfg(idx, ctx->y_avg_cfg_idx);
	pthread_mutex_unlock(&ctx->lock);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Release y_avg configuration %d on win %u failed.", ctx->y_avg_cfg_idx, idx.value);
		return -ENODEV;
	}
	vftrYAvgResInc();

	// Temporary do not remove var config, otherwise leads video pipeline hanging
	// pthread_mutex_lock(&ctx->lock);
	// ret = MPI_DEV_rmIspVarCfg(idx, ctx->var_cfg_idx);
	// pthread_mutex_unlock(&ctx->lock);
	// if (ret != 0) {
	// 	avftr_log_err("Release var configuration %d on win %u failed.", ctx->var_cfg_idx, idx.value);
	// 	return -ENODEV;
	// }

	vftr_pfm_ctx[enable_idx].resource_registered = 0;

	return 0;
}

// Store MPI info to static memory
// This function should be invoked by AVFTR server routine only.
int AVFTR_PFM_updateMpiInfo(const MPI_WIN idx)
{
	INT32 ret = 0;
	PFM_CTX_S *ctx;
	AVFTR_PFM_CTX_S *vftr_pfm_ctx = vftr_res_shm->pfm_ctx;

	const int enable_idx = findPfmCtx(idx, vftr_pfm_ctx, NULL);
	if (enable_idx < 0 || !vftr_pfm_ctx[enable_idx].en) {
		return 0;
	}

	if (!vftr_pfm_ctx[enable_idx].resource_registered) {
		avftr_log_err("MPI Resource of PFM has not been registered yet!");
		return -ENODEV;
	}

	ctx = PFM_GET_CTX(enable_idx);

	ret = MPI_DEV_getIspYAvg(idx, ctx->y_avg_cfg_idx, &ctx->mpi_input.y_avg);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get ISP Y average on win %u failed. err: %d", idx.value, ret);
		return ret;
	}

	ret = MPI_DEV_getIspVar(idx, ctx->var_cfg_idx, &ctx->mpi_input.var);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get ISP variance on win %u failed. err: %d", idx.value, ret);
		return ret;
	}

	ret = MPI_getStatistics(ctx->path, &ctx->mpi_input.dip_stat);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get IS stat on configuration of window %u. err: %d", idx.value, ret);
		return ret;
	}

	return 0;
}

static void __attribute__((unused)) addPfmResOffset(MPI_WIN idx, MPI_RECT_POINT_S *roi)
{
	uint32_t x = 0;
	uint32_t y = 0;
	uint8_t i;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	int ret;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Cannot get channel layout for chn%d, err: %d", chn.chn, ret);
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
}

static inline uint32_t getCurrentTime(void)
{
	time_t linux_time = time(NULL);
	struct tm *now;
	now = localtime(&linux_time);
	return (uint32_t)now->tm_hour * 3600 + now->tm_min * 60 + now->tm_sec;
}

static int __attribute__((unused))
checkPfmSchedule(MPI_WIN idx __attribute__((unused)), PFM_CTX_S *ctx, uint32_t timestamp)
{
	int32_t regis_time, feeding_time;
	const AVFTR_PFM_SCHEDULE_S *schedule = &ctx->param.schedule;
	int i, ret = 0;
	int cur_time;
	if (ctx->hist_time == VFTR_PFM_TIME_INIT) {
		ctx->hist_time = timestamp;
	} else {
		if ((ctx->hist_time / AVFTR_VIDEO_JIF_HZ) == (timestamp / AVFTR_VIDEO_JIF_HZ)) {
			return 0;
		}
	}

	if (schedule->time_num == 0)
		return 0;

	cur_time = getCurrentTime();

	// avftr_log_debug(" set:%u cur:%u schedule:num:%d [%u,%u,..]\n",
	// *set_time, cur_time, schedule->time_num, schedule->times[0], schedule->times[1]);

	if (ctx->set_time == (uint32_t)cur_time)
		return 0;

	/* TODO: dealing with newday issue */
	for (i = 0; i < schedule->time_num && i < VIDEO_PFM_SCHEDULE_MAX_NUM; i++) {
		regis_time = (schedule->times[i] - schedule->regisBg_feed_interval);
		regis_time = (regis_time >= 0) ? regis_time : (VIDEO_PFM_SEC_PER_DAY + regis_time);
		feeding_time = schedule->times[i] - 1;
		feeding_time = (feeding_time >= 0) ? feeding_time : (VIDEO_PFM_SEC_PER_DAY + feeding_time);
		if (regis_time && cur_time == regis_time) {
			// ret = AVFTR_PFM_reset(idx, VFTR_PFM_REGIS_ENVIR);
			if (ret) {
				avftr_log_err("Failed to register Pfm Background Data");
				return ret;
			}
		}
		if (cur_time == feeding_time) {
			// ret = AVFTR_PFM_reset(idx, VFTR_PFM_REGIS_FEED);
			if (ret) {
				avftr_log_err("Failed to register Pfm Feeding information");
				return ret;
			}
		}
	}
	ctx->set_time = cur_time;
	ctx->hist_time = timestamp;
	return 0;
}

static int __attribute__((unused)) checkVftrPfmParam(const AVFTR_PFM_PARAM_S *param)
{
	const AVFTR_PFM_SCHEDULE_S *schedule = &param->schedule;
	int i;
	if (schedule->time_num >= VIDEO_PFM_SCHEDULE_MAX_NUM) {
		avftr_log_err("[PFM] Exceed maximum schedule number(%d) vs expected(%d)!", VIDEO_PFM_SCHEDULE_MAX_NUM,
		              schedule->time_num);
		return -1;
	}
	for (i = 0; i < schedule->time_num; i++) {
		if (schedule->times[i] > VIDEO_PFM_SEC_PER_DAY - 1) {
			avftr_log_err("[PFM] Exceed maximum seconds per day(%d) vs expected(%d)!",
			              VIDEO_PFM_SEC_PER_DAY - 1, schedule->times[i]);
			return -1;
		}
	}
	if (schedule->regisBg_feed_interval > VIDEO_PFM_REGIS_INTERVAL_MAX) {
		avftr_log_err("[PFM] Exceed maximum registration-feeding interval(%d) vs expected(%d)!",
		              VIDEO_PFM_REGIS_INTERVAL_MAX, schedule->regisBg_feed_interval);
	}
	return 0;
}