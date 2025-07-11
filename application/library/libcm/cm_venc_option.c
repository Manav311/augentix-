#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#include <stdio.h>
#include <string.h>

#include "json.h"

#include "agtx_venc_option.h"


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

static void parse_rc_vbr_param(AGTX_RC_VBR_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "max_quality_range", &tmp_obj)) {
		data->max_quality_range = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_quality_range", &tmp_obj)) {
		data->min_quality_range = json_object_get_double(tmp_obj);
	}
}

static void parse_rc_cqp_param(AGTX_RC_CQP_PARAM_S *data, struct json_object *cmd_obj)
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

static void parse_rc_cbr_param(AGTX_RC_CBR_PARAM_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	if (json_object_object_get_ex(cmd_obj, "max_q_factor", &tmp_obj)) {
		data->max_q_factor = json_object_get_double(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_q_factor", &tmp_obj)) {
		data->min_q_factor = json_object_get_double(tmp_obj);
	}
}

static void parse_venc_entity(AGTX_VENC_ENTITY_S *data, struct json_object *cmd_obj)
{
	int i, j;
	const char *str;
	struct json_object *tmp_obj;

	if (json_object_object_get_ex(cmd_obj, "cbr_param", &tmp_obj)) {
		parse_rc_cbr_param(&(data->cbr_param), tmp_obj);
	}

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

		for (i = 0; i < MAX_AGTX_VENC_ENTITY_S_PROFILE_SIZE; i++) {
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

		for (i = 0; i < MAX_AGTX_VENC_ENTITY_S_RC_MODE_SIZE; i++) {
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

static void parse_strm_venc_option(AGTX_STRM_VENC_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "venc_idx", &tmp_obj)) {
		data->venc_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "venc", &tmp_obj)) {
		if (data->venc_idx == -1) {
			for (i = 0; i < MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE; i++) {
				parse_venc_entity(&(data->venc[i]), json_object_array_get_idx(tmp_obj, i));
			}
		} else {
			parse_venc_entity(&(data->venc[data->venc_idx]), json_object_array_get_idx(tmp_obj, 0));
		}
	}
}

void parse_venc_option(AGTX_VENC_OPTION_S *data, struct json_object *cmd_obj)
{
	struct json_object *tmp_obj;
	int i;

	if (json_object_object_get_ex(cmd_obj, "strm_idx", &tmp_obj)) {
		data->strm_idx = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "strm", &tmp_obj)) {
		if (data->strm_idx == -1) {
			for (i = 0; i < MAX_AGTX_VENC_OPTION_S_STRM_SIZE; i++) {
				parse_strm_venc_option(&(data->strm[i]), json_object_array_get_idx(tmp_obj, i));
			}
		} else {
			parse_strm_venc_option(&(data->strm[data->strm_idx]), json_object_array_get_idx(tmp_obj, 0));
		}
	}
}

static void comp_rc_vbr_param(struct json_object *ret_obj, AGTX_RC_VBR_PARAM_S *data)
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

static void comp_rc_cqp_param(struct json_object *ret_obj, AGTX_RC_CQP_PARAM_S *data)
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

static void comp_rc_cbr_param(struct json_object *ret_obj, AGTX_RC_CBR_PARAM_S *data)
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

static void comp_venc_entity(struct json_object *array_obj, AGTX_VENC_ENTITY_S *data)
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
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_VENC_ENTITY_S_PROFILE_SIZE; i++) {
				if (data->profile[i] == AGTX_PRFL_NONE) {
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
		if (tmp1_obj) {
			for (i = 0; i < MAX_AGTX_VENC_ENTITY_S_RC_MODE_SIZE; i++) {
				if (data->rc_mode[i] == AGTX_RC_MODE_NONE) {
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

static void comp_strm_venc_option(struct json_object *array_obj, AGTX_STRM_VENC_OPTION_S *data)
{
	struct json_object *tmp_obj = NULL;

	tmp_obj = json_object_new_object();

	if (tmp_obj) {
		int i;
		struct json_object *tmp1_obj = NULL;

		tmp1_obj = json_object_new_int(data->venc_idx);
		if (tmp1_obj) {
			json_object_object_add(tmp_obj, "venc_idx", tmp1_obj);
		} else {
			printf("Cannot create %s object\n", "venc_idx");
		}

		tmp1_obj = json_object_new_array();
		if (tmp1_obj) {
			if (data->venc_idx == -1) {
				for (i = 0; i < MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE; i++) {
					comp_venc_entity(tmp1_obj, &(data->venc[i]));
				}
			} else {
				comp_venc_entity(tmp1_obj, &(data->venc[data->venc_idx]));
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

void comp_venc_option(struct json_object *ret_obj, AGTX_VENC_OPTION_S *data)
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
			for (i = 0; i < MAX_AGTX_VENC_OPTION_S_STRM_SIZE; i++) {
				comp_strm_venc_option(tmp_obj, &(data->strm[i]));
			}
		} else {
			comp_strm_venc_option(tmp_obj, &(data->strm[data->strm_idx]));
		}
		json_object_object_add(ret_obj, "strm", tmp_obj);
	} else {
		printf("Cannot create array %s object\n", "strm");
	}
}

static void init_rc_vbr_param(AGTX_RC_VBR_PARAM_S *data)
{
	data->max_quality_range = 0;
	data->min_quality_range = 0;
}

static void init_rc_cbr_param(AGTX_RC_CBR_PARAM_S *data)
{
	data->max_q_factor = 0;
	data->min_q_factor = 0;
}

static void init_rc_cqp_param(AGTX_RC_CQP_PARAM_S *data)
{
	data->max_qp = 0;
	data->min_qp = 0;
	data->q_factor = 0;
}

static void init_venc_entity(AGTX_VENC_ENTITY_S *data)
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

	for (i = 0; i < MAX_AGTX_VENC_ENTITY_S_PROFILE_SIZE; i++) {
		data->profile[i] = AGTX_PRFL_NONE;
	}

	for (i = 0; i < MAX_AGTX_VENC_ENTITY_S_RC_MODE_SIZE; i++) {
		data->rc_mode[i] = AGTX_RC_MODE_NONE;
	}
}

static void init_strm_venc_option(AGTX_STRM_VENC_OPTION_S *data)
{
	int i;

	for (i = 0; i < MAX_AGTX_STRM_VENC_OPTION_S_VENC_SIZE; i++) {
		init_venc_entity(&(data->venc[i]));
	}

	data->venc_idx = 0;
}

void init_venc_option(AGTX_VENC_OPTION_S *data)
{
	int i;

	for (i = 0; i < MAX_AGTX_VENC_OPTION_S_STRM_SIZE; i++) {
		init_strm_venc_option(&(data->strm[i]));
	}

	data->strm_idx = 0;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
