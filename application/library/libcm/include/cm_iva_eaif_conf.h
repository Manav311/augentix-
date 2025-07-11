/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_IVA_EAIF_CONF_H_
#define CM_IVA_EAIF_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_iva_eaif_conf.h"

#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

struct json_object;


void parse_iva_eaif_conf(AGTX_IVA_EAIF_CONF_S *data, struct json_object *cmd_obj);
void comp_iva_eaif_conf(struct json_object *ret_obj, AGTX_IVA_EAIF_CONF_S *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_IVA_EAIF_CONF_H_ */
