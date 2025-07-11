#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_ae_conf.h"

const char *agtx_ae_zone_weight_table_mode_map[] = { "AVG", "CENTRAL", "SPOT", "MANUAL" };

const char *agtx_fps_mode_map[] = { "VARIABLE", "FIXED" };

const char * agtx_exp_strategy_map[] = {
	"NORMAL",
	"HI_LIGHT_SUPPRES"
};

void parse_dip_ae_zone_weight_table_conf(AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_ae_zone_weight_table_mode_map) / sizeof(char *); i++) {
			if (strcmp(agtx_ae_zone_weight_table_mode_map[i], str) == 0) {
				data->mode = (AGTX_AE_ZONE_WEIGHT_TABLE_MODE)i;
				break;
			}
		}
	}

	if (json_object_object_get_ex(cmd_obj, "manual_table", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S_MANUAL_TABLE_SIZE; i++) {
			data->manual_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_dip_ae_manual_conf(AGTX_DIP_AE_MANUAL_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "exp_value", &tmp_obj)) {
		data->exp_value = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "flag", &tmp_obj)) {
		data->flag = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "inttime", &tmp_obj)) {
		data->inttime = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "isp_gain", &tmp_obj)) {
		data->isp_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sensor_gain", &tmp_obj)) {
		data->sensor_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "sys_gain", &tmp_obj)) {
		data->sys_gain = json_object_get_int64(tmp_obj);
	}
}

void parse_dip_ae_anti_flicker_conf(AGTX_DIP_AE_ANTI_FLICKER_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
		data->enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "frequency", &tmp_obj)) {
		data->frequency = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "luma_delta", &tmp_obj)) {
		data->luma_delta = json_object_get_int(tmp_obj);
	}
}

void parse_dip_ae_conf(AGTX_DIP_AE_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;
	if (json_object_object_get_ex(cmd_obj, "anti_flicker", &tmp_obj)) {
		parse_dip_ae_anti_flicker_conf(&(data->anti_flicker), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "black_delay_frame", &tmp_obj)) {
		data->black_delay_frame = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "black_speed_bias", &tmp_obj)) {
		data->black_speed_bias = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "brightness", &tmp_obj)) {
		data->brightness = json_object_get_int(tmp_obj);
	}

	if (json_object_object_get_ex(cmd_obj, "fps_mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_fps_mode_map) / sizeof(char *); i++) {
			if (strcmp(agtx_fps_mode_map[i], str) == 0) {
				data->fps_mode = (AGTX_FPS_MODE)i;
				break;
			}
		}
	}

	if (json_object_object_get_ex(cmd_obj, "exp_strategy", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_exp_strategy_map) / sizeof(char *); i++) {
			if (strcmp(agtx_exp_strategy_map[i], str) == 0) {
				data->exp_strategy = (AGTX_EXP_STRATEGY)i;
				break;
			}
		}
	}

	if (json_object_object_get_ex(cmd_obj, "exp_strength", &tmp_obj)) {
		data->exp_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "frame_rate", &tmp_obj)) {
		data->frame_rate = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gain_thr_down", &tmp_obj)) {
		data->gain_thr_down = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "gain_thr_up", &tmp_obj)) {
		data->gain_thr_up = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "interval", &tmp_obj)) {
		data->interval = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual", &tmp_obj)) {
		parse_dip_ae_manual_conf(&(data->manual), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_isp_gain", &tmp_obj)) {
		data->max_isp_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_sensor_gain", &tmp_obj)) {
		data->max_sensor_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_sys_gain", &tmp_obj)) {
		data->max_sys_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_isp_gain", &tmp_obj)) {
		data->min_isp_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_sensor_gain", &tmp_obj)) {
		data->min_sensor_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_sys_gain", &tmp_obj)) {
		data->min_sys_gain = json_object_get_int64(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_awb_weight", &tmp_obj)) {
		data->roi_awb_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_luma_weight", &tmp_obj)) {
		data->roi_luma_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_zone_lum_avg_weight", &tmp_obj)) {
		data->roi_zone_lum_avg_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "slow_frame_rate", &tmp_obj)) {
		data->slow_frame_rate = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "speed", &tmp_obj)) {
		data->speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "tolerance", &tmp_obj)) {
		data->tolerance = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "white_delay_frame", &tmp_obj)) {
		data->white_delay_frame = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zone_weight", &tmp_obj)) {
		parse_dip_ae_zone_weight_table_conf(&(data->zone_weight), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_inttime", &tmp_obj)) {
		data->max_inttime = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_inttime", &tmp_obj)) {
		data->min_inttime = json_object_get_int(tmp_obj);
	}
}

void comp_dip_ae_zone_weight_table_conf(struct json_object *ret_obj, AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_AE_ZONE_WEIGHT_TABLE_CONF_S_MANUAL_TABLE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->manual_table[i]));
		}
		json_object_object_add(ret_obj, "manual_table", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "manual_table");
	}

	const char *str;
	str = agtx_ae_zone_weight_table_mode_map[data->mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}
}

void comp_dip_ae_manual_conf(struct json_object *ret_obj, AGTX_DIP_AE_MANUAL_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->exp_value);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "exp_value", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "exp_value");
	}

	tmp_obj = json_object_new_int(data->flag);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "flag", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "flag");
	}

	tmp_obj = json_object_new_int(data->inttime);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "inttime", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "inttime");
	}

	tmp_obj = json_object_new_int64(data->isp_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "isp_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "isp_gain");
	}

	tmp_obj = json_object_new_int64(data->sensor_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sensor_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sensor_gain");
	}

	tmp_obj = json_object_new_int64(data->sys_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "sys_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "sys_gain");
	}

}

void comp_dip_ae_anti_flicker_conf(struct json_object *ret_obj, AGTX_DIP_AE_ANTI_FLICKER_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enable");
	}

	tmp_obj = json_object_new_int(data->frequency);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "frequency", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "frequency");
	}

	tmp_obj = json_object_new_int(data->luma_delta);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "luma_delta", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "luma_delta");
	}

}

void comp_dip_ae_conf(struct json_object *ret_obj, AGTX_DIP_AE_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_ae_anti_flicker_conf(tmp_obj, &(data->anti_flicker));
		json_object_object_add(ret_obj, "anti_flicker", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "anti_flicker");
	}

	tmp_obj = json_object_new_int(data->black_delay_frame);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "black_delay_frame", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "black_delay_frame");
	}

	tmp_obj = json_object_new_int(data->black_speed_bias);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "black_speed_bias", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "black_speed_bias");
	}

	tmp_obj = json_object_new_int(data->brightness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "brightness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "brightness");
	}

	const char *str;
	str = agtx_exp_strategy_map[data->exp_strategy];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "exp_strategy", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "exp_strategy");
	}

	tmp_obj = json_object_new_int(data->exp_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "exp_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "exp_strength");
	}

	str = agtx_fps_mode_map[data->fps_mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "fps_mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "fps_mode");
	}

	tmp_obj = json_object_new_double(data->frame_rate);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "frame_rate", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "frame_rate");
	}

	tmp_obj = json_object_new_int64(data->gain_thr_down);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gain_thr_down", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gain_thr_down");
	}

	tmp_obj = json_object_new_int64(data->gain_thr_up);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "gain_thr_up", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "gain_thr_up");
	}

	tmp_obj = json_object_new_int(data->interval);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "interval", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "interval");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_ae_manual_conf(tmp_obj, &(data->manual));
		json_object_object_add(ret_obj, "manual", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "manual");
	}

	tmp_obj = json_object_new_int64(data->max_isp_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_isp_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_isp_gain");
	}

	tmp_obj = json_object_new_int64(data->max_sensor_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_sensor_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_sensor_gain");
	}

	tmp_obj = json_object_new_int64(data->max_sys_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_sys_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_sys_gain");
	}

	tmp_obj = json_object_new_int64(data->min_isp_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_isp_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_isp_gain");
	}

	tmp_obj = json_object_new_int64(data->min_sensor_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_sensor_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_sensor_gain");
	}

	tmp_obj = json_object_new_int64(data->min_sys_gain);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_sys_gain", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_sys_gain");
	}

	tmp_obj = json_object_new_int(data->roi_awb_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "roi_awb_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi_awb_weight");
	}

	tmp_obj = json_object_new_int(data->roi_luma_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "roi_luma_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi_luma_weight");
	}

	tmp_obj = json_object_new_int(data->roi_zone_lum_avg_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "roi_zone_lum_avg_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "roi_zone_lum_avg_weight");
	}

	tmp_obj = json_object_new_double(data->slow_frame_rate);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "slow_frame_rate", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "slow_frame_rate");
	}

	tmp_obj = json_object_new_int(data->speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "speed");
	}

	tmp_obj = json_object_new_int(data->tolerance);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "tolerance", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "tolerance");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	tmp_obj = json_object_new_int(data->white_delay_frame);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "white_delay_frame", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "white_delay_frame");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_ae_zone_weight_table_conf(tmp_obj, &(data->zone_weight));
		json_object_object_add(ret_obj, "zone_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "zone_weight");
	}

	tmp_obj = json_object_new_int(data->max_inttime);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_inttime", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_inttime");
	}

	tmp_obj = json_object_new_int(data->min_inttime);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_inttime", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_inttime");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
