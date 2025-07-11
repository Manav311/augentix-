#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>

#include "json.h"
#include "cm_view_type.h"

const char *agtx_view_type_map[] = { "NORMAL", "LDC", "PANORAMA", "PANNING", "SURROUND", "STITCH", "GRAPHICS" };

void parse_view_type(AGTX_VIEW_TYPE_INFO_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	const char *str;
	int i;

	if (json_object_object_get_ex(cmd_obj, "video_win_idx", &tmp_obj)) {
		data->video_win_idx = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "video_win_idx", data->video_win_idx);
	}

	if (json_object_object_get_ex(cmd_obj, "view_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_view_type_map) / sizeof(char *); i++) {
			if (strcmp(agtx_view_type_map[i], str) == 0) {
				data->view_type = (AGTX_WIN_VIEW_TYPE_E)i;
				break;
			}
		}
	}
}

void comp_view_type(struct json_object *ret_obj, AGTX_VIEW_TYPE_INFO_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->video_win_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_win_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_win_idx");
	}

	const char *str;
	str = agtx_view_type_map[data->view_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "view_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "view_type");
	}
}

#ifdef __cplusplus
}
#endif /* __cplusplus */