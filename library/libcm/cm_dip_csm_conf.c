#include "cm_dip_csm_conf.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"

#include "agtx_dip_csm_conf.h"

static void parse_dip_cst_matrix(AGTX_DIP_CST_MATRIX_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "coeff", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_CST_MATRIX_S_COEFF_SIZE; i++) {
			data->coeff[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "offset", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_CST_MATRIX_S_OFFSET_SIZE; i++) {
			data->offset[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_dip_csm_conf(AGTX_DIP_CSM_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_sat_table", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_CSM_CONF_S_AUTO_SAT_TABLE_SIZE; i++) {
			data->auto_sat_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "bw_en", &tmp_obj)) {
		data->bw_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_0", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_0), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_1", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_1), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_10", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_10), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_2", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_2), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_3", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_3), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_4", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_4), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_5", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_5), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_6", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_6), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_7", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_7), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_8", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_8), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_9", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_auto_9), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_auto_en", &tmp_obj)) {
		data->cst_auto_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_bw", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_bw), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "cst_color", &tmp_obj)) {
		parse_dip_cst_matrix(&(data->cst_color), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "hue", &tmp_obj)) {
		data->hue = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_sat", &tmp_obj)) {
		data->manual_sat = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

static void comp_dip_cst_matrix(struct json_object *ret_obj, AGTX_DIP_CST_MATRIX_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_CST_MATRIX_S_COEFF_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->coeff[i]));
		}
		json_object_object_add(ret_obj, "coeff", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "coeff");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_CST_MATRIX_S_OFFSET_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->offset[i]));
		}
		json_object_object_add(ret_obj, "offset", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "offset");
	}
}

void comp_dip_csm_conf(struct json_object *ret_obj, AGTX_DIP_CSM_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_CSM_CONF_S_AUTO_SAT_TABLE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->auto_sat_table[i]));
		}
		json_object_object_add(ret_obj, "auto_sat_table", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "auto_sat_table");
	}

	tmp_obj = json_object_new_int(data->bw_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "bw_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "bw_en");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_0));
		json_object_object_add(ret_obj, "cst_auto_0", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_0");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_1));
		json_object_object_add(ret_obj, "cst_auto_1", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_1");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_10));
		json_object_object_add(ret_obj, "cst_auto_10", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_10");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_2));
		json_object_object_add(ret_obj, "cst_auto_2", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_2");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_3));
		json_object_object_add(ret_obj, "cst_auto_3", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_3");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_4));
		json_object_object_add(ret_obj, "cst_auto_4", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_4");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_5));
		json_object_object_add(ret_obj, "cst_auto_5", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_5");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_6));
		json_object_object_add(ret_obj, "cst_auto_6", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_6");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_7));
		json_object_object_add(ret_obj, "cst_auto_7", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_7");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_8));
		json_object_object_add(ret_obj, "cst_auto_8", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_8");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_auto_9));
		json_object_object_add(ret_obj, "cst_auto_9", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_9");
	}

	tmp_obj = json_object_new_int(data->cst_auto_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "cst_auto_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_auto_en");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_bw));
		json_object_object_add(ret_obj, "cst_bw", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_bw");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_cst_matrix(tmp_obj, &(data->cst_color));
		json_object_object_add(ret_obj, "cst_color", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "cst_color");
	}

	tmp_obj = json_object_new_int(data->hue);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "hue", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "hue");
	}

	tmp_obj = json_object_new_int(data->manual_sat);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "manual_sat", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual_sat");
	}

	tmp_obj = json_object_new_int(data->mode);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
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
