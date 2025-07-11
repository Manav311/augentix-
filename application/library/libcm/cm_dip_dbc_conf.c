#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_dbc_conf.h"

const char *agtx_dbc_type_e_map[] = { "SAME_BLACK_LEVEL", "IND_BLACK_LEVEL" };

void parse_dip_dbc_attr(AGTX_DIP_DBC_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_0", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_0_SIZE; i++) {
			data->auto_black_level_0[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_1", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_1_SIZE; i++) {
			data->auto_black_level_1[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_10", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_10_SIZE; i++) {
			data->auto_black_level_10[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_2", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_2_SIZE; i++) {
			data->auto_black_level_2[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_3", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_3_SIZE; i++) {
			data->auto_black_level_3[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_4", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_4_SIZE; i++) {
			data->auto_black_level_4[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_5", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_5_SIZE; i++) {
			data->auto_black_level_5[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_6", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_6_SIZE; i++) {
			data->auto_black_level_6[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_7", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_7_SIZE; i++) {
			data->auto_black_level_7[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_8", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_8_SIZE; i++) {
			data->auto_black_level_8[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_black_level_9", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_9_SIZE; i++) {
			data->auto_black_level_9[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "dbc_level", &tmp_obj)) {
		data->dbc_level = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "manual_black_level", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_MANUAL_BLACK_LEVEL_SIZE; i++) {
			data->manual_black_level[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_dbc_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_dbc_type_e_map[i], str) == 0) {
				data->type = (AGTX_DBC_TYPE_E)i;
				break;
			}
		}
	}
}

void parse_dip_dbc_conf(AGTX_DIP_DBC_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "dbc", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_CONF_S_DBC_SIZE; i++) {
			parse_dip_dbc_attr(&(data->dbc[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_dbc_attr(struct json_object *array_obj, AGTX_DIP_DBC_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_0_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_0[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_0", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_0");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_1_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_1[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_1", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_1");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_10_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_10[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_10", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_10");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_2_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_2[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_2", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_2");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_3_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_3[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_3", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_3");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_4_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_4[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_4", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_4");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_5_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_5[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_5", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_5");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_6_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_6[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_6", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_6");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_7_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_7[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_7", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_7");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_8_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_8[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_8", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_8");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_AUTO_BLACK_LEVEL_9_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->auto_black_level_9[i]));
			}
			json_object_object_add(tmp_obj, "auto_black_level_9", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_black_level_9");
		}

		tmp1_obj = json_object_new_int(data->dbc_level);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "dbc_level", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "dbc_level");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DBC_ATTR_S_MANUAL_BLACK_LEVEL_SIZE; i++) {
				json_object_array_add(tmp1_obj, json_object_new_int(data->manual_black_level[i]));
			}
			json_object_object_add(tmp_obj, "manual_black_level", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "manual_black_level");
		}

		tmp1_obj = json_object_new_int(data->mode);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mode", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mode");
		}

		const char *str;
		str = agtx_dbc_type_e_map[data->type];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "type", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "type");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_dbc_conf(struct json_object *ret_obj, AGTX_DIP_DBC_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_DBC_CONF_S_DBC_SIZE; i++) {
			comp_dip_dbc_attr(tmp_obj, &(data->dbc[i]));
		}
		json_object_object_add(ret_obj, "dbc", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dbc");
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
