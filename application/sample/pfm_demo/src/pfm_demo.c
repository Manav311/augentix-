#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#include "json.h"
#include "log.h"
#include "vftr.h"
#include "vftr_pfm.h"
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "mpi_sys.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_PFM_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#define WAIT_WIN_TIMEOUT 0

static sig_atomic_t g_run_flag = false;

#ifdef CONFIG_APP_PFM_SUPPORT_SEI
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static int findOdCtx(MPI_WIN idx, VIDEO_OD_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_OD_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && (ctx[i].en || ctx[i].en_implicit)) {
			find_idx = i;
		} else if (emp_idx == -1 && !(ctx[i].en || ctx[i].en_implicit)) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int findPfmCtx(MPI_WIN idx, AVFTR_PFM_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_PFM_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].reg) {
			find_idx = i;
		} else if (emp_idx == -1 && !ctx[i].en) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int findVftrBufCtx(MPI_WIN idx, const AVFTR_VIDEO_BUF_INFO_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < AVFTR_VIDEO_MAX_SUPPORT_NUM; i++) {
		if (find_idx == -1 && ctx[i].idx.value == idx.value) {
			find_idx = i;
		} else if (emp_idx == -1) {
			emp_idx = i;
		}
	}

	if (empty != NULL) {
		*empty = emp_idx;
	}

	return find_idx;
}

static int updateBufferIdx(AVFTR_VIDEO_BUF_INFO_S *buf_info, UINT32 timestamp)
{
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;

	return buf_info->buf_cur_idx;
}

// If you need to generate SEI, set the process as SEI server.
static int initSeiServer(MPI_WIN win_idx)
{
	MPI_CHN_ATTR_S attr;
	MPI_CHN chn_idx = MPI_VIDEO_CHN(0, win_idx.chn);

	AVFTR_PFM_CTX_S *pfm_ctx;
	VIDEO_OD_CTX_S *od_ctx;
	int ret;

	ret = AVFTR_initServer();
	if (ret) {
		log_err("Failed to initialize AVFTR server. err: %d", ret);
		return ret;
	}

	ret = MPI_DEV_getChnAttr(chn_idx, &attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel attribute. err: %d", ret);
		return ret;
	}

	/* init OD ctx */
	int empty_idx, set_idx;
	set_idx = findOdCtx(win_idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_err("OD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -EINVAL;
	}
	if (empty_idx < 0) {
		log_err("Failed to create OD on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	od_ctx = &vftr_res_shm->od_ctx[empty_idx];
	od_ctx->en = 1;
	od_ctx->en_implicit = 1;
	od_ctx->en_shake_det = 0;
	od_ctx->en_crop_outside_obj = 0;
	od_ctx->idx = win_idx;
	od_ctx->cb = NULL;
	od_ctx->bdry = (MPI_RECT_POINT_S){ .sx = 0, .sy = 0, .ex = attr.res.width - 1, .ey = attr.res.height - 1 };

	/* init PFM ctx */
	set_idx = findPfmCtx(win_idx, vftr_res_shm->pfm_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("PFM is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return 0;
	}
	if (empty_idx < 0) {
		log_err("Failed to create PFM instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		return -ENOMEM;
	}

	pfm_ctx = &vftr_res_shm->pfm_ctx[empty_idx];
	pfm_ctx->en = 1;
	pfm_ctx->reg = 1;
	pfm_ctx->idx = win_idx;

	// init vftr buffer ctx
	int buf_info_idx;
	set_idx = findVftrBufCtx(win_idx, vftr_res_shm->buf_info, &empty_idx);
	if (set_idx >= 0) {
		buf_info_idx = set_idx;
	} else if (empty_idx >= 0) {
		buf_info_idx = empty_idx;
	} else {
		log_err("Failed to register vftr buffer ctx on win(%u, %u, %u).", win_idx.dev, win_idx.chn,
		        win_idx.win);
		return -ENOMEM;
	}

	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	buf_info->idx = win_idx;

	return 0;
}
#define exitSeiServer() AVFTR_exitServer()
#else
// If that's no need to generated SEI, skips the actions.
static int initSeiServer(MPI_WIN win __attribute__((unused)))
{
	return 0;
}
#define exitSeiServer()
#endif

int runPfmDetection(MPI_WIN idx, const VFTR_PFM_PARAM_S *attr, uint8_t var_cfg_idx, uint8_t y_avg_cfg_idx)
{
	VFTR_PFM_INSTANCE_S *instance;
	VFTR_PFM_INPUT_S input;
	MPI_WIN_ATTR_S win_attr;
	MPI_PATH mpi_path;
	uint32_t timestamp = 0;
	int ret = 0;

	/* register mpi path */
	UINT32 dev_idx = MPI_GET_VIDEO_DEV(idx);

	ret = MPI_DEV_getWindowAttr(idx, &win_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video window attribute. err: %d", ret);
		return -EINVAL;
	}

	if (win_attr.path.bit.path0_en) {
		mpi_path = MPI_INPUT_PATH(dev_idx, 0);
	} else if (win_attr.path.bit.path1_en) {
		mpi_path = MPI_INPUT_PATH(dev_idx, 1);
	} else if (win_attr.path.bit.path2_en) {
		mpi_path = MPI_INPUT_PATH(dev_idx, 2);
	} else if (win_attr.path.bit.path3_en) {
		mpi_path = MPI_INPUT_PATH(dev_idx, 3);
	} else {
		log_err("Wrong path bmp %d setting.", win_attr.path.bmp);
		return -EINVAL;
	}

#ifdef CONFIG_APP_PFM_SUPPORT_SEI
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	int pfm_idx = findPfmCtx(idx, vftr_res_shm->pfm_ctx, NULL);
	int buf_info_idx = findVftrBufCtx(idx, vftr_res_shm->buf_info, NULL);

	VIDEO_OD_CTX_S *od_ctx = &vftr_res_shm->od_ctx[od_idx];
	AVFTR_PFM_CTX_S *pfm_ctx = &vftr_res_shm->pfm_ctx[pfm_idx];
	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	MPI_IVA_OBJ_LIST_S *obj_list;
	VFTR_PFM_STATUS_S *status;
	int buf_idx;
#else
	// Use stack memory instead shared memory.
	MPI_IVA_OBJ_LIST_S tmp_ol;
	MPI_IVA_OBJ_LIST_S *obj_list = &tmp_ol;
	VFTR_PFM_STATUS_S tmp_status;
	VFTR_PFM_STATUS_S *status = &tmp_status;
#endif
	VFTR_init(NULL);

	instance = VFTR_PFM_newInstance();
	if (!instance) {
		log_err("Failed to create PFM instance.");
		return -EINVAL;
	}

	// Set parameter to PFM
	ret = VFTR_PFM_setParam(instance, attr);
	if (ret) {
		log_err("Failed to set PFM parameters.");
		return -EINVAL;
	}

	g_run_flag = true;
	while (g_run_flag) {
		// Wait one video frame processed.
		// This function returns after one frame is done.
		ret = MPI_DEV_waitWin(idx, &timestamp, WAIT_WIN_TIMEOUT);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to waitWin for win: (%d, %d, %d). err: %d", idx.dev, idx.chn, idx.win, ret);
			continue;
		}

#ifdef CONFIG_APP_PFM_SUPPORT_SEI
		buf_idx = updateBufferIdx(buf_info, timestamp);
		obj_list = &od_ctx->ol[buf_idx].basic_list;
#endif

		input.obj_list = obj_list;

		// Continuously get detection result from MPI.
		// If SEI is supported, we write the result into shared memory.
		ret = MPI_IVA_getBitStreamObjList(idx, timestamp, obj_list);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to get object list. err: %d", ret);
			continue;
		}

		ret = MPI_DEV_getIspVar(idx, var_cfg_idx, &input.var);
		if (ret != MPI_SUCCESS) {
			log_err("Get variance fail for win: (%d, %d, %d). err: %d", idx.dev, idx.chn, idx.win, ret);
			continue;
		}

		ret = MPI_DEV_getIspYAvg(idx, y_avg_cfg_idx, &input.y_avg);
		if (ret != MPI_SUCCESS) {
			log_err("Get Y average fail for win: (%d, %d, %d). err: %d", idx.dev, idx.chn, idx.win, ret);
			continue;
		}

		ret = MPI_getStatistics(mpi_path, &input.dip_stat);
		if (ret != MPI_SUCCESS) {
			log_err("Get DIP stat on configuration of window %u\n", idx.value);
			continue;
		}

#ifdef CONFIG_APP_PFM_SUPPORT_SEI
		pfm_ctx->stat[buf_idx].roi = attr->roi;
		status = &pfm_ctx->stat[buf_idx].data;
#endif

		// Send video statistics to PFM algorithm module.
		ret = VFTR_PFM_detect(instance, &input, status);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to update PFM. err: %d", ret);
			continue;
		}

#ifdef CONFIG_APP_PFM_SUPPORT_SEI
		// Mark buffer ready for sending SEI info
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	VFTR_PFM_deleteInstance(&instance);

	return 0;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_run_flag = 0;
	}
}

int parseConfig(const char *file_name, MPI_IVA_OD_PARAM_S *od_param, VFTR_PFM_PARAM_S *pfm_param)
{
	json_object *root = NULL;
	json_object *child = NULL;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	if (json_object_object_get_ex(root, "sensitivity", &child)) {
		pfm_param->sensitivity = json_object_get_int(child);
	} else {
		json_object_object_add(root, "sensitivity", json_object_new_int(pfm_param->sensitivity));
	}

	if (json_object_object_get_ex(root, "endurance", &child)) {
		pfm_param->endurance = json_object_get_int(child);
	} else {
		json_object_object_add(root, "endurance", json_object_new_int(pfm_param->endurance));
	}

	// RoI is required parameter.
	if (!json_object_object_get_ex(root, "roi", &child)) {
		log_err("Not found required parameter 'roi' in configuration file.");
		exit(1);
	}

	pfm_param->roi.sx = (int16_t)json_object_get_int(json_object_array_get_idx(child, 0));
	pfm_param->roi.sy = (int16_t)json_object_get_int(json_object_array_get_idx(child, 1));
	pfm_param->roi.ex = (int16_t)json_object_get_int(json_object_array_get_idx(child, 2));
	pfm_param->roi.ey = (int16_t)json_object_get_int(json_object_array_get_idx(child, 3));

	if (json_object_object_get_ex(root, "od_qual", &child)) {
		od_param->od_qual = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_qual", json_object_new_int(od_param->od_qual));
	}

	if (json_object_object_get_ex(root, "od_track_refine", &child)) {
		od_param->od_track_refine = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_track_refine", json_object_new_int(od_param->od_track_refine));
	}

	if (json_object_object_get_ex(root, "od_size_th", &child)) {
		od_param->od_size_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_size_th", json_object_new_int(od_param->od_size_th));
	}

	if (json_object_object_get_ex(root, "od_sen", &child)) {
		od_param->od_sen = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_sen", json_object_new_int(od_param->od_sen));
	}

	if (json_object_object_get_ex(root, "en_stop_det", &child)) {
		od_param->en_stop_det = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "en_stop_det", json_object_new_boolean(od_param->en_stop_det));
	}

	printf("%s\n", json_object_to_json_string(root));
	json_object_put(root);

	return 0;
}

void help(const char *name)
{
	printf("Usage: %s -i [options] ...\n"
	       "Options:\n"
	       "  -i <file>            PFM parameter in .json format.\n"
	       "  -c <channel>         Specify which video channel to use. (Default 0).\n"
	       "  -w <window>          Specify which video window to user (Default 0).\n"
	       "\n"
	       "Example:\n"
	       "  $ mpi_stream -d /system/mpp/case_config/case_config_1001_FHD &\n"
	       "  $ %s -i /system/mpp/pfm_config/pfm_conf.json &\n"
#ifdef CONFIG_APP_PFM_SUPPORT_SEI
	       "  $ testOnDemandRTSPServer 0 -S\n",
#else
	       "  $ testOnDemandRTSPServer 0 -n\n",
#endif
	       name, name);
}

int main(int argc, char **argv)
{
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);
	MPI_CHN chn_idx = MPI_VIDEO_CHN(0, 0);
	MPI_CHN_LAYOUT_S layout_attr;

	MPI_ISP_VAR_CFG_S var_cfg = { { 0 } };
	MPI_ISP_Y_AVG_CFG_S y_avg_cfg = {
		.roi = { .sx = 0, .sy = 0, .ex = 0, .ey = 0 },
		.diff_thr = 0,
	};
	MPI_IVA_OD_PARAM_S od_param = {
		.od_qual = 46,
		.od_track_refine = 42,
		.od_size_th = 40,
		.od_sen = 254,
		.en_stop_det = 0,
		.en_gmv_det = 0,
	};
	// RoI attribute must be load from config file.
	VFTR_PFM_PARAM_S pfm_param = {
		.sensitivity = 116,
		.endurance = 5,
		.roi = { .sx = 0, .sy = 0, .ex = 0, .ey = 0 },
	};

	int c;
	char config_fname[256] = { 0 };
	uint8_t var_cfg_idx;
	uint8_t y_avg_cfg_idx;
	int ret;

	while ((c = getopt(argc, argv, "i:c:w:h")) != -1) {
		switch (c) {
		case 'i':
			sprintf(config_fname, optarg);
			break;

		case 'c':
			win_idx.chn = atoi(optarg);
			chn_idx.chn = atoi(optarg);
			break;

		case 'w':
			win_idx.win = atoi(optarg);
			break;

		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;

		default:
			help(argv[0]);
			return EXIT_FAILURE;
		}
	}

	// Open logger. Currently we use syslog as our logging system.
	openlog(NULL, 0, LOG_LOCAL7);

	if (signal(SIGINT, handleSigInt) == SIG_ERR) {
		log_err("Cannot handle SIGINT! err: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	if (signal(SIGTERM, handleSigInt) == SIG_ERR) {
		log_err("Cannot handle SIGTERM! err: %s", strerror(errno));
		return EXIT_FAILURE;
	}

	parseConfig(config_fname, &od_param, &pfm_param);
	var_cfg.roi = y_avg_cfg.roi = pfm_param.roi;

	// First, initialize MPI system to access video pipeline.
	ret = MPI_SYS_init();
	if (ret != MPI_SUCCESS) {
		return EXIT_FAILURE;
	}

	// Then, set OD attribute and start OD threading.
	// For system wide, OD threading can be activated only once.
	ret = MPI_IVA_setObjParam(win_idx, &od_param);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to set OD param. err: %d", ret);
		goto error;
	}

	ret = MPI_DEV_getChnLayout(chn_idx, &layout_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel layout attribute. err: %d", ret);
		goto error;
	}

	/* Check parameters */
	pfm_param.window = layout_attr.window[win_idx.win];
	ret = VFTR_PFM_checkParam(&pfm_param);
	if (ret) {
		log_err("Invalid PFM parameters. err: %d", ret);
		goto error;
	}

	ret = MPI_IVA_enableObjDet(win_idx);
	if (ret) {
		log_err("Failed to enable object detection. err: %d", ret);
		goto error;
	}

	// After that, continuously get object list from MPI.
	// Initialize SEI server if you need to send SEI.
	ret = initSeiServer(win_idx);
	if (ret) {
		goto error;
	}

	ret = MPI_DEV_addIspYAvgCfg(win_idx, &y_avg_cfg, &y_avg_cfg_idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to add ISP Y average config. err: %d", ret);
		goto error;
	}

	ret = MPI_DEV_addIspVarCfg(win_idx, &var_cfg, &var_cfg_idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to add ISP variance config. err: %d", ret);
		goto error;
	}

	runPfmDetection(win_idx, &pfm_param, var_cfg_idx, y_avg_cfg_idx);
	VFTR_exit();

error:

	// Temporary do not remove config, otherwise leads video pipeline hanging
	// ret = MPI_DEV_rmIspVarCfg(win_idx, var_cfg_idx);
	// if (ret != MPI_SUCCESS) {
	// 	log_err("Failed to remove variance config. err: %d", ret);
	// }

	ret = MPI_DEV_rmIspYAvgCfg(win_idx, y_avg_cfg_idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to remove ISP Y average config. err: %d", ret);
	}

	exitSeiServer();

	ret = MPI_IVA_disableObjDet(win_idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to disable object detection. err: %d", ret);
	}

	MPI_SYS_exit();

	closelog();

	return 0;
}
