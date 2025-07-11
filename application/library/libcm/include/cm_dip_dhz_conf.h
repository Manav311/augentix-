/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_DIP_DHZ_CONF_H_
#define CM_DIP_DHZ_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_dip_dhz_conf.h"
#include "agtx_common.h"

struct json_object;

void parse_dip_dhz_conf(AGTX_DIP_DHZ_CONF_S *data, struct json_object *cmd_obj);
void comp_dip_dhz_conf(struct json_object *ret_obj, AGTX_DIP_DHZ_CONF_S *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_DIP_DHZ_CONF_H_ */
