/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_PRODUCT_OPTION_LIST_H_
#define CM_PRODUCT_OPTION_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_product_option_list.h"

struct json_object;


void parse_product_option_list(AGTX_PRODUCT_OPTION_LIST_S *data, struct json_object *cmd_obj);
void comp_product_option_list(struct json_object *ret_obj, AGTX_PRODUCT_OPTION_LIST_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_PRODUCT_OPTION_LIST_H_ */
