#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_aroi_conf.h"


void parse_iva_aroi_conf(AGTX_IVA_AROI_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "aspect_ratio_height", &tmp_obj)) {
		data->aspect_ratio_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "aspect_ratio_width", &tmp_obj)) {
		data->aspect_ratio_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_skip_shake", &tmp_obj)) {
		data->en_skip_shake = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_roi_height", &tmp_obj)) {
		data->max_roi_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_roi_width", &tmp_obj)) {
		data->max_roi_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_roi_height", &tmp_obj)) {
		data->min_roi_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_roi_width", &tmp_obj)) {
		data->min_roi_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "return_speed", &tmp_obj)) {
		data->return_speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "track_speed", &tmp_obj)) {
		data->track_speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_aroi_conf(struct json_object *ret_obj, AGTX_IVA_AROI_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->aspect_ratio_height);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "aspect_ratio_height", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "aspect_ratio_height");
	}

	tmp_obj = json_object_new_int(data->aspect_ratio_width);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "aspect_ratio_width", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "aspect_ratio_width");
	}

	tmp_obj = json_object_new_int(data->en_skip_shake);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_skip_shake", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_skip_shake");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->max_roi_height);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_roi_height", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_roi_height");
	}

	tmp_obj = json_object_new_int(data->max_roi_width);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_roi_width", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_roi_width");
	}

	tmp_obj = json_object_new_int(data->min_roi_height);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_roi_height", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_roi_height");
	}

	tmp_obj = json_object_new_int(data->min_roi_width);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_roi_width", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_roi_width");
	}

	tmp_obj = json_object_new_int(data->return_speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "return_speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "return_speed");
	}

	tmp_obj = json_object_new_int(data->track_speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "track_speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "track_speed");
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
