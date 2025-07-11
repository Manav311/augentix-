#ifndef UVCD_STREAM_H_
#define UVCD_STREAM_H_

#include <linux/videodev2.h>
#include "uvcd_common.h"

int initBitstreamSystem();
void destroyBitstreamSystem();
void fillVideoBuffer(uvc_device *dev, struct v4l2_buffer *buf);

#endif /* UVCD_STREAM_H_ */
