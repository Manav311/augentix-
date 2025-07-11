#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_rms_conf.h"


void parse_iva_rms_conf(AGTX_IVA_RMS_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "split_x", &tmp_obj)) {
		data->split_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "split_y", &tmp_obj)) {
		data->split_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_rms_conf(struct json_object *ret_obj, AGTX_IVA_RMS_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensitivity");
	}

	tmp_obj = json_object_new_int(data->split_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "split_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "split_x");
	}

	tmp_obj = json_object_new_int(data->split_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "split_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "split_y");
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
