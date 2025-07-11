#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_enh_conf.h"

void parse_dip_enh_conf(AGTX_DIP_ENH_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_c_edge_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_EDGE_LIST_SIZE; i++) {
			data->auto_c_edge_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_c_radius_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_RADIUS_LIST_SIZE; i++) {
			data->auto_c_radius_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_c_strength_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_STRENGTH_LIST_SIZE; i++) {
			data->auto_c_strength_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_txr_detail_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_DETAIL_LIST_SIZE; i++) {
			data->auto_y_txr_detail_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_txr_edge_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_EDGE_LIST_SIZE; i++) {
			data->auto_y_txr_edge_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_txr_strength_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_STRENGTH_LIST_SIZE; i++) {
			data->auto_y_txr_strength_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_zone_detail_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_DETAIL_LIST_SIZE; i++) {
			data->auto_y_zone_detail_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_zone_edge_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_EDGE_LIST_SIZE; i++) {
			data->auto_y_zone_edge_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_zone_radius_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_RADIUS_LIST_SIZE; i++) {
			data->auto_y_zone_radius_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_zone_strength_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_STRENGTH_LIST_SIZE; i++) {
			data->auto_y_zone_strength_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_zone_weight_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_WEIGHT_LIST_SIZE; i++) {
			data->auto_y_zone_weight_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "manual_c_edge", &tmp_obj)) {
		data->manual_c_edge = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_c_radius", &tmp_obj)) {
		data->manual_c_radius = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_c_strength", &tmp_obj)) {
		data->manual_c_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_txr_detail", &tmp_obj)) {
		data->manual_y_txr_detail = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_txr_edge", &tmp_obj)) {
		data->manual_y_txr_edge = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_txr_strength", &tmp_obj)) {
		data->manual_y_txr_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_zone_detail", &tmp_obj)) {
		data->manual_y_zone_detail = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_zone_edge", &tmp_obj)) {
		data->manual_y_zone_edge = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_zone_radius", &tmp_obj)) {
		data->manual_y_zone_radius = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_zone_strength", &tmp_obj)) {
		data->manual_y_zone_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_zone_weight", &tmp_obj)) {
		data->manual_y_zone_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_enh_conf(struct json_object *ret_obj, AGTX_DIP_ENH_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_EDGE_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_c_edge_list[i]));
		}
		json_object_object_add(ret_obj, "auto_c_edge_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_c_edge_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_RADIUS_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_c_radius_list[i]));
		}
		json_object_object_add(ret_obj, "auto_c_radius_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_c_radius_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_C_STRENGTH_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_c_strength_list[i]));
		}
		json_object_object_add(ret_obj, "auto_c_strength_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_c_strength_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_DETAIL_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_txr_detail_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_txr_detail_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_txr_detail_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_EDGE_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_txr_edge_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_txr_edge_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_txr_edge_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_TXR_STRENGTH_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_txr_strength_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_txr_strength_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_txr_strength_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_DETAIL_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_zone_detail_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_zone_detail_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_zone_detail_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_EDGE_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_zone_edge_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_zone_edge_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_zone_edge_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_RADIUS_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_zone_radius_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_zone_radius_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_zone_radius_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_STRENGTH_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_zone_strength_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_zone_strength_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_zone_strength_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ENH_CONF_S_AUTO_Y_ZONE_WEIGHT_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_zone_weight_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_zone_weight_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_zone_weight_list");
	}

	tmp_obj = json_object_new_int(data->manual_c_edge);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_c_edge", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_c_edge");
	}

	tmp_obj = json_object_new_int(data->manual_c_radius);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_c_radius", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_c_radius");
	}

	tmp_obj = json_object_new_int(data->manual_c_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_c_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_c_strength");
	}

	tmp_obj = json_object_new_int(data->manual_y_txr_detail);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_txr_detail", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_txr_detail");
	}

	tmp_obj = json_object_new_int(data->manual_y_txr_edge);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_txr_edge", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_txr_edge");
	}

	tmp_obj = json_object_new_int(data->manual_y_txr_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_txr_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_txr_strength");
	}

	tmp_obj = json_object_new_int(data->manual_y_zone_detail);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_zone_detail", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_zone_detail");
	}

	tmp_obj = json_object_new_int(data->manual_y_zone_edge);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_zone_edge", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_zone_edge");
	}

	tmp_obj = json_object_new_int(data->manual_y_zone_radius);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_zone_radius", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_zone_radius");
	}

	tmp_obj = json_object_new_int(data->manual_y_zone_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_zone_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_zone_strength");
	}

	tmp_obj = json_object_new_int(data->manual_y_zone_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_zone_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_zone_weight");
	}

	tmp_obj = json_object_new_int(data->mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
