#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_dk_conf.h"

void parse_iva_dk_region(AGTX_IVA_DK_REGION_S *data, struct json_object *cmd_obj)
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

void parse_iva_dk_conf(AGTX_IVA_DK_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "overlap_ratio_th", &tmp_obj)) {
		data->overlap_ratio_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "loiter_count_th", &tmp_obj)) {
		data->loiter_count_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "loiter_period_th", &tmp_obj)) {
		data->loiter_period_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi", &tmp_obj)) {
		parse_iva_dk_region(&(data->roi), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "visit_count_th", &tmp_obj)) {
		data->visit_count_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "visit_period_th", &tmp_obj)) {
		data->visit_period_th = json_object_get_int(tmp_obj);
	}
}

void comp_iva_dk_region(struct json_object *ret_obj, AGTX_IVA_DK_REGION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->end_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "end_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "end_x");
	}

	tmp_obj = json_object_new_int(data->end_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "end_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "end_y");
	}

	tmp_obj = json_object_new_int(data->start_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "start_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "start_x");
	}

	tmp_obj = json_object_new_int(data->start_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "start_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "start_y");
	}
}

void comp_iva_dk_conf(struct json_object *ret_obj, AGTX_IVA_DK_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
	}

	tmp_obj = json_object_new_int(data->overlap_ratio_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "overlap_ratio_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "overlap_ratio_th");
	}

	tmp_obj = json_object_new_int(data->loiter_count_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "loiter_count_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "loiter_count_th");
	}

	tmp_obj = json_object_new_int(data->loiter_period_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "loiter_period_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "loiter_period_th");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_iva_dk_region(tmp_obj, &(data->roi));
		json_object_object_add(ret_obj, "roi", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi");
	}

	tmp_obj = json_object_new_int(data->video_chn_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_chn_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_chn_idx");
	}

	tmp_obj = json_object_new_int(data->visit_count_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "visit_count_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "visit_count_th");
	}

	tmp_obj = json_object_new_int(data->visit_period_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "visit_period_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "visit_period_th");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
