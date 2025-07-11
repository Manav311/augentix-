#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
// for fake interface
#include "mpi_dev.h"
#include "mpi_iva.h"
// for src
#include "app.h"
#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"

#ifdef EAIF_INFERENCE_INAPP
#include "inf_types.h"
#endif

// for utils
#include "test_app.h"
#include "utils.h"

extern int g_eaif_verbose;
int g_capture = 0;

// for custom callback
extern void *runEaif(void *input);

EAIF_ALGO_STATUS_S g_eaif_ctx;
ObjListArray g_obj_list_arr = {};
EaifTestConfig g_test_config = {};
EaifCvCtx g_cv_config = {};
MPI_ECHN g_snapshot_chn_t;
MPI_BCHN g_bchn_t;

#define OD_JSON_FMT \
	"{\"obj\":{\"id\":%d,\"life\":%d,\"rect\":[%d,%d,%d,%d],\"mv\":[%d,%d],\"cat\":\"%s\",\"conf\":\"%s\"}},"

// copy from video_eaif.c
int genEaifDetect(const EAIF_STATUS_S *status, MPI_IVA_OBJ_LIST_S *ol, char *msg)
{
	int i;
	MPI_IVA_OBJ_ATTR_S *od = NULL;
	const EAIF_OBJ_ATTR_S *eobj = NULL;
	int size = 0;
	ol->obj_num = status->obj_cnt;
	for (i = 0; i < status->obj_cnt; i++) {
		eobj = &status->obj_attr[i];
		//obj = &ol->obj_attr[i];
		od = &ol->obj[i];
		od->id = eobj->id; // TBD how to maintain detection id ?
		od->mv = (MPI_MOTION_VEC_S){ 0, 0 }; // TBD how to get detection motion vector ?
		od->life = EAIF_MAX_OBJ_LIFE_TH; // TBD how to determine detection life ?
		od->rect = eobj->rect;
		size += sprintf(&msg[size], OD_JSON_FMT, eobj->id, od->life, eobj->rect.sx, eobj->rect.sy,
		                eobj->rect.ex, eobj->rect.ey, 0, 0, eobj->category[0], eobj->prob[0]);
	}
	return size;
}

// copy from video_eaif.c
int genEaifClassify(const EAIF_STATUS_S *status, MPI_IVA_OBJ_LIST_S *ol, char *msg)
{
	int i, j, k, set;
	int size = 0;
	MPI_IVA_OBJ_ATTR_S *od = NULL;
	const EAIF_OBJ_ATTR_S *eobj = NULL;
	for (i = 0; i < ol->obj_num; i++) {
		od = &ol->obj[i];
		set = 0;
		char cat_str[128] = {};
		char prob_str[128] = {};
		int cat_size = 0;
		int prob_size = 0;
		for (j = 0; j < status->obj_cnt; j++) {
			eobj = &status->obj_attr[j];
			if (od->id == eobj->id) {
				for (k = 0; k < eobj->label_num; ++k) {
					cat_size += sprintf(&cat_str[cat_size], "%s ", eobj->category[k]);
					prob_size += sprintf(&prob_str[prob_size], "%s ", eobj->prob[k]);
				}
				set = 1;
				break;
			}
		}
		if (set == 0) {
			cat_size += sprintf(&cat_str[cat_size], "NoMatch");
		}
		size += sprintf(&msg[size], OD_JSON_FMT, od->id, od->life, od->rect.sx, od->rect.sy, od->rect.ex,
		                od->rect.ey, od->mv.x, od->mv.y, cat_str, prob_str);
	}
	return size;
}

// copy from video_eaif.c
static void genEaifRes(EaifInfo *info, EAIF_STATUS_S *status, MPI_IVA_OBJ_LIST_S *ol, int frame_no, FILE *fw)
{
	char msg[2046] = {};
	int size = sprintf(msg, "{\"frame_no\":%d,\"timestamp\":%u,", frame_no, ol->timestamp);
	size += sprintf(&msg[size], "\"od\":[");
	const EAIF_PARAM_S *p = info->algo->p;

	if (status->server_reachable == EAIF_URL_REACHABLE) {
		if (p->api == EAIF_API_FACERECO || p->api == EAIF_API_CLASSIFY ||
		    p->api == EAIF_API_CLASSIFY_CV || p->api == EAIF_API_HUMAN_CLASSIFY) {
			size += genEaifClassify(status, ol, &msg[size]);
		} else if (p->api == EAIF_API_DETECT || p->api == EAIF_API_FACEDET) {
			size += genEaifDetect(status, ol, &msg[size]);
		}
	} else {
		for (int i = 0; i < ol->obj_num; i++) {
			size = sprintf(
			        &msg[size],
			        "{\"obj\":[\"id\":%d,\"life\":%d,\"rect\":[%d,%d,%d,%d],\"cat\":\"\",\"prob\":\"\"},",
			        ol->obj[i].id, ol->obj[i].life, ol->obj[i].rect.sx, ol->obj[i].rect.sy,
			        ol->obj[i].rect.ex, ol->obj[i].rect.ey);
		}
	}
	if (ol->obj_num)
		size--;
	size = sprintf(&msg[size], "]}\n");
	fprintf(fw, "%s", msg);
}

int testRemoteSetDataPayload(const EAIF_PARAM_S *param, EaifRequestDataRemote *payload)
{
	int data_size = 0;
	unsigned char *data = 0;

	strncpy(payload->data.name, EAIF_STR_DATA, EAIF_CHAR_LEN);

	EaifCvCtx *cv_info = &g_cv_config;
	data_size = CV_utils_EnodeJpegImage(&cv_info->image, &data);

	payload->data.size = data_size;
	payload->data.ptr = (void *)data;
	return 0;
}

int testGetInfImage(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifCvCtx *cv_info = &g_cv_config;
	EaifRequestDataInApp *payload = instance->info.payload.inapp;
	payload->size = 0;
	pthread_mutex_lock(&instance->lock);

#ifdef EAIF_INFERENCE_INAPP

	InfImage *image = &payload->inf_image;
	if (image->data) {
		free(image->data);
		image->data = 0;
	}
	image->w = cv_info->image.w;
	image->h = cv_info->image.h;
	image->c = cv_info->image.c;
	image->dtype = cv_info->image.dtype;
	image->data = malloc(1 * image->w * image->h * image->c);
	memcpy(image->data, cv_info->image.data, image->w * image->h * image->c * sizeof(char));
	payload->size = image->w * image->h * image->c;

#endif // EAIF_INFERENCE_INAPP

	pthread_mutex_unlock(&instance->lock);

	return payload->size;
}

char g_dirname[256];
int g_msleep = 0;

static int testDebug(void *ctx)
{
#if 0 //EAIF_INFERENCE_INAPP
	EAIF_ALGO_STATUS_S *ctx = (EAIF_ALGO_STATUS_S*)ctx;
	EaifInfo *info = &ctx->info;
	EaifCvCtx *cv_info = &g_cv_config;
	const MPI_IVA_OBJ_LIST_S *obj_list = &info->payload.inapp->obj_list;
	const EaifStatusInternal *status = &info->local_status;
	if (g_capture) {
		int i = obj_list->timestamp;
		for (int j = 0; j < obj_list->obj_num; j++) {
			char img_path[256] = {};
			const MPI_IVA_OBJ_ATTR_S *obj = &obj_list->obj[j];
			for (int k = 0; k < status->obj_cnt; k++) {
				if (status->obj_attr_ex[k].basic.id == obj->id) {
					if (status->obj_attr_ex[k].basic.label_num)
						sprintf(img_path, "%s/%05d-%03d-%s.jpg", g_dirname, i, j,
						        status->obj_attr_ex[k].basic.category[0]);
					else
						sprintf(img_path, "%s/%05d-%03d-negative.jpg", g_dirname, i, j);
					//printf("LINE 255 ti:%d obj num:%d vs status cnt:%d t:%d %d -> %s\n", i, obj_list->obj_num,
					//	status->obj_cnt, i, j, img_path);

					CV_utils_dumpRectJpeg(&cv_info->image, obj->rect.sx, obj->rect.sy, obj->rect.ex,
					                      obj->rect.ey, img_path);
					break;
				}
			}
		}
	}
	int u_sleep = g_msleep * 100;
	usleep(u_sleep);
#endif
	return 0;
}

int eaif_test_enable(EAIF_ALGO_STATUS_S *ctx, const EaifTestConfig *c, const ObjListArray *ols)
{
	EAIF_INSTANCE_S instance = { ctx };
	ctx->running = 0;
	//ctx->fill_payload_cb = eaif_test_setupPayload;
	ctx->info.src_resoln = (MPI_SIZE_S){ .width = c->width, .height = c->height };
	EaifCvCtx *cv_info = &g_cv_config;
	EaifInfo *info = &ctx->info;
	const ObjListArray *obj_lists = ols;

	int ret = 0;
	initAppCb(ctx);

	if (info->mode == EAIF_MODE_REMOTE)
		ctx->set_payload = testRemoteSetPayload;
	else if (ctx->param.api == EAIF_API_FACERECO) // inapp face reco
		ctx->set_payload = testFaceRecoInappSetPayload;
	else // inapp other
		ctx->set_payload = testInappSetPayload;

	ctx->debug = testDebug;
	ctx->init_module(ctx);
	ctx->init_algo(ctx);

	char *debug = getenv("INF_CAP_PREFIX");
	if (debug) {
		Inf_Setup(&ctx->model.ctx, 1, 1, 4);
	}

	cv_info->vid = CV_utils_OpenVideo(c->video_input_file);
	ctx->running = 1;
	info->req_sta.val = 0;
	cv_info->is_running = 1;
	MPI_WIN idx = ctx->idx;

	if (pthread_create(&ctx->tid_eaif, NULL, runEaif, (void *)ctx) != 0) {
		eaif_log_warn("Cannot create EAIF thread for window 0x%x.", idx.value);
		exit(0);
	}

	int try_time = 0;
	int try_time_max = 10;
	if (ctx->status.server_reachable == EAIF_URL_NONREACHABLE) {
		usleep(1000 * 1000);
		if (try_time++ > try_time_max) {
			eaif_log_warn("Server connection timeout!");
			exit(0);
		}
	}

	int sleep_time = 1000000 / c->video_fps;
	int num_frames = c->num_frames;
	int i = 0;
	EAIF_STATUS_S status = {};
	char output_log[256] = {};
	int len = strlen(c->eaif_objList_file);
	strcpy(output_log, c->eaif_objList_file);
	output_log[len - 3] = 'l';
	output_log[len - 2] = 'o';
	output_log[len - 1] = 'g';

	cv_info->fw = fopen(output_log, "w");
	if (!cv_info->fw) {
		eaif_log_info("Cannot open log file %s", output_log);
		exit(0);
	}

	if (num_frames != obj_lists->size) {
		eaif_log_warn("Number of frames: %d vs objlist size: %d are different!", num_frames, obj_lists->size);
	}

	//if (g_capture) { disable!
	if (0) {
		strcpy(g_dirname, output_log);
		g_dirname[len - 4] = 0;
		if (access(g_dirname, 0) != 0)
			mkdir(g_dirname, 0755);
	}

	while (cv_info->is_running && i < num_frames) {
		pthread_mutex_lock(&ctx->lock);
		CV_utils_GetFrame(&cv_info->vid, &cv_info->image, c->channel);
		if (ctx->param.snapshot_width && ctx->param.snapshot_height)
			CV_utils_ResizeFrame(&cv_info->image, ctx->param.snapshot_width, ctx->param.snapshot_height);
		//printf("[INFO] image info %dx%dx%d %d data:%p\n",
		//        cv_info->image.h, cv_info->image.w,  cv_info->image.c,  cv_info->image.dtype, cv_info->image.data);
		pthread_mutex_unlock(&ctx->lock);
		MPI_IVA_OBJ_LIST_S ol = obj_lists->obj_list[i];
		ol.timestamp = i;
		//printf("LINE 343 ti:%d obj num:%d\n", ol.timestamp, ol.obj_num);
		EAIF_testRequest(&instance, &ol, &status);
		genEaifRes(&ctx->info, &status, &ol, i, cv_info->fw);
		if ((i % 20) == 0)
			fprintf(stderr, "Process:%d/%d\n", i, num_frames);
		usleep(sleep_time);
		i++;
	}
	fprintf(stderr, "Process:%d/%d\n", i, num_frames);
	ret = pthread_cancel(ctx->tid_eaif);
	if (ret != 0) {
		eaif_log_err("Cancel thread to run eaif failed.");
		return MPI_FAILURE;
	}
	int *res = 0;
	ret = pthread_join(ctx->tid_eaif, (void **)&res);
	if (ret != 0) {
		if (res)
			free(res);
		eaif_log_err("Join thread to run eaif failed.");
		return MPI_FAILURE;
	}

	fclose(cv_info->fw);
	CV_utils_destroyFrame(&cv_info->image);
	CV_utils_closeVideo(&cv_info->vid);
	ctx->exit_module(ctx);
	ctx->release_buffer(ctx);
	delAppCb(ctx);
	eaif_log_notice("Exit eaif thread!");
	return 0;
}

int runOneVideo(EAIF_ALGO_STATUS_S *ctx, const EaifTestConfig *c, const ObjListArray *ols)
{
	eaif_test_enable(ctx, c, ols);
	return 0;
}

void parseEaifObjList(const char *obj_list_file, ObjListArray *arr)
{
	FILE *fp = fopen(obj_list_file, "rb");
	if (!fp) {
		eaif_log_info("Cannot open %s", obj_list_file);
		exit(0);
	}
	fseek(fp, 0, SEEK_END);
	int fsize = ftell(fp);
	int size = sizeof(MPI_IVA_OBJ_LIST_S) * arr->size;
	if (fsize != size) {
		eaif_log_info("Expected file size, fsize (%d) not matched vs arr size (%d)!\n",
		              (int)fsize / (int)sizeof(MPI_IVA_OBJ_LIST_S), (int)arr->size);
		fclose(fp);
		exit(0);
	}
	rewind(fp);
	arr->obj_list = malloc(size);
	int ret __attribute__((unused));
	ret = fread(arr->obj_list, 1, size, fp);
	fclose(fp);
}

void parseEaifParam(const char *param_file, EAIF_PARAM_S *p)
{
	FILE *fp = fopen(param_file, "r");
	if (!fp) {
		eaif_log_info("Cannot open %s", param_file);
		exit(0);
	}

	int obj_life_th;
	int idx_dev, idx_chn, idx_win;
	int snapshot_width;
	int snapshot_height;
	int obj_exist_classify_period = 0;
	char api[32] = {};
	char data_fmt[16] = {};
	char url[128] = {};
	int pos_stop_count_th = 0, pos_classify_period = 0, neg_classify_period = 0, detection_period = 0, identification_period = 0;
	int inf_with_obj_list = 0;
	char face_detect_model[64];
	char face_reco_model[64];
	char detect_model[64];
	char classify_model[64];
	char classify_cv_model[64];
	char human_classify_model[64];
	char face_dir[64];
	char face_db[64];
	int msleep = 0;

	if ((fscanf(fp, "obj_life_th=%d\n", &obj_life_th) != 1) ||
	    (fscanf(fp, "target_idx=%d,%d,%d\n", &idx_dev, &idx_chn, &idx_win) != 3) ||
	    (fscanf(fp, "snapshot_width=%d\n", &snapshot_width) != 1) ||
	    (fscanf(fp, "snapshot_height=%d\n", &snapshot_height) != 1) || (fscanf(fp, "api=%s\n", api) != 1) ||
	    (fscanf(fp, "data_fmt=%s\n", data_fmt) != 1) || (fscanf(fp, "url=%s\n", url) != 1) ||
	    (fscanf(fp, "pos_stop_count_th=%d\n", &pos_stop_count_th) != 1) ||
	    (fscanf(fp, "pos_classify_period=%d\n", &pos_classify_period) != 1) ||
	    (fscanf(fp, "neg_classify_period=%d\n", &neg_classify_period) != 1) ||
	    (fscanf(fp, "obj_exist_classify_period=%d\n", &obj_exist_classify_period) != 1) ||
	    (fscanf(fp, "detection_period=%d\n", &detection_period) != 1) ||
	    (fscanf(fp, "identification_period=%d\n", &identification_period) != 1) ||
	    (fscanf(fp, "inf_with_obj_list=%d\n", &inf_with_obj_list) != 1) ||
	    (fscanf(fp, "face_detect_model=%s\n", face_detect_model) != 1) ||
	    (fscanf(fp, "face_reco_model=%s\n", face_reco_model) != 1) ||
	    (fscanf(fp, "detect_model=%s\n", detect_model) != 1) ||
	    (fscanf(fp, "classify_model=%s\n", classify_model) != 1) ||
	    (fscanf(fp, "classify_cv_model=%s\n", classify_cv_model) != 1) ||
	    (fscanf(fp, "human_classify_model=%s\n", human_classify_model) != 1) ||
	    (fscanf(fp, "msleep=%d\n", &msleep) != 1) || (fscanf(fp, "face_dir=%s\n", face_dir) != 1) ||
	    (fscanf(fp, "face_db=%s\n", face_db) != 1)) {
		eaif_log_info("Config format of (%s) is not correct!", param_file);
		fclose(fp);
		exit(0);
	}

	fclose(fp);
	fp = NULL;

	p->obj_life_th = obj_life_th;
	p->target_idx = MPI_VIDEO_WIN(idx_dev, idx_chn, idx_win);
	p->snapshot_width = snapshot_width;
	p->snapshot_height = snapshot_height;
	strcpy(p->url, url);
	p->pos_stop_count_th = pos_stop_count_th;
	p->pos_classify_period = pos_classify_period;
	p->neg_classify_period = neg_classify_period;
	p->detection_period = detection_period;
	p->identification_period = identification_period;
	p->inf_with_obj_list = inf_with_obj_list;
	p->obj_exist_classify_period = obj_exist_classify_period;
	strcpy(p->face_detect_model, face_detect_model);
	strcpy(p->face_reco_model, face_reco_model);
	strcpy(p->detect_model, detect_model);
	strcpy(p->classify_model, classify_model);
	strcpy(p->classify_cv_model, classify_cv_model);
	strcpy(p->human_classify_model, human_classify_model);
	strcpy(p->inf_utils.dir, face_dir);
	strcpy(p->inf_utils.face_db, face_db);

	if (!strcmp(api, "facedet"))
		p->api = EAIF_API_FACEDET;
	else if (!strcmp(api, "facereco"))
		p->api = EAIF_API_FACERECO;
	else if (!strcmp(api, "detect"))
		p->api = EAIF_API_DETECT;
	else if (!strcmp(api, "classify"))
		p->api = EAIF_API_CLASSIFY;
	else if (!strcmp(api, "classify_cv"))
		p->api = EAIF_API_CLASSIFY_CV;
	else if (!strcmp(api, "human_classify"))
		p->api = EAIF_API_HUMAN_CLASSIFY;
	else {
		eaif_log_info("API method (%s) not supported yet!", api);
		exit(0);
	}

	if (!strcmp(data_fmt, "jpg"))
		p->data_fmt = EAIF_DATA_JPEG;
	else if (!strcmp(data_fmt, "mpi_jpg"))
		p->data_fmt = EAIF_DATA_MPI_JPEG;
	else if (!strcmp(data_fmt, "y__"))
		p->data_fmt = EAIF_DATA_Y;
	else if (!strcmp(data_fmt, "mpi_y__"))
		p->data_fmt = EAIF_DATA_MPI_Y;
	else if (!strcmp(data_fmt, "mpi_rgb"))
		p->data_fmt = EAIF_DATA_MPI_RGB;
	else if (!strcmp(data_fmt, "rgb"))
		p->data_fmt = EAIF_DATA_RGB;
	else {
		eaif_log_info("Data format (%s) not supported yet!", data_fmt);
		exit(0);
	}

	g_msleep = msleep;
}

void parseEaifTestConfig(const char *test_config_file, EaifTestConfig *c)
{
	FILE *fp = fopen(test_config_file, "r");
	if (!fp) {
		eaif_log_info("Cannot open %s", test_config_file);
		exit(0);
	}

	if ((fscanf(fp, "eaif_param_file=%s\n", c->eaif_param_file) != 1) ||
	    (fscanf(fp, "eaif_objList_file=%s\n", c->eaif_objList_file) != 1) ||
	    (fscanf(fp, "video_input_file=%s\n", c->video_input_file) != 1) ||
	    (fscanf(fp, "video_fps=%f\n", &c->video_fps) != 1) ||
	    (fscanf(fp, "resoln=%dx%d\n", &c->width, &c->height) != 2) ||
	    (fscanf(fp, "num_frames=%d\n", &c->num_frames) != 1)) {
		eaif_log_info("Config format of (%s) is not correct!", test_config_file);
		fclose(fp);
		exit(0);
	}
	fclose(fp);
}

void printHelp(const char *msg)
{
	printf("\n"
	       "\t\t%s\n\n"
	       "\tUsage eaif_test.elf\n"
	       "\t	./eaif_test.elf [video-test-conf] [capture]\n"
	       "\t  please also specify model latency \"msleep\" in config\\eaif.param\n",
	       msg);
}

int main(int argc, char **argv)
{
	if ((argc == 2 || argc == 3)!=1) {
		printHelp("input args not equal to 2 or 3!");
		return 0;
	}

	if (argc == 2 && !strcmp(argv[1], "-h")) {
		printHelp("input args not equal to 2 or 3!");
		return 0;
	}

	if (argc == 3)
		g_capture = atoi(argv[2]);

	const char *test_config = argv[1];
	EAIF_PARAM_S *p = &g_eaif_ctx.param;

	ObjListArray *obj_list_arr = &g_obj_list_arr;
	EaifTestConfig *c = &g_test_config;
	EAIF_ALGO_STATUS_S *ctx = &g_eaif_ctx;
	ctx->idx = MPI_VIDEO_WIN(0, 0, 0);

	parseEaifTestConfig(test_config, c);
	parseEaifParam(c->eaif_param_file, p);
	obj_list_arr->size = c->num_frames;
	if (p->data_fmt == EAIF_DATA_MPI_Y || p->data_fmt == EAIF_DATA_Y) {
		c->channel = 1;
	} else {
		c->channel = 3;
	}
	ctx->info.mode = eaif_utilsDetermineMode(p->url);
	printf("Input image channe is %d Eaif inference Mode %s\n", c->channel, eaif_utilsGetModeChar(ctx->info.mode));

	if (ctx->info.mode == EAIF_MODE_INAPP) {
		strncpy(ctx->model.path, p->api_models[p->api], EAIF_MODEL_LEN);
	}
	parseEaifObjList(c->eaif_objList_file, obj_list_arr);
	runOneVideo(ctx, c, obj_list_arr);
	return 0;
}
