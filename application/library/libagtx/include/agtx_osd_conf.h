/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_OSD_CONF_H
#define AGTX_OSD_CONF_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


#define MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE    257
#define MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE       4
#define MAX_AGTX_OSD_CONF_S_STRM_SIZE               4


typedef enum {
	AGTX_OSD_TYPE_TEXT,
	AGTX_OSD_TYPE_IMAGE,
	AGTX_OSD_TYPE_INFO
} AGTX_OSD_TYPE_E;


typedef struct {
	AGTX_INT32 enabled;
	AGTX_INT32 start_x; /* Start coordinates of detection region. (in percentage) */
	AGTX_INT32 start_y; /* Start coordinates of detection region. (in percentage) */
	AGTX_OSD_TYPE_E type;
	AGTX_UINT8 type_spec[MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE];
} AGTX_OSD_CONF_INNER_S;

typedef struct {
	AGTX_OSD_CONF_INNER_S region[MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE];
} AGTX_OSD_CONF_OUTER_S;

typedef struct {
	AGTX_OSD_CONF_OUTER_S strm[MAX_AGTX_OSD_CONF_S_STRM_SIZE];
	AGTX_INT32 showWeekDay;
} AGTX_OSD_CONF_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_OSD_CONF_H */
