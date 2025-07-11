/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_RES_OPTION_H_
#define AGTX_RES_OPTION_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


#define MAX_AGTX_RES_ENTITY_S_FRAME_RATE_LIST_SIZE 60
#define MAX_AGTX_RES_ENTITY_S_MAX_FRAME_RATE_SIZE 8
#define MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE 8
#define MAX_AGTX_RES_OPTION_S_STRM_SIZE 4

typedef struct {
	AGTX_FLOAT frame_rate_list[MAX_AGTX_RES_ENTITY_S_FRAME_RATE_LIST_SIZE];
	AGTX_INT32 height;
	AGTX_FLOAT max_frame_rate[MAX_AGTX_RES_ENTITY_S_MAX_FRAME_RATE_SIZE];
	AGTX_INT32 width;
} AGTX_RES_ENTITY_S;

typedef struct {
	AGTX_RES_ENTITY_S res[MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE];
	AGTX_INT32 res_idx;
} AGTX_STRM_RES_OPTION_S;

typedef struct {
	AGTX_STRM_RES_OPTION_S strm[MAX_AGTX_RES_OPTION_S_STRM_SIZE];
	AGTX_INT32 strm_idx;
} AGTX_RES_OPTION_S;


#endif /* !AGTX_RES_OPTION_H_ */
