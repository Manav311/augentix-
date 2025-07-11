#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_panning_conf.h"


void parse_panning_conf(AGTX_PANNING_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "center_offset_x", &tmp_obj)) {
		data->center_offset_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "center_offset_y", &tmp_obj)) {
		data->center_offset_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
		data->enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "hor_strength", &tmp_obj)) {
		data->hor_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ldc_ratio", &tmp_obj)) {
		data->ldc_ratio = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "radius", &tmp_obj)) {
		data->radius = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ver_strength", &tmp_obj)) {
		data->ver_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_panning_conf(struct json_object *ret_obj, AGTX_PANNING_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->center_offset_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "center_offset_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "center_offset_x");
	}

	tmp_obj = json_object_new_int(data->center_offset_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "center_offset_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "center_offset_y");
	}

	tmp_obj = json_object_new_int(data->enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enable");
	}

	tmp_obj = json_object_new_int(data->hor_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "hor_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "hor_strength");
	}

	tmp_obj = json_object_new_int(data->ldc_ratio);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ldc_ratio", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ldc_ratio");
	}

	tmp_obj = json_object_new_int(data->radius);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "radius", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "radius");
	}

	tmp_obj = json_object_new_int(data->ver_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ver_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ver_strength");
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
