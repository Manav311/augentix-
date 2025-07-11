#include "app.h"

#include <stdlib.h>

#include "eaif_algo.h"

int baseInitAlgo(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifAlgo *algo = &instance->algo;
	EaifInfo *info = &instance->info;

	info->algo = algo;
	algo->p = &instance->param;
	algo->face_preserve_prev = false;
	algo->face_det_roi = false;

	return 0;
}

int baseReleaseBuffer(void *ctx)
{
	EAIF_ALGO_STATUS_S *instance = (EAIF_ALGO_STATUS_S *)ctx;
	EaifInfo *info = &instance->info;

	if (info->payload.ptr)
		free(info->payload.ptr);

	info->payload.ptr = NULL;

	return 0;
}
