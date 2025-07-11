#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_gamma_conf.h"

void parse_dip_gamma_conf(AGTX_DIP_GAMMA_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "gamma", &tmp_obj)) {
		data->gamma = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "gamma_manual", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_GAMMA_CONF_S_GAMMA_MANUAL_SIZE; i++) {
			data->gamma_manual[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_gamma_conf(struct json_object *ret_obj, AGTX_DIP_GAMMA_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->gamma);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gamma", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gamma");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_GAMMA_CONF_S_GAMMA_MANUAL_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->gamma_manual[i]));
		}
		json_object_object_add(ret_obj, "gamma_manual", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "gamma_manual");
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
