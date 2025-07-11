#include "mpi_iva.h"

#include "app.h"
#include "cm_iva_eaif_resp.h"
#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_utils.h"

extern int g_eaif_verbose;

int classifySetLocalInfo(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;

	EaifStatusInternal *status = &instance->status;
	EaifInfo *info = &instance->info;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;

	const int infer_id = info->obj_list.obj[0].id;
	for (int i = 0; (UINT32)i < status->obj_cnt; ++i) {
		EaifObjAttrEx *const obj_attr = &status->obj_attr_ex[i];
		if (obj_attr->basic.id == infer_id) {
			obj_attr->infer_counter++;
			obj_attr->frame_counter = 0;
			break;
		}
	}

	eaif_cpyInternalStatus(status, &info->local_status);

	local_list->obj_num = 0;
	eaif_copyObjList(&info->obj_list, local_list);
	return 0;
}

int classifyInappSendRequest(void *ctx)
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

	InfResultList *result = &info->resp.result_list;
	ret = Inf_InvokeClassify(model, image, obj_list, result);

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

int classifyInappDecodeResult(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const InferenceModel *model = &instance->model;
	EaifInfo *info = &instance->info;
	EaifRespond *msg = &info->resp;
	EaifStatusInternal *status = &info->local_status;
	AGTX_IVA_EAIF_RESP_S *resp = &info->agtx_resp;

	int ret = 0;

	ret = auxParseInappClassify(model, msg, resp);

	if (ret) {
		eaif_log_warn("Cannot decode message resp!");
	} else {
		if (resp->success == 1) {
			status->timestamp = (UINT32)resp->time;
			if (resp->pred_num == 0) {
				status->obj_cnt = 0;
				return ret;
			}

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

int classifyMergeResult(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *status = &instance->status;

	int ret = eaif_mergeInternalStatus(&info->local_status, status);

	eaif_algo_log(
	        "MERGE status size:%d obj: {id: %d, fr_cnt: %d, inf_cnt: %d confid_cnt: %d cat:\"%s\" stage:%d }\n"
	        "                     obj: {id: %d, fr_cnt: %d, inf_cnt: %d confid_cnt: %d cat:\"%s\" stage:%d }\n",
	        status->obj_cnt, status->obj_attr_ex[0].basic.id, status->obj_attr_ex[0].frame_counter,
	        status->obj_attr_ex[0].infer_counter, status->obj_attr_ex[0].confid_counter,
	        status->obj_attr_ex[0].basic.category[0], status->obj_attr_ex[0].stage, status->obj_attr_ex[1].basic.id,
	        status->obj_attr_ex[1].frame_counter, status->obj_attr_ex[1].infer_counter,
	        status->obj_attr_ex[1].confid_counter, status->obj_attr_ex[1].basic.category[0],
	        status->obj_attr_ex[1].stage);

	return ret;
}

int classifyUpdate(void *ctx, const MPI_IVA_OBJ_LIST_S *obj_list)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	const EaifAlgo *algo = &instance->algo;
	const EAIF_PARAM_S *p = &instance->param;
	EaifStatusInternal *pstatus = &instance->status;

	eaif_updateObjAttr(obj_list, p->obj_life_th, algo, pstatus);
	int appendable = eaif_checkAppendable(obj_list, algo, pstatus, &info->obj_list);
	eaif_log_debug("req_sta:0x%x ol_cnt:%d ol_num(extlife):id:%d \n", info->req_sta.val, info->obj_list.obj_num,
	               info->obj_list.obj[0].id);

	return appendable;
}

int initInappClassifyCb(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	instance->probe = inappProbe;
	instance->init_buffer = inappInit;
	instance->set_local_info = classifySetLocalInfo;
	instance->set_payload = inappSetPayload;
	instance->send_request = classifyInappSendRequest;
	instance->decode_result = classifyInappDecodeResult;
	instance->merge_result = classifyMergeResult;
	instance->debug = debugEmptyCb;

	instance->init_algo = baseInitAlgo;
	instance->init_module = inappInitModule;
	instance->exit_module = inappExitModule;
	instance->release_buffer = baseReleaseBuffer;

	instance->update = classifyUpdate;
	return 0;
}
