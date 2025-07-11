#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_od_conf.h"

const char *agtx_iva_od_version_e_map[] = { "OD_V4", "OD_V5" };

void parse_iva_od_conf(AGTX_IVA_OD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "en_crop_outside_obj", &tmp_obj)) {
		data->en_crop_outside_obj = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_gmv_det", &tmp_obj)) {
		data->en_gmv_det = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_motor", &tmp_obj)) {
		data->en_motor = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_shake_det", &tmp_obj)) {
		data->en_shake_det = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "en_stop_det", &tmp_obj)) {
		data->en_stop_det = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_conf_th", &tmp_obj)) {
		data->od_conf_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_iou_th", &tmp_obj)) {
		data->od_iou_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_qual", &tmp_obj)) {
		data->od_qual = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_sen", &tmp_obj)) {
		data->od_sen = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_size_th", &tmp_obj)) {
		data->od_size_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_snapshot_h", &tmp_obj)) {
		data->od_snapshot_h = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_snapshot_type", &tmp_obj)) {
		data->od_snapshot_type = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_snapshot_w", &tmp_obj)) {
		data->od_snapshot_w = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "od_track_refine", &tmp_obj)) {
		data->od_track_refine = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "version", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_od_version_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_od_version_e_map[i], str) == 0) {
				data->version = (AGTX_IVA_OD_VERSION_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_od_conf(struct json_object *ret_obj, AGTX_IVA_OD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->en_crop_outside_obj);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_crop_outside_obj", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_crop_outside_obj");
	}

	tmp_obj = json_object_new_int(data->en_gmv_det);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_gmv_det", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_gmv_det");
	}

	tmp_obj = json_object_new_int(data->en_motor);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_motor", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_motor");
	}

	tmp_obj = json_object_new_int(data->en_shake_det);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_shake_det", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_shake_det");
	}

	tmp_obj = json_object_new_int(data->en_stop_det);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "en_stop_det", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "en_stop_det");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	tmp_obj = json_object_new_int(data->od_conf_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_conf_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_conf_th");
	}

	tmp_obj = json_object_new_int(data->od_iou_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_iou_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_iou_th");
	}

	tmp_obj = json_object_new_int(data->od_qual);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_qual", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_qual");
	}

	tmp_obj = json_object_new_int(data->od_sen);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_sen", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_sen");
	}

	tmp_obj = json_object_new_int(data->od_size_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_size_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_size_th");
	}

	tmp_obj = json_object_new_int(data->od_snapshot_h);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_snapshot_h", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_snapshot_h");
	}

	tmp_obj = json_object_new_int(data->od_snapshot_type);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_snapshot_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_snapshot_type");
	}

	tmp_obj = json_object_new_int(data->od_snapshot_w);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_snapshot_w", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_snapshot_w");
	}

	tmp_obj = json_object_new_int(data->od_track_refine);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "od_track_refine", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "od_track_refine");
	}

	const char *str;
	str = agtx_iva_od_version_e_map[data->version];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "version", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "version");
	}

	tmp_obj = json_object_new_int(data->video_chn_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_chn_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_chn_idx");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
