#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_shp_conf.h"

const char *agtx_shp_type_e_map[] = { "SHP_TYPE", "SHP_TYPE_EX" };

void parse_point_int32(AGTX_POINT_INT32_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "x", &tmp_obj)) {
		data->x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "y", &tmp_obj)) {
		data->y = json_object_get_int(tmp_obj);
	}
}

void parse_dip_shp_conf(AGTX_DIP_SHP_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_hpf_ratio", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_HPF_RATIO_SIZE; i++) {
			data->auto_hpf_ratio[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_0", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_0_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_0[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_1", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_1_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_1[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_10", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_10_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_10[i]),
			                  json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_2", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_2_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_2[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_3", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_3_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_3[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_4", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_4_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_4[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_5", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_5_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_5[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_6", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_6_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_6[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_7", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_7_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_7[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_8", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_8_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_8[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_luma_ctrl_gain_9", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_9_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_luma_ctrl_gain_9[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_table", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TABLE_SIZE; i++) {
			data->auto_shp_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_0", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_0_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_0[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_1", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_1_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_1[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_10", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_10_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_10[i]),
			                  json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_2", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_2_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_2[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_3", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_3_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_3[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_4", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_4_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_4[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_5", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_5_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_5[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_6", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_6_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_6[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_7", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_7_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_7[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_8", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_8_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_8[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_shp_transfer_curve_9", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_9_SIZE; i++) {
			parse_point_int32(&(data->auto_shp_transfer_curve_9[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_soft_clip_slope", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SOFT_CLIP_SLOPE_SIZE; i++) {
			data->auto_soft_clip_slope[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "ma_conf_high_th", &tmp_obj)) {
		data->ma_conf_high_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ma_conf_low_th", &tmp_obj)) {
		data->ma_conf_low_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "ma_weak_shp_ratio", &tmp_obj)) {
		data->ma_weak_shp_ratio = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_hpf_ratio", &tmp_obj)) {
		data->manual_hpf_ratio = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_shp", &tmp_obj)) {
		data->manual_shp = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_shp_luma_ctrl_gain", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_LUMA_CTRL_GAIN_SIZE; i++) {
			parse_point_int32(&(data->manual_shp_luma_ctrl_gain[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "manual_shp_transfer_curve", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_TRANSFER_CURVE_SIZE; i++) {
			parse_point_int32(&(data->manual_shp_transfer_curve[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "manual_soft_clip_slope", &tmp_obj)) {
		data->manual_soft_clip_slope = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "motion_adaptive_en", &tmp_obj)) {
		data->motion_adaptive_en = json_object_get_int(tmp_obj);
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "shp_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_shp_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_shp_type_e_map[i], str) == 0) {
				data->shp_type = (AGTX_SHP_TYPE_E)i;
				break;
			}
		}
	}

	if (json_object_object_get_ex(cmd_obj, "strength", &tmp_obj)) {
		data->strength = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_point_int32(struct json_object *array_obj, AGTX_POINT_INT32_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "x");
		}

		tmp1_obj = json_object_new_int(data->y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "y");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_shp_conf(struct json_object *ret_obj, AGTX_DIP_SHP_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_HPF_RATIO_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_hpf_ratio[i]));
		}
		json_object_object_add(ret_obj, "auto_hpf_ratio", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_hpf_ratio");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_0_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_0[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_0", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_0");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_1_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_1[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_1", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_1");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_10_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_10[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_10", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_10");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_2_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_2[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_2", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_2");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_3_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_3[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_3", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_3");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_4_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_4[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_4", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_4");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_5_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_5[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_5", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_5");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_6_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_6[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_6", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_6");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_7_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_7[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_7", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_7");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_8_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_8[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_8", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_8");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_LUMA_CTRL_GAIN_9_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_luma_ctrl_gain_9[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_luma_ctrl_gain_9", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_luma_ctrl_gain_9");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TABLE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_shp_table[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_table", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_table");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_0_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_0[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_0", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_0");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_1_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_1[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_1", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_1");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_10_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_10[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_10", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_10");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_2_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_2[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_2", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_2");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_3_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_3[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_3", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_3");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_4_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_4[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_4", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_4");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_5_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_5[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_5", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_5");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_6_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_6[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_6", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_6");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_7_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_7[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_7", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_7");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_8_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_8[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_8", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_8");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SHP_TRANSFER_CURVE_9_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->auto_shp_transfer_curve_9[i]));
		}
		json_object_object_add(ret_obj, "auto_shp_transfer_curve_9", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_shp_transfer_curve_9");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_AUTO_SOFT_CLIP_SLOPE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_soft_clip_slope[i]));
		}
		json_object_object_add(ret_obj, "auto_soft_clip_slope", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_soft_clip_slope");
	}

	tmp_obj = json_object_new_int(data->ma_conf_high_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ma_conf_high_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ma_conf_high_th");
	}

	tmp_obj = json_object_new_int(data->ma_conf_low_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ma_conf_low_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ma_conf_low_th");
	}

	tmp_obj = json_object_new_int(data->ma_weak_shp_ratio);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ma_weak_shp_ratio", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ma_weak_shp_ratio");
	}

	tmp_obj = json_object_new_int(data->manual_hpf_ratio);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_hpf_ratio", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_hpf_ratio");
	}

	tmp_obj = json_object_new_int(data->manual_shp);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_shp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_shp");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_LUMA_CTRL_GAIN_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->manual_shp_luma_ctrl_gain[i]));
		}
		json_object_object_add(ret_obj, "manual_shp_luma_ctrl_gain", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "manual_shp_luma_ctrl_gain");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_SHP_CONF_S_MANUAL_SHP_TRANSFER_CURVE_SIZE; i++) {
			comp_point_int32(tmp_obj, &(data->manual_shp_transfer_curve[i]));
		}
		json_object_object_add(ret_obj, "manual_shp_transfer_curve", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "manual_shp_transfer_curve");
	}

	tmp_obj = json_object_new_int(data->manual_soft_clip_slope);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_soft_clip_slope", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_soft_clip_slope");
	}

	tmp_obj = json_object_new_int(data->mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	tmp_obj = json_object_new_int(data->motion_adaptive_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "motion_adaptive_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "motion_adaptive_en");
	}

	const char *str;
	str = agtx_shp_type_e_map[data->shp_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "shp_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "shp_type");
	}

	tmp_obj = json_object_new_int(data->strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "strength");
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
