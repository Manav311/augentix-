#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_shd_conf.h"


void parse_iva_shd_lt_list(AGTX_IVA_SHD_LT_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "end_x", &tmp_obj)) {
		data->end_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "end_y", &tmp_obj)) {
		data->end_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_x", &tmp_obj)) {
		data->start_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_y", &tmp_obj)) {
		data->start_y = json_object_get_int(tmp_obj);
	}
}

void parse_iva_shd_conf(AGTX_IVA_SHD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "instance_duration", &tmp_obj)) {
		data->instance_duration = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "longterm_dec_period", &tmp_obj)) {
		data->longterm_dec_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "longterm_life_th", &tmp_obj)) {
		data->longterm_life_th = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "longterm_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_IVA_SHD_CONF_S_LONGTERM_LIST_SIZE; i++) {
			parse_iva_shd_lt_list(&(data->longterm_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "longterm_num", &tmp_obj)) {
		data->longterm_num = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "quality", &tmp_obj)) {
		data->quality = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "shaking_update_duration", &tmp_obj)) {
		data->shaking_update_duration = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_shd_lt_list(struct json_object *array_obj, AGTX_IVA_SHD_LT_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->end_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "end_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "end_x");
		}

		tmp1_obj = json_object_new_int(data->end_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "end_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "end_y");
		}

		tmp1_obj = json_object_new_int(data->start_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "start_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "start_x");
		}

		tmp1_obj = json_object_new_int(data->start_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "start_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "start_y");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_iva_shd_conf(struct json_object *ret_obj, AGTX_IVA_SHD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->instance_duration);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "instance_duration", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "instance_duration");
	}

	tmp_obj = json_object_new_int(data->longterm_dec_period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "longterm_dec_period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "longterm_dec_period");
	}

	tmp_obj = json_object_new_int(data->longterm_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "longterm_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "longterm_life_th");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_IVA_SHD_CONF_S_LONGTERM_LIST_SIZE; i++) {
			comp_iva_shd_lt_list(tmp_obj, &(data->longterm_list[i]));
		}
		json_object_object_add(ret_obj, "longterm_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "longterm_list");
	}

	tmp_obj = json_object_new_int(data->longterm_num);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "longterm_num", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "longterm_num");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
	}

	tmp_obj = json_object_new_int(data->quality);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "quality", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "quality");
	}

	tmp_obj = json_object_new_int(data->sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensitivity");
	}

	tmp_obj = json_object_new_int(data->shaking_update_duration);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "shaking_update_duration", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "shaking_update_duration");
	}

	tmp_obj = json_object_new_int(data->video_chn_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_chn_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_chn_idx");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
