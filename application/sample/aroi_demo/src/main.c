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
#include "mpi_iva.h"
#include "mpi_sys.h"
#include "vftr_aroi.h"
#include "video_ptz.h"
#include "aroi_demo.h"
#include "log.h"

/**
 * If SUPPORT_SEI is defined, the application supports sending
 * inference result to clients via RTSP server.
 *
 * To do so, inter-process communication should be initalized by
 * API AVFTR_initServer().
 */
#ifdef CONFIG_APP_AROI_SUPPORT_SEI
#include "avftr_conn.h"
#endif

/* default setting */
VFTR_AROI_PARAM_S g_aroi_attr = { .obj_life_th = 30,
	                          .aspect_ratio = (192 << VFTR_AROI_AR_FRACTIONAL_BIT) / 108,
	                          .min_roi = { 100, 100 },
	                          .max_roi = { 1920, 1080 },
	                          .max_track_delta_x = 50,
	                          .max_track_delta_y = 50,
	                          .max_return_delta_x = 8,
	                          .max_return_delta_y = 8,
	                          .wait_time = 29,
	                          .update_ratio = 16 };
MPI_WIN g_win_idx;
int g_aroi_running = 1;
int g_detect_interval = 1;

#ifdef CONFIG_APP_AROI_SUPPORT_SEI
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
		g_aroi_running = 0;
	}
}

int readJsonFromFile(const char *file_name, VFTR_AROI_PARAM_S *data,
                     VIDEO_FTR_PTZ_PARAM_S *ptz_attr __attribute__((unused)))
{
	int aspect_ratio_height = -1;
	int aspect_ratio_width = -1;
	int track_speed = 0;
	int return_speed = -1;

	json_object *cmd_obj = NULL;
	json_object *tmp_obj = NULL;

	cmd_obj = json_object_from_file(file_name);
	if (!cmd_obj) {
		log_err("Cannot open %s", file_name);
		return -EBADF;
	}

	if (json_object_object_get_ex(cmd_obj, "aspect_ratio_height", &tmp_obj)) {
		aspect_ratio_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "aspect_ratio_width", &tmp_obj)) {
		aspect_ratio_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		data->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_roi_height", &tmp_obj)) {
		data->max_roi.height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "max_roi_width", &tmp_obj)) {
		data->max_roi.width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_roi_height", &tmp_obj)) {
		data->min_roi.height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "min_roi_width", &tmp_obj)) {
		data->min_roi.width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "return_speed", &tmp_obj)) {
		return_speed = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "track_speed", &tmp_obj)) {
		track_speed = json_object_get_int(tmp_obj);
	}

#ifdef CONFIG_APP_PTZ_SUPPORT
	json_object *tmp1_obj = NULL;
	json_object *tmp2_obj = NULL;

	if (json_object_object_get_ex(cmd_obj, "roi_width", &tmp_obj)) {
		ptz_attr->roi_bd.width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "roi_height", &tmp_obj)) {
		ptz_attr->roi_bd.height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_speed_x", &tmp_obj)) {
		ptz_attr->speed.x = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_speed_y", &tmp_obj)) {
		ptz_attr->speed.y = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_size_limit_max", &tmp_obj)) {
		ptz_attr->win_size_limit.max = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "win_size_limit_min", &tmp_obj)) {
		ptz_attr->win_size_limit.min = json_object_get_int(tmp_obj);
	}
	json_object_object_get_ex(cmd_obj, "subwindow_disp", &tmp_obj);

	if (json_object_object_get_ex(tmp_obj, "win_num", &tmp1_obj)) {
		ptz_attr->win_num = json_object_get_int(tmp1_obj);
	}

	json_object_object_get_ex(tmp_obj, "win", &tmp1_obj);

	if (json_object_object_get_ex(tmp1_obj, "chn_idx", &tmp2_obj)) {
		ptz_attr->win_id->chn = json_object_get_int(tmp2_obj);
	}
	if (json_object_object_get_ex(tmp1_obj, "win_idx", &tmp2_obj)) {
		ptz_attr->win_id->win = json_object_get_int(tmp2_obj);
	}
	/* only support auto mode and video index(0,0,0) */
	ptz_attr->mode = 0;
	ptz_attr->win_id->dev = 0;
	ptz_attr->mv = (MPI_MOTION_VEC_S){ .x = 0, .y = 0 };
	ptz_attr->zoom_v = (MPI_MOTION_VEC_S){ .x = 0, .y = 0 };

#endif /* CONFIG_APP_PTZ_SUPPORT */

	if (aspect_ratio_width != -1 && aspect_ratio_width != -1) {
		if (aspect_ratio_width == 0 || aspect_ratio_height == 0)
			data->aspect_ratio = 0;
		else
			data->aspect_ratio = (aspect_ratio_width << VFTR_AROI_AR_FRACTIONAL_BIT) / aspect_ratio_height;
	}
	if (return_speed != -1) {
		data->max_return_delta_y = return_speed;
		data->max_return_delta_y = return_speed;
	}
	data->max_track_delta_x = track_speed;
	data->max_track_delta_y = track_speed;

	json_object_put(cmd_obj);

	return MPI_SUCCESS;
}

void printParam(void)
{
	VFTR_AROI_PARAM_S *aroi_attr = &g_aroi_attr;
#define printInt(p) printf(#p " = %d\n", (int)p)
	printInt(aroi_attr->max_roi.height);
	printInt(aroi_attr->max_roi.width);
	printInt(aroi_attr->min_roi.height);
	printInt(aroi_attr->min_roi.width);
	printInt(aroi_attr->obj_life_th);
	printInt(aroi_attr->aspect_ratio);
	printInt(g_detect_interval);
	printInt(g_detect_interval);
}

int enableOd(MPI_WIN win, const MPI_IVA_OD_PARAM_S *od)
{
	int ret;

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
	printf("USAGE:\taroi_demo -i <CONFIG>\t\n"
	       "\t-i <file>\t\tROI config list in .json file\n"
	       "\t@min_roi_width\tMinimum roi width. (Default:192, unit:pixel)\n"
	       "\t@min_roi_height\tMinimum roi height. (Default:108, unit:pixel)\n"
	       "\t@max_roi_width\tMaximum roi width. (Default:1920, unit:pixel)\n"
	       "\t@max_roi_height\tMaximum roi width. (Default:1080, unit:pixel)\n"
	       "\t@aspect_ratio_width\tExpected Output ROI aspect ratio width. (Default:192)\n"
	       "\t@aspect_ratio_height\tExpected Output ROI aspect ratio height. (Default:108)\n"
	       "\t@\tNote. aspect ratio = aspect_ratio_width / aspect_ratio_height\n"
	       "\t@\t      if either aspect_ratio_width==0 or aspect_ratio_height==0\n"
	       "\t@\t      it means Expected output ROI does not follow fixed aspect ratio\n"
	       "\t@track_speed\tROI moving speed. (Default:32, unit:pixels per detection)\n"
	       "\t@return_speed\tROI moving speed. (Default:8, unit:pixels per detection)\n"
	       "\t@obj_life_th\tAROI will filter out all object life less than this threshold (Default:32, range 0-160),\n"
	       "\t@             object life represents confidence level of an object existing on screen\n"
	       "\t@Users select the target video window(substream), then PTZ module applies the result to target video window(substream)\n"
	       "\t\nOPTIONS:\n"
	       "\t-c <channel>\t\tSpecify which video channel to use. (Deatult 0).\n"
	       "\t-d <detect interval>\tSpecify which video detect interval. (Deatult 1).\n"
	       /* UINT8 od_qual; < Quality index of OD performance. */
	       "\t-q <value>\t\tSpecify OD quality index.[0-63] (Deatult 46).\n"
	       /* UINT8 od_sen; < sensitivity index of OD performance. */
	       "\t-s <channel>\t\tSpecify OD sensitivity.[0-255] (Deatult 254).\n"
	       "\n"
	       "For example:\n"
	       "\tmpi_stream -d /system/mpp/case_config/case_config_1001_FHD -precord_enable=1 -poutput_file=/dev/null -pframe_num=-1 &\n"
	       "\taroi_demo -i /system/mpp/aroi_config/aroi_conf_fixed_aspect_ratio.json &\n"
	       "\n"
	       "\t(Substream support PTZ)\n"
	       "\tmpi_stream -d /mnt/nfs/case_config_1006_FHD -p record_enable=1 -p output_file=/dev/null -p frame_num=-1 >> /dev/null &\n"
	       "\taroi_demo -i /system/mpp/aroi_config/aroi_conf_1080p_fixed_aspect_ratio_ptz.json &\n"
	       "\n"
	       "\t(Mainstream)\n"
#ifdef CONFIG_APP_AROI_SUPPORT_SEI
	       "\ttestOnDemandRTSPServer 0 -S\n"
#else
	       "\ttestOnDemandRTSPServer 0 -n\n"
#endif
	       "\t(Substream support PTZ)\n"
#ifdef CONFIG_APP_AROI_SUPPORT_SEI
	       "\ttestOnDemandRTSPServer 1 -S\n"
#else
	       "\ttestOnDemandRTSPServer 1 -n\n"
#endif
	       "\n");
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
	VIDEO_FTR_PTZ_PARAM_S ptz_attr;
	VFTR_AROI_PARAM_S *aroi_attr = &g_aroi_attr;

	while ((c = getopt(argc, argv, "i:c:w:d:q:s:h")) != -1) {
		switch (c) {
		case 'i':
			sprintf(cfg_file_name, "%s", optarg);
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
		case 'd':
			g_detect_interval = atoi(optarg);
			DBG("set detect interval:%d\n", g_detect_interval);
			break;
		case 'q':
			od_param.od_qual = atoi(optarg);
			DBG("set detect quality index:%s\n", optarg);
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

	if (signal(SIGPIPE, handleSigInt) == SIG_ERR) {
		perror("Cannot handle SIGINT!\n");
		exit(1);
	}

	if (strlen(cfg_file_name) == 0) {
		log_err("Config file path is not specified !");
		exit(1);
	}

	if (readJsonFromFile(cfg_file_name, aroi_attr, &ptz_attr)) {
		log_err("Cannot read json from file!");
		exit(1);
	}

	DBG("Read Json form file:%s\n", cfg_file_name);

	MPI_CHN chn = MPI_VIDEO_CHN(0, chn_idx);
	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_RECT_S winroi;

	/* Initialize MPI system */
	MPI_SYS_init();

	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel attribute. err: %d", ret);
		ret = -ENODEV;
		goto error;
	}

	/*  please check the following code */
	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video layout attribute. err: %d", ret);
		ret = -EFAULT;
		goto error;
	}

	/* NOTICE: we only support the case that only one window in a channel */
	assert(layout_attr.window_num < 8);

	/* set window index */
	g_win_idx = MPI_VIDEO_WIN(0, chn_idx, win_idx);

	ret = MPI_DEV_getWindowRoi(g_win_idx, &winroi);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get window roi. err: %d", ret);
		ret = -EFAULT;
		goto error;
	}

	/* Check parameters with window resolution */
	/* MPI_RECT_S window_resoln; */
	ret = VFTR_AROI_checkParam(aroi_attr, &chn_attr.res);
	if (ret) {
		log_err("Invalid AROI parameters");
		ret = -EINVAL;
		goto error;
	}

#ifdef CONFIG_APP_AROI_SUPPORT_SEI
	/* init AV server to wait RTSP client connect
	 * allow transfer IVA result to RTSP streaming server */
	/* NOTE: this step only for showing result from RTSP server
	 * related code is not needed
	 */
	ret = AVFTR_initServer();
	if (ret) {
		log_err("Failed to initialize AV server %d", ret);
		ret = -ENOPROTOOPT;
		goto error;
	}
#endif

	/* enableOd */
	ret = enableOd(g_win_idx, &od_param);
	if (ret < 0) {
		log_err("Failed to enable OD, please check if OD is enabled. [LINE]: %d", __LINE__);
	}

	/* crop chn resolution to fit winroi resolution */
	chn_attr.res.width = chn_attr.res.width * winroi.width / 1024;
	chn_attr.res.height = chn_attr.res.height * winroi.height / 1024;
	printf("input chn attr res: %d, %d\n", chn_attr.res.width, chn_attr.res.height);
	printf("pos x,y = %d, %d\n", layout_attr.window[win_idx].width, layout_attr.window[win_idx].height);

	/* crop layout to fit winroi rect */
	layout_attr.window[win_idx].x = layout_attr.window[win_idx].x * winroi.x / 1024;
	layout_attr.window[win_idx].y = layout_attr.window[win_idx].y * winroi.y / 1024;
	layout_attr.window[win_idx].width = layout_attr.window[win_idx].width * winroi.width / 1024;
	layout_attr.window[win_idx].height = layout_attr.window[win_idx].height * winroi.height / 1024;

	/* run aroi and apply to target window */
	ret = detectAroi(&layout_attr.window[win_idx], &winroi, g_win_idx, &chn_attr.res, aroi_attr, &ptz_attr);
	if (ret) {
		log_err("Unexpected error in aroi running loop!");
	}

	/* disable od */
	ret = MPI_IVA_disableObjDet(g_win_idx);

#ifdef CONFIG_APP_AROI_SUPPORT_SEI
	AVFTR_exitServer();
#endif

error:
	MPI_SYS_exit();

	return ret;
}
