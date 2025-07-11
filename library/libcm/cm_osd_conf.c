#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_osd_conf.h"


#define min(X,Y) (((X) < (Y)) ? (X) : (Y))


const char * agtx_osd_type_e_map[] = {
	"TEXT",
	"IMAGE",
	"INFO"
};


void parse_osd_conf_inner(AGTX_OSD_CONF_INNER_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "enabled", &tmp_obj)) {
		data->enabled = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_x", &tmp_obj)) {
		data->start_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "start_y", &tmp_obj)) {
		data->start_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_osd_type_e_map) / sizeof(char *); i++) {
			if(strcmp(agtx_osd_type_e_map[i], str) == 0) {
				data->type = (AGTX_OSD_TYPE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "type_spec", &tmp_obj)) {
		i = min(MAX_AGTX_OSD_CONF_INNER_S_TYPE_SPEC_SIZE, json_object_get_string_len(tmp_obj));
		strncpy((char *)data->type_spec, json_object_get_string(tmp_obj), i);
		data->type_spec[i] = '\0';
	}
}

void parse_osd_conf_outer(AGTX_OSD_CONF_OUTER_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "region", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
			parse_osd_conf_inner(&(data->region[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_osd_conf(AGTX_OSD_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "showWeekDay", &tmp_obj)) {
		data->showWeekDay = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "strm", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_S_STRM_SIZE; i++) {
			parse_osd_conf_outer(&(data->strm[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void comp_osd_conf_inner(struct json_object *array_obj, AGTX_OSD_CONF_INNER_S *data)
{
	struct json_object *tmp_obj = NULL, *elm_obj = NULL;
	const char *str;

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		elm_obj = json_object_new_int(data->enabled);
		if (elm_obj) {
			json_object_object_add(tmp_obj, "enabled", elm_obj);
		} else {
			printf("Cannot create %s object\n", "enabled");
		}

		elm_obj = json_object_new_int(data->start_x);
		if (elm_obj) {
			json_object_object_add(tmp_obj, "start_x", elm_obj);
		} else {
			printf("Cannot create %s object\n", "start_x");
		}

		elm_obj = json_object_new_int(data->start_y);
		if (elm_obj) {
			json_object_object_add(tmp_obj, "start_y", elm_obj);
		} else {
			printf("Cannot create %s object\n", "start_y");
		}

		str = agtx_osd_type_e_map[data->type];
		elm_obj = json_object_new_string(str);
		if (elm_obj) {
			json_object_object_add(tmp_obj, "type", elm_obj);
		} else {
			printf("Cannot create %s object\n", "type");
		}

		str = (const char *)data->type_spec;
		elm_obj = json_object_new_string(str);
		if (elm_obj) {
			json_object_object_add(tmp_obj, "type_spec", elm_obj);
		} else {
			printf("Cannot create %s object\n", "type_spec");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create array object\n");
	}
}

void comp_osd_conf_outer(struct json_object *array_obj, AGTX_OSD_CONF_OUTER_S *data)
{
	struct json_object *tmp_obj = NULL, *tmp1_obj = NULL;
	int i;

	tmp_obj = json_object_new_object();
	if (tmp_obj) {
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_OSD_CONF_OUTER_S_REGION_SIZE; i++) {
				comp_osd_conf_inner(tmp1_obj, &(data->region[i]));
			}
			json_object_object_add(tmp_obj, "region", tmp1_obj);
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create array object\n");
	}

}

void comp_osd_conf(struct json_object *ret_obj, AGTX_OSD_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;
	int i;

	tmp_obj = json_object_new_int(data->showWeekDay);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "showWeekDay", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "showWeekDay");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_OSD_CONF_S_STRM_SIZE; i++) {
			comp_osd_conf_outer(tmp_obj, &(data->strm[i]));
		}
		json_object_object_add(ret_obj, "strm", tmp_obj);
	} else {
		printf("Cannot create array object\n");
	}
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
