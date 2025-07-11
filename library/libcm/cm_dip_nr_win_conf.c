#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_nr_win_conf.h"

const char *agtx_nr_win_lut_type_map[] = { "LUT_TYPE_0", "LUT_TYPE_1", "LUT_TYPE_2" };

void parse_nr_window_param(AGTX_NR_WINDOW_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	const char *str;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_c_level_2d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_2D_LIST_SIZE; i++) {
			data->auto_c_level_2d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_c_level_3d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_3D_LIST_SIZE; i++) {
			data->auto_c_level_3d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_fss_y_level_3d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE; i++) {
			data->auto_fss_y_level_3d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_level_2d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_2D_LIST_SIZE; i++) {
			data->auto_y_level_2d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_level_3d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_3D_LIST_SIZE; i++) {
			data->auto_y_level_3d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "fss_ratio_max", &tmp_obj)) {
		data->fss_ratio_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "fss_ratio_min", &tmp_obj)) {
		data->fss_ratio_min = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ghost_remove", &tmp_obj)) {
		data->ghost_remove = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "lut_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < (sizeof(agtx_nr_win_lut_type_map) / sizeof(char *)); i++) {
			if (!strcmp(agtx_nr_win_lut_type_map[i], str)) {
				data->lut_type = (AGTX_NR_LUT_TYPE_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "ma_c_strength", &tmp_obj)) {
		data->ma_c_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ma_y_strength", &tmp_obj)) {
		data->ma_y_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_c_level_2d", &tmp_obj)) {
		data->manual_c_level_2d = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_c_level_3d", &tmp_obj)) {
		data->manual_c_level_3d = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_fss_y_level_3d", &tmp_obj)) {
		data->manual_fss_y_level_3d = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_level_2d", &tmp_obj)) {
		data->manual_y_level_2d = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_y_level_3d", &tmp_obj)) {
		data->manual_y_level_3d = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mc_y_level_offset", &tmp_obj)) {
		data->mc_y_level_offset = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mc_y_strength", &tmp_obj)) {
		data->mc_y_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "me_frame_fallback_en", &tmp_obj)) {
		data->me_frame_fallback_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "motion_comp", &tmp_obj)) {
		data->motion_comp = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ratio_3d", &tmp_obj)) {
		data->ratio_3d = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "trail_suppress", &tmp_obj)) {
		data->trail_suppress = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "window_idx", &tmp_obj)) {
		data->window_idx = json_object_get_int(tmp_obj);
	}
}

void parse_nr_strm_param(AGTX_NR_STRM_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "video_strm_idx", &tmp_obj)) {
		data->video_strm_idx = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "window_array", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_NR_STRM_PARAM_S_WINDOW_ARRAY_SIZE; i++) {
			parse_nr_window_param(&(data->window_array[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "window_num", &tmp_obj)) {
		data->window_num = json_object_get_int(tmp_obj);
	}
}

void parse_dip_nr_win_conf(AGTX_DIP_NR_WIN_CONF_S *data, struct json_object *cmd_obj)
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
		for (i = 0; i < MAX_AGTX_DIP_NR_WIN_CONF_S_VIDEO_STRM_SIZE; i++) {
			parse_nr_strm_param(&(data->video_strm[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "win_nr_en", &tmp_obj)) {
		data->win_nr_en = json_object_get_int(tmp_obj);
	}
}

void comp_nr_window_param(struct json_object *array_obj, AGTX_NR_WINDOW_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_2D_LIST_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_c_level_2d_list[i]));
			}
			json_object_object_add(tmp_obj, "auto_c_level_2d_list", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_c_level_2d_list");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_C_LEVEL_3D_LIST_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_c_level_3d_list[i]));
			}
			json_object_object_add(tmp_obj, "auto_c_level_3d_list", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_c_level_3d_list");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_fss_y_level_3d_list[i]));
			}
			json_object_object_add(tmp_obj, "auto_fss_y_level_3d_list", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_fss_y_level_3d_list");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_2D_LIST_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_y_level_2d_list[i]));
			}
			json_object_object_add(tmp_obj, "auto_y_level_2d_list", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_y_level_2d_list");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_NR_WINDOW_PARAM_S_AUTO_Y_LEVEL_3D_LIST_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_y_level_3d_list[i]));
			}
			json_object_object_add(tmp_obj, "auto_y_level_3d_list", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_y_level_3d_list");
		}

		tmp1_obj = json_object_new_int(data->fss_ratio_max);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "fss_ratio_max", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "fss_ratio_max");
		}

		tmp1_obj = json_object_new_int(data->fss_ratio_min);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "fss_ratio_min", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "fss_ratio_min");
		}

		tmp1_obj = json_object_new_int(data->ghost_remove);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ghost_remove", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ghost_remove");
		}

		const char *str;
		str = agtx_nr_win_lut_type_map[data->lut_type];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "lut_type", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "lut_type");
		}

		tmp1_obj = json_object_new_int(data->ma_c_strength);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ma_c_strength", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ma_c_strength");
		}

		tmp1_obj = json_object_new_int(data->ma_y_strength);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ma_y_strength", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ma_y_strength");
		}

		tmp1_obj = json_object_new_int(data->manual_c_level_2d);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "manual_c_level_2d", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "manual_c_level_2d");
		}

		tmp1_obj = json_object_new_int(data->manual_c_level_3d);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "manual_c_level_3d", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "manual_c_level_3d");
		}

		tmp1_obj = json_object_new_int(data->manual_fss_y_level_3d);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "manual_fss_y_level_3d", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "manual_fss_y_level_3d");
		}

		tmp1_obj = json_object_new_int(data->manual_y_level_2d);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "manual_y_level_2d", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "manual_y_level_2d");
		}

		tmp1_obj = json_object_new_int(data->manual_y_level_3d);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "manual_y_level_3d", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "manual_y_level_3d");
		}

		tmp1_obj = json_object_new_int(data->mc_y_level_offset);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mc_y_level_offset", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mc_y_level_offset");
		}

		tmp1_obj = json_object_new_int(data->mc_y_strength);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mc_y_strength", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mc_y_strength");
		}

		tmp1_obj = json_object_new_int(data->me_frame_fallback_en);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "me_frame_fallback_en", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "me_frame_fallback_en");
		}

		tmp1_obj = json_object_new_int(data->mode);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mode", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mode");
		}

		tmp1_obj = json_object_new_int(data->motion_comp);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "motion_comp", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "motion_comp");
		}

		tmp1_obj = json_object_new_int(data->ratio_3d);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "ratio_3d", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "ratio_3d");
		}

		tmp1_obj = json_object_new_int(data->trail_suppress);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "trail_suppress", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "trail_suppress");
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

void comp_nr_strm_param(struct json_object *array_obj, AGTX_NR_STRM_PARAM_S *data)
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
			for (i = 0; i < MAX_AGTX_NR_STRM_PARAM_S_WINDOW_ARRAY_SIZE; i++) {
				comp_nr_window_param(tmp1_obj, &(data->window_array[i]));
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

void comp_dip_nr_win_conf(struct json_object *ret_obj, AGTX_DIP_NR_WIN_CONF_S *data)
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
		for (i = 0; i < MAX_AGTX_DIP_NR_WIN_CONF_S_VIDEO_STRM_SIZE; i++) {
			comp_nr_strm_param(tmp_obj, &(data->video_strm[i]));
		}
		json_object_object_add(ret_obj, "video_strm", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "video_strm");
	}

	tmp_obj = json_object_new_int(data->win_nr_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "win_nr_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "win_nr_en");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
