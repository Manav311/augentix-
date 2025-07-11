/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_VIDEO_DEV_CONF_H
#define AGTX_VIDEO_DEV_CONF_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef enum {
	AGTX_HDR_MODE_NONE,
	AGTX_HDR_MODE_FRAME_PARL,
	AGTX_HDR_MODE_FRAME_ITLV,
	AGTX_HDR_MODE_TOP_N_BTM,
	AGTX_HDR_MODE_SIDE_BY_SIDE,
	AGTX_HDR_MODE_LINE_COLOC,
	AGTX_HDR_MODE_LINE_ITLV,
	AGTX_HDR_MODE_PIX_COLOC,
	AGTX_HDR_MODE_PIX_ITLV,
	AGTX_HDR_MODE_FRAME_COMB,
} AGTX_HDR_MODE_E;


typedef struct {
	AGTX_INT8   path_idx;
	AGTX_UINT8  path_en;
	AGTX_INT8   sensor_idx;
	AGTX_FLOAT  fps;
	AGTX_UINT16 width;
	AGTX_UINT16 height;
	AGTX_INT8   eis_strength;
} AGTX_PATH_CONF_S;

typedef struct {
	AGTX_INT8         video_dev_idx;
	AGTX_INT8         input_fps; /* TODO: remove it */
	AGTX_UINT8        stitch_en;
	AGTX_UINT8        eis_en;
	AGTX_HDR_MODE_E   hdr_mode;
	AGTX_BAYER_E      bayer;     /* TODO: remove it */
	AGTX_UINT32       input_path_cnt;
	AGTX_PATH_CONF_S  input_path[AGTX_MAX_INPUT_PATH_NUM];
} AGTX_DEV_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_VIDEO_DEV_CONF_H */
