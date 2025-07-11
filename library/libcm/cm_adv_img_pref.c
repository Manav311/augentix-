#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_adv_img_pref.h"


const char * agtx_night_mode_e_map[] = {
	"OFF",
	"ON",
	"AUTO",
	"AUTOSWITCH"
};

const char * agtx_ir_led_mode_e_map[] = {
	"OFF",
	"ON",
	"AUTO"
};

const char * agtx_image_mode_e_map[] = {
	"COLOR",
	"GRAYSCALE",
	"AUTO"
};

const char * agtx_icr_mode_e_map[] = {
	"OFF",
	"ON",
	"AUTO"
};

void parse_adv_img_pref(AGTX_ADV_IMG_PREF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "backlight_compensation", &tmp_obj)) {
		data->backlight_compensation = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "icr_mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_icr_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_icr_mode_e_map[i], str) == 0) {
				data->icr_mode = (AGTX_ICR_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "image_mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_image_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_image_mode_e_map[i], str) == 0) {
				data->image_mode = (AGTX_IMAGE_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "ir_led_mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_ir_led_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_ir_led_mode_e_map[i], str) == 0) {
				data->ir_led_mode = (AGTX_IR_LED_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "ir_light_suppression", &tmp_obj)) {
		data->ir_light_suppression = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "night_mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_night_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_night_mode_e_map[i], str) == 0) {
				data->night_mode = (AGTX_NIGHT_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "wdr_en", &tmp_obj)) {
		data->wdr_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wdr_strength", &tmp_obj)) {
		data->wdr_strength = json_object_get_int(tmp_obj);
	}
}

void comp_adv_img_pref(struct json_object *ret_obj, AGTX_ADV_IMG_PREF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->backlight_compensation);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "backlight_compensation", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "backlight_compensation");
	}

	const char *str;
	str = agtx_icr_mode_e_map[data->icr_mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "icr_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "icr_mode");
	}

	str = agtx_image_mode_e_map[data->image_mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "image_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "image_mode");
	}

	str = agtx_ir_led_mode_e_map[data->ir_led_mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ir_led_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ir_led_mode");
	}

	tmp_obj = json_object_new_int(data->ir_light_suppression);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ir_light_suppression", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ir_light_suppression");
	}

	str = agtx_night_mode_e_map[data->night_mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "night_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "night_mode");
	}

	tmp_obj = json_object_new_int(data->wdr_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "wdr_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wdr_en");
	}

	tmp_obj = json_object_new_int(data->wdr_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "wdr_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wdr_strength");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
