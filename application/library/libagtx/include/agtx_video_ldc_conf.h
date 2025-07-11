/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_VIDEO_LDC_CONF_H_
#define AGTX_VIDEO_LDC_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_LDC_VIEW_TYPE_CROP,
	AGTX_LDC_VIEW_TYPE_ALL
} AGTX_LDC_VIEW_TYPE_E;


typedef struct {
	AGTX_INT32 center_x_offset;
	AGTX_INT32 center_y_offset;
	AGTX_INT32 enable;
	AGTX_INT32 ratio;
	AGTX_INT32 video_dev_idx;
	AGTX_LDC_VIEW_TYPE_E view_type;
} AGTX_LDC_CONF_S;


#endif /* AGTX_VIDEO_LDC_CONF_H_ */
