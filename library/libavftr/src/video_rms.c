#include "video_rms.h"

#include <stdio.h>

#include "avftr_log.h"
#include "mpi_dev.h"

#include "avftr.h"
#include "avftr_common.h"

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static int findRmsCtx(MPI_WIN idx, VIDEO_RMS_CTX_S *rms_ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_RMS_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && rms_ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1 && !rms_ctx[i].en) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static void addRmsOffset(MPI_IVA_RMS_PARAM_S *rms_param, MPI_RECT_POINT_S *roi, MPI_RECT_S *win)
{
	int blk_w, blk_h;
	int cnt = 0;
	int x = 0;
	int y = 0;

	blk_w = win->width / rms_param->split_x;
	blk_h = win->height / rms_param->split_y;
	for (y = 0; y < rms_param->split_y; y++) {
		for (x = 0; x < rms_param->split_x; x++) {
			roi[cnt] = (MPI_RECT_POINT_S){.sx = x * blk_w + win->x,
				                      .sy = y * blk_h + win->y,
				                      .ex = (x + 1) * blk_w - 1 + win->x,
				                      .ey = (y + 1) * blk_h - 1 + win->y };
			cnt++;
		}
	}
}

/**
 * @brief Invoke callback function when alarm condition is satisfied.
 * @param[in] idx         video channel index.
 * @param[in] reg_list    region list detected result.
 * @see VIDEO_FTR_getRmsRes()
 * @retval none.
 */
#define RMS_ALARM_THR 128
static void genRmsAlarm(VIDEO_RMS_CTX_S *rms_ctx, const MPI_IVA_RMS_REG_LIST_S *reg_list)
{
	int i;
	for (i = 0; (UINT32)i < reg_list->reg_cnt; i++) {
		if (reg_list->reg[i].conf > RMS_ALARM_THR) {
			rms_ctx->cb();
			return;
		}
	}
}

/**
 * @brief Get predefined metadata format for Multiplayer.
 * @param[in]  src_idx         mpi win index of source window
 * @param[in]  dst_idx         mpi win index of destination window
 * @param[in]  src_rect        source window
 * @param[in]  dst_rect        destination window
 * @param[in] roi         roi of region list
 * @param[in] reg_list    region list detected result.
 * @param[in] str         metadata string buffer.
 * @see VIDEO_FTR_getRmsRes()
 * @retval length of metadata.
 */
static int getRmsMeta(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                      const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const MPI_RECT_POINT_S *roi,
                      const MPI_IVA_RMS_REG_LIST_S *reg_list, char *str)
{
	int offset = 0;
	unsigned int i = 0;
	MPI_RECT_POINT_S copy_roi[MPI_IVA_RMS_MAX_REG_NUM];

	for (i = 0; i < reg_list->reg_cnt; i++) {
		copy_roi[i] = roi[i];
	}

	if (src_idx.value != dst_idx.value) {
		for (i = 0; i < reg_list->reg_cnt; i++) {
			rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &copy_roi[i]);
		}
	}

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "<RMS>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "\"rms\":[ ");
#endif /* !IVA_FORMAT_XML */
	for (i = 0; i < reg_list->reg_cnt; i++) {
		offset += sprintf(&str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		                  "<REG ID=\"%d\" RECT=\"%d %d %d %d\" CONF=\"%d\"/>",
#else /* IVA_FORMAT_JSON */
		                  "{\"reg\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"conf\":%d}},",
#endif /* !IVA_FORMAT_XML */
		                  i, copy_roi[i].sx, copy_roi[i].sy, copy_roi[i].ex, copy_roi[i].ey, reg_list->reg[i].conf);
	}
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "</RMS>");
#else /* IVA_FORMAT_JSON */
	offset += (sprintf(&str[offset - 1], "],") - 1);
#endif /* !IVA_FORMAT_XML */
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] none.
 * @see VIDEO_FTR_enableRms()
 * @retval none.
 */
static void alarmEmptyCb()
{
	//avftr_log_err("Please register regional motion sensor alarm callback function.");
	return;
}

/**
 * @brief Get enable status of regional motion sensor.
 * @param[in]  idx        video window index.
 * @see none
 * @retval enable status of regional motion sensor.
 */
int VIDEO_FTR_getRmsStat(MPI_WIN idx, VIDEO_RMS_CTX_S *vftr_rms_ctx)
{
	const int enable_idx = findRmsCtx(idx, vftr_rms_ctx, NULL);

	return enable_idx < 0 ? 0 : vftr_rms_ctx[enable_idx].en;
}

/**
 * @brief Get results of regional motion sensor.
 * @param[in]  idx        video window index.
 * @see none
 * @retval length of metadata.
 */
int VIDEO_FTR_getRmsRes(MPI_WIN idx, int buf_idx)
{
	VIDEO_RMS_CTX_S *vftr_rms_ctx = vftr_res_shm->rms_ctx;
	const int enable_idx = findRmsCtx(idx, vftr_rms_ctx, NULL);

	if (enable_idx < 0 || !vftr_rms_ctx[enable_idx].en) {
		return 0;
	}

	UINT8 i;
	MPI_CHN chn_id = MPI_VIDEO_CHN(idx.dev, idx.chn);
	int ret = 0;
	MPI_IVA_RMS_PARAM_S rms_param;
	MPI_CHN_LAYOUT_S layout_attr;

	ret = MPI_DEV_getChnLayout(chn_id, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Failed to get video channel attributes on chn %u.", idx.chn);
		goto err;
	}

	ret = MPI_IVA_getRegMotSensParam(idx, &rms_param);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Failed to get regional motion sensor parameter on %u.", idx.win);
		goto err;
	}

	MPI_IVA_RMS_REG_LIST_S *reg_list = &vftr_rms_ctx[enable_idx].reg_list[buf_idx];
	ret = MPI_IVA_getRegMotSens(idx, reg_list);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Failed to get regional motion sensor result");
		goto err;
	}
	genRmsAlarm(&vftr_rms_ctx[enable_idx], reg_list);

	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			break;
		}
	}
	if (i == layout_attr.window_num) {
		avftr_log_err("Window %d does not exist in channel %d", idx.win, idx.chn);
		goto err;
	}
	addRmsOffset(&rms_param, vftr_rms_ctx[enable_idx].roi, &layout_attr.window[i]);

	return 0;

err:
	return -1;
}

/**
 * @brief Get predefined metadata format for Multiplayer.
 * @param[in] src_idx  mpi win index of source window
 * @param[in] dst_idx  mpi win index of destination window
 * @param[in] src_rect source window
 * @param[in] dst_rect destination window
 * @param[in] reg_list    region list detected result.
 * @param[in] str         metadata string buffer.
 * @see VIDEO_FTR_getRmsRes()
 * @retval length of metadata.
 */
int VIDEO_FTR_transRmsRes(VIDEO_RMS_CTX_S *vftr_rms_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                          const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str, int buf_idx)
{
	const int enable_idx = findRmsCtx(src_idx, vftr_rms_ctx, NULL);
	if (enable_idx < 0 || !vftr_rms_ctx[enable_idx].en) {
		return 0;
	}

	return getRmsMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi, vftr_rms_ctx[enable_idx].roi,
	                  &vftr_rms_ctx[enable_idx].reg_list[buf_idx], str);
}

/**
 * @brief Enable regional motion sensor.
 * @param[in]  idx        video window index.
 * @see VIDEO_FTR_disableRms
 * @retval MPI_SUCCESS                 success.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_enableRms(MPI_WIN idx)
{
	VIDEO_RMS_CTX_S *vftr_rms_ctx = vftr_res_shm->rms_ctx;

	int empty_idx;
	int set_idx = findRmsCtx(idx, vftr_rms_ctx, &empty_idx);
	int enable_idx;
	int ret;

	if (set_idx >= 0) {
		enable_idx = set_idx;
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		vftr_rms_ctx[enable_idx].idx = idx;
	} else {
		avftr_log_err("RMS detection enable failed on win %u.", idx.win);
		goto err;
	}

	if (!vftr_rms_ctx[enable_idx].en) {
		ret = MPI_IVA_enableRegMotSens(idx);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Enable regional motion sensor on win %u failed.", idx.win);
			goto err;
		}
		if (vftr_rms_ctx[enable_idx].cb == NULL) {
			//avftr_log_err("Regional motion sensor alarm callback function is not registered on win %u.",
			//          idx.win);
			vftr_rms_ctx[enable_idx].cb = alarmEmptyCb;
		}
		vftr_rms_ctx[enable_idx].en = 1;
	}
	return 0;
err:
	return -1;
}

/**
 * @brief Disable regional motion sensor.
 * @param[in]  idx        video window index.
 * @see VIDEO_FTR_enableRms
 * @retval MPI_SUCCESS                 success.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_disableRms(MPI_WIN idx)
{
	VIDEO_RMS_CTX_S *vftr_rms_ctx = vftr_res_shm->rms_ctx;

	int enable_idx = findRmsCtx(idx, vftr_rms_ctx, NULL);

	if (enable_idx < 0) {
		return 0;
	}

	if (vftr_rms_ctx[enable_idx].en) {
		INT32 ret;
		ret = MPI_IVA_disableRegMotSens(idx);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Disable regional motion sensor on win %u failed.", idx.win);
			goto err;
		}
		vftr_rms_ctx[enable_idx].en = 0;
	}
	return 0;
err:
	return -1;
}

/**
 * @brief Get parameters of regional motion sensor.
 * @param[in]  idx        video window index.
 * @param[out] param      regional motion sensor parameters.
 * @see VIDEO_FTR_setRmsParam
 * @retval MPI_SUCCESS                 success.
 * @retval MPI_ERR_DEV_NULL_POINTER    input pointer is NULL.
 * @retval MPI_ERR_DEV_INVALID_WIN_ID  invalid video window index.
 * @retval MPI_ERR_DEV_INVALID_CHN_ID  invalid video channel index.
 * @retval MPI_ERR_DEV_INVALID_DEV_ID  invalid device index.
 * @retval MPI_ERR_DEV_NOT_EXIST       device/channel not exist.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_getRmsParam(MPI_WIN idx, MPI_IVA_RMS_PARAM_S *param)
{
	return MPI_IVA_getRegMotSensParam(idx, param);
}

/**
 * @brief Set parameters of regional motion sensor.
 * @param[in]  idx        video window index.
 * @param[in]  param      regional motion sensor parameters.
 * @see VIDEO_FTR_getRmsParam
 * @retval MPI_SUCCESS                 success.
 * @retval MPI_ERR_DEV_NULL_POINTER    input pointer is NULL.
 * @retval MPI_ERR_DEV_INVALID_WIN_ID  invalid video window index.
 * @retval MPI_ERR_DEV_INVALID_CHN_ID  invalid video channel index.
 * @retval MPI_ERR_DEV_INVALID_DEV_ID  invalid device index.
 * @retval MPI_ERR_DEV_INVALID_PARAM   invalid parameters.
 * @retval MPI_ERR_DEV_NOT_EXIST       device/channel not exist.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_setRmsParam(MPI_WIN idx, const MPI_IVA_RMS_PARAM_S *param)
{
	VIDEO_RMS_CTX_S *vftr_rms_ctx = vftr_res_shm->rms_ctx;
	int empty_idx;
	int set_idx = findRmsCtx(idx, vftr_rms_ctx, &empty_idx);

	if (set_idx < 0 && empty_idx >= 0) {
		vftr_rms_ctx[empty_idx].idx = idx;
	}

	return MPI_IVA_setRegMotSensParam(idx, param);
}

/**
 * @brief Register alarm callback function of regional motion sensor.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none
 * @retval MPI_SUCCESS                 success.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_regRmsCallback(MPI_WIN idx, const VIDEO_FTR_RMS_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Pointer to regional motion sensor alarm callback function should not be NULL.");
		return -1;
	}

	VIDEO_RMS_CTX_S *vftr_rms_ctx = vftr_res_shm->rms_ctx;
	int empty_idx, enable_idx;
	int set_idx = findRmsCtx(idx, vftr_rms_ctx, &empty_idx);

	if (set_idx >= 0) {
		enable_idx = set_idx;
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		vftr_rms_ctx[empty_idx].idx = idx;
	} else {
		avftr_log_err("No available seting of RMS on the win %u.", idx.win);
		goto err;
	}

	vftr_rms_ctx[enable_idx].cb = alarm_cb_fptr;

	return 0;
err:
	return -1;
}
