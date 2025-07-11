#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>
#include "json.h"
#include "cm_dip_roi_conf.h"

void parse_dip_roi_attr(AGTX_DIP_ROI_ATTR_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "awb_roi_ex", &tmp_obj)) {
		data->awb_roi_ex = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "awb_roi_ey", &tmp_obj)) {
		data->awb_roi_ey = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "awb_roi_sx", &tmp_obj)) {
		data->awb_roi_sx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "awb_roi_sy", &tmp_obj)) {
		data->awb_roi_sy = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "luma_roi_ex", &tmp_obj)) {
		data->luma_roi_ex = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "luma_roi_ey", &tmp_obj)) {
		data->luma_roi_ey = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "luma_roi_sx", &tmp_obj)) {
		data->luma_roi_sx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "luma_roi_sy", &tmp_obj)) {
		data->luma_roi_sy = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zone_lum_avg_roi_ex", &tmp_obj)) {
		data->zone_lum_avg_roi_ex = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zone_lum_avg_roi_ey", &tmp_obj)) {
		data->zone_lum_avg_roi_ey = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zone_lum_avg_roi_sx", &tmp_obj)) {
		data->zone_lum_avg_roi_sx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "zone_lum_avg_roi_sy", &tmp_obj)) {
		data->zone_lum_avg_roi_sy = json_object_get_int(tmp_obj);
	}
}

void parse_dip_roi_conf(AGTX_DIP_ROI_CONF_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "roi", &tmp_obj)) {
		for (i = 0; i < MAX_AGTX_DIP_ROI_CONF_S_ROI_SIZE; i++) {
			parse_dip_roi_attr(&(data->roi[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_dev_idx", &tmp_obj)) {
		data->video_dev_idx = json_object_get_int(tmp_obj);
	}
}

void comp_dip_roi_attr(struct json_object *array_obj, AGTX_DIP_ROI_ATTR_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->awb_roi_ex);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "awb_roi_ex", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "awb_roi_ex");
		}

		tmp1_obj = json_object_new_int(data->awb_roi_ey);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "awb_roi_ey", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "awb_roi_ey");
		}

		tmp1_obj = json_object_new_int(data->awb_roi_sx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "awb_roi_sx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "awb_roi_sx");
		}

		tmp1_obj = json_object_new_int(data->awb_roi_sy);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "awb_roi_sy", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "awb_roi_sy");
		}

		tmp1_obj = json_object_new_int(data->luma_roi_ex);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "luma_roi_ex", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "luma_roi_ex");
		}

		tmp1_obj = json_object_new_int(data->luma_roi_ey);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "luma_roi_ey", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "luma_roi_ey");
		}

		tmp1_obj = json_object_new_int(data->luma_roi_sx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "luma_roi_sx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "luma_roi_sx");
		}

		tmp1_obj = json_object_new_int(data->luma_roi_sy);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "luma_roi_sy", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "luma_roi_sy");
		}

		tmp1_obj = json_object_new_int(data->zone_lum_avg_roi_ex);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "zone_lum_avg_roi_ex", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "zone_lum_avg_roi_ex");
		}

		tmp1_obj = json_object_new_int(data->zone_lum_avg_roi_ey);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "zone_lum_avg_roi_ey", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "zone_lum_avg_roi_ey");
		}

		tmp1_obj = json_object_new_int(data->zone_lum_avg_roi_sx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "zone_lum_avg_roi_sx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "zone_lum_avg_roi_sx");
		}

		tmp1_obj = json_object_new_int(data->zone_lum_avg_roi_sy);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "zone_lum_avg_roi_sy", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "zone_lum_avg_roi_sy");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_dip_roi_conf(struct json_object *ret_obj, AGTX_DIP_ROI_CONF_S *data)
{
	struct json_object *tmp_obj = NULL;

	int i;
	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < MAX_AGTX_DIP_ROI_CONF_S_ROI_SIZE; i++) {
			comp_dip_roi_attr(tmp_obj, &(data->roi[i]));
		}
		json_object_object_add(ret_obj, "roi", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "roi");
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
