/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_EVENT_PARAM_H_
#define AGTX_EVENT_PARAM_H_


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


#define MAX_AGTX_EVENT_ATTR_S_NAME_SIZE 129
#define MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE 10

typedef struct {
	AGTX_INT32 enabled; /* Process GPIO/SW event list only when enabled. */
	AGTX_UINT8 name[MAX_AGTX_EVENT_ATTR_S_NAME_SIZE]; /* Event name. */
} AGTX_EVENT_ATTR_S;

typedef struct {
	AGTX_EVENT_ATTR_S event_attr[MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE];
} AGTX_EVENT_PARAM_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_EVENT_PARAM_H_ */
