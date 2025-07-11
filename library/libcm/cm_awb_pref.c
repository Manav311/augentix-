#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_awb_pref.h"


void parse_awb_pref(AGTX_AWB_PREF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "b_gain", &tmp_obj)) {
		data->b_gain = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "color_temp", &tmp_obj)) {
		data->color_temp = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "r_gain", &tmp_obj)) {
		data->r_gain = json_object_get_int(tmp_obj);
	}
}

void comp_awb_pref(struct json_object *ret_obj, AGTX_AWB_PREF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->b_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "b_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "b_gain");
	}

	tmp_obj = json_object_new_int(data->color_temp);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "color_temp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "color_temp");
	}

	tmp_obj = json_object_new_int(data->mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->r_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "r_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "r_gain");
	}
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
