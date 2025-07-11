/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef AGTX_DIP_CORING_CONF_H_
#define AGTX_DIP_CORING_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_CORING_CONF_S_AUTO_ABS_TH_LIST_SIZE 11

typedef struct {
	AGTX_INT32 auto_abs_th_list[MAX_AGTX_DIP_CORING_CONF_S_AUTO_ABS_TH_LIST_SIZE];
	AGTX_INT32 coring_slope;
	AGTX_INT32 manual_abs_th;
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_CORING_CONF_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AGTX_DIP_CORING_CONF_H_ */
