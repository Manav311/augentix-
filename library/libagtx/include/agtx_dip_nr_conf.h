/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/
#ifndef AGTX_DIP_NR_CONF_H_
#define AGTX_DIP_NR_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_2D_LIST_SIZE 11
#define MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_3D_LIST_SIZE 11
#define MAX_AGTX_DIP_NR_CONF_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE 11
#define MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_2D_LIST_SIZE 11
#define MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_3D_LIST_SIZE 11

typedef struct {
	AGTX_INT32 auto_c_level_2d_list[MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_2D_LIST_SIZE];
	AGTX_INT32 auto_c_level_3d_list[MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_3D_LIST_SIZE];
	AGTX_INT32 auto_fss_y_level_3d_list[MAX_AGTX_DIP_NR_CONF_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE];
	AGTX_INT32 auto_y_level_2d_list[MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_2D_LIST_SIZE];
	AGTX_INT32 auto_y_level_3d_list[MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_3D_LIST_SIZE];
	AGTX_INT32 fss_ratio_max;
	AGTX_INT32 fss_ratio_min;
	AGTX_INT32 ghost_remove;
	AGTX_NR_LUT_TYPE_E lut_type;
	AGTX_INT32 ma_c_strength;
	AGTX_INT32 ma_y_strength;
	AGTX_INT32 manual_c_level_2d; /* This is a description. */
	AGTX_INT32 manual_c_level_3d;
	AGTX_INT32 manual_fss_y_level_3d;
	AGTX_INT32 manual_y_level_2d;
	AGTX_INT32 manual_y_level_3d;
	AGTX_INT32 mc_y_level_offset;
	AGTX_INT32 mc_y_strength;
	AGTX_INT32 me_frame_fallback_en;
	AGTX_INT32 mode;
	AGTX_INT32 motion_comp;
	AGTX_INT32 ratio_3d;
	AGTX_INT32 trail_suppress;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_NR_CONF_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !AGTX_DIP_NR_CONF_H_ */
