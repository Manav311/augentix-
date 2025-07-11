#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_anti_flicker_conf.h"


void parse_anti_flicker_list(AGTX_ANTI_FLICKER_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "fps", &tmp_obj)) {
		data->fps = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "frequency", &tmp_obj)) {
		data->frequency = json_object_get_int(tmp_obj);
	}
}

void parse_anti_flicker_conf(AGTX_ANTI_FLICKER_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
		data->enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "frequency_idx", &tmp_obj)) {
		data->frequency_idx = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "frequency_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_ANTI_FLICKER_CONF_S_FREQUENCY_LIST_SIZE; i++) {
			parse_anti_flicker_list(&(data->frequency_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_anti_flicker_list(struct json_object *array_obj, AGTX_ANTI_FLICKER_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_double(data->fps);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "fps", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "fps");
		}

		tmp1_obj = json_object_new_int(data->frequency);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "frequency", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "frequency");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_anti_flicker_conf(struct json_object *ret_obj, AGTX_ANTI_FLICKER_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enable");
	}

	tmp_obj = json_object_new_int(data->frequency_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "frequency_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "frequency_idx");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_ANTI_FLICKER_CONF_S_FREQUENCY_LIST_SIZE; i++) {
			comp_anti_flicker_list(tmp_obj, &(data->frequency_list[i]));
		}
		json_object_object_add(ret_obj, "frequency_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "frequency_list");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
