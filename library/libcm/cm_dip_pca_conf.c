#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_pca_conf.h"

void parse_dip_pca_conf(AGTX_DIP_PCA_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "current_table", &tmp_obj)) {
		int count = 0;
		for (int i = 0; i < PCA_S_ENTRY_NUM * PCA_H_ENTRY_NUM * PCA_L_ENTRY_NUM; i++) {
			data->h_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, count++));
		}

		for (int i = 0; i < PCA_S_ENTRY_NUM * PCA_H_ENTRY_NUM * PCA_L_ENTRY_NUM; i++) {
			data->s_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, count++));
		}

		for (int i = 0; i < PCA_S_ENTRY_NUM * PCA_H_ENTRY_NUM * PCA_L_ENTRY_NUM; i++) {
			data->l_table[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, count++));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_pca_conf(struct json_object *ret_obj, AGTX_DIP_PCA_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (int i = 0; i < PCA_S_ENTRY_NUM * PCA_H_ENTRY_NUM * PCA_L_ENTRY_NUM; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->h_table[i]));
		}
		for (int i = 0; i < PCA_S_ENTRY_NUM * PCA_H_ENTRY_NUM * PCA_L_ENTRY_NUM; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->s_table[i]));
		}
		for (int i = 0; i < PCA_S_ENTRY_NUM * PCA_H_ENTRY_NUM * PCA_L_ENTRY_NUM; i++) {
			json_object_array_add(tmp_obj, json_object_new_int(data->l_table[i]));
		}
		json_object_object_add(ret_obj, "current_table", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "current_table");
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
