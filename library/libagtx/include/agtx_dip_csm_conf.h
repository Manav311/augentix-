/******************************************************************************
*
* Copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef AGTX_DIP_CSM_CONF_H_
#define AGTX_DIP_CSM_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_CST_MATRIX_S_COEFF_SIZE 9
#define MAX_AGTX_DIP_CST_MATRIX_S_OFFSET_SIZE 3
#define MAX_AGTX_DIP_CSM_CONF_S_AUTO_SAT_TABLE_SIZE 11

typedef struct {
	AGTX_INT32 coeff[MAX_AGTX_DIP_CST_MATRIX_S_COEFF_SIZE];
	AGTX_INT32 offset[MAX_AGTX_DIP_CST_MATRIX_S_OFFSET_SIZE];
} AGTX_DIP_CST_MATRIX_S;

typedef struct {
	AGTX_INT32 auto_sat_table[MAX_AGTX_DIP_CSM_CONF_S_AUTO_SAT_TABLE_SIZE];
	AGTX_INT32 bw_en;
	AGTX_DIP_CST_MATRIX_S cst_auto_0;
	AGTX_DIP_CST_MATRIX_S cst_auto_1;
	AGTX_DIP_CST_MATRIX_S cst_auto_10;
	AGTX_DIP_CST_MATRIX_S cst_auto_2;
	AGTX_DIP_CST_MATRIX_S cst_auto_3;
	AGTX_DIP_CST_MATRIX_S cst_auto_4;
	AGTX_DIP_CST_MATRIX_S cst_auto_5;
	AGTX_DIP_CST_MATRIX_S cst_auto_6;
	AGTX_DIP_CST_MATRIX_S cst_auto_7;
	AGTX_DIP_CST_MATRIX_S cst_auto_8;
	AGTX_DIP_CST_MATRIX_S cst_auto_9;
	AGTX_INT32 cst_auto_en;
	AGTX_DIP_CST_MATRIX_S cst_bw;
	AGTX_DIP_CST_MATRIX_S cst_color;
	AGTX_INT32 hue;
	AGTX_INT32 manual_sat;
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_CSM_CONF_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !AGTX_DIP_CSM_CONF_H_ */