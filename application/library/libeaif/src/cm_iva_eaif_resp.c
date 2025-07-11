#include "cm_iva_eaif_resp.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#ifdef EAIF_SUPPORT_JSON
#include "json.h"
#define min(x, y) ((x) < (y)) ? (x) : (y)
#define J(x) json_object_##x

void comp_iva_eaif_resp_obj_attr(struct json_object *array_obj, AGTX_IVA_EAIF_RESP_OBJ_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL, *tmp_array_obj = NULL;
	int i = 0, array_len = 0;
	tmp_obj = J(new_object)();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = J(new_int)(data->idx);
		if (tmp1_obj) {
			J(object_add)(tmp_obj, "idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "idx");
		}

		tmp1_obj = J(new_int)(data->label_num);
		if (tmp1_obj) {
			J(object_add)(tmp_obj, "label_num", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "label_num");
		}

		tmp_array_obj = J(new_array)();
		if (tmp_array_obj) {
			for (i = 0; i < 4; ++i) {
				tmp1_obj = J(new_int)(data->rect[i]);
				if (tmp1_obj) {
					J(array_add)(tmp_array_obj, tmp1_obj);
				} else {
					printf("Cannot create rect object \n");
				}
			}
		}
		J(object_add)(array_obj, "rect", tmp_array_obj);

		const char *str;
		tmp_array_obj = J(new_array)();
		if (tmp_array_obj) {
			for (i = 0; i < data->label_num; ++i) {
				str = (const char *)data->label[i];
				tmp1_obj = J(new_string)(str);
				if (tmp1_obj) {
					J(array_add)(tmp_array_obj, tmp1_obj);
				} else {
					printf("Cannot create %s object\n", "label");
				}
			}
		}
		J(object_add)(array_obj, "label", tmp_array_obj);

		tmp_array_obj = J(new_array)();
		if (tmp_array_obj) {
			for (i = 0; i < array_len; ++i) {
				str = (const char *)data->prob[i];
				tmp1_obj = J(new_string)(str);
				if (tmp1_obj) {
					J(array_add)(tmp_array_obj, tmp1_obj);
				} else {
					printf("Cannot create %s object\n", "prob");
				}
			}
		}
		J(object_add)(array_obj, "prob", tmp_array_obj);

		J(array_add)(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void parse_iva_eaif_resp_obj_attr(AGTX_IVA_EAIF_RESP_OBJ_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	struct json_object *tmp_arr_obj;
	int i;
	if (J(object_get_ex)(cmd_obj, "idx", &tmp_obj)) {
		data->idx = J(get_int)(tmp_obj);
	}
	int array_len = 0;
	if (J(object_get_ex)(cmd_obj, "label", &tmp_obj)) {
		array_len = min(J(array_length)(tmp_obj), MAX_EAIF_RESP_OBJ_CLS_NUM);
		for (i = 0; i < array_len; ++i) {
			tmp_arr_obj = J(array_get_idx)(tmp_obj, i);
			strncpy((char *)data->label[i], J(get_string)(tmp_arr_obj), MAX_EAIF_RESP_OBJ_ATTR_SIZE);
		}
	}
	data->label_num = array_len;
	if (J(object_get_ex)(cmd_obj, "prob", &tmp_obj)) {
		for (i = 0; i < array_len; ++i) {
			tmp_arr_obj = J(array_get_idx)(tmp_obj, i);
			strncpy((char *)data->prob[i], J(get_string)(tmp_arr_obj), MAX_EAIF_RESP_OBJ_ATTR_SIZE);
		}
	}
	if (J(object_get_ex)(cmd_obj, "rect", &tmp_obj)) {
		array_len = min(4, J(array_length)(tmp_obj));
		for (i = 0; i < array_len; ++i) {
			data->rect[i] = J(get_int)(J(array_get_idx)(tmp_obj, i));
		}
	}
}

#endif /* !EAIF_SUPPORT_JSON */

void comp_iva_eaif_resp(struct json_object *ret_obj, AGTX_IVA_EAIF_RESP_S *data)
{
#ifdef EAIF_SUPPORT_JSON
	struct json_object *tmp_obj = NULL;
	int array_len = 0;
	tmp_obj = J(new_int)(data->pred_num);
	if (tmp_obj) {
		J(object_add)(ret_obj, "pred_num", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "pred_num");
	}

	int i;
	array_len = data->pred_num;
	tmp_obj = J(new_array)();
	if (tmp_obj) {
		for (i = 0; i < array_len; i++) {
			comp_iva_eaif_resp_obj_attr(tmp_obj, &(data->predictions[i]));
		}
		J(object_add)(ret_obj, "predictions", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "predictions");
	}

	tmp_obj = J(new_int)(data->success);
	if (tmp_obj) {
		J(object_add)(ret_obj, "success", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "success");
	}

	tmp_obj = J(new_int)(data->time);
	if (tmp_obj) {
		J(object_add)(ret_obj, "time", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "time");
	}
#else
	// Unused parameters.
	(void)ret_obj;
	(void)data;

	fprintf(stderr, "JSON library is not supported!\n");
	assert(0);
#endif /* !EAIF_SUPPORT_JSON */
}

void parse_iva_eaif_resp(AGTX_IVA_EAIF_RESP_S *data, struct json_object *cmd_obj)
{
#ifdef EAIF_SUPPORT_JSON
	struct json_object *tmp_obj;
	int pred_num = 0;
	if (J(object_get_ex)(cmd_obj, "pred_num", &tmp_obj)) {
		data->pred_num = J(get_int)(tmp_obj);
	}
	int i;
	if (J(object_get_ex)(cmd_obj, "predictions", &tmp_obj)) {
		data->pred_num = min(MAX_AGTX_IVA_EAIF_RESP_S_PREDICTIONS_SIZE, data->pred_num);
		pred_num = min(data->pred_num, J(array_length)(tmp_obj));

		for (i = 0; i < pred_num; i++) {
			parse_iva_eaif_resp_obj_attr(&data->predictions[i], J(array_get_idx)(tmp_obj, i));
		}
	}
	if (J(object_get_ex)(cmd_obj, "success", &tmp_obj)) {
		data->success = J(get_int)(tmp_obj);
	}
	if (J(object_get_ex)(cmd_obj, "time", &tmp_obj)) {
		data->time = J(get_int)(tmp_obj);
	}
#else
	// Unused parameters.
	(void)cmd_obj;
	(void)data;

	fprintf(stderr, "JSON library is not supported!\n");
	assert(0);
#endif /* !EAIF_SUPPORT_JSON */
}
