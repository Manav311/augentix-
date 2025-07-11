#ifndef EVENT_H
#define EVENT_H

#include "agtx_cmd.h"
#include "agtx_types.h"
#include "agtx_common.h"
#include "agtx_gpio_conf.h"
#include "agtx_event_conf.h"
#include "agtx_event_param.h"
#include "agtx_video.h"
#include "agtx_pwm_conf.h"
#include "led.h"

#define EVT_NAME "EVT_MAIN"

#define EVENT_DEBUG (0)

#if EVENT_DEBUG
#define EVT_TRACE(x, ...) fprintf(stderr, "[EVENT][DEBUG] " x, ##__VA_ARGS__)
#else
#define EVT_TRACE(x, ...)
#endif

#define EVT_ERR(x, ...) fprintf(stderr, "[EVENT][ERROR] %s(): " x, __func__, ##__VA_ARGS__)
#define EVT_WARN(x, ...) fprintf(stderr, "[EVENT][WARNING] %s(): " x, __func__, ##__VA_ARGS__)
#define EVT_NOTICE(x, ...) fprintf(stderr, "\n[EVENT][NOTICE] %s(): " x, __func__, ##__VA_ARGS__)
#define EVT_INFO(x, ...) fprintf(stderr, "[EVENT][INFO] " x, ##__VA_ARGS__)

typedef int (*AGTX_EVENT_ACTION_CB)(void *action_args, void *rsv);

AGTX_INT32 AGTX_EVENT_print(void *action_args, void *rsv);
AGTX_INT32 AGTX_EVENT_execCmd(void *action_args, void *rsv);
AGTX_INT32 AGTX_EVENT_parseString(void *action_args, void *rsv);

#endif /* EVENT_H */
