/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_DIP_DCC_CONF_H_
#define AGTX_DIP_DCC_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

typedef enum { AGTX_DCC_TYPE, AGTX_DCC_TYPE_EX } AGTX_DCC_TYPE_E;

#define MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE 4
#define MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE 4
#define MAX_AGTX_DIP_DCC_CONF_S_DCC_SIZE 4

typedef struct {
	AGTX_INT32 auto_gain_0[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_0[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_1[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_1[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_2[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_2[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_3[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_3[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_4[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_4[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_5[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_5[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_6[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_6[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_7[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_7[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_8[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_8[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_9[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_9[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_10[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_10[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_11[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_11[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_12[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_12[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_13[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_13[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_14[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_14[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 auto_gain_15[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 auto_offset_15[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 manual_gain[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 manual_offset[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 gain[MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE];
	AGTX_INT32 offset[MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE];
	AGTX_INT32 mode;
	AGTX_DCC_TYPE_E type;
} AGTX_DIP_DCC_ATTR_S;

typedef struct {
	AGTX_DIP_DCC_ATTR_S dcc[MAX_AGTX_DIP_DCC_CONF_S_DCC_SIZE];
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_DCC_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_DIP_DCC_CONF_H_ */
