#include "app.h"

#include "eaif.h"
#include "eaif_algo.h"

int initAppCb(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;
	EAIF_PARAM_S *param = &instance->param;

	int ret = 0;
	if (info->mode == EAIF_MODE_REMOTE) {
		ret = initRemoteCb(ctx);
	} else if (info->mode == EAIF_MODE_INAPP &&
	           (param->api == EAIF_API_CLASSIFY || param->api == EAIF_API_HUMAN_CLASSIFY)) {
		ret = initInappClassifyCb(ctx);
	} else if (info->mode == EAIF_MODE_INAPP && (param->api == EAIF_API_DETECT || param->api == EAIF_API_FACEDET)) {
		ret = initInappDetectCb(ctx);
	} else if (info->mode == EAIF_MODE_INAPP && (param->api == EAIF_API_FACERECO)) {
		//ret = initInappFaceRecoCb(ctx);
		ret = initInappFaceRecoCbV2(ctx);
	} else {
		// unknown configuration
		ret = -1;
	}
	return ret;
}

int delAppCb(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	instance->probe = NULL;
	instance->init_buffer = NULL;
	instance->set_local_info = NULL;
	instance->set_payload = NULL;
	instance->send_request = NULL;
	instance->decode_result = NULL;
	instance->debug = NULL;
	instance->merge_result = NULL;

	instance->init_algo = NULL;
	instance->init_module = NULL;
	instance->exit_module = NULL;
	instance->release_buffer = NULL;

	instance->update = NULL;
	return 0;
}
