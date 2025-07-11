#ifndef AGTX_DIP_ROI_CONF_H_
#define AGTX_DIP_ROI_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_DIP_ROI_CONF_S_ROI_SIZE 4

typedef struct {
	AGTX_INT32 awb_roi_ex;
	AGTX_INT32 awb_roi_ey;
	AGTX_INT32 awb_roi_sx;
	AGTX_INT32 awb_roi_sy;
	AGTX_INT32 luma_roi_ex;
	AGTX_INT32 luma_roi_ey;
	AGTX_INT32 luma_roi_sx;
	AGTX_INT32 luma_roi_sy;
	AGTX_INT32 zone_lum_avg_roi_ex;
	AGTX_INT32 zone_lum_avg_roi_ey;
	AGTX_INT32 zone_lum_avg_roi_sx;
	AGTX_INT32 zone_lum_avg_roi_sy;
} AGTX_DIP_ROI_ATTR_S;

typedef struct {
	AGTX_DIP_ROI_ATTR_S roi[MAX_AGTX_DIP_ROI_CONF_S_ROI_SIZE];
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_ROI_CONF_S;

#endif /* AGTX_DIP_ROI_CONF_H_ */
