/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_RES_OPTION_H_
#define CM_RES_OPTION_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_res_option.h"

struct json_object;


void parse_res_option(AGTX_RES_OPTION_S *data, struct json_object *cmd_obj);
void comp_res_option(struct json_object *ret_obj, AGTX_RES_OPTION_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_RES_OPTION_H_ */
