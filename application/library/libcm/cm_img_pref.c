#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_img_pref.h"


void parse_img_pref(AGTX_IMG_PREF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "brightness", &tmp_obj)) {
		data->brightness = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "brightness", data->brightness);
	}

	if (json_object_object_get_ex(cmd_obj, "hue", &tmp_obj)) {
		data->hue = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "hue", data->hue);
	}

	if (json_object_object_get_ex(cmd_obj, "saturation", &tmp_obj)) {
		data->saturation = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "saturation", data->saturation);
	}

	if (json_object_object_get_ex(cmd_obj, "contrast", &tmp_obj)) {
		data->contrast = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "contrast", data->contrast);
	}

	if (json_object_object_get_ex(cmd_obj, "sharpness", &tmp_obj)) {
		data->sharpness = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "sharpness", data->sharpness);
	}

	if (json_object_object_get_ex(cmd_obj, "anti_flicker", &tmp_obj)) {
		data->anti_flicker = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "anti_flicker", data->anti_flicker);
	}

	return;
}

void comp_img_pref(struct json_object *ret_obj, AGTX_IMG_PREF_S *data)
{
	struct json_object *tmp_obj  = NULL;

	tmp_obj = json_object_new_int(data->brightness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "brightness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "brightness");
	}

	tmp_obj = json_object_new_int(data->saturation);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "saturation", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "saturation");
	}

	tmp_obj = json_object_new_int(data->hue);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "hue", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "hue");
	}

	tmp_obj = json_object_new_int(data->contrast);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "contrast", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "contrast");
	}

	tmp_obj = json_object_new_int(data->sharpness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sharpness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sharpness");
	}

	tmp_obj = json_object_new_int(data->anti_flicker);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "anti_flicker", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "anti_flicker");
	}

	return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
