/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_IMG_PREF_H
#define AGTX_IMG_PREF_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef enum {
	AGTX_ANTI_FLICKER_50HZ,
	AGTX_ANTI_FLICKER_60HZ,
	AGTX_ANTI_FLICKER_AUTO,
	AGTX_ANTI_FLICKER_OFF,
	AGTX_ANTI_FLICKER_NUM,
} AGTX_ANTI_FLICKER_E;


typedef struct {
	AGTX_INT16           brightness;
	AGTX_INT16           saturation;
	AGTX_INT16           hue;
	AGTX_INT16           contrast;
	AGTX_INT16           sharpness;
	AGTX_ANTI_FLICKER_E  anti_flicker;
} AGTX_IMG_PREF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_IMG_PREF_H */
