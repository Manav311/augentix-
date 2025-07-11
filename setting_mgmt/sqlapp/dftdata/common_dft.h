/******************************************************************************
*
* Copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef COMMON_DFT_H_
#define COMMON_DFT_H_


/*JSON string table*/
#define CREATE_JSON_TABLE "CREATE TABLE json_tbl(id integer primary key, jstr TEXT);"

#define INSERT_CMD_SYS_SYS_INFO "INSERT INTO json_tbl(id, jstr)" \
                                "VALUES(1048579, '"\
"{"\
	"\"dev_name\": \"MT801-1\""\
"}"\
"');"

#define INSERT_CMD_SYS_FEATURE_OPTION "INSERT INTO json_tbl(id, jstr)" \
                                      "VALUES(1048580, '"\
"{"\
	"\"stitch_support\": true"\
"}"\
"');"

#define INSERT_CMD_SYS_PRODUCT_OPTION_LIST "INSERT INTO json_tbl(id, jstr)" \
                                           "VALUES(1048581, '"\
"{"\
	"\"option\": ["\
		"\"AGTX_CMD_RES_OPTION\", \"AGTX_CMD_VENC_OPTION\", \"AGTX_CMD_SYS_FEATURE_OPTION\" "\
	"]"\
"}"\
"');"

#define INSERT_CMD_VIDEO_BUF_CONF "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145729, '{"\
	"\"max_pool_cnt\": 64,"\
	"\"pub_pool_cnt\": 1, "\
	"\"pub_pool_list\": ["\
		"{"\
			"\"block_size\": 327684,"\
			"\"block_cnt\": 3,"\
			"\"pool_name\": \"isp_TMV\","\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_VIDEO_DEV_CONF "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145730, '{"\
	"\"video_dev_idx\": 0,"\
	"\"hdr_mode\": 0,"\
	"\"stitch_en\": 1,"\
	"\"eis_en\": 0,"\
	"\"bayer\": 1,"\
	"\"input_fps\": 25,"\
	"\"input_path_cnt\": 2,"\
	"\"input_path_list\": ["\
		"{"\
			"\"path_idx\": 0,"\
			"\"path_en\": 1,"\
			"\"sensor_idx\": 0,"\
			"\"path_fps\": 0,"\
			"\"width\": 1920,"\
			"\"height\": 1080,"\
			"\"eis_strength\": 60,"\
		"},"\
		"{"\
			"\"path_idx\": 1,"\
			"\"path_en\": 1,"\
			"\"sensor_idx\": 1,"\
			"\"path_fps\": 0,"\
			"\"width\": 1920,"\
			"\"height\": 1080,"\
			"\"eis_strength\": 60,"\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_VIDEO_STRM_CONF "INSERT INTO json_tbl(id, jstr)" \
                                   "VALUES(3145731, '{"\
	"\"video_dev_idx\": 0,"\
	"\"video_strm_cnt\": 2,"\
	"\"video_strm_list\": ["\
		"{"\
			"\"video_strm_idx\": 0,"\
			"\"strm_en\": 1,"\
			"\"width\": 3840,"\
			"\"height\": 1080,"\
			"\"output_fps\": 15,"\
			"\"rotate\": 0,"\
			"\"mirr_en\": 0,"\
			"\"flip_en\": 0,"\
			"\"stitch_en\": 1,"\
			"\"venc_type\": 0,"\
			"\"venc_profile\": 2,"\
			"\"rc_mode\": 1,"\
			"\"gop_size\": 15,"\
			"\"max_frame_size\": 0,"\
			"\"bit_rate\": 4096,"\
			"\"min_qp\": 15,"\
			"\"max_qp\": 40,"\
			"\"min_q_factor\": 10,"\
			"\"max_q_factor\": 90,"\
			"\"fluc_level\": 0,"\
			"\"scene_smooth\": 0,"\
			"\"regression_speed\": 6,"\
			"\"i_continue_weight\": 0,"\
			"\"i_qp_offset\": -2,"\
			"\"motion_tolerance_level\": 16,"\
			"\"motion_tolerance_qp\": 31,"\
			"\"motion_tolerance_qfactor\": 38,"\
			"\"vbr_max_bit_rate\": 4096,"\
			"\"vbr_quality_level_index\": 7,"\
			"\"sbr_adjust_br_thres_pc\": 70,"\
			"\"sbr_adjust_step_times\": 20,"\
			"\"sbr_converge_frame\": 15,"\
			"\"cqp_i_frame_qp\": 20,"\
			"\"cqp_p_frame_qp\": 28,"\
			"\"cqp_q_factor\": 80,"\
			"\"obs\": 0,"\
			"\"obs_off_period\": 2,"\
		"},"\
		"{"\
			"\"video_strm_idx\": 1,"\
			"\"strm_en\": 1,"\
			"\"width\": 960,"\
			"\"height\": 272,"\
			"\"output_fps\": 10,"\
			"\"rotate\": 0,"\
			"\"mirr_en\": 0,"\
			"\"flip_en\": 0,"\
			"\"stitch_en\": 1,"\
			"\"venc_type\": 0,"\
			"\"venc_profile\": 2,"\
			"\"rc_mode\": 1,"\
			"\"gop_size\": 10,"\
			"\"max_frame_size\": 0,"\
			"\"bit_rate\": 800,"\
			"\"min_qp\": 10,"\
			"\"max_qp\": 40,"\
			"\"min_q_factor\": 10,"\
			"\"max_q_factor\": 90,"\
			"\"fluc_level\": 0,"\
			"\"scene_smooth\": 0,"\
			"\"regression_speed\": 6,"\
			"\"i_continue_weight\": 0,"\
			"\"i_qp_offset\": -2,"\
			"\"motion_tolerance_level\": 16,"\
			"\"motion_tolerance_qp\": 31,"\
			"\"motion_tolerance_qfactor\": 38,"\
			"\"vbr_max_bit_rate\": 4096,"\
			"\"vbr_quality_level_index\": 7,"\
			"\"sbr_adjust_br_thres_pc\": 70,"\
			"\"sbr_adjust_step_times\": 20,"\
			"\"sbr_converge_frame\": 10,"\
			"\"cqp_i_frame_qp\": 20,"\
			"\"cqp_p_frame_qp\": 28,"\
			"\"cqp_q_factor\": 80,"\
			"\"obs\": 0,"\
			"\"obs_off_period\": 2,"\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_VIDEO_STITCH_CONF "INSERT INTO json_tbl(id, jstr)" \
                                     "VALUES(3145732, '{"\
	"\"video_dev_idx\": 0,"\
	"\"center_0_x\": 3917,"\
	"\"center_0_y\": 2153,"\
	"\"center_1_x\": 3978,"\
	"\"center_1_y\": 2340,"\
	"\"dft_dist\": 0,"\
	"\"dist_tbl_cnt\": 3,"\
	"\"dist_tbl_list\": ["\
		"{"\
			"\"tbl_idx\": 0,"\
			"\"dist\": 100,"\
			"\"ver_disp\": 1450,"\
			"\"straighten\": 12728,"\
			"\"src_zoom\": 1638,"\
			"\"theta_0\": 92,"\
			"\"theta_1\": -20,"\
			"\"radius_0\": 3008,"\
			"\"radius_1\": 3017,"\
			"\"curvature_0\": 4619,"\
			"\"curvature_1\": 4644,"\
			"\"fov_ratio_0\": 4096,"\
			"\"fov_ratio_1\": 4096,"\
			"\"ver_scale_0\": 4506,"\
			"\"ver_scale_1\": 4506,"\
			"\"ver_shift_0\": 0,"\
			"\"ver_shift_1\": 7,"\
		"},"\
		"{"\
			"\"tbl_idx\": 1,"\
			"\"dist\": 300,"\
			"\"ver_disp\": 1450,"\
			"\"straighten\": 12728,"\
			"\"src_zoom\": 1638,"\
			"\"theta_0\": 88,"\
			"\"theta_1\": -16,"\
			"\"radius_0\": 2942,"\
			"\"radius_1\": 2940,"\
			"\"curvature_0\": 4606,"\
			"\"curvature_1\": 4637,"\
			"\"fov_ratio_0\": 4096,"\
			"\"fov_ratio_1\": 4096,"\
			"\"ver_scale_0\": 4506,"\
			"\"ver_scale_1\": 4534,"\
			"\"ver_shift_0\": 0,"\
			"\"ver_shift_1\": 8,"\
		"},"\
		"{"\
			"\"tbl_idx\": 2,"\
			"\"dist\": 500,"\
			"\"ver_disp\": 1450,"\
			"\"straighten\": 12728,"\
			"\"src_zoom\": 1638,"\
			"\"theta_0\": 88,"\
			"\"theta_1\": -16,"\
			"\"radius_0\": 2921,"\
			"\"radius_1\": 2929,"\
			"\"curvature_0\": 4597,"\
			"\"curvature_1\": 4636,"\
			"\"fov_ratio_0\": 4096,"\
			"\"fov_ratio_1\": 4096,"\
			"\"ver_scale_0\": 4506,"\
			"\"ver_scale_1\": 4542,"\
			"\"ver_shift_0\": 0,"\
			"\"ver_shift_1\": 9,"\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_VIDEO_AWB_PREF "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145733, '{"\
	"\"mode\":1,"\
	"\"color_temp\":5000,"\
	"\"r_gain\":50,"\
	"\"b_gain\":50,"\
"}');"

#define INSERT_CMD_VIDEO_IMG_PREF "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145734, '{"\
	"\"brightness\":50,"\
	"\"saturation\":50,"\
	"\"contrast\":50,"\
	"\"sharpness\":50,"\
	"\"anti_flicker\":0,"\
"}');"

#define INSERT_CMD_VIDEO_ADV_IMG_PREF "INSERT INTO json_tbl(id, jstr)" \
                                      "VALUES(3145735, '{"\
	"\"backlight_compensation\":0,"\
	"\"night_mode\":\"AUTO\","\
	"\"icr_mode\":\"AUTO\","\
	"\"image_mode\":\"AUTO\","\
	"\"wdr_en\":0,"\
	"\"wdr_strength\":128,"\
"}');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_CAL  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145736, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_DBC  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145737, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_DCC  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145738, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_LSC  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145739, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_CTRL "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145740, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_AE   "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145741, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_AWB  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145742, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_PTA  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145743, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_CSM  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145744, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_SHP  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145745, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_NR   "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145746, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_ROI  "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145747, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_TE   "INSERT INTO json_tbl(id, jstr)" \
                                  "VALUES(3145748, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_GAMMA "INSERT INTO json_tbl(id, jstr)" \
                                   "VALUES(3145749, '"\
"{"\
"}"\
"');"

/* deprecated */
#define INSERT_CMD_VIDEO_DIP_ISO "INSERT INTO json_tbl(id, jstr)" \
                                 "VALUES(3145750, '"\
"{"\
"}"\
"');"

#define INSERT_CMD_VIDEO_COLOR_CONF "INSERT INTO json_tbl(id, jstr)" \
                                    "VALUES(3145751, '"\
"{"\
	"\"color_mode\": \"DAY\","\
"}"\
"');"

#define INSERT_CMD_VIDEO_PRODUCT_OPTION "INSERT INTO json_tbl(id, jstr)" \
                                        "VALUES(3145752, '"\
"{"\
	"\"video_option\": ["\
		"{"\
			"\"res\": ["\
				"{"\
					"\"width\": 3840,"\
					"\"height\": 1080,"\
					"\"max_frame_rate\": ["\
						"25, 20, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"},"\
				"{"\
					"\"width\": 2560,"\
					"\"height\": 720,"\
					"\"max_frame_rate\": ["\
						"25, 25, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"},"\
				"{"\
					"\"width\": 1920,"\
					"\"height\": 544,"\
					"\"max_frame_rate\": ["\
						"25, 25, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"}"\
			"],"\
			"\"venc\": ["\
				"{"\
					"\"codec\": \"H264\","\
					"\"profile\": ["\
						"\"BASELINE\", \"MAIN\", \"HIGH\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"H265\","\
					"\"profile\": ["\
						"\"MAIN\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"MJPEG\","\
					"\"profile\": ["\
						"\"BASELINE\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"}"\
			"]"\
		"},"\
		"{"\
			"\"res\": ["\
				"{"\
					"\"width\": 960,"\
					"\"height\": 272,"\
					"\"max_frame_rate\": ["\
						"0, 10, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"},"\
				"{"\
					"\"width\": 640,"\
					"\"height\":184,"\
					"\"max_frame_rate\": ["\
						"0, 10, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"}"\
			"],"\
			"\"venc\": ["\
				"{"\
					"\"codec\": \"H264\","\
					"\"profile\": ["\
						"\"BASELINE\", \"MAIN\", \"HIGH\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"H265\","\
					"\"profile\": ["\
						"\"MAIN\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"MJPEG\","\
					"\"profile\": ["\
						"\"BASELINE\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"}"\
			"]"\
		"}"\
	"]"\
"}"\
"');"

#define INSERT_CMD_VIDEO_RES_OPTION "INSERT INTO json_tbl(id, jstr)" \
                                    "VALUES(3145753, '"\
"{"\
	"\"strm_idx\": -1,"\
	"\"strm\": ["\
		"{"\
			"\"res_idx\": -1,"\
			"\"res\": ["\
				"{"\
					"\"width\": 3840,"\
					"\"height\": 1080,"\
					"\"max_frame_rate\": ["\
						"25, 20, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"},"\
				"{"\
					"\"width\": 2560,"\
					"\"height\": 720,"\
					"\"max_frame_rate\": ["\
						"25, 25, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"},"\
				"{"\
					"\"width\": 1920,"\
					"\"height\": 544,"\
					"\"max_frame_rate\": ["\
						"25, 25, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"25, 24, 23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"}"\
			"]"\
		"},"\
		"{"\
			"\"res_idx\": -1,"\
			"\"res\": ["\
				"{"\
					"\"width\": 960,"\
					"\"height\": 272,"\
					"\"max_frame_rate\": ["\
						"0, 10, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"},"\
				"{"\
					"\"width\": 640,"\
					"\"height\":184,"\
					"\"max_frame_rate\": ["\
						"0, 10, 0, 0, 0, 0, 0, 0"\
					"],"\
					"\"frame_rate_list\": ["\
						"10, 9, 8, 7, 6, 5, 4, 3, 2, 1"\
					"]"\
				"}"\
			"]"\
		"}"\
	"]"\
"}"\
"');"

#define INSERT_CMD_VIDEO_VENC_OPTION "INSERT INTO json_tbl(id, jstr)" \
                                     "VALUES(3145754, '"\
"{"\
	"\"strm_idx\": -1,"\
	"\"strm\": ["\
		"{"\
			"\"venc_idx\": -1,"\
			"\"venc\": ["\
				"{"\
					"\"codec\": \"H264\","\
					"\"profile\": ["\
						"\"BASELINE\", \"MAIN\", \"HIGH\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"H265\","\
					"\"profile\": ["\
						"\"MAIN\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"MJPEG\","\
					"\"profile\": ["\
						"\"NONE\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"}"\
			"]"\
		"},"\
		"{"\
			"\"venc_idx\": -1,"\
			"\"venc\": ["\
				"{"\
					"\"codec\": \"H264\","\
					"\"profile\": ["\
						"\"BASELINE\", \"MAIN\", \"HIGH\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"H265\","\
					"\"profile\": ["\
						"\"MAIN\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"},"\
				"{"\
					"\"codec\": \"MJPEG\","\
					"\"profile\": ["\
						"\"NONE\""\
					"],"\
					"\"rc_mode\": ["\
						"\"CBR\", \"VBR\", \"CQP\""\
					"],"\
					"\"min_bit_rate\": 64,"\
					"\"max_bit_rate\": 4096,"\
					"\"cbr_param\": {"\
						"\"min_q_factor\": 1,"\
						"\"max_q_factor\": 100"\
					"},"\
					"\"vbr_param\": {"\
						"\"min_quality_range\": 0,"\
						"\"max_quality_range\": 0"\
					"},"\
					"\"cqp_param\": {"\
						"\"min_qp\": 1,"\
						"\"max_qp\": 51,"\
						"\"q_factor\": 90"\
					"},"\
					"\"min_gop_size\": 1,"\
					"\"max_gop_size\": 120"\
				"}"\
			"]"\
		"}"\
	"]"\
"}"\
"');"

#define INSERT_CMD_EVT_CONF "INSERT INTO json_tbl(id, jstr)" \
                            "VALUES(5242881, '{"\
	"\"event\": ["\
		"{"\
			"\"in_use\": true,"\
			"\"always_enabled\": false,"\
			"\"name\": \"LIGHT_SENSOR_IN\","\
			"\"source\": \"GPIO\","\
			"\"gpio\": {"\
				"\"polling_period_usec\": 3000000,"\
				"\"init_level\": ["\
					"{"\
						"\"action_args\": \"/system/bin/switch_day_night_mode -c2; sh /system/mpp/script/ir_cut.sh 53 54 remove\""\
					"},"\
					"{"\
						"\"action_args\": \"sh /system/mpp/script/ir_cut.sh 53 54 active;/system/bin/switch_day_night_mode -c1\""\
					"}"\
				"],"\
				"\"event\": ["\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"EDGE\","\
							"\"level_value\": -1,"\
							"\"level_time_sec\": 0,"\
							"\"edge\": -1,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"/system/bin/switch_day_night_mode -c2; sh /system/mpp/script/ir_cut.sh 53 54 remove\" "\
					"},"\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"EDGE\","\
							"\"level_value\": -1,"\
							"\"level_time_sec\": 0,"\
							"\"edge\": 1,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"sh /system/mpp/script/ir_cut.sh 53 54 active; /system/bin/switch_day_night_mode -c1\" "\
					"}"\
				"]"\
			"}"\
		"},"\
		"{"\
			"\"in_use\": true,"\
			"\"always_enabled\": false,"\
			"\"name\": \"PIR_IN\","\
			"\"source\": \"GPIO\","\
			"\"gpio\": {"\
				"\"polling_period_usec\": 1000000,"\
				"\"init_level\": ["\
					"{"\
						"\"action_args\": \"\","\
					"},"\
					"{"\
						"\"action_args\": \"\","\
					"}"\
				"],"\
				"\"event\": ["\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"EDGE\","\
							"\"level_value\": -1,"\
							"\"level_time_sec\": 0,"\
							"\"edge\": -1,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"/system/bin/alarmoutc 11 1 5\","\
					"},"\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"LEVEL\","\
							"\"level_value\": 0,"\
							"\"level_time_sec\": 4,"\
							"\"edge\": 0,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"/system/bin/alarmoutc 11 1 5\""\
					"}"\
				"]"\
			"}"\
		"},"\
		"{"\
			"\"in_use\": true,"\
			"\"always_enabled\": false,"\
			"\"name\": \"IVA_MD\","\
			"\"source\": \"SW\","\
			"\"sw\": {"\
				"\"socket_path\": \"IVA_MD\","\
				"\"event\": ["\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"IVA_MD_NEGATIVE\""\
						"},"\
						"\"action\": \"PRINT\","\
						"\"action_args\": \"IVA_MD_NEGATIVE\""\
					"},"\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"IVA_MD_POSITIVE\""\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"/system/bin/alarmoutc 11 1 10\""\
					"}"\
				"]"\
			"}"\
		"},"\
		"{"\
			"\"in_use\": true,"\
			"\"always_enabled\": false,"\
			"\"name\": \"IVA_TD\","\
			"\"source\": \"SW\","\
			"\"sw\": {"\
				"\"socket_path\": \"IVA_TD\","\
				"\"event\": ["\
					"{"\
						"\"rule\": { \"trigger_type\": \"IVA_TD_NEGATIVE\" },"\
						"\"action\": \"PRINT\","\
						"\"action_args\": \"IVA_TD_NEGATIVE\""\
					"},"\
					"{"\
						"\"rule\": { \"trigger_type\": \"IVA_TD_POSITIVE\" },"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"/system/bin/alarmoutc 11 1 15\""\
					"}"\
				"]"\
			"}"\
		"},"\
		"{"\
			"\"in_use\": true,"\
			"\"always_enabled\": true,"\
			"\"name\": \"PUSH_BUTTON_IN\","\
			"\"source\": \"GPIO\","\
			"\"gpio\": {"\
				"\"polling_period_usec\": 500000,"\
				"\"init_level\": ["\
					"{ \"action_args\": \"\", },"\
					"{ \"action_args\": \"\" }"\
				"],"\
				"\"event\": ["\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"LEVEL\","\
							"\"level_value\": 0,"\
							"\"level_time_sec\": 5,"\
							"\"edge\": 0,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"touch /usrdata/reset_file;reboot\","\
					"},"\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"EDGE\","\
							"\"level_value\": -1,"\
							"\"level_time_sec\": 0,"\
							"\"edge\": 1,"\
							"\"edge_time_sec\": -2,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"sysupd\""\
					"}"\
				"]"\
			"}"\
		"},"\
		"{"\
			"\"in_use\": true,"\
			"\"always_enabled\": false,"\
			"\"name\": \"SD_CARD_IN\","\
			"\"source\": \"GPIO\","\
			"\"gpio\": {"\
				"\"polling_period_usec\": 2000000,"\
				"\"init_level\": ["\
					"{ \"action_args\": \"devmem 0x800000E6 8 0x09\", },"\
					"{ \"action_args\": \"\" }"\
				"],"\
				"\"event\": ["\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"EDGE\","\
							"\"level_value\": -1,"\
							"\"level_time_sec\": 0,"\
							"\"edge\": -1,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"devmem 0x800000E6 8 0x09\" "\
					"},"\
					"{"\
						"\"rule\": {"\
							"\"trigger_type\": \"EDGE\","\
							"\"level_value\": -1,"\
							"\"level_time_sec\": 0,"\
							"\"edge\": 1,"\
							"\"edge_time_sec\": 0,"\
						"},"\
						"\"action\": \"EXEC_CMD\","\
						"\"action_args\": \"devmem 0x800000E6 8 0x05\" "\
					"}"\
				"]"\
			"}"\
		"}"\
	"]"\
"}');"

#define INSERT_CMD_EVT_GPIO_CONF "INSERT INTO json_tbl(id, jstr)" \
                                 "VALUES(5242882, '"\
"{"\
	"\"gpio_alias\": ["\
		"{"\
			"\"name\": \"PUSH_BUTTON_IN\","\
			"\"dir\": \"IN\","\
			"\"pin_num\": 56,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"LIGHT_SENSOR_IN\","\
			"\"dir\": \"IN\","\
			"\"pin_num\": 8,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"PIR_IN\","\
			"\"dir\": \"IN\","\
			"\"pin_num\": 57,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"SD_CARD_IN\","\
			"\"dir\": \"IN\","\
			"\"pin_num\": 6,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"IRCUT0_OUT\","\
			"\"dir\": \"OUT\","\
			"\"pin_num\": 53,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"IRCUT1_OUT\","\
			"\"dir\": \"OUT\","\
			"\"pin_num\": 54,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"ALARM_OUT\","\
			"\"dir\": \"OUT\","\
			"\"pin_num\": 11,"\
			"\"value\": 0,"\
		"},"\
		"{"\
			"\"name\": \"\","\
			"\"dir\": \"IN\","\
			"\"pin_num\": -1,"\
			"\"value\": -1,"\
		"},"\
	"],"\
"}"\
"');"

#define INSERT_CMD_EVT_PARAM "INSERT INTO json_tbl(id, jstr)" \
                             "VALUES(5242883, '"\
"{"\
    "\"event_attr\": ["\
        "{"\
            "\"name\": \"LIGHT_SENSOR_IN\","\
            "\"enabled\": true,"\
        "},"\
        "{"\
            "\"name\": \"PIR_IN\","\
            "\"enabled\": true,"\
        "},"\
        "{"\
            "\"name\": \"IVA_MD\","\
            "\"enabled\": true,"\
        "},"\
        "{"\
            "\"name\": \"IVA_TD\","\
            "\"enabled\": true,"\
        "},"\
        "{"\
            "\"name\": \"PUSH_BUTTON_IN\","\
            "\"enabled\": true,"\
        "},"\
        "{"\
            "\"name\": \"SD_CARD_IN\","\
            "\"enabled\": true,"\
        "},"\
    "],"\
"}"\
"');"

#define INSERT_CMD_OSD_CONF "INSERT INTO json_tbl(id, jstr)" \
                            "VALUES(6291457, '{"\
	"\"strm\": ["\
		"{"\
			"\"region\": ["\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"TEXT\","\
					"\"type_spec\": \"Augentix\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"INFO\","\
					"\"type_spec\": \"YYYY-MM-DD\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"IMAGE\","\
					"\"type_spec\": \"/system/mpp/font/logo_augentix_size0.ayuv\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"IMAGE\","\
					"\"type_spec\": \"icon:bell\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
			"],"\
		"},"\
		"{"\
			"\"region\": ["\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"TEXT\","\
					"\"type_spec\": \"Augentix\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"INFO\","\
					"\"type_spec\": \"YYYY-MM-DD\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"IMAGE\","\
					"\"type_spec\": \"/system/mpp/font/logo_augentix_size1.ayuv\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
				"{"\
					"\"enabled\": true,"\
					"\"type\": \"IMAGE\","\
					"\"type_spec\": \"icon:bell\","\
					"\"start_x\": -1,"\
					"\"start_y\": -1,"\
				"},"\
			"],"\
		"},"\
		"{"\
			"\"region\": ["\
				"{"\
					"\"enabled\": 0,"\
				"},"\
				"{"\
					"\"enabled\": 0,"\
				"},"\
				"{"\
					"\"enabled\": 0,"\
				"},"\
				"{"\
					"\"enabled\": 0,"\
				"},"\
			"],"\
		"},"\
		"{"\
			"\"region\": ["\
				"{"\
					"\"enabled\": 0,"\
				"},"\
				"{"\
					"\"enabled\": 0,"\
				"},"\
				"{"\
					"\"enabled\": 0,"\
				"},"\
				"{"\
					"\"enabled\": 0,"\
				"},"\
			"],"\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_OSD_PM_CONF "INSERT INTO json_tbl(id, jstr)" \
                            "VALUES(6291458, '{"\
	"\"conf\": ["\
		"{"\
			"\"param\": ["\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
			"],"\
		"},"\
		"{"\
			"\"param\": ["\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
			"],"\
		"},"\
		"{"\
			"\"param\": ["\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
			"],"\
		"},"\
		"{"\
			"\"param\": ["\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
				"{"\
					"\"enabled\": false,"\
					"\"alpha\": 0,"\
					"\"color\": 0,"\
					"\"start_x\": 0,"\
					"\"start_y\": 0,"\
					"\"end_x\": 0,"\
					"\"end_y\": 0,"\
				"},"\
			"],"\
		"},"\
	"],"\
"}');"
#define INSERT_CMD_IVA_TD_CONF           \
	"INSERT INTO json_tbl(id, jstr)" \
	"VALUES(7340033, '{"             \
	"\"enabled\":0,"                 \
	"\"video_chn_idx\":0,"           \
	"\"en_block_det\":1,"            \
	"\"en_redirect_det\":0,"         \
	"\"sensitivity\":50,"            \
	"\"endurance\":48,"              \
	"\"redirect_sensitivity\":50,"   \
	"\"redirect_global_change\":20," \
	"\"redirect_trigger_delay\":5,"  \
	"\"register_scene\":0,"          \
	"}');"

#define INSERT_CMD_IVA_MD_CONF "INSERT INTO json_tbl(id, jstr)" \
                               "VALUES(7340034, '{"\
	"\"enabled\":0,"\
	"\"video_chn_idx\":0,"\
	"\"active_cell\":\"1wA=\","\
	"\"mode\":\"ENERGY\","\
	"\"min_spd\":5,"\
	"\"max_spd\":255,"\
	"\"sens\":100,"\
	"\"rgn_cnt\":1,"\
	"\"rgn_list\": ["\
		"{"\
			"\"id\":0,"\
			"\"sx\":0,"\
			"\"sy\":0,"\
			"\"ex\":100,"\
			"\"ey\":100,"\
			"\"mode\":\"ENERGY\","\
			"\"min_spd\":5,"\
			"\"max_spd\":255,"\
			"\"sens\":100,"\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_IVA_AROI_CONF "INSERT INTO json_tbl(id, jstr)" \
                                 "VALUES(7340035, '{"\
	"\"enabled\":0,"\
	"\"video_chn_idx\":0,"\
	"\"aspect_ratio_width\":0,"\
	"\"aspect_ratio_height\":0,"\
	"\"min_roi_width\":50,"\
	"\"min_roi_height\":100,"\
	"\"max_roi_width\":100,"\
	"\"max_roi_height\":100,"\
	"\"track_speed\":32,"\
	"\"return_speed\":50,"\
"}');"

#define INSERT_CMD_IVA_PD_CONF "INSERT INTO json_tbl(id, jstr)" \
                                 "VALUES(7340036, '{"\
	"\"enabled\":0,"\
	"\"video_chn_idx\":0,"\
	"\"max_aspect_ratio_w\":1,"\
	"\"max_aspect_ratio_h\":1,"\
	"\"min_aspect_ratio_w\":1,"\
	"\"min_aspect_ratio_h\":5,"\
	"\"min_size\":0,"\
	"\"max_size\":100,"\
"}');"

#define INSERT_CMD_IVA_OD_CONF           \
	"INSERT INTO json_tbl(id, jstr)" \
	"VALUES(7340037, '{"             \
	"\"enabled\":0,"                 \
	"\"en_crop_outside_obj\":1,"     \
	"\"en_shake_det\":1,"            \
	"\"video_chn_idx\":0,"           \
	"\"od_qual\":85,"                \
	"\"od_track_refine\":86,"        \
	"\"od_size_th\":6,"              \
	"\"od_sen\":99,"                 \
	"\"en_stop_det\":1,"             \
	"\"en_gmv_det\":0,"              \
	"\"en_motor\":0,"                \
	"}');"

#define INSERT_CMD_IVA_RMS_CONF "INSERT INTO json_tbl(id, jstr)" \
                                "VALUES(7340038, '{"\
	"\"enabled\":0,"\
	"\"video_chn_idx\":0,"\
	"\"sensitivity\":100,"\
	"\"split_x\":1,"\
	"\"split_y\":1,"\
"}');"

#define INSERT_CMD_IVA_LD_CONF "INSERT INTO json_tbl(id, jstr)" \
                                 "VALUES(7340039, '{"\
	"\"enabled\":0,"\
	"\"video_chn_idx\":0,"\
	"\"sensitivity\":80,"\
	"\"trigger_cond\":\"BOTH\","\
	"\"det_region\": {"\
		"\"start_x\":0,"\
		"\"start_y\":0,"\
		"\"end_x\":100,"\
		"\"end_y\":100,"\
	"},"\
"}');"

#define INSERT_CMD_IVA_EF_CONF "INSERT INTO json_tbl(id, jstr)" \
                               "VALUES(7340040, '{"\
	"\"enabled\":0,"\
	"\"video_chn_idx\":0,"\
	"\"active_cell\":\"/gA=\","\
	"\"line_cnt\":1,"\
	"\"line_list\": ["\
		"{"\
			"\"id\":0,"\
			"\"start_x\":50,"\
			"\"start_y\":0,"\
			"\"end_x\":50,"\
			"\"end_y\":100,"\
			"\"obj_size_th\":0,"\
			"\"obj_v_th\":0,"\
			"\"mode\":\"DIR_BOTH\","\
		"},"\
	"],"\
"}');"

#define INSERT_CMD_IVA_VDBG_CONF "INSERT INTO json_tbl(id, jstr)" \
                               "VALUES(7340041, '{"\
	"\"enabled\":0,"\
	"\"ctx\":0"\
"}');"
#endif /* !COMMON_DFT_H_ */

