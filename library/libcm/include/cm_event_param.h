/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_EVENT_PARAM_H_
#define CM_EVENT_PARAM_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_event_param.h"


struct json_object;


void parse_event_param(AGTX_EVENT_PARAM_S *data, struct json_object *cmd_obj);
void comp_event_param(struct json_object *ret_obj, AGTX_EVENT_PARAM_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_EVENT_PARAM_H_ */
