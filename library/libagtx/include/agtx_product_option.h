/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_PRODUCT_OPTION_H_
#define AGTX_PRODUCT_OPTION_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

typedef enum {
	AGTX_PRODUCT_VIDEO_TYPE_NORMAL,
	AGTX_PRODUCT_VIDEO_TYPE_STITCH,
	AGTX_PRODUCT_VIDEO_TYPE_BCD
} AGTX_PRODUCT_VIDEO_TYPE_E;

#define _MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE 3
#define _MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE 4
#define _MAX_AGTX_RES_OPTION_S_MAX_FRAME_RATE_SIZE 8
#define _MAX_AGTX_RES_OPTION_S_FRAME_RATE_LIST_SIZE 60
#define _MAX_AGTX_VIDEO_OPTION_S_RES_SIZE 8
#define _MAX_AGTX_VIDEO_OPTION_S_VENC_SIZE 3
#define _MAX_AGTX_PRODUCT_OPTION_S_VIDEO_OPTION_SIZE 4

typedef struct {
	AGTX_FLOAT max_quality_range;
	AGTX_FLOAT min_quality_range;
} _AGTX_RC_VBR_PARAM_S;

typedef struct {
	AGTX_INT32 max_qp;
	AGTX_INT32 min_qp;
	AGTX_INT32 q_factor;
} _AGTX_RC_CQP_PARAM_S;

typedef struct {
	AGTX_FLOAT max_q_factor;
	AGTX_FLOAT min_q_factor;
} _AGTX_RC_CBR_PARAM_S;

typedef struct {
       _AGTX_RC_CBR_PARAM_S cbr_param;
	AGTX_VENC_TYPE_E codec;
       _AGTX_RC_CQP_PARAM_S cqp_param;
	AGTX_INT32 max_bit_rate;
	AGTX_INT32 max_gop_size;
	AGTX_INT32 min_bit_rate;
	AGTX_INT32 min_gop_size;
	AGTX_PRFL_E profile[_MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE];
	AGTX_RC_MODE_E rc_mode[_MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE];
       _AGTX_RC_VBR_PARAM_S vbr_param;
} _AGTX_VENC_OPTION_S;

typedef struct {
	AGTX_FLOAT max_frame_rate[_MAX_AGTX_RES_OPTION_S_MAX_FRAME_RATE_SIZE];
	AGTX_FLOAT frame_rate_list[_MAX_AGTX_RES_OPTION_S_FRAME_RATE_LIST_SIZE];
	AGTX_INT32 height;
	AGTX_INT32 width;
} _AGTX_RES_OPTION_S;

typedef struct {
       _AGTX_RES_OPTION_S res[_MAX_AGTX_VIDEO_OPTION_S_RES_SIZE];
       _AGTX_VENC_OPTION_S venc[_MAX_AGTX_VIDEO_OPTION_S_VENC_SIZE];
} _AGTX_VIDEO_OPTION_S;

typedef struct {
	   AGTX_PRODUCT_VIDEO_TYPE_E product_video_type;
       _AGTX_VIDEO_OPTION_S video_option[_MAX_AGTX_PRODUCT_OPTION_S_VIDEO_OPTION_SIZE];
} _AGTX_PRODUCT_OPTION_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_PRODUCT_OPTION_H_ */
