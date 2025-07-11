#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_pta_conf.h"


void parse_dip_pta_conf(AGTX_DIP_PTA_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_tone_table", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_PTA_CONF_S_AUTO_TONE_TABLE_SIZE; i++) {
			data->auto_tone_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "break_point", &tmp_obj)) {
		data->break_point = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "brightness", &tmp_obj)) {
		data->brightness = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "contrast", &tmp_obj)) {
		data->contrast = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "curve", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_PTA_CONF_S_CURVE_SIZE; i++) {
			data->curve[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_pta_conf(struct json_object *ret_obj, AGTX_DIP_PTA_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_PTA_CONF_S_AUTO_TONE_TABLE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_tone_table[i]));
		}
		json_object_object_add(ret_obj, "auto_tone_table", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_tone_table");
	}

	tmp_obj = json_object_new_int(data->break_point);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "break_point", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "break_point");
	}

	tmp_obj = json_object_new_int(data->brightness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "brightness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "brightness");
	}

	tmp_obj = json_object_new_int(data->contrast);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "contrast", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "contrast");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_PTA_CONF_S_CURVE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->curve[i]));
		}
		json_object_object_add(ret_obj, "curve", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "curve");
	}

	tmp_obj = json_object_new_int(data->mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
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
