#include "handlers.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "agtx_video.h"
#include "cm_video_strm_conf.h"

#include "agtx_cmd.h"

#include "log_define.h"
#include "core.h"

extern GlobalConf g_conf;
extern AgtxConf g_old_conf_tmp;

int READ_DB(VideoStrm)(const char *path)
{
	struct json_object *ret_obj = NULL;
	ret_obj = get_db_record_obj(path, AGTX_CMD_VIDEO_STRM_CONF);
	parse_video_strm_conf(&g_conf.strm, ret_obj);

	avmain2_log_info("strm cnt:%d. dev_idx:%d", g_conf.strm.video_strm_cnt, g_conf.strm.video_dev_idx);

	json_object_put(ret_obj);

	return 0;
}

int WRITE_DB(VideoStrm)(const char *path)
{
	AGTX_UNUSED(path);

	return 0;
}

static bool isChnResolutionBiggerThenProductOption(AGTX_INT32 width, AGTX_INT32 height)
{
	if (width > g_conf.res_option.strm[0].res[0].width || height > g_conf.res_option.strm[0].res[0].height) {
		avmain2_log_err("(%d, %d) >  option max res (%d, %d)", width, height,
		                g_conf.res_option.strm[0].res[0].width, g_conf.res_option.strm[0].res[0].height);
		return true;
	}
	return false;
}

int APPLY(VideoStrm)(AGTX_CDATA data /*agtx input*/, int len /*len*/, void *node /*node should be root*/)
{
	int ret = 0;
	AGTX_STRM_CONF_S *strm = (AGTX_STRM_CONF_S *)data;
	if (len != sizeof(AGTX_STRM_CONF_S)) {
		avmain2_log_err("invalid size %d, should be:%d", len, sizeof(AGTX_STRM_CONF_S));
		return -EINVAL;
	}
	if (strm->video_strm_cnt > AGTX_MAX_VIDEO_STRM_NUM) {
		avmain2_log_err("strm->video_strm_cnt > AGTX_MAX_VIDEO_STRM_NUM:%d", strm->video_strm_cnt);
		return -EINVAL;
	}

	avmain2_log_info("copy strm to g_conf, and save old to tmp");
	saveOldConftoTmp(&g_conf.strm, strm, sizeof(AGTX_STRM_CONF_S));
	AGTX_STRM_CONF_S *strm_old = (AGTX_STRM_CONF_S *)&g_old_conf_tmp;

	NodeId id = NONE;
	NodeAct act = UNDO;
	int ptr = 0;

	/*compare for each video strm CHN restart case*/
	for (int i = 0; (unsigned)i < strm->video_strm_cnt; i++) {
		if (isChnResolutionBiggerThenProductOption(strm->video_strm[i].width, strm->video_strm[i].height)) {
			recoverOldConfFromTmp(&g_conf.strm, sizeof(AGTX_STRM_CONF_S));
			return -EINVAL;
		}

		if (strm_old->video_strm_cnt != strm->video_strm_cnt) {
			id = CHN;
			act = RESTART;
			break;
		}

		if (strm_old->video_strm[i].strm_en != strm->video_strm[i].strm_en) {
			avmain2_log_debug("Strm[%d] en %d != %d", i, strm_old->video_strm[i].strm_en,
			                  strm->video_strm[i].strm_en);
			id = CHN;
			act = RESTART;
			break;
		}

		if (strm_old->video_strm[i].width != strm->video_strm[i].width ||
		    strm_old->video_strm[i].height != strm->video_strm[i].height) {
			avmain2_log_debug("get chn[%d] w, h(%d, %d)\tgconf (%d, %d)", i, strm->video_strm[i].width,
			                  strm->video_strm[i].height, g_conf.strm.video_strm[i].width,
			                  g_conf.strm.video_strm[i].height);
			id = CHN;
			act = RESTART;
			break;
		}
		if (strm_old->video_strm[i].rotate != strm->video_strm[i].rotate ||
		    strm_old->video_strm[i].mirr_en != strm->video_strm[i].mirr_en ||
		    strm_old->video_strm[i].flip_en != strm->video_strm[i].flip_en) {
			/*chn restart*/
			id = CHN;
			act = RESTART;
			break;
		}
	}

	if (id != NONE && act == RESTART) {
		avmain2_log_debug("node_%d CHN restart case", id);
		ptr = ((int)node) + (id * sizeof(Node));
		ret = NODES_execRestart((Node *)ptr);
		if (ret != 0) {
			avmain2_log_info("copy strm to g_conf, and save old to tmp");
			recoverOldConfFromTmp(&g_conf.strm, sizeof(AGTX_STRM_CONF_S));
			return ret;
		}
		recoverTmptoZero();
		return 0;
	}

	/*compare every video strm CHN SET case*/
	for (int i = 0; (unsigned)i < strm->video_strm_cnt; i++) {
		if (strm_old->video_strm[i].output_fps != strm->video_strm[i].output_fps) {
			/*both chn and enc set case ? ENC frm_rate_o is deprecated*/
			ptr = ((int)node) + (CHN * sizeof(Node));
			ret = NODES_execSet(((Node *)ptr), FPS, strm);
			if (ret != 0) {
				recoverOldConfFromTmp(&g_conf.strm, sizeof(AGTX_STRM_CONF_S));
				return ret;
			}
		}
	}

	/*compare every video strm ENC restart case*/
	for (int i = 0; (unsigned)i < strm->video_strm_cnt; i++) {
		if (strm_old->video_strm[i].venc_type != strm->video_strm[i].venc_type ||
		    strm_old->video_strm[i].rc_mode != strm->video_strm[i].rc_mode ||
		    strm_old->video_strm[i].venc_profile != strm->video_strm[i].venc_profile) {
			/*enc restart*/
			id = ENC; /*enc restart*/
			act = RESTART;
			break;
		}
	}

	if (id != NONE && act == RESTART) {
		avmain2_log_debug("node_%d ENC restart case", id);
		ptr = ((int)node) + (id * sizeof(Node));
		ret = NODES_execRestart((Node *)ptr);
		if (ret != 0) {
			avmain2_log_info("copy strm to g_conf, and save old to tmp");
			recoverOldConfFromTmp(&g_conf.strm, sizeof(AGTX_STRM_CONF_S));
			return ret;
		}

		recoverTmptoZero();
		return 0;
	}

	/*compare for each video strm ENC SET case*/
	for (int i = 0; (unsigned)i < strm->video_strm_cnt; i++) {
		/*it is allow to set in chn 0 but restart when analyze chn 1*/
		if (strm_old->video_strm[i].gop_size != strm->video_strm[i].gop_size ||
		    strm_old->video_strm[i].bit_rate != strm->video_strm[i].bit_rate ||
		    strm_old->video_strm[i].min_qp != strm->video_strm[i].min_qp ||
		    strm_old->video_strm[i].max_qp != strm->video_strm[i].max_qp ||
		    strm_old->video_strm[i].min_q_factor != strm->video_strm[i].min_q_factor ||
		    strm_old->video_strm[i].max_q_factor != strm->video_strm[i].max_q_factor ||
		    strm_old->video_strm[i].fluc_level != strm->video_strm[i].fluc_level ||
		    strm_old->video_strm[i].regression_speed != strm->video_strm[i].regression_speed ||
		    strm_old->video_strm[i].scene_smooth != strm->video_strm[i].scene_smooth ||
		    strm_old->video_strm[i].i_continue_weight != strm->video_strm[i].i_continue_weight ||
		    strm_old->video_strm[i].i_qp_offset != strm->video_strm[i].i_qp_offset ||
		    strm_old->video_strm[i].vbr_max_bit_rate != strm->video_strm[i].vbr_max_bit_rate ||
		    strm_old->video_strm[i].vbr_quality_level_index != strm->video_strm[i].vbr_quality_level_index ||
		    strm_old->video_strm[i].sbr_adjust_br_thres_pc != strm->video_strm[i].sbr_adjust_br_thres_pc ||
		    strm_old->video_strm[i].sbr_adjust_step_times != strm->video_strm[i].sbr_adjust_step_times ||
		    strm_old->video_strm[i].sbr_converge_frame != strm->video_strm[i].sbr_converge_frame ||
		    strm_old->video_strm[i].cqp_i_frame_qp != strm->video_strm[i].cqp_i_frame_qp ||
		    strm_old->video_strm[i].cqp_p_frame_qp != strm->video_strm[i].cqp_p_frame_qp ||
		    strm_old->video_strm[i].cqp_q_factor != strm->video_strm[i].cqp_q_factor ||
		    strm_old->video_strm[i].obs != strm->video_strm[i].obs) {
			ptr = ((int)node) + (ENC * sizeof(Node));
			ret = NODES_execSet(((Node *)ptr), STRM_CONF, strm);
			if (ret != 0) {
				recoverOldConfFromTmp(&g_conf.strm, sizeof(AGTX_STRM_CONF_S));
				return ret;
			}
		}
	}

	recoverTmptoZero();
	return 0;
}

static JsonConfHandler video_strm_ops = MAKE_JSON_CONF_HANDLER(AGTX_CMD_VIDEO_STRM_CONF, VideoStrm);

__attribute__((constructor)) void registerVideoStrm(void)
{
	HANDLERS_registerHandlers(&video_strm_ops);
}
