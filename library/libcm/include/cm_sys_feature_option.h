/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_SYS_FEATURE_OPTION_H_
#define CM_SYS_FEATURE_OPTION_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_sys_feature_option.h"


struct json_object;


void parse_sys_feature_option(AGTX_SYS_FEATURE_OPTION_S *data, struct json_object *cmd_obj);
void comp_sys_feature_option(struct json_object *ret_obj, AGTX_SYS_FEATURE_OPTION_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_SYS_FEATURE_OPTION_H_ */
