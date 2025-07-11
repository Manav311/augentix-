/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_DIP_CAL_CONF_H_
#define AGTX_DIP_CAL_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_CAL_CONF_S_CAL_SIZE 4

typedef struct {
	AGTX_INT32 cal_en;
	AGTX_INT32 dbc_en;
	AGTX_INT32 dcc_en;
	AGTX_INT32 lsc_en;
} AGTX_DIP_CAL_ATTR_S;

typedef struct {
	AGTX_DIP_CAL_ATTR_S cal[MAX_AGTX_DIP_CAL_CONF_S_CAL_SIZE];
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_CAL_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_DIP_CAL_CONF_H_ */
