#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "core.h"
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "log_define.h"

extern GlobalConf g_conf;
static MPI_CHN_ATTR_S exist_chn[MPI_MAX_VIDEO_CHN_NUM];

static int addExistChnRecord(const MPI_CHN_ATTR_S *p_chn_attr, int idx)
{
	memcpy(&exist_chn[idx], p_chn_attr, sizeof(MPI_CHN_ATTR_S));
	avmain2_log_debug("save exist chn[%i] res %dx%d", idx, exist_chn[idx].res.width, exist_chn[idx].res.height);

	return 0;
}

static int resetExistChnRecord(void)
{
	memset(&exist_chn, 0, sizeof(exist_chn));
	avmain2_log_debug("reset exist_enc record");
	return 0;
}

static void toMpiStitchAttr(MPI_STITCH_ATTR_S *stitch_attr, const AGTX_STITCH_CONF_S *attr)
{
	INT32 i = 0;

	stitch_attr->enable = attr->enable;
	stitch_attr->dft_dist = attr->dft_dist;
	stitch_attr->table_num = AGTX_MAX_STITCH_TABLE_NUM;

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

static inline void toMpiLayoutWindow(const MPI_RECT_S *pos, MPI_SIZE_S *chn_res, MPI_RECT_S *lyt_res)
{
#define MIN(a, b) ((a) < (b) ? (a) : (b))
	lyt_res->x = (((pos->x * (chn_res->width - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	lyt_res->y = (((pos->y * (chn_res->height - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;

	/* Handle boundary condition */
	if (pos->x + pos->width == 1024) {
		lyt_res->width = chn_res->width - lyt_res->x;
	} else {
		lyt_res->width =
		        MIN((((pos->width * (chn_res->width - 1) + 512) >> 10) + 9) & 0xFFFFFFF0, chn_res->width);
	}

	/* Handle boundary condition */
	if (pos->y + pos->height == 1024) {
		lyt_res->height = chn_res->height - lyt_res->y;
	} else {
		lyt_res->height = (((pos->height * (chn_res->height - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	}
}

int NODE_initChn(void)
{
	return 0;
}

int NODE_startChn(void)
{
	INT32 ret = MPI_FAILURE;
	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);
	MPI_CHN chn_idx = MPI_VIDEO_CHN(dev_idx.dev, 0);
	MPI_WIN win_idx = MPI_VIDEO_WIN(dev_idx.dev, 0, 0);

	MPI_STITCH_ATTR_S stitch_attr = { 0 };
	MPI_LDC_ATTR_S ldc_attr = { 0 };
	MPI_PANORAMA_ATTR_S pano_attr = { 0 };
	MPI_PANNING_ATTR_S pann_attr = { 0 };
	MPI_SURROUND_ATTR_S surr_attr = { 0 };

	MPI_CHN_ATTR_S chn_attr = {
		.res = { .width = 0, .height = 0 },
		.fps = 0.0,
		.binding_capability = MPI_CHN_BIND_CAP_NONE,
	};
	MPI_CHN_LAYOUT_S chn_layout = { 0 };
	MPI_WIN_ATTR_S window_attr[MAX_VIDEO_WINDOW];
	MPI_WIN_VIEW_TYPE_E view_type = MPI_WIN_VIEW_TYPE_NUM;
	MPI_RECT_S pos = { 0 };

	INT32 output_num = 0;

	/*reset exist_chn to invalid number*/
	resetExistChnRecord();

	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		if (g_conf.strm.video_strm[i].strm_en == 1) {
			output_num += 1;
			avmain2_log_info("add output chn %d", output_num);

			chn_attr.res.width = g_conf.strm.video_strm[i].width;
			chn_attr.res.height = g_conf.strm.video_strm[i].height;
			chn_attr.fps = g_conf.strm.video_strm[i].output_fps;
			chn_attr.binding_capability = g_conf.strm.video_strm[i].binding_capability;
			chn_idx.chn = g_conf.strm.video_strm[i].video_strm_idx;
			avmain2_log_info("add chn %d, g_conf w,h=(%d, %d)", g_conf.strm.video_strm[i].video_strm_idx,
			                 chn_attr.res.width, chn_attr.res.height);

			ret = MPI_DEV_addChn(chn_idx, &chn_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Add video channel %d failed. ret: %d", chn_idx.chn, ret);
				return -EINVAL;
			}
			if (g_conf.layout.layout_en == 1) {
				if (g_conf.layout.video_layout[i].video_strm_idx == i) {
					chn_layout.window_num = g_conf.layout.video_layout[i].window_num;
					for (int j = 0; j < chn_layout.window_num; j++) {
						win_idx.chn = g_conf.layout.video_layout[i].video_strm_idx;
						win_idx.win = g_conf.layout.video_layout[i].window_array[j].window_idx;
						chn_layout.win_id[j] = win_idx;

						pos.x = g_conf.layout.video_layout[i].window_array[j].pos_x;
						pos.y = g_conf.layout.video_layout[i].window_array[j].pos_y;
						pos.width = g_conf.layout.video_layout[i].window_array[j].pos_width;
						pos.height = g_conf.layout.video_layout[i].window_array[j].pos_height;

						toMpiLayoutWindow(&pos, &chn_attr.res, &chn_layout.window[j]);
						/*f49763 layout has diff path bmp, chn src from diff sensor*/
						window_attr[j].path.bmp =
						        g_conf.layout.video_layout[i].window_array[j].path_bmp;
						window_attr[j].fps =
						        g_conf.layout.video_layout[i].window_array[j].update_fps;
						window_attr[j].prio =
						        g_conf.layout.video_layout[i].window_array[j].priority;
						if (g_conf.layout.video_layout[i].window_array[j].parent == -1) {
							window_attr[j].src_id = MPI_INVALID_VIDEO_WIN;
						} else {
							window_attr[j].src_id.value = g_conf.layout.video_layout[i].window_array[j].parent;
						}
						window_attr[j].const_qual =
						        g_conf.layout.video_layout[i].window_array[j].const_qual;
						window_attr[j].dyn_adj =
						        g_conf.layout.video_layout[i].window_array[j].dyn_adj;
						window_attr[j].eis_en =
						        g_conf.layout.video_layout[i].window_array[j].eis_en;
						window_attr[j].rotate = g_conf.strm.video_strm[i].rotate;
						window_attr[j].mirr_en = g_conf.strm.video_strm[i].mirr_en;
						window_attr[j].flip_en = g_conf.strm.video_strm[i].flip_en;

						switch (g_conf.layout.video_layout[i].window_array[j].view_type) {
						case AGTX_WINDOW_VIEW_TYPE_NORMAL:
							view_type = MPI_WIN_VIEW_TYPE_NORMAL;
							break;
						case AGTX_WINDOW_VIEW_TYPE_LDC:
							view_type = MPI_WIN_VIEW_TYPE_LDC;
							break;
						case AGTX_WINDOW_VIEW_TYPE_STITCH:
							view_type = MPI_WIN_VIEW_TYPE_STITCH;
							break;
						case AGTX_WINDOW_VIEW_TYPE_PANORAMA:
							view_type = MPI_WIN_VIEW_TYPE_PANORAMA;
							break;
						case AGTX_WINDOW_VIEW_TYPE_PANNING:
							view_type = MPI_WIN_VIEW_TYPE_PANNING;
							break;
						case AGTX_WINDOW_VIEW_TYPE_SURROUND:
							view_type = MPI_WIN_VIEW_TYPE_SURROUND;
							break;
						default:
							avmain2_log_err("Invaild view_type");
							break;
						}
						window_attr[j].view_type = view_type;
						window_attr[j].roi.x =
						        g_conf.layout.video_layout[i].window_array[j].roi_x;
						window_attr[j].roi.y =
						        g_conf.layout.video_layout[i].window_array[j].roi_y;
						window_attr[j].roi.width =
						        g_conf.layout.video_layout[i].window_array[j].roi_width;
						window_attr[j].roi.height =
						        g_conf.layout.video_layout[i].window_array[j].roi_height;
					}
				} else {
					avmain2_log_err("i != g_conf.layout.video_layout[i].video_strm_idx");
				}
			} else {
				chn_layout.window_num = 1;
				win_idx.win = 0;
				chn_layout.win_id[0] = win_idx;
				chn_layout.window[0].x = 0;
				chn_layout.window[0].y = 0;
				chn_layout.window[0].width = g_conf.strm.video_strm[i].width;
				chn_layout.window[0].height = g_conf.strm.video_strm[i].height;
				window_attr[0].path.bmp = 1;
				window_attr[0].fps = g_conf.strm.video_strm[i].output_fps;
				window_attr[0].prio = 0;
				window_attr[0].src_id = MPI_INVALID_VIDEO_WIN;
				window_attr[0].const_qual = 1;
				window_attr[0].dyn_adj = 0;
				window_attr[0].eis_en = 0;
				window_attr[0].rotate = g_conf.strm.video_strm[i].rotate;
				window_attr[0].mirr_en = g_conf.strm.video_strm[i].mirr_en;
				window_attr[0].flip_en = g_conf.strm.video_strm[i].flip_en;
				window_attr[0].view_type = MPI_WIN_VIEW_TYPE_NORMAL;
				window_attr[0].roi.x = 0;
				window_attr[0].roi.y = 0;
				window_attr[0].roi.width = 1024;
				window_attr[0].roi.height = 1024;
			}

			ret = MPI_DEV_setChnLayout(chn_idx, &chn_layout);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Set video channel layout %d failed. ret: %d", chn_idx.chn, ret);
				return -EINVAL;
			}

			for (int w_idx = 0; w_idx < chn_layout.window_num; ++w_idx) {
				ret = MPI_DEV_setWindowAttr(chn_layout.win_id[w_idx], &window_attr[w_idx]);
				if (ret != MPI_SUCCESS) {
					avmain2_log_err("Set video window %d failed. ret: %d", chn_layout.win_id[w_idx].win, ret);
					return -EINVAL;
				}
			}

			addExistChnRecord(&chn_attr, i);

			MPI_DEV_getChnAttr(chn_idx, &chn_attr);
			avmain2_log_debug("log chn attr: [%d]res (%d, %d), fps: %f", i, chn_attr.res.width,
			                  chn_attr.res.height, chn_attr.fps);
		}
	}
	/*set window transform*/
	win_idx.chn = 0;
	win_idx.win = 0;

	toMpiStitchAttr(&stitch_attr, &g_conf.stitch);
	ret = MPI_DEV_setStitchAttr(win_idx, &stitch_attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Set STITCH attr for channel %d failed. ret: %d", win_idx.chn, ret);
		return -EINVAL;
	}

	toMpiLdcAttr(&ldc_attr, &g_conf.ldc);
	ret = MPI_DEV_setLdcAttr(win_idx, &ldc_attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Set LDC attr for channel %d failed. ret:%d", win_idx.chn, ret);
	}

	toMpiPanoramaAttr(&pano_attr, &g_conf.panorama);
	ret = MPI_DEV_setPanoramaAttr(win_idx, &pano_attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Set Panorama attr for channel %d failed. ret:%d", win_idx.chn, ret);
	}

	toMpiPanningAttr(&pann_attr, &g_conf.panning);
	ret = MPI_DEV_setPanningAttr(win_idx, &pann_attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Set Panning attr for channel %d failed.ret:%d", win_idx.chn, ret);
	}

	toMpiSurroundAttr(&surr_attr, &g_conf.surround);
	ret = MPI_DEV_setSurroundAttr(win_idx, &surr_attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Set Surround attr for channel %d failed. ret: %d", win_idx.chn, ret);
	}

	if (output_num == 1) {
		chn_idx.chn = 0;
		ret = MPI_DEV_startChn(chn_idx);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Start video channel %d failed. ret: %d", chn_idx.chn, ret);
			return -ENXIO;
		}
	} else if (output_num > 1) {
		ret = MPI_DEV_startAllChn(dev_idx);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Start all video channels on video device %d failed. ret:%d", dev_idx.dev, ret);
			return MPI_FAILURE;
		}
	} else {
		avmain2_log_info("no chn to start");
	}

	avmain2_log_info("Start video channel %d succeeded!", chn_idx.chn);

	return 0;
}

int NODE_stopChn(void)
{
	INT32 ret = MPI_FAILURE;

	MPI_DEV dev_idx = MPI_VIDEO_DEV(g_conf.dev.video_dev_idx);
	MPI_CHN chn_idx = MPI_VIDEO_CHN(dev_idx.dev, 0);

	/*align mpi_stream call flow*/
	ret = MPI_DEV_stopAllChn(MPI_VIDEO_DEV(0));
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Stop all video channels on video device %d failed. err: %d",
		                MPI_GET_VIDEO_DEV(dev_idx), ret);
		return -ENXIO;
	}

	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		/*Use exist_chn.res to check is running or not*/
		if (exist_chn[i].res.width == 0) {
			avmain2_log_info("chn[%d] not exist, skip", i);
			continue;
		}
		chn_idx.chn = g_conf.strm.video_strm[i].video_strm_idx;
		ret = MPI_DEV_deleteChn(chn_idx);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Delete video channel %d failed. ret:%d", chn_idx.chn, ret);
			return -ENXIO;
		}

		avmain2_log_info("Stop video channel %d succeeded!", chn_idx.chn);
	}
	/*reset exist_chn to invalid number*/
	resetExistChnRecord();

	return 0;
}

int NODE_exitChn(void)
{
	return 0;
}

int NODE_setChn(int cmd_id, void *data)
{
	INT32 ret = 0;
	MPI_DEV dev_idx;
	MPI_CHN chn_idx;
	MPI_WIN win_idx;

	MPI_DEV_ATTR_S dev_attr;
	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_WIN_ATTR_S window_attr;

	MPI_STITCH_ATTR_S stitch_attr;
	MPI_LDC_ATTR_S ldc_attr;
	MPI_PANORAMA_ATTR_S pano_attr;
	MPI_PANNING_ATTR_S pann_attr;
	MPI_SURROUND_ATTR_S surr_attr;

	if (cmd_id == FPS) {
		int new_fps = 0;
		avmain2_log_notice("AGTX_CMD_VIDEO_STRM_CONF change fps");
		dev_idx = MPI_VIDEO_DEV(((AGTX_STRM_CONF_S *)data)->video_dev_idx);

		for (int i = 0; (unsigned)i < ((AGTX_STRM_CONF_S *)data)->video_strm_cnt; i++) {
			if (!g_conf.strm.video_strm[i].strm_en) {
				continue;
			}
			chn_idx = MPI_VIDEO_CHN(dev_idx.dev, i);
			win_idx = MPI_VIDEO_WIN(chn_idx.dev, chn_idx.chn, 0);
			new_fps = ((AGTX_STRM_CONF_S *)data)->video_strm[i].output_fps;

			ret =MPI_DEV_getChnAttr(chn_idx, &chn_attr);
			if (MPI_SUCCESS != ret) {
				avmain2_log_err("Get chn attr for channel %d failed. ret:%d", win_idx.chn, ret);
				return -EINVAL;
			}
			avmain2_log_info("Set chn[%d]fps %f --> %d", i, chn_attr.fps, new_fps);
			chn_attr.fps = new_fps;

			ret = MPI_DEV_setChnAttr(chn_idx, &chn_attr);
			if (MPI_SUCCESS != ret) {
				avmain2_log_err("Set chn attr for channel %d failed. ret:%d", win_idx.chn, ret);
				return -EINVAL;
			}

			if (!g_conf.layout.layout_en) {
				ret = MPI_DEV_getChnLayout(chn_idx, &layout_attr);
				if (MPI_SUCCESS != ret) {
					avmain2_log_err("Get layout attr for channel %d failed. ret: %d", win_idx.chn, ret);
					return -EINVAL;
				}

				ret = MPI_DEV_getWindowAttr(win_idx, &window_attr);
				if (MPI_SUCCESS != ret) {
					avmain2_log_err("Get win attr for channel %d failed. ret:%d", win_idx.chn, ret);
					return -EINVAL;
				}

				if (layout_attr.window_num != 1) {
					avmain2_log_err("Invalid windows num, disable layout win num should be 1:%d",
					                layout_attr.window_num);
					return -EINVAL;
				}

				window_attr.fps = new_fps;
				ret = MPI_DEV_setChnLayout(chn_idx, &layout_attr);
				if (MPI_SUCCESS != ret) {
					avmain2_log_err("Set layout attr for channel %d failed. ret: %d", win_idx.chn, ret);
					return -EINVAL;
				}

				ret = MPI_DEV_setWindowAttr(win_idx, &window_attr);
				if (MPI_SUCCESS != ret) {
					avmain2_log_err("Set win attr for channel %d failed. ret:%d", win_idx.chn, ret);
					return -EINVAL;
				}
			}
		}
	}

	if (cmd_id == STITCH_ATTR) {
		avmain2_log_notice("AGTX_CMD_STITCH_CONF change STITCH_ATTR");
		dev_idx = MPI_VIDEO_DEV(((AGTX_STITCH_CONF_S *)data)->video_dev_idx);

		/* check dev enable stitch or not */
		ret = MPI_DEV_getDevAttr(dev_idx, &dev_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("failed to get dev[%d] attr, ignore STITCH_ATTR, ret:%d", dev_idx.dev, ret);
			return 0;
		}

		if (dev_attr.stitch_en == 0) {
			avmain2_log_warn("dev not support stitch, ignore STITCH_ATTR");
			return 0;
		}

		win_idx = MPI_VIDEO_WIN(dev_idx.dev, 0, 0);
		ret = MPI_DEV_getStitchAttr(win_idx, &stitch_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Get STITCH attr for channel %d failed. ret: %d", win_idx.chn, ret);
			return -EINVAL;
		}

		toMpiStitchAttr(&stitch_attr, (AGTX_STITCH_CONF_S *)data);

		ret = MPI_DEV_setStitchAttr(win_idx, &stitch_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Set STITCH attr for channel %d failed. ret: %d", win_idx.chn, ret);
			return -EINVAL;
		}
	}

	if (cmd_id == LDC_ATTR) {
		avmain2_log_notice("AGTX_CMD_LDC_CONF change LDC_ATTR");
		dev_idx = MPI_VIDEO_DEV(((AGTX_LDC_CONF_S *)data)->video_dev_idx);
		win_idx = MPI_VIDEO_WIN(dev_idx.dev, 0, 0);
		ret = MPI_DEV_getLdcAttr(win_idx, &ldc_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Get LDC attr for channel %d failed. ret:%d", win_idx.chn, ret);
			return -EINVAL;
		}

		toMpiLdcAttr(&ldc_attr, (AGTX_LDC_CONF_S *)data);

		ret = MPI_DEV_setLdcAttr(win_idx, &ldc_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Set LDC attr for channel %d failed. ret: %d", win_idx.chn, ret);
			return -EINVAL;
		}
	}

	if (cmd_id == PANNING_ATTR) {
		avmain2_log_notice("AGTX_CMD_PANNING_CONF change PANNING_ATTR");
		dev_idx = MPI_VIDEO_DEV(((AGTX_PANNING_CONF_S *)data)->video_dev_idx);
		win_idx = MPI_VIDEO_WIN(dev_idx.dev, 0, 0);
		ret = MPI_DEV_getPanningAttr(win_idx, &pann_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Get Panning attr for channel %d failed. ret:%d", win_idx.chn, ret);
			return -EINVAL;
		}

		toMpiPanningAttr(&pann_attr, (AGTX_PANNING_CONF_S *)data);

		ret = MPI_DEV_setPanningAttr(win_idx, &pann_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Set Panning attr for channel %d failed. ret: %d", win_idx.chn, ret);
			return -EINVAL;
		}
	}

	if (cmd_id == PANORAMA_ATTR) {
		avmain2_log_notice("AGTX_CMD_PANORAMA_CONF change PANORAMA_ATTR");
		dev_idx = MPI_VIDEO_DEV(((AGTX_PANORAMA_CONF_S *)data)->video_dev_idx);
		win_idx = MPI_VIDEO_WIN(dev_idx.dev, 0, 0);
		ret = MPI_DEV_getPanoramaAttr(win_idx, &pano_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Get Panorama attr for channel %d failed. ret:%d", win_idx.chn, ret);
			return -EINVAL;
		}

		toMpiPanoramaAttr(&pano_attr, (AGTX_PANORAMA_CONF_S *)data);

		ret = MPI_DEV_setPanoramaAttr(win_idx, &pano_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Set Panorama attr for channel %d failed. ret:%d", win_idx.chn, ret);
			return -EINVAL;
		}
	}

	if (cmd_id == SURROUND_ATTR) {
		avmain2_log_notice("AGTX_CMD_SURROUND_CONF change SURROUND_ATTR");
		dev_idx = MPI_VIDEO_DEV(((AGTX_SURROUND_CONF_S *)data)->video_dev_idx);
		win_idx = MPI_VIDEO_WIN(dev_idx.dev, 0, 0);
		ret = MPI_DEV_getSurroundAttr(win_idx, &surr_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Get Surround attr for channel %d failed. ret:%d", win_idx.chn, ret);
			return -EINVAL;
		}

		toMpiSurroundAttr(&surr_attr, (AGTX_SURROUND_CONF_S *)data);

		ret = MPI_DEV_setSurroundAttr(win_idx, &surr_attr);
		if (ret != MPI_SUCCESS) {
			avmain2_log_err("Set Surround attr for channel %d failed. ret: %d", win_idx.chn, ret);
			return -EINVAL;
		}
	}

	return 0;
}
