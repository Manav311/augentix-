/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_VENC_OPTION_H_
#define AGTX_VENC_OPTION_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


#define MAX_AGTX_VENC_ENTITY_S_PROFILE_SIZE 3
#define MAX_AGTX_VENC_ENTITY_S_RC_MODE_SIZE 4
#define MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE 3
#define MAX_AGTX_VENC_OPTION_S_STRM_SIZE 4

typedef struct {
	AGTX_FLOAT max_quality_range;
	AGTX_FLOAT min_quality_range;
} AGTX_RC_VBR_PARAM_S;

typedef struct {
	AGTX_INT32 max_qp;
	AGTX_INT32 min_qp;
	AGTX_INT32 q_factor;
} AGTX_RC_CQP_PARAM_S;

typedef struct {
	AGTX_FLOAT max_q_factor;
	AGTX_FLOAT min_q_factor;
} AGTX_RC_CBR_PARAM_S;

typedef struct {
	AGTX_RC_CBR_PARAM_S cbr_param;
	AGTX_VENC_TYPE_E codec;
	AGTX_RC_CQP_PARAM_S cqp_param;
	AGTX_INT32 max_bit_rate;
	AGTX_INT32 max_gop_size;
	AGTX_INT32 min_bit_rate;
	AGTX_INT32 min_gop_size;
	AGTX_PRFL_E profile[MAX_AGTX_VENC_ENTITY_S_PROFILE_SIZE];
	AGTX_RC_MODE_E rc_mode[MAX_AGTX_VENC_ENTITY_S_RC_MODE_SIZE];
	AGTX_RC_VBR_PARAM_S vbr_param;
} AGTX_VENC_ENTITY_S;

typedef struct {
	AGTX_VENC_ENTITY_S venc[MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE];
	AGTX_INT32 venc_idx;
} AGTX_STRM_VENC_OPTION_S;

typedef struct {
	AGTX_STRM_VENC_OPTION_S strm[MAX_AGTX_VENC_OPTION_S_STRM_SIZE];
	AGTX_INT32 strm_idx;
} AGTX_VENC_OPTION_S;


#endif /* !AGTX_VENC_OPTION_H_ */
