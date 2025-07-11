/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_IVA_TD_CONF_H
#define AGTX_IVA_TD_CONF_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 en_block_det;
	AGTX_INT32 en_redirect_det;
	AGTX_INT32 endurance;
	AGTX_INT32 sensitivity;
	AGTX_INT32 redirect_sensitivity;
	AGTX_INT32 redirect_global_change;
	AGTX_INT32 redirect_trigger_delay;
	AGTX_INT32 video_chn_idx;
} AGTX_IVA_TD_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_IVA_TD_CONF_H */
