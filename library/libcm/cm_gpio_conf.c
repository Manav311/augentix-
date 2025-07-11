#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_gpio_conf.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


const char * agtx_gpio_dir_e_map[] = {
	"IN",
	"OUT",
};


void parse_gpio_alias(AGTX_GPIO_ALIAS_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "dir", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_gpio_dir_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_gpio_dir_e_map[i], str) == 0) {
				data->dir = (AGTX_GPIO_DIR_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "name", &tmp_obj)) {
		i = min(MAX_AGTX_GPIO_ALIAS_S_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->name, json_object_get_string(tmp_obj), i);
		data->name[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "pin_num", &tmp_obj)) {
		data->pin_num = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "value", &tmp_obj)) {
		data->value = json_object_get_int(tmp_obj);
	}
}

void parse_gpio_conf(AGTX_GPIO_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "gpio_alias", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE; i++) {
			parse_gpio_alias(&(data->gpio_alias[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_gpio_alias(struct json_object *array_obj, AGTX_GPIO_ALIAS_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_gpio_dir_e_map[data->dir];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "dir", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "dir");
		}

		str = (const char *)data->name;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "name", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "name");
		}

		tmp1_obj = json_object_new_int(data->pin_num);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "pin_num", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "pin_num");
		}

		tmp1_obj = json_object_new_int(data->value);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "value", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "value");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_gpio_conf(struct json_object *ret_obj, AGTX_GPIO_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_GPIO_CONF_S_GPIO_ALIAS_SIZE; i++) {
			comp_gpio_alias(tmp_obj, &(data->gpio_alias[i]));
		}
		json_object_object_add(ret_obj, "gpio_alias", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "gpio_alias");
	}

}


#ifdef __cplusplus
}
#endif /* __cplusplus */
