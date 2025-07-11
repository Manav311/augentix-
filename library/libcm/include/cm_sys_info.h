/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_SYS_INFO_H_
#define CM_SYS_INFO_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_sys_info.h"


struct json_object;


void parse_sys_info(AGTX_SYS_INFO_S *data, struct json_object *cmd_obj);
void comp_sys_info(struct json_object *ret_obj, AGTX_SYS_INFO_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_SYS_INFO_H_ */
