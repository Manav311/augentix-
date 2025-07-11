/*
 * UVC gadget
 */
#define _GNU_SOURCE

#ifdef CONFIG_WEBCAM_CAP_FIXED
#include "agtx_webcam_cap.h"
#include "cm_res_option.h"
#include "cm_venc_option.h"
#else
#include "agtx_res_option.h"
#include "agtx_venc_option.h"
#include "cm_res_option.h"
#include "cm_venc_option.h"
#endif

#include "agtx_cmd.h"
#include "webcam.h"
#include "mpi_base_types.h"
#include "uvcd_common.h"
#include "uvcd_gadget.h"
#include "uvcd_stream.h"
#include "uvcd_ccclient.h"
#include "agtx_types.h"

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <assert.h>
#include <getopt.h>
#include <json.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/file.h>
#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

#define UVCD_THREAD_STREAM_NAME "uvc_stream"
#define UVCD_THREAD_UPDATE_PU_NAME "uvc_update_pu"
#define LOCK_NAME "/tmp/uvcd.lock"

#ifndef BIT
#define BIT(x) (1 << x)
#endif
#define BitToByte(x) (x >> 3)
#define BitToKBit(x) (x >> 10)
#define KBitToBit(x) (x << 10)

#define UVCD_UPDATE_IDLE 0

#define UVCD_UPDATE_PU_MASK 0x3
#define UVCD_UPDATE_PU_PART_MASK 0x1C
#define UVCD_UPDATE_PU_IDLE 0
#define UVCD_UPDATE_PU_PREPARING BIT(0)
#define UVCD_UPDATE_PU_UPDATING BIT(1)
#define UVCD_UPDATE_PU_PART1 BIT(2)
#define UVCD_UPDATE_PU_PART2 BIT(3)
#define UVCD_UPDATE_PU_PART3 BIT(4)

uvc_device *dev_for_signal = NULL;
volatile int streaming = 0;
volatile int halt = 0;
volatile UINT32 update_flag = UVCD_UPDATE_IDLE;
pthread_t threadStreaming;
pthread_t threadUpdating;
int g_agtx_webcam_fd;

#ifdef CONFIG_WEBCAM_CAP_FIXED
AgtxWebcamCapFrame *g_webcam_cap_frames = NULL;
AgtxWebcamCapFormat *g_webcam_cap_formats = NULL;
#else
AgtxWebcamCapFrame *g_webcam_cap_frames = NULL;
AgtxWebcamCapFormat *g_webcam_cap_formats = NULL;
AgtxWebcamCap g_webcam_cap;
#endif

static uvc_control default_uvc_controls[] = {
	{ 0, 0, 0, 0 }, //UVC_PU_CONTROL_UNDEFINED
	{ 0, 1, 0, 0 }, //UVC_PU_BACKLIGHT_COMPENSATION_CONTROL
	{ 0, 100, 50, 50 }, //UVC_PU_BRIGHTNESS_CONTROL
	{ 0, 100, 50, 50 }, //UVC_PU_CONTRAST_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_GAIN_CONTROL
	{ 0, 2, 0, 0 }, //UVC_PU_POWER_LINE_FREQUENCY_CONTROL
	{ -90, 90, 0, 0 }, //UVC_PU_HUE_CONTROL
	{ 0, 100, 50, 50 }, //UVC_PU_SATURATION_CONTROL
	{ 0, 100, 50, 50 }, //UVC_PU_SHARPNESS_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_GAMMA_CONTROL
	{ 2800, 8000, 6500, 6500 }, //UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL
	{ 0, 1, 1, 1 }, //UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_DIGITAL_MULTIPLIER_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_HUE_AUTO_CONTROL
	{ 0, 0, 0, 0 }, //UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL
	{ 0, 0, 0, 0 } //UVC_PU_ANALOG_LOCK_STATUS_CONTROL
};

/* -----------
 * UVC device operations
 */
uvc_device *uvcOpen(char *video_dev)
{
	trace("%s=>\n", __func__);
	uvc_device *dev = malloc(sizeof(*dev));
	memset(dev, 0, sizeof(*dev));
	dev->entity = 0;
	dev->control = UVC_VS_CONTROL_UNDEFINED;
	dev->bFormatIndex = 1;
	dev->bFrameIndex = 1;
	dev->bIntervalIndex = 1;
	dev->fd = open(video_dev, O_RDWR | O_NONBLOCK);
	memcpy(dev->uvc_controls, &default_uvc_controls, sizeof(default_uvc_controls));

	trace("%s=<\n", __func__);
	return dev;
}

void uvcInit(uvc_device *dev)
{
	trace("%s=>\n", __func__);
	struct v4l2_event_subscription s;
#if 0
	uvc_format *format = &uvc_formats[0];
	uvc_frame *frame = &format->frames[0];
	uvc_interval *interval = &frame->intervals[0]; //30fps //uvc updated
#endif

	ioctl(dev->fd, UVCIOC_DISCONNECT, 0);

	dev->entity = 0;
	dev->control = UVC_VS_PROBE_CONTROL;
	verbose("Init: ");
	uvcControlInit(dev, &dev->probe, dev->bFormatIndex, dev->bFrameIndex, dev->bIntervalIndex);
	dev->control = UVC_VS_COMMIT_CONTROL;
	verbose("Init: ");
	uvcControlInit(dev, &dev->commit, dev->bFormatIndex, dev->bFrameIndex, dev->bIntervalIndex);
	dev->control = UVC_VS_CONTROL_UNDEFINED;

	memset(&s, 0, sizeof(s));
	for (int i = UVC_EVENT_FIRST; i <= UVC_EVENT_LAST; i++) {
		s.type = i;
		if (ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &s) < 0) {
			error("VIDIOC_SUBSCRIBE_EVENT failed");
		}
	}

	ioctl(dev->fd, UVCIOC_CONNECT, 0);
	trace("%s=<\n", __func__);
}

void uvcControlInit(uvc_device *dev, struct uvc_streaming_control *ctrl, int formatIndex, int frameIndex,
                    int intervalIndex)
{
	trace("%s=>\n", __func__);
	debug("formatIndex=%d\n", formatIndex);
	debug("frameIndex=%d\n", frameIndex);
	debug("intervalIndex=%d\n", intervalIndex);
#if 0
	uvc_format *format = &uvc_formats[formatIndex - 1];
	uvc_frame *frame = &format->frames[frameIndex - 1];
	uvc_interval *interval = &frame->intervals[intervalIndex - 1];
#else
	struct agtx_webcam_cap_format *format = &g_webcam_cap.formats[formatIndex - 1];
	struct agtx_webcam_cap_frame *frame = &format->frames[frameIndex - 1];
	__u32 interval = frame->intervals[intervalIndex - 1];
#endif

	memset(ctrl, 0, sizeof(*ctrl));

	verbose("Set %s format: ", dev->control == UVC_VS_PROBE_CONTROL ? "PROBE" : "COMMIT");
	switch (format->codec) {
	case AGTX_VENC_TYPE_MJPEG:
		verbose("MJPEG, ");
		break;
	case AGTX_VENC_TYPE_H264:
		verbose("H264, ");
		break;
	default:
		verbose("Unsupported format, ");
		break;
	}
	verbose("res: %dx%d, fps=%d\n", frame->wWidth, frame->wHeight, 10000000 / interval);

	ctrl->bmHint = 1;
	ctrl->bFormatIndex = formatIndex;
	ctrl->bFrameIndex = frameIndex;
	ctrl->dwFrameInterval = interval;
	ctrl->wKeyFrameRate = 0;
	ctrl->wPFrameRate = 0;
	ctrl->wCompQuality = 0;
	ctrl->wCompWindowSize = 0;
	ctrl->wDelay = 0;
	ctrl->dwMaxVideoFrameSize = frame->wWidth * frame->wHeight * 2;
	ctrl->dwMaxPayloadTransferSize = 512;
	ctrl->bmFramingInfo = 0;
	ctrl->bPreferedVersion = 1;
	ctrl->bMaxVersion = 1;
	trace("%s=<\n", __func__);
}

void uvcClose(uvc_device *dev)
{
	trace("%s=>\n", __func__);
	if (dev->fd >= 0) {
		close(dev->fd);
	}
	free(dev);
	dev_for_signal = NULL;
	trace("%s=<\n", __func__);
}

/* ---------------
 * UVC events
 */

void uvcEvents(uvc_device *dev)
{
	trace("%s=>\n", __func__);
	struct v4l2_event v4l2_event;
	struct uvc_request_data req;
	memset(&req, 0, sizeof(req));
	req.length = -EL2HLT;

	if (ioctl(dev->fd, VIDIOC_DQEVENT, &v4l2_event) < 0) {
		error("VIDIOC_DQEVENT failed");
		goto done;
	}

	switch (v4l2_event.type) {
	case UVC_EVENT_CONNECT:
		verbose("UVC_EVENT_CONNECT\n");
		goto done;
	case UVC_EVENT_DISCONNECT:
		verbose("UVC_EVENT_DISCONNECT\n");
		if (streaming) {
			uvcEventStreamOff(dev);
			halt = 1;
		}
		goto done;
	case UVC_EVENT_DATA:
		debug("UVC_EVENT_DATA\n");
		uvcEventData(dev, &((struct uvc_event *)&v4l2_event.u)->data);
		goto done;

	case UVC_EVENT_SETUP:
		debug("UVC_EVENT_SETUP\n");
		uvcEventSetup(dev, &((struct uvc_event *)&v4l2_event.u)->req, &req);
		break;

	case UVC_EVENT_STREAMON:
		verbose("UVC_EVENT_STREAMON\n");
		uvcEventStreamOn(dev);
		break;

	case UVC_EVENT_STREAMOFF:
		verbose("UVC_EVENT_STREAMOFF\n");
		uvcEventStreamOff(dev);
		break;
	}

	if (ioctl(dev->fd, UVCIOC_SEND_RESPONSE, &req) < 0) {
		error("Send response failed");
	}

done:
	trace("%s=<\n", __func__);
	return;
}

void uvcEventData(uvc_device *dev, struct uvc_request_data *data)
{
	trace("%s=>\n", __func__);

	switch (dev->entity) {
	case ENTITY_ZERO:
		debug("ENTITY_ZERO\n");

		struct uvc_streaming_control *target = dev->control == UVC_VS_PROBE_CONTROL ? &dev->probe :
		                                                                              &dev->commit;
		struct uvc_streaming_control *ctrl = (struct uvc_streaming_control *)&data->data;
		int pixel_format = V4L2_PIX_FMT_MJPEG;

		if (dev->control != UVC_VS_PROBE_CONTROL && dev->control != UVC_VS_COMMIT_CONTROL)
			return;

		if (ctrl->bFormatIndex <= 0) {
			verbose("Reset bFormatIndex\n");
			ctrl->bFormatIndex = dev->bFormatIndex;
		}
		if (ctrl->bFrameIndex <= 0) {
			verbose("Reset bFrameIndex\n");
			ctrl->bFrameIndex = dev->bFrameIndex;
		}

#if 0
			uvc_format *format = &uvc_formats[ctrl->bFormatIndex - 1];
			uvc_frame *frame = &format->frames[ctrl->bFrameIndex - 1];
			uvc_interval *interval = &frame->intervals[0];
#else
		struct agtx_webcam_cap_format *format = &g_webcam_cap.formats[ctrl->bFormatIndex - 1];
		struct agtx_webcam_cap_frame *frame = &format->frames[ctrl->bFrameIndex - 1];
		unsigned int interval = frame->intervals[0];
		unsigned int fps = 10000000 / interval;
#endif
		dev->bFormatIndex = ctrl->bFormatIndex;
		dev->bFrameIndex = ctrl->bFrameIndex;
		dev->bIntervalIndex = 1;

		if (ctrl->dwFrameInterval <= 0) {
			verbose("Reset dwFrameInterval\n");
			interval = frame->intervals[dev->bIntervalIndex - 1];
			ctrl->dwFrameInterval = interval;
		} else {
			for (int i = 0; i < frame->bFrameIntervalType; i++) {
				if (ctrl->dwFrameInterval == frame->intervals[i]) {
					interval = frame->intervals[i];
					fps = 10000000 / interval;
					dev->bIntervalIndex = i + 1;
					break;
				}
			}
		}

		verbose("Set %s format: ", dev->control == UVC_VS_PROBE_CONTROL ? "PROBE" : "COMMIT");
		switch (format->codec) {
		case AGTX_VENC_TYPE_MJPEG:
			verbose("MJPEG, ");
			pixel_format = V4L2_PIX_FMT_MJPEG;
			break;
		case AGTX_VENC_TYPE_H264:
			verbose("H264, ");
			pixel_format = V4L2_PIX_FMT_H264;
			break;
		default:
			verbose("Unsupported format, ");
			break;
		}
		verbose("res: %dx%d, fps=%d\n", frame->wWidth, frame->wHeight, fps);

		target->bmHint = 1;
		target->bFormatIndex = ctrl->bFormatIndex;
		target->bFrameIndex = ctrl->bFrameIndex;
		target->dwFrameInterval = interval;
		target->dwMaxVideoFrameSize = frame->wWidth * frame->wHeight * 2;

		if (dev->control == UVC_VS_COMMIT_CONTROL) {
			struct v4l2_format fmt;
			memset(&fmt, 0, sizeof(fmt));
			fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
			fmt.fmt.pix.width = frame->wWidth;
			fmt.fmt.pix.height = frame->wHeight;
			fmt.fmt.pix.pixelformat = pixel_format;
			fmt.fmt.pix.field = V4L2_FIELD_NONE;
			fmt.fmt.pix.sizeimage = 0;
			fmt.fmt.pix.colorspace = V4L2_COLORSPACE_SRGB;
#define MAX(a, b) (a > b ? a : b)
			switch (pixel_format) {
			case V4L2_PIX_FMT_MJPEG:
				fmt.fmt.pix.sizeimage = MAX(frame->wWidth * frame->wHeight / 8,
				                            (int)(BitToByte(format->dwMaxBitRate) / fps * 4));
				break;
			case V4L2_PIX_FMT_H264:
				fmt.fmt.pix.sizeimage =
				        MAX(frame->wWidth * frame->wHeight / 8, (int)(BitToByte(format->dwMaxBitRate)));
				break;
			default:
				break;
			}
			if (ioctl(dev->fd, VIDIOC_S_FMT, &fmt) < 0) {
				error("VIDIOC_S_FMT failed");
			}
		}
		break;
	case INPUT_TERMINAL:
		debug("INPUT_TERMINAL\n");
		break;
	case PROCESSING_UNIT: {
		debug("PROCESSING_UNIT\n");
		ASSERT(dev->control < UVC_MAX_CONTROL_NUM);
		dev->uvc_controls[dev->control].cur = 0;
		for (int i = 0; i < data->length; i++) {
			dev->uvc_controls[dev->control].cur |= (unsigned int)data->data[i] << (i * 8);
		}
		switch (dev->control) {
		case UVC_PU_BRIGHTNESS_CONTROL:
		case UVC_PU_CONTRAST_CONTROL:
		case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
		case UVC_PU_SATURATION_CONTROL:
		case UVC_PU_SHARPNESS_CONTROL:
			update_flag |= UVCD_UPDATE_PU_PART1;
			break;
		case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL:
			update_flag |= UVCD_UPDATE_PU_PART2;
			break;
		case UVC_PU_HUE_CONTROL:
		case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
		case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
			update_flag |= UVCD_UPDATE_PU_PART3;
			break;
		default:
			break;
		}
		if ((update_flag & UVCD_UPDATE_PU_MASK) == UVCD_UPDATE_PU_IDLE) {
			update_flag |= UVCD_UPDATE_PU_PREPARING;
			pthread_create(&threadUpdating, NULL, uvcThreadUpdatePu, (void *)dev);
			pthread_setname_np(threadUpdating, UVCD_THREAD_UPDATE_PU_NAME);
		}
		break;
	}
	case OUTPUT_TERMINAL:
		debug("OUTPUT_TERMINAL\n");
		break;
	default:
		debug("dev->entity=%d\n", dev->entity);
		break;
	}

	trace("%s=<\n", __func__);
}

void uvcEventSetup(uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *req)
{
	trace("%s=>\n", __func__);
	switch (ctrl->bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		debug("USB TYPE: Standard\n");
		break;
	case USB_TYPE_CLASS:
		debug("USB TYPE: Class\n");
		break;
	case USB_TYPE_VENDOR:
		debug("USB TYPE: Vendor\n");
		break;
	case USB_TYPE_RESERVED:
		debug("USB TYPE: Reserved\n");
		break;
	default:
		debug("USB TYPE: Undefined\n");
		break;
	}

	if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS) {
		uvcEventUsbClass(dev, ctrl, req);
	}
	trace("%s=<\n", __func__);
}

void uvcEventUsbClass(uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *req)
{
	trace("%s=>\n", __func__);
	debug("bRequestType: %02x, bRequest: %02x, wValue: %04x, wIndex: %04x, wLength: %04x\n", ctrl->bRequestType,
	      ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);
	switch (ctrl->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		debug("Device\n");
		break;
	case USB_RECIP_INTERFACE:
		debug("Interface\n");
		uvcRecipInterface(dev, ctrl, req);
		break;
	case USB_RECIP_ENDPOINT:
		debug("Endpoint\n");
		break;
	case USB_RECIP_OTHER:
		debug("Others\n");
		break;
	default:
		debug("Unknown\n");
		break;
	}
	trace("%s=<\n", __func__);
}

void uvcRecipInterface(uvc_device *dev, struct usb_ctrlrequest *ctrl, struct uvc_request_data *req)
{
	trace("%s=>\n", __func__);
	switch (ctrl->wIndex & 0x03) {
	case UVC_INTF_CONTROL:
		debug("UVC_INTF_CONTROL\n");
		uvcIntfControl(dev, ctrl->bRequest, ctrl->wIndex >> 8, ctrl->wValue >> 8, ctrl->wLength, req);
		break;
	case UVC_INTF_STREAMING:
		debug("UVC_INTF_STREAMING\n");
		uvcIntfStreaming(dev, ctrl->bRequest, ctrl->wValue >> 8, req);
		break;
	}
	trace("%s=<\n", __func__);
}

char *strControl(int selector)
{
	switch (selector) {
	case UVC_PU_BACKLIGHT_COMPENSATION_CONTROL:
		return "UVC_PU_BACKLIGHT_COMPENSATION_CONTROL\n";
	case UVC_PU_BRIGHTNESS_CONTROL:
		return "UVC_PU_BRIGHTNESS_CONTROL\n";
	case UVC_PU_CONTRAST_CONTROL:
		return "UVC_PU_CONTRAST_CONTROL\n";
	case UVC_PU_GAIN_CONTROL:
		return "UVC_PU_GAIN_CONTROL\n";
	case UVC_PU_POWER_LINE_FREQUENCY_CONTROL:
		return "UVC_PU_POWER_LINE_FREQUENCY_CONTROL\n";
	case UVC_PU_HUE_CONTROL:
		return "UVC_PU_HUE_CONTROL\n";
	case UVC_PU_SATURATION_CONTROL:
		return "UVC_PU_SATURATION_CONTROL\n";
	case UVC_PU_SHARPNESS_CONTROL:
		return "UVC_PU_SHARPNESS_CONTROL\n";
	case UVC_PU_GAMMA_CONTROL:
		return "UVC_PU_GAMMA_CONTROL\n";
	case UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
		return "UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL\n";
	case UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
		return "UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL\n";
	case UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL:
		return "UVC_PU_WHITE_BALANCE_COMPONENT_CONTROL\n";
	case UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
		return "UVC_PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL\n";
	case UVC_PU_DIGITAL_MULTIPLIER_CONTROL:
		return "UVC_PU_DIGITAL_MULTIPLIER_CONTROL\n";
	case UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
		return "UVC_PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL\n";
	case UVC_PU_HUE_AUTO_CONTROL:
		return "UVC_PU_HUE_AUTO_CONTROL\n";
	case UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL:
		return "UVC_PU_ANALOG_VIDEO_STANDARD_CONTROL\n";
	case UVC_PU_ANALOG_LOCK_STATUS_CONTROL:
		return "UVC_PU_ANALOG_LOCK_STATUS_CONTROL\n";
	default:
		debug("selector=%x\n", selector);
		return "Unknown control\n";
	}
}

char *strInput(int selector)
{
	switch (selector) {
	case UVC_CT_SCANNING_MODE_CONTROL:
		return "UVC_CT_SCANNING_MODE_CONTROL\n";
	case UVC_CT_AE_MODE_CONTROL:
		return "UVC_CT_AE_MODE_CONTROL\n";
	case UVC_CT_AE_PRIORITY_CONTROL:
		return "UVC_CT_AE_PRIORITY_CONTROL\n";
	case UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
		return "UVC_CT_EXPOSURE_TIME_ABSOLUTE_CONTROL\n";
	case UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL:
		return "UVC_CT_EXPOSURE_TIME_RELATIVE_CONTROL\n";
	case UVC_CT_FOCUS_ABSOLUTE_CONTROL:
		return "UVC_CT_FOCUS_ABSOLUTE_CONTROL\n";
	case UVC_CT_FOCUS_RELATIVE_CONTROL:
		return "UVC_CT_FOCUS_RELATIVE_CONTROL\n";
	case UVC_CT_FOCUS_AUTO_CONTROL:
		return "UVC_CT_FOCUS_AUTO_CONTROL\n";
	case UVC_CT_IRIS_ABSOLUTE_CONTROL:
		return "UVC_CT_IRIS_ABSOLUTE_CONTROL\n";
	case UVC_CT_IRIS_RELATIVE_CONTROL:
		return "UVC_CT_IRIS_RELATIVE_CONTROL\n";
	case UVC_CT_ZOOM_ABSOLUTE_CONTROL:
		return "UVC_CT_ZOOM_ABSOLUTE_CONTROL\n";
	case UVC_CT_ZOOM_RELATIVE_CONTROL:
		return "UVC_CT_ZOOM_RELATIVE_CONTROL\n";
	case UVC_CT_PANTILT_ABSOLUTE_CONTROL:
		return "UVC_CT_PANTILT_ABSOLUTE_CONTROL\n";
	case UVC_CT_PANTILT_RELATIVE_CONTROL:
		return "UVC_CT_PANTILT_RELATIVE_CONTROL\n";
	case UVC_CT_ROLL_ABSOLUTE_CONTROL:
		return "UVC_CT_ROLL_ABSOLUTE_CONTROL\n";
	case UVC_CT_ROLL_RELATIVE_CONTROL:
		return "UVC_CT_ROLL_RELATIVE_CONTROL\n";
	case UVC_CT_PRIVACY_CONTROL:
		return "UVC_CT_PRIVACY_CONTROL\n";
	default:
		debug("selector=%x\n", selector);
		return "Unknown input\n";
	}
}
void uvcSetInputTerminal(uvc_device *dev, int selector, int length, struct uvc_request_data *response)
{
	trace("%s=>\n", __func__);
	debug(strInput(selector));
	dev->control = selector;
	response->length = length;
	trace("%s=<\n", __func__);
}

void uvcSetProcessingUnit(uvc_device *dev, int selector, int length, struct uvc_request_data *response)
{
	trace("%s=>\n", __func__);
	debug(strControl(selector));
	dev->control = selector;
	response->length = length;
	trace("%s=<\n", __func__);
}

void uvcSetControl(uvc_device *dev, int entity, int selector, int length, struct uvc_request_data *response)
{
	dev->entity = entity;
	switch (entity) {
	default:
	case ENTITY_ZERO:
		debug("ENTITY_ZERO\n");
		break;
	case INPUT_TERMINAL:
		debug("INPUT_TERMINAL\n");
		uvcSetInputTerminal(dev, selector, length, response);
		break;
	case PROCESSING_UNIT:
		debug("PROCESSING_UNIT\n");
		uvcSetProcessingUnit(dev, selector, length, response);
		break;
	case OUTPUT_TERMINAL:
		debug("OUTPUT_TERMINAL\n");
		break;
	}
}

void uvcGetControl(uvc_device *dev, int req, int entity, int selector, int length, struct uvc_request_data *response)
{
	AGTX_UNUSED(entity);

	debug(strControl(selector));
	response->length = length;
	if (selector >= (int)ARRAY_SIZE(dev->uvc_controls))
		return;
	debug("response->length=%d\n", response->length);
	for (int i = 0; i < length; i++) {
		switch (req) {
		case UVC_GET_MIN:
			response->data[i] = (dev->uvc_controls[selector].min >> (i * 8)) & 0xFF;
			break;
		case UVC_GET_MAX:
			response->data[i] = (dev->uvc_controls[selector].max >> (i * 8)) & 0xFF;
			break;
		case UVC_GET_DEF:
			response->data[i] = (dev->uvc_controls[selector].def >> (i * 8)) & 0xFF;
			break;
		case UVC_GET_CUR:
			response->data[i] = (dev->uvc_controls[selector].cur >> (i * 8)) & 0xFF;
			break;
		}
		debug("response->data[%d] = %04x\n", i, response->data[i]);
	}
}

void uvcIntfControl(uvc_device *dev, int req, int entity, int selector, int length, struct uvc_request_data *response)
{
	trace("%s=>\n", __func__);
	switch (req) {
	case UVC_SET_CUR:
		debug("UVC_SET_CUR\n");
		uvcSetControl(dev, entity, selector, length, response);
		break;

	case UVC_GET_MAX:
		debug("UVC_GET_MAX\n");
		uvcGetControl(dev, req, entity, selector, length, response);
		break;
	case UVC_GET_MIN:
		debug("UVC_GET_MIN\n");
		uvcGetControl(dev, req, entity, selector, length, response);
		break;
	case UVC_GET_DEF:
		debug("UVC_GET_DEF\n");
		uvcGetControl(dev, req, entity, selector, length, response);
		break;
	case UVC_GET_CUR:
		debug("UVC_GET_CUR\n");
		uvcGetControl(dev, req, entity, selector, length, response);
		break;

	case UVC_GET_RES:
		debug("UVC_GET_RES\n");
		response->length = 2;
		response->data[0] = 0x1;
		response->data[1] = 0x00;
		break;
	case UVC_GET_LEN:
		debug("UVC_GET_LEN\n");
		/*
			   response->data[0] = 0x02;
			   response->length = 1;
			   */
		break;
	case UVC_GET_INFO:
		debug("UVC_GET_INFO\n");
		response->data[0] = 0x03;
		response->length = 1;
		break;
	}
	trace("%s=<\n", __func__);
}

void uvcIntfStreaming(uvc_device *dev, int req, int selector, struct uvc_request_data *response)
{
	trace("%s=>\n", __func__);
	struct uvc_streaming_control *ctrl;
#if 0
	uvc_format *format;
	uvc_frame *frame;
#endif
	debug("req: %02x, selector: %02x\n", req, selector);

	switch (selector) {
	case UVC_VS_PROBE_CONTROL:
		debug("UVC_VS_PROBE_CONTROL\n");
		break;
	case UVC_VS_COMMIT_CONTROL:
		debug("UVC_VS_COMMIT_CONTROL\n");
		break;
	case UVC_VS_STILL_PROBE_CONTROL:
		debug("UVC_VS_STILL_PROBE_CONTROL\n");
		break;
	case UVC_VS_STILL_COMMIT_CONTROL:
		debug("UVC_VS_STILL_COMMIT_CONTROL\n");
		break;
	case UVC_VS_STILL_IMAGE_TRIGGER_CONTROL:
		debug("UVC_VS_STILL_IMAGE_TRIGGER_CONTROL\n");
		break;
	case UVC_VS_STREAM_ERROR_CODE_CONTROL:
		debug("UVC_VS_STREAM_ERROR_CODE_CONTROL\n");
		break;
	case UVC_VS_GENERATE_KEY_FRAME_CONTROL:
		debug("UVC_VS_GENERATE_KEY_FRAME_CONTROL\n");
		break;
	case UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL:
		debug("UVC_VS_UPDATE_FRAME_SEGMENT_CONTROL\n");
		break;
	case UVC_VS_SYNC_DELAY_CONTROL:
		debug("UVC_VS_SYNC_DELAY_CONTROL\n");
		break;
	default:
		debug("Unknown\n");
		break;
	}

	if (selector != UVC_VS_PROBE_CONTROL && selector != UVC_VS_COMMIT_CONTROL) {
		return;
	}

	ctrl = (struct uvc_streaming_control *)&response->data;
	response->length = 34;

	switch (req) {
	case UVC_SET_CUR:
		verbose("UVC_SET_CUR\n");
		dev->entity = 0;
		dev->control = selector;
		break;

	case UVC_GET_CUR: {
		verbose("UVC_GET_CUR\n");
		if (selector == UVC_VS_PROBE_CONTROL)
			memcpy(ctrl, &dev->probe, sizeof(*ctrl));
		else
			memcpy(ctrl, &dev->commit, sizeof(*ctrl));
#if 0
			uvc_format *format = &uvc_formats[ctrl->bFormatIndex - 1];
			uvc_frame *frame = &format->frames[ctrl->bFrameIndex - 1];
#else
		struct agtx_webcam_cap_format *format = &g_webcam_cap.formats[ctrl->bFormatIndex - 1];
		struct agtx_webcam_cap_frame *frame = &format->frames[ctrl->bFrameIndex - 1];
#endif
		verbose("Get Current format: ");
		switch (format->codec) {
		case AGTX_VENC_TYPE_MJPEG: //V4L2_PIX_FMT_MJPEG:
			verbose("MJPEG, ");
			break;
		case AGTX_VENC_TYPE_H264: //V4L2_PIX_FMT_H264:
			verbose("H264, ");
			break;
		default:
			verbose("Not Supported, ");
			break;
		}
		verbose("res: %dx%d, fps=%d\n", frame->wWidth, frame->wHeight, 10000000 / ctrl->dwFrameInterval);
	} break;

	case UVC_GET_MIN:
		verbose("UVC_GET_MIN\n");
#if 0
			format = &uvc_formats[dev->bFormatIndex - 1];
			frame = &format->frames[dev->bFrameIndex - 1];
#endif
		uvcControlInit(dev, ctrl, dev->bFormatIndex, dev->bFrameIndex, 1);
		break;

	case UVC_GET_MAX:
		verbose("UVC_GET_MAX\n");
#if 0
			format = &uvc_formats[dev->bFormatIndex - 1];
			frame = &format->frames[dev->bFrameIndex - 1];
#else
		struct agtx_webcam_cap_format *format = &g_webcam_cap.formats[dev->bFormatIndex - 1];
		struct agtx_webcam_cap_frame *frame = &format->frames[dev->bFrameIndex - 1];
#endif
		uvcControlInit(dev, ctrl, dev->bFormatIndex, dev->bFrameIndex, frame->bFrameIntervalType);
		break;

	case UVC_GET_DEF:
		verbose("UVC_GET_DEF\n");
#if 0
			format = &uvc_formats[dev->bFormatIndex - 1];
			frame = &format->frames[dev->bFrameIndex - 1];
#endif
		uvcControlInit(dev, ctrl, dev->bFormatIndex, dev->bFrameIndex, 1);
		break;

	case UVC_GET_RES:
		verbose("UVC_GET_RES\n");
		memset(ctrl, 0, sizeof(*ctrl));
		break;

	case UVC_GET_LEN:
		verbose("UVC_GET_LEN\n");
		response->data[0] = 0x00;
		response->data[1] = 0x22;
		response->length = 2;
		break;

	case UVC_GET_INFO:
		verbose("UVC_GET_INFO\n");
		response->data[0] = 0x03;
		response->length = 1;
		break;
	}
	trace("%s=<\n", __func__);
}

void uvcMmap(uvc_device *dev, int count)
{
	trace("%s=>\n", __func__);
	struct v4l2_requestbuffers requestbufs;

	memset(&requestbufs, 0, sizeof(requestbufs));
	requestbufs.count = count;
	requestbufs.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	requestbufs.memory = V4L2_MEMORY_MMAP;
	if (ioctl(dev->fd, VIDIOC_REQBUFS, &requestbufs) < 0) {
		error("VIDIOC_REQBUFS failed");
		goto failed;
	}
	dev->buf_count = requestbufs.count;

	dev->buf = malloc(dev->buf_count * sizeof(void *));
	for (int i = 0; (unsigned)i < dev->buf_count; ++i) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		if (ioctl(dev->fd, VIDIOC_QUERYBUF, &buf) < 0) {
			error("VIDIOC_QUERYBUF failed");
			goto failed;
		}
		dev->buf[i] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, buf.m.offset);
		if (dev->buf[i] == MAP_FAILED) {
			error("mmap failed");
			goto failed;
		}
		dev->buf_size = buf.length;
	}
	trace("%s=<\n", __func__);
	return;

failed:
	uvcMunmap(dev);
}

void uvcMunmap(uvc_device *dev)
{
	trace("%s=>\n", __func__);
	for (int i = 0; (unsigned)i < dev->buf_count; ++i) {
		if (dev->buf[i] != MAP_FAILED) {
			if (munmap(dev->buf[i], dev->buf_size) < 0) {
				error("munmap failed");
			}
		}
	}

	if (dev->buf != NULL) {
		free(dev->buf);
		dev->buf = NULL;
	}

	dev->buf_count = 0;
	trace("%s=<\n", __func__);
}

void *uvcThreadUpdateVideoOptions(void *ptr)
{
	int ret;

	ret = ccSet((char *)ptr);
	if (ret < 0) {
		error("ccSet");
	}
	return NULL;
}

void *uvcThreadUpdatePu(void *ptr)
{
	uvc_device *dev = (uvc_device *)ptr;
	int ret;
	int anti_flicker = 0;
	short r_gain = 50;
	short b_gain = 50;
	short tmp_gain = 0;
	char jstr2cc[JSON_STR_LEN];
	uvc_control *pu = dev->uvc_controls;

	trace("%s=>\n", __func__);

renew_pu:
	update_flag |= UVCD_UPDATE_PU_UPDATING;

	if (update_flag & UVCD_UPDATE_PU_PART1) {
		update_flag &= ~UVCD_UPDATE_PU_PART1;
		/* Brightness, Satuartion, Contrast, Sharpness, Anti_flicker */
		switch (pu[UVC_PU_POWER_LINE_FREQUENCY_CONTROL].cur) {
		case 0:
			anti_flicker = 2; // Auto anti-flicker
			break;
		case 1:
			anti_flicker = 0; // 50Hz
			break;
		case 2:
			anti_flicker = 1; // 60HZ
			break;
		default:
			assert(0);
			break;
		}
		sprintf(jstr2cc,
		        "\"brightness\":%d, \"saturation\":%d, \"contrast\":%d, \"sharpness\":%d, \"anti_flicker\":%d,  \"cmd_id\":%d",
		        pu[UVC_PU_BRIGHTNESS_CONTROL].cur, pu[UVC_PU_SATURATION_CONTROL].cur,
		        pu[UVC_PU_CONTRAST_CONTROL].cur, pu[UVC_PU_SHARPNESS_CONTROL].cur, anti_flicker,
		        AGTX_CMD_IMG_PREF);

		ret = ccSet(jstr2cc);
		if (ret < 0) {
			error("ccSet");
		}
	}

	if (update_flag & UVCD_UPDATE_PU_PART2) {
		update_flag &= ~UVCD_UPDATE_PU_PART2;
		/* Backlight_compensation */
		sprintf(jstr2cc, "\"backlight_compensation\":%d, \"cmd_id\":%d",
		        pu[UVC_PU_BACKLIGHT_COMPENSATION_CONTROL].cur, AGTX_CMD_ADV_IMG_PREF);

		ret = ccSet(jstr2cc);
		if (ret < 0) {
			error("ccSet");
		}
	}

	if (update_flag & UVCD_UPDATE_PU_PART3) {
		update_flag &= ~UVCD_UPDATE_PU_PART3;
		/* White balance, Hue (Color tone) */
		tmp_gain = (short)pu[UVC_PU_HUE_CONTROL].cur * 5 / 9;
		b_gain = tmp_gain + 50;
		r_gain = 100 - b_gain;

		sprintf(jstr2cc, "\"mode\":%d, \"color_temp\":%d, \"r_gain\":%d, \"b_gain\":%d, \"cmd_id\":%d",
		        pu[UVC_PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL].cur,
		        pu[UVC_PU_WHITE_BALANCE_TEMPERATURE_CONTROL].cur, r_gain, b_gain, AGTX_CMD_AWB_PREF);

		ret = ccSet(jstr2cc);
		if (ret < 0) {
			error("ccSet");
		}
	}

	if (update_flag & UVCD_UPDATE_PU_PART_MASK) {
		goto renew_pu;
	}

	update_flag &= ~UVCD_UPDATE_PU_MASK;
	trace("%s=<\n", __func__);
	return NULL;
}

void *uvcThreadVideoStreaming(void *ptr)
{
	uvc_device *dev = (uvc_device *)ptr;
	trace("%s=>\n", __func__);

	while (streaming) {
		struct v4l2_buffer buf;

		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		if (ioctl(dev->fd, VIDIOC_DQBUF, &buf) < 0) {
			//error("VIDIOC_DQBUF failed");
			continue;
		}

		fillVideoBuffer(dev, &buf);

		if (ioctl(dev->fd, VIDIOC_QBUF, &buf) < 0) {
			error("VIDIOC_QBUF failed");
		}
	}

	trace("%s=<\n", __func__);
	return NULL;
}

void uvcEventStreamOn(uvc_device *dev)
{
	trace("%s=>\n", __func__);
#if 0
	int venc_type = 0;
	uvc_format *format = &uvc_formats[dev->bFormatIndex - 1];
	uvc_frame *frame = &format->frames[dev->bFrameIndex - 1];
	int fps = 10000000 / frame->intervals[dev->bIntervalIndex - 1].interval;
#else
	struct agtx_webcam_cap_format *format = &g_webcam_cap.formats[dev->bFormatIndex - 1];
	struct agtx_webcam_cap_frame *frame = &format->frames[dev->bFrameIndex - 1];
	unsigned int fps = 10000000 / frame->intervals[dev->bIntervalIndex - 1];
	unsigned int bitrate = 0;
	char jstr2cc[JSON_STR_LEN];
#endif

	/* 3. Request buffers */
	uvcMmap(dev, 3);

	/* 5. Tell driver streamon */
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(dev->fd, VIDIOC_STREAMON, &type) < 0) {
		error("VIDIOC_STREAMON failed");
	}

	/* 1. Set setting via ccserver */
	streaming = 1;

	bitrate = BitToKBit(format->dwMaxBitRate);

	PREPARE_VIDEO(jstr2cc, frame->wWidth, frame->wHeight, fps, format->codec, bitrate, AGTX_CMD_VIDEO_STRM_CONF);

	if (ccSet(jstr2cc) < 0) {
		error("cccSet failed\n");
	}

	/* 2. Init bit stream system */
	if (initBitstreamSystem() != MPI_SUCCESS) {
		error("Bitstream System init fail.");
	}

	/* 4. Init buffers */
	for (int i = 0; (unsigned)i < dev->buf_count; ++i) {
		struct v4l2_buffer buf;
		memset(&buf, 0, sizeof(buf));
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		fillVideoBuffer(dev, &buf);
		if (ioctl(dev->fd, VIDIOC_QBUF, &buf) < 0) {
			error("VIDIOC_QBUF failed");
		}
	}

	pthread_create(&threadStreaming, NULL, uvcThreadVideoStreaming, (void *)dev);
	pthread_setname_np(threadStreaming, UVCD_THREAD_STREAM_NAME);
	trace("%s=<\n", __func__);
	return;
}

void uvcEventStreamOff(uvc_device *dev)
{
	trace("%s=>\n", __func__);
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;

	if (ioctl(dev->fd, VIDIOC_STREAMOFF, &type) < 0) {
		error("VIDIOC_STREAMOFF failed");
	}
	streaming = 0;
	pthread_join(threadStreaming, NULL);
	for (int i = 0; (unsigned)i < dev->buf_count; ++i) {
		struct v4l2_buffer buf;

		memset(&buf, 0, sizeof(buf));
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		ioctl(dev->fd, VIDIOC_DQBUF, &buf);
	}
	destroyBitstreamSystem();
	uvcMunmap(dev);
	dev->entity = 0;
	dev->control = UVC_VS_CONTROL_UNDEFINED;
	dev->bFormatIndex = 1;
	dev->bFrameIndex = 1;
	dev->bIntervalIndex = 1;
	trace("%s=<\n", __func__);
}

int uvcParseResOption(int cmdId)
{
	AGTX_UNUSED(cmdId);

	int ret;
	int i, j, k;
	int frame_num = 0;
	int interval_num = 0;
	char jstr[JSON_STR_LEN];
	struct json_object *json_obj = NULL;
	struct json_tokener *json_tok = NULL;
	enum json_tokener_error json_err;
	AGTX_RES_OPTION_S data;

	sprintf(jstr, "\"cmd_id\":%d", AGTX_CMD_RES_OPTION);
	ret = ccGet(jstr);

	if (ret != 0) {
		return -1;
	}

	/* parse data */
	json_tok = json_tokener_new();
	json_obj = json_tokener_parse_ex(json_tok, jstr, strlen(jstr));
	json_err = json_tokener_get_error(json_tok);
	if (json_err == json_tokener_success) {
		parse_res_option(&data, json_obj);
		if (json_obj != NULL) {
			json_object_put(json_obj);
		}
	}
	json_tokener_free(json_tok);

	/* Assamble AgtxWebcamCapFrame from data */
	i = 0;
	if (data.strm[i].res_idx == -1) {
		for (j = 0; j < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; j++) {
			if (data.strm[i].res[j].width != 0 && data.strm[i].res[j].height != 0) {
				++frame_num;
			}
		}

		g_webcam_cap_frames = malloc(sizeof(AgtxWebcamCapFrame) * frame_num);

		for (j = 0; j < frame_num; j++) {
			interval_num = 10;
			g_webcam_cap_frames[j].wWidth = data.strm[i].res[j].width;
			g_webcam_cap_frames[j].wHeight = data.strm[i].res[j].height;
			// for (k = 0; k < data.strm[i].res[j].max_frame_rate[0]; k++) {
			// 	if (data.strm[i].res[j].frame_rate_list[k] > 0) {
			// 		if (k >= 1) {
			// 			if (data.strm[i].res[j].frame_rate_list[k] < data.strm[i].res[j].frame_rate_list[k-1]) {
			// 				++interval_num;
			// 			} else {
			// 				break;
			// 			}
			// 		} else {
			// 			++interval_num;
			// 		}
			// 	} else {
			// 		break;
			// 	}
			// }
			g_webcam_cap_frames[j].bFrameIntervalType = interval_num;
			g_webcam_cap_frames[j].intervals = malloc(sizeof(unsigned int) * interval_num);
			for (k = 0; k < interval_num; k++) {
				g_webcam_cap_frames[j].intervals[k] = 10000000 / data.strm[i].res[j].frame_rate_list[k];
			}
		}
	} else {
		g_webcam_cap_frames = NULL;
	}

	/* Check same resolution */
	for (i = 0; i < frame_num; i++) {
		for (j = i + 1; j < frame_num; j++) {
			if ((g_webcam_cap_frames[i].wWidth == g_webcam_cap_frames[j].wWidth) &&
			    (g_webcam_cap_frames[i].wHeight == g_webcam_cap_frames[j].wHeight)) {
				printf("Error: Duplicated resolution %dx%d found in database(AGTX_CMD_RES_OPTION).\n",
				       g_webcam_cap_frames[i].wWidth, g_webcam_cap_frames[i].wHeight);
				return -1;
			}
		}
	}

	return frame_num;
}

int uvcParseVencOption(int cmdId, int frame_num)
{
	AGTX_UNUSED(cmdId);

	int ret;
	int i, j, k;
	int format_num = 0;
	char jstr[JSON_STR_LEN];
	struct json_object *json_obj = NULL;
	struct json_tokener *json_tok = NULL;
	enum json_tokener_error json_err;
	AGTX_VENC_OPTION_S data;
	int h264_collected = 0;
	int mjpeg_collected = 0;

	sprintf(jstr, "\"cmd_id\":%d", AGTX_CMD_VENC_OPTION);
	ret = ccGet(jstr);

	if (ret != 0) {
		return -1;
	}

	/* parse data */
	json_tok = json_tokener_new();
	json_obj = json_tokener_parse_ex(json_tok, jstr, strlen(jstr));
	json_err = json_tokener_get_error(json_tok);
	if (json_err == json_tokener_success) {
		parse_venc_option(&data, json_obj);
		if (json_obj != NULL) {
			json_object_put(json_obj);
		}
	}
	json_tokener_free(json_tok);

	/* Assamble AgtxWebcamCapFrame from data */
	i = 0;
	if (data.strm[i].venc_idx == -1) {
		for (j = 0; j < MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE; j++) {
			if (data.strm[i].venc[j].codec == AGTX_VENC_TYPE_H264) {
				if (h264_collected > 0) {
					/* Skip more than 1 found H264 */
					continue;
				}
				h264_collected++;
			} else if (data.strm[i].venc[j].codec == AGTX_VENC_TYPE_MJPEG) {
				if (mjpeg_collected > 0) {
					/* Skip more than 1 found MJPEG */
					continue;
				}
				mjpeg_collected++;
			} else {
				continue;
			}
			++format_num;
		}

		g_webcam_cap_formats = malloc(sizeof(AgtxWebcamCapFormat) * format_num);
		k = 0;
		h264_collected = 0;
		mjpeg_collected = 0;
		for (j = 0; j < MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE; j++) {
			if (data.strm[i].venc[j].codec == AGTX_VENC_TYPE_H264) {
				if (h264_collected > 0) {
					/* Skip more than 1 found H264 */
					continue;
				}
				h264_collected++;
			} else if (data.strm[i].venc[j].codec == AGTX_VENC_TYPE_MJPEG) {
				if (mjpeg_collected > 0) {
					/* Skip more than 1 found MJPEG */
					continue;
				}
				mjpeg_collected++;
			} else {
				/* Skip unsupported codec */
				continue;
			}

			if ((data.strm[i].venc[j].min_bit_rate == 0) || (data.strm[i].venc[j].max_bit_rate == 0)) {
				/* Invalid value, skip collecting target venc option */
				continue;
			}

			/* Collect data survived from error checking */
			g_webcam_cap_formats[k].codec = data.strm[i].venc[j].codec;
			g_webcam_cap_formats[k].dwMinBitRate = KBitToBit(data.strm[i].venc[j].min_bit_rate);
			g_webcam_cap_formats[k].dwMaxBitRate = KBitToBit(data.strm[i].venc[j].max_bit_rate);
			g_webcam_cap_formats[k].bNumFrameDescriptors = frame_num;
			g_webcam_cap_formats[k].frames = g_webcam_cap_frames;
			k++;
		}
	} else {
		g_webcam_cap_formats = NULL;
	}

	return format_num;
}

int uvcRetrieveCap(void)
{
	int frame_num = 0;
	int format_num = 0;

	frame_num = uvcParseResOption(AGTX_CMD_RES_OPTION);
	if (frame_num <= 0) {
		return frame_num;
	}

	format_num = uvcParseVencOption(AGTX_CMD_VENC_OPTION, frame_num);

	if (g_webcam_cap_frames != NULL && g_webcam_cap_formats != NULL) {
		g_webcam_cap.bNumFormats = format_num;
		g_webcam_cap.formats = g_webcam_cap_formats;
	} else {
		g_webcam_cap.bNumFormats = 0;
		g_webcam_cap.formats = NULL;
	}

#ifdef DEBUG
	{
		int i, j, k;
		AgtxWebcamCapFormat *format;
		AgtxWebcamCapFrame *frame;
		for (i = 0; i < g_webcam_cap.bNumFormats; i++) {
			format = g_webcam_cap.formats[i];
			printf("Format %d:\n", i);
			printf("\tcodec                = %d\n", format.codec);
			printf("\tdwMinBitRate         = %d\n", format.dwMinBitRate);
			printf("\tdwMaxBitRate         = %d\n", format.dwMaxBitRate);
			printf("\tbNumFrameDescriptors = %d\n", format.bNumFrameDescriptors);
			for (j = 0; j < format->bNumFrameDescriptors; j++) {
				frame = format->frames[j];
				printf("\tFrame %d:\n", j);
				printf("\t\twWidth %d\n", frame->wWidth);
				printf("\t\twHeight %d\n", frame->wHeight);
				printf("\t\tbFrameIntervalType %d\n", frame->bFrameIntervalType);
			}
		}
	}
#endif

	return 0;
}

void uvcFreeCap(void)
{
	int i, j;

	if (g_webcam_cap.bNumFormats != 0) {
		for (i = 0; i < g_webcam_cap.bNumFormats; i++) {
			if (g_webcam_cap.formats[i].bNumFrameDescriptors != 0) {
				for (j = 0; j < g_webcam_cap.formats[i].bNumFrameDescriptors; j++) {
					if (g_webcam_cap.formats[i].frames[j].bFrameIntervalType != 0) {
						free(g_webcam_cap_frames[j].intervals);
						g_webcam_cap.formats[i].frames[j].bFrameIntervalType = 0;
						g_webcam_cap.formats[i].frames[j].intervals = NULL;
					}
				}
			}
			g_webcam_cap.formats[i].bNumFrameDescriptors = 0;
			g_webcam_cap.formats[i].frames = NULL;
		}
	}

	free(g_webcam_cap_frames);
	free(g_webcam_cap_formats);
	g_webcam_cap.bNumFormats = 0;
	g_webcam_cap.formats = NULL;
}

static int checkMultipleInstances(char *app_name)
{
	int lock_fd;
	char *lock_name = LOCK_NAME;

	/* Check multiple instances */
	lock_fd = open(lock_name, O_CREAT | O_RDWR, 0666);
	if (-1 == lock_fd) {
		perror("Cannot open lock file");
		return -EINVAL;
	}

	if (-1 == flock(lock_fd, LOCK_EX | LOCK_NB)) {
		printf("%s: Failed to lock %s. Multiple instance detected.\n", app_name, lock_name);
		close(lock_fd);
		return -EBUSY;
	}

	return 0;
}

static void handleSigInt(int signo)
{
	AGTX_UNUSED(signo);

	if (dev_for_signal) {
		if (streaming) {
			uvcEventStreamOff(dev_for_signal);
		}
		uvcClose(dev_for_signal);
	}

	ioctl(g_agtx_webcam_fd, AGTX_WEBCAM_IOCTL_UNBIND, NULL);
	close(g_agtx_webcam_fd);
	uvcFreeCap();
	closeCC();

	exit(0);
}

int main(int argc, char *argv[])
{
	AGTX_UNUSED(argc);

	int ret;
	uvc_device *dev;
	fd_set master;

	signal(SIGINT, handleSigInt);

	/* Check multiple instances */
	ret = checkMultipleInstances(argv[0]);
	if (ret != 0) {
		return ret;
	}

	openCC();
#ifdef CONFIG_WEBCAM_CAP_FIXED
#else
	if (uvcRetrieveCap() < 0) {
		printf("Failed to retrieve capability from database.\n");
		return -1;
	}
#endif

	g_agtx_webcam_fd = open(AGTX_WEBCAM_DEV_NODE, O_RDWR);
	if (g_agtx_webcam_fd < 0) {
		printf("Failed to open %s\n", AGTX_WEBCAM_DEV_NODE);
		uvcFreeCap();
		closeCC();
		return -1;
	}

	/* Set AGTX webcam driver capability*/
	ret = ioctl(g_agtx_webcam_fd, AGTX_WEBCAM_IOCTL_CAP_SET, &g_webcam_cap);
	if (ret) {
		printf("Failed to set cap: %d\n", errno);
		uvcFreeCap();
		closeCC();
		return -1;
	}

	/* bind AGTX webcam driver */
	ret = ioctl(g_agtx_webcam_fd, AGTX_WEBCAM_IOCTL_BIND, NULL);
	if (ret) {
		printf("Failed to bind: %d\n", errno);
		uvcFreeCap();
		closeCC();
		return -1;
	}

restart:
	dev = uvcOpen("/dev/video0");

	if (dev->fd < 0) {
		error("Open UVC device(/dev/video0) failed.\n");
		goto restart;
	}

	uvcInit(dev);

	dev_for_signal = dev;

	FD_ZERO(&master);
	FD_SET(dev->fd, &master);

	while (1) {
		fd_set efds = master;

		select(dev->fd + 1, NULL, NULL, &efds, NULL);
		if (FD_ISSET(dev->fd, &efds)) {
			uvcEvents(dev);
		}
		if (halt) {
			halt = 0;
			closeCC();
			uvcClose(dev);
			goto restart;
		}
	}

	uvcClose(dev);

	ret = ioctl(g_agtx_webcam_fd, AGTX_WEBCAM_IOCTL_UNBIND, NULL);
	if (ret) {
		printf("Failed to unbind: %d\n", errno);
	}

	close(g_agtx_webcam_fd);
	uvcFreeCap();
	closeCC();

	return ret;
}
