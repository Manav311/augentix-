#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_nr_conf.h"

const char *agtx_nr_lut_type_map[] = { "LUT_TYPE_0", "LUT_TYPE_1", "LUT_TYPE_2", "LUT_TYPE_3" };

void parse_dip_nr_conf(AGTX_DIP_NR_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	const char *str;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_c_level_2d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_2D_LIST_SIZE; i++) {
			data->auto_c_level_2d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_c_level_3d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_3D_LIST_SIZE; i++) {
			data->auto_c_level_3d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_fss_y_level_3d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE; i++) {
			data->auto_fss_y_level_3d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_level_2d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_2D_LIST_SIZE; i++) {
			data->auto_y_level_2d_list[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_y_level_3d_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_3D_LIST_SIZE; i++) {
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
		for (i = 0; (unsigned long)i < (sizeof(agtx_nr_lut_type_map) / sizeof(char *)); i++) {
			if (!strcmp(agtx_nr_lut_type_map[i], str)) {
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
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_nr_conf(struct json_object *ret_obj, AGTX_DIP_NR_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_2D_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_c_level_2d_list[i]));
		}
		json_object_object_add(ret_obj, "auto_c_level_2d_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_c_level_2d_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_C_LEVEL_3D_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_c_level_3d_list[i]));
		}
		json_object_object_add(ret_obj, "auto_c_level_3d_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_c_level_3d_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_FSS_Y_LEVEL_3D_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_fss_y_level_3d_list[i]));
		}
		json_object_object_add(ret_obj, "auto_fss_y_level_3d_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_fss_y_level_3d_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_2D_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_level_2d_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_level_2d_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_level_2d_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_NR_CONF_S_AUTO_Y_LEVEL_3D_LIST_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_y_level_3d_list[i]));
		}
		json_object_object_add(ret_obj, "auto_y_level_3d_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_y_level_3d_list");
	}

	tmp_obj = json_object_new_int(data->fss_ratio_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "fss_ratio_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "fss_ratio_max");
	}

	tmp_obj = json_object_new_int(data->fss_ratio_min);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "fss_ratio_min", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "fss_ratio_min");
	}

	tmp_obj = json_object_new_int(data->ghost_remove);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ghost_remove", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ghost_remove");
	}

	const char *str;
	str = agtx_nr_lut_type_map[data->lut_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "lut_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "lut_type");
	}

	tmp_obj = json_object_new_int(data->ma_c_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ma_c_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ma_c_strength");
	}

	tmp_obj = json_object_new_int(data->ma_y_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ma_y_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ma_y_strength");
	}

	tmp_obj = json_object_new_int(data->manual_c_level_2d);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_c_level_2d", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_c_level_2d");
	}

	tmp_obj = json_object_new_int(data->manual_c_level_3d);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_c_level_3d", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_c_level_3d");
	}

	tmp_obj = json_object_new_int(data->manual_fss_y_level_3d);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_fss_y_level_3d", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_fss_y_level_3d");
	}

	tmp_obj = json_object_new_int(data->manual_y_level_2d);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_level_2d", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_level_2d");
	}

	tmp_obj = json_object_new_int(data->manual_y_level_3d);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_y_level_3d", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_y_level_3d");
	}

	tmp_obj = json_object_new_int(data->mc_y_level_offset);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mc_y_level_offset", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mc_y_level_offset");
	}

	tmp_obj = json_object_new_int(data->mc_y_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mc_y_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mc_y_strength");
	}

	tmp_obj = json_object_new_int(data->me_frame_fallback_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "me_frame_fallback_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "me_frame_fallback_en");
	}

	tmp_obj = json_object_new_int(data->mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->motion_comp);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "motion_comp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "motion_comp");
	}

	tmp_obj = json_object_new_int(data->ratio_3d);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ratio_3d", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ratio_3d");
	}

	tmp_obj = json_object_new_int(data->trail_suppress);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "trail_suppress", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "trail_suppress");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
