#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_video_layout_conf.h"


const char * agtx_window_view_type_e_map[] = {
	"NORMAL",
	"LDC",
	"STITCH",
	"PANORAMA",
	"PANNING",
	"SURROUND"
};

void parse_window_param(AGTX_WINDOW_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "path_bmp", &tmp_obj)) {
		data->path_bmp = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_height", &tmp_obj)) {
		data->pos_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_width", &tmp_obj)) {
		data->pos_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_x", &tmp_obj)) {
		data->pos_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_y", &tmp_obj)) {
		data->pos_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "priority", &tmp_obj)) {
		data->priority = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "const_qual", &tmp_obj)) {
		data->const_qual = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "dyn_adj", &tmp_obj)) {
		data->dyn_adj = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "parent", &tmp_obj)) {
		data->parent = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_height", &tmp_obj)) {
		data->roi_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_width", &tmp_obj)) {
		data->roi_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_x", &tmp_obj)) {
		data->roi_x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_y", &tmp_obj)) {
		data->roi_y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "update_fps", &tmp_obj)) {
		data->update_fps = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "eis_en", &tmp_obj)) {
		data->eis_en = json_object_get_int(tmp_obj);
	}
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "view_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_window_view_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_window_view_type_e_map[i], str) == 0) {
				data->view_type = (AGTX_WINDOW_VIEW_TYPE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "window_idx", &tmp_obj)) {
		data->window_idx = json_object_get_int(tmp_obj);
	}
}

void parse_layout_param(AGTX_LAYOUT_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "video_strm_idx", &tmp_obj)) {
		data->video_strm_idx = json_object_get_int(tmp_obj);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "window_array", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_LAYOUT_PARAM_S_WINDOW_ARRAY_SIZE; i++) {
			parse_window_param(&(data->window_array[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "window_num", &tmp_obj)) {
		data->window_num = json_object_get_int(tmp_obj);
	}
}

void parse_layout_conf(AGTX_LAYOUT_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "layout_en", &tmp_obj)) {
		data->layout_en = json_object_get_int(tmp_obj);
		fprintf(stderr, "layout_en = %d\n", data->layout_en);
	}
	if (json_object_object_get_ex(cmd_obj, "layout_num", &tmp_obj)) {
		data->layout_num = json_object_get_int(tmp_obj);
		fprintf(stderr, "layout_num = %d\n", data->layout_num);
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
		fprintf(stderr, "video_dev_idx = %d\n", data->video_dev_idx);
	}
	int i;
	if (json_object_object_get_ex(cmd_obj, "video_layout", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_LAYOUT_CONF_S_VIDEO_LAYOUT_SIZE; i++) {
			parse_layout_param(&(data->video_layout[i]), json_object_array_get_idx(tmp_obj, i));
			fprintf(stderr, "video_layout[%d]: window_num = %d, video_strm_idx = %d\n", i,
			data->video_layout[i].window_num,
			data->video_layout[i].video_strm_idx);
			for (int k = 0; k < data->video_layout[i].window_num; k++) {
				fprintf(stderr, "window_idx[%d]: update_fps = %d, path_bmp = %d, view_type = %d\n",
				        data->video_layout[i].window_array[k].window_idx,
				        data->video_layout[i].window_array[k].update_fps,
				        data->video_layout[i].window_array[k].path_bmp,
				        data->video_layout[i].window_array[k].view_type);
				fprintf(stderr, "               pos_x = %d, pos_y = %d, pos_width = %d, pos_height = %d\n",
				        data->video_layout[i].window_array[k].pos_x,
				        data->video_layout[i].window_array[k].pos_y,
				        data->video_layout[i].window_array[k].pos_width,
				        data->video_layout[i].window_array[k].pos_height);
				fprintf(stderr, "               roi_x = %d, roi_y = %d, roi_width = %d, roi_height = %d\n",
				        data->video_layout[i].window_array[k].roi_x,
				        data->video_layout[i].window_array[k].roi_y,
				        data->video_layout[i].window_array[k].roi_width,
				        data->video_layout[i].window_array[k].roi_height);
				fprintf(stderr, "               priority = %d, const_qual = %d, dyn_adj = %d, parent = %d\n",
				        data->video_layout[i].window_array[k].priority,
				        data->video_layout[i].window_array[k].const_qual,
				        data->video_layout[i].window_array[k].dyn_adj,
				        data->video_layout[i].window_array[k].parent);
				fprintf(stderr, "               eis_en = %d\n",
				        data->video_layout[i].window_array[k].eis_en);
			}
		}
	}
}

void comp_window_param(struct json_object *array_obj, AGTX_WINDOW_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->path_bmp);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "path_bmp", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "path_bmp");
		}

		tmp1_obj = json_object_new_int(data->pos_height);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "pos_height", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "pos_height");
		}

		tmp1_obj = json_object_new_int(data->pos_width);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "pos_width", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "pos_width");
		}

		tmp1_obj = json_object_new_int(data->pos_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "pos_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "pos_x");
		}

		tmp1_obj = json_object_new_int(data->pos_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "pos_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "pos_y");
		}

		tmp1_obj = json_object_new_int(data->priority);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "priority", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "priority");
		}

		tmp1_obj = json_object_new_int(data->const_qual);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "const_qual", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "const_qual");
		}

		tmp1_obj = json_object_new_int(data->dyn_adj);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "dyn_adj", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "dyn_adj");
		}

		tmp1_obj = json_object_new_int(data->parent);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "parent", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "parent");
		}

		tmp1_obj = json_object_new_int(data->roi_height);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "roi_height", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "roi_height");
		}

		tmp1_obj = json_object_new_int(data->roi_width);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "roi_width", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "roi_width");
		}

		tmp1_obj = json_object_new_int(data->roi_x);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "roi_x", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "roi_x");
		}

		tmp1_obj = json_object_new_int(data->roi_y);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "roi_y", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "roi_y");
		}

		tmp1_obj = json_object_new_int(data->update_fps);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "update_fps", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "update_fps");
		}

		tmp1_obj = json_object_new_int(data->eis_en);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "eis_en", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "eis_en");
		}

		const char *str;
		str = agtx_window_view_type_e_map[data->view_type];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "view_type", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "view_type");
		}

		tmp1_obj = json_object_new_int(data->window_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "window_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "window_idx");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_layout_param(struct json_object *array_obj, AGTX_LAYOUT_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->video_strm_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "video_strm_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "video_strm_idx");
		}

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_LAYOUT_PARAM_S_WINDOW_ARRAY_SIZE; i++) {
				comp_window_param(tmp1_obj, &(data->window_array[i]));
			}
			json_object_object_add(tmp_obj, "window_array", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "window_array");
		}

		tmp1_obj = json_object_new_int(data->window_num);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "window_num", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "window_num");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_layout_conf(struct json_object *ret_obj, AGTX_LAYOUT_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->layout_en);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "layout_en", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "layout_en");
	}

	tmp_obj = json_object_new_int(data->layout_num);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "layout_num", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "layout_num");
	}

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_LAYOUT_CONF_S_VIDEO_LAYOUT_SIZE; i++) {
			comp_layout_param(tmp_obj, &(data->video_layout[i]));
		}
		json_object_object_add(ret_obj, "video_layout", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "video_layout");
	}

}

#ifdef __cplusplus
}
#endif /* __cplusplus */
