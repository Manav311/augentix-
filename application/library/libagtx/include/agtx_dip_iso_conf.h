/******************************************************************************
*
* Copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef AGTX_DIP_ISO_CONF_H_
#define AGTX_DIP_ISO_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_ISO_CONF_S_AUTO_ISO_TABLE_SIZE 11

typedef struct {
	AGTX_INT32 di_max;
	AGTX_INT32 di_rising_speed;
	AGTX_INT32 di_fallen_speed;
	AGTX_INT32 qp_upper_th;
	AGTX_INT32 qp_lower_th;
	AGTX_INT32 enable;
} AGTX_DIP_ISO_DAA_S;

typedef struct {
	AGTX_INT32 auto_iso_table[MAX_AGTX_DIP_ISO_CONF_S_AUTO_ISO_TABLE_SIZE];
	AGTX_INT32 manual_iso;
	AGTX_INT32 mode;
	AGTX_INT32 iso_type;
	AGTX_DIP_ISO_DAA_S daa;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_ISO_CONF_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !AGTX_DIP_ISO_CONF_H_ */
