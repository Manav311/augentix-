#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "app_view_api.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "mpi_dev.h"

#define STITCH_SENSOR_NUM 2
#define STITCH_TABLE_NUM 3

static void toMpiStitchAttr(MPI_STITCH_ATTR_S *stitch_attr, const AGTX_STITCH_CONF_S *attr)
{
	INT32 i = 0;

	stitch_attr->enable = attr->enable;
	stitch_attr->dft_dist = attr->dft_dist;
	stitch_attr->table_num = attr->dist_tbl_cnt;

	stitch_attr->center[0].x = attr->center_0_x;
	stitch_attr->center[0].y = attr->center_0_y;
	stitch_attr->center[1].x = attr->center_1_x;
	stitch_attr->center[1].y = attr->center_1_y;

	for (i = 0; i < STITCH_TABLE_NUM; i++) {
		stitch_attr->table[i].dist = attr->dist_tbl[i].dist;
		stitch_attr->table[i].ver_disp = attr->dist_tbl[i].ver_disp;
		stitch_attr->table[i].straighten = attr->dist_tbl[i].straighten;
		stitch_attr->table[i].src_zoom = attr->dist_tbl[i].src_zoom;

		stitch_attr->table[i].theta[0] = attr->dist_tbl[i].theta_0;
		stitch_attr->table[i].radius[0] = attr->dist_tbl[i].radius_0;
		stitch_attr->table[i].curvature[0] = attr->dist_tbl[i].curvature_0;
		stitch_attr->table[i].fov_ratio[0] = attr->dist_tbl[i].fov_ratio_0;
		stitch_attr->table[i].ver_scale[0] = attr->dist_tbl[i].ver_scale_0;
		stitch_attr->table[i].ver_shift[0] = attr->dist_tbl[i].ver_shift_0;

		stitch_attr->table[i].theta[1] = attr->dist_tbl[i].theta_1;
		stitch_attr->table[i].radius[1] = attr->dist_tbl[i].radius_1;
		stitch_attr->table[i].curvature[1] = attr->dist_tbl[i].curvature_1;
		stitch_attr->table[i].fov_ratio[1] = attr->dist_tbl[i].fov_ratio_1;
		stitch_attr->table[i].ver_scale[1] = attr->dist_tbl[i].ver_scale_1;
		stitch_attr->table[i].ver_shift[1] = attr->dist_tbl[i].ver_shift_1;
	}
}

static void toMpiLdcAttr(MPI_LDC_ATTR_S *ldc_attr, const AGTX_LDC_CONF_S *ldc_cfg)
{
	ldc_attr->enable = ldc_cfg->enable;
	ldc_attr->view_type = ldc_cfg->view_type;
	ldc_attr->center_offset.x = ldc_cfg->center_x_offset;
	ldc_attr->center_offset.y = ldc_cfg->center_y_offset;
	ldc_attr->ratio = ldc_cfg->ratio;
}

static void toMpiPanoramaAttr(MPI_PANORAMA_ATTR_S *pano_attr, const AGTX_PANORAMA_CONF_S *pano_cfg)
{
	pano_attr->enable = pano_cfg->enable;
	pano_attr->center_offset.x = pano_cfg->center_offset_x;
	pano_attr->center_offset.y = pano_cfg->center_offset_y;
	pano_attr->ldc_ratio = pano_cfg->ldc_ratio;
	pano_attr->radius = pano_cfg->radius;
	pano_attr->curvature = pano_cfg->curvature;
	pano_attr->straighten = pano_cfg->straighten;
}

static void toMpiPanningAttr(MPI_PANNING_ATTR_S *pann_attr, const AGTX_PANNING_CONF_S *pann_cfg)
{
	pann_attr->enable = pann_cfg->enable;
	pann_attr->center_offset.x = pann_cfg->center_offset_x;
	pann_attr->center_offset.y = pann_cfg->center_offset_y;
	pann_attr->ldc_ratio = pann_cfg->ldc_ratio;
	pann_attr->radius = pann_cfg->radius;
	pann_attr->hor_strength = pann_cfg->hor_strength;
	pann_attr->ver_strength = pann_cfg->ver_strength;
}

static void toMpiSurroundAttr(MPI_SURROUND_ATTR_S *surr_attr, const AGTX_SURROUND_CONF_S *surr_cfg)
{
	surr_attr->enable = surr_cfg->enable;
	surr_attr->center_offset.x = surr_cfg->center_offset_x;
	surr_attr->center_offset.y = surr_cfg->center_offset_y;
	surr_attr->ldc_ratio = surr_cfg->ldc_ratio;
	surr_attr->min_radius = surr_cfg->min_radius;
	surr_attr->max_radius = surr_cfg->max_radius;
	surr_attr->rotate = surr_cfg->rotate;
}

static void toAppStitchCfg(MPI_STITCH_ATTR_S *stitch_attr, AGTX_STITCH_CONF_S *stitch_cfg)
{
	INT32 i = 0;

	stitch_cfg->enable = stitch_attr->enable;
	stitch_cfg->dft_dist = stitch_attr->dft_dist;
	stitch_cfg->dist_tbl_cnt = stitch_attr->table_num;

	stitch_cfg->center_0_x = stitch_attr->center[0].x;
	stitch_cfg->center_0_y = stitch_attr->center[0].y;
	stitch_cfg->center_1_x = stitch_attr->center[1].x;
	stitch_cfg->center_1_y = stitch_attr->center[1].y;

	for (i = 0; i < STITCH_TABLE_NUM; i++) {
		stitch_cfg->dist_tbl[i].tbl_idx = i;
		stitch_cfg->dist_tbl[i].dist = stitch_attr->table[i].dist;
		stitch_cfg->dist_tbl[i].ver_disp = stitch_attr->table[i].ver_disp;
		stitch_cfg->dist_tbl[i].straighten = stitch_attr->table[i].straighten;
		stitch_cfg->dist_tbl[i].src_zoom = stitch_attr->table[i].src_zoom;
		stitch_cfg->dist_tbl[i].theta_0 = stitch_attr->table[i].theta[0];
		stitch_cfg->dist_tbl[i].radius_0 = stitch_attr->table[i].radius[0];
		stitch_cfg->dist_tbl[i].curvature_0 = stitch_attr->table[i].curvature[0];
		stitch_cfg->dist_tbl[i].fov_ratio_0 = stitch_attr->table[i].fov_ratio[0];
		stitch_cfg->dist_tbl[i].ver_scale_0 = stitch_attr->table[i].ver_scale[0];
		stitch_cfg->dist_tbl[i].ver_shift_0 = stitch_attr->table[i].ver_shift[0];
		stitch_cfg->dist_tbl[i].theta_1 = stitch_attr->table[i].theta[1];
		stitch_cfg->dist_tbl[i].radius_1 = stitch_attr->table[i].radius[1];
		stitch_cfg->dist_tbl[i].curvature_1 = stitch_attr->table[i].curvature[1];
		stitch_cfg->dist_tbl[i].fov_ratio_1 = stitch_attr->table[i].fov_ratio[1];
		stitch_cfg->dist_tbl[i].ver_scale_1 = stitch_attr->table[i].ver_scale[1];
		stitch_cfg->dist_tbl[i].ver_shift_1 = stitch_attr->table[i].ver_shift[1];
	}

	stitch_cfg->video_dev_idx = 0;
}

static void toAppLdcCfg(MPI_LDC_ATTR_S *ldc_attr, AGTX_LDC_CONF_S *ldc_cfg)
{
	ldc_cfg->enable = ldc_attr->enable;
	ldc_cfg->view_type = ldc_attr->view_type;
	ldc_cfg->center_x_offset = ldc_attr->center_offset.x;
	ldc_cfg->center_y_offset = ldc_attr->center_offset.y;
	ldc_cfg->ratio = ldc_attr->ratio;
	ldc_cfg->video_dev_idx = 0;
}

static void toAppPanoramaCfg(MPI_PANORAMA_ATTR_S *pano_attr, AGTX_PANORAMA_CONF_S *pano_cfg)
{
	pano_cfg->enable = pano_attr->enable;
	pano_cfg->center_offset_x = pano_attr->center_offset.x;
	pano_cfg->center_offset_y = pano_attr->center_offset.y;
	pano_cfg->ldc_ratio = pano_attr->ldc_ratio;
	pano_cfg->radius = pano_attr->radius;
	pano_cfg->curvature = pano_attr->curvature;
	pano_cfg->straighten = pano_attr->straighten;
	pano_cfg->video_dev_idx = 0;
}

static void toAppPanningCfg(MPI_PANNING_ATTR_S *pann_attr, AGTX_PANNING_CONF_S *pann_cfg)
{
	pann_cfg->enable = pann_attr->enable;
	pann_cfg->center_offset_x = pann_attr->center_offset.x;
	pann_cfg->center_offset_y = pann_attr->center_offset.y;
	pann_cfg->ldc_ratio = pann_attr->ldc_ratio;
	pann_cfg->radius = pann_attr->radius;
	pann_cfg->hor_strength = pann_attr->hor_strength;
	pann_cfg->ver_strength = pann_attr->ver_strength;
	pann_cfg->video_dev_idx = 0;
}

static void toAppSurroundCfg(MPI_SURROUND_ATTR_S *surr_attr, AGTX_SURROUND_CONF_S *surr_cfg)
{
	surr_cfg->enable = surr_attr->enable;
	surr_cfg->center_offset_x = surr_attr->center_offset.x;
	surr_cfg->center_offset_y = surr_attr->center_offset.y;
	surr_cfg->ldc_ratio = surr_attr->ldc_ratio;
	surr_cfg->min_radius = surr_attr->min_radius;
	surr_cfg->max_radius = surr_attr->max_radius;
	surr_cfg->rotate = surr_attr->rotate;
	surr_cfg->video_dev_idx = 0;
}

INT32 APP_VIEW_setStitchAttr(MPI_PATH path_idx, AGTX_STITCH_CONF_S *conf)
{
	INT32 ret = MPI_FAILURE;
	MPI_STITCH_ATTR_S attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	toMpiStitchAttr(&attr, conf);
	ret = MPI_DEV_setStitchAttr(idx, &attr);

	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Set STITCH attr for device %d channel %d window %d failed.\n", idx.dev, idx.chn,
		        idx.win);
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_VIEW_setLdcConf(MPI_PATH path_idx, const AGTX_LDC_CONF_S *ldc_cfg)
{
	INT32 ret = MPI_FAILURE;
	MPI_LDC_ATTR_S ldc_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	toMpiLdcAttr(&ldc_attr, ldc_cfg);

	ret = MPI_DEV_setLdcAttr(idx, &ldc_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Set LDC attr for device %d channel %d window %d failed.\n", idx.dev, idx.chn, idx.win);
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_VIEW_setPanoramaConf(MPI_PATH path_idx, const AGTX_PANORAMA_CONF_S *pano_cfg)
{
	INT32 ret = 0;
	MPI_PANORAMA_ATTR_S pano_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	toMpiPanoramaAttr(&pano_attr, pano_cfg);

	ret = MPI_DEV_setPanoramaAttr(idx, &pano_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_VIEW_setPanningConf(MPI_PATH path_idx, const AGTX_PANNING_CONF_S *pann_cfg)
{
	INT32 ret = 0;
	MPI_PANNING_ATTR_S pann_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	toMpiPanningAttr(&pann_attr, pann_cfg);

	ret = MPI_DEV_setPanningAttr(idx, &pann_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_VIEW_setSurroundConf(MPI_PATH path_idx, const AGTX_SURROUND_CONF_S *surr_cfg)
{
	INT32 ret = 0;
	MPI_SURROUND_ATTR_S surr_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	toMpiSurroundAttr(&surr_attr, surr_cfg);

	ret = MPI_DEV_setSurroundAttr(idx, &surr_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_VIEW_setStitchConf(MPI_PATH path_idx, const AGTX_STITCH_CONF_S *stitch_cfg)
{
	INT32 ret = 0;
	MPI_STITCH_ATTR_S stitch_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	toMpiStitchAttr(&stitch_attr, stitch_cfg);

	ret = MPI_DEV_setStitchAttr(idx, &stitch_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	return MPI_SUCCESS;
}

INT32 APP_VIEW_getLdcConf(MPI_PATH path_idx, AGTX_LDC_CONF_S *ldc_cfg)
{
	INT32 ret = MPI_FAILURE;
	MPI_LDC_ATTR_S ldc_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	ret = MPI_DEV_getLdcAttr(idx, &ldc_attr);
	if (ret != MPI_SUCCESS) {
		fprintf(stderr, "Set LDC attr for device %d channel %d window %d failed.\n", idx.dev, idx.chn, idx.win);
		return MPI_FAILURE;
	}

	toAppLdcCfg(&ldc_attr, ldc_cfg);

	return MPI_SUCCESS;
}

INT32 APP_VIEW_getPanoramaConf(MPI_PATH path_idx, AGTX_PANORAMA_CONF_S *pano_cfg)
{
	INT32 ret = 0;
	MPI_PANORAMA_ATTR_S pano_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	ret = MPI_DEV_getPanoramaAttr(idx, &pano_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	toAppPanoramaCfg(&pano_attr, pano_cfg);

	return MPI_SUCCESS;
}

INT32 APP_VIEW_getPanningConf(MPI_PATH path_idx, AGTX_PANNING_CONF_S *pann_cfg)
{
	INT32 ret = 0;
	MPI_PANNING_ATTR_S pann_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	ret = MPI_DEV_getPanningAttr(idx, &pann_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	toAppPanningCfg(&pann_attr, pann_cfg);

	return MPI_SUCCESS;
}

INT32 APP_VIEW_getSurroundConf(MPI_PATH path_idx, AGTX_SURROUND_CONF_S *surr_cfg)
{
	INT32 ret = 0;
	MPI_SURROUND_ATTR_S surr_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	ret = MPI_DEV_getSurroundAttr(idx, &surr_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	toAppSurroundCfg(&surr_attr, surr_cfg);

	return MPI_SUCCESS;
}

INT32 APP_VIEW_getStitchConf(MPI_PATH path_idx, AGTX_STITCH_CONF_S *stitch_cfg)
{
	INT32 ret = 0;
	MPI_STITCH_ATTR_S stitch_attr = { 0 };
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);

	ret = MPI_DEV_getStitchAttr(idx, &stitch_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}

	toAppStitchCfg(&stitch_attr, stitch_cfg);

	return MPI_SUCCESS;
}

INT32 APP_VIEW_getWinViewType(MPI_PATH path_idx, AGTX_VIEW_TYPE_INFO_S *view_type_info)
{
	INT32 ret = 0;
	MPI_WIN_ATTR_S mpi_win_attr;
	MPI_WIN idx = MPI_VIDEO_WIN(path_idx.dev, 0, 0);
	ret = MPI_DEV_getWindowAttr(idx, &mpi_win_attr);
	if (ret != MPI_SUCCESS) {
		return MPI_FAILURE;
	}
	view_type_info->video_win_idx = idx.value;
	view_type_info->view_type = mpi_win_attr.view_type;
	return MPI_SUCCESS;
}

#ifdef __cplusplus
}
#endif /**< __cplusplus */
