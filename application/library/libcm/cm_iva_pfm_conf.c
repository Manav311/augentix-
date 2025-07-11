#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_pfm_conf.h"


void parse_iva_pfm_region(AGTX_IVA_PFM_REGION_S *data, struct json_object *cmd_obj)
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

void parse_iva_pfm_conf(AGTX_IVA_PFM_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "endurance", &tmp_obj)) {
		data->endurance = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "regis_to_feeding_interval", &tmp_obj)) {
		data->regis_to_feeding_interval = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "register_scene", &tmp_obj)) {
		data->register_scene = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi", &tmp_obj)) {
		parse_iva_pfm_region(&(data->roi), tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "schedule", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_IVA_PFM_CONF_S_SCHEDULE_SIZE; i++) {
			data->schedule[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "time_number", &tmp_obj)) {
		data->time_number = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_pfm_region(struct json_object *ret_obj, AGTX_IVA_PFM_REGION_S *data)
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

void comp_iva_pfm_conf(struct json_object *ret_obj, AGTX_IVA_PFM_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->endurance);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "endurance", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "endurance");
	}

	tmp_obj = json_object_new_int(data->regis_to_feeding_interval);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "regis_to_feeding_interval", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "regis_to_feeding_interval");
	}

	tmp_obj = json_object_new_int(data->register_scene);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "register_scene", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "register_scene");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_iva_pfm_region(tmp_obj, &(data->roi));
		json_object_object_add(ret_obj, "roi", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_IVA_PFM_CONF_S_SCHEDULE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->schedule[i]));
		}
		json_object_object_add(ret_obj, "schedule", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "schedule");
	}

	tmp_obj = json_object_new_int(data->sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensitivity");
	}

	tmp_obj = json_object_new_int(data->time_number);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "time_number", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "time_number");
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
