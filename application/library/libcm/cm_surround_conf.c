#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_surround_conf.h"


const char * agtx_surr_rotate_e_map[] = {
	"0",
	"90",
	"180",
	"270"
};

void parse_surround_conf(AGTX_SURROUND_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "center_offset_x", &tmp_obj)) {
		data->center_offset_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "center_offset_y", &tmp_obj)) {
		data->center_offset_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
		data->enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ldc_ratio", &tmp_obj)) {
		data->ldc_ratio = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_radius", &tmp_obj)) {
		data->max_radius = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_radius", &tmp_obj)) {
		data->min_radius = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "rotate", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_surr_rotate_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_surr_rotate_e_map[i], str) == 0) {
				data->rotate = (AGTX_SURR_ROTATE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_surround_conf(struct json_object *ret_obj, AGTX_SURROUND_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->center_offset_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "center_offset_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "center_offset_x");
	}

	tmp_obj = json_object_new_int(data->center_offset_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "center_offset_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "center_offset_y");
	}

	tmp_obj = json_object_new_int(data->enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enable");
	}

	tmp_obj = json_object_new_int(data->ldc_ratio);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ldc_ratio", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ldc_ratio");
	}

	tmp_obj = json_object_new_int(data->max_radius);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_radius", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_radius");
	}

	tmp_obj = json_object_new_int(data->min_radius);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_radius", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_radius");
	}

	const char *str;
	str = agtx_surr_rotate_e_map[data->rotate];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "rotate", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "rotate");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
