#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iaa_lsd_conf.h"


void parse_iaa_lsd_conf(AGTX_IAA_LSD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "audio_dev_idx", &tmp_obj)) {
		data->audio_dev_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "duration", &tmp_obj)) {
		data->duration = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "suppression", &tmp_obj)) {
		data->suppression = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "volume", &tmp_obj)) {
		data->volume = json_object_get_int(tmp_obj);
	}
}

void comp_iaa_lsd_conf(struct json_object *ret_obj, AGTX_IAA_LSD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->audio_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "audio_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "audio_dev_idx");
	}

	tmp_obj = json_object_new_double(data->duration);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "duration", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "duration");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_double(data->suppression);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "suppression", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "suppression");
	}

	tmp_obj = json_object_new_int(data->volume);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "volume", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "volume");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
