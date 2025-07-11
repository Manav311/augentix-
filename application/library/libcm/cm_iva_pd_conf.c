#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_pd_conf.h"


void parse_iva_pd_conf(AGTX_IVA_PD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_aspect_ratio_h", &tmp_obj)) {
		data->max_aspect_ratio_h = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_aspect_ratio_w", &tmp_obj)) {
		data->max_aspect_ratio_w = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_size", &tmp_obj)) {
		data->max_size = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_aspect_ratio_h", &tmp_obj)) {
		data->min_aspect_ratio_h = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_aspect_ratio_w", &tmp_obj)) {
		data->min_aspect_ratio_w = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_size", &tmp_obj)) {
		data->min_size = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
}

void comp_iva_pd_conf(struct json_object *ret_obj, AGTX_IVA_PD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->max_aspect_ratio_h);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_aspect_ratio_h", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_aspect_ratio_h");
	}

	tmp_obj = json_object_new_int(data->max_aspect_ratio_w);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_aspect_ratio_w", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_aspect_ratio_w");
	}

	tmp_obj = json_object_new_int(data->max_size);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_size", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_size");
	}

	tmp_obj = json_object_new_int(data->min_aspect_ratio_h);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_aspect_ratio_h", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_aspect_ratio_h");
	}

	tmp_obj = json_object_new_int(data->min_aspect_ratio_w);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_aspect_ratio_w", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_aspect_ratio_w");
	}

	tmp_obj = json_object_new_int(data->min_size);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_size", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_size");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
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
