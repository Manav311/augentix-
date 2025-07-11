#include "parse_enc.h"

#include <stdio.h>
#include <string.h>

#include "sample_stream.h"

#include "parse_utils.h"

#define INVALID_ENC_CHN_IDX (-1)
#define MIN_ENC_CHN_IDX (0)
#define MAX_ENC_CHN_IDX (MPI_MAX_ENC_CHN_NUM - 1)

#define INVALID_ENC_TYPE (-1)

static void get_venc_type(MPI_VENC_TYPE_E *dest)
{
	char *val = strtok(NULL, " =\n");

	parse_str_to_upper(val);

	if (!strcmp(val, "H264")) {
		*dest = MPI_VENC_TYPE_H264;
	} else if (!strcmp(val, "H265")) {
		*dest = MPI_VENC_TYPE_H265;
	} else if (!strcmp(val, "MJPEG")) {
		*dest = MPI_VENC_TYPE_MJPEG;
	} else if (!strcmp(val, "JPEG")) {
		*dest = MPI_VENC_TYPE_JPEG;
	} else {
		printf("ERROR: Invalid VENC type (%s)\n", val);
	}

	return;
}

static void get_enc_profile(MPI_VENC_PRFL_E *dest)
{
	char *val = strtok(NULL, " =\n");

	parse_str_to_upper(val);

	if (!strcmp(val, "BASELINE")) {
		*dest = MPI_PRFL_BASELINE;
	} else if (!strcmp(val, "MAIN")) {
		*dest = MPI_PRFL_MAIN;
	} else if (!strcmp(val, "HIGH")) {
		*dest = MPI_PRFL_HIGH;
	} else {
		printf("ERROR: Invalid profile type (%s)\n", val);
	}

	return;
}

static void get_enc_rc_mode(MPI_RC_MODE_E *dest)
{
	char *val = strtok(NULL, " =\n");

	parse_str_to_upper(val);

	if (!strcmp(val, "VBR")) {
		*dest = MPI_RC_MODE_VBR;
	} else if (!strcmp(val, "CBR")) {
		*dest = MPI_RC_MODE_CBR;
	} else if (!strcmp(val, "SBR")) {
		*dest = MPI_RC_MODE_SBR;
	} else if (!strcmp(val, "CQP")) {
		*dest = MPI_RC_MODE_CQP;
	} else {
		printf("ERROR: Invalid RC mode (%s)\n", val);
	}

	return;
}

static int parse_mcvc_vbr_param(char *tok, MPI_MCVC_VBR_PARAM_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "max_bit_rate")) {
		get_value((void *)&p->max_bit_rate, TYPE_UINT32);
	} else if (!strcmp(tok, "quality_level_index")) {
		get_value((void *)&p->quality_level_index, TYPE_UINT32);
	} else if (!strcmp(tok, "fluc_level")) {
		get_value((void *)&p->fluc_level, TYPE_UINT32);
	} else if (!strcmp(tok, "regression_speed")) {
		get_value((void *)&p->regression_speed, TYPE_UINT32);
	} else if (!strcmp(tok, "scene_smooth")) {
		get_value((void *)&p->scene_smooth, TYPE_UINT32);
	} else if (!strcmp(tok, "i_continue_weight")) {
		get_value((void *)&p->i_continue_weight, TYPE_UINT32);
	} else if (!strcmp(tok, "max_qp")) {
		get_value((void *)&p->max_qp, TYPE_UINT32);
	} else if (!strcmp(tok, "i_qp_offset")) {
		get_value((void *)&p->i_qp_offset, TYPE_INT32);
	} else if (!strcmp(tok, "motion_tolerance_level")) {
		get_value((void *)&p->motion_tolerance_level, TYPE_UINT32);
	} else if (!strcmp(tok, "motion_tolerance_qp")) {
		get_value((void *)&p->motion_tolerance_qp, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_mcvc_cbr_param(char *tok, MPI_MCVC_CBR_PARAM_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "bit_rate")) {
		get_value((void *)&p->bit_rate, TYPE_UINT32);
	} else if (!strcmp(tok, "fluc_level")) {
		get_value((void *)&p->fluc_level, TYPE_UINT32);
	} else if (!strcmp(tok, "regression_speed")) {
		get_value((void *)&p->regression_speed, TYPE_UINT32);
	} else if (!strcmp(tok, "scene_smooth")) {
		get_value((void *)&p->scene_smooth, TYPE_UINT32);
	} else if (!strcmp(tok, "i_continue_weight")) {
		get_value((void *)&p->i_continue_weight, TYPE_UINT32);
	} else if (!strcmp(tok, "max_qp")) {
		get_value((void *)&p->max_qp, TYPE_UINT32);
	} else if (!strcmp(tok, "min_qp")) {
		get_value((void *)&p->min_qp, TYPE_UINT32);
	} else if (!strcmp(tok, "i_qp_offset")) {
		get_value((void *)&p->i_qp_offset, TYPE_INT32);
	} else if (!strcmp(tok, "motion_tolerance_level")) {
		get_value((void *)&p->motion_tolerance_level, TYPE_UINT32);
	} else if (!strcmp(tok, "motion_tolerance_qp")) {
		get_value((void *)&p->motion_tolerance_qp, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_mcvc_sbr_param(char *tok, MPI_MCVC_SBR_PARAM_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "bit_rate")) {
		get_value((void *)&p->bit_rate, TYPE_UINT32);
	} else if (!strcmp(tok, "fluc_level")) {
		get_value((void *)&p->fluc_level, TYPE_UINT32);
	} else if (!strcmp(tok, "regression_speed")) {
		get_value((void *)&p->regression_speed, TYPE_UINT32);
	} else if (!strcmp(tok, "scene_smooth")) {
		get_value((void *)&p->scene_smooth, TYPE_UINT32);
	} else if (!strcmp(tok, "i_continue_weight")) {
		get_value((void *)&p->i_continue_weight, TYPE_UINT32);
	} else if (!strcmp(tok, "max_qp")) {
		get_value((void *)&p->max_qp, TYPE_UINT32);
	} else if (!strcmp(tok, "min_qp")) {
		get_value((void *)&p->min_qp, TYPE_UINT32);
	} else if (!strcmp(tok, "adjust_br_thres_pc")) {
		get_value((void *)&p->adjust_br_thres_pc, TYPE_UINT32);
	} else if (!strcmp(tok, "adjust_step_times")) {
		get_value((void *)&p->adjust_step_times, TYPE_UINT32);
	} else if (!strcmp(tok, "converge_frame")) {
		get_value((void *)&p->converge_frame, TYPE_UINT32);
	} else if (!strcmp(tok, "i_qp_offset")) {
		get_value((void *)&p->i_qp_offset, TYPE_INT32);
	} else if (!strcmp(tok, "motion_tolerance_level")) {
		get_value((void *)&p->motion_tolerance_level, TYPE_UINT32);
	} else if (!strcmp(tok, "motion_tolerance_qp")) {
		get_value((void *)&p->motion_tolerance_qp, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_mcvc_cqp_param(char *tok, MPI_MCVC_CQP_PARAM_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "i_frame_qp")) {
		get_value((void *)&p->i_frame_qp, TYPE_UINT32);
	} else if (!strcmp(tok, "p_frame_qp")) {
		get_value((void *)&p->p_frame_qp, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_mcvc_param(char *tok, MPI_MCVC_RC_ATTR_S *p)
{
	int hit = 0;

	switch (p->mode) {
	case MPI_RC_MODE_VBR:
		hit = parse_mcvc_vbr_param(tok, &p->vbr);
		break;
	case MPI_RC_MODE_CBR:
		hit = parse_mcvc_cbr_param(tok, &p->cbr);
		break;
	case MPI_RC_MODE_SBR:
		hit = parse_mcvc_sbr_param(tok, &p->sbr);
		break;
	case MPI_RC_MODE_CQP:
		hit = parse_mcvc_cqp_param(tok, &p->cqp);
		break;
	default:
		printf("Invalid rc mode %d.\n", p->mode);
		hit = 0;
		break;
	}
	return hit;
}

static int parse_vc_param(char *tok, MPI_VC_RC_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "bit_rate")) {
		get_value((void *)&p->bit_rate, TYPE_UINT32);
	} else if (!strcmp(tok, "max_q_factor")) {
		get_value((void *)&p->max_q_factor, TYPE_UINT32);
	} else if (!strcmp(tok, "min_q_factor")) {
		get_value((void *)&p->min_q_factor, TYPE_UINT32);
	} else if (!strcmp(tok, "max_bit_rate")) {
		get_value((void *)&p->max_bit_rate, TYPE_UINT32);
	} else if (!strcmp(tok, "quality_level_index")) {
		get_value((void *)&p->quality_level_index, TYPE_UINT32);
	} else if (!strcmp(tok, "q_factor")) {
		get_value((void *)&p->q_factor, TYPE_UINT32);
	} else if (!strcmp(tok, "fluc_level")) {
		get_value((void *)&p->fluc_level, TYPE_UINT32);
	} else if (!strcmp(tok, "adjust_br_thres_pc")) {
		get_value((void *)&p->adjust_br_thres_pc, TYPE_UINT32);
	} else if (!strcmp(tok, "adjust_step_times")) {
		get_value((void *)&p->adjust_step_times, TYPE_UINT32);
	} else if (!strcmp(tok, "converge_frame")) {
		get_value((void *)&p->converge_frame, TYPE_UINT32);
	} else if (!strcmp(tok, "motion_tolerance_level")) {
		get_value((void *)&p->motion_tolerance_level, TYPE_UINT32);
	} else if (!strcmp(tok, "motion_tolerance_qfactor")) {
		get_value((void *)&p->motion_tolerance_qfactor, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_h264_attr(char *tok, MPI_VENC_ATTR_H264_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "venc_profile")) {
		get_enc_profile((void *)&p->profile);
	} else if (!strcmp(tok, "rc_mode")) {
		get_enc_rc_mode(&p->rc.mode);
	} else if (!strcmp(tok, "gop_size")) {
		get_value((void *)&p->rc.gop, TYPE_UINT32);
	} else if (!strcmp(tok, "enc_fps")) {
		get_value((void *)&p->rc.frm_rate_o, TYPE_INT32);
	} else if (!strcmp(tok, "max_frame_size")) {
		get_value((void *)&p->rc.max_frame_size, TYPE_UINT32);
	} else if (p->rc.mode < MPI_RC_MODE_NUM) {
		hit = parse_mcvc_param(tok, &p->rc);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_h265_attr(char *tok, MPI_VENC_ATTR_H265_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "venc_profile")) {
		get_enc_profile((void *)&p->profile);
	} else if (!strcmp(tok, "rc_mode")) {
		get_enc_rc_mode(&p->rc.mode);
	} else if (!strcmp(tok, "gop_size")) {
		get_value((void *)&p->rc.gop, TYPE_UINT32);
	} else if (!strcmp(tok, "enc_fps")) {
		get_value((void *)&p->rc.frm_rate_o, TYPE_UINT32);
	} else if (!strcmp(tok, "max_frame_size")) {
		get_value((void *)&p->rc.max_frame_size, TYPE_UINT32);
	} else if (p->rc.mode < MPI_RC_MODE_NUM) {
		hit = parse_mcvc_param(tok, &p->rc);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_mjpeg_attr(char *tok, MPI_VENC_ATTR_MJPEG_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "rc_mode")) {
		get_enc_rc_mode(&p->rc.mode);
	} else if (!strcmp(tok, "enc_fps")) {
		get_value((void *)&p->rc.frm_rate_o, TYPE_UINT32);
	} else if (!strcmp(tok, "max_frame_size")) {
		get_value((void *)&p->rc.max_frame_size, TYPE_UINT32);
	} else if (p->rc.mode < MPI_RC_MODE_NUM) {
		hit = parse_vc_param(tok, &p->rc);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_jpeg_attr(char *tok, MPI_VENC_ATTR_JPEG_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "q_factor")) {
		get_value((void *)&p->q_factor, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_chn_attr(char *tok, MPI_ENC_CHN_ATTR_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "enc_res")) {
		get_res(&p->res);
	} else if (!strcmp(tok, "max_enc_res")) {
		get_res(&p->max_res);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_bind_info(char *tok, MPI_ENC_BIND_INFO_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "bind_dev_idx")) {
		get_value((void *)&p->idx.dev, TYPE_UINT8);
	} else if (!strcmp(tok, "bind_chn_idx")) {
		get_value((void *)&p->idx.chn, TYPE_UINT8);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_enc_venc_attr(char *tok, MPI_VENC_ATTR_S *p)
{
	static MPI_VENC_TYPE_E type = MPI_VENC_TYPE_NUM;

	int hit = 0;

	if (!strcmp(tok, "venc_type")) {
		get_venc_type((void *)&p->type);
		type = p->type;

		hit = 1;

		goto end;
	}

	if (type < MPI_VENC_TYPE_NUM) {
		switch (type) {
		case MPI_VENC_TYPE_H264:
			hit = parse_enc_h264_attr(tok, &p->h264);
			break;
		case MPI_VENC_TYPE_H265:
			hit = parse_enc_h265_attr(tok, &p->h265);
			break;
		case MPI_VENC_TYPE_MJPEG:
			hit = parse_enc_mjpeg_attr(tok, &p->mjpeg);
			break;
		case MPI_VENC_TYPE_JPEG:
			hit = parse_enc_jpeg_attr(tok, &p->jpeg);

			break;
		default:
			printf("Invalid encode type %d.\n", type);
			hit = 0;
			break;
		}
	} else {
		hit = 0;
	}
end:
	return hit;
}

static int parse_enc_venc_attr_ex(char *tok, MPI_VENC_ATTR_EX_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "obs")) {
		get_value((void *)&p->obs, TYPE_UINT32);
	} else if (!strcmp(tok, "obs_off_period")) {
		get_value((void *)&p->obs_off_period, TYPE_UINT32);
	} else {
		hit = 0;
	}

	return hit;
}

static int parse_stream_param(char *tok, CONF_BITSTREAM_PARAM_S *p)
{
	int hit = 1;

	if (!strcmp(tok, "udpstream_enable")) {
		get_value((void *)&p->stream.enable, TYPE_UINT8);
	} else if (!strcmp(tok, "frame_num")) {
		get_value((void *)&p->record.frame_num, TYPE_INT32);
	} else if (!strcmp(tok, "record_enable")) {
		get_value((void *)&p->record.enable, TYPE_UINT8);
	} else if (!strcmp(tok, "client_ip")) {
		char *val = strtok(NULL, " =\n");
		strncpy(p->stream.client_ip, val, MAX_IP_LENGTH);
	} else if (!strcmp(tok, "client_port")) {
		get_value((void *)&p->stream.client_port, TYPE_UINT32);
	} else if (!strcmp(tok, "output_file")) {
		char *val = strtok(NULL, " =\n");
		strcpy(p->record.fname, val);
	} else if (!strcmp(tok, "max_dumped_files")) {
		get_value((void *)&p->record.max_dumped_files, TYPE_UINT8);
	} else {
		hit = 0;
	}

	return hit;
}

static void print_mcvc_rc_param(MPI_RC_MODE_E mode, MPI_MCVC_RC_ATTR_S *rc)
{
	switch (mode) {
	case MPI_RC_MODE_VBR:
		printf("VBR: bit_rate = %u, fluc_level = %u, regression_speed = %u, scene_smooth = %u, i_continue_weight = %u, "
		       "quality_level_index = %u, max_qp = %u, i_qp_offset = %d, motion_tolerance_level = %u, "
			   "motion_tolerance_qp = %u.\n", rc->vbr.max_bit_rate, rc->vbr.fluc_level, rc->vbr.regression_speed,
		       rc->vbr.scene_smooth, rc->vbr.i_continue_weight, rc->vbr.quality_level_index, rc->vbr.max_qp,
		       rc->vbr.i_qp_offset, rc->vbr.motion_tolerance_level, rc->vbr.motion_tolerance_qp);
		break;
	case MPI_RC_MODE_CBR:
		printf("CBR: bit_rate = %u, fluc_level = %u, regression_speed = %u, scene_smooth = %u, i_continue_weight = %u, "
		       "min_qp = %u, max_qp = %u, i_qp_offset = %d, motion_tolerance_level = %u, motion_tolerance_qp = %u.\n",
		       rc->cbr.bit_rate, rc->cbr.fluc_level, rc->cbr.regression_speed, rc->cbr.scene_smooth,
		       rc->cbr.i_continue_weight, rc->cbr.min_qp, rc->cbr.max_qp, rc->cbr.i_qp_offset,
		       rc->cbr.motion_tolerance_level, rc->cbr.motion_tolerance_qp);
		break;
	case MPI_RC_MODE_SBR:
		printf("SBR: bit_rate = %u, fluc_level = %u, regression_speed = %u, scene_smooth = %u, i_continue_weight = %u, "
		       "min_qp = %u, max_qp = %u, i_qp_offset = %d, adjust_br_thres_pc = %u, adjust_step_times = %u, "
		       "converge_frame = %u, motion_tolerance_level = %u, motion_tolerance_qp = %u.\n", rc->sbr.bit_rate,
		       rc->sbr.fluc_level, rc->sbr.regression_speed, rc->sbr.scene_smooth, rc->sbr.i_continue_weight,
		       rc->sbr.min_qp, rc->sbr.max_qp, rc->sbr.i_qp_offset, rc->sbr.adjust_br_thres_pc,
		       rc->sbr.adjust_step_times, rc->sbr.converge_frame, rc->sbr.motion_tolerance_level,
		       rc->sbr.motion_tolerance_qp);
		break;
	case MPI_RC_MODE_CQP:
		printf("CQP: i_frame_qp = %u, p_frame_qp = %u.\n", rc->cqp.i_frame_qp, rc->cqp.p_frame_qp);
		break;
	default:
		printf("Incorrect rate control mode!\n");
		break;
	}
}

static void print_vc_rc_param(MPI_VENC_TYPE_E mode, MPI_VC_RC_ATTR_S *rc)
{
	switch (mode) {
	case MPI_RC_MODE_VBR:
		printf("VBR: max_bit_rate = %u, fluc_level = %u, quality_level_index = %u.\n", rc->max_bit_rate,
		       rc->fluc_level, rc->quality_level_index);
		break;
	case MPI_RC_MODE_CBR:
		printf("CBR: bit_rate = %u, fluc_level = %u, max_q_factor = %u, min_q_factor = %u.\n", rc->bit_rate,
		       rc->fluc_level, rc->max_q_factor, rc->min_q_factor);
		break;
	case MPI_RC_MODE_SBR:
		printf("SBR: bit_rate = %u, fluc_level = %u, max_q_factor = %u, min_q_factor = %u, adjust_br_thres_pc = %u, "
		       "adjust_step_times = %u, converge_frame = %d.\n", rc->bit_rate, rc->fluc_level, rc->max_q_factor,
		       rc->min_q_factor, rc->adjust_br_thres_pc, rc->adjust_step_times, rc->converge_frame);
		break;
	case MPI_RC_MODE_CQP:
		printf("CQP: q_factor = %u.\n", rc->q_factor);
		break;
	default:
		printf("Incorrect rate control mode!\n");
		break;
	}
}

int parse_enc_chn_param(char *tok, SAMPLE_CONF_S *conf)
{
	static INT32 idx = INVALID_ENC_CHN_IDX;

	int hit = 0;

	//	printf("tok = |%s|.\n", tok);

	if (!strcmp(tok, "enc_idx")) {
		get_value((void *)&idx, TYPE_INT32);

		if (idx >= MPI_MAX_ENC_CHN_NUM) {
			return 0;
		}

		conf->enc_chn[idx].enable = 1;
		return 1;
	}

	/* Parae other device parameter for valid channel index */
	if (idx >= MIN_ENC_CHN_IDX && idx <= MAX_ENC_CHN_IDX) {
		CONF_ECHN_PARAM_S *p = &conf->enc_chn[idx];

		/* Parse case channel related parameter */
		hit = parse_stream_param(tok, &conf->bitstream[idx]);
		if (hit) {
			goto end;
		}

		/* Parse encoder channel attr */
		hit = parse_enc_chn_attr(tok, &p->attr);
		if (hit) {
			goto end;
		}

		/* Parse binding info */
		hit = parse_enc_bind_info(tok, &p->bind);
		if (hit) {
			goto end;
		}

		/* Parse video encoder attr */
		hit = parse_enc_venc_attr(tok, &p->venc_attr);
		if (hit) {
			goto end;
		}

		/* Parse video encoder attr ex*/
		hit = parse_enc_venc_attr_ex(tok, &p->venc_ex);
		if (hit) {
			goto end;
		}
	}

end:
	return hit;
}

void print_venc_attr(MPI_VENC_ATTR_S *p)
{
	switch (p->type) {
	case MPI_VENC_TYPE_H264:
		printf("H264:\n");
		printf("venc_profile = %d, rc_mode = %d, gop_size = %u, frm_rate_o = %d, max_frame_size = %u\n",
		       p->h264.profile, p->h264.rc.mode, p->h264.rc.gop, p->h264.rc.frm_rate_o, p->h264.rc.max_frame_size);
		print_mcvc_rc_param(p->h264.rc.mode, &p->h264.rc);
		break;
	case MPI_VENC_TYPE_H265:
		printf("H265:\n");
		printf("venc_profile = %d, rc_mode = %d, gop_size = %u, frm_rate_o = %d, max_frame_size = %u\n",
		       p->h265.profile, p->h265.rc.mode, p->h265.rc.gop, p->h265.rc.frm_rate_o, p->h265.rc.max_frame_size);
		print_mcvc_rc_param(p->h265.rc.mode, &p->h265.rc);
		break;
	case MPI_VENC_TYPE_MJPEG:
		printf("MJPEG:\n");
		printf("rc_mode = %d, frm_rate_o = %d, max_frame_size = %u\n", p->mjpeg.rc.mode, p->mjpeg.rc.frm_rate_o,
		       p->mjpeg.rc.max_frame_size);
		print_vc_rc_param(p->mjpeg.rc.mode, &p->mjpeg.rc);
		break;
	case MPI_VENC_TYPE_JPEG:
		printf("JPEG: q_factor = %u.\n", p->jpeg.q_factor);
		break;
	default:
		printf("Incorrect encode type!\n");
		break;
	}
}
