#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_product_option.h"


static const char * agtx_rc_mode_e_map[] = {
	"VBR",
	"CBR",
	"SBR",
	"CQP",
	"NONE"
};

static const char * agtx_prfl_e_map[] = {
	"BASELINE",
	"MAIN",
	"HIGH",
	"NONE"
};

static const char * agtx_venc_type_e_map[] = {
	"H264",
	"H265",
	"MJPEG",
	"NONE"
};

const char * agtx_product_video_type_e_map[] = {
	"NORMAL",
	"STITCH",
	"BCD"
};

static void parse_rc_vbr_param(_AGTX_RC_VBR_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "max_quality_range", &tmp_obj)) {
		data->max_quality_range = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_quality_range", &tmp_obj)) {
		data->min_quality_range = json_object_get_double(tmp_obj);
	}
}

static void parse_rc_cqp_param(_AGTX_RC_CQP_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "max_qp", &tmp_obj)) {
		data->max_qp = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_qp", &tmp_obj)) {
		data->min_qp = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "q_factor", &tmp_obj)) {
		data->q_factor = json_object_get_int(tmp_obj);
	}
}

static void parse_rc_cbr_param(_AGTX_RC_CBR_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "max_q_factor", &tmp_obj)) {
		data->max_q_factor = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_q_factor", &tmp_obj)) {
		data->min_q_factor = json_object_get_double(tmp_obj);
	}
}

static void parse_venc_option(_AGTX_VENC_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "cbr_param", &tmp_obj)) {
		parse_rc_cbr_param(&(data->cbr_param), tmp_obj);
	}
	int i, j;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "codec", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_venc_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_venc_type_e_map[i], str) == 0) {
				data->codec = (AGTX_VENC_TYPE_E) i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "cqp_param", &tmp_obj)) {
		parse_rc_cqp_param(&(data->cqp_param), tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_bit_rate", &tmp_obj)) {
		data->max_bit_rate = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_gop_size", &tmp_obj)) {
		data->max_gop_size = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_bit_rate", &tmp_obj)) {
		data->min_bit_rate = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_gop_size", &tmp_obj)) {
		data->min_gop_size = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "profile", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < _MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (tmp1_obj) {
				str = json_object_get_string(tmp1_obj);

				for (j = 0; (unsigned long)j < sizeof(agtx_prfl_e_map) / sizeof(char *); j++) {
					if (strcmp(agtx_prfl_e_map[j], str) == 0) {
						data->profile[i] = (AGTX_PRFL_E) j;
						break;
					}
				}
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "rc_mode", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < _MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (tmp1_obj) {
				str = json_object_get_string(tmp1_obj);

				for (j = 0; (unsigned long)j < sizeof(agtx_rc_mode_e_map) / sizeof(char *); j++) {
					if (strcmp(agtx_rc_mode_e_map[j], str) == 0) {
						data->rc_mode[i] = (AGTX_RC_MODE_E) j;
						break;
					}
				}
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "vbr_param", &tmp_obj)) {
		parse_rc_vbr_param(&(data->vbr_param), tmp_obj);
	}
}

static void parse_res_option(_AGTX_RES_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "max_frame_rate", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < _MAX_AGTX_RES_OPTION_S_MAX_FRAME_RATE_SIZE; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (tmp1_obj) {
				data->max_frame_rate[i] = json_object_get_double(tmp1_obj);
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "frame_rate_list", &tmp_obj)) {
		struct json_object *tmp1_obj = NULL;

		for (i = 0; i < _MAX_AGTX_RES_OPTION_S_FRAME_RATE_LIST_SIZE; i++) {
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

static void parse_video_option(_AGTX_VIDEO_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	if (json_object_object_get_ex(cmd_obj, "res", &tmp_obj)) {
		for (i = 0; i < _MAX_AGTX_VIDEO_OPTION_S_RES_SIZE; i++) {
			parse_res_option(&(data->res[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
	if (json_object_object_get_ex(cmd_obj, "venc", &tmp_obj)) {
		for (i = 0; i < _MAX_AGTX_VIDEO_OPTION_S_VENC_SIZE; i++) {
			parse_venc_option(&(data->venc[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

void parse_product_option(_AGTX_PRODUCT_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;
	const char *str;

	if (json_object_object_get_ex(cmd_obj, "product_video_type", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (unsigned long)i < sizeof(agtx_product_video_type_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_product_video_type_e_map[i], str) == 0) {
				data->product_video_type = (AGTX_PRODUCT_VIDEO_TYPE_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "video_option", &tmp_obj)) {
		for (i = 0; i < _MAX_AGTX_PRODUCT_OPTION_S_VIDEO_OPTION_SIZE; i++) {
			parse_video_option(&(data->video_option[i]), json_object_array_get_idx(tmp_obj, i));
		}
	}
}

static void comp_rc_vbr_param(struct json_object *ret_obj, _AGTX_RC_VBR_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_double(data->max_quality_range);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_quality_range", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_quality_range");
	}

	tmp_obj = json_object_new_double(data->min_quality_range);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_quality_range", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_quality_range");
	}

}

static void comp_rc_cqp_param(struct json_object *ret_obj, _AGTX_RC_CQP_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_int(data->max_qp);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_qp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_qp");
	}

	tmp_obj = json_object_new_int(data->min_qp);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_qp", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_qp");
	}

	tmp_obj = json_object_new_int(data->q_factor);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "q_factor", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "q_factor");
	}

}

static void comp_rc_cbr_param(struct json_object *ret_obj, _AGTX_RC_CBR_PARAM_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_double(data->max_q_factor);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "max_q_factor", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "max_q_factor");
	}

	tmp_obj = json_object_new_double(data->min_q_factor);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "min_q_factor", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "min_q_factor");
	}

}

static void comp_venc_option(struct json_object *array_obj, _AGTX_VENC_OPTION_S *data)
{
	const char *str;
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;
		struct json_object *tmp2_obj = NULL;

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_rc_cbr_param(tmp1_obj, &(data->cbr_param));
			json_object_object_add(tmp_obj, "cbr_param", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "cbr_param");
		}

		str = agtx_venc_type_e_map[data->codec];
		tmp1_obj = json_object_new_string(str);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "codec", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "codec");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_rc_cqp_param(tmp1_obj, &(data->cqp_param));
			json_object_object_add(tmp_obj, "cqp_param", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "cqp_param");
		}

		tmp1_obj = json_object_new_int(data->max_bit_rate);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "max_bit_rate", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "max_bit_rate");
		}

		tmp1_obj = json_object_new_int(data->max_gop_size);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "max_gop_size", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "max_gop_size");
		}

		tmp1_obj = json_object_new_int(data->min_bit_rate);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "min_bit_rate", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "min_bit_rate");
		}

		tmp1_obj = json_object_new_int(data->min_gop_size);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "min_gop_size", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "min_gop_size");
		}

		int i;
		int obj_check;
		int k;

		tmp1_obj = json_object_new_array();
		//printf("profile process start\n");
		if (tmp1_obj) {
			for (i = 0; i < _MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE; i++) {
				if (data->profile[i] == _MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE) {
					break;
				}
				str = agtx_prfl_e_map[data->profile[i]];
				tmp2_obj = json_object_new_string(str);
				if (tmp2_obj) {
					obj_check = 0;
					printf("obj_check: %i\n", obj_check);
					if (json_object_array_length(tmp2_obj) != 0) {
						//printf("profile str: %s\n", json_object_to_json_string(tmp2_obj));
					}
					if (json_object_array_length(tmp1_obj) != 0) {
						for (k = 0; k < json_object_array_length(tmp1_obj); k++) {
							if (strcmp(json_object_to_json_string(tmp2_obj),
							json_object_to_json_string(json_object_array_get_idx(tmp1_obj, k))) == 0) {
								obj_check = 1;
								/*printf("obj_check change: %i\n", obj_check);
							} else {
								printf("obj_check didn't change: tmp1_obj[k]: %s\n",
								json_object_to_json_string(json_object_array_get_idx(tmp1_obj, k)));*/
							}
						}
					}
					if (obj_check == 0) {
						json_object_array_add(tmp1_obj, tmp2_obj);
						//printf("profile add: %s\n", json_object_to_json_string(tmp2_obj));
					}
				} else {
					printf("Cannot create %s object\n", "profile");
				}
			}
			json_object_object_add(tmp_obj, "profile", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "profile");
		}

		tmp1_obj = json_object_new_array();
		//printf("rc_mode process start\n");
		if (tmp1_obj) {
			for (i = 0; i < _MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE; i++) {
				if (data->rc_mode[i] == _MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE) {
					break;
				}
				str = agtx_rc_mode_e_map[data->rc_mode[i]];
				tmp2_obj = json_object_new_string(str);
				if (tmp2_obj) {
					obj_check = 0;
					//printf("obj_check: %i\n", obj_check);
					if (json_object_array_length(tmp2_obj) != 0) {
						//printf("rc_mode str: %s\n", json_object_to_json_string(tmp2_obj));
					}
					if (json_object_array_length(tmp1_obj) != 0) {
						for (k = 0; k < json_object_array_length(tmp1_obj); k++) {
							if (strcmp(json_object_to_json_string(tmp2_obj),
							json_object_to_json_string(json_object_array_get_idx(tmp1_obj, k))) == 0) {
								obj_check = 1;
								/*printf("obj_check change: %i\n", obj_check);
							} else {
								printf("obj_check didn't change: tmp1_obj[k]: %s\n",
								json_object_to_json_string(json_object_array_get_idx(tmp1_obj, k)));*/
							}
						}
					}
					if (obj_check == 0) {
						json_object_array_add(tmp1_obj, tmp2_obj);
						//printf("rc_mode add: %s\n", json_object_to_json_string(tmp2_obj));
					}
				} else {
					printf("Cannot create %s object\n", "rc_mode");
				}
			}
			json_object_object_add(tmp_obj, "rc_mode", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "rc_mode");
		}

		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			comp_rc_vbr_param(tmp1_obj, &(data->vbr_param));
			json_object_object_add(tmp_obj, "vbr_param", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "vbr_param");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

static void comp_res_option(struct json_object *array_obj, _AGTX_RES_OPTION_S *data)
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
			for (i = 0; i < _MAX_AGTX_RES_OPTION_S_MAX_FRAME_RATE_SIZE; i++) {
				obj_check = 0;
				if (json_object_array_length(tmp1_obj) != 0) {
					//printf("max_frame_rate str: %f\n", data->max_frame_rate[i]);
					for (k = 0; k < json_object_array_length(tmp1_obj); k++) {
						if (data->max_frame_rate[i] == json_object_get_double(
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
			for (i = 0; i < _MAX_AGTX_RES_OPTION_S_FRAME_RATE_LIST_SIZE; i++) {
				/* TODO: define value for valid frame rate */
				if (data->frame_rate_list[i] == 0) {
					break;
				}

				obj_check = 0;
				if (json_object_array_length(tmp1_obj) != 0) {
					//printf("frame_rate_list str: %f\n", data->frame_rate_list[i]);
					for (k = 0; k < json_object_array_length(tmp1_obj); k++) {
						if (data->frame_rate_list[i] == json_object_get_double(json_object_array_get_idx(tmp1_obj, k))) {
							obj_check = 1;
							//printf("obj_check change: %i\n", obj_check);
						} else {
							//printf("obj_check didn't change: tmp1_obj[k]: %f\n", json_object_get_double(json_object_array_get_idx(tmp1_obj, k)));
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

static void comp_video_option(struct json_object *array_obj, _AGTX_VIDEO_OPTION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		struct json_object *tmp1_obj = NULL;

		int i;
		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < _MAX_AGTX_VIDEO_OPTION_S_RES_SIZE; i++) {
				comp_res_option(tmp1_obj, &(data->res[i]));
			}
			json_object_object_add(tmp_obj, "res", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "res");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			for (i = 0; i < _MAX_AGTX_VIDEO_OPTION_S_VENC_SIZE; i++) {
				comp_venc_option(tmp1_obj, &(data->venc[i]));
			}
			json_object_object_add(tmp_obj, "venc", tmp1_obj);
		} else {
			printf("Cannot create array %s object\n", "venc");
		}

		json_object_array_add(array_obj, tmp_obj);
	} else {
		printf("Cannot create object in array\n");
	}
}

void comp_product_option(struct json_object *ret_obj, _AGTX_PRODUCT_OPTION_S *data)
{
	struct json_object *tmp_obj = NULL;
	int i;
	const char *str;

	str = agtx_product_video_type_e_map[data->product_video_type];
	tmp_obj = json_object_new_string(str);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "product_video_type", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "product_video_type");
	}

	tmp_obj = json_object_new_array();
	if (tmp_obj) {
		for (i = 0; i < _MAX_AGTX_PRODUCT_OPTION_S_VIDEO_OPTION_SIZE; i++) {
			comp_video_option(tmp_obj, &(data->video_option[i]));
		}
		json_object_object_add(ret_obj, "video_option", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "video_option");
	}

}


static void init_rc_vbr_param(_AGTX_RC_VBR_PARAM_S *data)
{
	data->max_quality_range = 0;
	data->min_quality_range = 0;
}

static void init_rc_cbr_param(_AGTX_RC_CBR_PARAM_S *data)
{
	data->max_q_factor = 0;
	data->min_q_factor = 0;
}

static void init_rc_cqp_param(_AGTX_RC_CQP_PARAM_S *data)
{
	data->max_qp = 0;
	data->min_qp = 0;
	data->q_factor = 0;
}

static void init_venc_option(_AGTX_VENC_OPTION_S *data)
{
	int i;

	init_rc_vbr_param(&(data->vbr_param));
	init_rc_cbr_param(&(data->cbr_param));
	init_rc_cqp_param(&(data->cqp_param));

	data->codec = 0;
	data->max_bit_rate = 0;
	data->min_bit_rate = 0;
	data->max_gop_size = 0;
	data->min_gop_size = 0;

	for (i = 0; i < _MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE; i++) {
		data->profile[i] = _MAX_AGTX_VENC_OPTION_S_PROFILE_SIZE;
	}

	for (i = 0; i < _MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE; i++) {
		data->rc_mode[i] = _MAX_AGTX_VENC_OPTION_S_RC_MODE_SIZE;
	}
}

static void init_res_option(_AGTX_RES_OPTION_S *data)
{
	int i;

	for (i = 0; i < _MAX_AGTX_RES_OPTION_S_MAX_FRAME_RATE_SIZE; i++) {
		data->max_frame_rate[i] = 0;
	}

	for (i = 0; i < _MAX_AGTX_RES_OPTION_S_FRAME_RATE_LIST_SIZE; i++) {
		data->frame_rate_list[i] = 0;
	}

	data->height = 0;
	data->width = 0;
}

static void init_video_option(_AGTX_VIDEO_OPTION_S *data)
{
	int i;

	for (i = 0; i < _MAX_AGTX_VIDEO_OPTION_S_RES_SIZE; i++) {
		init_res_option(&(data->res[i]));
	}

	for (i = 0; i < _MAX_AGTX_VIDEO_OPTION_S_VENC_SIZE; i++) {
		init_venc_option(&(data->venc[i]));
	}
}

void init_product_option(_AGTX_PRODUCT_OPTION_S *data)
{
	int i;

	data->product_video_type = AGTX_PRODUCT_VIDEO_TYPE_NORMAL;

	for (i = 0; i < _MAX_AGTX_PRODUCT_OPTION_S_VIDEO_OPTION_SIZE; i++) {
		init_video_option(&(data->video_option[i]));
	}
}


#ifdef __cplusplus
}
#endif /* __cplusplus */
