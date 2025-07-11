#include <assert.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <pthread.h>
#include <math.h>

#include "mpi_base_types.h"
#include "mpi_sys.h"
#include "mpi_dev.h"
#include "mpi_errno.h"
#include "mpi_index.h"
#include "mpi_iva.h"
#include "log.h"
#include "vftr.h"
#include "od_event.h"
#include "human_discriminator.h"

#include "json.h"
#include "utlist.h"

#ifdef CONFIG_APP_HD_SUPPORT_SEI
#include "avftr_conn.h"
extern AVFTR_CTX_S *avftr_res_shm;
#else
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

#include "eaif.h"
#include "hd_demo.h"

#define MAX_SERVICE_NUM (MIN(AVFTR_VIDEO_MAX_SUPPORT_NUM, VIDEO_OD_MAX_SUPPORT_NUM))

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

// #define DEBUG
#ifdef DEBUG
#define DBG(...) fprintf(stderr, __VA_ARGS__)
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
		DBG("Caught SIGINT!\n");
	} else if (signo == SIGTERM) {
		DBG("Caught SIGTERM!\n");
	} else if (signo == SIGKILL) {
		DBG("Caught SIGKILL\n");
	} else if (signo == SIGQUIT) {
		DBG("Caught SIGQUIT!\n");
	} else {
		DBG("Unexpected signal!\n");
	}
}

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
		fprintf(stderr, "The OD ctx doesn't exist.");
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
		} else if (emp_idx == -1 && (!ctx[i].en)) {
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
		fprintf(stderr, "The VFTR buffer ctx doesn't exist.");
		return -EINVAL;
	}
	memset(&info[info_idx], 0, sizeof(AVFTR_VIDEO_BUF_INFO_S));
	return 0;
}

int parse_config(char *file_name, EAIF_PARAM_S *param, HD_SCENE_PARAM_S *scene_param)
{
	static const char *agtx_iva_eaif_data_fmt_e_map[] = { "JPEG",     "Y",     "YUV",     "RGB",
		                                              "MPI_JPEG", "MPI_Y", "MPI_YUV", "MPI_RGB" };
	static const char *agtx_iva_eaif_api_method_e_map[] = { "FACEDET",  "FACERECO",    "DETECT",
		                                                "CLASSIFY", "CLASSIFY_CV", "HUMAN_CLASSIFY" };

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
	printf("USAGE:\teai_hd_demo -i <CONFIG>\t\n");
	printf("\t-i <file>\t\thuman detection config in .json file\n");
	printf("\t@obj_life_th\thd module filters out object with life less than this threshold. Range[0-160]\n");
	printf("\t@pos_classify_period\tClassify period there is a positive result for the object. Range[1-999].\n");
	printf("\t@neg_classify_period\tClassify period there is a negative result for the object. Range[1-999].\n");
	printf("\t@pos_stop_count_th\tStop classify for a object when it accumulates enough consecutive positive result. Range[1-99].\n");
	printf("\t@obj_exist_classify_period\tClassify period there is a positive object detected. Range[1-99].\n");
	printf("OPTIONS:\n");
	printf("\t-c <channel>\t\tSpecify which video channel to use, support multiple-input in quotation mark. (Default 0).\n");
	printf("\t-w <channel>\t\tSpecify which video window to use, support multiple-input in quotation mark. (Default 0).\n");
	/* UINT8 od_qual; < Quality index of OD performance. */
	printf("\t-q <value>\t\tSpecify OD quality index.[0-63] (Default 58).\n");
	/* UINT8 od_sen; < sensitivity index of OD performance. */
	printf("\t-s <sensitivity>\tSpecify OD sensitivity.[0-255] (Default 254).\n");
	printf("\t-S\t\t\tEnable OD stop detection. (Default disabled).\n");
	printf("\t-p\t\t\tShow priority settings.\n");
	printf("\t-1 <priority factor 1>\tDetection period after 1st inference in minute seconds. Range[1-] (Default: 300).\n");
	printf("\t-2 <priority factor 2>\tDetection period after 2nd inference in minute seconds. Range[1-] (Default: 600).\n");
	printf("\t-3 <priority factor 3>\tDetection period after 3rd inference in minute seconds. Range[1-] (Default: 1000).\n");
	printf("\n");
	printf("Example for single sensor:\n");
	printf("\tmpi_stream -d /system/mpp/case_config/case_config_1001 -precord_enable=1 -poutput_file=/dev/null -pframe_num=-1 &\n");
	printf("\teai_hd_demo -i /system/mpp/hd_config/hd_conf_inapp.json &\n");
#ifdef CONFIG_APP_HD_SUPPORT_SEI
	printf("\ttestOnDemandRTSPServer 0 -S\n");
#else
	printf("\ttestOnDemandRTSPServer 0 -n\n");
#endif
	printf("\n");
	printf("Example for multi sensor:\n");
	printf("\tmpi_stream -d /system/mpp/case_config/case_config_1004_FHD &\n");
	printf("\teai_hd_demo -i /system/mpp/hd_config/hd_conf_inapp.json -c \"0 1\" -w \"0 0\" &\n");
#ifdef CONFIG_APP_HD_SUPPORT_SEI
	printf("\ttestOnDemandRTSPServer 0 -S\n");
#else
	printf("\ttestOnDemandRTSPServer 0 -n\n");
#endif
	printf("\n");
}

static bool cancelSubscription()
{
	return g_hd_running == 0;
}

typedef struct event_node {
	OdEvent event;
	struct event_node *prev;
	struct event_node *next;
} EventNode;

static json_object *jsonDeepClone(json_object *obj)
{
	const char *serialized = json_object_to_json_string_ext(obj, JSON_C_TO_STRING_PLAIN);
	return json_tokener_parse(serialized);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"
static void dumpEvent(void *context, const OdEvent *event)
{
	(void)context;
	const MPI_RECT_POINT_S *rect = &event->rect;
	DBG("[%u] obj(%d)<%d>, rect: (%d, %d) - (%d, %d), mv: (%d, %d), attrs=%s\n", event->timestamp, event->id,
	    event->life, rect->sx, rect->sy, rect->ex, rect->ey, event->mv.x, event->mv.y,
	    json_object_to_json_string(event->attrs));
}
#pragma GCC diagnostic pop

typedef struct event_collector {
	EventNode **queue;
	pthread_mutex_t *queue_lock;
} EventCollector;

static void collectEvent(void *context, const OdEvent *event)
{
	EventCollector *collector = context;
	EventNode *new_node = malloc(sizeof(*new_node));
	new_node->event = *event;
	new_node->event.attrs = jsonDeepClone(event->attrs);
	pthread_mutex_lock(collector->queue_lock);
	DL_APPEND(*collector->queue, new_node);
	pthread_mutex_unlock(collector->queue_lock);
}

typedef struct od_collector_params {
	EaiOdContext *service;
	MPI_IVA_OD_PARAM_S *od_params;
} OdCollectorParams;

static void *startEAIService(void *arg)
{
	DBG("startEAIService\n");
	OdCollectorParams *params = arg;
	EAI_OD_publish(params->service, params->od_params, cancelSubscription);
	DBG("startEAIService done.\n");
	return 0;
}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
extern AVFTR_VIDEO_CTX_S *vftr_res_shm;

static int updateVftrBufferInfo(AVFTR_VIDEO_BUF_INFO_S *buf_info, uint32_t timestamp, uint32_t *previous_idx)
{
	if (previous_idx) {
		*previous_idx = buf_info->buf_cur_idx;
	}
	buf_info->buf_cur_idx = ((buf_info->buf_cur_idx + 1) % AVFTR_VIDEO_RING_BUF_SIZE);
	buf_info->buf_ready[buf_info->buf_cur_idx] = 0;
	buf_info->buf_time[buf_info->buf_cur_idx] = timestamp;
	buf_info->buf_cur_time = timestamp;
	return buf_info->buf_cur_idx;
}

static bool isHuman(json_object *attrs, float threshold, float *conf)
{
	json_object *hd;
	if (!json_object_object_get_ex(attrs, "hd", &hd)) {
		if (conf) {
			*conf = 0;
		}
		return false;
	}

	json_object *scores;
	if (!json_object_object_get_ex(hd, "scores", &scores)) {
		if (conf) {
			*conf = 0;
		}
		return false;
	}

	int qualified = 0;
	float max_conf = 0;
	int num_scores = json_object_array_length(scores);
	for (int i = 0; i < num_scores; ++i) {
		float score = (float)json_object_get_double(json_object_array_get_idx(scores, i));
		if (score >= threshold) {
			++qualified;
			max_conf = fmaxf(max_conf, score);
		}
	}

	if (qualified > num_scores / 2) {
		if (conf) {
			*conf = max_conf;
		}
		return true;
	}

	if (conf) {
		*conf = 0;
	}
	return false;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static bool populateObject(EventNode **queue, int od_idx, int buf_idx, uint32_t timestamp, float conf_threshold,
                           const char *label)
{
	VIDEO_OD_CTX_S *vftr_od_ctx = vftr_res_shm->od_ctx;

	if (vftr_od_ctx[od_idx].en) {
		VIDEO_FTR_OBJ_LIST_S *obj_list = &vftr_od_ctx[od_idx].ol[buf_idx];
		MPI_IVA_OBJ_LIST_S *list = &obj_list->basic_list;
		list->obj_num = 0;
		EventNode *node, *tmp;
		float conf;
		int i = 0;
		DBG("--- [%u] ---\n", timestamp);
		DL_FOREACH_SAFE(*queue, node, tmp)
		{
			if (i < MPI_IVA_MAX_OBJ_NUM && node->event.timestamp < timestamp) {
				// Check if there exist an event with identical id in the linked list, if yes, keep the latest one
				EventNode *search_node;
				DL_SEARCH_SCALAR(tmp, search_node, event.id, node->event.id);
				if (search_node && search_node != node && search_node->event.timestamp < timestamp) {
					DBG("%d(%u) removed\n", node->event.id, node->event.timestamp);
					DL_DELETE(*queue, node);
					json_object_put(node->event.attrs);
					free(node);
					continue;
				}

				++list->obj_num;
				list->timestamp = node->event.timestamp;
				list->obj[i].id = node->event.id;
				list->obj[i].rect = node->event.rect;
				list->obj[i].mv = node->event.mv;
				list->obj[i].life = node->event.life;

				if (isHuman(node->event.attrs, conf_threshold, &conf)) {
					snprintf(obj_list->obj_attr[i].cat, sizeof(obj_list->obj_attr[i].cat), "%s",
					         label);
					snprintf(obj_list->obj_attr[i].conf, sizeof(obj_list->obj_attr[i].conf), "%.3f",
					         conf);
				} else {
					obj_list->obj_attr[i].cat[0] = 0;
					obj_list->obj_attr[i].conf[0] = 0;
				}
				++i;
				DBG("%d(%u) \n", node->event.id, node->event.timestamp);
				DL_DELETE(*queue, node);
				json_object_put(node->event.attrs);
				free(node);
			} else if (node->event.timestamp >= timestamp) {
				DBG("+%d(%u) \n", node->event.id, node->event.timestamp);
			} else {
				DBG("%d(%u) removed for limited memory\n", node->event.id, node->event.timestamp);
				DL_DELETE(*queue, node);
				json_object_put(node->event.attrs);
				free(node);
			}
		}
		DBG("\n");

		return i > 0;
	}
	return false;
}
#pragma GCC diagnostic pop
#endif

int main(int argc, char **argv)
{
	int ret;
	int c;
	int chn_indices[MAX_SERVICE_NUM] = { 0 };
	int win_indices[MAX_SERVICE_NUM] = { 0 };
	int service_count = 0;
	char cfg_file_name[256] = { 0 };
	bool show_priority_factor = false;
	uint32_t pri_for_once = 0;
	uint32_t pri_for_twice = 0;
	uint32_t pri_for_steady = 0;

	MPI_IVA_OD_PARAM_S *od_param = &g_od_param;
	*od_param = (MPI_IVA_OD_PARAM_S){
		.od_qual = 58, .od_track_refine = 42, .od_size_th = 25, .od_sen = 254, .en_stop_det = 0, .en_gmv_det = 0
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

	int chn_cnt = 0, win_cnt = 0;

	while ((c = getopt(argc, argv, "i:c:w:q:s:Shp1:2:3:")) != -1) {
		switch (c) {
		case 'i':
			sprintf(cfg_file_name, "%s", optarg);
			DBG("input .json file:%s\n", cfg_file_name);
			break;
		case 'c': {
			// chn_idx = atoi(optarg);
			char *token = strtok(optarg, " ");
			while (token != NULL) {
				chn_indices[chn_cnt++] = atoi(token);
				token = strtok(NULL, " ");
			}
			// DBG("set device channel:%d\n", chn_idx);
			break;
		}
		case 'w': {
			// win_idx = atoi(optarg);
			char *token = strtok(optarg, " ");
			while (token != NULL) {
				win_indices[win_cnt++] = atoi(token);
				token = strtok(NULL, " ");
			}
			// DBG("set device window:%d\n", win_idx);
			break;
		}
		case 'q':
			od_param->od_qual = atoi(optarg);
			DBG("set detect quality index:%s\n", optarg);
			break;
		case 's':
			od_param->od_sen = atoi(optarg);
			DBG("set detect sensitivity:%s\n", optarg);
			break;
		case 'S':
			od_param->en_stop_det = 1;
			DBG("enable OD stop detection\n");
			break;
		case 'p':
			show_priority_factor = true;
			break;
		case '1':
			pri_for_once = atoi(optarg);
			break;
		case '2':
			pri_for_twice = atoi(optarg);
			break;
		case '3':
			pri_for_steady = atoi(optarg);
			break;
		case 'h':
			help();
			exit(0);
		default:
			abort();
		}
	}

	if (chn_cnt == 0) {
		if (win_cnt != 0) {
			fprintf(stderr, "[ERROR] Channel is not specified !\n");
			exit(1);
		}
	} else {
		if (win_cnt != 0 && win_cnt != chn_cnt) {
			fprintf(stderr, "[ERROR] Window settings is not aligned with channel settings!\n");
			exit(1);
		} else if (chn_cnt > MAX_SERVICE_NUM) {
			fprintf(stderr,
			        "[ERROR] Target channel number [%d] is larger than the maximum channel number [%d]!\n",
			        chn_cnt, MAX_SERVICE_NUM);
			exit(1);
		}
	}

	service_count = chn_cnt ? chn_cnt : 1; // default = chn 0 win 0

	DBG("Start %d service on:\n", service_count);
	for (int i = 0; i < service_count; ++i) {
		DBG("Chn %d Win %d\n", chn_indices[i], win_indices[i]);
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

	/* Initialize MPI system */
	MPI_SYS_init();
	VFTR_init(NULL);

	EaiHdContext *hd_services[MAX_SERVICE_NUM] = { 0 };
	EaiOdContext *od_services[MAX_SERVICE_NUM] = { 0 };
	MPI_WIN wins[MAX_SERVICE_NUM];
	static pthread_mutex_t queue_locks[MAX_SERVICE_NUM];
	EventNode *queues[MAX_SERVICE_NUM] = { 0 };
	InfModelCtx classifiers[MAX_SERVICE_NUM];
	EventCollector collectors[MAX_SERVICE_NUM];
	EaiOdEventPublisher *publishers[MAX_SERVICE_NUM] = { 0 };
	AVFTR_VIDEO_BUF_INFO_S *buf_infos[MAX_SERVICE_NUM] = { 0 };
	OdCollectorParams od_collector_params[MAX_SERVICE_NUM];
	int od_idx[MAX_SERVICE_NUM] = { 0 };
	pthread_t eai_service_thread[MAX_SERVICE_NUM];

	float conf_thresh;
	char *label;

	for (int i = 0; i < service_count; ++i) {
		MPI_CHN_ATTR_S chn_attr;
		MPI_CHN_LAYOUT_S layout_attr;
		MPI_CHN chn = MPI_VIDEO_CHN(0, chn_indices[i]);
		wins[i] = MPI_VIDEO_WIN(0, chn_indices[i], win_indices[i]);

		ret = MPI_DEV_getChnAttr(chn, &chn_attr);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to get video channel attribute on chn %d. err: %d", chn.chn, ret);
			return -EINVAL;
		}

		ret = MPI_DEV_getChnLayout(chn, &layout_attr);
		if (ret != MPI_SUCCESS) {
			fprintf(stderr, "Failed to get video layout attribute on chn %d. err: %d", chn.chn, ret);
			return -EINVAL;
		}

		/* NOTICE: we only support the case that only one window in a channel */
		assert(layout_attr.window_num == 1);
		assert(layout_attr.window[0].x == 0);
		assert(layout_attr.window[0].y == 0);
		assert(layout_attr.window[0].width == chn_attr.res.width);
		assert(layout_attr.window[0].height == chn_attr.res.height);

		g_chn_bdry = (MPI_RECT_POINT_S){
			.sx = 0, .sy = 0, .ex = chn_attr.res.width - 1, .ey = chn_attr.res.height - 1
		};

		ret = parse_config(cfg_file_name, hd_param, &g_hd_scene_param);
		if (ret) {
			fprintf(stderr, "[ERROR] fail to parse config file\n");
			return -EINVAL;
		}

#ifdef CONFIG_APP_HD_SUPPORT_SEI
		// init avftr serv
		static int init_AVFTR;
		if (!init_AVFTR) {
			ret = AVFTR_initServer();
			if (ret) {
				fprintf(stderr, "[ERROR] init avftr server fail\n");
				return -EINVAL;
			}
			init_AVFTR = 1;
		}
#endif
		queue_locks[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

#ifdef CONFIG_APP_HD_SUPPORT_SEI
		int empty_idx, set_idx, buf_info_idx;
		set_idx = findVftrBufCtx(wins[i], vftr_res_shm->buf_info, &empty_idx);

		if (set_idx >= 0) {
			buf_info_idx = set_idx;
		} else if (empty_idx >= 0) {
			buf_info_idx = empty_idx;
		} else {
			fprintf(stderr, "Failed to register vftr buffer ctx on win(%u, %u, %u).", wins[i].dev,
			        wins[i].chn, wins[i].win);
			return -ENOMEM;
		}
		DBG("CHN %d WIN %d assigned to buf_info_idx %d\n", wins[i].chn, wins[i].win, buf_info_idx);
		buf_infos[i] = &vftr_res_shm->buf_info[buf_info_idx];
		buf_infos[i]->idx = wins[i];
		buf_infos[i]->en = 1;

		/* OD information */
		VIDEO_OD_CTX_S *od_ctx;
		set_idx = findOdCtx(wins[i], vftr_res_shm->od_ctx, &empty_idx);
		if (set_idx >= 0) {
			fprintf(stderr, "OD is already registered on win(%u, %u, %u) set_idx:%d.", wins[i].dev,
			        wins[i].chn, wins[i].win, set_idx);
			od_ctx = &vftr_res_shm->od_ctx[set_idx];
			od_idx[i] = set_idx;
		} else {
			if (empty_idx < 0) {
				fprintf(stderr, "Failed to create OD on win(%u, %u, %u).", wins[i].dev, wins[i].chn,
				        wins[i].win);
				return -ENOMEM;
			}
			od_idx[i] = empty_idx;
			od_ctx = &vftr_res_shm->od_ctx[empty_idx];
			od_ctx->en = 1;
			od_ctx->en_shake_det = 0;
			od_ctx->en_crop_outside_obj = 0;
			od_ctx->idx = wins[i];
			od_ctx->cb = NULL;
			od_ctx->bdry = g_chn_bdry;
		}
		DBG("CHN %d WIN %d od_idx =  %d\n", wins[i].chn, wins[i].win, od_idx[i]);
#endif

		Inf_InitModel(&classifiers[i], hd_param->human_classify_model);
		conf_thresh = classifiers[i].info->conf_thresh.data[0];
		label = classifiers[i].info->labels.data[0];
		DBG("conf_thresh=%.3f, label=%s\n", conf_thresh, label);

		hd_services[i] = EAI_HD_create(wins[i], layout_attr.window[0].width, layout_attr.window[0].height,
		                               hd_param->snapshot_width, hd_param->snapshot_height, &classifiers[i]);
		EAI_HD_setPriorityFactor(hd_services[i], pri_for_once, pri_for_twice, pri_for_steady);
		if (show_priority_factor) {
			EAI_HD_getPriorityFactor(hd_services[i], &pri_for_once, &pri_for_twice, &pri_for_steady);
			fprintf(stderr, "Priority factor 1: %d, 2: %d, 3: %d", pri_for_once, pri_for_twice,
			        pri_for_steady);
		}

		collectors[i].queue = &queues[i];
		collectors[i].queue_lock = &queue_locks[i];

		publishers[i] = EAI_HD_getPublisher(hd_services[i]);
		EAI_OD_subscribe(publishers[i], dumpEvent, 0);
		EAI_OD_subscribe(publishers[i], collectEvent, &collectors[i]);
		od_services[i] = EAI_OD_create(wins[i]);
		EAI_HD_startServiceWith(hd_services[i], EAI_OD_getPublisher(od_services[i]));

		od_collector_params[i].service = od_services[i];
		od_collector_params[i].od_params = od_param;

		g_hd_running = 1;
		pthread_create(&eai_service_thread[i], 0, startEAIService, &od_collector_params[i]);
	}

	do {
		DBG("Starting HD\n");

		uint32_t timestamp;
		while (g_hd_running) {
			for (int i = 0; i < service_count; ++i) {
				int err = MPI_DEV_waitWin(wins[i], &timestamp, 1000);
				if (MPI_SUCCESS != err) {
					fprintf(stderr, "FAILED to waitWin for win: (%d, %d, %d). err: %d", wins[i].dev,
					        wins[i].chn, wins[i].win, err);
					g_hd_running = 0;
					break;
				}
#ifdef CONFIG_APP_HD_SUPPORT_SEI
				uint32_t prev_idx;
				int buf_idx = -1;
				buf_idx = updateVftrBufferInfo(buf_infos[i], timestamp, &prev_idx);
				DBG("CHN %d WIN %d buf_idx = %d prev_idx = %d \n", wins[i].chn, wins[i].win, buf_idx,
				    prev_idx);
				pthread_mutex_lock(&queue_locks[i]);
#ifdef DEBUG
				int queue_size;
				EventNode *element;
				DL_COUNT(queues[i], element, queue_size);
				DBG("[%d %u] queue size = %d.\n", buf_idx, timestamp, queue_size);
				if (queue_size == 0) {
					DBG("[%d %u] empty queue.\n", buf_idx, timestamp);
				}
#endif // DEBUG
				if (populateObject(&queues[i], od_idx[i], buf_idx, timestamp, conf_thresh, label)) {
					buf_infos[i]->buf_ready[buf_idx] = 1;
				}
				pthread_mutex_unlock(&queue_locks[i]);
#endif
			}
		}

		for (int i = 0; i < service_count; ++i) {
			pthread_join(eai_service_thread[i], 0);
			EAI_HD_stopService(hd_services[i]);
			EAI_OD_dispose(od_services[i]);
			EAI_HD_dispose(hd_services[i]);
			clearOdCtx(wins[i], vftr_res_shm->od_ctx);
#ifdef CONFIG_APP_HD_SUPPORT_SEI
			clearVftrBufCtx(wins[i], vftr_res_shm->buf_info);
#endif
		}
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
