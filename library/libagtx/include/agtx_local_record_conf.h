#ifndef AGTX_LOCAL_RECORD_CONF_H_
#define AGTX_LOCAL_RECORD_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum {
	AGTX_RECORD_MODE_event,
	AGTX_RECORD_MODE_continuous
} AGTX_RECORD_MODE_E;


typedef struct {
	AGTX_INT32 enabled; /* Enable recording to local storage. */
	AGTX_INT32 max_event_time; /* Max duration of a event. (unit: second) */
	AGTX_INT32 min_event_time; /* Min duration of a event. (unit: second) */
	AGTX_RECORD_MODE_E mode; /* Modes of recording. */
	AGTX_INT32 post_record_time; /* Post-recording time before event trigger. (unit: second) */
	AGTX_INT32 pre_record_time; /* Pre-recording time before event trigger. (unit: second) */
} AGTX_LOCAL_RECORD_CONF_S;


#endif /* AGTX_LOCAL_RECORD_CONF_H_ */
