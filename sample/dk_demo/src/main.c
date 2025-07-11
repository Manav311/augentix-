#define _GNU_SOURCE
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
#include "mpi_sys.h"
#include "vftr_dk.h"
#include "log.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_DK_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#include <dk_demo.h>

int g_dk_running; /* stop and start dk program*/

#ifdef CONFIG_APP_DK_SUPPORT_SEI
extern AVFTR_CTX_S *avftr_res_shm;
#endif

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

static void handleSigInt(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_dk_running = 0;
	}
}

int readJsonFromFile(char *file_name, VFTR_DK_PARAM_S *dk_attr)
{
	int ret = 0;
	json_object *test_obj = NULL;
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;

	/* load json config from json file */
	test_obj = json_object_from_file(file_name);
	if (!test_obj) {
		log_err("Cannot open %s", file_name);
		return -EBADF;
	}

	/* DK parameter */
	json_object_object_get_ex(test_obj, "obj_life_th", &tmp1_obj);
	dk_attr->obj_life_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "sensitivity");
		ret = -EINVAL;
		goto error;
	}

	json_object_object_get_ex(test_obj, "loiter_period_th", &tmp1_obj);
	dk_attr->loiter_period_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "endurance");
		ret = -EINVAL;
		goto error;
	}

	json_object_object_get_ex(test_obj, "overlap_ratio_th", &tmp1_obj);
	dk_attr->overlap_ratio_th = json_object_get_int(tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "endurance");
		ret = -EINVAL;
		goto error;
	}

	json_object_object_get_ex(test_obj, "roi", &tmp1_obj);
	if (!tmp1_obj) {
		log_err("Cannot get %s object", "roi");
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "start_x", &tmp2_obj);
	dk_attr->roi_pts.sx = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		log_err("Cannot get %s object", "start_x");
		ret = -EINVAL;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "start_y", &tmp2_obj);
	dk_attr->roi_pts.sy = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		log_err("Cannot get %s object", "start_y");
		ret = -EINVAL;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "end_x", &tmp2_obj);
	dk_attr->roi_pts.ex = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		log_err("Cannot get %s object", "end_x");
		ret = -EINVAL;
		goto error;
	}

	json_object_object_get_ex(tmp1_obj, "end_y", &tmp2_obj);
	dk_attr->roi_pts.ey = json_object_get_int(tmp2_obj);
	if (!tmp2_obj) {
		log_err("Cannot get %s object", "end_y");
		ret = -EINVAL;
		goto error;
	}

	/*Avoid invalid value*/
	dk_attr->overlap_ratio_th = (dk_attr->overlap_ratio_th << VFTR_DK_OVERLAP_FRACTION) /
	                            100; /* dk->overlap_ratio_th is the value setting from webgui, range [0,100] .*/
	dk_attr->roi_size =
	        (dk_attr->roi_pts.ex - dk_attr->roi_pts.sx) * (dk_attr->roi_pts.ey - dk_attr->roi_pts.sy) / 100;

error:
	json_object_put(test_obj);

	return ret;
}

int enableOd(MPI_WIN win, const MPI_IVA_OD_PARAM_S *od)
{
	int ret = 0;

	ret = MPI_IVA_setObjParam(win, od);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to set OD param. err: %d", ret);
		goto error;
	}

	ret = MPI_IVA_enableObjDet(win);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to enable OD. err: %d", ret);
		goto error;
	}

	return MPI_SUCCESS;

error:
	return MPI_FAILURE;
}
/* OD parameters share to all region */

void help(void)
{
	printf("USAGE:\tdk_demo -i <CONFIG>\t\n");
	printf("\t-i <file>\t\tdoor keeper config in .json file\n");
	printf("\tobj_life_th: obj_life_th stands for object confidence threshold of OD. It uses to filter low confident objects.\n");
	printf("\tThe range is [0-160] and the default value 30.\n");
	printf("\tloiter_period_th: Period threshold for loitering status (unit: 1/100s). The range is [0-6000] and the default value 300.\n");
	printf("\toverlap_ratio_th: Overlap threshold with roi. If the object and roi overlap proportion is over overlap_ratio_th,\n");
	printf("\tit means the object enter roi. The range is [0-256] and the default value 25.\n");
	printf("OPTIONS:\n");
	printf("\t-c <channel>\t\tSpecify which video channel to use. (Deatult 0).\n");
	printf("\n");
	printf("For example:\n");
	printf("\tmpi_stream -d /system/mpp/case_config/case_config_1001_FHD -precord_enable=1 -poutput_file=/dev/null -pframe_num=-1 &\n");
	printf("\tdk_demo -i /system/mpp/dk_config/dk_conf_1080p_1.json &\n");
#ifdef CONFIG_APP_DK_SUPPORT_SEI
	printf("\ttestOnDemandRTSPServer 0 -S\n");
#else
	printf("\ttestOnDemandRTSPServer 0 -n\n");
#endif
	printf("\n");
}

int main(int argc, char **argv)
{
	int ret;
	int c;
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);
	MPI_CHN chn_idx = MPI_VIDEO_CHN(0, 0);
	char cfg_file_name[256] = { 0 };
	MPI_IVA_OD_PARAM_S od_param = {
		.od_qual = 46, .od_track_refine = 42, .od_size_th = 40, .od_sen = 254, .en_stop_det = 0, .en_gmv_det = 0
	};

	VFTR_DK_PARAM_S dk_attr;

	while ((c = getopt(argc, argv, "i:c:w:r:q:s:t:h")) != -1) {
		switch (c) {
		case 'i':
			sprintf(cfg_file_name, optarg);
			DBG("input .json file:%s\n", cfg_file_name);
			break;
		case 'c':
			win_idx.chn = atoi(optarg);
			chn_idx.chn = atoi(optarg);
			DBG("set device channel:%d\n", win_idx.chn);
			break;
		case 'w':
			win_idx.win = atoi(optarg);
			DBG("set device window:%d\n", win_idx.win);
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
			help();
			exit(0);
		default:
			abort();
		}
	}

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (strlen(cfg_file_name) == 0) {
		log_err("Config file path is not specified!");
		exit(1);
	}

	ret = readJsonFromFile(cfg_file_name, &dk_attr);
	if (ret != MPI_SUCCESS) {
		DBG("Read Json form file:%s\n", cfg_file_name);
		return -EINVAL;
	}

	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN_STAT_S chn_stat;

	/* Initialize MPI system */
	MPI_SYS_init();

	ret = MPI_DEV_getChnAttr(chn_idx, &chn_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel attribute. err: %d", ret);
		return -EINVAL;
	}

	ret = MPI_DEV_getChnLayout(chn_idx, &layout_attr);
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

	ret = MPI_DEV_queryChnState(chn_idx, &chn_stat);
	if (ret != 0) {
		log_err("Query channel state on channel %d on device %d failed", win_idx.chn, win_idx.dev);
		return ret;
	}

	if (!MPI_STATE_IS_ADDED(chn_stat.status)) {
		log_err("Channel %d on device %d is not added", win_idx.chn, win_idx.dev);
		return ENODEV;
	}

	/* Check parameters */
	ret = VFTR_DK_checkParam(&dk_attr);
	if (ret) {
		log_err("Invalid DK parameters");
		return -EINVAL;
	}

#ifdef CONFIG_APP_DK_SUPPORT_SEI
	/* init AV server to wait RTSP client connect
	 * allow transfer IVA result to RTSP streaming server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */
	ret = AVFTR_initServer();
	if (ret) {
		log_err("Failed to initialize AV server %d", ret);
		return -ENOPROTOOPT;
	}
#endif

	/* Implement the following function */
	/* EnableOd() */
	ret = enableOd(win_idx, &od_param);
	if (ret < 0) {
		log_err("Failed to enable OD, please check if OD is enabled");
	}

	/* DetectDkObject() */
	ret = detectDkObject(win_idx, &chn_attr.res, &dk_attr);
	ret = MPI_IVA_disableObjDet(win_idx);

#ifdef CONFIG_APP_DK_SUPPORT_SEI
	AVFTR_exitServer();
#endif

	MPI_SYS_exit();

	return 0;
}
