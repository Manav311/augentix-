#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_stat_conf.h"

void parse_dip_stat_wb_conf(AGTX_DIP_STAT_WB_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "lum_max", &tmp_obj)) {
		data->lum_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "lum_min", &tmp_obj)) {
		data->lum_min = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "lum_slope", &tmp_obj)) {
		data->lum_slope = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "rb_point_x", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_X_SIZE; i++) {
			data->rb_point_x[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "rb_point_y", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_Y_SIZE; i++) {
			data->rb_point_y[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "rb_rgn_slope", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_SLOPE_SIZE; i++) {
			data->rb_rgn_slope[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "rb_rgn_th", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_TH_SIZE; i++) {
			data->rb_rgn_th[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_lum_thd_enable", &tmp_obj)) {
		data->gwd_auto_lum_thd_enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_lum_max_degree", &tmp_obj)) {
		data->gwd_auto_lum_max_degree = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_indoor_ev_thd", &tmp_obj)) {
		data->gwd_auto_indoor_ev_thd = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_outdoor_ev_thd", &tmp_obj)) {
		data->gwd_auto_outdoor_ev_thd = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_indoor_lum_range", &tmp_obj)) {
		data->gwd_auto_indoor_lum_range = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_outdoor_lum_range", &tmp_obj)) {
		data->gwd_auto_outdoor_lum_range = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gwd_auto_min_lum_bnd", &tmp_obj)) {
		data->gwd_auto_min_lum_bnd = json_object_get_int(tmp_obj);
	}
}

void parse_dip_stat_conf(AGTX_DIP_STAT_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wb", &tmp_obj)) {
		parse_dip_stat_wb_conf(&(data->wb), tmp_obj);
	}
}

void comp_dip_stat_wb_conf(struct json_object *ret_obj, AGTX_DIP_STAT_WB_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->lum_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "lum_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "lum_max");
	}

	tmp_obj = json_object_new_int(data->lum_min);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "lum_min", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "lum_min");
	}

	tmp_obj = json_object_new_int(data->lum_slope);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "lum_slope", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "lum_slope");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_X_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->rb_point_x[i]));
		}
		json_object_object_add(ret_obj, "rb_point_x", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "rb_point_x");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_POINT_Y_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->rb_point_y[i]));
		}
		json_object_object_add(ret_obj, "rb_point_y", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "rb_point_y");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_SLOPE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->rb_rgn_slope[i]));
		}
		json_object_object_add(ret_obj, "rb_rgn_slope", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "rb_rgn_slope");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_STAT_WB_CONF_S_RB_RGN_TH_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->rb_rgn_th[i]));
		}
		json_object_object_add(ret_obj, "rb_rgn_th", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "rb_rgn_th");
	}

	tmp_obj = json_object_new_int(data->gwd_auto_lum_thd_enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_lum_thd_enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gwd_auto_lum_thd_enable");
	}

	tmp_obj = json_object_new_int(data->gwd_auto_lum_max_degree);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_lum_max_degree", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "lum_max_degree");
	}

	tmp_obj = json_object_new_int64(data->gwd_auto_indoor_ev_thd);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_indoor_ev_thd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "indoor_ev_thd");
	}

	tmp_obj = json_object_new_int64(data->gwd_auto_outdoor_ev_thd);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_outdoor_ev_thd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "outdoor_ev_thd");
	}

	tmp_obj = json_object_new_int(data->gwd_auto_indoor_lum_range);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_indoor_lum_range", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "indoor_lum_range");
	}

	tmp_obj = json_object_new_int(data->gwd_auto_outdoor_lum_range);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_outdoor_lum_range", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "outdoor_lum_range");
	}

	tmp_obj = json_object_new_int(data->gwd_auto_min_lum_bnd);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gwd_auto_min_lum_bnd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_lum_bnd");
	}
}

void comp_dip_stat_conf(struct json_object *ret_obj, AGTX_DIP_STAT_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_stat_wb_conf(tmp_obj, &(data->wb));
		json_object_object_add(ret_obj, "wb", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wb");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
