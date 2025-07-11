#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cm_dip_dhz_conf.h"

#include "json.h"

#include <stdio.h>
#include <string.h>

void parse_dip_dhz_conf(AGTX_DIP_DHZ_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "auto_y_gain_max_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DHZ_CONF_S_AUTO_Y_GAIN_MAX_LIST_SIZE; i++) {
			data->auto_y_gain_max_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_c_gain_max_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DHZ_CONF_S_AUTO_C_GAIN_MAX_LIST_SIZE; i++) {
			data->auto_c_gain_max_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}

	if (json_object_object_get_ex(cmd_obj, "manual_y_gain_max", &tmp_obj)) {
		data->manual_y_gain_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_c_gain_max", &tmp_obj)) {
		data->manual_c_gain_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "dc_iir_weight", &tmp_obj)) {
		data->dc_iir_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gain_step_th", &tmp_obj)) {
		data->gain_step_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_dhz_conf(struct json_object *ret_obj, AGTX_DIP_DHZ_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;
	int i;

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_DHZ_CONF_S_AUTO_Y_GAIN_MAX_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_gain_max_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_gain_max_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_gain_max_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_DHZ_CONF_S_AUTO_C_GAIN_MAX_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_c_gain_max_list[i]));
		}
		json_object_object_add(ret_obj, "auto_c_gain_max_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_c_gain_max_list");
	}

	tmp_obj = json_object_new_int(data->manual_y_gain_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_gain_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_gain_max");
	}

	tmp_obj = json_object_new_int(data->manual_c_gain_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_c_gain_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_c_gain_max");
	}

	tmp_obj = json_object_new_int(data->dc_iir_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "dc_iir_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "dc_iir_weight");
	}

	tmp_obj = json_object_new_int(data->gain_step_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gain_step_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gain_step_th");
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
