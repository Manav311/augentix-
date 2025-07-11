#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_osd_pm_conf.h"


void parse_osd_pm(AGTX_OSD_PM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "alpha", &tmp_obj)) {
		data->alpha = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "color", &tmp_obj)) {
		data->color = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "end_x", &tmp_obj)) {
		data->end_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "end_y", &tmp_obj)) {
		data->end_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_x", &tmp_obj)) {
		data->start_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_y", &tmp_obj)) {
		data->start_y = json_object_get_int(tmp_obj);
	}
}

void parse_osd_pm_param(AGTX_OSD_PM_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "param", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE; i++) {
			parse_osd_pm(&(data->param[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_osd_pm_conf(AGTX_OSD_PM_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "conf", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_OSD_PM_CONF_S_CONF_SIZE; i++) {
			parse_osd_pm_param(&(data->conf[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_osd_pm(struct json_object *array_obj, AGTX_OSD_PM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->alpha);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "alpha", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "alpha");
		}

		tmp1_obj = json_object_new_int(data->color);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "color", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "color");
		}

		tmp1_obj = json_object_new_int(data->enabled);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "enabled", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "enabled");
		}

		tmp1_obj = json_object_new_int(data->end_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "end_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "end_x");
		}

		tmp1_obj = json_object_new_int(data->end_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "end_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "end_y");
		}

		tmp1_obj = json_object_new_int(data->start_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "start_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "start_x");
		}

		tmp1_obj = json_object_new_int(data->start_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "start_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "start_y");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_osd_pm_param(struct json_object *array_obj, AGTX_OSD_PM_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_OSD_PM_PARAM_S_PARAM_SIZE; i++) {
				comp_osd_pm(tmp1_obj, &(data->param[i]));
			}
			json_object_object_add(tmp_obj, "param", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "param");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_osd_pm_conf(struct json_object *ret_obj, AGTX_OSD_PM_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_OSD_PM_CONF_S_CONF_SIZE; i++) {
			comp_osd_pm_param(tmp_obj, &(data->conf[i]));
		}
		json_object_object_add(ret_obj, "conf", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "conf");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
