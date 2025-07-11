/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_VIDEO_STRM_CONF_H
#define AGTX_VIDEO_STRM_CONF_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef struct {
	AGTX_INT8         video_strm_idx;
	AGTX_UINT8        strm_en;
	AGTX_ROTATE_E     rotate;
	AGTX_UINT8        mirr_en;
	AGTX_UINT8        flip_en;
	AGTX_INT32        output_fps;
	AGTX_INT32        binding_capability;
	AGTX_INT32        width;
	AGTX_INT32        height;
	AGTX_VENC_TYPE_E  venc_type;
	AGTX_PRFL_E       venc_profile;
	// RC ATTR
	AGTX_RC_MODE_E    rc_mode;
	AGTX_INT32        gop_size;
	AGTX_INT32        max_frame_size;
	// CBR & common attr
	AGTX_INT32        bit_rate;
	AGTX_INT32        min_qp;
	AGTX_INT32        max_qp;
	AGTX_INT32        min_q_factor;
	AGTX_INT32        max_q_factor;
	AGTX_INT32        fluc_level;
	AGTX_INT32        scene_smooth;
	AGTX_INT32        regression_speed;
	AGTX_INT32        i_continue_weight;
	AGTX_INT32        i_qp_offset;
	AGTX_INT32        motion_tolerance_level;
	AGTX_INT32        motion_tolerance_qp;
	AGTX_INT32        motion_tolerance_qfactor;
	// VBR
	AGTX_INT32        vbr_max_bit_rate;
	AGTX_INT32        vbr_quality_level_index;
	// SBR
	AGTX_INT32        sbr_adjust_br_thres_pc;
	AGTX_INT32        sbr_adjust_step_times;
	AGTX_INT32        sbr_converge_frame;
	// CQP
	AGTX_INT32        cqp_i_frame_qp;
	AGTX_INT32        cqp_p_frame_qp;
	AGTX_INT32        cqp_q_factor;
	// Optical-flow Bit-rate Saving
	AGTX_INT32        obs;
	AGTX_INT32        obs_off_period;
} AGTX_STRM_PARAM_S;

typedef struct {
	AGTX_INT8          video_dev_idx;
	AGTX_UINT32        video_strm_cnt;
	AGTX_STRM_PARAM_S  video_strm[AGTX_MAX_VIDEO_STRM_NUM];
} AGTX_STRM_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_VIDEO_STRM_CONF_H */
