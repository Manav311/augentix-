#include "app.h"

#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpi_enc.h"

#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"

extern int g_eaif_verbose;
static struct timespec start;

int remoteProbe(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;

	EAIF_PARAM_S *param = &instance->param;
	EaifInfo *info = &instance->info;
	EaifStatusInternal *status = &instance->status;
	int ret = 0;

	// eaif_utilsSetUrl(info->mode, param, url);
	sprintf(info->local_url, EAIF_STR_API_FMT, param->url, param->api_models[param->api]);

	while (1) {
		ret = eaif_utilsTesturl(param->url, status);
		if (ret || status->server_reachable == EAIF_URL_NONREACHABLE) {
			eaif_log_warn("Cannot reach target url:%s", param->url);
			sleep(EAIF_TRY_SLEEP_TIME);
		} else {
			eaif_log_debug("Server reaches! resp:%s", info->resp.data);
			break;
		}
	}
	return 0;
}

int remoteInitBuf(EaifInfo *info)
{
	if (!info->payload.ptr) {
		info->payload.remote = (EaifRequestDataRemote *)malloc(sizeof(EaifRequestDataRemote));
		if (!info->payload.ptr) {
			eaif_log_warn("Cannot Alloc Payload Buffer for eaif on Mode %s!\n",
			              eaif_utilsGetModeChar(info->mode));
			return -1;
		}
	}
	return 0;
}

int remoteInit(void *ctx)
{
	int ret = 0;
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EAIF_PARAM_S *param = &instance->param;
	EaifInfo *info = &instance->info;
	do {
		pthread_testcancel();
		ret = remoteInitBuf(info);
		if (ret == -1)
			usleep(EAIF_RUN_USLEEP);
	} while (ret == -1);

	eaif_calcScaleFactor(param->snapshot_width, param->snapshot_height, &info->src_resoln, &info->scale_factor);

	return 0;
}

int remoteSetPayload(void *ctx)
{
	if (g_eaif_verbose)
		TI_TIC(start);

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;

	int ret = eaif_utilsSetupRequestRemote(local_list, param, info, info->payload.remote);

	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

	return ret;
}

int remoteSendRequest(void *ctx)
{
	struct timespec req_time;

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;

	if (g_eaif_verbose)
		TI_TIC(req_time);

	int ret = eaif_utilsSendRequestRemote(info->local_url, info);

	if (g_eaif_verbose)
		TI_TOC("Total Resp", req_time);
	return ret;
}

int remoteDecodeResult(void *ctx)
{
	/* sample return from eaif_serv
	{'pred_num': 1,
	 'predictions': [
		// classification
		 {'idx': 0, 'label': ['obama3'], 'prob': ['0.8838138990677292'], 'label_num':1},
		// detection
		 {'idx': 0, 'rect': [0,0,100,100], 'prob':['0.83'], 'label':['obama'], 'label_num':1}
		 ],
	  'success': True,
	  'time': 189953} */

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EaifRespond *msg = &info->resp;
	EaifStatusInternal *status = &info->local_status;
	AGTX_IVA_EAIF_RESP_S *resp = &info->agtx_resp;

	int ret = 0;
	bool is_det = (instance->param.api == EAIF_API_DETECT || instance->param.api == EAIF_API_FACEDET ||
	               instance->param.api == EAIF_API_FACERECO);

	ret = eaif_auxParseIvaRespJson(msg, resp);

	if (ret) {
		eaif_log_warn("Cannot decode message resp!");
	} else {
		if (resp->success == 1) {
			status->timestamp = (UINT32)resp->time;
			if (resp->pred_num == 0) {
				status->obj_cnt = 0;
				return ret;
			}
			if (is_det)
				decodeDetection(status, resp);
			else
				decodeClassification(status, resp);
			eaif_log_debug("ti:%u cnt:%d id:%d vs %u %d %u\n", status->timestamp, status->obj_cnt,
			               status->obj_attr_ex[0].basic.id, resp->time, resp->pred_num,
			               resp->predictions[0].idx);
		} else {
			eaif_log_debug("Resp Parsing server json string fail message!.");
			ret = -1;
		}
	}
	return ret;
}

int remoteInitModule(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const EAIF_PARAM_S *p = &instance->param;
	int ret = 0;

	if (p->data_fmt == EAIF_DATA_JPEG || p->data_fmt == EAIF_DATA_MPI_JPEG) {
		eaif_log_debug("[DEBUG] init MPI Bitstream!");
		MPI_initBitStreamSystem();
		instance->snapshot_chn = MPI_ENC_CHN(3);
		instance->bchn = MPI_createBitStreamChn(instance->snapshot_chn);
	} else {
		instance->snapshot_chn.value = CHN_UNINIT;
		instance->bchn.value = CHN_UNINIT;
	}
	return ret;
}

int remoteExitModule(void *ctx)
{
	eaif_log_debug("Destroy bitstream chn!");
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const EAIF_PARAM_S *p = &instance->param;

	int ret = 0;
	if (p->data_fmt == EAIF_DATA_JPEG || p->data_fmt == EAIF_DATA_MPI_JPEG) {
		eaif_log_debug("Exit MPI bitstream!");
		ret = MPI_destroyBitStreamChn(instance->bchn);
		ret = MPI_exitBitStreamSystem();
		//MPI_SYS_exit();
	}
	return ret;
}

int initRemoteCb(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EAIF_PARAM_S *param = &instance->param;

	// run functors
	instance->probe = remoteProbe;
	instance->init_buffer = remoteInit;
	instance->set_payload = remoteSetPayload;
	instance->send_request = remoteSendRequest;
	instance->decode_result = remoteDecodeResult;
	instance->debug = NULL;

	if (param->api == EAIF_API_CLASSIFY || param->api == EAIF_API_HUMAN_CLASSIFY) {
		instance->set_local_info = classifySetLocalInfo;
		instance->merge_result = classifyMergeResult;
		instance->update = classifyUpdate;
	} else if (param->api == EAIF_API_DETECT || param->api == EAIF_API_FACEDET) {
		instance->set_local_info = detectSetLocalInfo;
		instance->merge_result = detectMergeResult;
		instance->update = detectUpdate;
	} else if (param->api == EAIF_API_FACERECO) {
		instance->set_local_info = detectSetLocalInfo;
		instance->merge_result = faceRecoMergeResult;
		instance->update = faceRecoUpdate;
	}

	// de/activate functors
	instance->init_algo = baseInitAlgo;
	instance->init_module = remoteInitModule;
	instance->exit_module = remoteExitModule;
	instance->release_buffer = baseReleaseBuffer;
	return 0;
}
