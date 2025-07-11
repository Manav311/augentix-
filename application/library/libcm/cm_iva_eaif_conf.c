#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_iva_eaif_conf.h"

const char *agtx_iva_eaif_inf_cmd_e_map[] = { "NONE",      "FACE_REGISTER", "FACE_LOAD",
	                                      "FACE_SAVE", "FACE_DELETE",   "FACE_RESET" };

const char *agtx_iva_eaif_data_fmt_e_map[] = { "JPEG", "Y", "YUV", "RGB", "MPI_JPEG", "MPI_Y", "MPI_YUV", "MPI_RGB" };

const char *agtx_iva_eaif_api_method_e_map[] = { "FACEDET",  "FACERECO",    "DETECT",
	                                         "CLASSIFY", "CLASSIFY_CV", "HUMAN_CLASSIFY" };

void parse_iva_eaif_conf(AGTX_IVA_EAIF_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "api", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_eaif_api_method_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_eaif_api_method_e_map[i], str) == 0) {
				data->api = (AGTX_IVA_EAIF_API_METHOD_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "classify_cv_model", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_CLASSIFY_CV_MODEL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->classify_cv_model, json_object_get_string(tmp_obj), i);
		data->classify_cv_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "classify_model", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_CLASSIFY_MODEL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->classify_model, json_object_get_string(tmp_obj), i);
		data->classify_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "data_fmt", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_eaif_data_fmt_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_eaif_data_fmt_e_map[i], str) == 0) {
				data->data_fmt = (AGTX_IVA_EAIF_DATA_FMT_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "detect_model", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_DETECT_MODEL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->detect_model, json_object_get_string(tmp_obj), i);
		data->detect_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "detection_period", &tmp_obj)) {
		data->detection_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "face_detect_model", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_FACE_DETECT_MODEL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->face_detect_model, json_object_get_string(tmp_obj), i);
		data->face_detect_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "face_name", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_FACE_NAME_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->face_name, json_object_get_string(tmp_obj), i);
		data->face_name[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "face_reco_model", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_FACE_RECO_MODEL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->face_reco_model, json_object_get_string(tmp_obj), i);
		data->face_reco_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "facereco_roi_ex", &tmp_obj)) {
		data->facereco_roi_ex = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "facereco_roi_ey", &tmp_obj)) {
		data->facereco_roi_ey = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "facereco_roi_sx", &tmp_obj)) {
		data->facereco_roi_sx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "facereco_roi_sy", &tmp_obj)) {
		data->facereco_roi_sy = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "human_classify_model", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_HUMAN_CLASSIFY_MODEL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->human_classify_model, json_object_get_string(tmp_obj), i);
		data->human_classify_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "identification_period", &tmp_obj)) {
		data->identification_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "inf_cmd", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_iva_eaif_inf_cmd_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_eaif_inf_cmd_e_map[i], str) == 0) {
				data->inf_cmd = (AGTX_IVA_EAIF_INF_CMD_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "inf_with_obj_list", &tmp_obj)) {
		data->inf_with_obj_list = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_size", &tmp_obj)) {
		data->min_size = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "neg_classify_period", &tmp_obj)) {
		data->neg_classify_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_exist_classify_period", &tmp_obj)) {
		data->obj_exist_classify_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_classify_period", &tmp_obj)) {
		data->pos_classify_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_stop_count_th", &tmp_obj)) {
		data->pos_stop_count_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "snapshot_height", &tmp_obj)) {
		data->snapshot_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "snapshot_width", &tmp_obj)) {
		data->snapshot_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "target_idx", &tmp_obj)) {
		data->target_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "url", &tmp_obj)) {
		i = min(MAX_AGTX_IVA_EAIF_CONF_S_URL_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->url, json_object_get_string(tmp_obj), i);
		data->url[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "video_chn_idx", &tmp_obj)) {
		data->video_chn_idx = json_object_get_int(tmp_obj);
	}
}

void comp_iva_eaif_conf(struct json_object *ret_obj, AGTX_IVA_EAIF_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	const char *str;
	str = agtx_iva_eaif_api_method_e_map[data->api];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "api", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "api");
	}

	str = (const char *)data->classify_cv_model;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "classify_cv_model", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "classify_cv_model");
	}

	str = (const char *)data->classify_model;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "classify_model", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "classify_model");
	}

	str = agtx_iva_eaif_data_fmt_e_map[data->data_fmt];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "data_fmt", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "data_fmt");
	}

	str = (const char *)data->detect_model;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "detect_model", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "detect_model");
	}

	tmp_obj = json_object_new_int(data->detection_period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "detection_period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "detection_period");
	}

	tmp_obj = json_object_new_int(data->enabled);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "enabled", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "enabled");
	}

	str = (const char *)data->face_detect_model;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "face_detect_model", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "face_detect_model");
	}

	str = (const char *)data->face_name;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "face_name", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "face_name");
	}

	str = (const char *)data->face_reco_model;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "face_reco_model", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "face_reco_model");
	}

	tmp_obj = json_object_new_int(data->facereco_roi_ex);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "facereco_roi_ex", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "facereco_roi_ex");
	}

	tmp_obj = json_object_new_int(data->facereco_roi_ey);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "facereco_roi_ey", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "facereco_roi_ey");
	}

	tmp_obj = json_object_new_int(data->facereco_roi_sx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "facereco_roi_sx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "facereco_roi_sx");
	}

	tmp_obj = json_object_new_int(data->facereco_roi_sy);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "facereco_roi_sy", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "facereco_roi_sy");
	}

	str = (const char *)data->human_classify_model;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "human_classify_model", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "human_classify_model");
	}

	tmp_obj = json_object_new_int(data->identification_period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "identification_period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "identification_period");
	}

	str = agtx_iva_eaif_inf_cmd_e_map[data->inf_cmd];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "inf_cmd", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "inf_cmd");
	}

	tmp_obj = json_object_new_int(data->inf_with_obj_list);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "inf_with_obj_list", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "inf_with_obj_list");
	}

	tmp_obj = json_object_new_int(data->min_size);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_size", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_size");
	}

	tmp_obj = json_object_new_int(data->neg_classify_period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "neg_classify_period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "neg_classify_period");
	}

	tmp_obj = json_object_new_int(data->obj_exist_classify_period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_exist_classify_period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_exist_classify_period");
	}

	tmp_obj = json_object_new_int(data->obj_life_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "obj_life_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "obj_life_th");
	}

	tmp_obj = json_object_new_int(data->pos_classify_period);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "pos_classify_period", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "pos_classify_period");
	}

	tmp_obj = json_object_new_int(data->pos_stop_count_th);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "pos_stop_count_th", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "pos_stop_count_th");
	}

	tmp_obj = json_object_new_int(data->snapshot_height);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "snapshot_height", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "snapshot_height");
	}

	tmp_obj = json_object_new_int(data->snapshot_width);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "snapshot_width", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "snapshot_width");
	}

	tmp_obj = json_object_new_int(data->target_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "target_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "target_idx");
	}

	str = (const char *)data->url;
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "url", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "url");
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
