#include "nodes.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "sample_dip.h"

#include "log_define.h"
#include "core.h"
#include "agtx_types.h"

extern GlobalConf g_conf;

static int setNrwin(AGTX_DIP_NR_WIN_CONF_S *nr_win)
{
	MPI_NR_ATTR_S nr_attr;
	MPI_WIN win_idx;
	int ret = MPI_SUCCESS;

	if ((!g_conf.nr_win.win_nr_en) == 1) {
		avmain2_log_info("NR win conf disabled");
		return 0;
	}

	avmain2_log_debug("Apply NR win conf");
	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		if ((!g_conf.strm.video_strm[i].strm_en) == 1) {
			avmain2_log_debug("skip chn [%d]", i);
			continue;
		}
		for (int j = 0; j < g_conf.layout.video_layout[i].window_num; j++) {
			win_idx = MPI_VIDEO_WIN(0, i, j);
			ret = MPI_getWinNrAttr(win_idx, &nr_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("failed to get nr v2 by win (0, %d, %d), ret: %d", i, j, ret);
				/*not return is only a workaround*/
			}

			if (nr_win->video_strm[i].window_array[j].mode == 0) {
				avmain2_log_info("NR win auto mode");
				nr_attr.mode = ALG_OPT_AUTO;
			} else if (nr_win->video_strm[i].window_array[j].mode == 1) {
				avmain2_log_info("NR win half auto mode");
				nr_attr.mode = ALG_OPT_HALF_AUTO;
			} else if (nr_win->video_strm[i].window_array[j].mode == 2) {
				avmain2_log_info("NR win manual mode");
				nr_attr.mode = ALG_OPT_MANUAL;
			}

			nr_attr.motion_comp = (UINT8)nr_win->video_strm[i].window_array[j].motion_comp;
			nr_attr.trail_suppress = (UINT8)nr_win->video_strm[i].window_array[j].trail_suppress;
			nr_attr.ghost_remove = (UINT8)nr_win->video_strm[i].window_array[j].ghost_remove;
			nr_attr.ma_y_strength = (UINT8)nr_win->video_strm[i].window_array[j].ma_y_strength;
			nr_attr.mc_y_strength = (UINT8)nr_win->video_strm[i].window_array[j].mc_y_strength;
			nr_attr.ma_c_strength = (UINT8)nr_win->video_strm[i].window_array[j].ma_c_strength;
			nr_attr.ratio_3d = (UINT8)nr_win->video_strm[i].window_array[j].ratio_3d;
			nr_attr.mc_y_level_offset = (INT16)nr_win->video_strm[i].window_array[j].mc_y_level_offset;
			nr_attr.me_frame_fallback_en =
			        (UINT8)nr_win->video_strm[i].window_array[j].me_frame_fallback_en;

			nr_attr.nr_manual.y_level_3d = (UINT8)nr_win->video_strm[i].window_array[j].manual_y_level_3d;
			nr_attr.nr_manual.c_level_3d = (UINT8)nr_win->video_strm[i].window_array[j].manual_c_level_3d;
			nr_attr.nr_manual.y_level_2d = (UINT8)nr_win->video_strm[i].window_array[j].manual_y_level_2d;
			nr_attr.nr_manual.c_level_2d = (UINT8)nr_win->video_strm[i].window_array[j].manual_c_level_2d;

			for (INT32 num = 0; num < MPI_ISO_LUT_ENTRY_NUM; num++) {
				nr_attr.nr_auto.y_level_3d[num] =
				        (UINT8)nr_win->video_strm[i].window_array[j].auto_y_level_3d_list[num];
				nr_attr.nr_auto.c_level_3d[num] =
				        (UINT8)nr_win->video_strm[i].window_array[j].auto_c_level_3d_list[num];
				nr_attr.nr_auto.y_level_2d[num] =
				        (UINT8)nr_win->video_strm[i].window_array[j].auto_y_level_2d_list[num];
				nr_attr.nr_auto.c_level_2d[num] =
				        (UINT8)nr_win->video_strm[i].window_array[j].auto_c_level_2d_list[num];
			}

			ret = MPI_setWinNrAttr(win_idx, &nr_attr);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("failed to set nr v2 by win (0, %d, %d), ret: %d", i, j, ret);
				/*not return is only a workaround*/
			}
		}
	}
	return 0;
}

static int setShpwin(AGTX_DIP_SHP_WIN_CONF_S *shp_win_conf)
{
	MPI_SHP_ATTR_V2_S shp_attr_v2;
	MPI_WIN win_idx;
	int ret = MPI_SUCCESS;

	if ((!g_conf.shp_win.win_shp_en) == 1) {
		avmain2_log_info("SHP win conf disabled");
		return 0;
	}

	avmain2_log_info("Apply SHP win conf");
	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		if ((!g_conf.strm.video_strm[i].strm_en) == 1) {
			avmain2_log_debug("skip chn [%d]", i);
			continue;
		}
		for (int j = 0; j < g_conf.layout.video_layout[i].window_num; j++) {
			win_idx = MPI_VIDEO_WIN(0, i, j);

			ret = MPI_getWinShpAttrV2(win_idx, &shp_attr_v2);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("failed to get shp v2 by win(0, %d, %d), ret: %d", i, j, ret);
				return -EINVAL;
			}

			/*AGTX_CMD_DIP_SHP_WIN strength is the same to MPI by window strength*/
			shp_attr_v2.strength = shp_win_conf->video_strm[i].window_array[j].strength;
			avmain2_log_info("win sharpness %d strength: %d",
			                 shp_win_conf->video_strm[i].window_array[j].strength, shp_attr_v2.strength);

			ret = MPI_setWinShpAttrV2(win_idx, &shp_attr_v2);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("set ImgPref(Sharpness) failed on (0, %d, %d), ret: %d", i, j, ret);
				return -EINVAL;
			}
		}
	}

	return 0;
}

static int setShpWinbyImgPref(AGTX_INT16 *sharpness)
{
	MPI_SHP_ATTR_V2_S shp_attr_v2;
	int ret = MPI_SUCCESS;

	if ((!g_conf.shp_win.win_shp_en) == 1) {
		avmain2_log_info("SHP win conf disabled");
		return 0;
	}

	/*after channel sharpness, set to all windows*/
	for (int i = 0; (unsigned)i < g_conf.strm.video_strm_cnt; i++) {
		for (int j = 0; j < g_conf.layout.video_layout[i].window_num; j++) {
			MPI_WIN win_idx = MPI_VIDEO_WIN(0, i, j);

			ret = MPI_getWinShpAttrV2(win_idx, &shp_attr_v2);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("failed to get shp v2 by win(0, %d, %d), ret: %d", i, j, ret);
				return -EINVAL;
			}

			UINT16 strength =
			        g_conf.shp_win.video_strm[i].window_array[j].strength * 2 * (*sharpness) / 100;
			avmain2_log_info("sharpness %d strength: %d", *sharpness, strength);
			shp_attr_v2.strength = strength;

			ret = MPI_setWinShpAttrV2(win_idx, &shp_attr_v2);
			if (ret != MPI_SUCCESS) {
				avmain2_log_err("set ImgPref(Sharpness) failed on (0, %d, %d), ret: %d", i, j, ret);
				return -EINVAL;
			}
		}
	}

	return 0;
}

int NODE_initWinImagePreference(void)
{
	return 0;
}
int NODE_exitWinImagePreference(void)
{
	return 0;
}
int NODE_startWinImagePreference(void)
{
	/* apply dip attr by window from window.ini */
	char ini_name[PATH_MAX];
	sprintf(ini_name, "%s/window.ini", DIP_FILE_PATH);
	for (int idx = 0; (unsigned)idx < g_conf.strm.video_strm_cnt; idx++) {
		if (g_conf.strm.video_strm[idx].strm_en == 1) {
			SAMPLE_updateChnDipAttr(MPI_VIDEO_CHN(0, idx), ini_name);
		}
	}

	/* apply dip attr by window from ini.db*/

	if (setShpwin(&g_conf.shp_win) != 0) {
		return -EINVAL;
	}

	if (setNrwin(&g_conf.nr_win) != 0) {
		return -EINVAL;
	}

	if (setShpWinbyImgPref(&g_conf.img.sharpness) != 0) {
		return -EINVAL;
	}

	return 0;
}
int NODE_stopWinImagePreference(void)
{
	return 0;
}
int NODE_setWinImagePreference(int cmd_id, void *data)
{
	switch (cmd_id) {
	case NR_WIN:
		avmain2_log_info("set NR win");
		AGTX_DIP_NR_WIN_CONF_S *p_nr_win = (AGTX_DIP_NR_WIN_CONF_S *)data;
		if (0 != setNrwin(p_nr_win)) {
			return -EINVAL;
		}
		break;
	case SHP_WIN:
		avmain2_log_info("set SHP win");
		AGTX_DIP_SHP_WIN_CONF_S *p_shp_win = (AGTX_DIP_SHP_WIN_CONF_S *)data;
		if (0 != setShpwin(p_shp_win)) {
			return -EINVAL;
		}
		break;
	case IMG_PREF_WIN:
		avmain2_log_info("image pref percentage change win SHP");
		AGTX_IMG_PREF_S *p_img_pref = (AGTX_IMG_PREF_S *)data;
		if (0 != setShpWinbyImgPref(&p_img_pref->sharpness)) {
		}
		break;
	default:
		avmain2_log_err("invalid cmd_id: %d", cmd_id);
		return -EINVAL;
	}

	return 0;
}