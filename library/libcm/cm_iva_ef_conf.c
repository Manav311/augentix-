#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_ef_conf.h"


const char * agtx_iva_ef_mode_e_map[] = {
	"DIR_NONE",
	"DIR_POS",
	"DIR_NEG",
	"DIR_BOTH"
};

void parse_iva_ef_line(AGTX_IVA_EF_LINE_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "end_x", &tmp_obj)) {
		data->end_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "end_y", &tmp_obj)) {
		data->end_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "id", &tmp_obj)) {
		data->id = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_ef_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_ef_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_IVA_EF_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "obj_area", &tmp_obj)) {
		data->obj_area = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_max_h", &tmp_obj)) {
		data->obj_max_h = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_max_w", &tmp_obj)) {
		data->obj_max_w = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_min_h", &tmp_obj)) {
		data->obj_min_h = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_min_w", &tmp_obj)) {
		data->obj_min_w = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_v_th", &tmp_obj)) {
		data->obj_v_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_x", &tmp_obj)) {
		data->start_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_y", &tmp_obj)) {
		data->start_y = json_object_get_int(tmp_obj);
	}
}

void parse_iva_ef_conf(AGTX_IVA_EF_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "active_cell", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EF_CONF_S_ACTIVE_CELL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->active_cell, json_object_get_string(tmp_obj), i);
		data->active_cell[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "line_cnt", &tmp_obj)) {
		data->line_cnt = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "line_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_IVA_EF_CONF_S_LINE_LIST_SIZE; i++) {
			parse_iva_ef_line(&(data->line_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_ef_line(struct json_object *array_obj, AGTX_IVA_EF_LINE_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->end_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "end_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "end_x");
		}

		tmp1_obj = json_object_new_int(data->end_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "end_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "end_y");
		}

		tmp1_obj = json_object_new_int(data->id);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "id", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "id");
		}

		const char *str;
		str = agtx_iva_ef_mode_e_map[data->mode];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mode", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mode");
		}

		tmp1_obj = json_object_new_int(data->obj_area);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_area", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_area");
		}

		tmp1_obj = json_object_new_int(data->obj_life_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_life_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_life_th");
		}

		tmp1_obj = json_object_new_int(data->obj_max_h);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_max_h", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_max_h");
		}

		tmp1_obj = json_object_new_int(data->obj_max_w);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_max_w", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_max_w");
		}

		tmp1_obj = json_object_new_int(data->obj_min_h);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_min_h", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_min_h");
		}

		tmp1_obj = json_object_new_int(data->obj_min_w);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_min_w", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_min_w");
		}

		tmp1_obj = json_object_new_int(data->obj_v_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_v_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_v_th");
		}

		tmp1_obj = json_object_new_int(data->start_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "start_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "start_x");
		}

		tmp1_obj = json_object_new_int(data->start_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "start_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "start_y");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_iva_ef_conf(struct json_object *ret_obj, AGTX_IVA_EF_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = (const char *)data->active_cell;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "active_cell", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "active_cell");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->line_cnt);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "line_cnt", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "line_cnt");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_IVA_EF_CONF_S_LINE_LIST_SIZE; i++) {
			comp_iva_ef_line(tmp_obj, &(data->line_list[i]));
		}
		json_object_object_add(ret_obj, "line_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "line_list");
	}

	tmp_obj = json_object_new_int(data->video_chn_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_chn_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_chn_idx");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
