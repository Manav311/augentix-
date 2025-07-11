#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_fcs_conf.h"

void parse_dip_fcs_conf(AGTX_DIP_FCS_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_offset_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_FCS_CONF_S_AUTO_OFFSET_LIST_SIZE; i++) {
			data->auto_offset_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_strength_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_FCS_CONF_S_AUTO_STRENGTH_LIST_SIZE; i++) {
			data->auto_strength_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_threshold_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_FCS_CONF_S_AUTO_THRESHOLD_LIST_SIZE; i++) {
			data->auto_threshold_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "manual_offset", &tmp_obj)) {
		data->manual_offset = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_strength", &tmp_obj)) {
		data->manual_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_threshold", &tmp_obj)) {
		data->manual_threshold = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_fcs_conf(struct json_object *ret_obj, AGTX_DIP_FCS_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_FCS_CONF_S_AUTO_OFFSET_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_offset_list[i]));
		}
		json_object_object_add(ret_obj, "auto_offset_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_offset_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_FCS_CONF_S_AUTO_STRENGTH_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_strength_list[i]));
		}
		json_object_object_add(ret_obj, "auto_strength_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_strength_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_FCS_CONF_S_AUTO_THRESHOLD_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_threshold_list[i]));
		}
		json_object_object_add(ret_obj, "auto_threshold_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_threshold_list");
	}

	tmp_obj = json_object_new_int(data->manual_offset);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_offset", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_offset");
	}

	tmp_obj = json_object_new_int(data->manual_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_strength");
	}

	tmp_obj = json_object_new_int(data->manual_threshold);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_threshold", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_threshold");
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
