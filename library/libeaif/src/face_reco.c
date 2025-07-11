#include "app.h"

#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>

#include "eaif.h"

#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"

extern int g_eaif_verbose;
static struct timespec start;

int inappInitFaceReco(void *ctx)
{
	int ret = inappInit(ctx);
	if (ret) {
		eaif_log_warn("Cannot init inapp buffer!");
		return ret;
	}
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EAIF_PARAM_S *param = &instance->param;
	InferenceModel *model = &instance->model;

	EAIF_INF_UTILS_S inf_utils = param->inf_utils;
	inf_utils.cmd = EAIF_INF_FACE_LOAD;
	char face_file[256] = {};
	sprintf(face_file, "%s/%s", inf_utils.dir, inf_utils.face_db);
	if (access(face_file, F_OK) == 0) {
		ret = eaif_faceUtils(&inf_utils, model);
		if (ret) {
			eaif_log_warn("Cannot preload existing face data!");
		}
	}
	return ret;
}

int faceRecoInappSetLocalInfo(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;

	EaifStatusInternal *status = &instance->status;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;

	eaif_cpyInternalStatus(status, &info->local_status);

	local_list->obj_num = 0;
	eaif_copyObjList(&info->obj_list, local_list);

	info->local_inf_fr_stage = info->inf_fr_stage;

	eaif_algo_log(
	        "LOCAL INFO: ti: %u, avail:%u, ol size: %d, obj0:{ id: %d }, st size: %d status0:{ id: %d, fr_cnt: %d, inf_cnt: %d stage: %d}\n",
	        local_list->timestamp, info->req_sta.avail, local_list->obj_num, local_list->obj[0].id, status->obj_cnt,
	        status->obj_attr_ex[0].basic.id, status->obj_attr_ex[0].frame_counter,
	        status->obj_attr_ex[0].infer_counter, status->obj_attr_ex[0].stage);

	return 0;
}

int faceRecoInappSetPayload(void *ctx)
{
	if (g_eaif_verbose)
		TI_TIC(start);

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;
	EaifRequestDataInApp *payload = info->payload.inapp;

	eaif_log_debug("Enter");

	int ret = 0;
#ifdef EAIF_INFERENCE_INAPP
	if (payload == NULL) {
		eaif_log_warn("EAIF payload inapp buffer is not init!");
		return -1;
	}
	// if fr and stage1 else pass
	eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, local_list, &payload->obj_list);

	ret = getInfFrame(param, info, payload);

#endif // EAIF_INFERENCE_INAPP;

	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

	return ret;
}

int faceRecoInappSetPayloadV2(void *ctx)
{
	if (g_eaif_verbose)
		TI_TIC(start);

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;
	EaifRequestDataInApp *payload = info->payload.inapp;
	int ret = 0;

	if (info->local_inf_fr_stage == 1) { /* stage 2 by pass image query */
		payload->size = 1;
		goto exit;
	}

	eaif_log_debug("Enter");

#ifdef EAIF_INFERENCE_INAPP
	if (payload == NULL) {
		eaif_log_warn("EAIF payload inapp buffer is not init!");
		ret = -1;
		goto exit;
	}
	// if fr and stage1 else pass
	eaif_copyScaledListWithBoundary(&info->src_resoln, &info->scale_factor, local_list, &payload->obj_list);

	ret = getInfFrame(param, info, payload);

#endif // EAIF_INFERENCE_INAPP;
	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

exit:
	eaif_algo_log("SET LOAD: inf_stage: %d, payload size: %d\n", info->inf_fr_stage, payload->size);

	return ret;
}

int faceRecoInappSendRequest(void *ctx)
{
	struct timespec req_time;
	int ret = 0;

	if (g_eaif_verbose)
		TI_TIC(req_time);

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

	if (info->algo->p->inf_with_obj_list) {
		// TBD
		// if stage 1, invoke face detection -> dismiss with face detect result
		// detect -> lock obj and set stage 2, stage 2
		// Inf_InvokeFaceDetObjList_marked
		ret = Inf_InvokeFaceDetObjList(model, image, obj_list, result);

		// if stage 2, invoke face identification -> dismiss with face id result
		// set stage 0, unlock obj
	} else {
		ret = Inf_InvokeDetect(model, image, result);
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

#endif // EAIF_INFERENCE_INAPP

	if (g_eaif_verbose)
		TI_TOC("Total Resp", req_time);

	return ret;
}

int faceRecoInappSendRequestV2(void *ctx)
{
	struct timespec req_time;
	int ret = 0;

	if (g_eaif_verbose)
		TI_TIC(req_time);

#ifdef EAIF_INFERENCE_INAPP

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const EaifAlgo *algo = &instance->algo;
	EaifInfo *info = &instance->info;
	InfModelCtx *model = &instance->model.ctx;
	MPI_VIDEO_FRAME_INFO_S *frame_info = &info->payload.inapp->frame_info;
	MPI_WIN target_idx = info->payload.inapp->target_idx;
	InfImage *image = &info->payload.inapp->inf_image;
	MPI_IVA_OBJ_LIST_S *obj_list = &info->payload.inapp->obj_list;
	info->resp.time = obj_list->timestamp;
	InfDetList *result = &info->resp.det_list;

	if (algo->p->inf_with_obj_list) {
		if ((info->local_inf_fr_stage == 0 || info->local_inf_fr_stage == 2) &&
		    obj_list->obj_num) { /* obj_num may be zero and filtered in set_payload */
			ret = Inf_InvokeFaceRecoStageOne(model, image, obj_list, result);
			if (ret)
				eaif_log_warn("Fail to invoke stage one face recognition!");

			eaif_algo_int(one);
			eaif_algo_log(
			        "RUN INF: run stage one: ind: %02d ! result size: %d, id: %d cat0: \"%s\", prob0: %.3f, coord0: [%d, %d, %d, %d]\n",
			        eaif_algo_exp(one++), result->size, (result->size) ? result->data[0].id : -1,
			        (result->size) ? "face" : "", (result->size) ? result->data[0].prob[0] : 0.0,
			        (result->size) ? result->data[0].rect.sx : 0,
			        (result->size) ? result->data[0].rect.sy : 0,
			        (result->size) ? result->data[0].rect.ex : 0,
			        (result->size) ? result->data[0].rect.ey : 0);

		} else if (info->local_inf_fr_stage == 1) {
			ret = Inf_InvokeFaceRecoStageTwo(model, result);

			if (ret)
				eaif_log_warn("Fail to invoke stage two face recognition!");

			eaif_algo_int(two);
			eaif_algo_log(
			        "RUN INF: run stage two: ind: %02d ! result size: %d, id: %d cat0: \"%s\", prob0: %.3f\n",
			        eaif_algo_exp(two++), result->size, (result->size) ? result->data[0].id : -1,
			        (result->size) ? model->info->labels.data[result->data[0].cls[0]] : "",
			        result->data[0].prob[result->data[0].cls[0]]);
		}
	} else {
		ret = Inf_InvokeDetect(model, image, result);
		if (ret)
			eaif_log_warn("Fail to invoke face recognition!");
	}

	if ((info->algo->p->data_fmt == EAIF_DATA_JPEG || info->algo->p->data_fmt == EAIF_DATA_MPI_JPEG) &&
	    image->data) { /* release image only if there is value */
		ret = Inf_Imrelease(image);
		if (!image->data) {
			eaif_log_warn("Cannot release image!");
		}
	} else if (frame_info->uaddr) { /* release frame only if there is value */
		MPI_DEV_releaseWinFrame(target_idx, frame_info);
		frame_info->uaddr = 0;
		frame_info->size = 0;
	}

#endif // EAIF_INFERENCE_INAPP

	if (g_eaif_verbose)
		TI_TOC("Total Resp", req_time);

	return ret;
}

int faceRecoUpdateInfState(void *ctx)
{
#ifdef EAIF_INFERENCE_INAPP
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const EaifAlgo *algo = &instance->algo;
	EaifInfo *info = &instance->info;

	const MPI_RECT_POINT_S *box = NULL;
	const InfDetList *results = &info->resp.det_list;

	int max_box_pix = 0;
	int min_face_pixel = algo->min_fr_pixel;
	int prev_stage = info->local_inf_fr_stage;

	if (info->local_inf_fr_stage == 0 || info->local_inf_fr_stage == 2) { /* only consider top one box */
		if (results->size) {
			box = &results->data[0].rect;
			max_box_pix = max(box->ex - box->sx + 1, box->ey - box->sy + 1);
			if (max_box_pix >= min_face_pixel) {
				info->local_inf_fr_stage = 1;
				info->inf_fr_stage_id = results->data[0].id;
			} else {
				info->local_inf_fr_stage = 2;
			}
		} else {
			info->local_inf_fr_stage = 0;
		}
	} else if (info->local_inf_fr_stage == 1) {
		info->local_inf_fr_stage = 2;
	}
	// info->inf_fr_counter = 0;
#endif // EAIF_INFERENCE_INAPP
	return prev_stage;
}

int faceRecoInappDecodeResult(void *ctx)
{
#ifdef EAIF_INFERENCE_INAPP

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *status = &info->local_status;
	MPI_IVA_OBJ_LIST_S *obj_list = &info->payload.inapp->obj_list;
	EaifObjAttrEx *obj_attr = NULL;

	/* update state here */
	int prev_stage = faceRecoUpdateInfState(ctx);

	/* copy result to internal status directly */

	/* decode model result to info->local_status directly
	   instead of intermediate representation in json format
	   which is necessary only for remote mode,
	   preserve top 1 only */

	const InferenceModel *model = &instance->model;
	const InfStrList *labels = &model->ctx.info->labels;
	InfDetList *results = &info->resp.det_list;

	int cls = 0;
	float prob = 0.0f;
	int markers[MPI_IVA_MAX_OBJ_NUM] = {};

	int match_id __attribute__((unused)) = -1;

	if (results->size == 0) {
		if ((prev_stage == 2) && (info->local_inf_fr_stage == 0)) {
			// cover face
			for (int j = 0; (UINT32)j < status->obj_cnt; j++) {
				obj_attr = &status->obj_attr_ex[j];
				if (obj_attr->basic.id != obj_list->obj[0].id)
					continue;
				obj_attr->stage = info->local_inf_fr_stage;
				strncpy(obj_attr->basic.category[0], "", EAIF_CHAR_LEN);
				sprintf(obj_attr->basic.prob[0], "%s", "");
				obj_attr->confid_counter = 0;
			}
		} else { // (prev_stage == 0) && (info->local_inf_fr_stage == 0)
			;
		}
	}

	for (int i = 0; i < results->size; i++) {
		const InfDetResult *result = &results->data[i];

		for (int j = 0; (UINT32)j < status->obj_cnt; j++) {
			if (markers[j])
				continue;

			obj_attr = &status->obj_attr_ex[j];

			if (obj_attr->basic.id != result->id)
				continue;

			match_id = j;
			markers[j] = 1;

			obj_attr->basic.id = result->id;
			obj_attr->basic.label_num = result->cls_num;
			obj_attr->stage = info->local_inf_fr_stage;

			/* for stage-0/2 do face det */
			/* for stage-1 do face reco */
			/* else */
			// confid_counter = 0

			// face detected

			if (prev_stage == 0 || prev_stage == 2) {
				if (info->local_inf_fr_stage == 2) {
					strncpy(obj_attr->basic.category[0], "small_face", EAIF_CHAR_LEN);
					sprintf(obj_attr->basic.prob[0], "%.4f", prob);
				} else if (info->local_inf_fr_stage == 1) {
					if (!obj_attr->infer_counter) { /* new person */
						// do nothing
					} else if (!obj_attr->confid_counter) { /* unknown */
						strncpy(obj_attr->basic.category[0], "unknown" /* or face */,
						        EAIF_CHAR_LEN);
						sprintf(obj_attr->basic.prob[0], "%.4f", prob);
					} else if (obj_attr->confid_counter) { /* keep person id */
						// do nothing
					}
				}
			} else if (prev_stage == 1 && info->local_inf_fr_stage == 2) {
				if (obj_attr->basic.label_num) {
					cls = result->cls[0];
					prob = result->confidence;
					// check for confid counter
					if (!strncmp(labels->data[cls], "unknown", EAIF_CHAR_LEN))
						obj_attr->confid_counter = 0;
					else if (strncmp(obj_attr->basic.category[0], labels->data[cls],
					                 EAIF_CHAR_LEN) == 0)
						obj_attr->confid_counter++;
					else
						obj_attr->confid_counter = 1;
					strncpy(obj_attr->basic.category[0], labels->data[cls], EAIF_CHAR_LEN);
					sprintf(obj_attr->basic.prob[0], "%.4f", prob);
				}
				obj_attr->infer_counter++;
			} else {
				obj_attr->confid_counter = 0;
			}

			obj_attr->frame_counter = 0;
		}
	}

	info->inf_fr_stage = info->local_inf_fr_stage;
	info->agtx_resp.success = 1;

#define ID(id) (id == -1) ? (0) : (id)

	eaif_algo_log(
	        "RESULT status size:%d match(%d) stage:%d obj: {id: %d, fr_cnt: %d, inf_cnt: %d confid_cnt: %d cat:\"%s\"}\n",
	        status->obj_cnt, match_id, info->inf_fr_stage, status->obj_attr_ex[ID(match_id)].basic.id,
	        status->obj_attr_ex[ID(match_id)].frame_counter, status->obj_attr_ex[ID(match_id)].infer_counter,
	        status->obj_attr_ex[ID(match_id)].confid_counter, status->obj_attr_ex[ID(match_id)].basic.category[0]);

	Inf_ReleaseDetResult(results);

#undef ID

#endif // EAIF_INFERENCE_INAPP

	return 0;
}

int faceRecoMergeResult(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *status = &instance->status;

	int ret = eaif_cpyScaledFaceStatus(&info->scale_factor, &info->local_status, &info->local_list, status);

	eaif_algo_log("MERGE status size:%d obj: {id: %d, fr_cnt: %d, inf_cnt: %d confid_cnt: %d cat:\"%s\"}\n"
	              "                          {id: %d, fr_cnt: %d, inf_cnt: %d confid_cnt: %d cat:\"%s\"}\n",
	              status->obj_cnt, status->obj_attr_ex[0].basic.id, status->obj_attr_ex[0].frame_counter,
	              status->obj_attr_ex[0].infer_counter, status->obj_attr_ex[0].confid_counter,
	              status->obj_attr_ex[0].basic.category[0], status->obj_attr_ex[1].basic.id,
	              status->obj_attr_ex[1].frame_counter, status->obj_attr_ex[1].infer_counter,
	              status->obj_attr_ex[1].confid_counter, status->obj_attr_ex[1].basic.category[0]);

	return ret;
}

int faceRecoUpdate(void *ctx, const MPI_IVA_OBJ_LIST_S *obj_list)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *pstatus = &instance->status;
	const EaifAlgo *algo = &instance->algo;

	int appendable = 0;
	if (obj_list->obj_num == 0) {
		info->obj_list.obj_num = 0;
		info->local_status.obj_exist_any = 0;
		info->inf_fr_stage = 0;
		pstatus->timestamp = obj_list->timestamp;
		pstatus->obj_cnt = 0;
		pstatus->obj_exist_any = 0;
		pstatus->obj_exist_any_counter = 0;
		info->inf_fr_counter++;
	} else {
		eaif_updateFrObjAttr(obj_list, algo, info, pstatus);
		appendable = eaif_checkFrAppendable(obj_list, algo, pstatus, info);
		eaif_log_debug("req_sta:0x%x ol_cnt:%d ol_num(extlife):id:%d \n", info->req_sta.val,
		               info->obj_list.obj_num, info->obj_list.obj[0].id);
	}

	eaif_algo_log(
	        "UPDATE (appendable:%d, avail:%u) time: %u, inf_cnt: %d, stage_wait_1: %d, _2:%d pos period: %d, neg: %d\n"
	        "       ol size:%d : {id: %d, life:%d} {id: %d, life:%d} \n"
	        "       st size:%d : {id: %d, fr_cnt:%d, inf_cnt:%d, conf_cnt:%d, stage:%d, cat:\"%s(%d)\"}\n"
	        "                    {id: %d, fr_cnt:%d, inf_cnt:%d, conf_cnt:%d, stage:%d, cat:\"%s(%d)\"}\n"
	        "       dst ol size:%d : {id: %d, life:%d} {id: %d, life:%d}\n",
	        appendable, info->req_sta.avail, obj_list->timestamp, info->inf_fr_counter, algo->fr_stage1_wait,
	        algo->fr_stage2_wait, algo->p->pos_classify_period, algo->p->neg_classify_period, obj_list->obj_num,
	        obj_list->obj[0].id, obj_list->obj[0].life, obj_list->obj[1].id, obj_list->obj[1].life,
	        pstatus->obj_cnt, pstatus->obj_attr_ex[0].basic.id, pstatus->obj_attr_ex[0].frame_counter,
	        pstatus->obj_attr_ex[0].infer_counter, pstatus->obj_attr_ex[0].confid_counter,
	        pstatus->obj_attr_ex[0].stage, pstatus->obj_attr_ex[0].basic.category[0],
	        pstatus->obj_attr_ex[0].basic.label_num, pstatus->obj_attr_ex[1].basic.id,
	        pstatus->obj_attr_ex[1].frame_counter, pstatus->obj_attr_ex[1].infer_counter,
	        pstatus->obj_attr_ex[1].confid_counter, pstatus->obj_attr_ex[1].stage,
	        pstatus->obj_attr_ex[1].basic.category[0], pstatus->obj_attr_ex[1].basic.label_num,
	        info->obj_list.obj_num, info->obj_list.obj[0].id, info->obj_list.obj[0].life, info->obj_list.obj[1].id,
	        info->obj_list.obj[1].life);

	return appendable;
}

int faceRecoInappInitAlgo(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifAlgo *algo = &instance->algo;
	info->algo = algo;
	algo->p = &instance->param;

	algo->min_fr_pixel = algo->p->min_size;

	// preserved
	algo->min_fr_snp_pixel = algo->min_fr_pixel * algo->p->snapshot_height / info->src_resoln.height;

	char *min_face_str = getenv("FR_MIN_FACE");
	if (min_face_str) {
		eaif_log_info("Environment variable FR_MIN_FACE detected. (%d)", atoi(min_face_str));
		algo->min_fr_pixel = atoi(min_face_str);
	}

	algo->min_fr_ratio = (Fraction){ 2, 10 };
	algo->max_fr_ratio = (Fraction){ 10, 2 };

	/* update */
	algo->new_obj_prio = 1000;
	algo->neg_obj_prio = 500;
	algo->pos_face_prio = 1;
	algo->face_area_prio = 10;
	algo->life_prio = 10;
	algo->inf_period_prio = 300;
	//algo->neg_obj_inf_prio = -10;
	algo->face_prio_rate = (Fraction){ 1, 10 };

	// 30 fps
	algo->pos_period = 300;
	algo->fr_stage1_wait = algo->p->identification_period;
	algo->fr_stage2_wait = algo->p->detection_period;
	return 0;
}

int initInappFaceRecoCb(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	instance->probe = inappProbe;
	instance->init_buffer = inappInitFaceReco;
	instance->set_local_info = detectSetLocalInfo;
	instance->set_payload = faceRecoInappSetPayload;
	instance->send_request = faceRecoInappSendRequest;
	instance->decode_result = detectInappDecodeResult;
	instance->merge_result = faceRecoMergeResult;
	instance->debug = debugEmptyCb;

	instance->init_algo = baseInitAlgo;
	instance->init_module = inappInitModule;
	instance->exit_module = inappExitModule;
	instance->release_buffer = baseReleaseBuffer;

	instance->update = detectUpdate;
	return 0;
}

int initInappFaceRecoCbV2(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	instance->probe = inappProbe;
	instance->init_buffer = inappInitFaceReco;
	instance->set_local_info = faceRecoInappSetLocalInfo;
	instance->set_payload = faceRecoInappSetPayloadV2;
	instance->send_request = faceRecoInappSendRequestV2;
	instance->decode_result = faceRecoInappDecodeResult;
	instance->merge_result = classifyMergeResult;
	instance->debug = debugEmptyCb;

	instance->init_algo = faceRecoInappInitAlgo;
	instance->init_module = inappInitModule;
	instance->exit_module = inappExitModule;
	instance->release_buffer = baseReleaseBuffer;

	instance->update = faceRecoUpdate; /* TBD */
	return 0;
}
