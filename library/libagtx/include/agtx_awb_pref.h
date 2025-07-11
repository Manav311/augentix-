/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_AWB_PREF_H
#define AGTX_AWB_PREF_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef struct {
	AGTX_INT32 b_gain;
	AGTX_INT32 color_temp;
	AGTX_INT32 mode;
	AGTX_INT32 r_gain;
} AGTX_AWB_PREF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_AWB_PREF_H */
