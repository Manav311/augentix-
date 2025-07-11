#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_res_option.h"


static void parse_res_entity(AGTX_RES_ENTITY_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "max_frame_rate", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;
		//fprintf(stderr, "Max_frame_rate = \n");
		for (i = 0; i < MAX_AGTX_RES_ENTITY_S_MAX_FRAME_RATE_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			//fprintf(stderr, "tmp1_obj = %d\n",tmp1_obj);
			if (tmp1_obj) {
				data->max_frame_rate[i] = json_object_get_double(tmp1_obj);
				//fprintf(stderr, "data->max_frame_rate[%d] = %f\n",i,data->max_frame_rate[i]);
			} else {
				data->max_frame_rate[i] = 0.0f;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "frame_rate_list", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < MAX_AGTX_RES_ENTITY_S_FRAME_RATE_LIST_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (tmp1_obj) {
				data->frame_rate_list[i] = json_object_get_double(tmp1_obj);
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "height", &tmp_obj)) {
		data->height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "width", &tmp_obj)) {
		data->width = json_object_get_int(tmp_obj);
	}
}

static void parse_strm_res_option(AGTX_STRM_RES_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "res_idx", &tmp_obj)) {
		data->res_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "res", &tmp_obj)) {
		if (data->res_idx == -1) {
			for (i = 0; i < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; i++) {
				parse_res_entity(&(data->res[i]), json_object_array_get_idx(tmp_obj, i));
			}
		} else {
			parse_res_entity(&(data->res[data->res_idx]), json_object_array_get_idx(tmp_obj, 0));
		}
	}
}

void parse_res_option(AGTX_RES_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "strm_idx", &tmp_obj)) {
		data->strm_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "strm", &tmp_obj)) {
		if (data->strm_idx == -1) {
			for (i = 0; i < MAX_AGTX_RES_OPTION_S_STRM_SIZE; i++) {
				parse_strm_res_option(&(data->strm[i]), json_object_array_get_idx(tmp_obj, i));
			}
		} else {
			parse_strm_res_option(&(data->strm[data->strm_idx]), json_object_array_get_idx(tmp_obj, 0));
		}
	}
}

static void comp_res_entity(struct json_object *array_obj, AGTX_RES_ENTITY_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		int obj_check;
		int k;

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_RES_ENTITY_S_MAX_FRAME_RATE_SIZE; i++) {
				obj_check = 0;
				if (json_object_array_length(tmp1_obj) != 0) {
					//printf("max_frame_rate str: %f\n", data->max_frame_rate[i]);
					for (k = 0; k < json_object_array_length(tmp1_obj); k++) {
						if (data->max_frame_rate[i] == 0.0f &&
						    data->max_frame_rate[i] ==
						            json_object_get_double(
						                    json_object_array_get_idx(tmp1_obj, k))) {
							obj_check = 1;
							//printf("obj_check change: %i\n", obj_check);
						} else {
							/*printf("obj_check didn't change: tmp1_obj[%d]: %f\n",k,
							json_object_get_double(json_object_array_get_idx(tmp1_obj, k)));*/
						}
					}
				}
				
				if (obj_check == 0) {
					json_object_array_add(tmp1_obj, json_object_new_double(data->max_frame_rate[i]));
					//printf("max_frame_rate add: %f\n", data->max_frame_rate[i]);
				}
			}
			json_object_object_add(tmp_obj, "max_frame_rate", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "max_frame_rate");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_RES_ENTITY_S_FRAME_RATE_LIST_SIZE; i++) {
				if (data->frame_rate_list[i] == 0) {
					break;
				}
				
				obj_check = 0;
				if (json_object_array_length(tmp1_obj) != 0) {
					//printf("frame_rate_list str: %f\n", data->frame_rate_list[i]);
					for (k = 0; k < json_object_array_length(tmp1_obj); k++) {
						if (data->frame_rate_list[i] == json_object_get_double(
						json_object_array_get_idx(tmp1_obj, k))) {
							obj_check = 1;
							//printf("obj_check change: %i\n", obj_check);
						} else {
							/*printf("obj_check didn't change: tmp1_obj[k]: %f\n", 
							json_object_get_double(json_object_array_get_idx(tmp1_obj, k)));*/
						}
					}
				}
				
				if (obj_check == 0) {
					json_object_array_add(tmp1_obj, json_object_new_double(data->frame_rate_list[i]));
					//printf("frame_rate_list add: %f\n", data->frame_rate_list[i]);
				}

			}
			json_object_object_add(tmp_obj, "frame_rate_list", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "frame_rate_list");
		}

		tmp1_obj = json_object_new_int(data->height);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "height", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "height");
		}

		tmp1_obj = json_object_new_int(data->width);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "width", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "width");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

static void comp_strm_res_option(struct json_object *array_obj, AGTX_STRM_RES_OPTION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		int i;
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->res_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "res_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "res_idx");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			if (data->res_idx == -1) {
				for (i = 0; i < MAX_AGTX_STRM_RES_OPTION_S_RES_SIZE; i++) {
					comp_res_entity(tmp1_obj, &(data->res[i]));
				}
			} else {
				comp_res_entity(tmp1_obj, &(data->res[data->res_idx]));
			}
			json_object_object_add(tmp_obj, "res", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "res");
		}
		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_res_option(struct json_object *ret_obj, AGTX_RES_OPTION_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_int(data->strm_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "strm_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "strm_idx");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		if (data->strm_idx == -1) {
			for (i = 0; i < MAX_AGTX_RES_OPTION_S_STRM_SIZE; i++) {
				comp_strm_res_option(tmp_obj, &(data->strm[i]));
			}
		} else {
			comp_strm_res_option(tmp_obj, &(data->strm[data->strm_idx]));
		}
		json_object_object_add(ret_obj, "strm", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "strm");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
