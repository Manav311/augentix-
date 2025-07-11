#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_md_conf.h"

#define min(a, b) ((a) > (b) ? (a) : (b))

const char * agtx_iva_md_mode_e_map[] = {
	"AREA",
	"ENERGY"
};

const char * agtx_iva_md_det_e_map[] = {
	"NORMAL",
	"SUBTRACT"
};

void parse_iva_md_region(AGTX_IVA_MD_REGION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "det", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_md_det_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_md_det_e_map[i], str) == 0) {
				data->det = (AGTX_IVA_MD_DET_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "ex", &tmp_obj)) {
		data->ex = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ey", &tmp_obj)) {
		data->ey = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "id", &tmp_obj)) {
		data->id = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_spd", &tmp_obj)) {
		data->max_spd = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_spd", &tmp_obj)) {
		data->min_spd = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_md_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_md_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_IVA_MD_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sens", &tmp_obj)) {
		data->sens = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sx", &tmp_obj)) {
		data->sx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sy", &tmp_obj)) {
		data->sy = json_object_get_int(tmp_obj);
	}
}

void parse_iva_md_conf(AGTX_IVA_MD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "active_cell", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_MD_CONF_S_ACTIVE_CELL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->active_cell, json_object_get_string(tmp_obj), i);
		data->active_cell[i] = '\0';
	}

	if (json_object_object_get_ex(cmd_obj, "alarm_buffer", &tmp_obj)) {
		data->alarm_buffer = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "alarm_switch_on_time", &tmp_obj)) {
		data->alarm_switch_on_time = json_object_get_int(tmp_obj);
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "det", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_md_det_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_md_det_e_map[i], str) == 0) {
				data->det = (AGTX_IVA_MD_DET_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "en_rgn", &tmp_obj)) {
		data->en_rgn = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_skip_pd", &tmp_obj)) {
		data->en_skip_pd = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_skip_shake", &tmp_obj)) {
		data->en_skip_shake = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_spd", &tmp_obj)) {
		data->max_spd = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_spd", &tmp_obj)) {
		data->min_spd = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_md_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_md_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_IVA_MD_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "rgn_cnt", &tmp_obj)) {
		data->rgn_cnt = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "rgn_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_IVA_MD_CONF_S_RGN_LIST_SIZE; i++) {
			parse_iva_md_region(&(data->rgn_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "sens", &tmp_obj)) {
		data->sens = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_md_region(struct json_object *array_obj, AGTX_IVA_MD_REGION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		const char *str;
		str = agtx_iva_md_det_e_map[data->det];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "det", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "det");
		}

		tmp1_obj = json_object_new_int(data->ex);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ex", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ex");
		}

		tmp1_obj = json_object_new_int(data->ey);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ey", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ey");
		}

		tmp1_obj = json_object_new_int(data->id);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "id", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "id");
		}

		tmp1_obj = json_object_new_int(data->max_spd);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "max_spd", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "max_spd");
		}

		tmp1_obj = json_object_new_int(data->min_spd);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "min_spd", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "min_spd");
		}

		str = agtx_iva_md_mode_e_map[data->mode];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mode", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mode");
		}

		tmp1_obj = json_object_new_int(data->obj_life_th);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "obj_life_th", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "obj_life_th");
		}

		tmp1_obj = json_object_new_int(data->sens);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "sens", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "sens");
		}

		tmp1_obj = json_object_new_int(data->sx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "sx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "sx");
		}

		tmp1_obj = json_object_new_int(data->sy);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "sy", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "sy");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_iva_md_conf(struct json_object *ret_obj, AGTX_IVA_MD_CONF_S *data)
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

	tmp_obj = json_object_new_double(data->alarm_buffer);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "alarm_buffer", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "alarm_buffer");
	}

	tmp_obj = json_object_new_int(data->alarm_switch_on_time);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "alarm_switch_on_time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "alarm_switch_on_time");
	}

	str = agtx_iva_md_det_e_map[data->det];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "det", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "det");
	}

	tmp_obj = json_object_new_int(data->en_rgn);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_rgn", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_rgn");
	}

	tmp_obj = json_object_new_int(data->en_skip_pd);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_skip_pd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_skip_pd");
	}

	tmp_obj = json_object_new_int(data->en_skip_shake);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_skip_shake", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_skip_shake");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->max_spd);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_spd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_spd");
	}

	tmp_obj = json_object_new_int(data->min_spd);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_spd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_spd");
	}

	str = agtx_iva_md_mode_e_map[data->mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
	}

	tmp_obj = json_object_new_int(data->rgn_cnt);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "rgn_cnt", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "rgn_cnt");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_IVA_MD_CONF_S_RGN_LIST_SIZE; i++) {
			comp_iva_md_region(tmp_obj, &(data->rgn_list[i]));
		}
		json_object_object_add(ret_obj, "rgn_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "rgn_list");
	}

	tmp_obj = json_object_new_int(data->sens);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sens", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sens");
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
