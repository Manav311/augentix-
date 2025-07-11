#include "inf_detect.h"

#include <stdio.h>

#include "inf_model.h"
#include "inf_model_internal.h"
#include "inf_log.h"

extern "C" int Inf_InvokeDetect(InfModelCtx *ctx, const InfImage *img, InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && result);

	static_cast<InfModel*>(ctx->model)->Detect(img, result);

	return 0;
}

extern "C" int Inf_InvokeDetectObjList(InfModelCtx *ctx, const InfImage *img,
                                       const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result)
{
	retIfNull(ctx && ctx->model && img && ol && result);

	static_cast<InfModel*>(ctx->model)->Detect(img, ol, result);

	return 0;
}
