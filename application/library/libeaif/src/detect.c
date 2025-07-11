#include "app.h"

#include "mpi_iva.h"

#include "cm_iva_eaif_resp.h"
#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_utils.h"

extern int g_eaif_verbose;

int detectSetLocalInfo(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;

	local_list->obj_num = 0;
	eaif_copyObjList(&info->obj_list, local_list);
	return 0;
}

int detectInappSendRequest(void *ctx)
{
	struct timespec req_time;

	if (g_eaif_verbose)
		TI_TIC(req_time);

	int ret = 0;

#ifdef EAIF_INFERENCE_INAPP

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	InfModelCtx *model = &instance->model.ctx;
	MPI_VIDEO_FRAME_INFO_S *frame_info = &info->payload.inapp->frame_info;
	MPI_WIN target_idx = info->payload.inapp->target_idx;
	InfImage *image = &info->payload.inapp->inf_image;
	MPI_IVA_OBJ_LIST_S *obj_list = &info->payload.inapp->obj_list;
	info->resp.time = obj_list->timestamp;

	InfDetList *result = &info->resp.det_list;
	ret = (info->algo->p->inf_with_obj_list) ? Inf_InvokeDetectObjList(model, image, obj_list, result) :
	                                           Inf_InvokeDetect(model, image, result);

	if (info->algo->p->data_fmt == EAIF_DATA_JPEG || info->algo->p->data_fmt == EAIF_DATA_MPI_JPEG) {
		ret = Inf_Imrelease(image);
		if (!image->data) {
			eaif_log_warn("Cannot release image!");
		}
	} else {
		MPI_DEV_releaseWinFrame(target_idx, frame_info);
		frame_info->uaddr = 0;
		frame_info->size = 0;
	}

#endif // EAIF_INFERENCE_INAPP

	if (g_eaif_verbose)
		TI_TOC("Total Resp", req_time);

	return ret;
}

int detectMergeResult(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *status = &instance->status;

	int ret = eaif_cpyScaledDetectionStatus(&info->scale_factor, &info->local_status, status);
	return ret;
}

int detectInappDecodeResult(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const InferenceModel *model = &instance->model;
	EaifInfo *info = &instance->info;
	EaifRespond *msg = &info->resp;
	EaifStatusInternal *status = &info->local_status;
	AGTX_IVA_EAIF_RESP_S *resp = &info->agtx_resp;

	int ret = 0;

	ret = auxParseInappDetect(model, msg, resp);

	if (ret) {
		eaif_log_warn("Cannot decode message resp!");
	} else {
		if (resp->success == 1) {
			status->timestamp = (UINT32)resp->time;
			if (resp->pred_num == 0) {
				status->obj_cnt = 0;
				return ret;
			}
			decodeDetection(status, resp);

			eaif_log_debug("ti:%u cnt:%d id:%d vs %u %d %u", status->timestamp, status->obj_cnt,
			               status->obj_attr_ex[0].basic.id, resp->time, resp->pred_num,
			               resp->predictions[0].idx);
		} else {
			eaif_log_debug("Resp Parsing server json string fail message!");
			ret = -1;
		}
	}

	return ret;
}

int detectInappDecodeResultV2(void *ctx)
{
	/* decode model result to info->local_status directly instead of
	   intermediate representation in json format which is
	   necessary only for remote mode, preserve top 1 only*/

#ifdef EAIF_INFERENCE_INAPP

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *status = &info->local_status;
	EAIF_OBJ_ATTR_S *obj_attr = NULL;

	const InferenceModel *model = &instance->model;
	const InfStrList *labels = &model->ctx.info->labels;
	InfDetList *results = &info->resp.det_list;

	int cls = 0;
	float prob = 0.0f;

	status->obj_cnt = results->size;

	for (int i = 0; i < results->size; i++) {
		const InfDetResult *result = &results->data[i];
		obj_attr = &status->obj_attr_ex[i].basic;

		obj_attr->id = result->id;
		obj_attr->label_num = result->cls_num;
		obj_attr->rect = result->rect;

		if (obj_attr->label_num) {
			cls = result->cls[0];
			prob = result->confidence;
			strncpy(obj_attr->category[0], labels->data[cls], EAIF_CHAR_LEN);
			sprintf(obj_attr->prob[0], "%.4f", prob);
		}
	}

	info->agtx_resp.success = 1;

	eaif_log_debug("PRINT DETECT obj nm:%d [%d] cat:\"%s\":\"%s\" label_num:%d:%d", status->obj_cnt,
	               status->obj_attr_ex[0].basic.id, status->obj_attr_ex[0].basic.category[0],
	               resp->predictions[0].label[0], status->obj_attr_ex[0].basic.label_num,
	               resp->predictions[0].label_num);

	Inf_ReleaseDetResult(results);

#endif // EAIF_INFERENCE_INAPP

	return 0;
}

int detectUpdate(void *ctx, const MPI_IVA_OBJ_LIST_S *obj_list)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	const EaifAlgo *algo = &instance->algo;
	const EAIF_PARAM_S *p = &instance->param;

	instance->status.timestamp = obj_list->timestamp;

	int appendable = 0;
	int detect_period = (p->detection_period) ? p->detection_period : 1;

	if (!info->detect_counter)
		appendable = 1;
	//info->req_sta.append = 1;
	info->detect_counter++;
	if (info->detect_counter >= detect_period)
		info->detect_counter = 0;

	if (instance->param.api == EAIF_API_FACEDET && algo->face_preserve_prev && obj_list->obj_num == 0) {
		info->obj_list = info->prev_obj_list;
	} else {
		if (p->inf_with_obj_list) {
			appendable &= eaif_checkDetAppendable(obj_list, algo, &info->obj_list);
		} else {
			info->obj_list = *obj_list;
		}
	}
	return appendable;
}

int initInappDetectCb(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	instance->probe = inappProbe;
	instance->init_buffer = inappInit;
	instance->set_local_info = detectSetLocalInfo;
	instance->set_payload = inappSetPayload;
	instance->send_request = detectInappSendRequest;
	instance->decode_result = detectInappDecodeResult;
	instance->merge_result = detectMergeResult;
	instance->debug = debugEmptyCb;

	instance->init_algo = baseInitAlgo;
	instance->init_module = inappInitModule;
	instance->exit_module = inappExitModule;
	instance->release_buffer = baseReleaseBuffer;

	instance->update = detectUpdate;
	return 0;
}
