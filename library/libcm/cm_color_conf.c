#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"
#include "cm_color_conf.h"
#include "agtx_color_conf.h"


const char * agtx_color_mode_e_map[] = {
	"DAY",
	"NIGHT"
};

void parse_sw_light_sensing_par(AGTX_SW_LIGHT_SENSING_PARAM *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "bg_ratio_max", &tmp_obj)) {
		data->bg_ratio_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "bg_ratio_min", &tmp_obj)) {
		data->bg_ratio_min = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "day2ir_th", &tmp_obj)) {
		data->day2ir_th = json_object_get_int(tmp_obj);
	}
	int i;

	if (json_object_object_get_ex(cmd_obj, "detect_name", &tmp_obj)) {
		i = min(MAX_AGTX_SW_LIGHT_SENSING_PARAM_DETECT_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->detect_name, json_object_get_string(tmp_obj), i);
		data->detect_name[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "ir2day_th", &tmp_obj)) {
		data->ir2day_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "rg_ratio_max", &tmp_obj)) {
		data->rg_ratio_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "rg_ratio_min", &tmp_obj)) {
		data->rg_ratio_min = json_object_get_int(tmp_obj);
	}
}

void parse_color_conf(AGTX_COLOR_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "color_mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_color_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_color_mode_e_map[i], str) == 0) {
				data->color_mode = (AGTX_COLOR_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "params", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_COLOR_CONF_S_PARAMS_SIZE; i++) {
			parse_sw_light_sensing_par(&(data->params[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_sw_light_sensing_par(struct json_object *array_obj, AGTX_SW_LIGHT_SENSING_PARAM *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->bg_ratio_max);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "bg_ratio_max", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "bg_ratio_max");
		}

		tmp1_obj = json_object_new_int(data->bg_ratio_min);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "bg_ratio_min", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "bg_ratio_min");
		}

		tmp1_obj = json_object_new_int(data->day2ir_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "day2ir_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "day2ir_th");
		}

		const char *str;
		str = (const char *)data->detect_name;
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "detect_name", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "detect_name");
		}

		tmp1_obj = json_object_new_int(data->ir2day_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ir2day_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ir2day_th");
		}

		tmp1_obj = json_object_new_int(data->rg_ratio_max);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "rg_ratio_max", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rg_ratio_max");
		}

		tmp1_obj = json_object_new_int(data->rg_ratio_min);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "rg_ratio_min", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "rg_ratio_min");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_color_conf(struct json_object *ret_obj, AGTX_COLOR_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_color_mode_e_map[data->color_mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "color_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "color_mode");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_COLOR_CONF_S_PARAMS_SIZE; i++) {
			comp_sw_light_sensing_par(tmp_obj, &(data->params[i]));
		}
		json_object_object_add(ret_obj, "params", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "params");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
