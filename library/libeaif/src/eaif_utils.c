#include "eaif_utils.h"

#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <libgen.h>

#include "mpi_base_types.h"
#include "mpi_dev.h"
#include "mpi_enc.h"
#include "mpi_index.h"

#include "eaif.h"
#include "eaif_log.h"
#include "eaif_algo.h"

#ifdef EAIF_SUPPORT_JSON
#include "json.h"
#endif /* EAIF_SUPPORT_JSON */

#ifdef EAIF_SUPPORT_CURL
#include "curl/curl.h"
#endif /* !EAIF_SUPPORT_CURL */

extern int g_eaif_verbose;
extern int g_eaif_lite_debug;

const char *eaif_utilsGetModeChar(EAIF_INFERENCE_MODE_E mode)
{
	if (mode == EAIF_MODE_REMOTE)
		return "INFERENCE REMOTE";
	else if (mode == EAIF_MODE_INAPP)
		return "INFERENCE INAPP";
	return "INFERENCE UNKNOWN TYPE";
}

EAIF_INFERENCE_MODE_E eaif_utilsDetermineMode(const char *str)
{
	return (strncmp(str, EAIF_INFERENCE_INAPP_STR, 5) == 0) ? EAIF_MODE_INAPP : EAIF_MODE_REMOTE;
}

static int isFileExist(const char *filename)
{
	struct stat buf;
	/* Get File Statistics for stat.c. */
	if (stat(filename, &buf) != 0) {
		return 0;
	}
	return 1;
}

int eaif_assignFrameInfo(int fr_width, int fr_height, MPI_VIDEO_FRAME_INFO_S *frame_info)
{
	if (!frame_info)
		return -EFAULT;
	//MPI_SIZE_S resoln;
	//getMpiSize(p->target_idx, &resoln);
	frame_info->width = fr_width;
	frame_info->height = fr_height;
	return 0;
}

int eaif_calcScaleFactor(int dst_width, int dst_height, const MPI_SIZE_S *src, EaifFixedPointSize *scale_factor)
{
	if (!src || !scale_factor)
		return -EFAULT;
	scale_factor->width = (dst_width << EAIF_FIXED_POINT_BS) / src->width;
	scale_factor->height = (dst_height << EAIF_FIXED_POINT_BS) / src->height;
	return 0;
}

static inline int eaif_utilsCheckIfModelPathValid(const EAIF_PARAM_S *param)
{
	const char *model_path = param->api_models[param->api];
	return access(model_path, F_OK) == 0;
}

static int checkWinState(const MPI_WIN idx)
{
	INT32 ret;
	INT32 i;
	MPI_CHN_LAYOUT_S layout_attr;
	MPI_CHN_STAT_S chn_stat;
	MPI_CHN chn = { { .dev = idx.dev, .chn = idx.chn } };

	ret = MPI_DEV_queryChnState(chn, &chn_stat);
	if (ret != 0)
		return -1;

	if (!MPI_STATE_IS_ADDED(chn_stat.status))
		return -1;

	ret = MPI_DEV_getChnLayout(chn, &layout_attr);
	if (ret != 0)
		return -1;

	/* FIXME: check window state */
	for (i = 0; i < layout_attr.window_num; i++) {
		if (idx.value == layout_attr.win_id[i].value) {
			break;
		}
	}
	if (i == layout_attr.window_num)
		return -1;

	return 0;
}

int mkdirp_helper(char *path, int *err)
{
	if (access(path, F_OK) != 0) {
		char local_path[256] = {};
		strcpy(local_path, path);
		char *dir = dirname(local_path);
		mkdirp_helper(dir, err);
		if (*err == EEXIST)
			return 0;
		*err = mkdir(path, 0777);
		if (*err == 0)
			chmod(path, 0777);
	}
	return 0;
}

int mkdirp(const char *path, int *err)
{
	char local_path[256] = {};
	strcpy(local_path, path);
	return mkdirp_helper(local_path, err);
}

int eaif_utilsCheckParam(const EAIF_PARAM_S *param)
{
	EAIF_INFERENCE_MODE_E mode = eaif_utilsDetermineMode(param->url);

#ifndef EAIF_INFERENCE_INAPP
	if (mode == EAIF_MODE_INAPP) {
		eaif_log_err("INAPP mode is invalid in this build!");
		return -EINVAL;
	}
#endif

#ifndef EAIF_SUPPORT_CURL
	if (mode == EAIF_MODE_REMOTE) {
		eaif_log_err("REMOTE mode is invalid in this build!");
		return -EINVAL;
	}
#endif

	if (param->obj_life_th > EAIF_MAX_OBJ_LIFE_TH) {
		eaif_log_err("Object life threshold %d exceeds the boundary.", param->obj_life_th);
		return -EINVAL;
	}

	/* TODO add check param */
	if (param->api >= EAIF_API_MODEL_NUM) {
		eaif_log_warn("API method %d not supported.", param->api);
		return -EINVAL;
	}

	if (mode == EAIF_MODE_INAPP) {
		if ((param->api == EAIF_API_CLASSIFY || param->api == EAIF_API_HUMAN_CLASSIFY ||
		     param->api == EAIF_API_DETECT || param->api == EAIF_API_FACEDET ||
		     param->api == EAIF_API_FACERECO) != 1) {
			eaif_log_err("API method not supported for in-app built.");
			return -EINVAL;
		}

		if (!isFileExist(param->api_models[param->api])) {
			eaif_log_err("Model config %s does not found!", param->api_models[param->api]);
			return -ENOENT;
		}
	} else { // !EAIF_MODE_INAPP
		if ((param->data_fmt == EAIF_DATA_JPEG || param->data_fmt == EAIF_DATA_Y ||
		     param->data_fmt == EAIF_DATA_MPI_JPEG || param->data_fmt == EAIF_DATA_MPI_Y) != 1) {
			eaif_log_err("Data format %d is not supported.", param->data_fmt);
			return -EINVAL;
		}
	}

	if (checkWinState(param->target_idx)) {
		eaif_log_err("Fail to access target video window!");
		return -ENODEV;
	}

	if (mode == EAIF_MODE_INAPP && param->api == EAIF_API_FACERECO) {
		if (access(param->inf_utils.dir, F_OK) != 0) {
			eaif_log_warn("Create path for facereco \"%s\"", param->inf_utils.dir);
			int err = 0;
			int ret = mkdirp(param->inf_utils.dir, &err);
			if (!(err == 0 || err == EEXIST)) {
				eaif_log_warn("Cannot create path! err: %d", ret);
				return ret;
			}
			char dir[256] = {};
			sprintf(dir, "%s/faces", param->inf_utils.dir);
			ret = mkdirp(dir, &err);
			if (!(err == 0 || err == EEXIST)) {
				eaif_log_warn("Cannot create path! err: %d", ret);
				return ret;
			}
		}
	}

	return 0;
}

void eaif_utilsSetUrl(EAIF_INFERENCE_MODE_E mode, const EAIF_PARAM_S *param, char *url)
{
	if (mode == EAIF_MODE_REMOTE)
		sprintf(url, EAIF_STR_API_FMT, param->url, param->api_models[param->api]);
	else
		strcpy(url, param->api_models[param->api]);
	return;
}

int eaif_utilsTesturl(const char *url, struct eaif_status_internal_s *status)
{
#ifdef EAIF_SUPPORT_CURL
	CURL *curl;
	CURLcode res;
	//CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl == NULL) {
		eaif_log_warn("Cannot init curl!");
		curl_global_cleanup();
		return -1;
	}
	/* Setup test server url */
	char iurl[128] = { 0 };
	sprintf(iurl, "%s", url);

	/* Setup curl header */
	curl_easy_setopt(curl, CURLOPT_URL, iurl);
	curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(curl, CURLOPT_NOBODY, 1);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, EAIF_TIMEOUT_UNIT);
	res = curl_easy_perform(curl);
	if (res == CURLE_OK) {
		status->server_reachable = EAIF_URL_REACHABLE;
	} else {
		eaif_log_warn("curl_easy_perform() failed: %s %d on %s", curl_easy_strerror(res), res, url);
		status->server_reachable = EAIF_URL_NONREACHABLE;
	}
	curl_easy_cleanup(curl);
	curl_global_cleanup();
#else
	// Unused parameters.
	(void)url;
	(void)status;
#endif /* !EAIF_SUPPORT_CURL */
	return 0;
}

int eaif_getImageDataJpeg(MPI_ECHN chn, unsigned char *data)
{
#define WAIT_FOREVER (-1)
	eaif_log_debug("Taking snapshot of encoder channel (c=%d)", chn);
	MPI_STREAM_PARAMS_S stream_param = { .seg = { { 0 } },
		                             .seg_cnt = 0,
		                             .frame_id = 0,
		                             .timestamp = 0,
		                             .win_timestamp = { 0 },
		                             .jiffies = 0,
		                             .win_jiffies = { 0 } };
	int data_size = 0;

	uint32_t err = MPI_ENC_getChnFrame(chn, &stream_param, WAIT_FOREVER);
	if (err != MPI_SUCCESS) {
		MPI_ENC_releaseChnFrame(chn, &stream_param);
		eaif_log_err("Failed to take snapshot. err: %d", err);
		return data_size;
	}

	for (int i = 0; (UINT32)i < stream_param.seg_cnt; i++) {
		memcpy(&data[data_size], stream_param.seg[i].uaddr, stream_param.seg[i].size);
		data_size += stream_param.seg[i].size;
	}

	eaif_log_debug("seg_cnt = %d, size = %d", stream_param.seg_cnt, data_size);
	MPI_ENC_releaseChnFrame(chn, &stream_param);
	return data_size;
}

int eaif_utilsFillDataPayload(const EAIF_PARAM_S *param, EaifRequestDataRemote *payload)
{
	int data_size = 0;
	unsigned char *data;

#ifdef EAIF_STATIC_IMAGE
#ifdef EAIF_STATIC_IMAGE_JPEG
	const char *imf = "/mnt/nfs/ethnfs/crowd.jpg";
	FILE *fp = fopen(imf, "rb");
	fseek(fp, 0, SEEK_END);
	data_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	data = (unsigned char *)payload->image_payload;
	fread(data, 1, data_size, fp);
	payload->image.size = data_size;
	fclose(fp);
#endif
#ifdef EAIF_STATIC_IMAGE_RAW_Y
	FILE *fp = fopen(imf, "rb");
	int shape[3];
	fseek(fp, 0, SEEK_END);
	data_size = ftell(fp) - sizeof(int) * 3;
	fseek(fp, 0, SEEK_SET);
	fread(shape, sizeof(int), 3, fp);
	data = (unsigned char *)payload->image_payload;
	fread(data, 0, data_size, fp);
	payload->image.size = data_size;
	fclose(fp);
#endif
#else /* SNAPSHOT */

	if (param->data_fmt == EAIF_DATA_MPI_JPEG || param->data_fmt == EAIF_DATA_MPI_Y ||
	    param->data_fmt == EAIF_DATA_MPI_YUV || param->data_fmt == EAIF_DATA_MPI_RGB) {
		data = (unsigned char *)payload->data_payload;
		if (param->data_fmt == EAIF_DATA_MPI_JPEG)
			*(int *)data = param->target_idx.chn;
		else if (param->data_fmt == EAIF_DATA_MPI_Y || param->data_fmt == EAIF_DATA_MPI_Y ||
		         param->data_fmt == EAIF_DATA_MPI_YUV || param->data_fmt == EAIF_DATA_MPI_RGB)
			*data = param->target_idx.value;
		payload->data.size = sizeof(int);
		payload->data.ptr = (void *)data;
		strncpy(payload->data.name, EAIF_STR_DATA, EAIF_CHAR_LEN);
		return 0;
	}

	MPI_ECHN chn = MPI_ENC_CHN(param->target_idx.chn);
	payload->data.size = 0;
	payload->data.ptr = NULL;

	data_size = eaif_getImageDataJpeg(chn, payload->data_payload);
	if (!data_size)
		return 0;

#endif /* !SNAPSHOT */
	payload->data.size = data_size;
	payload->data.ptr = (void *)payload->data_payload;
	strncpy(payload->data.name, EAIF_STR_DATA, EAIF_CHAR_LEN);
	return 0;
}

int eaif_utilsFillFormatPayload(const EAIF_PARAM_S *param, EaifRequestDataRemote *payload)
{
	int data_size = 0;
	char *data = (char *)payload->format_payload;
	if (param->data_fmt == EAIF_DATA_JPEG)
		data_size = sprintf(data, "%s", EAIF_STR_FMT_JPG);
	else if (param->data_fmt == EAIF_DATA_Y)
		data_size = sprintf(data, "%s", EAIF_STR_FMT_Y);
	else if (param->data_fmt == EAIF_DATA_MPI_JPEG)
		data_size = sprintf(data, "%s", EAIF_STR_FMT_MPI_JPG);
	else if (param->data_fmt == EAIF_DATA_MPI_Y)
		data_size = sprintf(data, "%s", EAIF_STR_FMT_MPI_Y);
	else if (param->data_fmt == EAIF_DATA_MPI_RGB)
		data_size = sprintf(data, "%s", EAIF_STR_FMT_MPI_RGB);
	else
		return -1;
	payload->format.size = data_size;
	strncpy(payload->format.name, EAIF_STR_FMT, EAIF_CHAR_LEN);
	//payload->format.name[data_size] = '\0';
	//strcpy(payload->format.name, EAIF_STR_FMT);
	payload->format.ptr = (void *)data;
	return 0;
}

int eaif_utilsFillMetaPayload(const MPI_IVA_OBJ_LIST_S *obj_list, EaifRequestDataRemote *payload)
{
	int data_size = 0;
	char *data = (char *)payload->meta_payload;
	eaif_log_debug("data:%s", data);
	data_size += sprintf(&data[data_size], "{\"od\":[");
	int send_obj_cnt = 0;
	eaif_log_debug("send_obj_cnt:%d", send_obj_cnt);
	for (int i = 0; i < obj_list->obj_num; i++) {
		eaif_log_debug("i:%d", i);
		//if (obj_list->obj[i].life < obj_life_th) {
		//  continue;
		//}
		eaif_log_debug("startp:%d", i);
		send_obj_cnt++;
		data_size += sprintf(
		        &data[data_size],
		        "{\"obj\":{\"id\":%d,\"rect\":[%d,%d,%d,%d],\"vel\":[%d,%d],\"cat\":\"\",\"shaking\":0}},",
		        obj_list->obj[i].id, obj_list->obj[i].rect.sx, obj_list->obj[i].rect.sy,
		        obj_list->obj[i].rect.ex, obj_list->obj[i].rect.ey, obj_list->obj[i].mv.x,
		        obj_list->obj[i].mv.y);
		eaif_log_debug("data_size:%d", data_size);
	}
	eaif_log_debug("data:%s", data);
	if (send_obj_cnt)
		data_size--;

	data_size += sprintf(&data[data_size], "]}");
	payload->meta.size = data_size;
	eaif_log_debug("data:%s", data);
	strncpy(payload->meta.name, EAIF_STR_META, EAIF_CHAR_LEN);
	//payload->meta.name[data_size] = '\0';
	//strcpy(payload->meta.name, EAIF_STR_META);
	payload->meta.ptr = (void *)data;
	return 0;
}

int eaif_utilsFillTimePayload(unsigned int time, EaifRequestDataRemote *payload)
{
	int data_size = 0;
	char *data = (char *)payload->time_payload;
	data_size += sprintf(&data[data_size], "%u", time);
	payload->time.size = data_size;
	strncpy(payload->time.name, EAIF_STR_TIME, EAIF_CHAR_LEN);
	//strcpy(payload->time.name, EAIF_STR_TIME);
	payload->time.ptr = (void *)data;
	return 0;
}

int eaif_utilsFillShapePayload(const MPI_SIZE_S *res, EaifRequestDataRemote *payload, int channel)
{
	int shape[3] = { res->height, res->width, channel };
	int data_size = sizeof(shape);
	char *data = (char *)payload->shape_payload;
	memcpy(data, shape, data_size);
	payload->shape.size = data_size;
	strncpy(payload->shape.name, EAIF_STR_SHAPE, EAIF_CHAR_LEN);
	//strcpy(payload->shape.name, EAIF_STR_SHAPE);
	payload->shape.ptr = (void *)data;
	return 0;
}

int eaif_utilsSetupRequestRemote(const MPI_IVA_OBJ_LIST_S *obj_list, const EAIF_PARAM_S *param,
                                 struct eaif_info_s *info, EaifRequestDataRemote *payload)
{
	eaif_log_debug("Enter");
	if (payload == NULL) {
		eaif_log_warn("EAIF payload remote buffer is not init!");
		return -1;
	}

	MPI_IVA_OBJ_LIST_S scaled_list = { 0 };
	MPI_SIZE_S resoln = { 0 };
	int channel = 3;
	const MPI_IVA_OBJ_LIST_S *ol_ptr = obj_list;
	const MPI_SIZE_S *resoln_ptr = &info->src_resoln;

	if (param->data_fmt == EAIF_DATA_MPI_Y || param->data_fmt == EAIF_DATA_MPI_RGB) {
		eaif_copyScaledObjList(&info->scale_factor, obj_list, &scaled_list);
		ol_ptr = &scaled_list;
		resoln.width = param->snapshot_width;
		resoln.height = param->snapshot_height;
		resoln_ptr = &resoln;
		channel = (param->data_fmt == EAIF_DATA_MPI_Y) ? 1 : 3;
	}

	/* Setup meta payload metadata*/
	eaif_utilsFillMetaPayload(ol_ptr, payload);

	/* Setup time payload metadata*/
	eaif_utilsFillTimePayload(obj_list->timestamp, payload);

	/* Setup shape payload */
	eaif_utilsFillShapePayload(resoln_ptr, payload, channel);

	/* Setup format payload */
	eaif_utilsFillFormatPayload(param, payload);

	/* Setup image payload */
	eaif_utilsFillDataPayload(param, payload);
	eaif_log_debug("Exit");
	return 0;
}

int eaif_getImageDataSnapshot(MPI_WIN idx, MPI_VIDEO_FRAME_INFO_S *frame_info)
{
	eaif_log_debug("Taking snapshot of target channel (c=%d) (w=%d)", idx.chn, idx.win);
	int err = 0;
	int repeat = 0;
	int try_time = 0;

	do {
		repeat = 0;
		err = MPI_DEV_getWinFrame(idx, frame_info, 1200);
		if (err == -EAGAIN) {
			MPI_DEV_releaseWinFrame(idx, frame_info);
			frame_info->uaddr = 0;
			frame_info->size = 0;
			if (try_time == EAIF_GETWINFRAME_TRY_MAX) {
				eaif_log_warn("MPI_DEV_getWinFrame is too Busy! exit request");
				break;
			}
			try_time += 1;
			repeat = 1;
			eaif_log_warn("GetWinFrame Timeout retry ... #%d/%d", try_time, EAIF_GETWINFRAME_TRY_MAX);
		} else if (err == -ENODATA) {
			eaif_log_warn("No Data from MPI!");
			return -1;
		}
	} while (repeat);

	if (err != MPI_SUCCESS) {
		//MPI_DEV_releaseWinFrame(param->target_idx, frame_info);
		eaif_log_warn("Failed to take snapshot for target channel (c=%d) (w=%d).", idx.chn, idx.win);
		return -1;
	}

	return 0;
}

int getInfFrame(const EAIF_PARAM_S *param, const struct eaif_info_s *info, EaifRequestDataInApp *payload)
{
	payload->size = 0;
#ifdef EAIF_INFERENCE_INAPP
	payload->target_idx = param->target_idx;
	InfImage *image = &payload->inf_image;
	MPI_VIDEO_FRAME_INFO_S *frame_info = &payload->frame_info;

	image->c = 3;
	image->dtype = Inf8UC3;
	int expected_data_size = 0;
	image->h = frame_info->height;
	image->w = frame_info->width;

	switch (param->data_fmt) {
	case EAIF_DATA_MPI_JPEG:
	case EAIF_DATA_JPEG: {
		break;
	}
	case EAIF_DATA_MPI_Y:
	case EAIF_DATA_Y: {
		frame_info->type = MPI_SNAPSHOT_Y;
		image->c = 1;
		image->dtype = Inf8UC1;
		break;
	}
	case EAIF_DATA_MPI_YUV:
	case (EAIF_DATA_YUV): {
		frame_info->type = MPI_SNAPSHOT_NV12;
		break;
	}
	case EAIF_DATA_MPI_RGB:
	case EAIF_DATA_RGB: {
		frame_info->type = MPI_SNAPSHOT_RGB;
		break;
	}
	default: {
		frame_info->type = MPI_SNAPSHOT_Y;
		image->c = 1;
		image->dtype = Inf8UC1;
	}
	};

	if (param->data_fmt == EAIF_DATA_MPI_JPEG || param->data_fmt == EAIF_DATA_MPI_JPEG) {
		MPI_ECHN chn = MPI_ENC_CHN(param->target_idx.chn);
		const MPI_SIZE_S *resoln = &info->src_resoln;
		InfImage raw = Inf_ImcreateEmpty(resoln->width, resoln->height, 3, Inf8U);
		int data_size = eaif_getImageDataJpeg(chn, raw.data);
		if (data_size && image->data) {
			image->buf_owner = 1;
			Inf_Imresize(&raw, image->h, image->w, image);
		}
		Inf_Imrelease(&raw);
		if (!image->data) {
			return 0;
		}
		data_size = image->h * image->w * image->c; // always uint8 dtype
		payload->size = data_size;
	} else {
		int ret = eaif_getImageDataSnapshot(param->target_idx, frame_info);
		if (ret < 0) {
			return -1;
		}

		expected_data_size = frame_info->height * frame_info->width * image->c;
		if ((UINT32)expected_data_size != frame_info->size) {
			eaif_log_warn("Invalid retrieved window snapshot size (%u) vs expected (%u)",
			              frame_info->size, expected_data_size);
			return -1;
		}

		payload->size = expected_data_size;
		image->data = frame_info->uaddr;
		image->buf_owner = 0;
	}

	eaif_log_debug("Window snapshot success! image size :%dx%dx%d type:%d size:%d\n", image->h, image->w, image->c,
	               frame_info->type, frame_info->size);
#endif // EAIF_INFERENCE_INAPP
	return 0;
}

int eaif_utilsSetupRequestInapp(const MPI_IVA_OBJ_LIST_S *obj_list, const EAIF_PARAM_S *param, struct eaif_info_s *info,
                                EaifRequestDataInApp *payload)
{
	eaif_log_debug("Enter");
	if (payload == NULL) {
		eaif_log_warn("EAIF payload inapp buffer is not init!");
		return -1;
	}
	// if fr and stage1 else pass
	payload->size = 0;
	payload->target_idx = param->target_idx;

	const bool face_det_roi = info->algo->face_det_roi;
	const bool face_preserve_prev = info->algo->face_preserve_prev;

	if (param->api == EAIF_API_FACEDET && face_preserve_prev && param->inf_with_obj_list)
		info->prev_obj_list = *obj_list;

	if (param->api == EAIF_API_FACERECO) {
		// Full image recognition
		eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, obj_list, &payload->obj_list);
	} else if (param->api == EAIF_API_FACEDET && face_det_roi && !param->inf_with_obj_list) {
		MPI_IVA_OBJ_LIST_S ol = { 0 };
		ol.obj_num = 1;
		ol.obj[0] = (MPI_IVA_OBJ_ATTR_S){
			.id = 0,
			.conf = 0.0,
			.cat = 0,
			.life = 160,
			.mv = (MPI_MOTION_VEC_S){ .x = 0, .y = 0 },
			.rect = (MPI_RECT_POINT_S){ (info->src_resoln.width * 9) / 100,
			                            (info->src_resoln.height * 16) / 100,
			                            (info->src_resoln.width * 91) / 100,
			                            (info->src_resoln.height * 84) / 100 },
		};
		eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, &ol, &payload->obj_list);
	} else {
		eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, obj_list, &payload->obj_list);
	}

	int ret = getInfFrame(param, info, payload);

	return ret;
}

#ifdef EAIF_SUPPORT_CURL
static size_t WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
	EaifRespond *mem = (EaifRespond *)data;

	if (size * nmemb > EAIF_RESP_LEN) {
		mem->size = sprintf(mem->data, "{\"fail\":\"resp data exceed max buffer lenght!\"}\n");
		return mem->size;
	}

	size_t realsize = size * nmemb;

	//bzero(mem->data, realsize+1);
	memcpy(mem->data, ptr, realsize);
	mem->data[realsize - 1] = '\0';
	mem->size = realsize;
	return realsize;
}
#endif /** !EAIF_SUPPORT_CURL*/

int eaif_utilsSendRequestRemote(const char *url, struct eaif_info_s *info)
{
	eaif_log_debug("Enter req_sta:0x%x", info->req_sta.val);
	/* init curl */
#ifdef EAIF_SUPPORT_CURL
	int i;
	CURL *curl;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);

	/* Setup curl header */
	struct curl_httppost *formpost = NULL;
	struct curl_httppost *lastptr = NULL;
	struct curl_slist *headerlist = NULL;
	static const char buf[] = "Expect:";

	//int rdy = info->payload_rdy;
	//EaifCurlFormData *formdata = info->payload[rdy].formdata;
	EaifRequestFormData *formdata = info->payload.remote->formdata;
	for (i = 0; i < EAIF_PAYLOAD_NUM; i++) {
		curl_formadd(&formpost, &lastptr, CURLFORM_COPYNAME, formdata[i].name, CURLFORM_BUFFER,
		             formdata[i].name, CURLFORM_BUFFERPTR, formdata[i].ptr, CURLFORM_BUFFERLENGTH,
		             formdata[i].size, CURLFORM_END);
	}

	eaif_log_debug("url:%s", url);

	curl = curl_easy_init();
	if (curl) {
		/* initialize custom header list (stating that Expect: 100-continue is not
		wanted */
		headerlist = curl_slist_append(headerlist, "Content-Type: multipart/form-data");
		headerlist = curl_slist_append(headerlist, buf);
		/* what URL that receives this POST */
		curl_easy_setopt(curl, CURLOPT_URL, url);
		/* only disable 100-continue header if explicitly requested */
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "gzip,deflate");
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, EAIF_TIMEOUT_UNIT);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &info->resp);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);

		/* Send request & wait response*/
		res = curl_easy_perform(curl);
		//eaif_log_debug("resp: %s", info->resp.data);
		if (res != CURLE_OK) {
			eaif_log_warn("curl_easy_perform() failed: %s %d", curl_easy_strerror(res), res);
			curl_easy_cleanup(curl);
			curl_global_cleanup();
			return -1;
		}
		/* clean up resp */
		curl_easy_cleanup(curl);
	} else {
		eaif_log_debug("Cannot init curl");
	}

	//info->payload_rdy ^= rdy;
	curl_global_cleanup();
#else
	// Unused parameters.
	(void)url;
	(void)info;
#endif /* !EAIF_SUPPORT_CURL  */
	eaif_log_debug("Exit req_sta:0x%x", info->req_sta.val);
	return 0;
}

int eaif_utilsSendRequestInApp(struct eaif_info_s *info, InferenceModel *model)
{
	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	eaif_log_debug("Enter req_sta:0x%x", info->req_sta.val);

	InfModelCtx *obj = &model->ctx;

	MPI_VIDEO_FRAME_INFO_S *frame_info = &info->payload.inapp->frame_info;
	MPI_WIN target_idx = info->payload.inapp->target_idx;
	InfImage *image = &info->payload.inapp->inf_image;
	MPI_IVA_OBJ_LIST_S *obj_list = &info->payload.inapp->obj_list;
	info->resp.time = obj_list->timestamp;

	switch (info->algo->p->api) {
	case EAIF_API_CLASSIFY:
	case EAIF_API_CLASSIFY_CV:
	case EAIF_API_HUMAN_CLASSIFY: {
		InfResultList *result = &info->resp.result_list;
		ret = Inf_InvokeClassify(obj, image, obj_list, result);
		break;
	}
	case EAIF_API_DETECT:
	case EAIF_API_FACEDET: {
		InfDetList *result = &info->resp.det_list;
		ret = (info->algo->p->inf_with_obj_list) ? Inf_InvokeDetectObjList(obj, image, obj_list, result) :
		                                           Inf_InvokeDetect(obj, image, result);
		break;
	}
	case EAIF_API_FACERECO: {
		InfDetList *result = &info->resp.det_list;
		if (info->algo->p->inf_with_obj_list) {
			// TBD
			// if stage 1, invoke face detection -> dismiss with face detect result
			// if stage 2, invoke face identification -> dismiss with face id result
			ret = Inf_InvokeFaceDetObjList(obj, image, obj_list, result);
		} else {
			ret = Inf_InvokeDetect(obj, image, result);
		}
		break;
	}
	default:
		eaif_log_warn("invalid api request!");
	}

	if (info->algo->p->data_fmt == EAIF_DATA_JPEG || info->algo->p->data_fmt == EAIF_DATA_MPI_JPEG) {
		// TBD
		// if face reco and stage 2 -> release image
		ret = Inf_Imrelease(image);
		if (!image->data) {
			eaif_log_warn("Cannot release image!");
		}
	} else {
		MPI_DEV_releaseWinFrame(target_idx, frame_info);
		frame_info->uaddr = 0;
		frame_info->size = 0;
	}
#endif
	return ret;
}

void decodeDetection(EaifStatusInternal *status, const AGTX_IVA_EAIF_RESP_S *resp)
{
	int i;
	status->obj_cnt = 0;
	EAIF_OBJ_ATTR_S *obj_attr = NULL;
	for (i = 0; i < resp->pred_num && i < MPI_IVA_MAX_OBJ_NUM; i++) {
		obj_attr = &status->obj_attr_ex[i].basic;
		strncpy(obj_attr->category[0], (char *)resp->predictions[i].label[0], EAIF_CHAR_LEN);
		strncpy(obj_attr->prob[0], (char *)resp->predictions[i].prob[0], EAIF_CHAR_LEN);
		obj_attr->id = resp->predictions[i].idx; // re-assign id
		obj_attr->label_num = 1;
		obj_attr->rect.sx = resp->predictions[i].rect[0];
		obj_attr->rect.sy = resp->predictions[i].rect[1];
		obj_attr->rect.ex = resp->predictions[i].rect[2];
		obj_attr->rect.ey = resp->predictions[i].rect[3];
	}
	status->obj_cnt = i;
	eaif_log_debug("PRINT DETECT obj nm:%d [%d] cat:\"%s\":\"%s\" label_num:%d:%d\n", status->obj_cnt,
	               status->obj_attr_ex[0].basic.id, status->obj_attr_ex[0].basic.category[0],
	               resp->predictions[0].label[0], status->obj_attr_ex[0].basic.label_num,
	               resp->predictions[0].label_num);
}

void decodeClassification(EaifStatusInternal *status, const AGTX_IVA_EAIF_RESP_S *resp)
{
	int i, j;
	//status->obj_cnt = resp->pred_num;
	for (i = 0; i < resp->pred_num; i++) {
		for (int k = 0; (UINT32)k < status->obj_cnt; k++) {
			EaifObjAttrEx *obj_attr = &status->obj_attr_ex[k];
			if (resp->predictions[i].idx == obj_attr->basic.id) {
				eaif_log_debug("GET REQUEST [%d]: id:%d, frame_counter:%d \n", k, obj_attr->basic.id,
				               obj_attr->frame_counter);
				//status->obj_attr[k].frame_counter = 0;
				obj_attr->basic.label_num = resp->predictions[i].label_num;
				if (obj_attr->basic.label_num == 0)
					obj_attr->confid_counter = 0;
				for (j = 0; j < resp->predictions[i].label_num; ++j) {
					if (j == 0) {
						if (!obj_attr->confid_counter) {
							obj_attr->confid_counter++;
						} else {
							if (strncmp(obj_attr->basic.category[j],
							            (char *)resp->predictions[i].label[j],
							            EAIF_CHAR_LEN) == 0) {
								obj_attr->confid_counter++;
								eaif_log_debug(
								        "%s obj[%d] cat[%s] vs res[%s] conf:%d\n",
								        __func__, k, obj_attr->basic.category[j],
								        resp->predictions[i].label[j],
								        obj_attr->confid_counter);
								continue;
							} else {
								obj_attr->confid_counter = 1;
							}
						}
					}
					eaif_log_debug("%s:%d obj[%d] cat[%s] vs res[%s] conf:%d\n", __func__, __LINE__,
					               k, obj_attr->basic.category[j], resp->predictions[i].label[j],
					               obj_attr->confid_counter);
					strncpy(obj_attr->basic.category[j], (char *)resp->predictions[i].label[j],
					        EAIF_CHAR_LEN);
					strncpy(obj_attr->basic.prob[j], (char *)resp->predictions[i].prob[j],
					        EAIF_CHAR_LEN);
				}
			}
		}
	}

	status->obj_exist_any = 0;
	for (int k = 0; (UINT32)k < status->obj_cnt; k++) {
		EaifObjAttrEx *obj_attr = &status->obj_attr_ex[k];
		if (obj_attr->basic.label_num) {
			status->obj_exist_any = 1;
			break;
		}
	}

	eaif_log_debug("obj nm:%d [%d] cat:\"%s\":\"%s\" label_num:%d:%d\n", status->obj_cnt,
	               status->obj_attr_ex[0].basic.id, status->obj_attr_ex[0].basic.category[0],
	               resp->predictions[0].label[0], status->obj_attr_ex[0].basic.label_num,
	               resp->predictions[0].label_num);
}

int eaif_auxParseIvaRespJson(const EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp)
{
	int retval = 0;
#ifdef EAIF_SUPPORT_JSON
	enum json_tokener_error jerr;
	struct json_object *json_obj = NULL;
	struct json_tokener *tok = json_tokener_new();

	json_obj = json_tokener_parse_ex(tok, msg->data, msg->size);

	jerr = json_tokener_get_error(tok);
	if (jerr == json_tokener_success) {
		parse_iva_eaif_resp(resp, json_obj);
		if (json_obj != NULL) {
			json_object_put(json_obj); //Decrement the ref count and free if it reaches zero
		} else {
			eaif_log_debug("empty json object parsed");
		}
	} else {
		eaif_log_warn("JSON Tokener errNo: %d = %s ", jerr, json_tokener_error_desc(jerr));
		eaif_log_notice("%s:%d msg:\"%s\"", __func__, __LINE__, msg->data);
		retval = -1;
	}
	json_tokener_free(tok);
#else //
	// Unused parameters.
	(void)msg;
	(void)resp;
	fprintf(stderr, "EAIF does not support JSON in this build!\n");
	assert(0);
#endif /* EAIF_SUPPORT_JSON */
	return retval;
}

int auxParseInappClassify(const InferenceModel *model, EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp)
{
#ifdef EAIF_INFERENCE_INAPP
	InfResultList *results = &msg->result_list;
	const InfStrList *labels = &model->ctx.info->labels;

	int size = results->size;
	int cls = 0;
	float prob = 0.0f;
	resp->time = msg->time;
	resp->pred_num = size;
	for (int i = 0; i < size; i++) {
		const InfResult *result = &results->data[i];
		AGTX_IVA_EAIF_RESP_OBJ_ATTR_S *prediction = &resp->predictions[i];
		prediction->idx = result->id;
		prediction->label_num = result->cls_num;
		for (int j = 0; j < result->cls_num; j++) {
			cls = result->cls[j];
			prob = result->prob[j];
			if (cls >= labels->size) {
				eaif_log_warn("Decode class number (%d) exceeds label size (%d)!", cls, labels->size);
				goto release;
			}
			strncpy((char *)prediction->label[j], labels->data[cls], MAX_EAIF_RESP_OBJ_ATTR_SIZE);
			sprintf((char *)prediction->prob[j], "%.4f", prob);
		}
	}
	resp->success = 1;

release:
	Inf_ReleaseResult(results);
#endif
	return 0;
}

/** @brief detection result
  * @details detection results from NMS
  * cls num stored in result array is in descending order for cls prob array [0-cls_num]
  */
int auxParseInappDetect(const InferenceModel *model, EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp)
{
#ifdef EAIF_INFERENCE_INAPP
	InfDetList *results = &msg->det_list;
	const InfStrList *labels = &model->ctx.info->labels;
#define PRESERVE_TOP_ONE (1)

	int size = results->size;
	int cls = 0;
	float prob = 0.0f;
	resp->time = msg->time;
	resp->pred_num = size;
	for (int i = 0; i < size; i++) {
		const InfDetResult *result = &results->data[i];
		AGTX_IVA_EAIF_RESP_OBJ_ATTR_S *prediction = &resp->predictions[i];
		prediction->idx = result->id;
		prediction->label_num = result->cls_num;
		prediction->rect[0] = result->rect.sx;
		prediction->rect[1] = result->rect.sy;
		prediction->rect[2] = result->rect.ex;
		prediction->rect[3] = result->rect.ey;

		int range = (result->cls_num > MAX_EAIF_RESP_OBJ_CLS_NUM) ? MAX_EAIF_RESP_OBJ_CLS_NUM : result->cls_num;
		for (int j = 0; j < range; j++) {
			cls = result->cls[j];
			prob = result->prob[j];
			if (cls < 0 || cls >= labels->size) {
				eaif_log_warn("Decode Class Number (%d) exceeds label size (%d)!", cls, labels->size);
				goto release;
			}
			strncpy((char *)prediction->label[j], labels->data[cls], MAX_EAIF_RESP_OBJ_ATTR_SIZE);
			sprintf((char *)prediction->prob[j], "%.4f", prob);
		}
	}
	resp->success = 1;

release:
	Inf_ReleaseDetResult(results);
#endif
	return 0;
}

int eaif_auxParseIvaRespInapp(const InferenceModel *model, EaifRespond *msg, AGTX_IVA_EAIF_RESP_S *resp)
{
	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	InfRunType type = model->ctx.info->inference_type;
	if (type == InfRunClassify)
		ret = auxParseInappClassify(model, msg, resp);
	else if (type == InfRunDetect || type == InfRunFaceReco)
		ret = auxParseInappDetect(model, msg, resp);
	return ret;
#endif
	return 0;
}

int eaif_utilsDecodeResult(EAIF_INFERENCE_MODE_E mode, const InferenceModel *model, EaifRespond *msg,
                           EaifStatusInternal *status, AGTX_IVA_EAIF_RESP_S *resp)
{
	/* example msg
	{'pred_num': 1,
	 'predictions': [
		// classification
		 {'idx': 0, 'label': ['obama3'], 'prob': ['0.8838138990677292'], 'label_num':1},
		// detection
		 {'idx': 0, 'rect': [0,0,100,100], 'prob':['0.83'], 'label':['obama'], 'label_num':1}
		 ],
	  'success': True,
	  'time': 189953} */
	int ret = 0, isdet = 0;
	resp->predictions[0].rect[0] = EAIF_RECT_INIT;
	status->obj_attr_ex[0].basic.rect.sx = EAIF_RECT_INIT;

	if (mode == EAIF_MODE_REMOTE) {
		ret = eaif_auxParseIvaRespJson(msg, resp);
	} else if (mode == EAIF_MODE_INAPP) {
		ret = eaif_auxParseIvaRespInapp(model, msg, resp);
	} else {
		eaif_log_warn("Unknown mode for pasring response!");
		return -1;
	}

	if (ret) {
		eaif_log_warn("Cannot decode message resp!");
	} else {
		if (resp->success == 1) {
			status->timestamp = (UINT32)resp->time;
			if (resp->pred_num == 0) {
				status->obj_cnt = 0;
				return ret;
			}
			isdet = (resp->predictions[0].rect[0] != EAIF_RECT_INIT);
			if (isdet)
				decodeDetection(status, resp);
			else
				decodeClassification(status, resp);
			eaif_log_debug("ti:%u cnt:%d id:%d vs %u %d %u\n", status->timestamp, status->obj_cnt,
			               status->obj_attr_ex[0].basic.id, resp->time, resp->pred_num,
			               resp->predictions[0].idx);
		} else {
			eaif_log_debug("Resp Parsing server json string fail message!");
			ret = -1;
		}
	}
	return ret;
}

#if 0
int utilsInvokeFaceRegisterRoi(struct eaif_info_s *info, InferenceModel *model)
{
	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	InfModelCtx *obj = &model->ctx;
	// InfDetList *result = &info->resp.det_list;
	InfImage *image = &info->payload.inapp->inf_image;
	const MPI_IVA_OBJ_LIST_S *ol = &info->payload.inapp->obj_list;
	const MPI_RECT_POINT_S *roi = &ol->obj[0].rect;
	const EAIF_INF_UTILS_S *inf_utils = &info->algo->p->inf_utils;
	const char *face_name = inf_utils->face_name;
	char file_name[256] = {};
	if (info->algo->p->data_fmt == EAIF_DATA_MPI_Y || info->algo->p->data_fmt == EAIF_DATA_Y)
		sprintf(file_name, "%s/faces/%s.pgm", inf_utils->dir, inf_utils->face_name);
	else
		sprintf(file_name, "%s/faces/%s.ppm", inf_utils->dir, inf_utils->face_name);

	ret = (info->algo->p->inf_with_obj_list) ? Inf_RegisterFaceRoiDet(obj, image, roi, face_name) :
	                                           Inf_RegisterFaceRoi(obj, image, roi, face_name);
	if (ret)
		return ret;

	InfImage dst = {};
	int w = roi->ex - roi->sx + 1;
	int h = roi->ey - roi->sy + 1;
	Inf_ImcropResize(image, roi->sx, roi->sy, roi->ex, roi->ey, &dst, w, h);
	Inf_Imwrite(file_name, &dst);
	Inf_Imrelease(&dst);
#endif
	return ret;
}
#endif

int utilsInvokeFaceRegister(const EAIF_INF_UTILS_S *inf_utils, InferenceModel *model)
{
	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	// get model
	InfModelCtx *obj = &model->ctx;
	// get image
	const char *face = inf_utils->face_name;
	char file_name[256] = {};
	sprintf(file_name, "%s/faces/%s.jpg", inf_utils->dir, face);
	if (access(file_name, F_OK) != 0) {
		eaif_log_notice("File \"%s\" not found!", file_name);
		return -ENODEV;
	}

	InfImage image = { 0 };
	Inf_Imread(file_name, &image, 0);
	if (!image.data) {
		eaif_log_notice("Invalid jpeg format, cannot decode \"%s\"!", file_name);
		return -ENODEV;
	}

	// get face
	MPI_RECT_POINT_S roi = { 0, 0, image.w - 1, image.h - 1 };
	ret = Inf_RegisterFaceRoiDet(obj, &image, &roi, face);
	if (ret) {
		eaif_log_notice("Cannot register face \"%s\"!", inf_utils->face_name);
	}

	int ret0 = Inf_Imrelease(&image);
	if (ret0 || image.data) {
		eaif_log_notice("Cannot release image data!");
	}
#endif // EAIF_INFERENCE_INAPP
	return ret;
}

int eaif_faceUtils(const EAIF_INF_UTILS_S *inf_utils, InferenceModel *model)
{
	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	InfModelCtx *ctx = &model->ctx;
	char face_file[256] = {};

	switch (inf_utils->cmd) {
	case EAIF_INF_FACE_REGISTER: {
		ret = utilsInvokeFaceRegister(inf_utils, model);
		break;
	}
	case EAIF_INF_FACE_LOAD: {
		sprintf(face_file, "%s/%s", inf_utils->dir, inf_utils->face_db);
		ret = Inf_LoadFaceData(ctx, face_file);
		break;
	}
	case EAIF_INF_FACE_SAVE: {
		sprintf(face_file, "%s/%s", inf_utils->dir, inf_utils->face_db);
		ret = Inf_SaveFaceData(ctx, face_file);
		break;
	}
	case EAIF_INF_FACE_DELETE: {
		ret = Inf_DeleteFaceData(ctx, inf_utils->face_name);
		break;
	}
	case EAIF_INF_FACE_RESET: {
		ret = Inf_ResetFaceData(ctx);
		break;
	}
	default:
		eaif_log_info("Invalid inference command request (%d)!", inf_utils->cmd);
		break;
	}
#endif // EAIF_INFERENCE_INAPP
	return ret;
}
