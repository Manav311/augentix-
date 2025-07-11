#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <string.h>

#include "json.h"
#include "agtx_video_strm_conf.h"

void parse_video_strm_conf(AGTX_STRM_CONF_S *data, struct json_object *cmd_obj)
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

	if (json_object_object_get_ex(cmd_obj, "video_strm_cnt", &tmp_obj)) {
		data->video_strm_cnt = json_object_get_int(tmp_obj);
		fprintf(stderr, "%s = %d\n", "video_strm_cnt", data->video_strm_cnt);
	}

	if (json_object_object_get_ex(cmd_obj, "video_strm_list", &tmp_obj)) {
		arr_length = json_object_array_length(tmp_obj);
		for (cnt = 0; cnt < arr_length; cnt++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, cnt);

			if (json_object_object_get_ex(tmp1_obj, "video_strm_idx", &tmp2_obj)) {
				data->video_strm[cnt].video_strm_idx = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "video_strm_idx", data->video_strm[cnt].video_strm_idx);
			}

			if (json_object_object_get_ex(tmp1_obj, "strm_en", &tmp2_obj)) {
				data->video_strm[cnt].strm_en = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "strm_en", data->video_strm[cnt].strm_en);
			}

			if (json_object_object_get_ex(tmp1_obj, "width", &tmp2_obj)) {
				data->video_strm[cnt].width = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "width", data->video_strm[cnt].width);
			}

			if (json_object_object_get_ex(tmp1_obj, "height", &tmp2_obj)) {
				data->video_strm[cnt].height = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "height", data->video_strm[cnt].height);
			}

			if (json_object_object_get_ex(tmp1_obj, "output_fps", &tmp2_obj)) {
				data->video_strm[cnt].output_fps = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "output_fps", data->video_strm[cnt].output_fps);
			}

			if (json_object_object_get_ex(tmp1_obj, "binding_capability", &tmp2_obj)) {
				data->video_strm[cnt].binding_capability = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "binding_capability",
				        data->video_strm[cnt].binding_capability);
			}

			if (json_object_object_get_ex(tmp1_obj, "rotate", &tmp2_obj)) {
				data->video_strm[cnt].rotate = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "rotate", data->video_strm[cnt].rotate);
			}

			if (json_object_object_get_ex(tmp1_obj, "mirr_en", &tmp2_obj)) {
				data->video_strm[cnt].mirr_en = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "mirr_en", data->video_strm[cnt].mirr_en);
			}

			if (json_object_object_get_ex(tmp1_obj, "flip_en", &tmp2_obj)) {
				data->video_strm[cnt].flip_en = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "flip_en", data->video_strm[cnt].flip_en);
			}

			if (json_object_object_get_ex(tmp1_obj, "venc_type", &tmp2_obj)) {
				data->video_strm[cnt].venc_type = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "venc_type", data->video_strm[cnt].venc_type);
			}

			if (json_object_object_get_ex(tmp1_obj, "venc_profile", &tmp2_obj)) {
				data->video_strm[cnt].venc_profile = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "venc_profile", data->video_strm[cnt].venc_profile);
			}

			if (json_object_object_get_ex(tmp1_obj, "rc_mode", &tmp2_obj)) {
				data->video_strm[cnt].rc_mode = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "rc_mode", data->video_strm[cnt].rc_mode);
			}

			if (json_object_object_get_ex(tmp1_obj, "gop_size", &tmp2_obj)) {
				data->video_strm[cnt].gop_size = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "gop_size", data->video_strm[cnt].gop_size);
			}

			if (json_object_object_get_ex(tmp1_obj, "max_frame_size", &tmp2_obj)) {
				data->video_strm[cnt].max_frame_size = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "max_frame_size", data->video_strm[cnt].max_frame_size);
			}

			if (json_object_object_get_ex(tmp1_obj, "bit_rate", &tmp2_obj)) {
				data->video_strm[cnt].bit_rate = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "bit_rate", data->video_strm[cnt].bit_rate);
			}

			if (json_object_object_get_ex(tmp1_obj, "min_qp", &tmp2_obj)) {
				data->video_strm[cnt].min_qp = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "min_qp", data->video_strm[cnt].min_qp);
			}

			if (json_object_object_get_ex(tmp1_obj, "max_qp", &tmp2_obj)) {
				data->video_strm[cnt].max_qp = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "max_qp", data->video_strm[cnt].max_qp);
			}

			if (json_object_object_get_ex(tmp1_obj, "min_q_factor", &tmp2_obj)) {
				data->video_strm[cnt].min_q_factor = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "min_q_factor", data->video_strm[cnt].min_q_factor);
			}

			if (json_object_object_get_ex(tmp1_obj, "max_q_factor", &tmp2_obj)) {
				data->video_strm[cnt].max_q_factor = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "max_q_factor", data->video_strm[cnt].max_q_factor);
			}

			if (json_object_object_get_ex(tmp1_obj, "fluc_level", &tmp2_obj)) {
				data->video_strm[cnt].fluc_level = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "fluc_level", data->video_strm[cnt].fluc_level);
			}

			if (json_object_object_get_ex(tmp1_obj, "scene_smooth", &tmp2_obj)) {
				data->video_strm[cnt].scene_smooth = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "scene_smooth", data->video_strm[cnt].scene_smooth);
			}

			if (json_object_object_get_ex(tmp1_obj, "regression_speed", &tmp2_obj)) {
				data->video_strm[cnt].regression_speed = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "regression_speed", data->video_strm[cnt].regression_speed);
			}

			if (json_object_object_get_ex(tmp1_obj, "i_continue_weight", &tmp2_obj)) {
				data->video_strm[cnt].i_continue_weight = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "i_continue_weight", data->video_strm[cnt].i_continue_weight);
			}

			if (json_object_object_get_ex(tmp1_obj, "i_qp_offset", &tmp2_obj)) {
				data->video_strm[cnt].i_qp_offset = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "i_qp_offset", data->video_strm[cnt].i_qp_offset);
			}

			if (json_object_object_get_ex(tmp1_obj, "motion_tolerance_level", &tmp2_obj)) {
				data->video_strm[cnt].motion_tolerance_level = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "motion_tolerance_level", data->video_strm[cnt].motion_tolerance_level);
			}

			if (json_object_object_get_ex(tmp1_obj, "motion_tolerance_qp", &tmp2_obj)) {
				data->video_strm[cnt].motion_tolerance_qp = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "motion_tolerance_qp", data->video_strm[cnt].motion_tolerance_qp);
			}

			if (json_object_object_get_ex(tmp1_obj, "motion_tolerance_qfactor", &tmp2_obj)) {
				data->video_strm[cnt].motion_tolerance_qfactor = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "motion_tolerance_qfactor",
				        data->video_strm[cnt].motion_tolerance_qfactor);
			}

			if (json_object_object_get_ex(tmp1_obj, "vbr_max_bit_rate", &tmp2_obj)) {
				data->video_strm[cnt].vbr_max_bit_rate = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "vbr_max_bit_rate", data->video_strm[cnt].vbr_max_bit_rate);
			}

			if (json_object_object_get_ex(tmp1_obj, "vbr_quality_level_index", &tmp2_obj)) {
				data->video_strm[cnt].vbr_quality_level_index = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "vbr_quality_level_index", data->video_strm[cnt].vbr_quality_level_index);
			}

			if (json_object_object_get_ex(tmp1_obj, "sbr_adjust_br_thres_pc", &tmp2_obj)) {
				data->video_strm[cnt].sbr_adjust_br_thres_pc = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "sbr_adjust_br_thres_pc", data->video_strm[cnt].sbr_adjust_br_thres_pc);
			}

			if (json_object_object_get_ex(tmp1_obj, "sbr_adjust_step_times", &tmp2_obj)) {
				data->video_strm[cnt].sbr_adjust_step_times = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "sbr_adjust_step_times", data->video_strm[cnt].sbr_adjust_step_times);
			}

			if (json_object_object_get_ex(tmp1_obj, "sbr_converge_frame", &tmp2_obj)) {
				data->video_strm[cnt].sbr_converge_frame = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "sbr_converge_frame", data->video_strm[cnt].sbr_converge_frame);
			}

			if (json_object_object_get_ex(tmp1_obj, "cqp_i_frame_qp", &tmp2_obj)) {
				data->video_strm[cnt].cqp_i_frame_qp = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "cqp_i_frame_qp", data->video_strm[cnt].cqp_i_frame_qp);
			}

			if (json_object_object_get_ex(tmp1_obj, "cqp_p_frame_qp", &tmp2_obj)) {
				data->video_strm[cnt].cqp_p_frame_qp = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "cqp_p_frame_qp", data->video_strm[cnt].cqp_p_frame_qp);
			}

			if (json_object_object_get_ex(tmp1_obj, "cqp_q_factor", &tmp2_obj)) {
				data->video_strm[cnt].cqp_q_factor = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "cqp_q_factor", data->video_strm[cnt].cqp_q_factor);
			}

			if (json_object_object_get_ex(tmp1_obj, "obs", &tmp2_obj)) {
				data->video_strm[cnt].obs = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "obs", data->video_strm[cnt].obs);
			}

			if (json_object_object_get_ex(tmp1_obj, "obs_off_period", &tmp2_obj)) {
				data->video_strm[cnt].obs_off_period = json_object_get_int(tmp2_obj);
				fprintf(stderr, "%s = %d\n", "obs_off_period", data->video_strm[cnt].obs_off_period);
			}
		}
	}
}

void comp_video_strm_conf(struct json_object *ret_obj, AGTX_STRM_CONF_S *data)
{
	int cnt;
	struct json_object *tmp_obj = NULL;
	struct json_object *tmp1_obj = NULL;
	struct json_object *tmp2_obj = NULL;

	tmp_obj = json_object_new_int(data->video_dev_idx);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_dev_idx", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_dev_idx");
	}

	tmp_obj = json_object_new_int(data->video_strm_cnt);
	if (tmp_obj) {
		json_object_object_add(ret_obj, "video_strm_cnt", tmp_obj);
	} else {
		printf("Cannot create %s object\n", "video_strm_cnt");
	}

	tmp_obj = json_object_new_array();
	for (cnt = 0; (AGTX_UINT32)cnt < data->video_strm_cnt; cnt++) {
		tmp1_obj = json_object_new_object();
		if (tmp1_obj) {
			tmp2_obj = json_object_new_int(data->video_strm[cnt].video_strm_idx);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "video_strm_idx", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "video_strm_idx");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].strm_en);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "strm_en", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "strm_en");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].width);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "width", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "width");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].height);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "height", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "height");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].output_fps);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "output_fps", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "output_fps");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].binding_capability);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "binding_capability", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "binding_capability");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].rotate);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "rotate", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "rotate");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].mirr_en);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "mirr_en", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "mirr_en");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].flip_en);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "flip_en", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "flip_en");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].venc_type);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "venc_type", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "venc_type");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].venc_profile);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "venc_profile", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "venc_profile");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].rc_mode);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "rc_mode", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "rc_mode");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].gop_size);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "gop_size", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "gop_size");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].max_frame_size);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "max_frame_size", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "max_frame_size");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].bit_rate);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "bit_rate", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "bit_rate");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].min_qp);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "min_qp", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "min_qp");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].max_qp);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "max_qp", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "max_qp");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].min_q_factor);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "min_q_factor", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "min_q_factor");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].max_q_factor);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "max_q_factor", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "max_q_factor");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].fluc_level);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "fluc_level", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "fluc_level");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].scene_smooth);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "scene_smooth", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "scene_smooth");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].regression_speed);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "regression_speed", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "regression_speed");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].i_continue_weight);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "i_continue_weight", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "i_continue_weight");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].i_qp_offset);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "i_qp_offset", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "i_qp_offset");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].motion_tolerance_level);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "motion_tolerance_level", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "motion_tolerance_level");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].motion_tolerance_qp);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "motion_tolerance_qp", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "motion_tolerance_qp");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].motion_tolerance_qfactor);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "motion_tolerance_qfactor", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "motion_tolerance_qfactor");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].vbr_max_bit_rate);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "vbr_max_bit_rate", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "vbr_max_bit_rate");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].vbr_quality_level_index);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "vbr_quality_level_index", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "vbr_quality_level_index");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].sbr_adjust_br_thres_pc);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "sbr_adjust_br_thres_pc", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "sbr_adjust_br_thres_pc");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].sbr_adjust_step_times);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "sbr_adjust_step_times", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "sbr_adjust_step_times");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].sbr_converge_frame);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "sbr_converge_frame", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "sbr_converge_frame");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].cqp_i_frame_qp);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "cqp_i_frame_qp", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "cqp_i_frame_qp");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].cqp_p_frame_qp);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "cqp_p_frame_qp", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "cqp_p_frame_qp");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].cqp_q_factor);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "cqp_q_factor", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "cqp_q_factor");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].obs);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "obs", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "obs");
			}

			tmp2_obj = json_object_new_int(data->video_strm[cnt].obs_off_period);
			if (tmp2_obj) {
				json_object_object_add(tmp1_obj, "obs_off_period", tmp2_obj);
			} else {
				printf("Cannot create %s object\n", "obs_off_period");
			}

			json_object_array_add(tmp_obj, tmp1_obj);
		} else {
			printf("Cannot create array object\n");
		}
	}

	json_object_object_add(ret_obj, "video_strm_list", tmp_obj);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
