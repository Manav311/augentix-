#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_shp_win_conf.h"


void parse_shp_window_param(AGTX_SHP_WINDOW_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "strength", &tmp_obj)) {
		data->strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "window_idx", &tmp_obj)) {
		data->window_idx = json_object_get_int(tmp_obj);
	}
}

void parse_shp_strm_param(AGTX_SHP_STRM_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "video_strm_idx", &tmp_obj)) {
		data->video_strm_idx = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "window_array", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_SHP_STRM_PARAM_S_WINDOW_ARRAY_SIZE; i++) {
			parse_shp_window_param(&(data->window_array[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "window_num", &tmp_obj)) {
		data->window_num = json_object_get_int(tmp_obj);
	}
}

void parse_dip_shp_win_conf(AGTX_DIP_SHP_WIN_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "strm_num", &tmp_obj)) {
		data->strm_num = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "video_strm", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_WIN_CONF_S_VIDEO_STRM_SIZE; i++) {
			parse_shp_strm_param(&(data->video_strm[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "win_shp_en", &tmp_obj)) {
		data->win_shp_en = json_object_get_int(tmp_obj);
	}
}

void comp_shp_window_param(struct json_object *array_obj, AGTX_SHP_WINDOW_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->strength);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "strength", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "strength");
		}

		tmp1_obj = json_object_new_int(data->window_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "window_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "window_idx");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_shp_strm_param(struct json_object *array_obj, AGTX_SHP_STRM_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->video_strm_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "video_strm_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "video_strm_idx");
		}

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_SHP_STRM_PARAM_S_WINDOW_ARRAY_SIZE; i++) {
				comp_shp_window_param(tmp1_obj, &(data->window_array[i]));
			}
			json_object_object_add(tmp_obj, "window_array", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "window_array");
		}

		tmp1_obj = json_object_new_int(data->window_num);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "window_num", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "window_num");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_shp_win_conf(struct json_object *ret_obj, AGTX_DIP_SHP_WIN_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->strm_num);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "strm_num", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "strm_num");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_WIN_CONF_S_VIDEO_STRM_SIZE; i++) {
			comp_shp_strm_param(tmp_obj, &(data->video_strm[i]));
		}
		json_object_object_add(ret_obj, "video_strm", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "video_strm");
	}

	tmp_obj = json_object_new_int(data->win_shp_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_shp_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_shp_en");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
