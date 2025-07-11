#ifndef AGTX_WEBCAM_H_
#define AGTX_WEBCAM_H_

#include <linux/ioctl.h>

typedef struct agtx_webcam_cap_frame {
	int wWidth;
	int wHeight;
	int bFrameIntervalType;
	unsigned int *intervals;
} AgtxWebcamCapFrame;

typedef struct agtx_webcam_cap_format {
	AGTX_VENC_TYPE_E codec;
	unsigned int dwMinBitRate;
	unsigned int dwMaxBitRate;
	int bNumFrameDescriptors;
	AgtxWebcamCapFrame *frames;
} AgtxWebcamCapFormat;

typedef struct agtx_webcam_cap {
	int bNumFormats;
	AgtxWebcamCapFormat *formats;
} AgtxWebcamCap;

#define AGTX_WEBCAM_DEV_NODE "/dev/agtx-webcam"

#define AGTX_WEBCAM_IOC_BIND 'b'
#define AGTX_WEBCAM_IOCTL_BIND   _IO(AGTX_WEBCAM_IOC_BIND, 0)
#define AGTX_WEBCAM_IOCTL_UNBIND _IO(AGTX_WEBCAM_IOC_BIND, 1)

#define AGTX_WEBCAM_IOC_CAP 'c'
#define AGTX_WEBCAM_IOCTL_CAP_SET _IOW(AGTX_WEBCAM_IOC_CAP, 0, AgtxWebcamCap)

#endif /* AGTX_WEBCAM_H_ */
