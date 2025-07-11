/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_AWB_PREF_H_
#define CM_AWB_PREF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_awb_pref.h"


struct json_object;


void parse_awb_pref(AGTX_AWB_PREF_S *data, struct json_object *cmd_obj);
void comp_awb_pref(struct json_object *ret_obj, AGTX_AWB_PREF_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_AWB_PREF_H_ */
