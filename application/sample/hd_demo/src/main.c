#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_errno.h"
#include "mpi_index.h"
#include "mpi_iva.h"
#include "log.h"
#include "vftr.h"

#include "json.h"

#ifdef CONFIG_APP_HD_SUPPORT_SEI
#include "avftr_conn.h"
extern AVFTR_CTX_S *avftr_res_shm;
#else
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#include "eaif.h"
#include "hd_demo.h"

#define HD_TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#define HD_TOC(str, start)                                                                               \
	do {                                                                                             \
		struct timespec end;                                                                     \
		uint64_t delta_us;                                                                       \
		float delta_s;                                                                           \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                \
		delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; \
		delta_s = (float)delta_us / 1000000;                                                     \
		printf("%s Elapsed time: %.8f (s).\n", str, delta_s);                                    \
	} while (0)

struct timespec g_start_time;
EAIF_PARAM_S g_hd_param;
MPI_WIN g_win_idx;
MPI_IVA_OD_PARAM_S g_od_param;
MPI_RECT_POINT_S g_chn_bdry;
int g_hd_running;
HD_SCENE_PARAM_S g_hd_scene_param = { 0 };

#ifdef DEBUG
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

int g_restart = 0;
static void handleSigInt(int signo)
{
	DBG("[%s]leave %s\n", __func__, __FILE__);

	g_hd_running = 0;
	char *restartc = getenv("HD_RESTART");
	if (restartc)
		g_restart += atoi(restartc);

	if (g_restart >= 10)
		g_restart = 0;

	HD_TIC(g_start_time);

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

int parse_config(char *file_name, EAIF_PARAM_S *param, HD_SCENE_PARAM_S *scene_param)
{
	static const char *agtx_iva_eaif_data_fmt_e_map[] = { "JPEG",     "Y",     "YUV",     "RGB",
		                                              "MPI_JPEG", "MPI_Y", "MPI_YUV", "MPI_RGB" };
	static const char *agtx_iva_eaif_api_method_e_map[] = { "FACEDET", "FACERECO", "DETECT", "CLASSIFY", "CLASSIFY_CV",
		                                                "HUMAN_CLASSIFY" };

	struct json_object *tmp_obj, *cmd_obj, *tmp1_obj, *tmp2_obj, *tmp3_obj;
	int i;
	const char *str;

	cmd_obj = json_object_from_file(file_name);
	if (!cmd_obj) {
		fprintf(stderr, "Cannot open %s\n", file_name);
		return -EBADF;
	}

	if (json_object_object_get_ex(cmd_obj, "api", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (UINT32)i < sizeof(agtx_iva_eaif_api_method_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_eaif_api_method_e_map[i], str) == 0) {
				param->api = (EAIF_API_METHOD_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "classify_cv_model", &tmp_obj)) {
		i = MIN(EAIF_MODEL_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->classify_cv_model, json_object_get_string(tmp_obj), i);
		param->classify_cv_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "classify_model", &tmp_obj)) {
		i = MIN(EAIF_MODEL_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->classify_model, json_object_get_string(tmp_obj), i);
		param->classify_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "data_fmt", &tmp_obj)) {
		str = json_object_get_string(tmp_obj);
		for (i = 0; (UINT32)i < sizeof(agtx_iva_eaif_data_fmt_e_map) / sizeof(char *); i++) {
			if (strcmp(agtx_iva_eaif_data_fmt_e_map[i], str) == 0) {
				param->data_fmt = (EAIF_DATA_FMT_E)i;
				break;
			}
		}
	}
	if (json_object_object_get_ex(cmd_obj, "face_detect_model", &tmp_obj)) {
		i = MIN(EAIF_MODEL_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->face_detect_model, json_object_get_string(tmp_obj), i);
		param->face_detect_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "detect_model", &tmp_obj)) {
		i = MIN(EAIF_MODEL_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->detect_model, json_object_get_string(tmp_obj), i);
		param->detect_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "face_reco_model", &tmp_obj)) {
		i = MIN(EAIF_MODEL_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->face_reco_model, json_object_get_string(tmp_obj), i);
		param->face_reco_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "human_classify_model", &tmp_obj)) {
		i = MIN(EAIF_MODEL_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->human_classify_model, json_object_get_string(tmp_obj), i);
		param->human_classify_model[i] = '\0';
	}
	if (json_object_object_get_ex(cmd_obj, "neg_classify_period", &tmp_obj)) {
		param->neg_classify_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_exist_classify_period", &tmp_obj)) {
		param->obj_exist_classify_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "obj_life_th", &tmp_obj)) {
		param->obj_life_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_classify_period", &tmp_obj)) {
		param->pos_classify_period = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "pos_stop_count_th", &tmp_obj)) {
		param->pos_stop_count_th = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "snapshot_height", &tmp_obj)) {
		param->snapshot_height = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "snapshot_width", &tmp_obj)) {
		param->snapshot_width = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "target_idx", &tmp_obj)) {
		param->target_idx.value = 0;
		param->target_idx.chn = json_object_get_int(tmp_obj);
	}
	if (json_object_object_get_ex(cmd_obj, "url", &tmp_obj)) {
		i = MIN(EAIF_URL_CHAR_LEN, json_object_get_string_len(tmp_obj));
		strncpy((char *)param->url, json_object_get_string(tmp_obj), i);
		param->url[i] = '\0';
	}

	json_object_object_get_ex(cmd_obj, "roi", &tmp_obj);
	if (tmp_obj) {
		int roi_cnt = json_object_array_length(tmp_obj);
		scene_param->size = roi_cnt;
		for (int i = 0; i < roi_cnt; i++) {
			tmp1_obj = json_object_array_get_idx(tmp_obj, i);
			if (!tmp1_obj) {
				break;
			}
			HD_ROI_FILTER_S *roi = &scene_param->rois[i];
			if (json_object_object_get_ex(tmp1_obj, "start", &tmp2_obj)) {
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
				roi->rect.sx = (unsigned int)json_object_get_int(tmp3_obj);
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
				roi->rect.sy = json_object_get_int(tmp3_obj);
			}

			if (json_object_object_get_ex(tmp1_obj, "end", &tmp2_obj)) {
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
				roi->rect.ex = json_object_get_int(tmp3_obj);
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
				roi->rect.ey = json_object_get_int(tmp3_obj);
			}

			if (json_object_object_get_ex(tmp1_obj, "max", &tmp2_obj)) {
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
				roi->max.width = json_object_get_int(tmp3_obj);
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
				roi->max.height = json_object_get_int(tmp3_obj);
			}

			if (json_object_object_get_ex(tmp1_obj, "min", &tmp2_obj)) {
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 0);
				roi->min.width = json_object_get_int(tmp3_obj);
				tmp3_obj = json_object_array_get_idx(tmp2_obj, 1);
				roi->min.height = json_object_get_int(tmp3_obj);
			}
		}
	}

#define printInt(x) printf("[INFO] %s=%d\n", #x, (int)x)
#define printStr(x) printf("[INFO] %s=\"%s\"\n", #x, x)
	printInt(param->obj_life_th);
	printInt(param->target_idx.chn);
	printInt(param->api);
	printInt(param->data_fmt);
	printInt(param->snapshot_width);
	printInt(param->snapshot_height);
	printInt(param->pos_stop_count_th);
	printInt(param->pos_classify_period);
	printInt(param->neg_classify_period);
	printInt(param->obj_exist_classify_period);
	printStr(param->face_reco_model);
	printStr(param->detect_model);
	printStr(param->classify_model);
	printStr(param->classify_cv_model);
	printStr(param->human_classify_model);
	printInt(scene_param->size);
	if (scene_param->size) {
		for (int i = 0; i < scene_param->size; i++) {
			printf("[INFO] roi:%d [%d %d, %d %d] max:[%d %d] min:[%d %d]\n", i,
			       scene_param->rois[i].rect.sx, scene_param->rois[i].rect.sy, scene_param->rois[i].rect.ex,
			       scene_param->rois[i].rect.ey, scene_param->rois[i].max.width,
			       scene_param->rois[i].max.height, scene_param->rois[i].min.width,
			       scene_param->rois[i].min.height);
		}
	}
	return 0;
}

void help(void)
{
	printf("USAGE:\thd_demo -i <CONFIG>\t\n");
	printf("\t-i <file>\t\thuman detection config in .json file\n");
	printf("\t@obj_life_th\thd module filters out object with life less than this threshold. Range[0-160]\n");
	printf("\t@pos_classify_period\tClassify period there is a positive result for the object. Range[1-999].\n");
	printf("\t@neg_classify_period\tClassify period there is a negative result for the object. Range[1-999].\n");
	printf("\t@pos_stop_count_th\tStop classify for a object when it accumulates enough consecutive positive result. Range[1-99].\n");
	printf("\t@obj_exist_classify_period\tClassify period there is a positive object detected. Range[1-99].\n");
	printf("OPTIONS:\n");
	printf("\t-c <channel>\t\tSpecify which video channel to use. (Default 0).\n");
	/* UINT8 od_qual; < Quality index of OD performance. */
	printf("\t-q <value>\t\tSpecify OD quality index.[0-63] (Default 58).\n");
	/* UINT8 od_sen; < sensitivity index of OD performance. */
	printf("\t-s <sensitivity>\t\tSpecify OD sensitivity.[0-255] (Default 254).\n");
	printf("\n");
	printf("For example:\n");
	printf("\tmpi_stream -d /system/mpp/case_config/case_config_1001 -precord_enable=1 -poutput_file=/dev/null -pframe_num=-1 &\n");
	printf("\thd_demo -i /system/mpp/hd_config/hd_conf_inapp.json &\n");
#ifdef CONFIG_APP_HD_SUPPORT_SEI
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
	int chn_idx = 0;
	int win_idx = 0;
	char cfg_file_name[256] = { 0 };

	MPI_IVA_OD_PARAM_S *od_param = &g_od_param;
	*od_param = (MPI_IVA_OD_PARAM_S){
		.od_qual = 58, .od_track_refine = 42, .od_size_th = 25, .od_sen = 254, .en_stop_det = 1, .en_gmv_det = 0
	};

	EAIF_PARAM_S *hd_param = &g_hd_param;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
	*hd_param = (EAIF_PARAM_S){ .obj_life_th = 16,
		                    .target_idx = MPI_VIDEO_WIN(0, 0, 0),
		                    .api = EAIF_API_HUMAN_CLASSIFY,
		                    .data_fmt = EAIF_DATA_MPI_Y,
		                    .url = "inapp",
		                    .snapshot_width = 1280,
		                    .snapshot_height = 720,
		                    .pos_stop_count_th = 99,
		                    .pos_classify_period = 100,
		                    .neg_classify_period = 25,
		                    .obj_exist_classify_period = 0,
		                    .inf_with_obj_list = 0,
		                    .face_detect_model = "whatever",
		                    .face_reco_model = "whatever",
		                    .detect_model = "whatever",
		                    .classify_model = "whatever",
		                    .classify_cv_model = "whatever",
		                    .human_classify_model = "/system/eaif/models/classifiers/shuffleNetV2/inapp.ini",
		                    .inf_utils = { 0 } };
#pragma GCC diagnostic pop

	while ((c = getopt(argc, argv, "i:c:w:q:s:h")) != -1) {
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
		case 'q':
			od_param->od_qual = atoi(optarg);
			DBG("set detect quality index:%s\n", optarg);
			break;
		case 's':
			od_param->od_sen = atoi(optarg);
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

	if (strlen(cfg_file_name) == 0) {
		fprintf(stderr, "Config file path is not specified !\n");
		exit(1);
	}

	//readJsonFromFile(cfg_file_name);
	DBG("Read Json form file:%s\n", cfg_file_name);

	MPI_CHN_ATTR_S chn_attr;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN chn = MPI_VIDEO_CHN(0, chn_idx);
	MPI_WIN win = MPI_VIDEO_WIN(0, chn_idx, win_idx);

	/* Initialize MPI system */
	MPI_SYS_init();
	VFTR_init(NULL);

	ret = MPI_DEV_getChnAttr(chn, &chn_attr);
	if (ret != MPI_SUCCESS) {
		log_err("Failed to get video channel attribute. err: %d", ret);
		return -EINVAL;
	}

	/* TODO: please check the following code */
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

	g_chn_bdry =
	        (MPI_RECT_POINT_S){ .sx = 0, .sy = 0, .ex = chn_attr.res.width - 1, .ey = chn_attr.res.height - 1 };

	ret = parse_config(cfg_file_name, hd_param, &g_hd_scene_param);
	if (ret) {
		fprintf(stderr, "[ERROR] fail to parse config file\n");
		return -EINVAL;
	}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
	// init avftr serv
	ret = AVFTR_initServer();
	if (ret) {
		fprintf(stderr, "[ERROR] init avftr server fail\n");
		return -EINVAL;
	}
#endif

	do {
		printf("[INFO] starting HD\n");
		ret = runHumanDetection(win, hd_param);
		HD_TOC("[HD EXIT]", g_start_time);
	} while (g_restart);

#ifdef CONFIG_APP_HD_SUPPORT_SEI
	// exit avftr serv
	AVFTR_exitServer();
#endif

	VFTR_exit();
	MPI_SYS_exit();
	return 0;
}
