/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_DIP_GAMMA_CONF_H_
#define AGTX_DIP_GAMMA_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_GAMMA_CONF_S_GAMMA_MANUAL_SIZE 60

/* clang-format off */
typedef struct {
	AGTX_INT32 gamma;
	AGTX_INT32 gamma_manual[MAX_AGTX_DIP_GAMMA_CONF_S_GAMMA_MANUAL_SIZE];
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_GAMMA_CONF_S;
/* clang-format on */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AGTX_DIP_GAMMA_CONF_H_ */
