#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_video_ptz_conf.h"


const char * agtx_video_ptz_mode_e_map[] = {
	"AUTO",
	"MANUAL",
	"SCAN"
};

void parse_subwin_param(AGTX_SUBWIN_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "chn_idx", &tmp_obj)) {
		data->chn_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_idx", &tmp_obj)) {
		data->win_idx = json_object_get_int(tmp_obj);
	}
}

void parse_subwin_disp(AGTX_SUBWIN_DISP_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "win", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_SUBWIN_DISP_S_WIN_SIZE; i++) {
			parse_subwin_param(&(data->win[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "win_num", &tmp_obj)) {
		data->win_num = json_object_get_int(tmp_obj);
	}
}

void parse_video_ptz_conf(AGTX_VIDEO_PTZ_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_video_ptz_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_video_ptz_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_VIDEO_PTZ_MODE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "roi_height", &tmp_obj)) {
		data->roi_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_width", &tmp_obj)) {
		data->roi_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "speed_x", &tmp_obj)) {
		data->speed_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "speed_y", &tmp_obj)) {
		data->speed_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "subwindow_disp", &tmp_obj)) {
		parse_subwin_disp(&(data->subwindow_disp), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_pos_x", &tmp_obj)) {
		data->win_pos_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_pos_y", &tmp_obj)) {
		data->win_pos_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_size_limit_max", &tmp_obj)) {
		data->win_size_limit_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_size_limit_min", &tmp_obj)) {
		data->win_size_limit_min = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_speed_x", &tmp_obj)) {
		data->win_speed_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_speed_y", &tmp_obj)) {
		data->win_speed_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zoom_change", &tmp_obj)) {
		data->zoom_change = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zoom_level", &tmp_obj)) {
		data->zoom_level = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zoom_speed_height", &tmp_obj)) {
		data->zoom_speed_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zoom_speed_width", &tmp_obj)) {
		data->zoom_speed_width = json_object_get_int(tmp_obj);
	}
}

void comp_subwin_param(struct json_object *array_obj, AGTX_SUBWIN_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->chn_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "chn_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "chn_idx");
		}

		tmp1_obj = json_object_new_int(data->win_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "win_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "win_idx");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_subwin_disp(struct json_object *ret_obj, AGTX_SUBWIN_DISP_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_SUBWIN_DISP_S_WIN_SIZE; i++) {
			comp_subwin_param(tmp_obj, &(data->win[i]));
		}
		json_object_object_add(ret_obj, "win", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "win");
	}

	tmp_obj = json_object_new_int(data->win_num);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_num", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_num");
	}

}

void comp_video_ptz_conf(struct json_object *ret_obj, AGTX_VIDEO_PTZ_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	const char *str;
	str = agtx_video_ptz_mode_e_map[data->mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->roi_height);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "roi_height", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi_height");
	}

	tmp_obj = json_object_new_int(data->roi_width);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "roi_width", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi_width");
	}

	tmp_obj = json_object_new_int(data->speed_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "speed_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "speed_x");
	}

	tmp_obj = json_object_new_int(data->speed_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "speed_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "speed_y");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_subwin_disp(tmp_obj, &(data->subwindow_disp));
		json_object_object_add(ret_obj, "subwindow_disp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "subwindow_disp");
	}

	tmp_obj = json_object_new_int(data->win_pos_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_pos_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_pos_x");
	}

	tmp_obj = json_object_new_int(data->win_pos_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_pos_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_pos_y");
	}

	tmp_obj = json_object_new_int(data->win_size_limit_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_size_limit_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_size_limit_max");
	}

	tmp_obj = json_object_new_int(data->win_size_limit_min);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_size_limit_min", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_size_limit_min");
	}

	tmp_obj = json_object_new_int(data->win_speed_x);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_speed_x", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_speed_x");
	}

	tmp_obj = json_object_new_int(data->win_speed_y);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_speed_y", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_speed_y");
	}

	tmp_obj = json_object_new_int(data->zoom_change);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "zoom_change", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "zoom_change");
	}

	tmp_obj = json_object_new_int(data->zoom_level);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "zoom_level", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "zoom_level");
	}

	tmp_obj = json_object_new_int(data->zoom_speed_height);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "zoom_speed_height", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "zoom_speed_height");
	}

	tmp_obj = json_object_new_int(data->zoom_speed_width);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "zoom_speed_width", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "zoom_speed_width");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
