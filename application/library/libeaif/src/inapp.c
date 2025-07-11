#include "app.h"

#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "mpi_enc.h"

#include "eaif.h"
#include "eaif_algo.h"
#include "eaif_log.h"
#include "eaif_utils.h"

extern int g_eaif_verbose;
extern int g_eaif_inf_debug;

static struct timespec start;

int inappProbe(void *ctx __attribute__((unused)))
{
	return 0;
}

int inappInitBuf(EaifInfo *info)
{
	if (!info->payload.ptr) {
		info->payload.inapp = (EaifRequestDataInApp *)calloc(sizeof(EaifRequestDataInApp), 1);
		if (!info->payload.inapp) {
			eaif_log_warn("Cannot alloc payload buffer on mode %s!", eaif_utilsGetModeChar(info->mode));
			return -ENOMEM;
		}
	}
	return 0;
}

int inappInit(void *ctx)
{
	int ret = 0;
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EAIF_PARAM_S *param = &instance->param;
	EaifInfo *info = &instance->info;

	do {
		pthread_testcancel();
		ret = inappInitBuf(info);
		if (ret) {
			usleep(EAIF_RUN_USLEEP);
		}
	} while (ret == -1);

	eaif_calcScaleFactor(param->snapshot_width, param->snapshot_height, &info->src_resoln, &info->scale_factor);
	eaif_assignFrameInfo(param->snapshot_width, param->snapshot_height, &instance->info.payload.inapp->frame_info);

	return ret;
}

int inappSetPayload(void *ctx)
{
	if (g_eaif_verbose)
		TI_TIC(start);

	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;
	MPI_IVA_OBJ_LIST_S *local_list = &info->local_list;

	int ret = eaif_utilsSetupRequestInapp(local_list, param, info, info->payload.inapp);

	if (g_eaif_verbose)
		TI_TOC("Setup Payload", start);

	return ret;
}

// int inappSendRequest(void *ctx)
// {
// 	struct timespec req_time;

// 	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S*)ctx;
// 	EaifInfo *info = &instance->info;
// 	InferenceModel *model = &instance->model;

// 	if (g_eaif_verbose)
// 		TI_TIC(req_time);

// 	int ret = eaif_utilsSendRequestInApp(info, model);

// 	if (g_eaif_verbose)
// 		TI_TOC("Total Resp", req_time);

// 	return ret;
// }

int inappInitModule(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	const EAIF_PARAM_S *p = &instance->param;
	InferenceModel *model = &instance->model;
	int ret = 0;

	if (p->data_fmt == EAIF_DATA_JPEG || p->data_fmt == EAIF_DATA_MPI_JPEG) {
		eaif_log_debug("User request JPEG data format."
		               "Initialize MPI BitStream system and create channel.");
		MPI_initBitStreamSystem();
		instance->snapshot_chn = MPI_ENC_CHN(3);
		instance->bchn = MPI_createBitStreamChn(instance->snapshot_chn);
	} else {
		instance->snapshot_chn.value = CHN_UNINIT;
		instance->bchn.value = CHN_UNINIT;
	}

#ifdef EAIF_INFERENCE_INAPP
	if (!model) {
		eaif_log_warn("Null model input!");
		return -1;
	}

	ret = Inf_InitModel(&model->ctx, model->path);
	if (g_eaif_verbose) {
		Inf_Setup(&model->ctx, 1, 0, 1);
		eaif_log_info("Setup Model verbose!");
	}

	if (g_eaif_inf_debug) {
		Inf_Setup(&model->ctx, 0, 1, 1);
		eaif_log_info("Setup Model Inf capture!");
	}
#endif // EAIF_INFERENCE_INAPP

	return ret;
}

int inappExitModule(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	InferenceModel *model = &instance->model;
	const EAIF_PARAM_S *p = &instance->param;

	int ret = 0;
	if (p->data_fmt == EAIF_DATA_JPEG || p->data_fmt == EAIF_DATA_MPI_JPEG) {
		eaif_log_debug("Destroy BitStream channel and exit MPI BitStream system.");
		MPI_destroyBitStreamChn(instance->bchn);
		MPI_exitBitStreamSystem();
	}

#ifdef EAIF_INFERENCE_INAPP
	if (model->ctx.model) {
		ret = Inf_ReleaseModel(&model->ctx);
		model->ctx.model = 0;
	}
#endif // EAIF_INFERENCE_INAPP

	return ret;
}