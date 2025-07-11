/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_IMG_PREF_H_
#define CM_IMG_PREF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_img_pref.h"


struct json_object;


void parse_img_pref(AGTX_IMG_PREF_S *data, struct json_object *cmd_obj);
void comp_img_pref(struct json_object *ret_obj, AGTX_IMG_PREF_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_IMG_PREF_H_ */
