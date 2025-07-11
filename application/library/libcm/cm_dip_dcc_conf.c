#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_dip_dcc_conf.h"

const char *agtx_dcc_type_e_map[] = { "DCC_TYPE", "DCC_TYPE_EX" };

void parse_agtx_dip_dcc_attr_s(AGTX_DIP_DCC_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	// auto dcc gain & offset
	if (json_object_object_get_ex(cmd_obj, "auto_gain_0", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_0[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_0", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_0[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_1", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_1[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_1", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_1[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_2", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_2[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_2", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_2[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_3", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_3[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_3", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_3[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_4", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_4[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_4", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_4[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_5", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_5[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_5", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_5[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_6", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_6[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_6", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_6[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_7", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_7[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_7", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_7[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_8", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_8[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_8", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_8[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_9", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_9[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_9", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_9[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_10", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_10[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_10", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_10[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_11", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_11[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_11", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_11[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_12", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_12[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_12", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_12[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_13", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_13[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_13", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_13[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_14", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_14[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_14", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_14[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_gain_15", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->auto_gain_15[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "auto_offset_15", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->auto_offset_15[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}

	// manual dcc gain & offset
	if (json_object_object_get_ex(cmd_obj, "manual_gain", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->manual_gain[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "manual_offset", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->manual_offset[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "gain", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
			data->gain[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "offset", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
			data->offset[i] = json_object_get_int(json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "mode", &tmp_obj)) {
		data->mode = json_object_get_int(tmp_obj);
	}
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_dcc_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_dcc_type_e_map[i], str) == 0) {
				data->type = (AGTX_DCC_TYPE_E)i;
				break;
			}
		}
	}
}

void parse_dip_dcc_conf(AGTX_DIP_DCC_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "dcc", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_CONF_S_DCC_SIZE; i++) {
			parse_agtx_dip_dcc_attr_s(&(data->dcc[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_dcc_attr(struct json_object *ret_obj, AGTX_DIP_DCC_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL;
	struct json_object *int_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;

		// auto dcc
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_0[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_0", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_0");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_0[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_0", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_0");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_1[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_1", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_1");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_1[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_1", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_1");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_2[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_2", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_2");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_2[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_2", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_2");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_3[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_3", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_3");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_3[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_3", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_3");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_4[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_4", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_4");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_4[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_4", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_4");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_5[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_5", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_5");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_5[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_5", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_5");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_6[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_6", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_6");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_6[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_6", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_6");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_7[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_7", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_7");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_7[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_7", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_7");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_8[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_8", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_8");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_8[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_8", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_8");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_9[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_9", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_9");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_9[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_9", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_9");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_10[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_10", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_10");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_10[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_10", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_10");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_11[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_11", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_11");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_11[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_11", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_11");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_12[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_12", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_12");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_12[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_12", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_12");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_13[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_13", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_13");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_13[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_13", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_13");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_14[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_14", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_14");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_14[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_14", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_14");
		}
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_gain_15[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_gain_15", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_gain_15");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->auto_offset_15[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "auto_offset_15", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "auto_offset_15");
		}

		// manual dcc
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->manual_gain[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "manual_gain", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "manual_gain");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->manual_offset[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "manual_offset", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "manual_offset");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_GAIN_SIZE; i++) {
				int_obj = json_object_new_int(data->gain[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "gain", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "gain");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_DIP_DCC_ATTR_S_OFFSET_SIZE; i++) {
				int_obj = json_object_new_int(data->offset[i]);
				json_object_array_add(tmp1_obj, int_obj);
			}
			json_object_object_add(tmp_obj, "offset", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "offset");
		}

		tmp1_obj = json_object_new_int(data->mode);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "mode", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "mode");
		}

		const char *str;
		str = agtx_dcc_type_e_map[data->type];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "type", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "type");
		}

		json_object_array_add(ret_obj, tmp_obj);
	} else {
		printf("Cannot create array object\n");
	}
}

void comp_dip_dcc_conf(struct json_object *ret_obj, AGTX_DIP_DCC_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_DCC_CONF_S_DCC_SIZE; i++) {
			comp_dip_dcc_attr(tmp_obj, &(data->dcc[i]));
		}
		json_object_object_add(ret_obj, "dcc", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "dcc");
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
