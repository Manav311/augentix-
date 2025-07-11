/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_DIP_WHITE_BALANCE_INFO_H_
#define CM_DIP_WHITE_BALANCE_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_dip_wb_info.h"

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

struct json_object;

void parse_dip_white_balance_info(AGTX_DIP_WHITE_BALANCE_INFO_S *data, struct json_object *cmd_obj);
void comp_dip_white_balance_info(struct json_object *ret_obj, AGTX_DIP_WHITE_BALANCE_INFO_S *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_DIP_WHITE_BALANCE_INFO_H_ */
