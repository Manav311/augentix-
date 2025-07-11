#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_te_conf.h"

const char *agtx_te_dri_type_e_map[] = { "GAMMA_DOMAIN_HIST_CV", "LINEAR_DOMAIN_HIST_CV" };

const char *agtx_te_mode_e_map[] = { "NORMAL", "WDR", "WDR_AUTO", "ADAPTIVE" };

const char *agtx_te_based_type_e_map[] = { "TE_ADAPT_NL_BASED", "TE_ADAPT_INTTIME_BASED", "TE_ADAPT_EV_BASED",
	                                   "TE_ADAPT_BASED_TYPE_RSV" };

void parse_dip_te_wdr(AGTX_DIP_TE_WDR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "brightness", &tmp_obj)) {
		data->brightness = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "dark_enhance", &tmp_obj)) {
		data->dark_enhance = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "interval", &tmp_obj)) {
		data->interval = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "iso_max", &tmp_obj)) {
		data->iso_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "iso_weight", &tmp_obj)) {
		data->iso_weight = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "noise_cstr", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_S_NOISE_CSTR_SIZE; i++) {
			data->noise_cstr[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "precision", &tmp_obj)) {
		data->precision = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "saliency", &tmp_obj)) {
		data->saliency = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "strength", &tmp_obj)) {
		data->strength = json_object_get_int(tmp_obj);
	}
}

void parse_dip_te_wdr_auto(AGTX_DIP_TE_WDR_AUTO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "brightness", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_BRIGHTNESS_SIZE; i++) {
			data->brightness[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dark_enhance", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_DARK_ENHANCE_SIZE; i++) {
			data->dark_enhance[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dri_gain", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_DRI_GAIN_SIZE; i++) {
			data->dri_gain[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dri_offset", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_DRI_OFFSET_SIZE; i++) {
			data->dri_offset[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "dri_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_te_dri_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_te_dri_type_e_map[i], str) == 0) {
				data->dri_type = (AGTX_TE_DRI_TYPE_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "interval", &tmp_obj)) {
		data->interval = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "iso_max", &tmp_obj)) {
		data->iso_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "iso_weight", &tmp_obj)) {
		data->iso_weight = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "noise_cstr", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_NOISE_CSTR_SIZE; i++) {
			data->noise_cstr[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "precision", &tmp_obj)) {
		data->precision = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "saliency", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_SALIENCY_SIZE; i++) {
			data->saliency[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "strength", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_STRENGTH_SIZE; i++) {
			data->strength[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_dip_te_adapt(AGTX_DIP_TE_ADAPT_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "black_th", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_BLACK_TH_SIZE; i++) {
			data->black_th[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dark_enhance", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_ENHANCE_SIZE; i++) {
			data->dark_enhance[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dark_enhance_th", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_ENHANCE_TH_SIZE; i++) {
			data->dark_enhance_th[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dark_protect_smooth", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_PROTECT_SMOOTH_SIZE; i++) {
			data->dark_protect_smooth[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dark_protect_str", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_PROTECT_STR_SIZE; i++) {
			data->dark_protect_str[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "max_str", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_MAX_STR_SIZE; i++) {
			data->max_str[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "max_str_prec_sel", &tmp_obj)) {
		data->max_str_prec_sel = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "speed", &tmp_obj)) {
		data->speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "str_auto", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_STR_AUTO_SIZE; i++) {
			data->str_auto[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "str_auto_en", &tmp_obj)) {
		data->str_auto_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "strength", &tmp_obj)) {
		data->strength = json_object_get_int(tmp_obj);
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "te_adapt_based_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_te_based_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_te_based_type_e_map[i], str) == 0) {
				data->te_adapt_based_type = (AGTX_TE_BASED_TYPE_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "white_th", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_WHITE_TH_SIZE; i++) {
			data->white_th[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_dip_te_conf(AGTX_DIP_TE_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "adapt_ctl", &tmp_obj)) {
		parse_dip_te_adapt(&(data->adapt_ctl), tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_te_mode_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_te_mode_e_map[i], str) == 0) {
				data->mode = (AGTX_TE_MODE_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "normal_ctl", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_CONF_S_NORMAL_CTL_SIZE; i++) {
			data->normal_ctl[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wdr_auto_ctl", &tmp_obj)) {
		parse_dip_te_wdr_auto(&(data->wdr_auto_ctl), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "wdr_ctl", &tmp_obj)) {
		parse_dip_te_wdr(&(data->wdr_ctl), tmp_obj);
	}
}

void comp_dip_te_wdr(struct json_object *ret_obj, AGTX_DIP_TE_WDR_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->brightness);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "brightness", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "brightness");
	}

	tmp_obj = json_object_new_int(data->dark_enhance);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "dark_enhance", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "dark_enhance");
	}

	tmp_obj = json_object_new_int(data->interval);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "interval", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "interval");
	}

	tmp_obj = json_object_new_int(data->iso_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "iso_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "iso_max");
	}

	tmp_obj = json_object_new_int(data->iso_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "iso_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "iso_weight");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_S_NOISE_CSTR_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->noise_cstr[i]));
		}
		json_object_object_add(ret_obj, "noise_cstr", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "noise_cstr");
	}

	tmp_obj = json_object_new_int(data->precision);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "precision", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "precision");
	}

	tmp_obj = json_object_new_int(data->saliency);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "saliency", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "saliency");
	}

	tmp_obj = json_object_new_int(data->strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "strength");
	}

}

void comp_dip_te_wdr_auto(struct json_object *ret_obj, AGTX_DIP_TE_WDR_AUTO_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_BRIGHTNESS_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->brightness[i]));
		}
		json_object_object_add(ret_obj, "brightness", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "brightness");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_DARK_ENHANCE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dark_enhance[i]));
		}
		json_object_object_add(ret_obj, "dark_enhance", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dark_enhance");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_DRI_GAIN_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dri_gain[i]));
		}
		json_object_object_add(ret_obj, "dri_gain", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dri_gain");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_DRI_OFFSET_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dri_offset[i]));
		}
		json_object_object_add(ret_obj, "dri_offset", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dri_offset");
	}

	const char *str;
	str = agtx_te_dri_type_e_map[data->dri_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "dri_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "dri_type");
	}

	tmp_obj = json_object_new_int(data->interval);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "interval", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "interval");
	}

	tmp_obj = json_object_new_int(data->iso_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "iso_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "iso_max");
	}

	tmp_obj = json_object_new_int(data->iso_weight);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "iso_weight", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "iso_weight");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_NOISE_CSTR_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->noise_cstr[i]));
		}
		json_object_object_add(ret_obj, "noise_cstr", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "noise_cstr");
	}

	tmp_obj = json_object_new_int(data->precision);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "precision", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "precision");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_SALIENCY_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->saliency[i]));
		}
		json_object_object_add(ret_obj, "saliency", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "saliency");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_WDR_AUTO_S_STRENGTH_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->strength[i]));
		}
		json_object_object_add(ret_obj, "strength", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "strength");
	}
}

void comp_dip_te_adapt(struct json_object *ret_obj, AGTX_DIP_TE_ADAPT_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_BLACK_TH_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->black_th[i]));
		}
		json_object_object_add(ret_obj, "black_th", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "black_th");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_ENHANCE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dark_enhance[i]));
		}
		json_object_object_add(ret_obj, "dark_enhance", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dark_enhance");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_ENHANCE_TH_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dark_enhance_th[i]));
		}
		json_object_object_add(ret_obj, "dark_enhance_th", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dark_enhance_th");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_PROTECT_SMOOTH_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dark_protect_smooth[i]));
		}
		json_object_object_add(ret_obj, "dark_protect_smooth", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dark_protect_smooth");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_DARK_PROTECT_STR_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->dark_protect_str[i]));
		}
		json_object_object_add(ret_obj, "dark_protect_str", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dark_protect_str");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_MAX_STR_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->max_str[i]));
		}
		json_object_object_add(ret_obj, "max_str", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "max_str");
	}

	tmp_obj = json_object_new_int(data->max_str_prec_sel);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_str_prec_sel", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_str_prec_sel");
	}

	tmp_obj = json_object_new_int(data->speed);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "speed", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "speed");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_STR_AUTO_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->str_auto[i]));
		}
		json_object_object_add(ret_obj, "str_auto", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "str_auto");
	}

	tmp_obj = json_object_new_int(data->str_auto_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "str_auto_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "str_auto_en");
	}

	tmp_obj = json_object_new_int(data->strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "strength");
	}

	const char *str;
	str = agtx_te_based_type_e_map[data->te_adapt_based_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "te_adapt_based_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "te_adapt_based_type");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_ADAPT_S_WHITE_TH_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->white_th[i]));
		}
		json_object_object_add(ret_obj, "white_th", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "white_th");
	}
}

void comp_dip_te_conf(struct json_object *ret_obj, AGTX_DIP_TE_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_te_adapt(tmp_obj, &(data->adapt_ctl));
		json_object_object_add(ret_obj, "adapt_ctl", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "adapt_ctl");
	}

	const char *str;
	str = agtx_te_mode_e_map[data->mode];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "mode", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "mode");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_CONF_S_NORMAL_CTL_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->normal_ctl[i]));
		}
		json_object_object_add(ret_obj, "normal_ctl", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "normal_ctl");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_te_wdr_auto(tmp_obj, &(data->wdr_auto_ctl));
		json_object_object_add(ret_obj, "wdr_auto_ctl", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wdr_auto_ctl");
	}

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		comp_dip_te_wdr(tmp_obj, &(data->wdr_ctl));
		json_object_object_add(ret_obj, "wdr_ctl", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "wdr_ctl");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
