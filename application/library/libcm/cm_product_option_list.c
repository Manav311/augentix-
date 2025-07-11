#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_product_option_list.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


void parse_product_option_list(AGTX_PRODUCT_OPTION_LIST_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "option", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;
		int len;

		for (i = 0; i < MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (tmp1_obj) {
				len = min(MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_STR_SIZE, json_object_get_string_len(tmp1_obj));
				strncpy((char *)data->option[i], json_object_get_string(tmp1_obj), len);
				data->option[i][len+1] = '\0';
			}
		}
	}
}

void comp_product_option_list(struct json_object *ret_obj, AGTX_PRODUCT_OPTION_LIST_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		int i;
		const char *str;
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < MAX_AGTX_PRODUCT_OPTION_LIST_S_OPTION_SIZE; i++) {
			str = (const char *)data->option[i];

			if (str[0] != '\0') {
				tmp1_obj = json_object_new_string(str);
				if (tmp1_obj) {
					json_object_array_add(tmp_obj, tmp1_obj);
				} else {
					printf("Cannot create array %s entity object\n", "option");
				}
			}
		}
		json_object_object_add(ret_obj, "option", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "option");
	}
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
