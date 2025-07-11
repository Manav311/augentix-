#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_voice_conf.h"


void parse_voice_conf(AGTX_VOICE_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "hold_time", &tmp_obj)) {
		data->hold_time = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "priority", &tmp_obj)) {
		data->priority = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "volume", &tmp_obj)) {
		data->volume = json_object_get_int(tmp_obj);
	}
}

void comp_voice_conf(struct json_object *ret_obj, AGTX_VOICE_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->hold_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "hold_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "hold_time");
	}

	tmp_obj = json_object_new_int(data->priority);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "priority", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "priority");
	}

	tmp_obj = json_object_new_int(data->volume);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "volume", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "volume");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
