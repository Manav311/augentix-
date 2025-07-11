#include "json_object.h"
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
#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "mpi_sys.h"
#include "vftr.h"
#include "vftr_fgd.h"
#include "avftr_bm.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_BM_SUPPORT_SEI
#include "avftr_conn.h"
#endif

#define WAIT_WIN_TIMEOUT 0

static volatile sig_atomic_t g_run_flag = false;

#ifdef CONFIG_APP_BM_SUPPORT_SEI
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

static int clearOdCtx(MPI_WIN idx, VIDEO_OD_CTX_S *ctx)
{
	int ctx_idx = findOdCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The OD ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(VIDEO_OD_CTX_S));
	return 0;
}

static int findBmCtx(MPI_WIN idx, const AVFTR_BM_CTX_S *ctx, int *empty)
{
	int i = 0;
	int find_idx = -1;
	int emp_idx = -1;

	if (empty == NULL) {
		emp_idx = -2;
	} else {
		emp_idx = -1;
	}

	for (i = 0; i < VIDEO_BM_MAX_SUPPORT_NUM; i++) {
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

static int clearBmCtx(MPI_WIN idx, AVFTR_BM_CTX_S *ctx)
{
	int ctx_idx = findBmCtx(idx, ctx, NULL);
	if (ctx_idx < 0) {
		log_err("The BM ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&ctx[ctx_idx], 0, sizeof(AVFTR_BM_CTX_S));
	return 0;
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
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].en) {
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

static int clearVftrBufCtx(MPI_WIN idx, AVFTR_VIDEO_BUF_INFO_S *info)
{
	int info_idx = findVftrBufCtx(idx, info, NULL);
	if (info_idx < 0) {
		log_err("The VFTR buffer ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&info[info_idx], 0, sizeof(AVFTR_VIDEO_BUF_INFO_S));
	return 0;
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

	AVFTR_BM_CTX_S *bm_ctx;
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
		log_warn("OD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		od_ctx = &vftr_res_shm->od_ctx[set_idx];
		return 0;
	} else {
		if (empty_idx < 0) {
			log_err("Failed to create OD on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
			return -ENOMEM;
		}

		od_ctx = &vftr_res_shm->od_ctx[empty_idx];
		od_ctx->en = 1;
		od_ctx->en_implicit = 0;
		od_ctx->en_shake_det = 0;
		od_ctx->en_crop_outside_obj = 0;
		od_ctx->idx = win_idx;
		od_ctx->cb = NULL;
		od_ctx->bdry =
		        (MPI_RECT_POINT_S){ .sx = 0, .sy = 0, .ex = attr.res.width - 1, .ey = attr.res.height - 1 };
	}

	/* init BM ctx */
	set_idx = findBmCtx(win_idx, vftr_res_shm->bm_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("BM is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		bm_ctx = &vftr_res_shm->bm_ctx[set_idx];
		return 0;
	} else {
		if (empty_idx < 0) {
			log_err("Failed to create BM instance on win(%u, %u, %u).", win_idx.dev, win_idx.chn,
			        win_idx.win);
			return -ENOMEM;
		}

		bm_ctx = &vftr_res_shm->bm_ctx[empty_idx];
		bm_ctx->en = 1;
		bm_ctx->reg = 1;
		bm_ctx->idx = win_idx;
	}

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
	buf_info->en = 1;

	return 0;
}
static void exitSeiServer(MPI_WIN idx)
{
	clearOdCtx(idx, vftr_res_shm->od_ctx);
	clearBmCtx(idx, vftr_res_shm->bm_ctx);
	clearVftrBufCtx(idx, vftr_res_shm->buf_info);
	AVFTR_exitServer();
}
#else
// If that's no need to generated SEI, skips the actions.
static int initSeiServer(MPI_WIN win __attribute__((unused)))
{
	return 0;
}
#define exitSeiServer()
#endif

static void determineBabyMonitorStatus(const VFTR_FGD_STATUS_S *foreground_status, const VFTR_FGD_PARAM_S *param,
                                       AVFTR_BM_STATUS_S *bm_res)
{
#define VIDEO_BM_ACTIVE_THRESHOLD (8)
#define VIDEO_BM_AWAKE_THRESHOLD (4)

	const MPI_RECT_POINT_S *roi = &param->roi;

	bm_res->fgd_stat = *foreground_status;
	bm_res->roi = *roi;
	switch (foreground_status->event) {
	case VFTR_FGD_OBJECT_ABSENT:
		bm_res->current_event = AVFTR_BM_EVENT_ABSENT;
		break;
	case VFTR_FGD_OBJECT_BOUNDARY:
		bm_res->current_event = AVFTR_BM_EVENT_BOUNDARY;
		bm_res->duration_active++;
		break;
	case VFTR_FGD_OBJECT_ENTERING:
		bm_res->current_event = AVFTR_BM_EVENT_ENTERING;
		break;
	case VFTR_FGD_OBJECT_LEAVING:
		bm_res->current_event = AVFTR_BM_EVENT_LEAVING;
		break;
	case VFTR_FGD_OBJECT_PRESENT:
		if (foreground_status->motion_level >= VIDEO_BM_ACTIVE_THRESHOLD) {
			bm_res->current_event = AVFTR_BM_EVENT_ACTIVE;
			bm_res->duration_active++;
		} else if (foreground_status->motion_level >= VIDEO_BM_AWAKE_THRESHOLD) {
			bm_res->current_event = AVFTR_BM_EVENT_AWAKE;
			bm_res->duration_awake++;
		} else {
			bm_res->current_event = AVFTR_BM_EVENT_SLEEP;
			bm_res->duration_sleep++;
		}
		break;
	default:
		assert(0);
	}

	return;
}

int runBabyMonitor(MPI_WIN idx, const VFTR_FGD_PARAM_S *attr, uint8_t y_avg_cfg_idx)
{
	VFTR_FGD_INSTANCE_S *instance;
	VFTR_FGD_INPUT_S input;
	uint32_t timestamp = 0;
	int ret = 0;

#ifdef CONFIG_APP_BM_SUPPORT_SEI
	int od_idx = findOdCtx(idx, vftr_res_shm->od_ctx, NULL);
	int bm_idx = findBmCtx(idx, vftr_res_shm->bm_ctx, NULL);
	int buf_info_idx = findVftrBufCtx(idx, vftr_res_shm->buf_info, NULL);
	AVFTR_BM_STATUS_S bm_res = { 0 };

	VIDEO_OD_CTX_S *od_ctx = &vftr_res_shm->od_ctx[od_idx];
	AVFTR_BM_CTX_S *bm_ctx = &vftr_res_shm->bm_ctx[bm_idx];
	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	MPI_IVA_OBJ_LIST_S *obj_list;
	VFTR_FGD_STATUS_S *status;
	int buf_idx;
#else
	// Use stack memory instead shared memory.
	AVFTR_BM_CTX_S tmp_bm_ctx;
	AVFTR_BM_CTX_S *bm_ctx = &tmp_bm_ctx;
	AVFTR_BM_STATUS_S bm_res = { 0 };
	MPI_IVA_OBJ_LIST_S tmp_ol;
	MPI_IVA_OBJ_LIST_S *obj_list = &tmp_ol;
	VFTR_FGD_STATUS_S tmp_status;
	VFTR_FGD_STATUS_S *status = &tmp_status;
	const int buf_idx = 0;
#endif

	VFTR_init(NULL);

	instance = VFTR_FGD_newInstance();
	if (!instance) {
		log_err("Failed to create FGD instance.");
		return -EINVAL;
	}

	// Set parameter to FGD
	ret = VFTR_FGD_setParam(instance, attr);
	if (ret) {
		log_err("Failed to set FGD parameters.");
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

#ifdef CONFIG_APP_BM_SUPPORT_SEI
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

		ret = MPI_DEV_getIspYAvg(idx, y_avg_cfg_idx, &input.y_avg);
		if (ret != MPI_SUCCESS) {
			log_err("Get Y average fail for win: (%d, %d, %d). err: %d", idx.dev, idx.chn, idx.win, ret);
			continue;
		}

#ifdef CONFIG_APP_BM_SUPPORT_SEI
		bm_res.roi = bm_ctx->bm_res[buf_idx].roi = attr->roi;
		status = &bm_ctx->bm_res[buf_idx].fgd_stat;
#endif

		// Send video statistics to FGD algorithm module.
		ret = VFTR_FGD_detect(instance, &input, status);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to detect foreground. err: %d", ret);
			continue;
		}

		determineBabyMonitorStatus(status, attr, &bm_res);
		bm_ctx->bm_res[buf_idx] = bm_res;
#ifdef CONFIG_APP_BM_SUPPORT_SEI
		// Mark buffer ready for sending SEI info
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	VFTR_FGD_deleteInstance(&instance);
	VFTR_exit();

	return 0;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_run_flag = false;
	}
}

int parseConfig(const char *file_name, MPI_IVA_OD_PARAM_S *od_param, VFTR_FGD_PARAM_S *fgd_param)
{
	json_object *root = NULL;
	json_object *child = NULL;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	if (json_object_object_get_ex(root, "sensitivity", &child)) {
		fgd_param->sensitivity = json_object_get_int(child);
	} else {
		json_object_object_add(root, "sensitivity", json_object_new_int(fgd_param->sensitivity));
	}

	if (json_object_object_get_ex(root, "boundary_thickness", &child)) {
		fgd_param->boundary_thickness = json_object_get_int(child);
	} else {
		json_object_object_add(root, "boundary_thickness", json_object_new_int(fgd_param->boundary_thickness));
	}

	if (json_object_object_get_ex(root, "quality", &child)) {
		fgd_param->quality = json_object_get_int(child);
	} else {
		json_object_object_add(root, "quality", json_object_new_int(fgd_param->quality));
	}

	if (json_object_object_get_ex(root, "obj_life_th", &child)) {
		fgd_param->obj_life_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "obj_life_th", json_object_new_int(fgd_param->obj_life_th));
	}

	if (json_object_object_get_ex(root, "time_buffer", &child)) {
		fgd_param->time_buffer = json_object_get_int(child);
	} else {
		json_object_object_add(root, "time_buffer", json_object_new_int(fgd_param->time_buffer));
	}

	if (json_object_object_get_ex(root, "suppression", &child)) {
		fgd_param->suppression = json_object_get_int(child);
	} else {
		json_object_object_add(root, "suppression", json_object_new_int(fgd_param->suppression));
	}

	// RoI is required parameter.
	if (!json_object_object_get_ex(root, "roi", &child)) {
		log_err("Not found required parameter 'roi' in configuration file.");
		exit(1);
	}

	fgd_param->roi.sx = (int16_t)json_object_get_int(json_object_array_get_idx(child, 0));
	fgd_param->roi.sy = (int16_t)json_object_get_int(json_object_array_get_idx(child, 1));
	fgd_param->roi.ex = (int16_t)json_object_get_int(json_object_array_get_idx(child, 2));
	fgd_param->roi.ey = (int16_t)json_object_get_int(json_object_array_get_idx(child, 3));

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
	       "  -i <file>            BM parameter in .json format.\n"
	       "  -c <channel>         Specify which video channel to use. (Default 0).\n"
	       "  -w <window>          Specify which video window to use. (Default 0).\n"
	       "\n"
	       "Example:\n"
	       "  $ mpi_stream -d /system/mpp/case_config/case_config_1001_FHD &\n"
	       "  $ %s -i /system/mpp/bm_config/bm_conf.json &\n"
#ifdef CONFIG_APP_BM_SUPPORT_SEI
	       "  $ testOnDemandRTSPServer 0 -S\n",
#else
	       "  $ testOnDemandRTSPServer 0 -n\n",
#endif
	       name, name);
}

int main(int argc, char **argv)
{
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);

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
	VFTR_FGD_PARAM_S fgd_param = {
		.sensitivity = 116,
		.boundary_thickness = 4,
		.quality = 255,
		.obj_life_th = 16,
		.time_buffer = 8192,
		.suppression = 100,
		.roi = { .sx = 0, .sy = 0, .ex = 0, .ey = 0 },
	};

	int c;
	const char *config_fname = NULL;
	uint8_t y_avg_cfg_idx;
	int err;

	while ((c = getopt(argc, argv, "i:c:w:h")) != -1) {
		switch (c) {
		case 'i':
			config_fname = optarg;
			break;

		case 'c':
			win_idx.chn = atoi(optarg);
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

	parseConfig(config_fname, &od_param, &fgd_param);
	y_avg_cfg.roi = fgd_param.roi;

	// First, initialize MPI system to access video pipeline.
	err = MPI_SYS_init();
	if (err != MPI_SUCCESS) {
		return EXIT_FAILURE;
	}

	// Then, set OD attribute and start OD threading.
	// For system wide, OD threading can be activated only once.
	err = MPI_IVA_setObjParam(win_idx, &od_param);
	if (err != MPI_SUCCESS) {
		log_err("Failed to set OD param. err: %d", err);
		goto error;
	}

	/* Check parameters */
	err = VFTR_FGD_checkParam(&fgd_param);
	if (err) {
		log_err("Invalid FGD parameters. err: %d", err);
		goto error;
	}

	err = MPI_IVA_enableObjDet(win_idx);
	if (err != MPI_SUCCESS) {
		log_err("Failed to enable object detection. err: %d", err);
		goto error;
	}

	// After that, continuously get object list from MPI.
	// Initialize SEI server if you need to send SEI.
	err = initSeiServer(win_idx);
	if (err) {
		goto error;
	}

	err = MPI_DEV_addIspYAvgCfg(win_idx, &y_avg_cfg, &y_avg_cfg_idx);
	if (err != MPI_SUCCESS) {
		log_err("Failed to add ISP Y average config. err: %d", err);
		goto error;
	}

	runBabyMonitor(win_idx, &fgd_param, y_avg_cfg_idx);

error:

	err = MPI_DEV_rmIspYAvgCfg(win_idx, y_avg_cfg_idx);
	if (err != MPI_SUCCESS) {
		log_err("Failed to remove ISP Y average config. err: %d", err);
	}

	exitSeiServer(win_idx);

	err = MPI_IVA_disableObjDet(win_idx);
	if (err != MPI_SUCCESS) {
		log_err("Failed to disable object detection. err: %d", err);
	}

	MPI_SYS_exit();

	closelog();

	return 0;
}
