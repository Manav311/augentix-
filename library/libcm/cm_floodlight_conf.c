#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_floodlight_conf.h"


void parse_floodlight_conf(AGTX_FLOODLIGHT_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "bright_mode", &tmp_obj)) {
		data->bright_mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "lightness", &tmp_obj)) {
		data->lightness = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "warn_switch", &tmp_obj)) {
		data->warn_switch = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "warn_time", &tmp_obj)) {
		data->warn_time = json_object_get_int(tmp_obj);
	}
}

void comp_floodlight_conf(struct json_object *ret_obj, AGTX_FLOODLIGHT_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->bright_mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "bright_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "bright_mode");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->lightness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "lightness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "lightness");
	}

	tmp_obj = json_object_new_int(data->warn_switch);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "warn_switch", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "warn_switch");
	}

	tmp_obj = json_object_new_int(data->warn_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "warn_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "warn_time");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
