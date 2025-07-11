#ifndef AGTX_DIP_DMS_CONF_H_
#define AGTX_DIP_DMS_CONF_H_

#include "agtx_types.h"
struct json_object;

typedef enum { AGTX_DMS_MODE_DMS_DEFAULT, AGTX_DMS_MODE_DMS_ISO } AGTX_DMS_MODE_E;

#define MAX_AGTX_DIP_DMS_CONF_S_AUTO_G_AT_M_INTER_RATIO_LIST_SIZE 11
#define MAX_AGTX_DIP_DMS_CONF_S_AUTO_M_AT_G_INTER_RATIO_LIST_SIZE 11
#define MAX_AGTX_DIP_DMS_CONF_S_AUTO_M_AT_M_INTER_RATIO_LIST_SIZE 11

typedef struct {
	AGTX_INT32 auto_g_at_m_inter_ratio_list[MAX_AGTX_DIP_DMS_CONF_S_AUTO_G_AT_M_INTER_RATIO_LIST_SIZE];
	AGTX_INT32 auto_m_at_g_inter_ratio_list[MAX_AGTX_DIP_DMS_CONF_S_AUTO_M_AT_G_INTER_RATIO_LIST_SIZE];
	AGTX_INT32 auto_m_at_m_inter_ratio_list[MAX_AGTX_DIP_DMS_CONF_S_AUTO_M_AT_M_INTER_RATIO_LIST_SIZE];
	AGTX_DMS_MODE_E dms_ctrl_method;
	AGTX_INT32 manual_g_at_m_inter_ratio;
	AGTX_INT32 manual_m_at_g_inter_ratio;
	AGTX_INT32 manual_m_at_m_inter_ratio;
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_DMS_CONF_S;

#endif /* AGTX_DIP_DMS_CONF_H_ */
