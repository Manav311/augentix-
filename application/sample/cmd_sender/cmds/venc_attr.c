#include "cmdparser.h"

#include "mpi_enc.h"
#include "cmd_util.h"

#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum opt_val {
	OV_HELP = 300, // helper args
	OV_ARGS,
	OV_GET_ATTR,
	OV_ENC_IDX = 400, // ENC args
	OV_CODEC,
	OV_PROFILE,
	OV_RCMODE = 500, // RC common args (CBR/VBR/SBR)
	OV_BR,
	OV_FPS,
	OV_GOP,
	OV_FLUC,
	OV_SCENE,
	OV_REGRESSION,
	OV_I_CONT,
	OV_IQP_OFFSET,
	OV_MIN_QP,
	OV_MAX_QP,
	OV_MIN_QFACTOR,
	OV_MAX_QFACTOR,
	OV_MTL_LV,
	OV_MTL_QP,
	OV_MTL_QFACTOR,
	OV_MAX_BR = 600, // VBR args
	OV_QUALITY,
	OV_ADJ_THRES = 700, // SBR args
	OV_ADJ_TIME,
	OV_CONVERGE,
	OV_C_IQP = 800, // CQP args
	OV_C_PQP,
	OV_C_QFACTOR,
	OV_OBS = 900, // OBS args
	OV_OBS_OFF_PERIOD,
} OptVal;

typedef struct cmd_venc_attr {
	MPI_VENC_ATTR_S venc_attr;
	MPI_VENC_ATTR_EX_S venc_ex;
	bool enable_echn[MPI_MAX_ENC_CHN_NUM];
} CmdVencAttr;

typedef struct dict {
	char *key;
	int val;
} Dict;

typedef struct parse_data {
	int enc_idx;
	int codec_type;
	int profile;
	int rc_mode;
	int bit_rate;
	int frm_rate_o;
	int gop_size;
	int fluc_level;
	int scene_smooth;
	int regression_speed;
	int i_continue_weight;
	int i_qp_offset;
	int min_qp;
	int max_qp;
	int min_q_factor;
	int max_q_factor;
	int motion_tolerance_level;
	int motion_tolerance_qp;
	int motion_tolerance_qfactor;
	int max_bit_rate;
	int quality_level_index;
	int adjust_br_thres_pc;
	int adjust_step_times;
	int converge_frame;
	int i_frame_qp;
	int p_frame_qp;
	int q_factor;
	int obs;
	int obs_off_period;
} ParseData;

static struct option opts[] = {
	{ "vhelp", no_argument, NULL, OV_HELP },
	{ "vh", no_argument, NULL, OV_HELP }, // short version of --vhelp
	{ "vargs", no_argument, NULL, OV_ARGS },
	{ "va", no_argument, NULL, OV_ARGS }, // short version of --vargs
	{ "get_vattr", no_argument, NULL, OV_GET_ATTR },
	{ "gva", no_argument, NULL, OV_GET_ATTR }, // short version of --get_vattr
	{ "enc_idx", optional_argument, NULL, OV_ENC_IDX },
	{ "eid", optional_argument, NULL, OV_ENC_IDX }, // short version of --enc_idx
	{ "codec_type", optional_argument, NULL, OV_CODEC },
	{ "codec", optional_argument, NULL, OV_CODEC }, // short version of --codec_type
	{ "profile", optional_argument, NULL, OV_PROFILE },
	{ "prof", optional_argument, NULL, OV_PROFILE }, // short version of --profile
	{ "rc_mode", optional_argument, NULL, OV_RCMODE },
	{ "rcm", optional_argument, NULL, OV_RCMODE }, // short version of --rc_mode
	{ "bit_rate", optional_argument, NULL, OV_BR },
	{ "br", optional_argument, NULL, OV_BR }, // short version of --bit_rate
	{ "enc_fps", optional_argument, NULL, OV_FPS },
	{ "fps", optional_argument, NULL, OV_FPS }, // short version of --enc_fps
	{ "gop_size", optional_argument, NULL, OV_GOP },
	{ "gop", optional_argument, NULL, OV_GOP }, // short version of --gop_size
	{ "fluc_level", optional_argument, NULL, OV_FLUC },
	{ "fluc", optional_argument, NULL, OV_FLUC }, // short version of --fluc_level
	{ "scene_smooth", optional_argument, NULL, OV_SCENE },
	{ "ss", optional_argument, NULL, OV_SCENE }, // short version of --scene_smooth
	{ "regression_speed", optional_argument, NULL, OV_REGRESSION },
	{ "rgs", optional_argument, NULL, OV_REGRESSION }, // short version of --regression_speed
	{ "i_continue_weight", optional_argument, NULL, OV_I_CONT },
	{ "icw", optional_argument, NULL, OV_I_CONT }, // short version of --i_continue_weight
	{ "i_qp_offset", optional_argument, NULL, OV_IQP_OFFSET },
	{ "iqo", optional_argument, NULL, OV_IQP_OFFSET }, // short version of --i_qp_offset
	{ "min_qp", optional_argument, NULL, OV_MIN_QP },
	{ "minqp", optional_argument, NULL, OV_MIN_QP }, // short version of --min_qp
	{ "max_qp", optional_argument, NULL, OV_MAX_QP },
	{ "maxqp", optional_argument, NULL, OV_MAX_QP }, // short version of --max_qp
	{ "min_q_factor", optional_argument, NULL, OV_MIN_QFACTOR },
	{ "minqf", optional_argument, NULL, OV_MIN_QFACTOR }, // short version of --min_q_factor
	{ "max_q_factor", optional_argument, NULL, OV_MAX_QFACTOR },
	{ "maxqf", optional_argument, NULL, OV_MAX_QFACTOR }, // short version of --max_q_factor
	{ "motion_tolerance_level", optional_argument, NULL, OV_MTL_LV },
	{ "mtllv", optional_argument, NULL, OV_MTL_LV }, // short version of --motion_tolerance_level
	{ "motion_tolerance_qp", optional_argument, NULL, OV_MTL_QP },
	{ "mtlqp", optional_argument, NULL, OV_MTL_QP }, // short version of --motion_tolerance_qp
	{ "motion_tolerance_qfactor", optional_argument, NULL, OV_MTL_QFACTOR },
	{ "mtlqf", optional_argument, NULL, OV_MTL_QFACTOR }, // short version of --motion_tolerance_qfactor
	{ "max_bit_rate", optional_argument, NULL, OV_MAX_BR },
	{ "mbr", optional_argument, NULL, OV_MAX_BR }, // short version of --max_bit_rate
	{ "quality_level_index", optional_argument, NULL, OV_QUALITY },
	{ "qli", optional_argument, NULL, OV_QUALITY }, // short version of --quality_level_index
	{ "adjust_br_thres_pc", optional_argument, NULL, OV_ADJ_THRES },
	{ "adjbr", optional_argument, NULL, OV_ADJ_THRES }, // short version of --adjust_br_thres_pc
	{ "adjust_step_times", optional_argument, NULL, OV_ADJ_TIME },
	{ "adjst", optional_argument, NULL, OV_ADJ_TIME }, // short version of --adjust_step_times
	{ "converge_frame", optional_argument, NULL, OV_CONVERGE },
	{ "conv", optional_argument, NULL, OV_CONVERGE }, // short version of --converge_frame
	{ "i_frame_qp", optional_argument, NULL, OV_C_IQP },
	{ "icqp", optional_argument, NULL, OV_C_IQP }, // short version of --i_frame_qp
	{ "p_frame_qp", optional_argument, NULL, OV_C_PQP },
	{ "pcqp", optional_argument, NULL, OV_C_PQP }, // short version of --p_frame_qp
	{ "q_factor", optional_argument, NULL, OV_C_QFACTOR },
	{ "cqf", optional_argument, NULL, OV_C_QFACTOR }, // short version of --q_factor
	{ "obs", optional_argument, NULL, OV_OBS },
	{ "obs_off_period", optional_argument, NULL, OV_OBS_OFF_PERIOD },
	{ "obsop", optional_argument, NULL, OV_OBS_OFF_PERIOD }, // short version of --obs_off_period
	{ 0 }
};

static Dict codec_dict[MPI_VENC_TYPE_NUM] = {
	{ "H264", MPI_VENC_TYPE_H264 },
	{ "H265", MPI_VENC_TYPE_H265 },
	{ "MJPEG", MPI_VENC_TYPE_MJPEG },
	{ "JPEG", MPI_VENC_TYPE_JPEG },
};

static Dict profile_dict[MPI_PRFL_NUM] = {
	{ "BASELINE", MPI_PRFL_BASELINE },
	{ "MAIN", MPI_PRFL_MAIN },
	{ "HIGH", MPI_PRFL_HIGH },
};

static Dict rc_mode_dict[MPI_RC_MODE_NUM] = {
	{ "VBR", MPI_RC_MODE_VBR },
	{ "CBR", MPI_RC_MODE_CBR },
	{ "SBR", MPI_RC_MODE_SBR },
	{ "CQP", MPI_RC_MODE_CQP },
};

// =========================================================================== //
//  HELPER FUNCTIONS
// =========================================================================== //

static int str2codec(const char *str)
{
	int i;

	for (i = 0; i < MPI_VENC_TYPE_NUM; i++) {
		if (strcmp(codec_dict[i].key, str) == 0) {
			return codec_dict[i].val;
		}
	}

	return -1;
}

static void codec2str(const int codec, char *str)
{
	int i;

	for (i = 0; i < MPI_VENC_TYPE_NUM; i++) {
		if (codec == codec_dict[i].val) {
			strcpy(str, codec_dict[i].key);
			return;
		}
	}

	strcpy(str, "Error!");
}

static int str2profile(const char *str)
{
	int i;

	for (i = 0; i < MPI_PRFL_NUM; i++) {
		if (strcmp(profile_dict[i].key, str) == 0) {
			return profile_dict[i].val;
		}
	}

	return -1;
}

static void profile2str(const int profile, char *str)
{
	int i;

	for (i = 0; i < MPI_PRFL_NUM; i++) {
		if (profile == profile_dict[i].val) {
			strcpy(str, profile_dict[i].key);
			return;
		}
	}

	strcpy(str, "Error!");
}

static int str2rcmode(const char *str)
{
	int i;

	for (i = 0; i < MPI_RC_MODE_NUM; i++) {
		if (strcmp(rc_mode_dict[i].key, str) == 0) {
			return rc_mode_dict[i].val;
		}
	}

	return -1;
}

static void rcmode2str(const int rc_mode, char *str)
{
	int i;

	for (i = 0; i < MPI_RC_MODE_NUM; i++) {
		if (rc_mode == rc_mode_dict[i].val) {
			strcpy(str, rc_mode_dict[i].key);
			return;
		}
	}

	strcpy(str, "Error!");
}

static int reset_rc_h2645_vbr(MPI_MCVC_VBR_PARAM_S *t_vbr, MPI_VENC_ATTR_EX_S *t_ex, const ParseData *p_data)
{
	int ret = 0;

	if ((p_data->max_bit_rate != -1) && (p_data->max_qp != -1) && (p_data->quality_level_index != -1) &&
	    (p_data->fluc_level != -1) && (p_data->regression_speed != -1) && (p_data->scene_smooth != -1) &&
	    (p_data->i_qp_offset != INT_MIN) && (p_data->motion_tolerance_level != -1) &&
	    (p_data->motion_tolerance_qp != -1) && (p_data->obs != -1) && (p_data->obs_off_period != -1)) {

		t_vbr->max_bit_rate = (uint32_t)p_data->max_bit_rate;
		t_vbr->max_qp = (uint32_t)p_data->max_qp;
		t_vbr->quality_level_index = (uint32_t)p_data->quality_level_index;
		t_vbr->fluc_level = (uint32_t)p_data->fluc_level;
		t_vbr->regression_speed = (uint32_t)p_data->regression_speed;
		t_vbr->scene_smooth = (uint32_t)p_data->scene_smooth;
		t_vbr->i_continue_weight = 0;
		t_vbr->i_qp_offset = p_data->i_qp_offset;
		t_vbr->motion_tolerance_level = (uint32_t)p_data->motion_tolerance_level;
		t_vbr->motion_tolerance_qp = (uint32_t)p_data->motion_tolerance_qp;

		t_ex->obs = (uint32_t)p_data->obs;
		t_ex->obs_off_period = (uint32_t)p_data->obs_off_period;

		ret = 0;
	} else {
		printf("VBR attributes 'max_bit_rate, max_qp, quality_level_index, fluc_level, regression_speed, scene_smooth, "
		       "i_qp_offset, motion_tolerance_level & motion_tolerance_qp' and Extended attributes 'obs & "
		       "obs_off_period' are necessary while changing to H264/H265 VBR.\n");
		ret = EINVAL;
	}

	return ret;
}

static void set_rc_h2645_vbr_attr(MPI_MCVC_VBR_PARAM_S *t_vbr, MPI_VENC_ATTR_EX_S *t_ex, const ParseData *p_data)
{
	t_vbr->max_bit_rate = (p_data->max_bit_rate != -1) ? (uint32_t)p_data->max_bit_rate : t_vbr->max_bit_rate;
	t_vbr->max_qp = (p_data->max_qp != -1) ? (uint32_t)p_data->max_qp : t_vbr->max_qp;
	t_vbr->quality_level_index = (p_data->quality_level_index != -1)
	                             ? (uint32_t)p_data->quality_level_index : t_vbr->quality_level_index;
	t_vbr->fluc_level = (p_data->fluc_level != -1) ? (uint32_t)p_data->fluc_level : t_vbr->fluc_level;
	t_vbr->regression_speed = (p_data->regression_speed != -1)
	                          ? (uint32_t)p_data->regression_speed : t_vbr->regression_speed;
	t_vbr->scene_smooth = (p_data->scene_smooth != -1) ? (uint32_t)p_data->scene_smooth : t_vbr->scene_smooth;
	t_vbr->i_continue_weight = 0;
	t_vbr->i_qp_offset = (p_data->i_qp_offset != INT_MIN) ? p_data->i_qp_offset : t_vbr->i_qp_offset;
	t_vbr->motion_tolerance_level = (p_data->motion_tolerance_level != -1)
	                                ? (uint32_t)p_data->motion_tolerance_level : t_vbr->motion_tolerance_level;
	t_vbr->motion_tolerance_qp = (p_data->motion_tolerance_qp != -1)
	                             ? (uint32_t)p_data->motion_tolerance_qp : t_vbr->motion_tolerance_qp;

	t_ex->obs = (p_data->obs != -1) ? (uint32_t)p_data->obs : t_ex->obs;
	t_ex->obs_off_period = (p_data->obs_off_period != -1) ? (uint32_t)p_data->obs_off_period : t_ex->obs_off_period;
}

static int reset_rc_h2645_cbr(MPI_MCVC_CBR_PARAM_S *t_cbr, MPI_VENC_ATTR_EX_S *t_ex, const ParseData *p_data)
{
	int ret = 0;

	if ((p_data->bit_rate != -1) && (p_data->max_qp != -1) && (p_data->min_qp != -1) && (p_data->fluc_level != -1) &&
	    (p_data->regression_speed != -1) && (p_data->scene_smooth != -1) && (p_data->i_qp_offset != INT_MIN) &&
	    (p_data->motion_tolerance_level != -1) && (p_data->motion_tolerance_qp != -1) && (p_data->obs != -1) &&
	    (p_data->obs_off_period != -1)) {

		t_cbr->bit_rate = (uint32_t)p_data->bit_rate;
		t_cbr->max_qp = (uint32_t)p_data->max_qp;
		t_cbr->min_qp = (uint32_t)p_data->min_qp;
		t_cbr->fluc_level = (uint32_t)p_data->fluc_level;
		t_cbr->regression_speed = (uint32_t)p_data->regression_speed;
		t_cbr->scene_smooth = (uint32_t)p_data->scene_smooth;
		t_cbr->i_continue_weight = 0;
		t_cbr->i_qp_offset = p_data->i_qp_offset;
		t_cbr->motion_tolerance_level = (uint32_t)p_data->motion_tolerance_level;
		t_cbr->motion_tolerance_qp = (uint32_t)p_data->motion_tolerance_qp;

		t_ex->obs = (uint32_t)p_data->obs;
		t_ex->obs_off_period = (uint32_t)p_data->obs_off_period;

		ret = 0;
	} else {
		printf("CBR attributes 'bit_rate, max_qp, min_qp, fluc_level, regression_speed, scene_smooth, i_qp_offset, "
		       "motion_tolerance_level & motion_tolerance_qp' and Extended attributes 'obs & obs_off_period' are "
		       "necessary while changing to H264/H265 CBR.\n");
		ret = EINVAL;
	}

	return ret;
}

static void set_rc_h2645_cbr_attr(MPI_MCVC_CBR_PARAM_S *t_cbr, MPI_VENC_ATTR_EX_S *t_ex, const ParseData *p_data)
{
	t_cbr->bit_rate = (p_data->bit_rate != -1) ? (uint32_t)p_data->bit_rate : t_cbr->bit_rate;
	t_cbr->max_qp = (p_data->max_qp != -1) ? (uint32_t)p_data->max_qp : t_cbr->max_qp;
	t_cbr->min_qp = (p_data->min_qp != -1) ? (uint32_t)p_data->min_qp : t_cbr->min_qp;
	t_cbr->fluc_level = (p_data->fluc_level != -1) ? (uint32_t)p_data->fluc_level : t_cbr->fluc_level;
	t_cbr->regression_speed = (p_data->regression_speed != -1)
	                          ? (uint32_t)p_data->regression_speed : t_cbr->regression_speed;
	t_cbr->scene_smooth = (p_data->scene_smooth != -1) ? (uint32_t)p_data->scene_smooth : t_cbr->scene_smooth;
	t_cbr->i_continue_weight = 0;
	t_cbr->i_qp_offset = (p_data->i_qp_offset != INT_MIN) ? p_data->i_qp_offset : t_cbr->i_qp_offset;
	t_cbr->motion_tolerance_level = (p_data->motion_tolerance_level != -1)
	                                ? (uint32_t)p_data->motion_tolerance_level : t_cbr->motion_tolerance_level;
	t_cbr->motion_tolerance_qp = (p_data->motion_tolerance_qp != -1)
	                             ? (uint32_t)p_data->motion_tolerance_qp : t_cbr->motion_tolerance_qp;

	t_ex->obs = (p_data->obs != -1) ? (uint32_t)p_data->obs : t_ex->obs;
	t_ex->obs_off_period = (p_data->obs_off_period != -1) ? (uint32_t)p_data->obs_off_period : t_ex->obs_off_period;
}

static int reset_rc_h2645_sbr(MPI_MCVC_SBR_PARAM_S *t_sbr, MPI_VENC_ATTR_EX_S *t_ex, const ParseData *p_data)
{
	int ret = 0;

	if ((p_data->bit_rate != -1) && (p_data->max_qp != -1) && (p_data->min_qp != -1) && (p_data->fluc_level != -1) &&
	    (p_data->regression_speed != -1) && (p_data->scene_smooth != -1) && (p_data->i_qp_offset != INT_MIN) &&
	    (p_data->motion_tolerance_level != -1) && (p_data->motion_tolerance_qp != -1) &&
	    (p_data->adjust_br_thres_pc != -1) && (p_data->adjust_step_times != -1) && (p_data->converge_frame != -1) &&
	    (p_data->obs != -1) && (p_data->obs_off_period != -1)) {

		t_sbr->bit_rate = (uint32_t)p_data->bit_rate;
		t_sbr->max_qp = (uint32_t)p_data->max_qp;
		t_sbr->min_qp = (uint32_t)p_data->min_qp;
		t_sbr->fluc_level = (uint32_t)p_data->fluc_level;
		t_sbr->regression_speed = (uint32_t)p_data->regression_speed;
		t_sbr->scene_smooth = (uint32_t)p_data->scene_smooth;
		t_sbr->i_continue_weight = 0;
		t_sbr->i_qp_offset = p_data->i_qp_offset;
		t_sbr->motion_tolerance_level = (uint32_t)p_data->motion_tolerance_level;
		t_sbr->motion_tolerance_qp = (uint32_t)p_data->motion_tolerance_qp;
		t_sbr->adjust_br_thres_pc = (uint32_t)p_data->adjust_br_thres_pc;
		t_sbr->adjust_step_times = (uint32_t)p_data->adjust_step_times;
		t_sbr->converge_frame = (uint32_t)p_data->converge_frame;

		t_ex->obs = (uint32_t)p_data->obs;
		t_ex->obs_off_period = (uint32_t)p_data->obs_off_period;

		ret = 0;
	} else {
		printf("SBR attributes 'bit_rate, max_qp, min_qp, fluc_level, regression_speed, scene_smooth, i_qp_offset, "
		       "motion_tolerance_level & motion_tolerance_qp, adjust_br_thres_pc, adjust_step_times & converge_frame' "
		       "and Extended attributes 'obs & obs_off_period' are necessary while changing to H264/H265 SBR.\n");
		ret = EINVAL;
	}

	return ret;
}

static void set_rc_h2645_sbr_attr(MPI_MCVC_SBR_PARAM_S *t_sbr, MPI_VENC_ATTR_EX_S *t_ex, const ParseData *p_data)
{
	t_sbr->bit_rate = (p_data->bit_rate != -1) ? (uint32_t)p_data->bit_rate : t_sbr->bit_rate;
	t_sbr->max_qp = (p_data->max_qp != -1) ? (uint32_t)p_data->max_qp : t_sbr->max_qp;
	t_sbr->min_qp = (p_data->min_qp != -1) ? (uint32_t)p_data->min_qp : t_sbr->min_qp;
	t_sbr->fluc_level = (p_data->fluc_level != -1) ? (uint32_t)p_data->fluc_level : t_sbr->fluc_level;
	t_sbr->regression_speed = (p_data->regression_speed != -1)
	                          ? (uint32_t)p_data->regression_speed : t_sbr->regression_speed;
	t_sbr->scene_smooth = (p_data->scene_smooth != -1) ? (uint32_t)p_data->scene_smooth : t_sbr->scene_smooth;
	t_sbr->i_continue_weight = 0;
	t_sbr->i_qp_offset = (p_data->i_qp_offset != INT_MIN) ? p_data->i_qp_offset : t_sbr->i_qp_offset;
	t_sbr->motion_tolerance_level = (p_data->motion_tolerance_level != -1)
	                                ? (uint32_t)p_data->motion_tolerance_level : t_sbr->motion_tolerance_level;
	t_sbr->motion_tolerance_qp = (p_data->motion_tolerance_qp != -1)
	                             ? (uint32_t)p_data->motion_tolerance_qp : t_sbr->motion_tolerance_qp;
	t_sbr->adjust_br_thres_pc = (p_data->adjust_br_thres_pc != -1)
	                            ? (uint32_t)p_data->adjust_br_thres_pc : t_sbr->adjust_br_thres_pc;
	t_sbr->adjust_step_times = (p_data->adjust_step_times != -1)
	                           ? (uint32_t)p_data->adjust_step_times : t_sbr->adjust_step_times;
	t_sbr->converge_frame = (p_data->converge_frame != -1) ? (uint32_t)p_data->converge_frame : t_sbr->converge_frame;

	t_ex->obs = (p_data->obs != -1) ? (uint32_t)p_data->obs : t_ex->obs;
	t_ex->obs_off_period = (p_data->obs_off_period != -1) ? (uint32_t)p_data->obs_off_period : t_ex->obs_off_period;
}

static int reset_rc_h2645_cqp(MPI_MCVC_CQP_PARAM_S *t_cqp, const ParseData *p_data)
{
	int ret = 0;

	if ((p_data->i_frame_qp != -1) && (p_data->p_frame_qp != -1)) {
		t_cqp->i_frame_qp = (uint32_t)p_data->i_frame_qp;
		t_cqp->p_frame_qp = (uint32_t)p_data->p_frame_qp;

		ret = 0;
	} else {
		printf("CQP attributes 'i_frame_qp & p_frame_qp' are necessary while changing to H264/H265 CQP.\n");
		ret = EINVAL;
	}

	return ret;
}

static void set_rc_h2645_cqp_attr(MPI_MCVC_CQP_PARAM_S *t_cqp, const ParseData *p_data)
{
	t_cqp->i_frame_qp = (p_data->i_frame_qp != -1) ? (uint32_t)p_data->i_frame_qp : t_cqp->i_frame_qp;
	t_cqp->p_frame_qp = (p_data->p_frame_qp != -1) ? (uint32_t)p_data->p_frame_qp : t_cqp->p_frame_qp;
}

static int reset_rc_mjpeg(MPI_VC_RC_ATTR_S *t_mrc, const ParseData *p_data)
{
	int ret = 0;

	if ((p_data->max_bit_rate != -1) && (p_data->quality_level_index != -1) && (p_data->fluc_level != -1) &&
	    (p_data->bit_rate != -1) && (p_data->max_q_factor != -1) && (p_data->min_q_factor != -1) &&
        (p_data->motion_tolerance_level != -1) && (p_data->motion_tolerance_qfactor != -1) &&
	    (p_data->adjust_br_thres_pc != -1) && (p_data->adjust_step_times != -1) && (p_data->converge_frame != -1) &&
	    (p_data->q_factor != -1)) {

		t_mrc->max_bit_rate = (uint32_t)p_data->max_bit_rate;
		t_mrc->quality_level_index = (uint32_t)p_data->quality_level_index;
		t_mrc->fluc_level = (uint32_t)p_data->fluc_level;
		t_mrc->bit_rate = (uint32_t)p_data->bit_rate;
		t_mrc->max_q_factor = (uint32_t)p_data->max_q_factor;
		t_mrc->min_q_factor = (uint32_t)p_data->min_q_factor;
		t_mrc->motion_tolerance_level = (uint32_t)p_data->motion_tolerance_level;
		t_mrc->motion_tolerance_qfactor = (uint32_t)p_data->motion_tolerance_qfactor;
		t_mrc->adjust_br_thres_pc = (uint32_t)p_data->adjust_br_thres_pc;
		t_mrc->adjust_step_times = (uint32_t)p_data->adjust_step_times;
		t_mrc->converge_frame = (uint32_t)p_data->converge_frame;
		t_mrc->q_factor = (uint32_t)p_data->q_factor;

		ret = 0;
	} else {
		printf("MJPEG attributes 'max_bit_rate, quality_level_index, fluc_level, bit_rate, max_q_factor, min_q_factor, "
		       "motion_tolerance_level, motion_tolerance_qfactor, adjust_br_thres_pc, adjust_step_times, "
		       "converge_frame & q_factor' are necessary while changing to MJPEG VBR/CBR/SBR/CQP.\n");
		ret = EINVAL;
	}

	return ret;
}

static int set_rc_mjpeg_attr(MPI_VC_RC_ATTR_S *t_mrc, const ParseData *p_data)
{
	t_mrc->max_bit_rate = (p_data->max_bit_rate != -1) ? (uint32_t)p_data->max_bit_rate : t_mrc->max_bit_rate;
	t_mrc->quality_level_index = (p_data->quality_level_index != -1)
	                             ? (uint32_t)p_data->quality_level_index : t_mrc->quality_level_index;
	t_mrc->fluc_level = (p_data->fluc_level != -1) ? (uint32_t)p_data->fluc_level : t_mrc->fluc_level;
	t_mrc->bit_rate = (p_data->bit_rate != -1) ? (uint32_t)p_data->bit_rate : t_mrc->bit_rate;
	t_mrc->max_q_factor = (p_data->max_q_factor != -1) ? (uint32_t)p_data->max_q_factor : t_mrc->max_q_factor;
	t_mrc->min_q_factor = (p_data->min_q_factor != -1) ? (uint32_t)p_data->min_q_factor : t_mrc->min_q_factor;
	t_mrc->motion_tolerance_level = (p_data->motion_tolerance_level != -1)
	                                ? (uint32_t)p_data->motion_tolerance_level : t_mrc->motion_tolerance_level;
	t_mrc->motion_tolerance_qfactor = (p_data->motion_tolerance_qfactor != -1)
	                                  ? (uint32_t)p_data->motion_tolerance_qfactor : t_mrc->motion_tolerance_qfactor;
	t_mrc->adjust_br_thres_pc = (p_data->adjust_br_thres_pc != -1)
	                            ? (uint32_t)p_data->adjust_br_thres_pc : t_mrc->adjust_br_thres_pc;
	t_mrc->adjust_step_times = (p_data->adjust_step_times != -1)
	                           ? (uint32_t)p_data->adjust_step_times : t_mrc->adjust_step_times;
	t_mrc->converge_frame = (p_data->converge_frame != -1)
	                        ? (uint32_t)p_data->converge_frame : t_mrc->converge_frame;
	t_mrc->q_factor = (p_data->q_factor != -1) ? (uint32_t)p_data->q_factor : t_mrc->q_factor;

	return 0;
}

// =========================================================================== //
//  VENC: CODEC + RATE CONTROL
// =========================================================================== //

static int GET(VencAttr)(CMD_DATA_S *cmd)
{
	int ret;
	CmdVencAttr *data = (CmdVencAttr *)cmd->data;

	ret = MPI_ENC_getVencAttr(cmd->echn_idx, &data->venc_attr);
	ret = MPI_ENC_getVencAttrEx(cmd->echn_idx, &data->venc_ex);

	return ret;
}

static int SET(VencAttr)(const CMD_DATA_S *cmd)
{
	CMD_DATA_S cur_cmd = { 0 };
	CmdVencAttr cur_data;
	CmdVencAttr *t_data = (CmdVencAttr *)cmd->data;
	const MPI_VENC_ATTR_S *t_vattr = &t_data->venc_attr;
	const MPI_VENC_ATTR_EX_S *t_vex = &t_data->venc_ex;
	MPI_VENC_ATTR_S *cur_vattr = &cur_data.venc_attr;
	MPI_ENC_BIND_INFO_S bind_info;
	MPI_ECHN echn_idx;
	int ret;
	int idx;

	cur_cmd.data = &cur_data;
	ret = GET(VencAttr)(&cur_cmd);
	if (ret) {
		printf("Error while getting VENC attr!\n");
		return EINVAL;
	}

	// Reset specific encoder (stop and rerun) by enc_idx if the codec is changed.
	if (t_vattr->type != cur_vattr->type) {
		// Stop the specific running encoder.
		memset(&bind_info, 0, sizeof(MPI_ENC_BIND_INFO_S));
		idx = cmd->echn_idx.chn;
		echn_idx = MPI_ENC_CHN(idx);
		t_data->enable_echn[idx] = false;
		ret = MPI_ENC_stopChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_stopChn(%d) failed. err: %d (%s).\n", echn_idx.chn, ret, strerror(-ret));
			return EINVAL;
		}
		t_data->enable_echn[idx] = true;

		// Configure target encoder.
		ret = MPI_ENC_setVencAttr(cmd->echn_idx, t_vattr);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_setVencAttr(%d) failed. err: %d (%s).\n", cmd->echn_idx.chn, ret, strerror(-ret));
			return ret;
		}

		ret = MPI_ENC_setVencAttrEx(cmd->echn_idx, t_vex);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_setVencAttrEx(%d) failed. err: %d (%s).\n", cmd->echn_idx.chn, ret, strerror(-ret));
			return ret;
		}

		// Restart the specific running encoder.
		bind_info.idx = MPI_VIDEO_CHN(0, idx);
		ret = MPI_ENC_bindToVideoChn(echn_idx, &bind_info);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_bindToVideoChn(%d, %d) failed. err: %d (%s).\n", echn_idx.chn, bind_info.idx.chn, ret,
			       strerror(-ret));
			return ret;
		}

		ret = MPI_ENC_startChn(echn_idx);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_startChn(%d) failed. err: %d (%s).\n", echn_idx.chn, ret, strerror(-ret));
			return ret;
		}
	} else {
		// Reset RC only.
		ret = MPI_ENC_setVencAttr(cmd->echn_idx, t_vattr);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_setVencAttr(%d) failed. err: %d (%s).\n", cmd->echn_idx.chn, ret, strerror(-ret));
			return ret;
		}

		ret = MPI_ENC_setVencAttrEx(cmd->echn_idx, t_vex);
		if (ret != MPI_SUCCESS) {
			printf("MPI_ENC_setVencAttrEx(%d) failed. err: %d (%s).\n", cmd->echn_idx.chn, ret, strerror(-ret));
			return ret;
		}
	}

	return 0;
}

static void ARGS(VencAttr)()
{
	printf("--vhelp      Hint for cmdsender VENC part.\n");
	printf("--vargs      See all the attributes and their value range of the encoder.\n");
	printf("--get_vattr  See the current value of the attributes (please also provide enc_idx).\n");
	printf("-----------------------------------------------------------------------\n");
	printf("VENC attr:\n\n");
	printf("--codec_type=[H264, H265, MJPEG, JPEG]\n");
	printf("--profile=[BASELINE, MAIN, HIGH]\n");
	printf("-----------------------------------------------------------------------\n");
	printf("RC basic attr:\n\n");
	printf("--rc_mode=[VBR, CBR, SBR, CQP]\n");
	printf("--bitrate=[128 ~ 16384 (H264/H265, CBR/SBR); 8192 ~ 51200 (MJPEG/JPEG, CBR/SBR)]\n");
	printf("--enc_fps=[1, 30] (please reset video channel's FPS)\n");
	printf("--gop_size=[1, 240] (H264/H265)\n");
	printf("--fluc_level=[0, 4] (H264/H265/MJPEG, VBR/CBR/SBR)\n");
	printf("--scene_smooth=[0, 9] (H264/H265, VBR/CBR/SBR)\n");
	printf("--regression_speed=[0, 9] (H264/H265, VBR/CBR/SBR)\n");
	printf("--i_continue_weight=[0] (H264/H265, VBR/CBR/SBR)\n");
	printf("--i_qp_offset=[-5, 0] (H264/H265, VBR/CBR/SBR)\n");
	printf("--motion_tolerance_level=[0, 100] (H264/H265/MJPEG, VBR/CBR/SBR)\n");
	printf("--motion_tolerance_qp=[0, 51] (H264/H265, VBR/CBR/SBR)\n");
	printf("--motion_tolerance_qfactor=[0, 100] (MJPEG, VBR/CBR/SBR)\n");
	printf("--min_qp=[0, 51] (H264/H265, CBR/SBR)\n");
	printf("--max_qp=[0, 51] (H264/H265, VBR/CBR/SBR)\n");
	printf("--min_q_factor=[0, 100] (MJPEG, CBR/SBR)\n");
	printf("--max_q_factor=[0, 100] (MJPEG, VBR/CBR/SBR)\n");
	printf("--max_bit_rate=[128 ~ 16384 (H264/H265, VBR); 10240 ~ 32768 (MJPEG/JPEG, VBR)]\n");
	printf("--quality_level_index=[0, 9] (H264/H265/MJPEG, VBR)\n");
	printf("--adjust_br_thres_pc=[50, 150] (H264/H265/MJPEG, SBR)\n");
	printf("--adjust_step_times=[1, 50] (H264/H265/MJPEG, SBR)\n");
	printf("--converge_frame=[1, 240] (H264/H265/MJPEG, SBR)\n");
	printf("--i_frame_qp=[0, 51] (H264/H265, CQP)\n");
	printf("--p_frame_qp=[0, 51] (H264/H265, CQP)\n");
	printf("--q_factor=[0, 100] (MJPEG, CQP)\n");
	printf("--obs=[0, 1] (H264/H265)\n");
	printf("--obs_off_period=[1, GOP] (H264/H265, OBS)\n");
	printf("-----------------------------------------------------------------------\n");
	printf("Short argument name:\n\n");
	printf("--vhelp -> --vh\n");
	printf("--vargs -> --va\n");
	printf("--get_vattr -> --gva\n");
	printf("--enc_idx -> --eid\n");
	printf("--codec_type -> --codec\n");
	printf("--profile -> --prof\n");
	printf("--rc_mode -> --rcm\n");
	printf("--bit_rate -> --br\n");
	printf("--enc_fps -> --fps\n");
	printf("--gop_size -> --gop\n");
	printf("--fluc_level -> --fluc\n");
	printf("--scene_smooth -> --ss\n");
	printf("--regression_speed -> --rgs\n");
	printf("--i_continue_weight -> --icw\n");
	printf("--i_qp_offset -> --iqo\n");
	printf("--motion_tolerance_level -> --mtllv\n");
	printf("--motion_tolerance_qp -> --mtlqp\n");
	printf("--motion_tolerance_qfactor -> --mtlqf\n");
	printf("--min_qp -> --minqp\n");
	printf("--max_qp -> --maxqp\n");
	printf("--min_q_factor -> --minqf\n");
	printf("--max_q_factor -> --maxqf\n");
	printf("--max_bit_rate -> --mbr\n");
	printf("--quality_level_index -> --qli\n");
	printf("--adjust_br_thres_pc -> --adjbr\n");
	printf("--adjust_step_times -> --adjst\n");
	printf("--converge_frame -> --conv\n");
	printf("--i_frame_qp -> --icqp\n");
	printf("--p_frame_qp -> --pcqp\n");
	printf("--q_factor -> --cqf\n");
	printf("--obs -> --obs\n");
	printf("--obs_off_period -> --obsop\n\n");
}

static void HELP(VencAttr)(const char *str)
{
	CMD_PRINT_HELP(str, "'--venc -enc_idx=[0, %d] {VENC ATTR} {RC ATTR}'", "Set encoder attributes, see '--venc'");
	CMD_PRINT_HELP(str, "'--venc'", "Show encoder attributes help()");
}

static void SHOW(VencAttr)(const CMD_DATA_S *cmd)
{
	CmdVencAttr *data = (CmdVencAttr *)cmd->data;
	MPI_VENC_ATTR_S *venc_attr = &data->venc_attr;
	MPI_VENC_ATTR_EX_S *venc_ex = &data->venc_ex;
	MPI_MCVC_RC_ATTR_S *mcvc;
	MPI_VC_RC_ATTR_S *vc;
	char str[10];
	bool is_h2645 = false;
	bool is_mjpeg = false;

	printf("Encoder '%u':\n", cmd->echn_idx.chn);
	codec2str(venc_attr->type, str);
	printf("codec = %s\n", str);

	switch (venc_attr->type) {
	case MPI_VENC_TYPE_H264:
		profile2str(venc_attr->h264.profile, str);
		printf("profile = %s\n", str);
		mcvc = &venc_attr->h264.rc;
		is_h2645 = true;
		break;
	case MPI_VENC_TYPE_H265:
		profile2str(venc_attr->h265.profile, str);
		printf("profile = %s\n", str);
		mcvc = &venc_attr->h265.rc;
		is_h2645 = true;
		break;
	case MPI_VENC_TYPE_MJPEG:
		vc = &venc_attr->mjpeg.rc;
		rcmode2str(vc->mode, str);
		printf("rc_mode = %s\n", str);
		printf("enc_fps = %u\n", vc->frm_rate_o);
		is_mjpeg = true;
		break;
	case MPI_VENC_TYPE_JPEG:
	default:
		printf("Unsupported Codec '%d'!\n", venc_attr->type);
		break;
	}

	if (is_h2645) {
		rcmode2str(mcvc->mode, str);
		printf("rc_mode = %s\n", str);
		printf("enc_fps = %u\n", mcvc->frm_rate_o);
		printf("gop_size = %u\n", mcvc->gop);
		printf("max_frame_size = %u\n", mcvc->max_frame_size);

		switch (mcvc->mode) {
		case MPI_RC_MODE_VBR:
			printf("max_bit_rate = %u\n", mcvc->vbr.max_bit_rate);
			printf("quality_level_index = %u\n", mcvc->vbr.quality_level_index);
			printf("max_qp = %u\n", mcvc->vbr.max_qp);
			printf("fluc_level = %u\n", mcvc->vbr.fluc_level);
			printf("regression_speed = %u\n", mcvc->vbr.regression_speed);
			printf("scene_smooth = %u\n", mcvc->vbr.scene_smooth);
			printf("i_continue_weight = %u\n", mcvc->vbr.i_continue_weight);
			printf("i_qp_offset = %d\n", mcvc->vbr.i_qp_offset);
			printf("motion_tolerance_level = %u\n", mcvc->vbr.motion_tolerance_level);
			printf("motion_tolerance_qp = %u\n", mcvc->vbr.motion_tolerance_qp);
			printf("obs = %d\n", venc_ex->obs);
			printf("obs_off_period = %u\n", venc_ex->obs_off_period);
			break;
		case MPI_RC_MODE_CBR:
			printf("bit_rate = %u\n", mcvc->cbr.bit_rate);
			printf("min_qp = %u\n", mcvc->cbr.min_qp);
			printf("max_qp = %u\n", mcvc->cbr.max_qp);
			printf("fluc_level = %u\n", mcvc->cbr.fluc_level);
			printf("regression_speed = %u\n", mcvc->cbr.regression_speed);
			printf("scene_smooth = %u\n", mcvc->cbr.scene_smooth);
			printf("i_continue_weight = %u\n", mcvc->cbr.i_continue_weight);
			printf("i_qp_offset = %d\n", mcvc->cbr.i_qp_offset);
			printf("motion_tolerance_level = %u\n", mcvc->cbr.motion_tolerance_level);
			printf("motion_tolerance_qp = %u\n", mcvc->cbr.motion_tolerance_qp);
			printf("obs = %d\n", venc_ex->obs);
			printf("obs_off_period = %u\n", venc_ex->obs_off_period);
			break;
		case MPI_RC_MODE_SBR:
			printf("bit_rate = %u\n", mcvc->sbr.bit_rate);
			printf("min_qp = %u\n", mcvc->sbr.min_qp);
			printf("max_qp = %u\n", mcvc->sbr.max_qp);
			printf("fluc_level = %u\n", mcvc->sbr.fluc_level);
			printf("regression_speed = %u\n", mcvc->sbr.regression_speed);
			printf("scene_smooth = %u\n", mcvc->sbr.scene_smooth);
			printf("i_continue_weight = %u\n", mcvc->sbr.i_continue_weight);
			printf("i_qp_offset = %d\n", mcvc->sbr.i_qp_offset);
			printf("motion_tolerance_level = %u\n", mcvc->sbr.motion_tolerance_level);
			printf("motion_tolerance_qp = %u\n", mcvc->sbr.motion_tolerance_qp);
			printf("adjust_br_thres_pc = %u\n", mcvc->sbr.adjust_br_thres_pc);
			printf("adjust_step_times = %u\n", mcvc->sbr.adjust_step_times);
			printf("converge_frame = %u\n", mcvc->sbr.converge_frame);
			printf("obs = %u\n", venc_ex->obs);
			printf("obs_off_period = %u\n", venc_ex->obs_off_period);
			break;
		case MPI_RC_MODE_CQP:
			printf("i_frame_qp = %u\n", mcvc->cqp.i_frame_qp);
			printf("p_frame_qp = %u\n", mcvc->cqp.p_frame_qp);
			break;
		default:
			printf("Unsupported RC mode '%d'!\n", mcvc->mode);
			break;
		}
	}

	if (is_mjpeg) {
		switch (vc->mode) {
		case MPI_RC_MODE_VBR:
			printf("max_bit_rate = %u\n", vc->max_bit_rate);
			printf("quality_level_index = %u\n", vc->quality_level_index);
			printf("min_q_factor = %u\n", vc->min_q_factor);
			printf("fluc_level = %u\n", vc->fluc_level);
			printf("max_frame_size = %u\n", vc->max_frame_size);
			printf("motion_tolerance_level = %u\n", vc->motion_tolerance_level);
			printf("motion_tolerance_qfactor = %u\n", vc->motion_tolerance_qfactor);
			break;
		case MPI_RC_MODE_CBR:
			printf("bit_rate = %u\n", vc->bit_rate);
			printf("min_q_factor = %u\n", vc->min_q_factor);
			printf("max_q_factor = %u\n", vc->max_q_factor);
			printf("fluc_level = %u\n", vc->fluc_level);
			printf("max_frame_size = %u\n", vc->max_frame_size);
			printf("motion_tolerance_level = %u\n", vc->motion_tolerance_level);
			printf("motion_tolerance_qfactor = %u\n", vc->motion_tolerance_qfactor);
			break;
		case MPI_RC_MODE_SBR:
			printf("bit_rate = %u\n", vc->bit_rate);
			printf("min_q_factor = %u\n", vc->min_q_factor);
			printf("max_q_factor = %u\n", vc->max_q_factor);
			printf("fluc_level = %u\n", vc->fluc_level);
			printf("max_frame_size = %u\n", vc->max_frame_size);
			printf("adjust_br_thres_pc = %u\n", vc->adjust_br_thres_pc);
			printf("adjust_step_times = %u\n", vc->adjust_step_times);
			printf("converge_frame = %u\n", vc->converge_frame);
			printf("motion_tolerance_level = %u\n", vc->motion_tolerance_level);
			printf("motion_tolerance_qfactor = %u\n", vc->motion_tolerance_qfactor);
			break;
		case MPI_RC_MODE_CQP:
			printf("q_factor = %u\n", vc->q_factor);
			break;
		default:
			printf("Unsupported RC mode '%d'!\n", vc->mode);
			break;
		}
	}
}

static int PARSE(VencAttr)(int argc, char **argv, CMD_DATA_S *cmd)
{
	CMD_DATA_S cur_cmd = { 0 };
	CmdVencAttr cur_data;
	CmdVencAttr *t_data = (CmdVencAttr *)cmd->data;
	MPI_VENC_ATTR_S *t_attr = &t_data->venc_attr;
	MPI_VENC_ATTR_S *cur_attr;
	MPI_VENC_ATTR_EX_S *t_ex = &t_data->venc_ex;
	MPI_VENC_ATTR_EX_S *cur_ex;
	MPI_RC_MODE_E cur_rc_mode = -1;
	ParseData p_data;
	int ret;
	bool is_get_attr = false;
	bool is_codec_change = false;

	cmd->action = CMD_ACTION_NON;
	memset(&p_data, -1, sizeof(ParseData));
	p_data.i_qp_offset = INT_MIN;
	while ((ret = getopt_long(argc, argv, ":", opts, NULL)) != -1) {
		switch (ret) {
		case OV_HELP:
			HELP(VencAttr)(optarg);
			cmd->action = CMD_ACTION_NON;
			return 0;
			break;
		case OV_ARGS:
			//ARGS(VencAttr)();
			cmd->action = CMD_ACTION_ARGS;
			return 0;
			break;
		case OV_GET_ATTR:
			is_get_attr = true;
			break;
		case OV_ENC_IDX:
			p_data.enc_idx = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_CODEC:
			p_data.codec_type = (optarg == NULL) ? -1 : str2codec(optarg);
			break;
		case OV_PROFILE:
			p_data.profile = (optarg == NULL) ? -1 : str2profile(optarg);
			break;
		case OV_RCMODE:
			p_data.rc_mode = (optarg == NULL) ? -1 : str2rcmode(optarg);
			break;
		case OV_BR:
			p_data.bit_rate = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_FPS:
			p_data.frm_rate_o = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_GOP:
			p_data.gop_size = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_FLUC:
			p_data.fluc_level = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_SCENE:
			p_data.scene_smooth = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_REGRESSION:
			p_data.regression_speed = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_I_CONT:
			p_data.i_continue_weight = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_IQP_OFFSET:
			p_data.i_qp_offset = (optarg == NULL) ? INT_MIN : atoi(optarg);
			break;
		case OV_MIN_QP:
			p_data.min_qp = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MAX_QP:
			p_data.max_qp = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MIN_QFACTOR:
			p_data.min_q_factor = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MAX_QFACTOR:
			p_data.max_q_factor = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MTL_LV:
			p_data.motion_tolerance_level = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MTL_QP:
			p_data.motion_tolerance_qp = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MTL_QFACTOR:
			p_data.motion_tolerance_qfactor = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_MAX_BR:
			p_data.max_bit_rate = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_QUALITY:
			p_data.quality_level_index = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_ADJ_THRES:
			p_data.adjust_br_thres_pc = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_ADJ_TIME:
			p_data.adjust_step_times = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_CONVERGE:
			p_data.converge_frame = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_C_IQP:
			p_data.i_frame_qp = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_C_PQP:
			p_data.p_frame_qp = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_C_QFACTOR:
			p_data.q_factor = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_OBS:
			p_data.obs = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case OV_OBS_OFF_PERIOD:
			p_data.obs_off_period = (optarg == NULL) ? -1 : atoi(optarg);
			break;
		case '?':
		default:
			printf("Unkown arguments!  Please enter 'cmdsender --venc --vargs' (or 'cmdsender --venc --va') ");
			printf("and check the agrument list.\n");
			cmd->action = CMD_ACTION_NON;
			return EINVAL;
			break;
		}
	}

	if (p_data.enc_idx == -1) {
		// Except for help and args, it needs enc_idx to handle other attributes.
		printf("Please provide an encoder channel index (0~%d)!\n", MPI_MAX_ENC_CHN_NUM - 1);
		printf("Or enter 'cmdsender --venc --vhelp' (or 'cmdsender --venc --vh') to see help message.\n");
		cmd->action = CMD_ACTION_NON;
		return EINVAL;
	}

	cur_cmd.echn_idx.chn = (uint8_t)p_data.enc_idx;
	cmd->echn_idx.chn = (uint8_t)p_data.enc_idx;
	cur_cmd.data = &cur_data;
	ret = GET(VencAttr)(&cur_cmd);
	if (ret) {
		printf("Error while getting VENC attr!\n");
		cmd->action = CMD_ACTION_NON;
		return EINVAL;
	}

	if (is_get_attr) {
		SHOW(VencAttr)(&cur_cmd);
		cmd->action = CMD_ACTION_NON;
		return 0;
	}

	cmd->action = CMD_ACTION_SET;
	cur_attr = &cur_data.venc_attr;
	cur_ex = &cur_data.venc_ex;
	// If the cmd wants to change the current codec.
	if ((p_data.codec_type != -1) && ((uint32_t)p_data.codec_type != cur_attr->type)) {
		t_attr->type = p_data.codec_type;

		if ((p_data.codec_type == MPI_VENC_TYPE_H264) && (cur_attr->type == MPI_VENC_TYPE_MJPEG)) {
			// MJPEG -> H264
			if ((p_data.profile != -1) && (p_data.rc_mode != -1) && (p_data.gop_size != -1)) {
				t_attr->h264.profile = p_data.profile;
				t_attr->h264.rc.gop = (uint32_t)p_data.gop_size;
				t_attr->h264.rc.mode = p_data.rc_mode;
				t_attr->h264.rc.frm_rate_o = (p_data.frm_rate_o != -1) ? (uint32_t)p_data.frm_rate_o :
				                                                         cur_attr->mjpeg.rc.frm_rate_o;

				switch (t_attr->h264.rc.mode) {
				case MPI_RC_MODE_VBR:
					ret = reset_rc_h2645_vbr(&t_attr->h264.rc.vbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CBR:
					ret = reset_rc_h2645_cbr(&t_attr->h264.rc.cbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_SBR:
					ret = reset_rc_h2645_sbr(&t_attr->h264.rc.sbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CQP:
					ret = reset_rc_h2645_cqp(&t_attr->h264.rc.cqp, &p_data);
					break;
				default:
					printf("Unsupported RC mode '%d'!\n", t_attr->h264.rc.mode);
					ret = EINVAL;
					break;
				}

				cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
				return (ret > 0) ? EINVAL : 0;
			} else {
				printf("VENC profile and RC related attributes are necessary while converting MJPEG to H264.\n");
				return EINVAL;
			}
		} else if ((p_data.codec_type == MPI_VENC_TYPE_H265) && (cur_attr->type == MPI_VENC_TYPE_MJPEG)) {
			// MJPEG -> H265
			if ((p_data.profile != -1) && (p_data.rc_mode != -1) && (p_data.gop_size != -1)) {
				t_attr->h265.profile = p_data.profile;
				t_attr->h265.rc.gop = (uint32_t)p_data.gop_size;
				t_attr->h265.rc.mode = p_data.rc_mode;
				t_attr->h265.rc.frm_rate_o = (p_data.frm_rate_o != -1) ? (uint32_t)p_data.frm_rate_o :
				                                                         cur_attr->mjpeg.rc.frm_rate_o;

				switch (t_attr->h265.rc.mode) {
				case MPI_RC_MODE_VBR:
					ret = reset_rc_h2645_vbr(&t_attr->h265.rc.vbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CBR:
					ret = reset_rc_h2645_cbr(&t_attr->h265.rc.cbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_SBR:
					ret = reset_rc_h2645_sbr(&t_attr->h265.rc.sbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CQP:
					ret = reset_rc_h2645_cqp(&t_attr->h265.rc.cqp, &p_data);
					break;
				default:
					printf("Unsupported RC mode '%d'!\n", t_attr->h265.rc.mode);
					ret = EINVAL;
					break;
				}

				cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
				return (ret > 0) ? EINVAL : 0;
			} else {
				printf("VENC profile and RC related attributes are necessary while converting MJPEG to H265.\n");
				return EINVAL;
			}
		} else if ((p_data.codec_type == MPI_VENC_TYPE_H264) && (cur_attr->type == MPI_VENC_TYPE_H265)) {
			// H265 -> H264
			t_attr->h264.profile = (p_data.profile != -1) ? (uint32_t)p_data.profile :
			                                                cur_attr->h265.profile;
			t_attr->h264.rc.gop = (p_data.gop_size != -1) ? (uint32_t)p_data.gop_size :
			                                                cur_attr->h265.rc.gop;
			t_attr->h264.rc.frm_rate_o = (p_data.frm_rate_o != -1)
			                             ? p_data.frm_rate_o : cur_attr->h265.rc.frm_rate_o;

			if ((p_data.rc_mode != -1) && ((uint32_t)p_data.rc_mode != cur_attr->h265.rc.mode)) {
				t_attr->h264.rc.mode = p_data.rc_mode;
				switch (t_attr->h264.rc.mode) {
				case MPI_RC_MODE_VBR:
					ret = reset_rc_h2645_vbr(&t_attr->h264.rc.vbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CBR:
					ret = reset_rc_h2645_cbr(&t_attr->h264.rc.cbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_SBR:
					ret = reset_rc_h2645_sbr(&t_attr->h264.rc.sbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CQP:
					ret = reset_rc_h2645_cqp(&t_attr->h264.rc.cqp, &p_data);
					break;
				default:
					printf("Unsupported RC mode '%d'!\n", t_attr->h264.rc.mode);
					ret = EINVAL;
					break;
				}

				cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
				return (ret > 0) ? EINVAL : 0;
			} else {
				// Assign RC mode attr between H264/H265 if the mode is not change.
				t_attr->h264.rc.mode = cur_attr->h265.rc.mode;
				is_codec_change = true;

				switch (t_attr->h264.rc.mode) {
				case MPI_RC_MODE_VBR:
					t_attr->h264.rc.vbr = cur_attr->h265.rc.vbr;
					break;
				case MPI_RC_MODE_CBR:
					t_attr->h264.rc.cbr = cur_attr->h265.rc.cbr;
					break;
				case MPI_RC_MODE_SBR:
					t_attr->h264.rc.sbr = cur_attr->h265.rc.sbr;
					break;
				case MPI_RC_MODE_CQP:
					t_attr->h264.rc.cqp = cur_attr->h265.rc.cqp;
					break;
				default:
					printf("Unsupported RC mode '%d'!\n", t_attr->h264.rc.mode);
					return EINVAL;
					break;
				}
			}
		} else if ((p_data.codec_type == MPI_VENC_TYPE_H265) && (cur_attr->type == MPI_VENC_TYPE_H264)) {
			// H264 -> H265
			t_attr->h265.profile = (p_data.profile != -1) ? (uint32_t)p_data.profile :
			                                                cur_attr->h264.profile;
			t_attr->h265.rc.gop = (p_data.gop_size != -1) ? (uint32_t)p_data.gop_size :
			                                                cur_attr->h264.rc.gop;
			t_attr->h265.rc.frm_rate_o = (p_data.frm_rate_o != -1)
			                             ? p_data.frm_rate_o : cur_attr->h264.rc.frm_rate_o;

			if ((p_data.rc_mode != -1) && ((uint32_t)p_data.rc_mode != cur_attr->h264.rc.mode)) {
				t_attr->h265.rc.mode = p_data.rc_mode;
				switch (t_attr->h265.rc.mode) {
				case MPI_RC_MODE_VBR:
					ret = reset_rc_h2645_vbr(&t_attr->h265.rc.vbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CBR:
					ret = reset_rc_h2645_cbr(&t_attr->h265.rc.cbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_SBR:
					ret = reset_rc_h2645_sbr(&t_attr->h265.rc.sbr, t_ex, &p_data);
					break;
				case MPI_RC_MODE_CQP:
					ret = reset_rc_h2645_cqp(&t_attr->h265.rc.cqp, &p_data);
					break;
				default:
					printf("Unsupported RC mode '%d'!\n", t_attr->h265.rc.mode);
					ret = EINVAL;
					break;
				}

				cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
				return (ret > 0) ? EINVAL : 0;
			} else {
				// Assign RC mode attr between H264/H265 if the mode is not change.
				t_attr->h265.rc.mode = cur_attr->h264.rc.mode;
				is_codec_change = true;

				switch (t_attr->h265.rc.mode) {
				case MPI_RC_MODE_VBR:
					t_attr->h265.rc.vbr = cur_attr->h264.rc.vbr;
					break;
				case MPI_RC_MODE_CBR:
					t_attr->h265.rc.cbr = cur_attr->h264.rc.cbr;
					break;
				case MPI_RC_MODE_SBR:
					t_attr->h265.rc.sbr = cur_attr->h264.rc.sbr;
					break;
				case MPI_RC_MODE_CQP:
					t_attr->h265.rc.cqp = cur_attr->h264.rc.cqp;
					break;
				default:
					printf("Unsupported RC mode '%d'!\n", t_attr->h265.rc.mode);
					return EINVAL;
					break;
				}
			}
		} else if ((p_data.codec_type == MPI_VENC_TYPE_MJPEG) &&
		           ((cur_attr->type == MPI_VENC_TYPE_H264) || (cur_attr->type == MPI_VENC_TYPE_H265))) {
			// H264/H265 -> MJPEG
			t_attr->mjpeg.rc.frm_rate_o = (p_data.frm_rate_o != -1)
			                             ? p_data.frm_rate_o
			                             : (cur_attr->type == MPI_VENC_TYPE_H264)
			                                  ? cur_attr->h264.rc.frm_rate_o
			                                  : cur_attr->h265.rc.frm_rate_o;

			if (p_data.rc_mode != -1) {
				t_attr->mjpeg.rc.mode = p_data.rc_mode;
				ret = reset_rc_mjpeg(&t_attr->mjpeg.rc, &p_data);
			} else {
				printf("It is necessary to provide RC mode and related attributes while converting the codec from "
				       "H264 or H265 to MJPEG\n");
				ret = EINVAL;
			}

			cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
			return (ret > 0) ? EINVAL : 0;
		} else {
			// Unkown codec type!
			printf("Unsupported codec new '%d', current '%d'!\n", p_data.codec_type, cur_attr->type);
			return EINVAL;
		}
	} else {
		// Codec is not changed, also handle the change of profile, GOP and FPS.
		t_attr->type = cur_attr->type;
		switch (t_attr->type) {
		case MPI_VENC_TYPE_H264:
			t_attr->h264.profile = (p_data.profile != -1) ? (uint32_t)p_data.profile :
			                                                cur_attr->h264.profile;
			t_attr->h264.rc.gop = (p_data.gop_size != -1) ? (uint32_t)p_data.gop_size :
			                                                cur_attr->h264.rc.gop;
			t_attr->h264.rc.frm_rate_o = (p_data.frm_rate_o != -1)
				                         ? p_data.frm_rate_o : cur_attr->h264.rc.frm_rate_o;
			cur_rc_mode = cur_attr->h264.rc.mode;
			break;
		case MPI_VENC_TYPE_H265:
			t_attr->h265.profile = (p_data.profile != -1) ? (uint32_t)p_data.profile :
			                                                cur_attr->h265.profile;
			t_attr->h265.rc.gop = (p_data.gop_size != -1) ? (uint32_t)p_data.gop_size :
			                                                cur_attr->h265.rc.gop;
			t_attr->h265.rc.frm_rate_o = (p_data.frm_rate_o != -1)
				                         ? p_data.frm_rate_o : cur_attr->h265.rc.frm_rate_o;
			cur_rc_mode = cur_attr->h265.rc.mode;
			break;
		case MPI_VENC_TYPE_MJPEG:
			cur_rc_mode = cur_attr->mjpeg.rc.mode;
			t_attr->mjpeg.rc.frm_rate_o = (p_data.frm_rate_o != -1) ? (uint32_t)p_data.frm_rate_o :
			                                                          cur_attr->mjpeg.rc.frm_rate_o;
			break;
		default:
			printf("Unsupported codec '%d'!\n", t_attr->type);
			return EINVAL;
			break;
		}
	}

	// If the cmd wants to change the current RC mode.
	if ((p_data.rc_mode != -1) && ((uint32_t)p_data.rc_mode != cur_rc_mode)) {
		switch (t_attr->type) {
		case MPI_VENC_TYPE_H264:
			t_attr->h264.rc.mode = p_data.rc_mode;

			switch (t_attr->h264.rc.mode) {
			case MPI_RC_MODE_VBR:
				ret = reset_rc_h2645_vbr(&t_attr->h264.rc.vbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CBR:
				ret = reset_rc_h2645_cbr(&t_attr->h264.rc.cbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_SBR:
				ret = reset_rc_h2645_sbr(&t_attr->h264.rc.sbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CQP:
				ret = reset_rc_h2645_cqp(&t_attr->h264.rc.cqp, &p_data);
				break;
			default:
				printf("Unsupported RC mode '%d'!\n", t_attr->h264.rc.mode);
				ret = EINVAL;
				break;
			}
			break;
		case MPI_VENC_TYPE_H265:
			t_attr->h265.rc.mode = p_data.rc_mode;

			switch (t_attr->h265.rc.mode) {
			case MPI_RC_MODE_VBR:
				ret = reset_rc_h2645_vbr(&t_attr->h265.rc.vbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CBR:
				ret = reset_rc_h2645_cbr(&t_attr->h265.rc.cbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_SBR:
				ret = reset_rc_h2645_sbr(&t_attr->h265.rc.sbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CQP:
				ret = reset_rc_h2645_cqp(&t_attr->h265.rc.cqp, &p_data);
				break;
			default:
				printf("Unsupported RC mode '%d'!\n", t_attr->h265.rc.mode);
				ret = EINVAL;
				break;
			}
			break;
		case MPI_VENC_TYPE_MJPEG:
			t_attr->mjpeg.rc.mode = p_data.rc_mode;
			ret = reset_rc_mjpeg(&t_attr->mjpeg.rc, &p_data);
			break;
		default:
			printf("Unsupported codec '%d'!\n", t_attr->type);
			ret = EINVAL;
			break;
		}

		cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
		return (ret > 0) ? EINVAL : 0;
	} else {
		// RC mode is not changed, and also handle the change of specific attributes.
		ret = 0;
		*t_ex = *cur_ex;

		switch (t_attr->type) {
		case MPI_VENC_TYPE_H264:
			t_attr->h264.rc.mode = cur_attr->h264.rc.mode;

			switch (t_attr->h264.rc.mode) {
			case MPI_RC_MODE_VBR:
				if (!is_codec_change) {
					t_attr->h264.rc.vbr = cur_attr->h264.rc.vbr;
				}

				set_rc_h2645_vbr_attr(&t_attr->h264.rc.vbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CBR:
				if (!is_codec_change) {
					t_attr->h264.rc.cbr = cur_attr->h264.rc.cbr;
				}

				set_rc_h2645_cbr_attr(&t_attr->h264.rc.cbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_SBR:
				if (!is_codec_change) {
					t_attr->h264.rc.sbr = cur_attr->h264.rc.sbr;
				}

				set_rc_h2645_sbr_attr(&t_attr->h264.rc.sbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CQP:
				if (!is_codec_change) {
					t_attr->h264.rc.cqp = cur_attr->h264.rc.cqp;
				}

				set_rc_h2645_cqp_attr(&t_attr->h264.rc.cqp, &p_data);
				break;
			default:
				printf("Unsupported RC mode '%d'!\n", t_attr->h265.rc.mode);
				ret = EINVAL;
				break;
			}
			break;
		case MPI_VENC_TYPE_H265:
			t_attr->h265.rc.mode = cur_attr->h265.rc.mode;

			switch (t_attr->h265.rc.mode) {
			case MPI_RC_MODE_VBR:
				if (!is_codec_change) {
					t_attr->h265.rc.vbr = cur_attr->h265.rc.vbr;
				}

				set_rc_h2645_vbr_attr(&t_attr->h265.rc.vbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CBR:
				if (!is_codec_change) {
					t_attr->h265.rc.cbr = cur_attr->h265.rc.cbr;
				}

				set_rc_h2645_cbr_attr(&t_attr->h265.rc.cbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_SBR:
				if (!is_codec_change) {
					t_attr->h265.rc.sbr = cur_attr->h265.rc.sbr;
				}

				set_rc_h2645_sbr_attr(&t_attr->h265.rc.sbr, t_ex, &p_data);
				break;
			case MPI_RC_MODE_CQP:
				if (!is_codec_change) {
					t_attr->h265.rc.cqp = cur_attr->h265.rc.cqp;
				}

				set_rc_h2645_cqp_attr(&t_attr->h265.rc.cqp, &p_data);
				break;
			default:
				printf("Unsupported RC mode '%d'!\n", t_attr->h265.rc.mode);
				ret = EINVAL;
				break;
			}
			break;
		case MPI_VENC_TYPE_MJPEG:
			t_attr->mjpeg.rc = cur_attr->mjpeg.rc;
			ret = set_rc_mjpeg_attr(&t_attr->mjpeg.rc, &p_data);
			break;
		default:
			printf("Unsupported codec '%d'!\n", t_attr->type);
			ret = EINVAL;
			break;
		}

		cmd->action = (ret > 0) ? CMD_ACTION_NON : CMD_ACTION_SET;
		return (ret > 0) ? EINVAL : 0;
	}

	return 0;
}

static CMD_S venc_ops = MAKE_CMD("venc", CmdVencAttr, VencAttr);

__attribute__((constructor)) void regVencCmd(void)
{
	CMD_register(&venc_ops);
}
