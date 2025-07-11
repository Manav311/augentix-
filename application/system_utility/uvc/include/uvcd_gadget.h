#ifndef UVCD_GADGET_H__
#define UVCD_GADGET_H__

#include "uvcd_common.h"

#define ENTITY_ZERO     0
#define INPUT_TERMINAL  1
#define PROCESSING_UNIT 2
#define OUTPUT_TERMINAL 3

#if 0
typedef struct _uvc_interval{
    int bIntervalIndex;
    int interval;
} uvc_interval;

typedef struct _uvc_frame {
    int bFrameIndex;
    int width;
    int height;
    uvc_interval intervals[3]; //uvc updated
} uvc_frame;

typedef struct _uvc_format {
    int bFormatIndex;
    int pixelformat;
    uvc_frame frames[3];
} uvc_format;

static uvc_format uvc_formats[] = {
    { 1, V4L2_PIX_FMT_MJPEG, {
                { 1, 1920, 1080, {      // FULL HD
                        { 1, 333333},   // 30 fps
                        { 2, 500000},   // 20 fps
                        { 3, 666666},   // 15 fps
                    }
                },
                { 2, 1280, 720, {       // HD
                        { 1, 333333},   // 30 fps
                        { 2, 500000},   // 20 fps
                        { 3, 666666},   // 15 fps
                    }
                },
                { 3, 2592, 1944, {      // 5M
                        { 1, 500000},   // 20 fps
                        { 2, 666666},   // 15 fps
                        { 3, 666666},   // TBD not support
                    }
                },
            },
    },
    { 2, V4L2_PIX_FMT_H264, {
                { 1, 1920, 1080, {      // FULL HD
                        { 1, 333333},   // 30 fps
                        { 2, 500000},   // 20 fps
                        { 3, 666666},   // 15 fps
                    }
                },
                { 2, 1280, 720, {       // FULL HD
                        { 1, 333333},   // 30 fps
                        { 2, 500000},   // 20 fps
                        { 3, 666666},   // 15 fps
                    }
                },
                { 3, 2592, 1944, {      // 5M
                        { 1, 500000},   // 20 fps
                        { 2, 666666},   // 15 fps
                        { 3, 666666},   // TBD not support
                    }
                },
            },
    },
};
#endif

// UVC device operations
uvc_device *uvcOpen(char *video_dev);
void uvcInit(uvc_device *dev);
void uvcControlInit(uvc_device *dev, struct uvc_streaming_control *ctrl, int formatIndex, int frameIndex, int intervalIndex);
void uvcClose(uvc_device *dev);

// UVC events
void uvcEvents(uvc_device *dev);
void uvcEventData(uvc_device *dev, struct uvc_request_data *data);
void uvcEventSetup(uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *req);
void uvcEventUsbClass(uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *req);
void uvcRecipInterface(uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *req);
void uvcIntfControl(uvc_device *dev, int req, int entity, int selector, int length, struct uvc_request_data *response);
void uvcIntfStreaming(uvc_device *dev, int req, int selector, struct uvc_request_data *response);
void uvcMmap(uvc_device *dev, int count);
void uvcMunmap(uvc_device *dev);
void uvcEventStreamOn(uvc_device *dev);
void uvcEventStreamOff(uvc_device *dev);

// UVC thread
void *uvcThreadUpdatePu(void *ptr);
void *uvcThreadVideoStreaming(void *ptr);

#define UVCIOC_DISCONNECT              _IO('U', 2)
#define UVCIOC_CONNECT                 _IO('U', 3)

#endif /* UVCD_GADGET_H_ */
