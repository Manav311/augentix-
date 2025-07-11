#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_iva_td_conf.h"


void parse_iva_td_conf(AGTX_IVA_TD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_block_det", &tmp_obj)) {
		data->en_block_det = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_redirect_det", &tmp_obj)) {
		data->en_redirect_det = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "endurance", &tmp_obj)) {
		data->endurance = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "redirect_sensitivity", &tmp_obj)) {
		data->redirect_sensitivity = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "redirect_global_change", &tmp_obj)) {
		data->redirect_global_change = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "redirect_trigger_delay", &tmp_obj)) {
		data->redirect_trigger_delay = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_td_conf(struct json_object *ret_obj, AGTX_IVA_TD_CONF_S *data)
{
	struct json_object *tmp_obj;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->en_block_det);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_block_det", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_block_det");
	}

	tmp_obj = json_object_new_int(data->en_redirect_det);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_redirect_det", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_redirect_det");
	}

	tmp_obj = json_object_new_int(data->endurance);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "endurance", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "endurance");
	}

	tmp_obj = json_object_new_int(data->sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensitivity");
	}

	tmp_obj = json_object_new_int(data->redirect_sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "redirect_sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "redirect_sensitivity");
	}

	tmp_obj = json_object_new_int(data->redirect_global_change);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "redirect_global_change", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "redirect_global_change");
	}

	tmp_obj = json_object_new_int(data->redirect_trigger_delay);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "redirect_trigger_delay", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "redirect_trigger_delay");
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
