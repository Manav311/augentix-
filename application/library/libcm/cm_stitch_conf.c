#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_stitch_conf.h"


void parse_stitch_conf(AGTX_STITCH_CONF_S *data, struct json_object *cmd_obj)
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

    if (json_object_object_get_ex(cmd_obj, "enable", &tmp_obj)) {
        data->enable = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "enable", data->enable);
    }

    if (json_object_object_get_ex(cmd_obj, "center_0_x", &tmp_obj)) {
        data->center_0_x = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "center_0_x", data->center_0_x);
    }

    if (json_object_object_get_ex(cmd_obj, "center_0_y", &tmp_obj)) {
        data->center_0_y = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "center_0_y", data->center_0_y);
    }

    if (json_object_object_get_ex(cmd_obj, "center_1_x", &tmp_obj)) {
        data->center_1_x = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "center_1_x", data->center_1_x);
    }

    if (json_object_object_get_ex(cmd_obj, "center_1_y", &tmp_obj)) {
        data->center_1_y = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "center_1_y", data->center_1_y);
    }

    if (json_object_object_get_ex(cmd_obj, "dft_dist", &tmp_obj)) {
        data->dft_dist = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "dft_dist", data->dft_dist);
    }

    if (json_object_object_get_ex(cmd_obj, "dist_tbl_cnt", &tmp_obj)) {
        data->dist_tbl_cnt = json_object_get_int(tmp_obj);
        fprintf(stderr, "%s = %d\n", "dist_tbl_cnt", data->dist_tbl_cnt);
    }

    if (json_object_object_get_ex(cmd_obj, "dist_tbl_list", &tmp_obj)) {
        arr_length = json_object_array_length(tmp_obj);

        for(cnt = 0; cnt < arr_length; cnt++){
            tmp1_obj = json_object_array_get_idx(tmp_obj, cnt);

            if (json_object_object_get_ex(tmp1_obj, "tbl_idx", &tmp2_obj)) {
                data->dist_tbl[cnt].tbl_idx = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "tbl_idx", data->dist_tbl[cnt].tbl_idx);
            }

            if (json_object_object_get_ex(tmp1_obj, "dist", &tmp2_obj)) {
                data->dist_tbl[cnt].dist = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "dist", data->dist_tbl[cnt].dist);
            }

            if (json_object_object_get_ex(tmp1_obj, "ver_disp", &tmp2_obj)) {
                data->dist_tbl[cnt].ver_disp = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "ver_disp", data->dist_tbl[cnt].ver_disp);
            }

            if (json_object_object_get_ex(tmp1_obj, "straighten", &tmp2_obj)) {
                data->dist_tbl[cnt].straighten = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "straighten", data->dist_tbl[cnt].straighten);
            }

            if (json_object_object_get_ex(tmp1_obj, "src_zoom", &tmp2_obj)) {
                data->dist_tbl[cnt].src_zoom = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "src_zoom", data->dist_tbl[cnt].src_zoom);
            }

            if (json_object_object_get_ex(tmp1_obj, "theta_0", &tmp2_obj)) {
                data->dist_tbl[cnt].theta_0 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "theta_0", data->dist_tbl[cnt].theta_0);
            }

            if (json_object_object_get_ex(tmp1_obj, "theta_1", &tmp2_obj)) {
                data->dist_tbl[cnt].theta_1 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "theta_1", data->dist_tbl[cnt].theta_1);
            }

            if (json_object_object_get_ex(tmp1_obj, "radius_0", &tmp2_obj)) {
                data->dist_tbl[cnt].radius_0 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "radius_0", data->dist_tbl[cnt].radius_0);
            }

            if (json_object_object_get_ex(tmp1_obj, "radius_1", &tmp2_obj)) {
                data->dist_tbl[cnt].radius_1 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "radius_1", data->dist_tbl[cnt].radius_1);
            }

            if (json_object_object_get_ex(tmp1_obj, "curvature_0", &tmp2_obj)) {
                data->dist_tbl[cnt].curvature_0 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "curvature_0", data->dist_tbl[cnt].curvature_0);
            }

            if (json_object_object_get_ex(tmp1_obj, "curvature_1", &tmp2_obj)) {
                data->dist_tbl[cnt].curvature_1 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "curvature_1", data->dist_tbl[cnt].curvature_1);
            }

            if (json_object_object_get_ex(tmp1_obj, "fov_ratio_0", &tmp2_obj)) {
                data->dist_tbl[cnt].fov_ratio_0 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "fov_ratio_0", data->dist_tbl[cnt].fov_ratio_0);
            }

            if (json_object_object_get_ex(tmp1_obj, "fov_ratio_1", &tmp2_obj)) {
                data->dist_tbl[cnt].fov_ratio_1 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "fov_ratio_1", data->dist_tbl[cnt].fov_ratio_1);
            }

            if (json_object_object_get_ex(tmp1_obj, "ver_scale_0", &tmp2_obj)) {
                data->dist_tbl[cnt].ver_scale_0 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "ver_scale_0", data->dist_tbl[cnt].ver_scale_0);
            }

            if (json_object_object_get_ex(tmp1_obj, "ver_scale_1", &tmp2_obj)) {
                data->dist_tbl[cnt].ver_scale_1 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "ver_scale_1", data->dist_tbl[cnt].ver_scale_1);
            }

            if (json_object_object_get_ex(tmp1_obj, "ver_shift_0", &tmp2_obj)) {
                data->dist_tbl[cnt].ver_shift_0 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "ver_shift_0", data->dist_tbl[cnt].ver_shift_0);
            }

            if (json_object_object_get_ex(tmp1_obj, "ver_shift_1", &tmp2_obj)) {
                data->dist_tbl[cnt].ver_shift_1 = json_object_get_int(tmp2_obj);
                fprintf(stderr, "%s = %d\n", "ver_shift_1", data->dist_tbl[cnt].ver_shift_1);
            }
        }
    }

    return;
}

void comp_stitch_conf(struct json_object *ret_obj, AGTX_STITCH_CONF_S *data)
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

    tmp_obj = json_object_new_int(data->enable);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "enable", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "enable");
    }

    tmp_obj = json_object_new_int(data->center_0_x);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "center_0_x", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "center_0_x");
    }

    tmp_obj = json_object_new_int(data->center_0_y);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "center_0_y", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "center_0_y");
    }

    tmp_obj = json_object_new_int(data->center_1_x);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "center_1_x", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "center_1_x");
    }

    tmp_obj = json_object_new_int(data->center_1_y);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "center_1_y", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "center_1_y");
    }

    tmp_obj = json_object_new_int(data->dft_dist);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "dft_dist", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "dft_dist");
    }

    tmp_obj = json_object_new_int(data->dist_tbl_cnt);
    if (tmp_obj) {
        json_object_object_add(ret_obj, "dist_tbl_cnt", tmp_obj);
    } else {
        printf("Cannot create %s object\n", "dist_tbl_cnt");
    }

    tmp_obj = json_object_new_array();

    for (cnt = 0; (AGTX_UINT32)cnt < data->dist_tbl_cnt; cnt++) {
	    tmp1_obj = json_object_new_object();

	    if (tmp1_obj) {
		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].tbl_idx);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "tbl_idx", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "tbl_idx");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].dist);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "dist", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "dist");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].ver_disp);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "ver_disp", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "ver_disp");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].straighten);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "straighten", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "straighten");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].src_zoom);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "src_zoom", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "src_zoom");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].theta_0);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "theta_0", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "theta_0");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].theta_1);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "theta_1", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "theta_1");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].radius_0);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "radius_0", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "radius_0");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].radius_1);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "radius_1", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "radius_1");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].curvature_0);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "curvature_0", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "curvature_0");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].curvature_1);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "curvature_1", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "curvature_1");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].fov_ratio_0);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "fov_ratio_0", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "fov_ratio_0");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].fov_ratio_1);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "fov_ratio_1", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "fov_ratio_1");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].ver_scale_0);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "ver_scale_0", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "ver_scale_0");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].ver_scale_1);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "ver_scale_1", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "ver_scale_1");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].ver_shift_0);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "ver_shift_0", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "ver_shift_0");
		    }

		    tmp2_obj = json_object_new_int(data->dist_tbl[cnt].ver_shift_1);
		    if (tmp2_obj) {
			    json_object_object_add(tmp1_obj, "ver_shift_1", tmp2_obj);
		    } else {
			    printf("Cannot create %s object\n", "ver_shift_1");
		    }

		    json_object_array_add(tmp_obj, tmp1_obj);
	    } else {
		    printf("Cannot create array object\n");
	    }
    }

    json_object_object_add(ret_obj, "dist_tbl_list", tmp_obj);

    return;
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
