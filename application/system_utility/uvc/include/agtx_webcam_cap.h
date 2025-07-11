#ifndef AGTX_WEBCAM_CAP_H_
#define AGTX_WEBCAM_CAP_H_
#include "agtx_common.h"
#include "agtx_webcam.h"
/* This is a template for fixed settings of webcam capability. *
 *                                                             *
 * Only use for DEBUG or no database case                      */

static unsigned int interval_1920x1080[] = {
	400000,  416666,  434782,  454545,  476190,  500000,  526316,   555556,  588235,
	625000,  666667,  714286,  769231,  833333,  909091,  1000000,  1111111, 1250000,
	1428571, 1666667, 2000000, 2500000, 3333333, 5000000, 10000000,
};

static struct agtx_webcam_cap_frame frames[] = {
	{
	        .wWidth = 1920,
	        .wHeight = 1080,
	        .bFrameIntervalType = sizeof(interval_1920x1080) / sizeof(unsigned int *),
	        .intervals = interval_1920x1080,
	},
};

static struct agtx_webcam_cap_format formats[] = {
	{
	        .codec = AGTX_VENC_TYPE_H264,
	        .dwMinBitRate = 64,
	        .dwMaxBitRate = 4096,
	        .bNumFrameDescriptors = sizeof(frames) / sizeof(struct agtx_webcam_cap_frame),
	        .frames = frames,
	},
	{
	        .codec = AGTX_VENC_TYPE_MJPEG,
	        .dwMinBitRate = 64,
	        .dwMaxBitRate = 20480,
	        .bNumFrameDescriptors = sizeof(frames) / sizeof(struct agtx_webcam_cap_frame),
	        .frames = frames,
	},
};

static struct agtx_webcam_cap g_webcam_cap = {
	.bNumFormats = sizeof(formats) / sizeof(struct agtx_webcam_cap_format),
	.formats = formats,
};

#endif /* AGTX_WEBCAM_CAP_H_ */
