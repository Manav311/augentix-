/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_STITCH_CONF_H_
#define AGTX_STITCH_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


#define MAX_AGTX_STITCH_CONF_S_PARAMETER_SIZE 3


typedef struct {
	AGTX_INT8    tbl_idx;
	AGTX_UINT16  dist;
	AGTX_UINT16  ver_disp;
	AGTX_UINT16  straighten;
	AGTX_UINT16  src_zoom;
	AGTX_INT16   theta_0;
	AGTX_INT16   theta_1;
	AGTX_UINT16  radius_0;
	AGTX_UINT16  radius_1;
	AGTX_UINT16  curvature_0;
	AGTX_UINT16  curvature_1;
	AGTX_UINT16  fov_ratio_0;
	AGTX_UINT16  fov_ratio_1;
	AGTX_UINT16  ver_scale_0;
	AGTX_UINT16  ver_scale_1;
	AGTX_INT16   ver_shift_0;
	AGTX_INT16   ver_shift_1;
} AGTX_STITCH_TABLE_S;

typedef struct {
	AGTX_INT8            video_dev_idx;
	AGTX_UINT8           enable;
	AGTX_INT16           dft_dist;
	AGTX_UINT16          center_0_x;
	AGTX_UINT16          center_0_y;
	AGTX_UINT16          center_1_x;
	AGTX_UINT16          center_1_y;
	AGTX_UINT32          dist_tbl_cnt;
	AGTX_STITCH_TABLE_S  dist_tbl[AGTX_MAX_STITCH_TABLE_NUM];
} AGTX_STITCH_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_STITCH_CONF_H_ */
