/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_DIP_AWB_CONF_H_
#define AGTX_DIP_AWB_CONF_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_AWB_COLOR_TEMP_S_GAIN_SIZE 4
#define MAX_AGTX_DIP_AWB_COLOR_TEMP_S_MAXTRIX_SIZE 9
#define MAX_AGTX_DIP_AWB_COLOR_DELTA_S_GAIN_SIZE 4

/* clang-format off */
typedef enum {
	AGTX_AWB_CCM_DOMA_GAMMA_ENCODED,
	AGTX_AWB_CCM_DOMA_LINEAR
} AGTX_AWB_CCM_DOMAIN;
/* clang-format on */

#define MAX_AGTX_DIP_AWB_CONF_S_DELTA_TABLE_LIST_SIZE 8
#define MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_BIAS_LIST_SIZE 8
#define MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_LIST_SIZE 8

typedef struct {
	AGTX_INT32 gain[MAX_AGTX_DIP_AWB_COLOR_TEMP_S_GAIN_SIZE];
	AGTX_INT32 k;
	AGTX_INT32 maxtrix[MAX_AGTX_DIP_AWB_COLOR_TEMP_S_MAXTRIX_SIZE];
} AGTX_DIP_AWB_COLOR_TEMP_S;

typedef struct {
	AGTX_INT32 b_extra_gain_bias;
	AGTX_INT32 color_tolerance_bias;
	AGTX_INT32 g_extra_gain_bias;
	AGTX_INT32 gwd_weight_bias;
	AGTX_INT32 k;
	AGTX_INT32 r_extra_gain_bias;
	AGTX_INT32 wht_weight_bias;
} AGTX_DIP_AWB_COLOR_TEMP_BIAS_S;

typedef struct {
	AGTX_INT32 gain[MAX_AGTX_DIP_AWB_COLOR_DELTA_S_GAIN_SIZE];
} AGTX_DIP_AWB_COLOR_DELTA_S;

typedef struct {
	AGTX_INT32 b_extra_gain;
	AGTX_AWB_CCM_DOMAIN ccm_domain;
	AGTX_INT32 color_tolerance;
	AGTX_DIP_AWB_COLOR_DELTA_S delta_table_list[MAX_AGTX_DIP_AWB_CONF_S_DELTA_TABLE_LIST_SIZE];
	AGTX_INT32 g_extra_gain;
	AGTX_INT32 gwd_weight;
	AGTX_INT32 high_k;
	AGTX_DIP_AWB_COLOR_TEMP_BIAS_S k_table_bias_list[MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_BIAS_LIST_SIZE];
	AGTX_DIP_AWB_COLOR_TEMP_S k_table_list[MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_LIST_SIZE];
	AGTX_INT32 k_table_valid_size;
	AGTX_INT32 low_k;
	AGTX_INT32 max_lum_gain;
	AGTX_INT32 over_exp_th;
	AGTX_INT32 r_extra_gain;
	AGTX_INT32 speed;
	AGTX_INT32 video_dev_idx;
	AGTX_INT32 wht_density;
	AGTX_INT32 wht_weight;
} AGTX_DIP_AWB_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AGTX_DIP_AWB_CONF_H_ */
