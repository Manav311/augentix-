#include "agtx_types.h"
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_video_dev_conf.h"


void parse_video_dev_conf(AGTX_DEV_CONF_S *data, struct json_object *cmd_obj)
{
	int cnt;
	int arr_length;
	struct json_object *tmp_obj;
	struct json_object *tmp1_obj;
	struct json_object *tmp2_obj;

	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "video_dev_idx", data->video_dev_idx);
	}

	if (json_object_object_get_ex(cmd_obj, "hdr_mode", &tmp_obj)) {
		data->hdr_mode = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "hdr_mode", data->hdr_mode);
	}

	if (json_object_object_get_ex(cmd_obj, "stitch_en", &tmp_obj)) {
		data->stitch_en = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "stitch_en", data->stitch_en);
	}

	if (json_object_object_get_ex(cmd_obj, "eis_en", &tmp_obj)) {
		data->eis_en = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "eis_en", data->eis_en);
	}

	if (json_object_object_get_ex(cmd_obj, "bayer", &tmp_obj)) {
		data->bayer = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "bayer", data->bayer);
	}

	if (json_object_object_get_ex(cmd_obj, "input_fps", &tmp_obj)) {
		data->input_fps = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "input_fps", data->input_fps);
	}

	if (json_object_object_get_ex(cmd_obj, "input_path_cnt", &tmp_obj)) {
		data->input_path_cnt = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "input_path_cnt", data->input_path_cnt);
	}

	if (json_object_object_get_ex(cmd_obj, "input_path_list", &tmp_obj)) {
		arr_length = json_object_array_length(tmp_obj);

		for (cnt = 0; cnt < arr_length; cnt++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, cnt);

			if (json_object_object_get_ex(tmp1_obj, "path_idx", &tmp2_obj)) {
				data->input_path[cnt].path_idx = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "path_idx", data->input_path[cnt].path_idx);
			}

			if (json_object_object_get_ex(tmp1_obj, "path_en", &tmp2_obj)) {
				data->input_path[cnt].path_en = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "path_en", data->input_path[cnt].path_en);
			}

			if (json_object_object_get_ex(tmp1_obj, "sensor_idx", &tmp2_obj)) {
				data->input_path[cnt].sensor_idx = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "sensor_idx", data->input_path[cnt].sensor_idx);
			}

			if (json_object_object_get_ex(tmp1_obj, "path_fps", &tmp2_obj)) {
				data->input_path[cnt].fps = json_object_get_double(tmp2_obj);
				fprintf(stderr, "%s = %f\n", "path_fps", data->input_path[cnt].fps);
			}

			if (json_object_object_get_ex(tmp1_obj, "width", &tmp2_obj)) {
				data->input_path[cnt].width = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "width", data->input_path[cnt].width);
			}

			if (json_object_object_get_ex(tmp1_obj, "height", &tmp2_obj)) {
				data->input_path[cnt].height = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "height", data->input_path[cnt].height);
			}

			if (json_object_object_get_ex(tmp1_obj, "eis_strength", &tmp2_obj)) {
				data->input_path[cnt].eis_strength = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "eis_strength", data->input_path[cnt].eis_strength);
			}
		}
	}

	return;
}

void comp_video_dev_conf(struct json_object *ret_obj, AGTX_DEV_CONF_S *data)
{
    int cnt;
    struct json_object *tmp_obj  = NULL;
    struct json_object *tmp1_obj  = NULL;
    struct json_object *tmp2_obj  = NULL;

    tmp_obj = json_object_new_int(data->video_dev_idx);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "video_dev_idx");
    }

    tmp_obj = json_object_new_int(data->hdr_mode);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "hdr_mode", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "hdr_mode");
    }

    tmp_obj = json_object_new_int(data->stitch_en);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "stitch_en", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "stitch_en");
    }

    tmp_obj = json_object_new_int(data->eis_en);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "eis_en", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "eis_en");
    }

    tmp_obj = json_object_new_int(data->bayer);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "bayer", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "bayer");
    }

    tmp_obj = json_object_new_int(data->input_fps);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "input_fps", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "input_fps");
    }

    tmp_obj = json_object_new_int(data->input_path_cnt);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "input_path_cnt", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "input_path_cnt");
    }

    tmp_obj = json_object_new_array();

    for (cnt = 0; (AGTX_UINT32)cnt < data->input_path_cnt; cnt++) {
	    tmp1_obj = json_object_new_object();

	    if (tmp1_obj) {
		    tmp2_obj = json_object_new_int(data->input_path[cnt].path_idx);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "path_idx", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "path_idx");
		    }

		    tmp2_obj = json_object_new_int(data->input_path[cnt].path_en);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "path_en", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "path_en");
		    }

		    tmp2_obj = json_object_new_int(data->input_path[cnt].sensor_idx);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "sensor_idx", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "sensor_idx");
		    }

		    tmp2_obj = json_object_new_double(data->input_path[cnt].fps);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "path_fps", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "path_fps");
		    }

		    tmp2_obj = json_object_new_int(data->input_path[cnt].width);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "width", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "width");
		    }

		    tmp2_obj = json_object_new_int(data->input_path[cnt].height);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "height", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "height");
		    }

		    tmp2_obj = json_object_new_int(data->input_path[cnt].eis_strength);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "eis_strength", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "eis_strength");
		    }

		    json_object_array_add(tmp_obj, tmp1_obj);
	    } else {
		    printf("Cannot create array object\n");
	    }
    }

    json_object_object_add(ret_obj, "input_path_list", tmp_obj);

    return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
