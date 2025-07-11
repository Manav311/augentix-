#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_dip_iso_conf.h"

void parse_dip_iso_daa(AGTX_DIP_ISO_DAA_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "di_fallen_speed", &tmp_obj)) {
		data->di_fallen_speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "di_max", &tmp_obj)) {
		data->di_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "di_rising_speed", &tmp_obj)) {
		data->di_rising_speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
		data->enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "qp_lower_th", &tmp_obj)) {
		data->qp_lower_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "qp_upper_th", &tmp_obj)) {
		data->qp_upper_th = json_object_get_int(tmp_obj);
	}
}

void parse_dip_iso_conf(AGTX_DIP_ISO_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_iso_table", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ISO_CONF_S_AUTO_ISO_TABLE_SIZE; i++) {
			data->auto_iso_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "daa", &tmp_obj)) {
		parse_dip_iso_daa(&(data->daa), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "iso_type", &tmp_obj)) {
		data->iso_type = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_iso", &tmp_obj)) {
		data->manual_iso = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_iso_daa(struct json_object *ret_obj, AGTX_DIP_ISO_DAA_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->di_fallen_speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "di_fallen_speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "di_fallen_speed");
	}

	tmp_obj = json_object_new_int(data->di_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "di_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "di_max");
	}

	tmp_obj = json_object_new_int(data->di_rising_speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "di_rising_speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "di_rising_speed");
	}

	tmp_obj = json_object_new_int(data->enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enable");
	}

	tmp_obj = json_object_new_int(data->qp_lower_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "qp_lower_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "qp_lower_th");
	}

	tmp_obj = json_object_new_int(data->qp_upper_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "qp_upper_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "qp_upper_th");
	}
}

void comp_dip_iso_conf(struct json_object *ret_obj, AGTX_DIP_ISO_CONF_S *data)
{
	struct json_object *tmp_obj  = NULL;
	struct json_object *tmp1_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ISO_CONF_S_AUTO_ISO_TABLE_SIZE; i++) {
			tmp1_obj = json_object_new_int(data->auto_iso_table[i]);
			json_object_array_add(tmp_obj, tmp1_obj);
		}
		json_object_object_add(ret_obj, "auto_iso_table", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_iso_table");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_iso_daa(tmp_obj, &(data->daa));
		json_object_object_add(ret_obj, "daa", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "daa");
	}

	tmp_obj = json_object_new_int(data->iso_type);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "iso_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "iso_type");
	}

	tmp_obj = json_object_new_int(data->manual_iso);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_iso", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_iso");
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
