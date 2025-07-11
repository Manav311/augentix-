/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_DIP_LSC_CONF_H_
#define AGTX_DIP_LSC_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_LSC_CONF_S_LSC_SIZE 4

typedef struct {
	AGTX_INT32 origin;
	AGTX_INT32 tilt;
	AGTX_INT32 x_curvature;
	AGTX_INT32 x_trend;
	AGTX_INT32 y_curvature;
	AGTX_INT32 y_trend;
} AGTX_DIP_LSC_ATTR_S;

typedef struct {
	AGTX_DIP_LSC_ATTR_S lsc[MAX_AGTX_DIP_LSC_CONF_S_LSC_SIZE];
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_LSC_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_DIP_LSC_CONF_H_ */
