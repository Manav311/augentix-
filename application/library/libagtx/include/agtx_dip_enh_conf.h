/******************************************************************************
*
* opyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef AGTX_DIP_ENH_CONF_H_
#define AGTX_DIP_ENH_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "agtx_types.h"
#include "agtx_common.h"

#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_EDGE_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_RADIUS_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_STRENGTH_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_DETAIL_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_EDGE_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_STRENGTH_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_DETAIL_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_EDGE_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_RADIUS_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_STRENGTH_LIST_SIZE 11
#define MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_WEIGHT_LIST_SIZE 11

typedef struct {
	AGTX_INT32 auto_c_edge_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_EDGE_LIST_SIZE];
	AGTX_INT32 auto_c_radius_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_RADIUS_LIST_SIZE];
	AGTX_INT32 auto_c_strength_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_STRENGTH_LIST_SIZE];
	AGTX_INT32 auto_y_txr_detail_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_DETAIL_LIST_SIZE];
	AGTX_INT32 auto_y_txr_edge_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_EDGE_LIST_SIZE];
	AGTX_INT32 auto_y_txr_strength_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_STRENGTH_LIST_SIZE];
	AGTX_INT32 auto_y_zone_detail_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_DETAIL_LIST_SIZE];
	AGTX_INT32 auto_y_zone_edge_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_EDGE_LIST_SIZE];
	AGTX_INT32 auto_y_zone_radius_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_RADIUS_LIST_SIZE];
	AGTX_INT32 auto_y_zone_strength_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_STRENGTH_LIST_SIZE];
	AGTX_INT32 auto_y_zone_weight_list[MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_WEIGHT_LIST_SIZE];
	AGTX_INT32 manual_c_edge;
	AGTX_INT32 manual_c_radius;
	AGTX_INT32 manual_c_strength;
	AGTX_INT32 manual_y_txr_detail;
	AGTX_INT32 manual_y_txr_edge;
	AGTX_INT32 manual_y_txr_strength;
	AGTX_INT32 manual_y_zone_detail;
	AGTX_INT32 manual_y_zone_edge;
	AGTX_INT32 manual_y_zone_radius;
	AGTX_INT32 manual_y_zone_strength;
	AGTX_INT32 manual_y_zone_weight;
	AGTX_INT32 mode;
	AGTX_INT32 video_dev_idx;
} AGTX_DIP_ENH_CONF_S;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AGTX_DIP_ENH_CONF_H_ */
