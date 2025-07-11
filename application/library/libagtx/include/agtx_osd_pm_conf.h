#ifndef AGTX_OSD_PM_CONF_H_
#define AGTX_OSD_PM_CONF_H_

#include "agtx_types.h"
struct json_object;

#define MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE 4
#define MAX_AGTX_OSD_PM_CONF_S_CONF_SIZE 4

typedef struct {
	AGTX_INT32 alpha;
	AGTX_INT32 color;
	AGTX_INT32 enabled;
	AGTX_INT32 end_x; /* Start coordinates of detection region. (in percentage) */
	AGTX_INT32 end_y; /* Start coordinates of detection region. (in percentage) */
	AGTX_INT32 start_x; /* Start coordinates of detection region. (in percentage) */
	AGTX_INT32 start_y; /* Start coordinates of detection region. (in percentage) */
} AGTX_OSD_PM_S;

typedef struct {
	AGTX_OSD_PM_S param[MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE];
} AGTX_OSD_PM_PARAM_S;

typedef struct {
	AGTX_OSD_PM_PARAM_S conf[MAX_AGTX_OSD_PM_CONF_S_CONF_SIZE];
} AGTX_OSD_PM_CONF_S;


#endif /* AGTX_OSD_PM_CONF_H_ */
