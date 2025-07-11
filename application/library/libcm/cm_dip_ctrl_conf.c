#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_ctrl_conf.h"

void parse_dip_ctrl_conf(AGTX_DIP_CTRL_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "is_ae_en", &tmp_obj)) {
		data->is_ae_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_awb_en", &tmp_obj)) {
		data->is_awb_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_coring_en", &tmp_obj)) {
		data->is_coring_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_csm_en", &tmp_obj)) {
		data->is_csm_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_dip_en", &tmp_obj)) {
		data->is_dip_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_dms_en", &tmp_obj)) {
		data->is_dms_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_dpc_en", &tmp_obj)) {
		data->is_dpc_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_enh_en", &tmp_obj)) {
		data->is_enh_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_fcs_en", &tmp_obj)) {
		data->is_fcs_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_dhz_en", &tmp_obj)) {
		data->is_dhz_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_gamma_en", &tmp_obj)) {
		data->is_gamma_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_iso_en", &tmp_obj)) {
		data->is_iso_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_me_en", &tmp_obj)) {
		data->is_me_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_nr_en", &tmp_obj)) {
		data->is_nr_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_pta_en", &tmp_obj)) {
		data->is_pta_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_shp_en", &tmp_obj)) {
		data->is_shp_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "is_te_en", &tmp_obj)) {
		data->is_te_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_ctrl_conf(struct json_object *ret_obj, AGTX_DIP_CTRL_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->is_ae_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_ae_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_ae_en");
	}

	tmp_obj = json_object_new_int(data->is_awb_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_awb_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_awb_en");
	}

	tmp_obj = json_object_new_int(data->is_coring_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_coring_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_coring_en");
	}

	tmp_obj = json_object_new_int(data->is_csm_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_csm_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_csm_en");
	}

	tmp_obj = json_object_new_int(data->is_dip_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_dip_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_dip_en");
	}

	tmp_obj = json_object_new_int(data->is_dms_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_dms_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_dms_en");
	}

	tmp_obj = json_object_new_int(data->is_dpc_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_dpc_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_dpc_en");
	}

	tmp_obj = json_object_new_int(data->is_enh_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_enh_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_enh_en");
	}

	tmp_obj = json_object_new_int(data->is_fcs_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_fcs_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_fcs_en");
	}

	tmp_obj = json_object_new_int(data->is_dhz_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_dhz_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_dhz_en");
	}

	tmp_obj = json_object_new_int(data->is_gamma_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_gamma_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_gamma_en");
	}

	tmp_obj = json_object_new_int(data->is_iso_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_iso_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_iso_en");
	}

	tmp_obj = json_object_new_int(data->is_me_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_me_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_me_en");
	}

	tmp_obj = json_object_new_int(data->is_nr_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_nr_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_nr_en");
	}

	tmp_obj = json_object_new_int(data->is_pta_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_pta_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_pta_en");
	}

	tmp_obj = json_object_new_int(data->is_shp_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_shp_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_shp_en");
	}

	tmp_obj = json_object_new_int(data->is_te_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "is_te_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "is_te_en");
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
