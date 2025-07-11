/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/


#ifndef AGTX_SYS_H
#define AGTX_SYS_H


#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include "agtx_types.h"
#include "agtx_common.h"


typedef enum {
	AGTX_TIME_FMT_YYYYMMDD,
	AGTX_TIME_FMT_MMDDYYYY,
	AGTX_TIME_FMT_DDMMYYYY,
	AGTX_TIME_FMT_NUM,
} AGTX_TIME_FMT_E;

typedef enum {
	AGTX_TIME_MODE_MANUAL,
	AGTX_TIME_MODE_SYNC_NTP,
	AGTX_TIME_MODE_SYNC_PC,
	AGTX_TIME_MODE_NUM,
} AGTX_TIME_MODE_E;


typedef struct {
	AGTX_TIME_FMT_E   fmt;
	AGTX_TIME_MODE_E  mode;
	AGTX_DATE_S       date;
	AGTX_TIME_S       time;
	AGTX_INT8         time_zone;
	AGTX_UINT8        ntp[AGTX_MAX_NTP_SERVER_NUM][AGTX_MAX_STR_LEN];
} AGTX_TIME_INFO_S;


#ifdef __cplusplus
}
#endif /* __cplusplus */


#endif /* !AGTX_SYS_H */
