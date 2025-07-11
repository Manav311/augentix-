#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_wb_info.h"

void parse_dip_white_balance_info(AGTX_DIP_WHITE_BALANCE_INFO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "color_temp", &tmp_obj)) {
		data->color_temp = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "gain0", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_WHITE_BALANCE_INFO_S_GAIN0_SIZE; i++) {
			data->gain0[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "gain1", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_WHITE_BALANCE_INFO_S_GAIN1_SIZE; i++) {
			data->gain1[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "matrix", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_WHITE_BALANCE_INFO_S_MATRIX_SIZE; i++) {
			data->matrix[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_white_balance_info(struct json_object *ret_obj, AGTX_DIP_WHITE_BALANCE_INFO_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->color_temp);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "color_temp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "color_temp");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_WHITE_BALANCE_INFO_S_GAIN0_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->gain0[i]));
		}
		json_object_object_add(ret_obj, "gain0", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "gain0");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_WHITE_BALANCE_INFO_S_GAIN1_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->gain1[i]));
		}
		json_object_object_add(ret_obj, "gain1", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "gain1");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_WHITE_BALANCE_INFO_S_MATRIX_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->matrix[i]));
		}
		json_object_object_add(ret_obj, "matrix", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "matrix");
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
