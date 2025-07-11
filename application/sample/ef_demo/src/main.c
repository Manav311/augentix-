#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <json.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "mpi_sys.h"
#include "vftr_shd.h"
#include "vftr_ef.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_EF_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include "ef_demo.h"
#include "log.h"

VFTR_EF_PARAM_S g_ef_param;
MPI_WIN g_win_idx;

VFTR_SHD_PARAM_S g_shd_param;
VFTR_SHD_LONGTERM_LIST_S g_shd_long_list;

int g_ef_running; /* stop and start ef parameter */

#define INPUT_GRID_WIDTH 8
#define floorN(x, base) ((int)((x) / (base)) * (base))
#define roundN(x, base) floorN(x + ((base) >> 1), (base))

#ifdef CONFIG_APP_EF_SUPPORT_SEI
extern AVFTR_CTX_S *avftr_res_shm;
#endif

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

static void handleSigInt(int signo)
{
	DBG("[%s]leave %s\n", __func__, __FILE__);

	g_ef_running = 0;
	if (signo == SIGINT) {
		printf("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		printf("Caught SIGTERM!\n");
	} else if (signo == SIGKILL) {
		printf("Caught SIGKILL\n");
	} else if (signo == SIGQUIT) {
		printf("Caught SIGQUIT!\n");
	} else {
		perror("Unexpected signal!\n");
	}
}

int readJsonFromFile(char *file_name, VFTR_EF_PARAM_S *ef_para)
{
	int ret = 0;
	json_object *test_obj = NULL;
	json_object *tmp_obj = NULL;
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;
	json_object *tmp3_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		log_err("Cannot open %s.", file_name);
		return -EBADF;
	}

	// line-list is required parameter.
	json_object_object_get_ex(test_obj, "line_list", &tmp_obj);
	if (!tmp_obj) {
		log_err("Not found required parameter 'line_list' in config.");
		goto error;
	}

	int line_cnt = json_object_array_length(tmp_obj);
	ef_para->fence_num = line_cnt;
	DBG("line_cnt:%d\n", ef_para->fence_num);

	for (int i = 0; i < line_cnt; i++) {
		VFTR_EF_VL_ATTR_S *ef_attr = &g_ef_param.attr[i];
		tmp1_obj = json_object_array_get_idx(tmp_obj, i);
		if (!tmp1_obj) {
			ret = -ENOCSI;
			break;
		}

		json_object_object_get_ex(tmp1_obj, "start", &tmp2_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
		ef_attr->line.sx = json_object_get_int(tmp3_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
		ef_attr->line.sy = json_object_get_int(tmp3_obj);

		json_object_object_get_ex(tmp1_obj, "end", &tmp2_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
		ef_attr->line.ex = json_object_get_int(tmp3_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
		ef_attr->line.ey = json_object_get_int(tmp3_obj);

		json_object_object_get_ex(tmp1_obj, "obj_min_w", &tmp2_obj);
		ef_attr->obj_size_min.width = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_min_w");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj_min_h", &tmp2_obj);
		ef_attr->obj_size_min.height = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_min_h");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj_max_w", &tmp2_obj);
		ef_attr->obj_size_max.width = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_max_w");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj_max_h", &tmp2_obj);
		ef_attr->obj_size_max.height = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_max_h");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj_area", &tmp2_obj);
		ef_attr->obj_area = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_area");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj_v_th", &tmp2_obj);
		ef_attr->obj_v_th = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_v_th");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "mode", &tmp2_obj);
		ef_attr->mode = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "mode");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "obj_life_th", &tmp2_obj);
		ef_attr->obj_life_th = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "obj_life_th");
			ret = -ENOCSI;
			goto error;
		}

		json_object_object_get_ex(tmp1_obj, "id", &tmp2_obj);
		ef_attr->id = json_object_get_int(tmp2_obj);
		if (!tmp2_obj) {
			fprintf(stderr, "Cannot get %s object\n", "id");
			ret = -ENOCSI;
			goto error;
		}
	}

	/* parser shd*/
	VFTR_SHD_PARAM_S *shd_param = &g_shd_param;
	VFTR_SHD_LONGTERM_LIST_S *shd_long_list = &g_shd_long_list;

	json_object_object_get_ex(test_obj, "shd", &tmp1_obj);
	if (!tmp_obj) {
		fprintf(stderr, "Cannot get %s object\n", "shd");
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "en", &tmp2_obj);
	shd_param->en = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "en");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "sensitivity", &tmp2_obj);
	shd_param->sensitivity = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "sensitivity");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "quality", &tmp2_obj);
	shd_param->quality = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "quality");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "obj_life_th", &tmp2_obj);
	shd_param->obj_life_th = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "obj_life_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "longterm_life_th", &tmp2_obj);
	shd_param->longterm_life_th = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "longterm_life_th");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "instance_duration", &tmp2_obj);
	shd_param->instance_duration = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "instance_duration");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "shaking_update_duration", &tmp2_obj);
	shd_param->shaking_update_duration = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "shaking_update_duration");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "longterm_dec_period", &tmp2_obj);
	shd_param->longterm_dec_period = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "longterm_dec_period");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "longterm_num", &tmp2_obj);
	shd_long_list->num = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "longterm_num");
		ret = -ENOCSI;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "longterm_list", &tmp2_obj);
	if (!tmp2_obj) {
		fprintf(stderr, "Cannot get %s object\n", "longterm_list");
		goto error;
	}

	int longterm_list = json_object_array_length(tmp2_obj);
	for (int i = 0; i < longterm_list; i++) {
		tmp1_obj = json_object_array_get_idx(tmp2_obj, i);
		if (!tmp1_obj) {
			ret = -ENOCSI;
			break;
		}
		json_object_object_get_ex(tmp1_obj, "start", &tmp2_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
		shd_long_list->item->rgn.sx = json_object_get_int(tmp3_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
		shd_long_list->item->rgn.sy = json_object_get_int(tmp3_obj);

		json_object_object_get_ex(tmp1_obj, "end", &tmp2_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
		shd_long_list->item->rgn.ex = json_object_get_int(tmp3_obj);
		tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
		shd_long_list->item->rgn.ey = json_object_get_int(tmp3_obj);
	}

error:
	json_object_put(test_obj);

	return ret;
}

int transferEfline(const MPI_RECT_S *pWinRect, VFTR_EF_PARAM_S *ef_param)
{
	int i;

	for (i = 0; i < ef_param->fence_num; i++) {
		ef_param->attr[i].line.sx = MIN(
		        MAX(roundN((pWinRect->width - 1) * ef_param->attr[i].line.sx / 100, INPUT_GRID_WIDTH) - 1, 0),
		        pWinRect->width);
		ef_param->attr[i].line.sy = MIN(
		        MAX(roundN((pWinRect->height - 1) * ef_param->attr[i].line.sy / 100, INPUT_GRID_WIDTH) - 1, 0),
		        pWinRect->height);
		ef_param->attr[i].line.ex = MIN(
		        MAX(roundN((pWinRect->width - 1) * ef_param->attr[i].line.ex / 100, INPUT_GRID_WIDTH) - 1, 0),
		        pWinRect->width);
		ef_param->attr[i].line.ey = MIN(
		        MAX(roundN((pWinRect->height - 1) * ef_param->attr[i].line.ey / 100, INPUT_GRID_WIDTH) - 1, 0),
		        pWinRect->height);
		ef_param->attr[i].obj_size_min.width =
		        roundN(pWinRect->width * ef_param->attr[i].obj_size_min.width / 100, INPUT_GRID_WIDTH);
		ef_param->attr[i].obj_size_min.height =
		        roundN(pWinRect->height * ef_param->attr[i].obj_size_min.height / 100, INPUT_GRID_WIDTH);
		ef_param->attr[i].obj_size_max.width =
		        roundN(pWinRect->width * ef_param->attr[i].obj_size_max.width / 100, INPUT_GRID_WIDTH);
		ef_param->attr[i].obj_size_max.height =
		        roundN(pWinRect->height * ef_param->attr[i].obj_size_max.height / 100, INPUT_GRID_WIDTH);
		ef_param->attr[i].obj_area =
		        roundN(ef_param->attr[i].obj_area * pWinRect->width * pWinRect->height / 100, INPUT_GRID_WIDTH);
		ef_param->attr[i].obj_v_th =
		        roundN(ef_param->attr[i].obj_v_th * VFTR_EF_MAX_THR_V_OBJ / 100, INPUT_GRID_WIDTH);
		ef_param->attr[i].obj_life_th = ef_param->attr[i].obj_life_th;
	}
	return 0;
}

int enableOd(MPI_WIN win, const MPI_IVA_OD_PARAM_S *od)
{
	int ret;

	if ((ret = MPI_IVA_setObjParam(win, od)) != MPI_SUCCESS) {
		log_err("Failed to set OD param. err: %d", ret);
		goto error;
	}

	if ((ret = MPI_IVA_enableObjDet(win)) != MPI_SUCCESS) {
		log_err("Failed to enable object detection. err: %d", ret);
		goto error;
	}

	return MPI_SUCCESS;

error:
	return ret;
}
/* OD parameters share to all region */

void help(const char *name)
{
	printf("Usage: %s -i [options] ...\n"
	       "Options:\n"
	       "  -i <file>            EF parameter in .json format.\n"
	       "  -c <channel>         Specify which video channel to use. (Default 0).\n"
	       "  -w <window>          Specify which video window to user (Default 0).\n"
	       "  -d <detect interval> Specify which video detect interval. (Default 30).\n"
	       "  -q <quality>         Specify OD quality index.[0-63] (Default 46).\n"
	       "  -s <sensitivity>     Specify OD sensitivity.[0-255] (Default 254).\n"
	       "\n"
	       "Example:\n"
	       "  $ mpi_stream -d /system/mpp/case_config/case_config_1001_FHD &\n"
	       "  $ %s -i /system/mpp/ef_config/ef_conf_1080p_line_list_1.json &\n"
#ifdef CONFIG_APP_EF_SUPPORT_SEI
	       "  $ testOnDemandRTSPServer 0 -S\n",
#else
	       "  $ testOnDemandRTSPServer 0 -n\n",
#endif
	       name, name);
}

int main(int argc, char **argv)
{
	int ret;
	int c;
	int chn_idx = 0;
	int win_idx = 0;
	char cfg_file_name[256] = { 0 };

	MPI_IVA_OD_PARAM_S od_param = {
		.od_qual = 46, .od_track_refine = 42, .od_size_th = 40, .od_sen = 254, .en_stop_det = 0, .en_gmv_det = 0
	};

	VFTR_EF_PARAM_S *ef_attr = &g_ef_param;
	VFTR_SHD_PARAM_S *shd_attr = &g_shd_param;
	VFTR_SHD_LONGTERM_LIST_S *shd_long_list = &g_shd_long_list;

	while ((c = getopt(argc, argv, "i:c:w:r:q:s:t:h")) != -1) {
		switch (c) {
		case 'i':
			sprintf(cfg_file_name, optarg);
			DBG("input .json file:%s\n", cfg_file_name);
			break;
		case 'c':
			chn_idx = atoi(optarg);
			DBG("set device channel:%d\n", chn_idx);
			break;
		case 'w':
			win_idx = atoi(optarg);
			DBG("set device window:%d\n", win_idx);
			break;
		case 'r':
			od_param.od_track_refine = atoi(optarg);
			DBG("set detect refine:%d\n", optarg);
			break;
		case 'q':
			od_param.od_qual = atoi(optarg);
			DBG("set detect quality index:%s\n", optarg);
			break;
		case 't':
			od_param.od_size_th = atoi(optarg);
			DBG("set detect size threshold:%s\n", optarg);
			break;
		case 's':
			od_param.od_sen = atoi(optarg);
			DBG("set detect sensitivity:%s\n", optarg);
			break;
		case 'h':
			help(argv[0]);
			exit(0);
		default:
			abort();
		}
	}

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (strlen(cfg_file_name) == 0) {
		log_err("Config file path is not specified!");
		exit(1);
	}

	readJsonFromFile(cfg_file_name, ef_attr);
	DBG("Read Json form file:%s\n", cfg_file_name);

	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(0, chn_idx);

	/* Initialize MPI system */
	MPI_SYS_init();

	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel attribute. err: %d", ret);
		return -EINVAL;
	}

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video layout attribute. err: %d", ret);
		return -EINVAL;
	}

	/* NOTICE: we only support the case that only one window in a channel */
	assert(layout_attr.window_num == 1);
	assert(layout_attr.window[0].x == 0);
	assert(layout_attr.window[0].y == 0);
	assert(layout_attr.window[0].width == chn_attr.res.width);
	assert(layout_attr.window[0].height == chn_attr.res.height);

	ret = transferEfline(&layout_attr.window[0], ef_attr);
	if (ret) {
		fprintf(stderr, "Invalid ef line\n");
		return -EINVAL;
	}

	/* Check parameters */
	ret = VFTR_EF_checkParam(ef_attr, &chn_attr.res);
	if (ret) {
		log_err("Invalid EF parameters. err: %d", ret);
		return -EINVAL;
	}

#ifdef CONFIG_APP_EF_SUPPORT_SEI
	/* init AV server to wait RTSP client connectd
	 * allow transfer IVA result to RTSP streaming server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */
	ret = AVFTR_initServer();
	if (ret) {
		fprintf(stderr, "Failed to initalize AV server %d\n", ret);
		return -ENOPROTOOPT;
	}
#endif

	/* Set window index */
	g_win_idx = MPI_VIDEO_WIN(0, chn_idx, win_idx);

	/* Implement the following function */
	ret = enableOd(g_win_idx, &od_param);
	if (ret < 0) {
		fprintf(stderr, "Failed to enable OD, please check if OD is enabled\n");
	}

	ret = detectEfObject(g_win_idx, &chn_attr.res, ef_attr, shd_attr, shd_long_list);
	ret = MPI_IVA_disableObjDet(g_win_idx);

	/* init server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */

#ifdef CONFIG_APP_EF_SUPPORT_SEI
	AVFTR_exitServer();
#endif

	MPI_SYS_exit();

	return 0;
}
