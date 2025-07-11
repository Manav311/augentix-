#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_ld_conf.h"


const char * agtx_ld_trig_cond_e_map[] = {
	"LIGHT_NONE",
	"LIGHT_ON",
	"LIGHT_OFF",
	"BOTH"
};

void parse_iva_ld_region(AGTX_IVA_LD_REGION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "end_x", &tmp_obj)) {
		data->end_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "end_y", &tmp_obj)) {
		data->end_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_x", &tmp_obj)) {
		data->start_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_y", &tmp_obj)) {
		data->start_y = json_object_get_int(tmp_obj);
	}
}

void parse_iva_ld_conf(AGTX_IVA_LD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "det_region", &tmp_obj)) {
		parse_iva_ld_region(&(data->det_region), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "trigger_cond", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_ld_trig_cond_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_ld_trig_cond_e_map[i], str) == 0) {
				data->trigger_cond = (AGTX_LD_TRIG_COND_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_ld_region(struct json_object *ret_obj, AGTX_IVA_LD_REGION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->end_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "end_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "end_x");
	}

	tmp_obj = json_object_new_int(data->end_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "end_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "end_y");
	}

	tmp_obj = json_object_new_int(data->start_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "start_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "start_x");
	}

	tmp_obj = json_object_new_int(data->start_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "start_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "start_y");
	}

}

void comp_iva_ld_conf(struct json_object *ret_obj, AGTX_IVA_LD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_iva_ld_region(tmp_obj, &(data->det_region));
		json_object_object_add(ret_obj, "det_region", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "det_region");
	}

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

	const char *str;
	str = agtx_ld_trig_cond_e_map[data->trigger_cond];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trigger_cond", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trigger_cond");
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
