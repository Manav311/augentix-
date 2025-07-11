#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

#include <time.h>
#include <pthread.h>

#include "sample_venc_extend.h"
#include "utlist.h"

#include "core.h"
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_osd.h"

#include "log_define.h"
#include "handlers.h"

extern GlobalConf g_conf;

static MPI_ENC_CHN_ATTR_S exist_enc[MPI_MAX_ENC_CHN_NUM];
static OSD_HANDLE g_osd_chn_handle[MPI_MAX_ENC_CHN_NUM][MPI_OSD_MAX_BIND_CHANNEL /*OSD*/ +
                                                        MPI_OSD_MAX_BIND_CHANNEL /*OSD PM*/]; /* [enc idx][osd idx] */
pthread_t threadUpdateDateTime = 0;
int g_osd_run_flag = 0;
#define OSD_THREAD_NAME "datetime"
#define OSD_INVALID_HANDLER (-1)

static VencExtendInfo *g_vencExtendInfo = NULL;

#define MAX8(x) (x > 7 ? 7 : x)
#define MAX_TEXT_LEN 32 /**< Max charater of title. */

typedef struct {
	UINT32 image_offset;
	UINT32 image_size;
	UINT32 image_width;
	UINT32 image_height;
} ASCII_INDEX;

typedef struct {
	UINT32 index_offset;
	UINT32 index_size;
	UINT32 ascii_index;
	UINT32 ascii_width;
	UINT32 cht_index;
	UINT32 cht_width;
	UINT32 other_index;
	UINT32 other_width;
	UINT32 data_offset;
	UINT32 data_size;
} AyuvInfo_S;

static int addExistEncRecord(const MPI_ENC_CHN_ATTR_S *p_enc_attr, int idx)
{
	memcpy(&exist_enc[idx], p_enc_attr, sizeof(MPI_ENC_CHN_ATTR_S));
	avmain2_log_debug("save enc[%i] res %dx%d", idx, exist_enc[idx].res.width, exist_enc[idx].res.height);

	return 0;
}

static int resetExistEncRecord(void)
{
	memset(&exist_enc, 0, sizeof(exist_enc));
	avmain2_log_debug("reset exist_enc record");
	return 0;
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
		avmain2_log_err("unknown Venc type");
		type = MPI_VENC_TYPE_H264;
		break;
	}

	avmain2_log_info("get venc type: %d", type);

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
			avmain2_log_err("Invalid rate control mode.");
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
			avmain2_log_err("Invalid rate control mode.");
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
		avmain2_log_err("Invalid code type.");
		break;
	}

}

INT32 createOsdInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle,
                               MPI_OSD_CANVAS_ATTR_S *canvas)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_info("MPI_createOsdRgn() failed. err: %d", ret);
		return ret;
	}

	ret = MPI_getOsdCanvas(*handle, canvas);
	if (ret != MPI_SUCCESS) {
		avmain2_log_info("MPI_getOsdCanvas() failed. err: %d", ret);
		goto release;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		avmain2_log_info("Bind OSD %d to encoder channel %d failed. err: %d", *handle, bind->idx.chn, ret);
		goto release;
	}

	return ret;

release:

	MPI_destroyOsdRgn(*handle);

	return ret;
}

static INT32 moveOsdInstance(MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle)
{
	INT32 ret = MPI_SUCCESS;
	MPI_OSD_BIND_ATTR_S bind_old;
	ret = MPI_getOsdBindAttr(*handle, &bind_old);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Failed to get MPI_getOsdBindAttr");
	}
	bind_old.point.x = bind->point.x;
	bind_old.point.y = bind->point.y;
	ret = MPI_setOsdBindAttr(*handle, &bind_old);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Set OSD %d to encoder channel %d failed. err: %d", *handle, bind->idx.chn, ret);
	}

	return ret;
}

static INT32 __attribute__((unused)) moveOsdPmInstance(MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle)
{
	return moveOsdInstance(bind, handle);
}

static INT32 createOsdPmInstance(const MPI_OSD_RGN_ATTR_S *attr, const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE *handle)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_createOsdRgn(handle, attr);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("MPI_createOsdRgn() failed. err: %d", ret);
		return ret;
	}

	ret = MPI_bindOsdToChn(*handle, bind);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Bind OSD %d to encoder channel %d failed. err: %d", *handle, bind->idx.chn, ret);
		goto release;
	}
	return ret;

release:

	MPI_destroyOsdRgn(*handle);
	return ret;
}

static INT32 destroyOsdPmInstance(const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE handle)
{
	INT32 ret = MPI_SUCCESS;

	ret = MPI_unbindOsdFromChn(handle, bind);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("Unbind Pm %d to encoder channel %d failed. err: %d", handle, bind->idx.chn, ret);
	}

	ret = MPI_destroyOsdRgn(handle);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("MPI_destroyOsdRgn() failed. err: %d", ret);
	}

	return ret;
}

static INT32 destroyOsdInstance(const MPI_OSD_BIND_ATTR_S *bind, OSD_HANDLE handle)
{
	return destroyOsdPmInstance(bind, handle);
}

UINT16 roundingUpAlign16(UINT16 num)
{
	UINT16 num_align = ((num + 15) / 16) * 16;
	avmain2_log_debug("num = %d, %s, = %d", num, (num % 16) > 7 ? ">" : "<", num_align);

	return num_align;
}

UINT16 roundingDownAlign16(UINT16 num)
{
	UINT16 num_align = ((num) / 16) * 16;
	avmain2_log_debug("num = %d, %s, = %d", num, (num % 16) > 7 ? ">" : "<", num_align);

	return num_align;
}

static void setOsdAttr(const AGTX_OSD_CONF_INNER_S *region, const MPI_SIZE_S *chn_res, int region_idx, bool show,
                       MPI_OSD_RGN_ATTR_S *osd_attr, MPI_OSD_BIND_ATTR_S *osd_bind)
{
	osd_attr->show = show;
	osd_attr->qp_enable = false;
	osd_attr->color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544;
	osd_attr->osd_type = MPI_OSD_OVERLAY_BITMAP;
	osd_attr->priority = region_idx;

	/* OSD start x,y will over resolution case */
	MPI_RECT_POINT_S rect;
	rect.sx = roundingDownAlign16((region->start_x * chn_res->width) / 100);
	rect.sy = roundingDownAlign16((region->start_y * chn_res->height) / 100);
	rect.ex = rect.sx + osd_attr->size.width - 1;
	rect.ey = rect.sy + osd_attr->size.height - 1;

	if (rect.ex > roundingDownAlign16(chn_res->width) - 1) {
		avmain2_log_warn("need to align right end %d ~ %d %d", rect.sx, rect.ex,
		                 roundingUpAlign16(chn_res->width) - 1);
		/* x translate*/
		rect.sx -= roundingUpAlign16(rect.ex - (roundingDownAlign16(chn_res->width) - 1));
		rect.ex -= roundingUpAlign16(rect.ex - (roundingDownAlign16(chn_res->width) - 1));
	}

	if (rect.ey > roundingDownAlign16(chn_res->height) - 1) {
		avmain2_log_warn("need to align buttom end %d %d", rect.ey, roundingUpAlign16(chn_res->height) - 1);
		/* y translate*/
		rect.sy -= roundingUpAlign16(rect.ey - (roundingDownAlign16(chn_res->height) - 1));
		rect.ey -= roundingUpAlign16(rect.ey - (roundingDownAlign16(chn_res->height) - 1));
	}

	osd_bind->point.x = rect.sx;
	osd_bind->point.y = rect.sy;
	osd_bind->module = 0;
}

static void setOsdPmAttr(AGTX_OSD_PM_S *osd_param, int region_idx, const MPI_SIZE_S *chn_res,
                         MPI_OSD_RGN_ATTR_S *osd_attr, MPI_OSD_BIND_ATTR_S *osd_bind)
{
	osd_attr->show = 1;
	osd_attr->qp_enable = FALSE;
	osd_attr->color_format = MPI_OSD_COLOR_FORMAT_AYUV_3544;
	osd_attr->polygon.point_nums = 4;
	osd_attr->polygon.fill = 1;

	UINT16 AYUV_TABLE[8] = {
		0b1110000010001000, // AYUV_BLACK
		0b1110100101011111, // AYUV_RED
		0b1111001000110001, // AYUV_GREEN
		0b1110001111110110, // AYUV_BLUE
		0b1111101100001001, // AYUV_YELLOW
		0b1110101010111011, // AYUV_PURPLE
		0b1111011000001011, // AYUV_ORANGE
		0b1111111110001000 // AYUV_WHITE
	};
	osd_attr->polygon.color = ((osd_param->alpha << 13) + (AYUV_TABLE[MAX8(osd_param->color)] & 0x1fff));
	osd_attr->polygon.line_width = MPI_OSD_THICKNESS_MIN;

	avmain2_log_debug("set MPI_OSD_OVERLAY_U1_POLYGON[%d]", region_idx);
	osd_attr->osd_type = MPI_OSD_OVERLAY_U1_POLYGON;
	osd_attr->priority = region_idx;

	MPI_POINT_S st;
	UINT16 w, h;

	st.x = roundingUpAlign16((osd_param->start_x * chn_res->width) / 100);
	st.y = roundingUpAlign16((osd_param->start_y * chn_res->height) / 100);
	int end_x = roundingUpAlign16((osd_param->end_x * chn_res->width) / 100);
	int end_y = roundingUpAlign16((osd_param->end_y * chn_res->height) / 100);
	w = end_x - st.x;
	h = end_y - st.y;

	osd_attr->polygon.point[0].x = 0;
	osd_attr->polygon.point[0].y = 0;
	osd_attr->polygon.point[1].x = w;
	osd_attr->polygon.point[1].y = 0;
	osd_attr->polygon.point[2].x = w;
	osd_attr->polygon.point[2].y = h;
	osd_attr->polygon.point[3].x = 0;
	osd_attr->polygon.point[3].y = h;

	osd_attr->size.width = w;
	osd_attr->size.height = h;

	osd_bind->point.x = st.x;
	osd_bind->point.y = st.y;
	osd_bind->module = 0;

	avmain2_log_debug("output: st:(%d,%d), end(%d,%d), cr:(%d), sz:(%d,%d), bind(%d,%d)",
	                  osd_attr->polygon.point[0].x, osd_attr->polygon.point[0].y, osd_attr->polygon.point[2].x,
	                  osd_attr->polygon.point[2].y, osd_attr->polygon.color, osd_attr->size.width,
	                  osd_attr->size.height, osd_bind->point.x, osd_bind->point.y);

	return;
}

static int setImage(const AGTX_OSD_CONF_INNER_S *region, int strm_idx, int region_idx, bool isCreateInstance,
                    const MPI_SIZE_S *chn_res, MPI_OSD_RGN_ATTR_S *osd_attr, MPI_OSD_CANVAS_ATTR_S *canvas_attr,
                    MPI_OSD_BIND_ATTR_S *osd_bind)
{
	uint32_t img_width, img_height;
	bool show = true;
	FILE *fp;
	if (strcmp((const char *)region->type_spec, "icon:bell") == 0) {
		fp = fopen("/system/mpp/font/alarm_bell.ayuv", "rb");
		/* Alarm bell only apprear when MD trigger */
		show = false;
	} else {
		fp = fopen((const char *)region->type_spec, "rb");
	}

	if (fp == NULL) {
		avmain2_log_err("failed to open %s", (const char *)region->type_spec);
		return -ENODATA;
	}

	fread(&img_width, sizeof(uint32_t), 1, fp);
	fseek(fp, sizeof(uint32_t), SEEK_SET);
	fread(&img_height, sizeof(uint32_t), 1, fp);
	fseek(fp, sizeof(uint32_t) * 3, SEEK_SET);
	avmain2_log_debug("%d %d image w,h = (%d, %d)", strm_idx, region_idx, img_width, img_height);

	if (isCreateInstance) {
		osd_attr->size.width = roundingUpAlign16(img_width);
		osd_attr->size.height = roundingUpAlign16(img_height);
	}

	setOsdAttr(region, chn_res, region_idx, show, osd_attr, osd_bind);

	if (isCreateInstance) {
		if (createOsdInstance(osd_attr, osd_bind, &g_osd_chn_handle[strm_idx][region_idx],
		                      canvas_attr /*add src on this ptr*/) != 0) {
			avmain2_log_err("failed creat OSD[%d]n", region_idx);
			fclose(fp);
			return -ENXIO;
		}
	} else {
		if (moveOsdInstance(osd_bind, &g_osd_chn_handle[strm_idx][region_idx]) != 0) {
			avmain2_log_err("failed move OSD[%d]n", region_idx);
			return -ENXIO;
		}
	}

	char ayuv_tmp[img_width * img_height * 2];
	memset(&ayuv_tmp[0], 0, sizeof(ayuv_tmp));
	fread(&ayuv_tmp[0], img_width * img_height * 2, 1, fp);
	fclose(fp);
	for (int i = 0; (unsigned)i < img_height; i++) {
		memcpy((char *)canvas_attr->canvas_addr + (i * (osd_attr->size.width) * 2),
		       &ayuv_tmp[i * img_width * 2], img_width * 2);
	}

	/* MPI_updateOsdCanvas */
	int ret = MPI_SUCCESS;
	ret = MPI_updateOsdCanvas(g_osd_chn_handle[strm_idx][region_idx]);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("failed to update canvas, ret: %d", ret);
		MPI_destroyOsdRgn(g_osd_chn_handle[strm_idx][region_idx]);
	}
	return 0;
}

static int setText(const AGTX_OSD_CONF_INNER_S *region, int strm_idx, int region_idx, bool isCreateInstance,
                   const MPI_SIZE_S *chn_res, MPI_OSD_RGN_ATTR_S *osd_attr, MPI_OSD_CANVAS_ATTR_S *canvas_attr,
                   MPI_OSD_BIND_ATTR_S *osd_bind)
{
	uint32_t txt_width = 0;
	uint32_t txt_height = 0;

	unsigned char input_ascii = 0;

	FILE *fp = NULL;
	AyuvInfo_S info;
	if (chn_res->height > 1944) {
		fp = fopen("/system/mpp/font/1944_unlimted_font.ayuv", "rb");
	} else if (chn_res->height > 920) {
		fp = fopen("/system/mpp/font/960_1943_font.ayuv", "rb");
	} else if (chn_res->height > 360) {
		fp = fopen("/system/mpp/font/360_959_font.ayuv", "rb");
	} else {
		fp = fopen("/system/mpp/font/0_359_font.ayuv", "rb");
	}

	if (fp == NULL) {
		avmain2_log_err("failed to open font file");
		return -ENODATA;
	}
	fread(&info, sizeof(info), 1, fp);

	ASCII_INDEX idx[info.index_size];
	fseek(fp, info.index_offset, SEEK_SET);
	fread(idx, info.index_size, 1, fp);

	int string_lens = ((string_lens = strlen((const char *)region->type_spec)) >= MAX_TEXT_LEN) ? MAX_TEXT_LEN :
	                                                                                              string_lens;

	if (isCreateInstance) {
		/* if don't create instance(not in start case) , can't change size, but can change point and src*/

		txt_height = idx[0].image_height;
		/* count string width */
		for (int i = 0; i < string_lens; i++) {
			input_ascii = region->type_spec[i] - 32;
			txt_width += idx[input_ascii].image_width;
		}

		osd_attr->size.width = roundingUpAlign16(txt_width);
		osd_attr->size.height = roundingUpAlign16(txt_height);
	}

	setOsdAttr(region, chn_res, region_idx, true, osd_attr, osd_bind);

	if (isCreateInstance) {
		avmain2_log_debug("%d %d w, h %d %d", strm_idx, region_idx, txt_width, txt_height);
		if (createOsdInstance(osd_attr, osd_bind, &g_osd_chn_handle[strm_idx][region_idx],
		                      canvas_attr /*add src on this ptr*/) != 0) {
			avmain2_log_err("failed creat OSD[%d]n", region_idx);
			fclose(fp);
			return -ENXIO;
		}
	} else {
		if (moveOsdInstance(osd_bind, &g_osd_chn_handle[strm_idx][region_idx]) != 0) {
			avmain2_log_err("failed move OSD[%d]n", region_idx);
			return -ENXIO;
		}
	}

	char font_data[info.data_size];
	avmain2_log_debug("data size:%d offset: %d, str len %d", info.data_size, info.data_offset, string_lens);

	fseek(fp, info.data_offset, SEEK_SET);
	fread(&font_data[0], info.data_size, 1, fp);
	fclose(fp);

	uint32_t tmp_width = 0;

	for (int i = 0; i < string_lens; i++) {
		input_ascii = region->type_spec[i] - 32;
		avmain2_log_debug("'%d' w,h %d, %d, size: %d ", region->type_spec[i], idx[input_ascii].image_width,
		                  idx[input_ascii].image_height, idx[input_ascii].image_size);

		for (int j = 0; (unsigned)j < idx[input_ascii].image_height; j++) {
			/* print the text by line  */
			memcpy((char *)canvas_attr->canvas_addr + (j * osd_attr->size.width + tmp_width) * 2,
			       &font_data[idx[input_ascii].image_offset + idx[input_ascii].image_width * j * 2],
			       idx[input_ascii].image_width * 2);
		}
		tmp_width += idx[input_ascii].image_width;
	}

	/* MPI_updateOsdCanvas */
	if (MPI_updateOsdCanvas(g_osd_chn_handle[strm_idx][region_idx]) != MPI_SUCCESS) {
		avmain2_log_err("failed to update canvas");
		MPI_destroyOsdRgn(g_osd_chn_handle[strm_idx][region_idx]);
	}
	return 0;
}

static int setTimeStamp(const AGTX_OSD_CONF_INNER_S *region, int strm_idx, int region_idx, bool isCreateInstance,
                        const MPI_SIZE_S *chn_res, MPI_OSD_RGN_ATTR_S *osd_attr, MPI_OSD_CANVAS_ATTR_S *canvas_attr,
                        MPI_OSD_BIND_ATTR_S *osd_bind)
{
	uint32_t txt_width = 0;
	uint32_t txt_height = 0;

	avmain2_log_debug("type info, dynamic timestamp %d, %d", strm_idx, region_idx);

	FILE *fp = NULL;
	AyuvInfo_S info;
	if (chn_res->height > 1944) {
		fp = fopen("/system/mpp/font/1944_unlimted_font.ayuv", "rb");
	} else if (chn_res->height > 920) {
		fp = fopen("/system/mpp/font/960_1943_font.ayuv", "rb");
	} else if (chn_res->height > 360) {
		fp = fopen("/system/mpp/font/360_959_font.ayuv", "rb");
	} else {
		fp = fopen("/system/mpp/font/0_359_font.ayuv", "rb");
	}

	if (fp == NULL) {
		avmain2_log_err("failed to open font file");
		return -ENODATA;
	}
	fread(&info, sizeof(info), 1, fp);

	ASCII_INDEX idx[info.index_size];
	fseek(fp, info.index_offset, SEEK_SET);
	fread(idx, info.index_size, 1, fp);
	fclose(fp);

	if (isCreateInstance) {
		/* if don't create instance(not in start case) , can't change size, but can change point and src*/
		/* Max time format " %4u-%02u-%02u  %02u:%02u:%02u tt %s%s%s" */
		txt_width = 14 * idx['1' - 32].image_width + 5 * idx[' ' - 32].image_width + 3 * info.ascii_width +
		            2 * idx['-' - 32].image_width + 2 * idx[':' - 32].image_width + idx['A' - 32].image_width +
		            idx['M' - 32].image_width;
		txt_height = idx[0].image_height;
		osd_attr->size.width = roundingUpAlign16(txt_width);
		osd_attr->size.height = roundingUpAlign16(txt_height);
	}

	setOsdAttr(region, chn_res, region_idx, true, osd_attr, osd_bind);

	if (isCreateInstance) {
		avmain2_log_debug("%d %d w, h %d %d", strm_idx, region_idx, txt_width, txt_height);
		if (createOsdInstance(osd_attr, osd_bind, &g_osd_chn_handle[strm_idx][region_idx],
		                      canvas_attr /*add src on this ptr*/) != 0) {
			avmain2_log_err("failed creat OSD[%d]n", region_idx);
			return -ENXIO;
		}
	} else {
		if (moveOsdInstance(osd_bind, &g_osd_chn_handle[strm_idx][region_idx]) != 0) {
			avmain2_log_err("failed move OSD[%d]n", region_idx);
			return -ENXIO;
		}
	}

	/*but update timestamp canvas is in update thread*/

	return 0;
}

static int setOsdDateTimeFormat(char *spec, struct tm *gmt_tm, char *time_str, char *date_str)
{
	char date_format[16];
	char time_format[16];
	int timehalfday = 0;
	int hr = 0;

	char *token = strtok(spec, " ");
	if (token == NULL) {
		avmain2_log_err("Invalid date time format");
		return -EINVAL;
	}
	if (token != NULL) {
		strcpy(date_format, token);
		token = strtok(NULL, " ");
	}
	if (token != NULL) {
		strcpy(time_format, token);
		token = strtok(NULL, " ");
		if (token != NULL) { /* hh:mm:ss tt case */
			sprintf(time_format, "%s %s", time_format, token);
		}
	}

	/* AM. PM */
	if (gmt_tm->tm_hour >= 12) {
		hr = (gmt_tm->tm_hour > 12) ? gmt_tm->tm_hour - 12 : gmt_tm->tm_hour;
		timehalfday = 1;
	} else {
		hr = gmt_tm->tm_hour;
		timehalfday = 0;
	}

	if (strcmp(time_format, "h:mm:ss tt") == 0) {
		sprintf(time_str, " %2u:%02u:%02u %s ", hr, gmt_tm->tm_min, gmt_tm->tm_sec,
		        (timehalfday) ? "PM" : "AM");
	} else if (strcmp(time_format, "hh:mm:ss tt") == 0) {
		sprintf(time_str, " %02u:%02u:%02u %s ", hr, gmt_tm->tm_min, gmt_tm->tm_sec,
		        (timehalfday) ? "PM" : "AM");
	} else if (strcmp(time_format, "H:mm:ss") == 0) {
		sprintf(time_str, " %2u:%02u:%02u %s ", gmt_tm->tm_hour, gmt_tm->tm_min, gmt_tm->tm_sec, "  ");
	} else if (strcmp(time_format, "HH:mm:ss") == 0) {
		sprintf(time_str, " %02u:%02u:%02u %s ", gmt_tm->tm_hour, gmt_tm->tm_min, gmt_tm->tm_sec, "  ");
	} else {
		avmain2_log_err("Invalid time format ; %s", spec);
		return -EINVAL;
	}

	if (strcmp(date_format, "YYYY-MM-DD") == 0) {
		sprintf(date_str, " %4u-%02u-%02u ", gmt_tm->tm_year + 1900, gmt_tm->tm_mon + 1, gmt_tm->tm_mday);
	} else if (strcmp(date_format, "MM-DD-YYYY") == 0) {
		sprintf(date_str, " %02u-%02u-%4u ", gmt_tm->tm_mon + 1, gmt_tm->tm_mday, gmt_tm->tm_year + 1900);
	} else if (strcmp(date_format, "DD-MM-YYYY") == 0) {
		sprintf(date_str, " %02u-%02u-%4u ", gmt_tm->tm_mday, gmt_tm->tm_mon + 1, gmt_tm->tm_year + 1900);
	} else {
		avmain2_log_err("Invalid date format");
		return -EINVAL;
	}

	return 0;
}

static void *updateDateTime(void *data)
{
	AGTX_OSD_CONF_S *osd = (AGTX_OSD_CONF_S *)data;
	int ret = MPI_SUCCESS;

	long utc = time(NULL);
	struct tm gmt_tm = { 0 };
	char time_format[g_conf.strm.video_strm_cnt][32];
	char timeFormatString[64];
	char dateFormatstring[64];
	unsigned char input_ascii = 0;
	int weekday_strlen = osd->showWeekDay == 1 ? 3 : 0;

#ifdef OSD_DATETIME_CHINESE
	int weekday[7][3] = { { 95, 96, 103 }, { 95, 96, 97 },  { 95, 96, 98 }, { 95, 96, 99 },
		              { 95, 96, 100 }, { 95, 96, 101 }, { 95, 96, 102 } };
#else /* OSD_DATETIME_ENGLISH */
	int weekday[7][3] = { { 51, 53, 46 }, { 45, 47, 46 }, { 52, 53, 37 }, { 55, 37, 36 },
		              { 52, 40, 53 }, { 38, 50, 41 }, { 51, 33, 52 } };
#endif /* !OSD_DATETIME_CHINESE */

	MPI_OSD_CANVAS_ATTR_S canvas_attr;
	MPI_OSD_RGN_ATTR_S osd_attr;
	MPI_VENC_ATTR_S venc_attr = { 0 };
	MPI_ECHN echn_idx = { { 0 } };

	FILE *fp = NULL;
	AyuvInfo_S info[g_conf.strm.video_strm_cnt];
	ASCII_INDEX *idx[g_conf.strm.video_strm_cnt];
	char *font_data[g_conf.strm.video_strm_cnt];

	memset(&info[0], 0, sizeof(info));
	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		memset(&time_format[i], 0, 32);
	}

	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		if (exist_enc[i].res.width == 0) {
			avmain2_log_debug("enc[%d] not exist, skip", i);
			continue;
		}
		for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
			/* new conf maybe stop single  enc */
			echn_idx = MPI_ENC_CHN(g_conf.strm.video_strm[i].video_strm_idx);

			/* Use this method to check enc is running == check strm_en in old conf */
			if (MPI_ENC_getVencAttr(echn_idx, &venc_attr) != MPI_SUCCESS) {
				break;
			}

			if (osd->strm[i].region[j].type == AGTX_OSD_TYPE_INFO && osd->strm[i].region[j].enabled == 1) {
				if (g_conf.strm.video_strm[i].height > 1944) {
					fp = fopen("/system/mpp/font/1944_unlimted_font.ayuv", "rb");
					avmain2_log_notice("fopen for chn %d, %d", i, j);
				} else if (g_conf.strm.video_strm[i].height > 920) {
					fp = fopen("/system/mpp/font/960_1943_font.ayuv", "rb");
					avmain2_log_notice("fopen for chn %d, %d", i, j);
				} else if (g_conf.strm.video_strm[i].height > 360) {
					fp = fopen("/system/mpp/font/360_959_font.ayuv", "rb");
					avmain2_log_notice("fopen for chn %d, %d", i, j);
				} else {
					fp = fopen("/system/mpp/font/0_359_font.ayuv", "rb");
					avmain2_log_notice("fopen for chn %d, %d", i, j);
				}

				if (fp == NULL) {
					avmain2_log_err("failed to open font file");
					return (void *)NULL;
				}

				fread(&info[i], sizeof(info[i]), 1, fp);
				idx[i] = malloc(info[i].index_size);
				fseek(fp, info[i].index_offset, SEEK_SET);
				fread(idx[i], info[i].index_size, 1, fp);

				fseek(fp, info[i].data_offset, SEEK_SET);
				font_data[i] = malloc(info[i].data_size);
				fread(font_data[i], info->data_size, 1, fp);
				fclose(fp);

				strcpy(&time_format[i][0], (char *)&osd->strm[i].region[j].type_spec);

				break;
			}
		}
	}

	uint32_t tmp_width = 0;
	while (g_osd_run_flag) {
		utc = time(NULL);
		localtime_r(&utc, &gmt_tm);

		for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
			for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
				if (g_conf.strm.video_strm[i].strm_en != 1) {
					break;
				}
				if (osd->strm[i].region[j].type == AGTX_OSD_TYPE_INFO &&
				    osd->strm[i].region[j].enabled == 1) {
					/*one chn can only have one font data*/
					memset(timeFormatString, 0, sizeof(timeFormatString));
					memset(dateFormatstring, 0, sizeof(dateFormatstring));
					setOsdDateTimeFormat((char *)&time_format[i][0], &gmt_tm, &timeFormatString[0],
					                     &dateFormatstring[0]);
					
					ret = MPI_getOsdCanvas(g_osd_chn_handle[i][j], &canvas_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("MPI_getOsdCanvas()[%d][%d] failed. ret: %d", i, j, ret);
						break;
					}

					ret = MPI_getOsdRgnAttr(g_osd_chn_handle[i][j], &osd_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to get timestamp attr[%d][%d], ret: %d", i, j, ret);
						break;
					}

					tmp_width = 0;

					/* date */
					for (int k = 0; (unsigned)k < strlen(dateFormatstring); k++) {
						input_ascii = dateFormatstring[k] - 32;

						for (int l = 0; (unsigned)l < idx[i][input_ascii].image_height; l++) {
							/* print the text by line  */
							memcpy((char *)canvas_attr.canvas_addr +
							               (l * osd_attr.size.width + tmp_width) * 2,
							       &font_data[i][idx[i][input_ascii].image_offset +
							                     idx[i][input_ascii].image_width * l * 2],
							       idx[i][input_ascii].image_width * 2);
						}
						tmp_width += idx[i][input_ascii].image_width;
					}
					/* day */
					for (int k = 0; k < weekday_strlen; k++) {
						input_ascii = weekday[gmt_tm.tm_wday][k];
						for (int l = 0; (unsigned)l < idx[i][input_ascii].image_height; l++) {
							/* print the text by line  */
							memcpy((char *)canvas_attr.canvas_addr +
							               (l * osd_attr.size.width + tmp_width) * 2,
							       &font_data[i][idx[i][input_ascii].image_offset +
							                     idx[i][input_ascii].image_width * l * 2],
							       idx[i][input_ascii].image_width * 2);
						}
						tmp_width += idx[i][input_ascii].image_width;
					}

					/* time */
					for (int k = 0; (unsigned)k < strlen(timeFormatString); k++) {
						input_ascii = timeFormatString[k] - 32;
						for (int l = 0; (unsigned)l < idx[i][input_ascii].image_height; l++) {
							/* print the text by line  */
							memcpy((char *)canvas_attr.canvas_addr +
							               (l * osd_attr.size.width + tmp_width) * 2,
							       &font_data[i][idx[i][input_ascii].image_offset +
							                     idx[i][input_ascii].image_width * l * 2],
							       idx[i][input_ascii].image_width * 2);
						}
						tmp_width += idx[i][input_ascii].image_width;
					}

					/* MPI_updateOsdCanvas */
					ret = MPI_updateOsdCanvas(g_osd_chn_handle[i][j]);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to update canvas, ret:%d", ret);
						MPI_destroyOsdRgn(g_osd_chn_handle[i][j]);
					}
				}
			}
		}
		sleep(1);
	}

	/*Free dynamic text data*/
	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		if (exist_enc[i].res.width == 0) {
			avmain2_log_debug("enc[%d] not exist, skip", i);
			continue;
		}
		for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
			/* new conf maybe stop single  enc */
			echn_idx = MPI_ENC_CHN(g_conf.strm.video_strm[i].video_strm_idx);
			/* Use this method to check enc is running == check strm_en in old conf */
			if (MPI_ENC_getVencAttr(echn_idx, &venc_attr) != MPI_SUCCESS) {
				break;
			}
			if (osd->strm[i].region[j].type == AGTX_OSD_TYPE_INFO &&
			    (osd->strm[i].region[j].enabled == 1)) {
				avmain2_log_notice("free font data[%d, %d]", i, j);
				if (idx[i] != NULL) {
					free(idx[i]);
				} else {
					avmain2_log_info("ascii[%d] is null", i);
				}

				if (font_data[i] != NULL) {
					free(font_data[i]);
				} else {
					avmain2_log_info("font[%d] is null", i);
				}
			}
		}
	}

	return 0;
}

static int createTimestampThread()
{
	/* Create osd timestamp thread*/
	if (isTimestampEnabled(&g_conf.osd) == true) {
		g_osd_run_flag = 1;
		if (pthread_create(&threadUpdateDateTime, NULL, (void *)updateDateTime, (void *)&g_conf.osd)) {
			avmain2_log_err("Create thread to DateTime failed.");
			return -ENXIO;
		}

		if (pthread_setname_np(threadUpdateDateTime, OSD_THREAD_NAME)) {
			avmain2_log_err("failed to set thread %s name", OSD_THREAD_NAME);
		}
	}

	return 0;
}

static int destroyTimestampThread()
{
	if (g_osd_run_flag != 0) {
		g_osd_run_flag = 0;
		if (pthread_join(threadUpdateDateTime, NULL)) {
			avmain2_log_err("Stop DateTime thread failed.");
			return -ENXIO;
		}
	}

	return 0;
}

int NODE_initEnc(void)
{
	return 0;
}

int NODE_startEnc(void)
{
	INT32 ret = MPI_FAILURE;
	MPI_ECHN echn_idx = { { 0 } };
	MPI_ENC_CHN_ATTR_S chn_attr = {
		.res = { .width = 0, .height = 0 },
		.max_res = { .width = 0, .height = 0 },
	};
	MPI_ENC_BIND_INFO_S bind_info = { { { 0 } } };
	MPI_VENC_ATTR_S venc_attr = { 0 };
	MPI_VENC_ATTR_EX_S venc_attr_ex = { 0 };
	AGTX_STRM_PARAM_S *conf = NULL;

	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { .x = 0, .y = 0 },
		.module = 0,
		.idx = { { .chn = 0, .dummy0 = 0, .dummy1 = 0, .dummy2 = 0 } },
	};
	MPI_OSD_RGN_ATTR_S osd_attr = { 0 };

	/*g_osd_chn_handler INVALID num*/
	memset(g_osd_chn_handle, OSD_INVALID_HANDLER, sizeof(g_osd_chn_handle));
	/*reset exist_enc to invalid number*/
	resetExistEncRecord();

	if (isOsdOverNumber(&g_conf.osd, &g_conf.osd_pm)) {
		avmain2_log_err("osd & osd pm overflow");
		return -EOVERFLOW;
	}

	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		if (g_conf.strm.video_strm[i].strm_en == 1) {
			conf = &g_conf.strm.video_strm[i];
			echn_idx = MPI_ENC_CHN(conf->video_strm_idx);
			echn_idx.chn = conf->video_strm_idx;
			chn_attr.res.width = conf->width;
			chn_attr.res.height = conf->height;
			chn_attr.max_res.width = conf->width;
			chn_attr.max_res.height = conf->height;

			ret = MPI_ENC_createChn(echn_idx, &chn_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Create encoder channel %d failed. ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}

			/*OSD & OSD Privacy mask*/
			osd_bind.idx = echn_idx;
			MPI_OSD_CANVAS_ATTR_S canvas_attr;
			/* OSD */
			/* set static osd src */
			for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
				if (g_conf.osd.strm[i].region[j].enabled == 1) {
					switch (g_conf.osd.strm[i].region[j].type) {
					case AGTX_OSD_TYPE_IMAGE:
						if (setImage(&g_conf.osd.strm[i].region[j], i, j, true, &chn_attr.res,
						             &osd_attr, &canvas_attr, &osd_bind) != 0) {
							return -EINVAL;
						}
						break;
					case AGTX_OSD_TYPE_TEXT:
						avmain2_log_debug("type text");
						if (setText(&g_conf.osd.strm[i].region[j], i, j, true, &chn_attr.res,
						            &osd_attr, &canvas_attr, &osd_bind) != 0) {
							return -EINVAL;
						}
						break;
					case AGTX_OSD_TYPE_INFO:

						if (setTimeStamp(&g_conf.osd.strm[i].region[j], i, j, true,
						                 &chn_attr.res, &osd_attr, &canvas_attr,
						                 &osd_bind) != 0) {
							avmain2_log_err("failed to set OSD text");
							return -EINVAL;
						}
						break;
					default:
						avmain2_log_err("Unknown OSD type: %d",
						                g_conf.osd.strm[i].region[j].type);
						break;
					}
				}
			}
			/* OSD PM */

			for (int j = 0; j < MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE; j++) {
				if (g_conf.osd_pm.conf[i].param[j].enabled == 1) {
					setOsdPmAttr(&g_conf.osd_pm.conf[i].param[j], j, &chn_attr.res, &osd_attr,
					             &osd_bind);

					if (createOsdPmInstance(
					            &osd_attr, &osd_bind,
					            &g_osd_chn_handle[i][MPI_OSD_MAX_BIND_CHANNEL /*shift OSD*/ + j]) !=
					    0) {
						avmain2_log_err("failed creat pm[%d]n", j);
						return -ENXIO;
					}
				}
			}

			bind_info.idx = MPI_VIDEO_CHN(0, conf->video_strm_idx);
			ret = MPI_ENC_bindToVideoChn(echn_idx, &bind_info);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Bind encoder channel %d failed.ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}

			setVencAttr(&venc_attr, conf);
			ret = MPI_ENC_setVencAttr(echn_idx, &venc_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Set VENC attr for encoder channel %d failed. ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}

			venc_attr_ex.obs = conf->obs;
			venc_attr_ex.obs_off_period = conf->obs_off_period;
			ret = MPI_ENC_setVencAttrEx(echn_idx, &venc_attr_ex);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Set VENC attr EX for encoder channel %d failed. ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}

			ret = MPI_ENC_startChn(echn_idx);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Start encoder channel %d failed. ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}

			addExistEncRecord(&chn_attr, i);

			avmain2_log_info("Start encoder channel %d succeeded! ret: %d", echn_idx.chn, ret);
		}
	}

	/** create venc_extend obj */
	for (int enc_chn_idx = 0; (unsigned)enc_chn_idx < g_conf.strm.video_strm_cnt; enc_chn_idx++) {
		if (g_conf.strm.video_strm[enc_chn_idx].strm_en != 1) {
			continue;
		}

		VencExtendInfo *vencExtendInfo = (VencExtendInfo *)malloc(sizeof(VencExtendInfo));

		vencExtendInfo->venc_extend[SRC_TYPE_DAY][0] = '\0';
		vencExtendInfo->venc_extend[SRC_TYPE_IR][0] = '\0';
		vencExtendInfo->venc_extend[SRC_TYPE_LIGHT][0] = '\0';
		vencExtendInfo->chn = MPI_ENC_CHN(g_conf.strm.video_strm[enc_chn_idx].video_strm_idx);

		sprintf(vencExtendInfo->venc_extend[SRC_TYPE_DAY], "%s/venc_extend_%d.ini", DIP_FILE_PATH, enc_chn_idx);

		switch (g_conf.layout.video_layout[enc_chn_idx].window_array[0].path_bmp) {
		case 1:
			/* bmp = 0001, path = 0 */
			vencExtendInfo->path = MPI_INPUT_PATH(g_conf.layout.video_dev_idx, 0);
			break;
		case 2:
			/* bmp = 0010, path = 1 */
			vencExtendInfo->path = MPI_INPUT_PATH(g_conf.layout.video_dev_idx, 1);
			break;
		case 4:
			/* bmp = 0100, path = 2 */
			vencExtendInfo->path = MPI_INPUT_PATH(g_conf.layout.video_dev_idx, 2);
			break;
		default:
			break;
		}

		if (access(vencExtendInfo->venc_extend[SRC_TYPE_DAY], R_OK) != 0) {
			memset(vencExtendInfo->venc_extend[SRC_TYPE_DAY], 0, PATH_MAX * sizeof(char));
		}

		sprintf(vencExtendInfo->venc_extend[SRC_TYPE_IR], "%s/venc_extend_ir_%d.ini", DIP_FILE_PATH,
		        enc_chn_idx);
		if (access(vencExtendInfo->venc_extend[SRC_TYPE_IR], R_OK) != 0) {
			sprintf(vencExtendInfo->venc_extend[SRC_TYPE_IR], "%s",
			        vencExtendInfo->venc_extend[SRC_TYPE_DAY]);
			avmain2_log_warn("IR mode is not exist, will apply the day mode venc_extend %s\n",
			                 vencExtendInfo->venc_extend[SRC_TYPE_DAY]);
		}

		sprintf(vencExtendInfo->venc_extend[SRC_TYPE_LIGHT], "%s/venc_extend_light_%d.ini", DIP_FILE_PATH,
		        enc_chn_idx);
		if (access(vencExtendInfo->venc_extend[SRC_TYPE_LIGHT], R_OK) != 0) {
			sprintf(vencExtendInfo->venc_extend[SRC_TYPE_LIGHT], "%s",
			        vencExtendInfo->venc_extend[SRC_TYPE_DAY]);
			avmain2_log_warn("Light mode is not exist, will apply the day mode venc_extend %s\n",
			                 vencExtendInfo->venc_extend[SRC_TYPE_DAY]);
		}

		LL_APPEND(g_vencExtendInfo, vencExtendInfo);
	}

	ret = SAMPLE_initVencExtendInfo(g_vencExtendInfo);
	if (ret != MPI_SUCCESS) {
		avmain2_log_err("SAMPLE_initVencExtendInfo() failed\n");
	}

	/* Create osd timestamp thread*/
	createTimestampThread();

	return 0;
}

static void deinitVencExtendInfo(VencExtendInfo **head)
{
	SAMPLE_deinitVencExtendInfo();

	VencExtendInfo *item, *test;
	LL_FOREACH_SAFE(*head, item, test)
	{
		LL_DELETE(*head, item);
		free(item);
	}
	*head = NULL;
}

int NODE_stopEnc(void)
{
	INT32 ret = MPI_FAILURE;
	MPI_ECHN echn_idx;
	AGTX_STRM_PARAM_S *conf;
	MPI_OSD_BIND_ATTR_S osd_bind = {
		.point = { .x = 0, .y = 0 },
		.module = 0,
		.idx = { { .chn = 0, .dummy0 = 0, .dummy1 = 0, .dummy2 = 0 } },
	};
	MPI_VENC_ATTR_S venc_attr = { 0 };

	destroyTimestampThread();

	for (int i = 0; i < MAX_VENC_STREAM; i++) {
		/* new conf maybe stop single  enc */
		conf = &g_conf.strm.video_strm[i];
		echn_idx = MPI_ENC_CHN(conf->video_strm_idx);
		/*Use exist_enc.res to check is running or not*/
		if (exist_enc[i].res.width == 0) {
			avmain2_log_debug("enc[%d] not exist, skip", i);
			continue;
		}

		if (MPI_ENC_getVencAttr(echn_idx, &venc_attr) == MPI_SUCCESS) {
			osd_bind.idx = echn_idx;

			ret = MPI_ENC_stopChn(echn_idx);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Stop encoder channel %d failed. ret:%d", echn_idx.chn, ret);
				return -ENXIO;
			}
			/*OSD & OSD PM*/
			for (int j = 0; j < MPI_OSD_MAX_BIND_CHANNEL; j++) {
				avmain2_log_info("destroy osd(%d, %d)", i, j);
				if (g_osd_chn_handle[i][j] != OSD_INVALID_HANDLER) {
					if (destroyOsdInstance(&osd_bind, g_osd_chn_handle[i][j]) != 0) {
					}
				}
			}

			for (int j = 0; j < MPI_OSD_MAX_BIND_CHANNEL; j++) {
				avmain2_log_info("destroy osd pm(%d, %d)", i, j);
				if (g_osd_chn_handle[i][MPI_OSD_MAX_BIND_CHANNEL + j] != OSD_INVALID_HANDLER) {
					if (destroyOsdPmInstance(&osd_bind,
					                         g_osd_chn_handle[i][MPI_OSD_MAX_BIND_CHANNEL + j]) !=
					    0) {
					}
				}
			}

			ret = MPI_ENC_unbindFromVideoChn(echn_idx);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Unbind encoder channel %d failed. ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}
			ret = MPI_ENC_destroyChn(echn_idx);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Destroy encoder channel %d failed. ret: %d", echn_idx.chn, ret);
				return -ENXIO;
			}

			avmain2_log_info("Stop encoder channel %d succeeded! ret: %d", echn_idx.chn, ret);
		}
	}

	/*g_osd_chn_handler INVALID num*/
	memset(g_osd_chn_handle, OSD_INVALID_HANDLER, sizeof(g_osd_chn_handle));
	/*reset exist_enc to invalid number*/
	resetExistEncRecord();

	deinitVencExtendInfo(&g_vencExtendInfo);

	return 0;
}

int NODE_exitEnc(void)
{
	avmain2_log_info("Exit encoder succeeded!");
	return 0;
}

int NODE_setEnc(int cmd_id, void *data)
{
	MPI_ECHN echn_idx;
	MPI_VENC_ATTR_S venc_attr;
	MPI_VENC_ATTR_EX_S venc_attr_ex;
	MPI_OSD_RGN_ATTR_S osd_attr;
	MPI_OSD_CANVAS_ATTR_S canvas_attr;
	MPI_ENC_CHN_ATTR_S chn_attr;
	MPI_OSD_BIND_ATTR_S osd_bind;

	INT32 ret = MPI_FAILURE;


	if (cmd_id == STRM_CONF) {
		avmain2_log_info("AGTX_CMD_VIDEO_STRM_CONF, gop, br, rc_mode attr, obs");
		AGTX_STRM_CONF_S *new_strm = (AGTX_STRM_CONF_S *)data;

		for (int i = 0; (unsigned)i < new_strm->video_strm_cnt; i++) {
			if (!new_strm->video_strm[i].strm_en) {
				continue;
			}

			echn_idx = MPI_ENC_CHN(new_strm->video_strm[i].video_strm_idx);
			ret = MPI_ENC_getVencAttr(echn_idx, &venc_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Get enc[%d] attr failed, ret:%d",
				                new_strm->video_strm[i].video_strm_idx, ret);
				return -EINVAL;
			}

			ret = MPI_ENC_getVencAttrEx(echn_idx, &venc_attr_ex);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Get enc[%d] attr EX failed, ret:%d", new_strm->video_strm[i].video_strm_idx, ret);
				return -EINVAL;
			}

			setVencAttr(&venc_attr, &new_strm->video_strm[i]);

			ret =  MPI_ENC_setVencAttr(echn_idx, &venc_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Set encoder attribute channel %d failed. ret: %d", echn_idx.chn, ret);
				return -EINVAL;
			}

			venc_attr_ex.obs = new_strm->video_strm[i].obs;
			venc_attr_ex.obs_off_period = new_strm->video_strm[i].obs_off_period;

			ret = MPI_ENC_setVencAttrEx(echn_idx, &venc_attr_ex);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("Set encoder attribute channel %d failed. ret: %d", echn_idx.chn, ret);
				return -EINVAL;
			}
		}
	}

	if (cmd_id == OSD_SHOW_WEEK_DAY) {
		avmain2_log_info("AGTX_CMD_OSD_CONF change OSD_SHOW_WEEK_DAY");

		destroyTimestampThread();
		createTimestampThread();
	}

	if (cmd_id == OSD_SRC) {
		avmain2_log_info("AGTX_CMD_OSD_CONF change OSD_SRC");
		AGTX_OSD_CONF_S *new_osd = (AGTX_OSD_CONF_S *)data;
		destroyTimestampThread();

		for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
			if (!g_conf.strm.video_strm[i].strm_en) {
				continue;
			}
			echn_idx = MPI_ENC_CHN(g_conf.strm.video_strm[i].video_strm_idx);
			echn_idx.chn = g_conf.strm.video_strm[i].video_strm_idx;
			osd_bind.idx = echn_idx;

			chn_attr.res.width = g_conf.strm.video_strm[i].width;
			chn_attr.res.height = g_conf.strm.video_strm[i].height;

			for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
				if (new_osd->strm[i].region[j].enabled == 1) {
					ret = MPI_getOsdCanvas(g_osd_chn_handle[i][j], &canvas_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("MPI_getOsdCanvas()[%d][%d] failed. ret: %d", i, j,
						                ret);
						break;
					}

					ret = MPI_getOsdRgnAttr(g_osd_chn_handle[i][j], &osd_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to get timestamp attr[%d][%d], ret: %d", i, j, ret);
						break;
					}

					switch (new_osd->strm[i].region[j].type) {
					case AGTX_OSD_TYPE_IMAGE:
						if (setImage(&new_osd->strm[i].region[j], i, j, false, &chn_attr.res,
						             &osd_attr, &canvas_attr, &osd_bind) != 0) {
							return -EINVAL;
						}
						break;
					case AGTX_OSD_TYPE_TEXT:
						if (setText(&new_osd->strm[i].region[j], i, j, false, &chn_attr.res,
						            &osd_attr, &canvas_attr, &osd_bind) != 0) {
							return -EINVAL;
						}
						break;
					case AGTX_OSD_TYPE_INFO:
						if (setTimeStamp(&new_osd->strm[i].region[j], i, j, false,
						                 &chn_attr.res, &osd_attr, &canvas_attr,
						                 &osd_bind) != 0) {
							return -EINVAL;
						}
						break;
					default:
						avmain2_log_err("Unknown OSD type: %d",
						                g_conf.osd.strm[i].region[j].type);
						break;
					}
				}
			}
		}

		createTimestampThread();
	}
	if (cmd_id == OSD_PM) {
		avmain2_log_info("AGTX_CMD_OSD_PM_CONF change OSD_SRC");
		AGTX_OSD_PM_CONF_S *new_osd_pm = (AGTX_OSD_PM_CONF_S *)data;

		for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
			if (!g_conf.strm.video_strm[i].strm_en) {
				continue;
			}
			echn_idx = MPI_ENC_CHN(g_conf.strm.video_strm[i].video_strm_idx);
			echn_idx.chn = g_conf.strm.video_strm[i].video_strm_idx;
			osd_bind.idx = echn_idx;

			chn_attr.res.width = g_conf.strm.video_strm[i].width;
			chn_attr.res.height = g_conf.strm.video_strm[i].height;
			for (int j = 0; j < MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE; j++) {
				if (new_osd_pm->conf[i].param[j].enabled == 1) {
					ret = MPI_getOsdRgnAttr(g_osd_chn_handle[i][MPI_OSD_MAX_BIND_CHANNEL + j],
					                      &osd_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to get osd pm attr[%d][%d], ret: %d", i, j,
						                ret);
					}

					setOsdPmAttr(&new_osd_pm->conf[i].param[j], j, &chn_attr.res, &osd_attr,
					             &osd_bind);
					ret = MPI_setOsdRgnAttr(g_osd_chn_handle[i][MPI_OSD_MAX_BIND_CHANNEL + j],
					                      &osd_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to get timestamp attr[%d][%d], ret: %d", i, j, ret);
					}
				}
			}
		}
	}

	if (cmd_id == TD_ALARM || cmd_id == MD_ALARM) {
		avmain2_log_debug("show alarm bell");
		for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
			for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
				if (g_conf.osd.strm[i].region[j].type == AGTX_OSD_TYPE_IMAGE &&
				    strcmp((const char *)g_conf.osd.strm[i].region[j].type_spec, "icon:bell") == 0) {
					if (g_osd_chn_handle[i][j] == OSD_INVALID_HANDLER) {
						break;
					}

					ret = MPI_getOsdRgnAttr(g_osd_chn_handle[i][j], &osd_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to get alarm bell attr, ret: %d", ret);
						break;
					}
					osd_attr.show = 1;
					MPI_setOsdRgnAttr(g_osd_chn_handle[i][j], &osd_attr);
				}
			}
		}
	}
	if (cmd_id == TD_ALARM_END || cmd_id == MD_ALARM_END) {
		avmain2_log_debug("alarm bell disable");
		for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
			for (int j = 0; j < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; j++) {
				if (g_conf.osd.strm[i].region[j].type == AGTX_OSD_TYPE_IMAGE &&
				    strcmp((const char *)g_conf.osd.strm[i].region[j].type_spec, "icon:bell") == 0) {
					if (g_osd_chn_handle[i][j] == OSD_INVALID_HANDLER) {
						break;
					}
					ret = MPI_getOsdRgnAttr(g_osd_chn_handle[i][j], &osd_attr);
					if (ret != MPI_SUCCESS) {
						avmain2_log_err("failed to get alarm bell attr, ret: %d", ret);
						break;
					}
					osd_attr.show = 0;
					MPI_setOsdRgnAttr(g_osd_chn_handle[i][j], &osd_attr);
				}
			}
		}
	}

	return 0;
}