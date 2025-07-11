#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_video_ldc_conf.h"


const char * agtx_ldc_view_type_e_map[] = {
	"CROP",
	"ALL"
};

void parse_ldc_conf(AGTX_LDC_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "center_x_offset", &tmp_obj)) {
		data->center_x_offset = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "center_y_offset", &tmp_obj)) {
		data->center_y_offset = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
		data->enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ratio", &tmp_obj)) {
		data->ratio = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "view_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_ldc_view_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_ldc_view_type_e_map[i], str) == 0) {
				data->view_type = (AGTX_LDC_VIEW_TYPE_E) i;
				break;
			}
		}
	}
}

void comp_ldc_conf(struct json_object *ret_obj, AGTX_LDC_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->center_x_offset);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "center_x_offset", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "center_x_offset");
	}

	tmp_obj = json_object_new_int(data->center_y_offset);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "center_y_offset", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "center_y_offset");
	}

	tmp_obj = json_object_new_int(data->enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enable");
	}

	tmp_obj = json_object_new_int(data->ratio);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ratio", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ratio");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	const char *str;
	str = agtx_ldc_view_type_e_map[data->view_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "view_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "view_type");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
