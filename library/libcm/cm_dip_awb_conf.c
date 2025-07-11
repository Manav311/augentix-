#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_awb_conf.h"

const char *agtx_awb_ccm_domain_map[] = { "GAMMA_ENCODED", "LINEAR" };

void parse_dip_awb_color_temp(AGTX_DIP_AWB_COLOR_TEMP_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "gain", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_COLOR_TEMP_S_GAIN_SIZE; i++) {
			data->gain[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "k", &tmp_obj)) {
		data->k = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "maxtrix", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_COLOR_TEMP_S_MAXTRIX_SIZE; i++) {
			data->maxtrix[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_dip_awb_color_temp_bias(AGTX_DIP_AWB_COLOR_TEMP_BIAS_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "b_extra_gain_bias", &tmp_obj)) {
		data->b_extra_gain_bias = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "color_tolerance_bias", &tmp_obj)) {
		data->color_tolerance_bias = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "g_extra_gain_bias", &tmp_obj)) {
		data->g_extra_gain_bias = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_weight_bias", &tmp_obj)) {
		data->gwd_weight_bias = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "k", &tmp_obj)) {
		data->k = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "r_extra_gain_bias", &tmp_obj)) {
		data->r_extra_gain_bias = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wht_weight_bias", &tmp_obj)) {
		data->wht_weight_bias = json_object_get_int(tmp_obj);
	}
}

void parse_dip_awb_color_delta(AGTX_DIP_AWB_COLOR_DELTA_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "gain", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_COLOR_DELTA_S_GAIN_SIZE; i++) {
			data->gain[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_dip_awb_conf(AGTX_DIP_AWB_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "b_extra_gain", &tmp_obj)) {
		data->b_extra_gain = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "color_tolerance", &tmp_obj)) {
		data->color_tolerance = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "delta_table_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_CONF_S_DELTA_TABLE_LIST_SIZE; i++) {
			parse_dip_awb_color_delta(&(data->delta_table_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "g_extra_gain", &tmp_obj)) {
		data->g_extra_gain = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_weight", &tmp_obj)) {
		data->gwd_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "high_k", &tmp_obj)) {
		data->high_k = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "k_table_bias_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_BIAS_LIST_SIZE; i++) {
			parse_dip_awb_color_temp_bias(&(data->k_table_bias_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "k_table_list", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_LIST_SIZE; i++) {
			parse_dip_awb_color_temp(&(data->k_table_list[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "k_table_valid_size", &tmp_obj)) {
		data->k_table_valid_size = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "low_k", &tmp_obj)) {
		data->low_k = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_lum_gain", &tmp_obj)) {
		data->max_lum_gain = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "over_exp_th", &tmp_obj)) {
		data->over_exp_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "r_extra_gain", &tmp_obj)) {
		data->r_extra_gain = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "speed", &tmp_obj)) {
		data->speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wht_density", &tmp_obj)) {
		data->wht_density = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wht_weight", &tmp_obj)) {
		data->wht_weight = json_object_get_int(tmp_obj);
	}
	const char *str;
	if (json_object_object_get_ex(cmd_obj, "ccm_domain", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_awb_ccm_domain_map) / sizeof(char *); i++) {
			if (strcmp(agtx_awb_ccm_domain_map[i], str) == 0) {
				data->ccm_domain = (AGTX_AWB_CCM_DOMAIN)i;
				break;
			}
		}
	}
}

void comp_dip_awb_color_temp(struct json_object *array_obj, AGTX_DIP_AWB_COLOR_TEMP_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		tmp1_obj = json_object_new_int(data->k);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "k", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "k");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_AWB_COLOR_TEMP_S_GAIN_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->gain[i]));
			}
			json_object_object_add(tmp_obj, "gain", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "gain");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_AWB_COLOR_TEMP_S_MAXTRIX_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->maxtrix[i]));
			}
			json_object_object_add(tmp_obj, "maxtrix", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "maxtrix");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_awb_color_temp_bias(struct json_object *array_obj, AGTX_DIP_AWB_COLOR_TEMP_BIAS_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->k);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "k", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "k");
		}

		tmp1_obj = json_object_new_int(data->color_tolerance_bias);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "color_tolerance_bias", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "color_tolerance_bias");
		}

		tmp1_obj = json_object_new_int(data->wht_weight_bias);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "wht_weight_bias", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "wht_weight_bias");
		}

		tmp1_obj = json_object_new_int(data->gwd_weight_bias);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "gwd_weight_bias", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "gwd_weight_bias");
		}

		tmp1_obj = json_object_new_int(data->r_extra_gain_bias);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "r_extra_gain_bias", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "r_extra_gain_bias");
		}

		tmp1_obj = json_object_new_int(data->g_extra_gain_bias);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "g_extra_gain_bias", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "g_extra_gain_bias");
		}

		tmp1_obj = json_object_new_int(data->b_extra_gain_bias);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "b_extra_gain_bias", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "b_extra_gain_bias");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_awb_color_delta(struct json_object *array_obj, AGTX_DIP_AWB_COLOR_DELTA_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_AWB_COLOR_DELTA_S_GAIN_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->gain[i]));
			}
			json_object_object_add(tmp_obj, "gain", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "gain");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_awb_conf(struct json_object *ret_obj, AGTX_DIP_AWB_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	tmp_obj = json_object_new_int(data->speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "speed");
	}

	tmp_obj = json_object_new_int(data->low_k);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "low_k", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "low_k");
	}

	tmp_obj = json_object_new_int(data->high_k);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "high_k", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "high_k");
	}

	tmp_obj = json_object_new_int(data->r_extra_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "r_extra_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "r_extra_gain");
	}

	tmp_obj = json_object_new_int(data->g_extra_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "g_extra_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "g_extra_gain");
	}

	tmp_obj = json_object_new_int(data->b_extra_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "b_extra_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "b_extra_gain");
	}

	tmp_obj = json_object_new_int(data->max_lum_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_lum_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_lum_gain");
	}

	tmp_obj = json_object_new_int(data->wht_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "wht_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wht_weight");
	}

	tmp_obj = json_object_new_int(data->gwd_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gwd_weight");
	}

	tmp_obj = json_object_new_int(data->color_tolerance);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "color_tolerance", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "color_tolerance");
	}

	tmp_obj = json_object_new_int(data->wht_density);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "wht_density", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wht_density");
	}

	tmp_obj = json_object_new_int(data->over_exp_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "over_exp_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "over_exp_th");
	}

	const char *str;
	str = agtx_awb_ccm_domain_map[data->ccm_domain];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "ccm_domain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "ccm_domain");
	}

	tmp_obj = json_object_new_int(data->k_table_valid_size);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "k_table_valid_size", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "k_table_valid_size");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_LIST_SIZE; i++) {
			comp_dip_awb_color_temp(tmp_obj, &(data->k_table_list[i]));
		}
		json_object_object_add(ret_obj, "k_table_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "k_table_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_CONF_S_K_TABLE_BIAS_LIST_SIZE; i++) {
			comp_dip_awb_color_temp_bias(tmp_obj, &(data->k_table_bias_list[i]));
		}
		json_object_object_add(ret_obj, "k_table_bias_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "k_table_bias_list");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_AWB_CONF_S_DELTA_TABLE_LIST_SIZE; i++) {
			comp_dip_awb_color_delta(tmp_obj, &(data->delta_table_list[i]));
		}
		json_object_object_add(ret_obj, "delta_table_list", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "delta_table_list");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
