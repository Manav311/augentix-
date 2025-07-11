#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_event_param.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


void parse_event_attr(AGTX_EVENT_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	int i;

	if (json_object_object_get_ex(cmd_obj, "name", &tmp_obj)) {
		i = min(MAX_AGTX_EVENT_ATTR_S_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->name, json_object_get_string(tmp_obj), i);
		data->name[i] = '\0';
	}
}

void parse_event_param(AGTX_EVENT_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "event_attr", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE; i++) {
			parse_event_attr(&(data->event_attr[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_event_attr(struct json_object *array_obj, AGTX_EVENT_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->enabled);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "enabled", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "enabled");
		}

		const char *str;
		str = (const char *)data->name;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "name", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "name");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_event_param(struct json_object *ret_obj, AGTX_EVENT_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_EVENT_PARAM_S_EVENT_ATTR_SIZE; i++) {
			comp_event_attr(tmp_obj, &(data->event_attr[i]));
		}
		json_object_object_add(ret_obj, "event_attr", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "event_attr");
	}

}


#ifdef __cplusplus
}
#endif /* __cplusplus */
