#include "video_od.h"

#include <stdio.h>

#include "avftr_log.h"
#include "mpi_dev.h"

#include "avftr_shd.h"
#include "avftr.h"
#include "avftr_common.h"
//#include "video_od_shake.h"

/* NOTE: Need "Add" and "Remove" functionality for supporting more than one setting. */

#define VIDEO_OD_SHAKE_DETECT

extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static int findOdCtx(MPI_WIN idx, VIDEO_OD_CTX_S *od_ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_OD_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && od_ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1 && !(od_ctx[i].en || od_ctx[i].en_implicit)) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static void addOdOffset(MPI_WIN idx, VIDEO_FTR_OBJ_LIST_S *src, VIDEO_FTR_OBJ_LIST_S *dest)
{
	UINT32 x = 0;
	UINT32 y = 0;
	INT32 i;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(idx.dev, idx.chn);
	int ret;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		return;
	}

	//FIXME: Check win idx exist
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
	MPI_IVA_OBJ_LIST_S *src_ol = &src->basic_list;
	MPI_IVA_OBJ_LIST_S *dest_ol = &dest->basic_list;
	MPI_IVA_OBJ_ATTR_S *src_obj;
	MPI_IVA_OBJ_ATTR_S *dest_obj;

	memcpy(dest->obj_attr, src->obj_attr, sizeof(VIDEO_FTR_OBJ_ATTR_S) * src_ol->obj_num);
	dest_ol->timestamp = src_ol->timestamp;
	dest_ol->obj_num = src_ol->obj_num;
	for (i = 0; i < src_ol->obj_num; i++) {
		src_obj = &src_ol->obj[i];
		dest_obj = &dest_ol->obj[i];

		dest_obj->id = src_obj->id;
		dest_obj->life = src_obj->life;
		dest_obj->mv.x = src_obj->mv.x;
		dest_obj->mv.y = src_obj->mv.y;

		dest_obj->rect.sx = src_obj->rect.sx + x;
		dest_obj->rect.sy = src_obj->rect.sy + y;
		dest_obj->rect.ex = src_obj->rect.ex + x;
		dest_obj->rect.ey = src_obj->rect.ey + y;
	}
}

/**
 * @brief Invoke callback function when alarm condition is satisfied.
 * @param[in] idx     video window index.
 * @param[in] list    object list detected result.
 * @see VIDEO_FTR_getOdRes()
 * @retval none.
 */
static void genOdAlarm(MPI_WIN idx __attribute__((unused)), const VIDEO_FTR_OBJ_LIST_S *list __attribute__((unused)))
{
	return;
	//FIXME: add vftr_od_ctx[dev_idx][chn_idx].cb() if alarm is needed
}

static void cropObjList(const MPI_RECT_S *rect, MPI_IVA_OBJ_LIST_S *list)
{
	int croped_obj_num = 0;
	for (INT32 i = 0; i < list->obj_num; i++) {
		if (cropRect(rect, &list->obj[i], &list->obj[croped_obj_num])) {
			croped_obj_num++;
		}
	}
	list->obj_num = croped_obj_num;
}

/**
 * @brief Get predefined metadata format.
 * @param[in] list    object list detected result.
 * @param[in]  src_idx         mpi win index of source window
 * @param[in]  dst_idx         mpi win index of destination window
 * @param[in]  src_rect        source window
 * @param[in]  dst_rect        destination window
 * @param[in] str     metadata string buffer.
 * @see VIDEO_FTR_getOdRes()
 * @retval length of metadata string.
 */
static int getOdMeta(MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect,
                     const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, const VIDEO_FTR_OBJ_LIST_S *vftr_list,
                     INT32 en_crop_outside_obj, char *str)
{
	int offset = 0;
	int i = 0;
	const MPI_IVA_OBJ_LIST_S *list = &vftr_list->basic_list;
	const VIDEO_FTR_OBJ_ATTR_S *attr = vftr_list->obj_attr;
	MPI_IVA_OBJ_LIST_S dst_list = *list;

	if (src_idx.value != dst_idx.value) {
		for (INT32 i = 0; i < dst_list.obj_num; i++) {
			rescaleMpiRectPoint(src_rect, dst_rect, src_roi, dst_roi, &dst_list.obj[i].rect);
		}
	}

	if (en_crop_outside_obj) {
		cropObjList(dst_rect, &dst_list);
	}

#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "<OD>");
#else /* IVA_FORMAT_JSON */
	offset += sprintf(&str[offset], "\"od\":[ ");
#endif /* !IVA_FORMAT_XML */
	for (i = 0; i < dst_list.obj_num; i++) {
		offset += sprintf(
		        &str[offset],
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
		        "<OBJ ID=\"%d\" RECT=\"%d %d %d %d\" VEL=\"%d %d\" CAT=\"%s\" CONF=\"%s\" SHAKING=\"%d\" LIFE=\"%d\"/>",
#else /* IVA_FORMAT_JSON */
		        "{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"vel\":[%d,%d],\"cat\":\"%s\",\"conf\":\"%s\",\"shaking\":%d,\"life\":%d}},",
#endif /* !IVA_FORMAT_XML */
		        dst_list.obj[i].id, dst_list.obj[i].rect.sx, dst_list.obj[i].rect.sy, dst_list.obj[i].rect.ex,
		        dst_list.obj[i].rect.ey, dst_list.obj[i].mv.x, dst_list.obj[i].mv.y, attr[i].cat, attr[i].conf,
		        attr[i].shaking, dst_list.obj[i].life);
	}
#ifdef IVA_FORMAT_XML /* IVA_FORMAT_XML */
	offset += sprintf(&str[offset], "</OD>");
#else /* IVA_FORMAT_JSON */
	offset += (sprintf(&str[offset - 1], "],") - 1);
#endif /* !IVA_FORMAT_XML */
	return offset;
}

/**
 * @brief Empty callback function for initialization.
 * @param[in] none.
 * @see VIDEO_FTR_enableOd()
 * @retval none.
 */
static void alarmEmptyCb()
{
	avftr_log_notice("Please register object detection alarm callback function.");
}

/**
 * @brief Get enable status of object detection.
 * @param[in]  idx       video window index.
 * @see none
 * @retval enable status of object detection.
 */
int VIDEO_FTR_getOdStat(MPI_WIN idx, VIDEO_OD_CTX_S *vftr_od_ctx)
{
	const int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);

	return enable_idx >= 0 ? (vftr_od_ctx[enable_idx].en > 0) : 0;
}

/**
 * @brief Get results of object detection.
 * @param[in]  idx        video window index.
 * @param[in]  obj_list    object list.
 * @param[out] str         metadata string buffer.
 * @see none
 * @retval length of metadata.
 */
int VIDEO_FTR_getOdRes(MPI_WIN idx, VIDEO_FTR_OBJ_LIST_S *raw_ol, int buf_idx)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	const int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);

	if (enable_idx < 0 || !vftr_od_ctx[enable_idx].en) {
		return 0;
	}

	VIDEO_FTR_OBJ_LIST_S *obj_list = &vftr_od_ctx[enable_idx].ol[buf_idx];
	genOdAlarm(idx, raw_ol);
	addOdOffset(idx, raw_ol, obj_list);

	return 0;
}

/**
 * @brief Get predefined metadata format.
 * @param[in] list    object list detected result.
 * @param[in]  src_idx         mpi win index of source window
 * @param[in]  dst_idx         mpi win index of destination window
 * @param[in]  src_rect        source window
 * @param[in]  dst_rect        destination window
 * @param[in] str     metadata string buffer.
 * @see VIDEO_FTR_getOdRes()
 * @retval length of metadata string.
 */
int VIDEO_FTR_transOdRes(VIDEO_OD_CTX_S *vftr_od_ctx, MPI_WIN src_idx, MPI_WIN dst_idx, const MPI_RECT_S *src_rect,
                         const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi, const MPI_RECT_S *dst_roi, char *str,
                         int buf_idx)
{
	int enable_idx = findOdCtx(src_idx, vftr_od_ctx, NULL);

	if (enable_idx < 0) {
		goto err;
	}

	if (vftr_od_ctx[enable_idx].en) {
		// Add enable check?
		return getOdMeta(src_idx, dst_idx, src_rect, dst_rect, src_roi, dst_roi,
		                 &vftr_od_ctx[enable_idx].ol[buf_idx], vftr_od_ctx[enable_idx].en_crop_outside_obj,
		                 str);
	}
err:
	return 0;
}

/**
 * @brief Enable object detection.
 * @param[in]  idx          video window index.
 * @see VIDEO_FTR_disableOd()
 * @retval MPI_SUCCESS      success.
 * @retval -1      unexpected fail.
 */
int VIDEO_FTR_enableOd(const MPI_WIN idx)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;

	int empty_idx;
	int set_idx = findOdCtx(idx, vftr_od_ctx, &empty_idx);
	int enable_idx;
	INT32 ret;

	if (set_idx >= 0) {
		enable_idx = set_idx;
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		vftr_od_ctx[enable_idx].idx.value = idx.value;
	} else {
		avftr_log_err("Object detection enable failed on win %u.", idx.win);
		goto err;
	}

	if (!vftr_od_ctx[enable_idx].en) {
		ret = VIDEO_FTR_enableOd_implicit(idx);
		if (ret != MPI_SUCCESS) {
			goto err;
		}
	}
	vftr_od_ctx[enable_idx].en = 1;

	return 0;
err:
	return -1;
}

/**
 * @brief Disable object detection.
 * @details Get parameters from buffer.
 * @param[in]  idx          video window index.
 * @see VIDEO_FTR_enableOd()
 * @retval MPI_SUCCESS      success.
 * @retval -1      unexpected fail.
 */
int VIDEO_FTR_disableOd(const MPI_WIN idx)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;

	int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);

	if (enable_idx < 0) {
		return 0;
	}

	if (vftr_od_ctx[enable_idx].en == 1) {
		INT32 ret = 0;
		ret = VIDEO_FTR_disableOd_implicit(idx);
		if (ret != 0) {
			avftr_log_err("Disable object detection on win %d failed!", idx.win);
			goto err;
		}
	}

	vftr_od_ctx[enable_idx].en = 0;

	return 0;
err:
	return -1;
}

/**
 * @brief Enable object detection implicitly.
 * @param[in]  idx          video window index.
 * @see VIDEO_FTR_disableOd()
 * @retval MPI_SUCCESS      success.
 * @retval -1      unexpected fail.
 */
int VIDEO_FTR_enableOd_implicit(const MPI_WIN idx)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	UINT8 win_idx;
	int empty_idx;
	int set_idx = findOdCtx(idx, vftr_od_ctx, &empty_idx);

	int enable_idx;

	if (set_idx >= 0) {
		enable_idx = set_idx;
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		vftr_od_ctx[empty_idx].idx.value = idx.value;
	} else {
		avftr_log_err("Object detection enable failed on win(%u, %u, %u).", idx.dev, idx.chn, idx.win);
		goto err;
	}

	if (!vftr_od_ctx[enable_idx].en_implicit) {
		INT32 ret;
		ret = vftrYAvgResDec();
		if (ret != 0) {
			avftr_log_err("All MPI YAVG ROI Resource are being used!");
			goto err;
		}
		ret = MPI_IVA_enableObjDet(idx);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Enable IVA object detect on win %u failed.", idx.win);
			goto err;
		}
		if (vftr_od_ctx[enable_idx].cb == NULL) {
			//avftr_log_err("Object detection alarm callback function is not registered on win %u.", idx.win);
			vftr_od_ctx[enable_idx].cb = alarmEmptyCb;
		}
		MPI_CHN chn_id = MPI_VIDEO_CHN(idx.dev, idx.chn);
		MPI_CHN_ATTR_S chn_attr;
		ret = MPI_DEV_getChnAttr(chn_id, &chn_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get channel attr on chn %u.", chn_id.chn);
			goto err;
		}

		MPI_CHN_LAYOUT_S layout_attr;
		ret = MPI_DEV_getChnLayout(chn_id, &layout_attr);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("Failed to get channel layout on chn %u.", chn_id.chn);
			goto err;
		}

		MPI_RECT_POINT_S *bd = &vftr_od_ctx[enable_idx].bdry;
		for (win_idx = 0; win_idx < layout_attr.window_num; win_idx++) {
			if (idx.value == layout_attr.win_id[win_idx].value) {
				break;
			}
		}
		if (win_idx == layout_attr.window_num) {
			avftr_log_err("Window %d does not exist in channel %d", idx.win, idx.chn);
			goto err;
		}
		MPI_RECT_S *win = &layout_attr.window[win_idx];
		MPI_SIZE_S *chn = &chn_attr.res;
		bd->sx = win->x == 0 ? -1 : 0;
		bd->sy = win->y == 0 ? -1 : 0;
		bd->ex = win->width + win->x == (chn->width) ? -1 : win->width - 1;
		bd->ey = win->height + win->y == (chn->height) ? -1 : win->height - 1;
	}
	vftr_od_ctx[enable_idx].en_implicit++;
	return 0;
err:
	return -1;
}

/**
 * @brief Disable object detection implicitly..
 * @param[in]  idx          video window index.
 * @see VIDEO_FTR_enableOd()
 * @retval MPI_SUCCESS      success.
 * @retval -1      unexpected fail.
 */
int VIDEO_FTR_disableOd_implicit(const MPI_WIN idx)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);
	INT32 ret;

	if (enable_idx < 0) {
		avftr_log_err("OD for the win %u is not found", idx.win);
		goto err;
	}

	if (vftr_od_ctx[enable_idx].en_implicit == 1) {
		vftrYAvgResInc();
		vftr_od_ctx[enable_idx].en_implicit--;
		ret = MPI_IVA_disableObjDet(idx);
		if (ret != MPI_SUCCESS) {
			vftr_od_ctx[enable_idx].en_implicit++;
			avftr_log_err("Disable object detect on win %u failed. err: %d", idx.win, ret);
			goto err;
		}
	} else if (!vftr_od_ctx[enable_idx].en_implicit) {
	} else {
		vftr_od_ctx[enable_idx].en_implicit--;
	}
	return 0;
err:
	return -1;
}

/**
 * @brief Get parameters of object detection.
 * @details Get parameters from buffer.
 * @param[in]  idx        video window index.
 * @param[out] param      object detection parameters.
 * @see VIDEO_FTR_setOdParam
 * @retval MPI_SUCCESS                 success.
 * @retval MPI_ERR_DEV_NULL_POINTER    input pointer is NULL.
 * @retval MPI_ERR_DEV_INVALID_WIN     invalid video window index.
 * @retval MPI_ERR_DEV_INVALID_CHN_ID  invalid video channel index.
 * @retval MPI_ERR_DEV_INVALID_DEV_ID  invalid device index.
 * @retval MPI_ERR_DEV_NOT_EXIST       device/channel not exist.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_getOdParam(MPI_WIN idx, VIDEO_FTR_OD_PARAM_S *param)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);

	if (enable_idx < 0) {
		//avftr_log_err("get Od param failed");
		return 0;
	}

	int ret = MPI_IVA_getObjParam(idx, &param->od_param);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_IVA_getObjMotorParam(idx, &param->od_motor_param);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	param->en_shake_det = vftr_od_ctx[enable_idx].en_shake_det;
	param->en_crop_outside_obj = vftr_od_ctx[enable_idx].en_crop_outside_obj;

	return ret;
}

/**
 * @brief Set parameters of object detection.
 * @param[in]  idx             video window index.
 * @param[in]  param           object detection parameters.
 * @see VIDEO_FTR_getOdParam
 * @retval MPI_SUCCESS                 success.
 * @retval MPI_ERR_DEV_NULL_POINTER    input pointer is NULL.
 * @retval MPI_ERR_DEV_INVALID_WIN     invalid video window index.
 * @retval MPI_ERR_DEV_INVALID_CHN     invalid video channel index.
 * @retval MPI_ERR_DEV_INVALID_DEV     invalid device index.
 * @retval MPI_ERR_DEV_INVALID_PARAM   invalid parameters.
 * @retval MPI_ERR_DEV_NOT_EXIST       device/channel not exist.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_setOdParam(MPI_WIN idx, const VIDEO_FTR_OD_PARAM_S *param)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	int empty_idx;
	int set_idx = findOdCtx(idx, vftr_od_ctx, &empty_idx);

	if (set_idx < 0 && empty_idx >= 0) {
		vftr_od_ctx[empty_idx].idx.value = idx.value;
		set_idx = empty_idx;
	}

	int ret = MPI_IVA_setObjParam(idx, &param->od_param);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = MPI_IVA_setObjMotorParam(idx, &param->od_motor_param);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	vftr_od_ctx[set_idx].en_shake_det = param->en_shake_det;
	vftr_od_ctx[set_idx].en_crop_outside_obj = param->en_crop_outside_obj;

	return ret;
}

/**
 * @brief Register alarm callback function of object detection.
 * @param[in]  idx             video window index.
 * @param[in]  alarm_cb_fptr   function pointer of callback function.
 * @see none
 * @retval MPI_SUCCESS                 success.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_regOdCallback(MPI_WIN idx, const VIDEO_FTR_OD_ALARM_CB alarm_cb_fptr)
{
	if (alarm_cb_fptr == NULL) {
		avftr_log_err("Args should not be NULL.");
		return -1;
	}

	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	int empty_idx;
	int enable_idx;
	int set_idx = findOdCtx(idx, vftr_od_ctx, &empty_idx);

	if (set_idx >= 0) {
		enable_idx = set_idx;
	} else if (empty_idx >= 0) {
		enable_idx = empty_idx;
		vftr_od_ctx[empty_idx].idx.value = idx.value;
	} else {
		avftr_log_err("No available setting of OD on the win %u.", idx.win);
		goto err;
	}

	vftr_od_ctx[enable_idx].cb = alarm_cb_fptr;

	return 0;
err:
	return -1;
}

/**
 * @brief Get object list from object detection.
 * @param[in]  idx             video window index.
 * @param[in]  timestamp       time stamp.
 * @param[out] obj_list        object list.
 * @see none
 * @retval MPI_SUCCESS                 success.
 * @retval -1                 unexpected fail.
 */
int VIDEO_FTR_getObjList(MPI_WIN idx, UINT32 timestamp, VIDEO_FTR_OBJ_LIST_S *obj_list)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	const int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);
	int ret = 0;

	if (enable_idx < 0) {
		goto err;
	}

	if (vftr_od_ctx[enable_idx].en || vftr_od_ctx[enable_idx].en_implicit) {
		ret = MPI_IVA_getBitStreamObjList(idx, timestamp, &obj_list->basic_list);
		if (ret != MPI_SUCCESS) {
			avftr_log_err("MPI_IVA_getBitStreamObjList() failed. err: %d", ret);
			goto err;
		}
		char *cat;
		int i;

		MPI_RECT_POINT_S *bd = &vftr_od_ctx[enable_idx].bdry;
		MPI_RECT_POINT_S *obj;
		MPI_RECT_POINT_S *final_obj;
		int obj_cnt = 0;

		for (i = 0; i < obj_list->basic_list.obj_num; i++) {
			/* Initialize object attr */
			cat = obj_list->obj_attr[obj_cnt].cat;
			INT32 cat_int32 = obj_list->basic_list.obj[obj_cnt].cat;
			if (cat_int32 == 0) {
				strncpy(cat, "", VFTR_OBJ_CAT_LEN - 1);
			} else if (cat_int32 == 1) {
				strncpy(cat, "human", VFTR_OBJ_CAT_LEN - 1);
			} else if (cat_int32 == 2) {
				strncpy(cat, "car", VFTR_OBJ_CAT_LEN - 1);
			} else if (cat_int32 == 3) {
				strncpy(cat, "pet", VFTR_OBJ_CAT_LEN - 1);
			} else {
				strncpy(cat, "unknown", VFTR_OBJ_CAT_LEN - 1);
			}

			obj_list->obj_attr[obj_cnt].shaking = 0;

			/* Limit OL boundary */
			obj = &obj_list->basic_list.obj[i].rect;
			final_obj = &obj_list->basic_list.obj[obj_cnt].rect;

			if (bd->sx != -1) {
				if (obj->ex < bd->sx) {
					continue;
				}
				final_obj->sx = obj->sx > bd->sx ? obj->sx : bd->sx;
			} else {
				final_obj->sx = obj->sx;
			}

			if (bd->sy != -1) {
				if (obj->ey < bd->sy) {
					continue;
				}
				final_obj->sy = obj->sy > bd->sy ? obj->sy : bd->sy;
			} else {
				final_obj->sy = obj->sy;
			}

			if (bd->ex != -1) {
				if (obj->sx > bd->ex) {
					continue;
				}
				final_obj->ex = obj->ex > bd->ex ? bd->ex : obj->ex;
			} else {
				final_obj->ex = obj->ex;
			}

			if (bd->ey != -1) {
				if (obj->sy > bd->ey) {
					continue;
				}
				final_obj->ey = obj->ey > bd->ey ? bd->ey : obj->ey;
			} else {
				final_obj->ey = obj->ey;
			}

			obj_list->basic_list.obj[obj_cnt].id = obj_list->basic_list.obj[i].id;
			obj_list->basic_list.obj[obj_cnt].life = obj_list->basic_list.obj[i].life;
			obj_list->basic_list.obj[obj_cnt].mv = obj_list->basic_list.obj[i].mv;

			obj_cnt++;
		}

		obj_list->basic_list.obj_num = obj_cnt;

#ifdef VIDEO_OD_SHAKE_DETECT

		VFTR_SHD_STATUS_S sta;
		memset(&sta, 0, sizeof(VFTR_SHD_STATUS_S));

		AVFTR_SHD_detectShake(idx, &obj_list->basic_list, &sta);
		for (i = 0; i < obj_cnt; i++) {
			obj_list->obj_attr[i].shaking = sta.shaking[i];
		}

#endif /* VIDEO_OD_SHAKE_DETECT */
	}

	return 0;
err:
	return -1;
}

#if (0)
/* deprecated */
int VIDEO_FTR_setOdShakedetStat(MPI_WIN idx, int en_shake_det)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);

	if (enable_idx < 0) {
		//avftr_log_err("OD for the win %u is not found", idx.win);
		return 0;
	}

	vftr_od_ctx[enable_idx].en_shake_det = en_shake_det;
	return 0;
}

int VIDEO_FTR_getOdShakedetStat(MPI_WIN idx, int *en_shake_det)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;
	int enable_idx = findOdCtx(idx, vftr_od_ctx, NULL);

	if (enable_idx < 0) {
		//avftr_log_err("OD for the win %u is not found", idx.win);
		return 0;
	}

	*en_shake_det = vftr_od_ctx[enable_idx].en_shake_det;
	return 0;
}
#endif
