#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_bm_conf.h"


const char * agtx_iva_fd_ctrl_e_map[] = {
	"NONE",
	"SAVE",
	"LOAD"
};

void parse_iva_bm_region(AGTX_IVA_BM_REGION_S *data, struct json_object *cmd_obj)
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

void parse_iva_bm_conf(AGTX_IVA_BM_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "boundary_thickness", &tmp_obj)) {
		data->boundary_thickness = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "data_ctrl", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_fd_ctrl_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_fd_ctrl_e_map[i], str) == 0) {
				data->data_ctrl = (AGTX_IVA_FD_CTRL_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "quality", &tmp_obj)) {
		data->quality = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "reset", &tmp_obj)) {
		data->reset = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi", &tmp_obj)) {
		parse_iva_bm_region(&(data->roi), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensitivity", &tmp_obj)) {
		data->sensitivity = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "suppression", &tmp_obj)) {
		data->suppression = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "time_buffer", &tmp_obj)) {
		data->time_buffer = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_bm_region(struct json_object *ret_obj, AGTX_IVA_BM_REGION_S *data)
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

void comp_iva_bm_conf(struct json_object *ret_obj, AGTX_IVA_BM_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->boundary_thickness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "boundary_thickness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "boundary_thickness");
	}

	const char *str;
	str = agtx_iva_fd_ctrl_e_map[data->data_ctrl];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "data_ctrl", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "data_ctrl");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->quality);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "quality", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "quality");
	}

	tmp_obj = json_object_new_int(data->reset);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "reset", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "reset");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_iva_bm_region(tmp_obj, &(data->roi));
		json_object_object_add(ret_obj, "roi", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi");
	}

	tmp_obj = json_object_new_int(data->sensitivity);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensitivity", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensitivity");
	}

	tmp_obj = json_object_new_int(data->suppression);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "suppression", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "suppression");
	}

	tmp_obj = json_object_new_int(data->time_buffer);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "time_buffer", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "time_buffer");
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
