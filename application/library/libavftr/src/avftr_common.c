#include "mpi_base_types.h"
#include <limits.h>
#include <stdio.h>
#include <errno.h>

#include "avftr_common.h"
#include "mpi_iva.h"
#include "avftr_log.h"

static int g_vftr_yavg_resource_cnt = AVFTR_VIDEO_YAVG_ROI_RESOURCE_NUM;

int checkMpiDevValid(const MPI_WIN idx)
{
	int ret = 0;

	UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	UINT32 chn_idx = MPI_GET_VIDEO_CHN(idx);
	//UINT32 win_idx = MPI_GET_VIDEO_WIN(idx);

	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = { { .dev = idx.dev, .chn = idx.chn, .dummy1 = 0, .dummy0 = 0 } };

	ret = MPI_DEV_queryChnState(chn, &chn_stat);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Query CHN(%d, %d) state failed. err: %d", dev_idx, chn_idx, ret);
		return ret;
	}

	if (!MPI_STATE_IS_ADDED(chn_stat.status)) {
		avftr_log_err("CHN(%d, %d) is not added.", dev_idx, chn_idx);
		return ENODEV;
	}

	return 0;
}

int getMpiSize(const MPI_WIN idx, MPI_SIZE_S *res)
{
	INT32 ret;
	INT32 i;
	UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);
	UINT32 chn_idx = MPI_GET_VIDEO_CHN(idx);
	UINT32 win_idx = MPI_GET_VIDEO_WIN(idx);
	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = { { .dev = idx.dev, .chn = idx.chn, .dummy1 = 0, .dummy0 = 0 } };

	ret = MPI_DEV_queryChnState(chn, &chn_stat);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Query CHN(%d, %d) state failed. err: %d", dev_idx, chn_idx, ret);
		return ret;
	}

	if (!MPI_STATE_IS_ADDED(chn_stat.status)) {
		avftr_log_err("CHN (%d, %d) is not added.", dev_idx, chn_idx);
		return ENODEV;
	}

	MPI_CHN_LAYOUT_S layout_attr;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get CHN %d layout attributes failed. err: %d", chn_idx, ret);
		return ret;
	}

	/* FIXME: check window state */
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			res->width = layout_attr.window[i].width;
			res->height = layout_attr.window[i].height;
			break;
		}
	}
	if (i == layout_attr.window_num) {
		avftr_log_err("Invalid video window index %d from video channel %d", win_idx, chn_idx);
		return EINVAL;
	}

	return 0;
}

int getRoi(const MPI_WIN idx, MPI_RECT_S *roi)
{
	MPI_CHN_LAYOUT_S chn_layout;
	MPI_CHN chn = (MPI_CHN){ .value = idx.value };
	int i, ret;

	ret = MPI_DEV_getChnLayout(chn, &chn_layout);
	if (ret != MPI_SUCCESS) {
		avftr_log_err("Get CHN (%d, %d) layout failed. err: %d", chn.dev, chn.chn, ret);
		return -1;
	}

	for (i = 0; i < chn_layout.window_num; i++) {
		if (idx.value == chn_layout.win_id[i].value) {
			break;
		}
	}

	const UINT32 win_idx = MPI_GET_VIDEO_WIN(idx);
	if (i == chn_layout.window_num) {
		avftr_log_err("Invalid WIN idx (%d, %d, %d)", idx.dev, idx.chn, win_idx);
		return -1;
	}

	*roi = chn_layout.window[i];

	return 0;
}

int vftrYAvgResDec(void)
{
	if (g_vftr_yavg_resource_cnt > 0) {
		g_vftr_yavg_resource_cnt--;
		return 0;
	}

	return -1;
}

void vftrYAvgResInc(void)
{
	g_vftr_yavg_resource_cnt = MIN(g_vftr_yavg_resource_cnt + 1, AVFTR_VIDEO_YAVG_ROI_RESOURCE_NUM);
}

void rescaleMpiRectPoint(const MPI_RECT_S *src_rect, const MPI_RECT_S *dst_rect, const MPI_RECT_S *src_roi,
                         const MPI_RECT_S *dst_roi, MPI_RECT_POINT_S *roi)
{
	INT32 roi_sx = roi->sx - src_rect->x;
	INT32 roi_sy = roi->sy - src_rect->y;
	INT32 roi_ex = roi->ex - src_rect->x;
	INT32 roi_ey = roi->ey - src_rect->y;

	INT32 src_roi_width = src_roi->width;
	INT32 src_roi_height = src_roi->height;
	INT32 src_roi_x = src_roi->x;
	INT32 src_roi_y = src_roi->y;

	INT32 dst_roi_width = dst_roi->width;
	INT32 dst_roi_height = dst_roi->height;
	INT32 dst_roi_x = dst_roi->x;
	INT32 dst_roi_y = dst_roi->y;

	INT32 src_rect_width = src_rect->width;
	INT32 src_rect_height = src_rect->height;

	INT32 dst_rect_width = dst_rect->width;
	INT32 dst_rect_height = dst_rect->height;
	INT32 dst_rect_x = dst_rect->x;
	INT32 dst_rect_y = dst_rect->y;

	roi_sx = (roi_sx * src_roi_width + (src_rect_width >> 1)) / src_rect_width + src_roi_x - dst_roi_x;
	roi_sx = (roi_sx * dst_rect_width + (dst_roi_width >> 1)) / dst_roi_width;
	roi_ex = (roi_ex * src_roi_width + (src_rect_width >> 1)) / src_rect_width + src_roi_x - dst_roi_x;
	roi_ex = (roi_ex * dst_rect_width + (dst_roi_width >> 1)) / dst_roi_width;

	roi_sy = (roi_sy * src_roi_height + (src_rect_height >> 1)) / src_rect_height + src_roi_y - dst_roi_y;
	roi_sy = (roi_sy * dst_rect_height + (dst_roi_height >> 1)) / dst_roi_height;
	roi_ey = (roi_ey * src_roi_height + (src_rect_height >> 1)) / src_rect_height + src_roi_y - dst_roi_y;
	roi_ey = (roi_ey * dst_rect_height + (dst_roi_height >> 1)) / dst_roi_height;

	roi_sx += dst_rect_x;
	roi_sy += dst_rect_y;
	roi_ex += dst_rect_x;
	roi_ey += dst_rect_y;

	roi->sx = CLAMP(roi_sx, SHRT_MIN, SHRT_MAX);
	roi->sy = CLAMP(roi_sy, SHRT_MIN, SHRT_MAX);
	roi->ex = CLAMP(roi_ex, SHRT_MIN, SHRT_MAX);
	roi->ey = CLAMP(roi_ey, SHRT_MIN, SHRT_MAX);
}

UINT8 cropRect(const MPI_RECT_S *roi, const MPI_IVA_OBJ_ATTR_S *src_obj, MPI_IVA_OBJ_ATTR_S *dst_obj)
{
	const MPI_RECT_POINT_S *src_rect = &src_obj->rect;
	if (roi->x > src_rect->ex || roi->y > src_rect->ey || roi->x + roi->width <= src_rect->sx ||
	    roi->y + roi->height <= src_rect->sy) {
		return 0;
	}
	if (src_obj != dst_obj) {
		*dst_obj = *src_obj;
	}
	MPI_RECT_POINT_S *dst_rect = &dst_obj->rect;
	dst_rect->sx = MAX(dst_rect->sx, roi->x);
	dst_rect->sy = MAX(dst_rect->sy, roi->y);
	dst_rect->ex = MIN(dst_rect->ex, roi->x + roi->width - 1);
	dst_rect->ey = MIN(dst_rect->ey, roi->y + roi->height - 1);
	return 1;
}
