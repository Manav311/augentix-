#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_te_info.h"

void parse_dip_te_info(AGTX_DIP_TE_INFO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "tm_curve", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_TE_INFO_S_TM_CURVE_SIZE; i++) {
			data->tm_curve[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "tm_enable", &tmp_obj)) {
		data->tm_enable = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_te_info(struct json_object *ret_obj, AGTX_DIP_TE_INFO_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_TE_INFO_S_TM_CURVE_SIZE; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->tm_curve[i]));
		}
		json_object_object_add(ret_obj, "tm_curve", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "tm_curve");
	}

	tmp_obj = json_object_new_int(data->tm_enable);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "tm_enable", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "tm_enable");
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
