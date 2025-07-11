/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_COLOR_CONF_H_
#define AGTX_COLOR_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_SW_LIGHT_SENSING_PARAM_DETECT_NAME_SIZE 255
typedef enum {
	AGTX_COLOR_MODE_DAY,
	AGTX_COLOR_MODE_NIGHT
} AGTX_COLOR_MODE_E;

#define MAX_AGTX_COLOR_CONF_S_PARAMS_SIZE 3

typedef struct {
	AGTX_INT32 bg_ratio_max;
	AGTX_INT32 bg_ratio_min;
	AGTX_INT32 day2ir_th;
	AGTX_UINT8 detect_name[MAX_AGTX_SW_LIGHT_SENSING_PARAM_DETECT_NAME_SIZE];
	AGTX_INT32 ir2day_th;
	AGTX_INT32 rg_ratio_max;
	AGTX_INT32 rg_ratio_min;
} AGTX_SW_LIGHT_SENSING_PARAM;

typedef struct {
	AGTX_COLOR_MODE_E color_mode;
	AGTX_SW_LIGHT_SENSING_PARAM params[MAX_AGTX_COLOR_CONF_S_PARAMS_SIZE];
} AGTX_COLOR_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_COLOR_CONF_H_ */
