/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_LDC_CONF_H_
#define CM_LDC_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_video_ldc_conf.h"


struct json_object;


void parse_ldc_conf(AGTX_LDC_CONF_S *data, struct json_object *cmd_obj);
void comp_ldc_conf(struct json_object *ret_obj, AGTX_LDC_CONF_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_LDC_CONF_H_ */
