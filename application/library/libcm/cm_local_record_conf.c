#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_local_record_conf.h"


const char * agtx_record_mode_e_map[] = {
	"event",
	"continuous"
};

void parse_local_record_conf(AGTX_LOCAL_RECORD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_event_time", &tmp_obj)) {
		data->max_event_time = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_event_time", &tmp_obj)) {
		data->min_event_time = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_record_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_record_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_RECORD_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "post_record_time", &tmp_obj)) {
		data->post_record_time = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pre_record_time", &tmp_obj)) {
		data->pre_record_time = json_object_get_int(tmp_obj);
	}
}

void comp_local_record_conf(struct json_object *ret_obj, AGTX_LOCAL_RECORD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->max_event_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_event_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_event_time");
	}

	tmp_obj = json_object_new_int(data->min_event_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_event_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_event_time");
	}

	const char *str;
	str = agtx_record_mode_e_map[data->mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->post_record_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "post_record_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "post_record_time");
	}

	tmp_obj = json_object_new_int(data->pre_record_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "pre_record_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "pre_record_time");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
