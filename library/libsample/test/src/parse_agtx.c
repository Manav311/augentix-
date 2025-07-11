#include "parse_agtx.h"

#include <asm-generic/errno-base.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "json.h"

#include "agtx_video.h"
#include "app_view_api.h"
#include "agtx_cmd.h"

#include "cm_video_layout_conf.h"
#include "cm_surround_conf.h"
#include "cm_stitch_conf.h"
#include "cm_panorama_conf.h"
#include "cm_panning_conf.h"
#include "cm_video_ldc_conf.h"
#include "cm_video_strm_conf.h"

#include "sample_ctrl.h"

#define STITCH_TABLE_NUM 3

#define JSON_CONF                      \
	AGTX_LAYOUT_CONF_S layout;     \
	AGTX_STRM_CONF_S strm;         \
	AGTX_STITCH_CONF_S stitch;     \
	AGTX_LDC_CONF_S ldc;           \
	AGTX_SURROUND_CONF_S surround; \
	AGTX_PANORAMA_CONF_S panorama; \
	AGTX_PANNING_CONF_S panning;

typedef struct json_conf {
	JSON_CONF
} JsonConf;

struct json_object *getJsonRecordObj(const char *json_path)
{
	if (access(json_path, F_OK) != 0) {
		return NULL;
	}

	FILE *fp;
	int file_size = 0;
	struct json_object *obj = NULL;
	struct json_tokener *tok = json_tokener_new();
	enum json_tokener_error jerr;

	/** get json file size */
	fp = fopen(json_path, "r");
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	if (file_size <= 0) {
		fprintf(stderr, "Invaild file size: %d\n", file_size);
		fclose(fp);
		json_tokener_free(tok);
		return NULL;
	}

	/** read all str to buffer */
	char json_str[file_size];
	fread(json_str, file_size, 1, fp);
	fclose(fp);

	/** parse str to json obj */
	obj = json_tokener_parse_ex(tok, json_str, file_size);

	jerr = json_tokener_get_error(tok);
	if (jerr != json_tokener_success) {
		fprintf(stderr, "JSON Parsing errorer %s \n", json_tokener_error_desc(jerr));
		obj = NULL;
	}

	json_tokener_free(tok);
	return obj;
}

#if 1
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

#endif

static int getViewTypeAttrInJsonPath(MPI_WIN_VIEW_TYPE_E type, const char *path, JsonConf *json)
{
	struct json_object *ret_obj = NULL;

	switch (type) {
	case MPI_WIN_VIEW_TYPE_LDC:
		ret_obj = getJsonRecordObj(path);
		parse_ldc_conf(&(json->ldc), ret_obj);
		break;

	case MPI_WIN_VIEW_TYPE_PANORAMA:
		ret_obj = getJsonRecordObj(path);
		parse_panorama_conf(&(json->panorama), ret_obj);
		break;

	case MPI_WIN_VIEW_TYPE_PANNING:
		ret_obj = getJsonRecordObj(path);
		parse_panning_conf(&(json->panning), ret_obj);
		break;

	case MPI_WIN_VIEW_TYPE_SURROUND:
		ret_obj = getJsonRecordObj(path);
		parse_surround_conf(&(json->surround), ret_obj);
		break;
	case MPI_WIN_VIEW_TYPE_STITCH:
		ret_obj = getJsonRecordObj(path);
		parse_stitch_conf(&(json->stitch), ret_obj);
		break;

	default:
		fprintf(stderr, "Unknown view type: %d\n", type);
		return -EINVAL;
	}

	json_object_put(ret_obj);

	return 0;
}

static int setJsonToMpi(const SAMPLE_CONF_S *conf, JsonConf *json, MPI_WIN_VIEW_TYPE_E type, int change_view_type)
{
	int ret = 0;
	MPI_LDC_ATTR_S ldc_attr;
	MPI_PANORAMA_ATTR_S pano_attr;
	MPI_PANNING_ATTR_S pann_attr;
	MPI_SURROUND_ATTR_S surr_attr;
	MPI_STITCH_ATTR_S stitch_attr;

	switch (type) {
	case MPI_WIN_VIEW_TYPE_LDC:
		toMpiLdcAttr(&ldc_attr, &(json->ldc));
		ret = SAMPLE_updateLdcAttr(&ldc_attr);
		if (ret != 0) {
			goto end;
		}
		break;

	case MPI_WIN_VIEW_TYPE_PANORAMA:
		toMpiPanoramaAttr(&pano_attr, &(json->panorama));
		ret = SAMPLE_updatePanoramaAttr(&pano_attr);
		if (ret != 0) {
			goto end;
		}
		break;

	case MPI_WIN_VIEW_TYPE_PANNING:
		toMpiPanningAttr(&pann_attr, &(json->panning));
		ret = SAMPLE_updatePanningAttr(&pann_attr);
		if (ret != 0) {
			goto end;
		}
		break;

	case MPI_WIN_VIEW_TYPE_SURROUND:
		toMpiSurroundAttr(&surr_attr, &(json->surround));
		ret = SAMPLE_updateSurroundAttr(&surr_attr);
		if (ret != 0) {
			goto end;
		}
		break;

	case MPI_WIN_VIEW_TYPE_STITCH:
		toMpiStitchAttr(&stitch_attr, &(json->stitch));
		ret = SAMPLE_updateStitchAttr(&stitch_attr);
		if (ret != 0) {
			goto end;
		}
		break;

	default:
		fprintf(stderr, "Unknown view type: %d\n", type);
		return -EINVAL;
	}

	if (change_view_type) {
		ret = SAMPLE_reconfigWindowViewType(conf, MPI_VIDEO_WIN(0, 0, 0), type);
	}
end:
	return ret;
}

static INT32 getVencType(UINT32 venc_type)
{
	INT32 type = 0;

	switch (venc_type) {
	case AGTX_VENC_TYPE_H264:
		type = MPI_VENC_TYPE_H264;
		break;
	case AGTX_VENC_TYPE_H265:
		type = MPI_VENC_TYPE_H265;
		break;
	case AGTX_VENC_TYPE_MJPEG:
		type = MPI_VENC_TYPE_MJPEG;
		break;
	default:
		fprintf(stderr, "unknown Venc type\n");
		type = MPI_VENC_TYPE_H264;
		break;
	}

	return type;
}

static INT32 getVencRcMode(AGTX_RC_MODE_E rc_mode)
{
	INT32 mode = 0;

	switch (rc_mode) {
	case AGTX_RC_MODE_VBR:
		mode = MPI_RC_MODE_VBR;
		break;
	case AGTX_RC_MODE_CBR:
		mode = MPI_RC_MODE_CBR;
		break;
	case AGTX_RC_MODE_SBR:
		mode = MPI_RC_MODE_SBR;
		break;
	case AGTX_RC_MODE_CQP:
		mode = MPI_RC_MODE_CQP;
		break;
	default:
		mode = MPI_RC_MODE_CBR;
		break;
	}

	return mode;
}

static INT32 getVencProfile(AGTX_PRFL_E venc_profile)
{
	INT32 profile = 0;

	switch (venc_profile) {
	case AGTX_PRFL_BASELINE:
		profile = MPI_PRFL_BASELINE;
		break;
	case AGTX_PRFL_MAIN:
		profile = MPI_PRFL_MAIN;
		break;
	case AGTX_PRFL_HIGH:
		profile = MPI_PRFL_HIGH;
		break;
	default:
		profile = MPI_PRFL_BASELINE;
		break;
	}

	return profile;
}

static VOID setVencAttr(MPI_VENC_ATTR_S *attr, AGTX_STRM_PARAM_S *conf)
{
	attr->type = getVencType(conf->venc_type);

	switch (attr->type) {
	case MPI_VENC_TYPE_H264:
		attr->h264.profile = getVencProfile(conf->venc_profile);
		attr->h264.rc.mode = getVencRcMode(conf->rc_mode);
		attr->h264.rc.gop = (UINT32)conf->gop_size;
		attr->h264.rc.frm_rate_o = conf->output_fps;
		attr->h264.rc.max_frame_size = (UINT32)conf->max_frame_size;

		if (attr->h264.rc.mode == MPI_RC_MODE_VBR) {
			attr->h264.rc.vbr.max_bit_rate = (UINT32)conf->vbr_max_bit_rate;
			attr->h264.rc.vbr.quality_level_index = (UINT32)conf->vbr_quality_level_index;
			attr->h264.rc.vbr.regression_speed = (UINT32)conf->regression_speed;
			attr->h264.rc.vbr.scene_smooth = (UINT32)conf->scene_smooth;
			attr->h264.rc.vbr.fluc_level = (UINT32)conf->fluc_level;
			attr->h264.rc.vbr.i_continue_weight = (UINT32)conf->i_continue_weight;
			attr->h264.rc.vbr.max_qp = (UINT32)conf->max_qp;
			attr->h264.rc.vbr.max_qp = (UINT32)conf->max_qp;
			attr->h264.rc.vbr.i_qp_offset = conf->i_qp_offset;
			attr->h264.rc.vbr.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
			attr->h264.rc.vbr.motion_tolerance_qp = (UINT32)conf->motion_tolerance_qp;
		} else if (attr->h264.rc.mode == MPI_RC_MODE_CBR) {
			attr->h264.rc.cbr.bit_rate = (UINT32)conf->bit_rate;
			attr->h264.rc.cbr.regression_speed = (UINT32)conf->regression_speed;
			attr->h264.rc.cbr.scene_smooth = (UINT32)conf->scene_smooth;
			attr->h264.rc.cbr.fluc_level = (UINT32)conf->fluc_level;
			attr->h264.rc.cbr.i_continue_weight = (UINT32)conf->i_continue_weight;
			attr->h264.rc.cbr.min_qp = (UINT32)conf->min_qp;
			attr->h264.rc.cbr.max_qp = (UINT32)conf->max_qp;
			attr->h264.rc.cbr.i_qp_offset = conf->i_qp_offset;
			attr->h264.rc.cbr.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
			attr->h264.rc.cbr.motion_tolerance_qp = (UINT32)conf->motion_tolerance_qp;
		} else if (attr->h264.rc.mode == MPI_RC_MODE_SBR) {
			attr->h264.rc.sbr.bit_rate = (UINT32)conf->bit_rate;
			attr->h264.rc.sbr.regression_speed = (UINT32)conf->regression_speed;
			attr->h264.rc.sbr.scene_smooth = (UINT32)conf->scene_smooth;
			attr->h264.rc.sbr.fluc_level = (UINT32)conf->fluc_level;
			attr->h264.rc.sbr.i_continue_weight = (UINT32)conf->i_continue_weight;
			attr->h264.rc.sbr.min_qp = (UINT32)conf->min_qp;
			attr->h264.rc.sbr.max_qp = (UINT32)conf->max_qp;
			attr->h264.rc.sbr.i_qp_offset = conf->i_qp_offset;
			attr->h264.rc.sbr.adjust_br_thres_pc = (UINT32)conf->sbr_adjust_br_thres_pc;
			attr->h264.rc.sbr.adjust_step_times = (UINT32)conf->sbr_adjust_step_times;
			attr->h264.rc.sbr.converge_frame = (UINT32)conf->sbr_converge_frame;
			attr->h264.rc.sbr.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
			attr->h264.rc.sbr.motion_tolerance_qp = (UINT32)conf->motion_tolerance_qp;
		} else if (attr->h264.rc.mode == MPI_RC_MODE_CQP) {
			attr->h264.rc.cqp.i_frame_qp = (UINT32)conf->cqp_i_frame_qp;
			attr->h264.rc.cqp.p_frame_qp = (UINT32)conf->cqp_p_frame_qp;
		} else {
			fprintf(stderr, "Invalid rate control mode.\n");
		}
		break;
	case MPI_VENC_TYPE_H265:
		attr->h265.profile = getVencProfile(conf->venc_profile);
		attr->h265.rc.mode = getVencRcMode(conf->rc_mode);
		attr->h265.rc.gop = (UINT32)conf->gop_size;
		attr->h265.rc.frm_rate_o = conf->output_fps;
		attr->h265.rc.max_frame_size = (UINT32)conf->max_frame_size;

		if (attr->h265.rc.mode == MPI_RC_MODE_VBR) {
			attr->h265.rc.vbr.max_bit_rate = (UINT32)conf->vbr_max_bit_rate;
			attr->h265.rc.vbr.quality_level_index = (UINT32)conf->vbr_quality_level_index;
			attr->h265.rc.vbr.regression_speed = (UINT32)conf->regression_speed;
			attr->h265.rc.vbr.scene_smooth = (UINT32)conf->scene_smooth;
			attr->h265.rc.vbr.fluc_level = (UINT32)conf->fluc_level;
			attr->h265.rc.vbr.i_continue_weight = (UINT32)conf->i_continue_weight;
			attr->h265.rc.vbr.max_qp = (UINT32)conf->max_qp;
			attr->h265.rc.vbr.max_qp = (UINT32)conf->max_qp;
			attr->h265.rc.vbr.i_qp_offset = conf->i_qp_offset;
			attr->h265.rc.vbr.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
			attr->h265.rc.vbr.motion_tolerance_qp = (UINT32)conf->motion_tolerance_qp;
		} else if (attr->h265.rc.mode == MPI_RC_MODE_CBR) {
			attr->h265.rc.cbr.bit_rate = (UINT32)conf->bit_rate;
			attr->h265.rc.cbr.min_qp = (UINT32)conf->min_qp;
			attr->h265.rc.cbr.max_qp = (UINT32)conf->max_qp;
			attr->h265.rc.cbr.regression_speed = (UINT32)conf->regression_speed;
			attr->h265.rc.cbr.scene_smooth = (UINT32)conf->scene_smooth;
			attr->h265.rc.cbr.fluc_level = (UINT32)conf->fluc_level;
			attr->h265.rc.cbr.i_continue_weight = (UINT32)conf->i_continue_weight;
			attr->h265.rc.cbr.i_qp_offset = conf->i_qp_offset;
			attr->h265.rc.cbr.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
			attr->h265.rc.cbr.motion_tolerance_qp = (UINT32)conf->motion_tolerance_qp;
		} else if (attr->h265.rc.mode == MPI_RC_MODE_SBR) {
			attr->h265.rc.sbr.bit_rate = (UINT32)conf->bit_rate;
			attr->h265.rc.sbr.min_qp = (UINT32)conf->min_qp;
			attr->h265.rc.sbr.max_qp = (UINT32)conf->max_qp;
			attr->h265.rc.sbr.regression_speed = (UINT32)conf->regression_speed;
			attr->h265.rc.sbr.scene_smooth = (UINT32)conf->scene_smooth;
			attr->h265.rc.sbr.fluc_level = (UINT32)conf->fluc_level;
			attr->h265.rc.sbr.i_continue_weight = (UINT32)conf->i_continue_weight;
			attr->h265.rc.sbr.i_qp_offset = conf->i_qp_offset;
			attr->h265.rc.sbr.adjust_br_thres_pc = (UINT32)conf->sbr_adjust_br_thres_pc;
			attr->h265.rc.sbr.adjust_step_times = (UINT32)conf->sbr_adjust_step_times;
			attr->h265.rc.sbr.converge_frame = (UINT32)conf->sbr_converge_frame;
			attr->h265.rc.sbr.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
			attr->h265.rc.sbr.motion_tolerance_qp = (UINT32)conf->motion_tolerance_qp;
		} else if (attr->h265.rc.mode == MPI_RC_MODE_CQP) {
			attr->h265.rc.cqp.i_frame_qp = (UINT32)conf->cqp_i_frame_qp;
			attr->h265.rc.cqp.p_frame_qp = (UINT32)conf->cqp_p_frame_qp;
		} else {
			fprintf(stderr, "Invalid rate control mode.\n");
		}
		break;
	case MPI_VENC_TYPE_MJPEG:
		attr->mjpeg.rc.mode = getVencRcMode(conf->rc_mode);
		attr->mjpeg.rc.frm_rate_o = (UINT32)conf->output_fps;
		attr->mjpeg.rc.max_frame_size = (UINT32)conf->max_frame_size;
		attr->mjpeg.rc.max_bit_rate = (UINT32)conf->vbr_max_bit_rate;
		attr->mjpeg.rc.quality_level_index = (UINT32)conf->vbr_quality_level_index;
		attr->mjpeg.rc.fluc_level = (UINT32)conf->fluc_level;
		attr->mjpeg.rc.bit_rate = (UINT32)conf->bit_rate;
		attr->mjpeg.rc.max_q_factor = (UINT32)conf->max_q_factor;
		attr->mjpeg.rc.min_q_factor = (UINT32)conf->min_q_factor;
		attr->mjpeg.rc.adjust_br_thres_pc = (UINT32)conf->sbr_adjust_br_thres_pc;
		attr->mjpeg.rc.adjust_step_times = (UINT32)conf->sbr_adjust_step_times;
		attr->mjpeg.rc.converge_frame = (UINT32)conf->sbr_converge_frame;
		attr->mjpeg.rc.q_factor = (UINT32)conf->cqp_q_factor;
		attr->mjpeg.rc.motion_tolerance_level = (UINT32)conf->motion_tolerance_level;
		attr->mjpeg.rc.motion_tolerance_qfactor = (UINT32)conf->motion_tolerance_qfactor;
		break;
	default:
		fprintf(stderr, "Invalid code type.\n");
		break;
	}
}

static inline void toMpiLayoutWindow(const MPI_RECT_S *pos, MPI_SIZE_S *chn_res, MPI_RECT_S *lyt_res)
{
#define MIN(a, b) ((a) < (b) ? (a) : (b))
	lyt_res->x = (((pos->x * (chn_res->width - 1) + 512) >> 10) + 8) & 0xFFFFFFF0;
	lyt_res->y = (((pos->y * (chn_res->height - 1) + 512) >> 10) + 16) & 0xFFFFFFE0;
	lyt_res->width = MIN((((pos->width * (chn_res->width - 1) + 512) >> 10) + 9) & 0xFFFFFFF0, chn_res->width);

	/* Handle boundary condition */
	if (pos->y + pos->height == 1024) {
		lyt_res->height = chn_res->height - lyt_res->y;
	} else {
		lyt_res->height = (((pos->height * (chn_res->height - 1) + 512) >> 10) + 16) & 0xFFFFFFE0;
	}
}

static void toLayoutAttr(const int chn_idx, AGTX_LAYOUT_CONF_S *layout, MPI_CHN_LAYOUT_S *chn_layout)
{
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);
	MPI_RECT_S pos = { 0 };
	MPI_CHN_ATTR_S chn_attr;
	MPI_DEV_getChnAttr(MPI_VIDEO_CHN(0, chn_idx), &chn_attr);

	if (layout->layout_en == 1) {
		if (layout->video_layout[chn_idx].video_strm_idx == chn_idx) {
			chn_layout->window_num = layout->video_layout[chn_idx].window_num;
			for (int j = 0; j < chn_layout->window_num; j++) {
				win_idx.chn = layout->video_layout[chn_idx].video_strm_idx;
				win_idx.win = layout->video_layout[chn_idx].window_array[j].window_idx;
				chn_layout->win_id[j] = win_idx;

				pos.x = layout->video_layout[chn_idx].window_array[j].pos_x;
				pos.y = layout->video_layout[chn_idx].window_array[j].pos_y;
				pos.width = layout->video_layout[chn_idx].window_array[j].pos_width;
				pos.height = layout->video_layout[chn_idx].window_array[j].pos_height;

				toMpiLayoutWindow(&pos, &chn_attr.res, &chn_layout->window[j]);
			}
		}
	} else {
		chn_layout->window_num = 1;
		win_idx.win = 0;
		chn_layout->win_id[0] = win_idx;
		chn_layout->window[0].x = 0;
		chn_layout->window[0].y = 0;
		chn_layout->window[0].width = chn_attr.res.width;
		chn_layout->window[0].height = chn_attr.res.height;
	}
}
/**
 * @brief parse AGTX_CMD_LAYOUT_CONF to MPI and set to SAMPLE_reconfigLayout
 * @param[in] sample_conf case_config video pipeline info
 * @param[in] chn_idx which video channel to reconfig layout 
 * @param[in] path AGTX_CMD_LAYOUT_CONF.json path
 * @return int 0 number for success, negative for error.
 * @see SAMPLE_reconfigLayout
 */
int parseLayout(const SAMPLE_CONF_S *sample_conf, const int chn_idx, const char *path)
{
	int ret = 0;
	AGTX_LAYOUT_CONF_S layout;
	MPI_CHN_LAYOUT_S chn_layout;

	struct json_object *ret_obj = NULL;
	ret_obj = getJsonRecordObj(path);
	parse_layout_conf(&layout, ret_obj);
	json_object_put(ret_obj);

	toLayoutAttr(chn_idx, &layout, &chn_layout);

	ret = SAMPLE_reconfigLayout(sample_conf, MPI_VIDEO_CHN(0, chn_idx), &chn_layout);

	return ret;
}
/**
 * @brief test reconfig codec
 * 
 * @param[in] sample_conf case_config video pipeline info
 * @param[in] enc_idx encoder to update
 * @param[in] path AGTX_CMD_VIDEO_STRM_CONF.json path
 * @return int 0 number for success, negative for error.
 * @see SAMPLE_reconfigCodec
 */
int parseCodecAttr(const SAMPLE_CONF_S *sample_conf, const int enc_idx, const char *path)
{
	int ret = 0;
	MPI_VENC_ATTR_S attr;
	AGTX_STRM_CONF_S strm;

	struct json_object *ret_obj = NULL;
	ret_obj = getJsonRecordObj(path);
	parse_video_strm_conf(&strm, ret_obj);
	json_object_put(ret_obj);

	ret = MPI_ENC_getVencAttr(MPI_ENC_CHN(enc_idx), &attr);
	if (ret) {
		goto end;
	}

	setVencAttr(&attr, &(strm.video_strm[enc_idx]));
	ret = SAMPLE_reconfigCodec(sample_conf, MPI_ENC_CHN(enc_idx), &attr);
end:
	return ret;
}
/**
 * @brief test to reconfig rate control or update rate control attributes
 * 
 * @param enc_idx encoder to update
 * @param path AGTX_CMD_VIDEO_STRM_CONF.json path
 * @return int 0 number for success, negative for error.
 * @see SAMPLE_updateVbrParams
 * @see SAMPLE_updateCbrParams
 * @see SAMPLE_updateSbrParams
 * @see SAMPLE_updateCqpParams
 */
int parseRateControlAttr(const int enc_idx, const char *path)
{
	int ret = 0;
	MPI_VENC_ATTR_S attr;
	AGTX_STRM_CONF_S strm;
	int rc_mode = 0;

	/** get json attr */
	struct json_object *ret_obj = NULL;
	ret_obj = getJsonRecordObj(path);
	parse_video_strm_conf(&strm, ret_obj);
	json_object_put(ret_obj);

	/** save current venc attr */
	ret = MPI_ENC_getVencAttr(MPI_ENC_CHN(enc_idx), &attr);

	/** parse json attr to venc attr */
	setVencAttr(&attr, &(strm.video_strm[enc_idx]));
	rc_mode = getVencRcMode(strm.video_strm[enc_idx].rc_mode);

	switch (attr.type) {
	case MPI_VENC_TYPE_H264:
		if (rc_mode == MPI_RC_MODE_VBR) {
			ret = SAMPLE_updateVbrParams(MPI_ENC_CHN(enc_idx), &(attr.h264.rc.vbr));
		} else if (rc_mode == MPI_RC_MODE_CBR) {
			ret = SAMPLE_updateCbrParams(MPI_ENC_CHN(enc_idx), &(attr.h264.rc.cbr));
		} else if (rc_mode == MPI_RC_MODE_SBR) {
			ret = SAMPLE_updateSbrParams(MPI_ENC_CHN(enc_idx), &(attr.h264.rc.sbr));
		} else if (rc_mode == MPI_RC_MODE_CQP) {
			ret = SAMPLE_updateCqpParams(MPI_ENC_CHN(enc_idx), &(attr.h264.rc.cqp));
		}
		break;
	case MPI_VENC_TYPE_H265:
		if (rc_mode == MPI_RC_MODE_VBR) {
			ret = SAMPLE_updateVbrParams(MPI_ENC_CHN(enc_idx), &(attr.h265.rc.vbr));
		} else if (rc_mode == MPI_RC_MODE_CBR) {
			ret = SAMPLE_updateCbrParams(MPI_ENC_CHN(enc_idx), &(attr.h265.rc.cbr));
		} else if (rc_mode == MPI_RC_MODE_SBR) {
			ret = SAMPLE_updateSbrParams(MPI_ENC_CHN(enc_idx), &(attr.h265.rc.sbr));
		} else if (rc_mode == MPI_RC_MODE_CQP) {
			ret = SAMPLE_updateCqpParams(MPI_ENC_CHN(enc_idx), &(attr.h265.rc.cqp));
		}
		break;
	case MPI_VENC_TYPE_MJPEG:
		fprintf(stderr, "MJPEG codec NOT support CBR/VBR/SBR/CQP rate control\n");
		break;
	default:
		return -EINVAL;
	}

	return ret;
}
/**
 * @brief test to reconfig view type or update view type attributes
 * 
 * @param type view type for win(0, 0, 0)
 * @param path AGTX_CMD_VIDEO_STRM_CONF.json path
 * @return int 0 number for success, negative for error
 * @see SAMPLE_reconfigWindowViewType
 * @see SAMPLE_updateStitchAttr
 * @see SAMPLE_updatePanoramaAttr
 * @see SAMPLE_updatePanningAttr
 * @see SAMPLE_updateSurroundAttr
 * @see SAMPLE_updateLdcAttr
 */
int parseViewTypeAttr(const SAMPLE_CONF_S *sample_conf, const MPI_WIN_VIEW_TYPE_E type, const char *path)
{
	int ret = 0;
	MPI_WIN_ATTR_S win_attr;
	JsonConf json_conf;

	ret = MPI_DEV_getWindowAttr(MPI_VIDEO_WIN(0, 0, 0), &win_attr);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	ret = getViewTypeAttrInJsonPath(type, path, &json_conf);
	if (ret != MPI_SUCCESS) {
		return ret;
	}

	if (type == win_attr.view_type) {
		/** Update view_type attr */
		setJsonToMpi(sample_conf, &json_conf, type, 0);
	} else {
		/** reconfig win view type */
		if (type == MPI_WIN_VIEW_TYPE_NORMAL) {
			ret = SAMPLE_reconfigWindowViewType(sample_conf, MPI_VIDEO_WIN(0, 0, 0), type);
			return ret;
		}
		setJsonToMpi(sample_conf, &json_conf, type, 1);
	}

	return 0;
}
