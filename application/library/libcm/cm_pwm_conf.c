#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_pwm_conf.h"

#define min(X,Y) (((X) < (Y)) ? (X) : (Y))

void parse_pwm_alias(AGTX_PWM_ALIAS_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "duty_cycle", &tmp_obj)) {
		data->duty_cycle = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	int i;

	if (json_object_object_get_ex(cmd_obj, "name", &tmp_obj)) {
		i = min(MAX_AGTX_PWM_ALIAS_S_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->name, json_object_get_string(tmp_obj), i);
		data->name[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "period", &tmp_obj)) {
		data->period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pin_num", &tmp_obj)) {
		data->pin_num = json_object_get_int(tmp_obj);
	}
}

void parse_pwm_conf(AGTX_PWM_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "pwm_alias", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE; i++) {
			parse_pwm_alias(&(data->pwm_alias[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_pwm_alias(struct json_object *array_obj, AGTX_PWM_ALIAS_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->duty_cycle);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "duty_cycle", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "duty_cycle");
		}

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

		tmp1_obj = json_object_new_int(data->period);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "period", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "period");
		}

		tmp1_obj = json_object_new_int(data->pin_num);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "pin_num", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "pin_num");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_pwm_conf(struct json_object *ret_obj, AGTX_PWM_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_PWM_CONF_S_PWM_ALIAS_SIZE; i++) {
			comp_pwm_alias(tmp_obj, &(data->pwm_alias[i]));
		}
		json_object_object_add(ret_obj, "pwm_alias", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "pwm_alias");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
