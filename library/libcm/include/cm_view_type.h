/******************************************************************************
*
* copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef CM_VIEW_TYPE_H_
#define CM_VIDEO_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_video_view_type.h"

struct json_object;

void parse_view_type(AGTX_VIEW_TYPE_INFO_S *data, struct json_object *cmd_obj);
void comp_view_type(struct json_object *ret_obj, AGTX_VIEW_TYPE_INFO_S *data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !CM_VIEW_TYPE_H_ */