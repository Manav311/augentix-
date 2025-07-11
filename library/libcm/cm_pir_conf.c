#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_pir_conf.h"

#define min(X,Y) (((X) < (Y)) ? (X) : (Y))

void parse_pir_group(AGTX_PIR_GROUP_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	int i;

	if (json_object_object_get_ex(cmd_obj, "name", &tmp_obj)) {
		i = min(MAX_AGTX_PIR_GROUP_S_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->name, json_object_get_string(tmp_obj), i);
		data->name[i] = '\0';
	}
}

void parse_pir_conf(AGTX_PIR_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "duty_cycle", &tmp_obj)) {
		data->duty_cycle = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "period", &tmp_obj)) {
		data->period = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "pir_alias", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; i++) {
			parse_pir_group(&(data->pir_alias[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_pir_group(struct json_object *array_obj, AGTX_PIR_GROUP_S *data)
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

void comp_pir_conf(struct json_object *ret_obj, AGTX_PIR_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->duty_cycle);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "duty_cycle", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "duty_cycle");
	}

	tmp_obj = json_object_new_int(data->period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "period");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_PIR_CONF_S_PIR_ALIAS_SIZE; i++) {
			comp_pir_group(tmp_obj, &(data->pir_alias[i]));
		}
		json_object_object_add(ret_obj, "pir_alias", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "pir_alias");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
