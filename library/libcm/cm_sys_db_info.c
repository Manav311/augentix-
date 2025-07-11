#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_sys_db_info.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))
#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


const char * agtx_update_rule_e_map[] = {
	"NONE",
	"OVERWRITE"
};

void parse_led_info(AGTX_LED_INFO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "gpio_pin0", &tmp_obj)) {
		data->gpio_pin0 = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "gpio_pin1", &tmp_obj)) {
		data->gpio_pin1 = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "inverse_pin0", &tmp_obj)) {
		data->inverse_pin0 = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "inverse_pin1", &tmp_obj)) {
		data->inverse_pin1 = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "period_msec", &tmp_obj)) {
		data->period_msec = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "duty_cycle", &tmp_obj)) {
		data->duty_cycle = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "interleave", &tmp_obj)) {
		data->interleave = json_object_get_int(tmp_obj);
	}
}

void parse_fw_setting_param(AGTX_FW_SETTING_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "name", &tmp_obj)) {
		i = min(MAX_AGTX_FW_SETTING_PARAM_S_NAME_SIZE - 1, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->name, json_object_get_string(tmp_obj), i);
		data->name[i] = '\0';
	}

	if (json_object_object_get_ex(cmd_obj, "update_rule", &tmp_obj)) {
		const char *str = json_object_get_string(tmp_obj);

		for (i = 0; (unsigned long)i < sizeof(agtx_update_rule_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_update_rule_e_map[i], str) == 0) {
				data->update_rule = (AGTX_UPDATE_RULE_E)i;
				break;
			}
		}
	}
}

void parse_sys_db_info(AGTX_SYS_DB_INFO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "db_ver", &tmp_obj)) {
		i = min(MAX_AGTX_SYS_DB_INFO_S_DB_VER_SIZE - 1, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->db_ver, json_object_get_string(tmp_obj), i);
		data->db_ver[i] = '\0';
	}

	if (json_object_object_get_ex(cmd_obj, "led_info", &tmp_obj)) {
		parse_led_info(&(data->led_info), tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "fw_setting_list", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < MAX_AGTX_SYS_DB_INFO_S_FW_SETTING_LIST_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (tmp1_obj) {
				parse_fw_setting_param(&(data->fw_setting_list[i]), tmp1_obj);
			}
		}
	}
}

void comp_led_info(struct json_object *ret_obj, AGTX_LED_INFO_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->gpio_pin0);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gpio_pin0", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gpio_pin0");
	}

	tmp_obj = json_object_new_int(data->gpio_pin1);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gpio_pin1", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gpio_pin1");
	}

	tmp_obj = json_object_new_int(data->inverse_pin0);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "inverse_pin0", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "inverse_pin0");
	}

	tmp_obj = json_object_new_int(data->inverse_pin1);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "inverse_pin1", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "inverse_pin1");
	}

	tmp_obj = json_object_new_int(data->period_msec);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "period_msec", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "period_msec");
	}

	tmp_obj = json_object_new_int(data->duty_cycle);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "duty_cycle", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "duty_cycle");
	}

	tmp_obj = json_object_new_int(data->interleave);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "interleave", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "interleave");
	}
}

void comp_fw_setting_param(struct json_object *array_obj, AGTX_FW_SETTING_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = (const char *)data->name;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "name", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "name");
		}

		str = agtx_update_rule_e_map[data->update_rule];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "update_rule", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "update_rule");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_sys_db_info(struct json_object *ret_obj, AGTX_SYS_DB_INFO_S *data)
{
	struct json_object *tmp_obj = NULL;
	int i;
	const char *str;

	str = (const char *)data->db_ver;

	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "db_ver", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "db_ver");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_led_info(tmp_obj, &(data->led_info));
		json_object_object_add(ret_obj, "led_info", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "led_info");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_SYS_DB_INFO_S_FW_SETTING_LIST_SIZE; i++) {
			comp_fw_setting_param(tmp_obj, &(data->fw_setting_list[i]));
		}
		json_object_object_add(ret_obj, "fw_setting_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "fw_setting_list");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
