#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_fld_conf.h"

void parse_iva_fld_conf(AGTX_IVA_FLD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "demo_level", &tmp_obj)) {
		data->demo_level = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "down_period_th", &tmp_obj)) {
		data->down_period_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "fallen_period_th", &tmp_obj)) {
		data->fallen_period_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "falling_period_th", &tmp_obj)) {
		data->falling_period_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_falling_mv_th", &tmp_obj)) {
		data->obj_falling_mv_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_high_ratio_th", &tmp_obj)) {
		data->obj_high_ratio_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_stop_mv_th", &tmp_obj)) {
		data->obj_stop_mv_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_fld_conf(struct json_object *ret_obj, AGTX_IVA_FLD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->demo_level);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "demo_level", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "demo_level");
	}

	tmp_obj = json_object_new_int(data->down_period_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "down_period_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "down_period_th");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->fallen_period_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "fallen_period_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "fallen_period_th");
	}

	tmp_obj = json_object_new_int(data->falling_period_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "falling_period_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "falling_period_th");
	}

	tmp_obj = json_object_new_int(data->obj_falling_mv_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_falling_mv_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_falling_mv_th");
	}

	tmp_obj = json_object_new_int(data->obj_high_ratio_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_high_ratio_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_high_ratio_th");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
	}

	tmp_obj = json_object_new_int(data->obj_stop_mv_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_stop_mv_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_stop_mv_th");
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
