#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_hdr_synth_conf.h"


void parse_dip_hdr_synth_conf(AGTX_DIP_HDR_SYNTH_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "frame_fb_strength", &tmp_obj)) {
		data->frame_fb_strength = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "local_fb_th", &tmp_obj)) {
		data->local_fb_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_le_weight_max", &tmp_obj)) {
		data->weight_le_weight_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_le_weight_min", &tmp_obj)) {
		data->weight_le_weight_min = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_le_weight_slope", &tmp_obj)) {
		data->weight_le_weight_slope = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_le_weight_th_max", &tmp_obj)) {
		data->weight_le_weight_th_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_se_weight_max", &tmp_obj)) {
		data->weight_se_weight_max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_se_weight_min", &tmp_obj)) {
		data->weight_se_weight_min = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_se_weight_slope", &tmp_obj)) {
		data->weight_se_weight_slope = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "weight_se_weight_th_min", &tmp_obj)) {
		data->weight_se_weight_th_min = json_object_get_int(tmp_obj);
	}
}

void comp_dip_hdr_synth_conf(struct json_object *ret_obj, AGTX_DIP_HDR_SYNTH_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->frame_fb_strength);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "frame_fb_strength", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "frame_fb_strength");
	}

	tmp_obj = json_object_new_int(data->local_fb_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "local_fb_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "local_fb_th");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	tmp_obj = json_object_new_int(data->weight_le_weight_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_le_weight_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_le_weight_max");
	}

	tmp_obj = json_object_new_int(data->weight_le_weight_min);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_le_weight_min", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_le_weight_min");
	}

	tmp_obj = json_object_new_int(data->weight_le_weight_slope);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_le_weight_slope", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_le_weight_slope");
	}

	tmp_obj = json_object_new_int(data->weight_le_weight_th_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_le_weight_th_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_le_weight_th_max");
	}

	tmp_obj = json_object_new_int(data->weight_se_weight_max);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_se_weight_max", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_se_weight_max");
	}

	tmp_obj = json_object_new_int(data->weight_se_weight_min);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_se_weight_min", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_se_weight_min");
	}

	tmp_obj = json_object_new_int(data->weight_se_weight_slope);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_se_weight_slope", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_se_weight_slope");
	}

	tmp_obj = json_object_new_int(data->weight_se_weight_th_min);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "weight_se_weight_th_min", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "weight_se_weight_th_min");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
