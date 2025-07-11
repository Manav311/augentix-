#ifndef UVCD_COMMON_H_
#define UVCD_COMMON_H_

#include <stdio.h>
#include <linux/usb/video.h>
#include <uvc.h>

#define error perror

#define VERBOSE
#ifdef VERBOSE
#define verbose(...) printf(__VA_ARGS__)
#else
#define verbose(...)
#endif

//#define TRACE
#ifdef TRACE
#define trace(...) printf(__VA_ARGS__)
#else
#define trace(...)
#endif

//#define DEBUG
#ifdef DEBUG
#define debug(...) printf(__VA_ARGS__)
#define DBG(x,...) fprintf(stderr,"%s() in %s: " x,__func__, __FILE__,##__VA_ARGS__)
#else
#define debug(args...)
#define DBG(x,...)
#endif

#define DBG_LOW(x,...) DBG(x,##__VA_ARGS__)
#define DBG_MED(x,...) DBG(x,##__VA_ARGS__)
#define DBG_HIGH(x,...) DBG(x,##__VA_ARGS__)

#define ASSERT(x) assert(x)

#define ARRAY_SIZE(element) ((sizeof(element) / sizeof(element[0])))

typedef struct _uvc_control {
	int min;
	int max;
	int def;
	int cur;
} uvc_control;

typedef struct _uvc_device {
	int fd;
	int entity;
	int control;
	int bFormatIndex;
	int bFrameIndex;
	int bIntervalIndex;
	void **buf;
	unsigned int buf_count;
	unsigned int buf_size;
	struct uvc_streaming_control probe;
	struct uvc_streaming_control commit;
#define UVC_MAX_CONTROL_NUM 19
	uvc_control uvc_controls[UVC_MAX_CONTROL_NUM];
} uvc_device;

#endif /* UVCD_COMMON_H_ */
