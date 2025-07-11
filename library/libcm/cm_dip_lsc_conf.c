#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_dip_lsc_conf.h"


void parse_agtx_dip_lsc_attr_s(AGTX_DIP_LSC_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "origin", &tmp_obj)) {
		data->origin = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "tilt", &tmp_obj)) {
		data->tilt = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "x_curvature", &tmp_obj)) {
		data->x_curvature = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "x_trend", &tmp_obj)) {
		data->x_trend = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "y_curvature", &tmp_obj)) {
		data->y_curvature = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "y_trend", &tmp_obj)) {
		data->y_trend = json_object_get_int(tmp_obj);
	}
}

void parse_dip_lsc_conf(AGTX_DIP_LSC_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "lsc", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_LSC_CONF_S_LSC_SIZE; i++) {
			parse_agtx_dip_lsc_attr_s(&(data->lsc[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_lsc_attr(struct json_object *ret_obj, AGTX_DIP_LSC_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->origin);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "origin", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "origin");
		}

		tmp1_obj = json_object_new_int(data->tilt);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "tilt", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "tilt");
		}

		tmp1_obj = json_object_new_int(data->x_curvature);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "x_curvature", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "x_curvature");
		}

		tmp1_obj = json_object_new_int(data->x_trend);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "x_trend", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "x_trend");
		}

		tmp1_obj = json_object_new_int(data->y_curvature);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "y_curvature", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "y_curvature");
		}

		tmp1_obj = json_object_new_int(data->y_trend);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "y_trend", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "y_trend");
		}

		json_object_array_add(ret_obj, tmp_obj);
	} else {
		printf("Cannot create array object\n");
	}
}

void comp_dip_lsc_conf(struct json_object *ret_obj, AGTX_DIP_LSC_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_LSC_CONF_S_LSC_SIZE; i++) {
			comp_dip_lsc_attr(tmp_obj, &(data->lsc[i]));
		}
		json_object_object_add(ret_obj, "lsc", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "lsc");
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
