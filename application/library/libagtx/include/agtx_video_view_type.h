/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef AGTX_VIDEO_VIEW_TYPE_H_
#define AGTX_VIDEO_VIEW_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"

typedef enum {
	AGTX_WIN_VIEW_TYPE_NORMAL,
	AGTX_WIN_VIEW_TYPE_LDC,
	AGTX_WIN_VIEW_TYPE_PANORAMA,
	AGTX_WIN_VIEW_TYPE_PANNING,
	AGTX_WIN_VIEW_TYPE_SURROUND,
	AGTX_WIN_VIEW_TYPE_STITCH,
	AGTX_WIN_VIEW_TYPE_GRAPHICS,
} AGTX_WIN_VIEW_TYPE_E;

typedef struct {
    AGTX_INT32           video_win_idx;
    AGTX_WIN_VIEW_TYPE_E view_type;
} AGTX_VIEW_TYPE_INFO_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !AGTX_VIDEO_VIEW_TYPE_H_ */
