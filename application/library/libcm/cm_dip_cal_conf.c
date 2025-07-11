#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_dip_cal_conf.h"


void parse_agtx_dip_cal_attr_s(AGTX_DIP_CAL_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "cal_en", &tmp_obj)) {
		data->cal_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "dbc_en", &tmp_obj)) {
		data->dbc_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "dcc_en", &tmp_obj)) {
		data->dcc_en = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "lsc_en", &tmp_obj)) {
		data->lsc_en = json_object_get_int(tmp_obj);
	}
}

void parse_dip_cal_conf(AGTX_DIP_CAL_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "cal", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_CAL_CONF_S_CAL_SIZE; i++) {
			parse_agtx_dip_cal_attr_s(&(data->cal[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_cal_attr(struct json_object *ret_obj, AGTX_DIP_CAL_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->cal_en);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "cal_en", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "cal_en");
		}

		tmp1_obj = json_object_new_int(data->dbc_en);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "dbc_en", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "dbc_en");
		}

		tmp1_obj = json_object_new_int(data->dcc_en);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "dcc_en", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "dcc_en");
		}

		tmp1_obj = json_object_new_int(data->lsc_en);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "lsc_en", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "lsc_en");
		}

		json_object_array_add(ret_obj, tmp_obj);
	} else {
		printf("Cannot create array object\n");
	}
}

void comp_dip_cal_conf(struct json_object *ret_obj, AGTX_DIP_CAL_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_CAL_CONF_S_CAL_SIZE; i++) {
			comp_dip_cal_attr(tmp_obj, &(data->cal[i]));
		}
		json_object_object_add(ret_obj, "cal", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "cal");
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
