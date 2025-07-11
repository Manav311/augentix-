#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "json.h"
#include "log.h"
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
#ifdef CONFIG_APP_OD_SUPPORT_SEI
#include "avftr_conn.h"
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;
#endif

#define WAIT_WIN_TIMEOUT 0

static sig_atomic_t g_run_flag = false;

#ifdef CONFIG_APP_OD_SUPPORT_SEI
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
		if (find_idx == -1 && ctx[i].idx.value == idx.value && ctx[i].en) {
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

	int ret;

	// When IPC server is initialized, reset the mmap region to all-zero
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
	VIDEO_OD_CTX_S *od_ctx;
	set_idx = findOdCtx(win_idx, vftr_res_shm->od_ctx, &empty_idx);
	if (set_idx >= 0) {
		log_warn("OD is already registered on win(%u, %u, %u).", win_idx.dev, win_idx.chn, win_idx.win);
		od_ctx = &vftr_res_shm->od_ctx[set_idx];
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

static void detectObject(MPI_WIN win_idx)
{
	UINT32 timestamp = 0;
	int ret = 0;

#ifdef CONFIG_APP_OD_SUPPORT_SEI
	int buf_info_idx = findVftrBufCtx(win_idx, vftr_res_shm->buf_info, NULL);
	int od_idx = findOdCtx(win_idx, vftr_res_shm->od_ctx, NULL);

	AVFTR_VIDEO_BUF_INFO_S *buf_info = &vftr_res_shm->buf_info[buf_info_idx];
	VIDEO_OD_CTX_S *od_ctx = &vftr_res_shm->od_ctx[od_idx];
	MPI_IVA_OBJ_LIST_S *obj_list;
	int buf_idx;
#else
	MPI_IVA_OBJ_LIST_S tmp;
	MPI_IVA_OBJ_LIST_S *obj_list = &tmp; // Use stack memory instead shared memory.
#endif

	while (g_run_flag) {
		// Wait one video frame processed.
		// This function returns after one frame is done.
		if (MPI_DEV_waitWin(win_idx, &timestamp, WAIT_WIN_TIMEOUT) != MPI_SUCCESS) {
			log_err("Failed to wait win for win: %d, %d", win_idx.chn, win_idx.win);
			continue;
		}

#ifdef CONFIG_APP_OD_SUPPORT_SEI
		buf_idx = updateBufferIdx(buf_info, timestamp);
		obj_list = &od_ctx->ol[buf_idx].basic_list;
		

#endif

		// Continuously get detection result from MPI.
		// If SEI is supported, we write the result into shared memory.
		ret = MPI_IVA_getBitStreamObjList(win_idx, timestamp, obj_list);
		if (ret != MPI_SUCCESS) {
			log_err("Failed to get object list. err: %d (It's ok if this message doesn't keep showing up.)\n",
			        ret);
			continue;
		}

#ifdef CONFIG_APP_OD_SUPPORT_SEI
		for(int i = 0; i< obj_list->obj_num; i++){
			if ( obj_list->obj[i].cat == 0 ){
				strncpy(od_ctx->ol[buf_idx].obj_attr[i].cat, "", VFTR_OBJ_CAT_LEN - 1);
				strncpy(od_ctx->ol[buf_idx].obj_attr[i].conf, "", VFTR_OBJ_CAT_LEN - 1);
			}
			else if ( obj_list->obj[i].cat == 1 ){
				strncpy(od_ctx->ol[buf_idx].obj_attr[i].cat, "person", VFTR_OBJ_CAT_LEN - 1);
				sprintf(od_ctx->ol[buf_idx].obj_attr[i].conf,"%.2f",obj_list->obj[i].conf/255.0);
			}
			else if ( obj_list->obj[i].cat == 2 ){
				strncpy(od_ctx->ol[buf_idx].obj_attr[i].cat, "car", VFTR_OBJ_CAT_LEN - 1);
				sprintf(od_ctx->ol[buf_idx].obj_attr[i].conf,"%.2f",obj_list->obj[i].conf/255.0);
			}
			else if ( obj_list->obj[i].cat == 3 ){
				strncpy(od_ctx->ol[buf_idx].obj_attr[i].cat, "pet", VFTR_OBJ_CAT_LEN - 1);
				sprintf(od_ctx->ol[buf_idx].obj_attr[i].conf,"%.2f",obj_list->obj[i].conf/255.0);
			}	
			else{
				sprintf(od_ctx->ol[buf_idx].obj_attr[i].cat,"CAT: %d",obj_list->obj[i].cat);
				sprintf(od_ctx->ol[buf_idx].obj_attr[i].conf,"%d",obj_list->obj[i].conf);
			}
		}
		// Mark buffer ready for sending SEI info
		buf_info->buf_ready[buf_idx] = 1;
#endif
	}

	return;
}

static void handleSigInt(int signo)
{
	if (signo == SIGINT || signo == SIGTERM) {
		g_run_flag = 0;
	}
}

int parseConfig(const char *file_name, MPI_IVA_OD_PARAM_S *param, MPI_IVA_OD_MOTOR_PARAM_S *motor_param)
{
	json_object *root = NULL;
	json_object *child = NULL;
	int ret = 0;

	// Error is logged by json-c.
	root = (file_name[0] != '\0') ? json_object_from_file(file_name) : json_object_new_object();
	if (!root) {
		root = json_object_new_object();
	}

	if (json_object_object_get_ex(root, "od_qual", &child)) {
		param->od_qual = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_qual", json_object_new_int(param->od_qual));
	}

	if (json_object_object_get_ex(root, "od_track_refine", &child)) {
		param->od_track_refine = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_track_refine", json_object_new_int(param->od_track_refine));
	}

	if (json_object_object_get_ex(root, "od_size_th", &child)) {
		param->od_size_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_size_th", json_object_new_int(param->od_size_th));
	}

	if (json_object_object_get_ex(root, "od_sen", &child)) {
		param->od_sen = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_sen", json_object_new_int(param->od_sen));
	}

	if (json_object_object_get_ex(root, "en_stop_det", &child)) {
		param->en_stop_det = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "en_stop_det", json_object_new_boolean(param->en_stop_det));
	}

	if (json_object_object_get_ex(root, "en_gmv_det", &child)) {
		param->en_gmv_det = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "en_gmv_det", json_object_new_boolean(param->en_gmv_det));
	}

	// Only en_motor is from MPI_IVA_OD_MOTOR_PARAM_S
	if (json_object_object_get_ex(root, "en_motor", &child)) {
		motor_param->en_motor = json_object_get_boolean(child);
	} else {
		json_object_object_add(root, "en_motor", json_object_new_boolean(motor_param->en_motor));
	}

	// New OD parameters from 2024/07 on
	if (json_object_object_get_ex(root, "od_conf_th", &child)) {
		param->od_conf_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_conf_th", json_object_new_int(param->od_conf_th));
	}

	if (json_object_object_get_ex(root, "od_iou_th", &child)) {
		param->od_iou_th = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_iou_th", json_object_new_int(param->od_iou_th));
	}

	if (json_object_object_get_ex(root, "od_snapshot_w", &child)) {
		param->od_snapshot_w = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_snapshot_w", json_object_new_int(param->od_snapshot_w));
	}

	if (json_object_object_get_ex(root, "od_snapshot_h", &child)) {
		param->od_snapshot_h = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_snapshot_h", json_object_new_int(param->od_snapshot_h));
	}

	// TODO: Seek the corresponding strings to convert od_snapshot_type into enum instead of int right now
	if (json_object_object_get_ex(root, "od_snapshot_type", &child)) {
		param->od_snapshot_type = json_object_get_int(child);
	} else {
		json_object_object_add(root, "od_snapshot_type", json_object_new_int(param->od_snapshot_type));
	}

	printf("%s\n", json_object_to_json_string(root));
	json_object_put(root);

	return ret;
}

void help(const char *name)
{
	printf("Usage: %s [options] ...\n"
	       "Options:\n"
	       "  -i <file>        MPI object detection config list. Expected .json file\n"
	       "  -c <channel>     Specify which video channel to use. (Default 0).\n"
	       "  -w <window>      Specify which video window to use. (Default 0).\n"
	       "  -t <type>        Specify which OD list to take. 0 = default OD version, 1 = ODv4.4 when ODv5.1 is on. (Default 0).\n"
	       "\n"
	       "Example:\n"
	       "  $ mpi_stream -d /system/mpp/case_config/case_config_1001_FHD &\n"
	       "  $ %s -i /system/mpp/od_config/od_conf.json &\n"
#ifdef CONFIG_APP_OD_SUPPORT_SEI
	       "  $ testOnDemandRTSPServer 0 -S\n",
#else
	       "  $ testOnDemandRTSPServer 0 -n\n",
#endif
	       name, name);
}

int main(int argc, char **argv)
{
	MPI_WIN win_idx = MPI_VIDEO_WIN(0, 0, 0);
	win_idx.dummy0 = 0;

	MPI_IVA_OD_PARAM_S od_param = {
		.od_qual = 46, .od_track_refine = 42, .od_size_th = 40, .od_sen = 254, .en_stop_det = 0, .en_gmv_det = 1
	};
	MPI_IVA_OD_MOTOR_PARAM_S od_motor_param = { .en_motor = 0 };

	char config_fname[256] = { 0 };
	int ret;
	int opt;

	while ((opt = getopt(argc, argv, "i:c:w:T:h")) != -1) {
		switch (opt) {
		case 'i':
			sprintf(config_fname, optarg);
			break;

		case 'c':
			win_idx.chn = atoi(optarg);
			break;

		case 'w':
			win_idx.win = atoi(optarg);
			break;

		case 'T':
			win_idx.dummy0 = atoi(optarg);
			break;

		case 'h':
			help(argv[0]);
			return EXIT_SUCCESS;

		default:
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

	parseConfig(config_fname, &od_param, &od_motor_param);

	// First, Initialize MPI system to access video pipeline.
	ret = MPI_SYS_init();
	if (ret) {
		return EXIT_FAILURE;
	}

	// Then, set OD attribute and start OD threading.
	// For system wide, OD threading can be activated only once.
	ret = MPI_IVA_setObjParam(win_idx, &od_param);
	if (ret != MPI_SUCCESS) {
		printf("Failed to set OD param. err: %d", ret);
		goto error;
	}

	ret = MPI_IVA_setObjMotorParam(win_idx, &od_motor_param);
	if (ret != MPI_SUCCESS) {
		printf("Failed to set OD motor param. err: %d", ret);
		goto error;
	}

	ret = MPI_IVA_enableObjDet(win_idx);
	if (ret != MPI_SUCCESS) {
		printf("Failed to enable OD. err: %d", ret);
		goto error;
	}

	// After that, continuously get object list from MPI.
	// Initialize SEI server if you need to send SEI.
	ret = initSeiServer(win_idx);
	if (ret) {
		goto error;
	}

	// Looping until user sends interrupt signals to process.
	g_run_flag = true;
	detectObject(win_idx);

error:

	exitSeiServer(win_idx);

	ret = MPI_IVA_disableObjDet(win_idx);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to disable object detection. err: %d\n", ret);
		goto error;
	}

	MPI_SYS_exit();

	// Close logger.
	closelog();

	return 0;
}
