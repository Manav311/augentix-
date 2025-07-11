#ifndef INF_DETECT_H_
#define INF_DETECT_H_

#include "inf_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "inf_model.h"

int Inf_InvokeDetect(InfModelCtx *ctx, const InfImage *img, InfDetList *result);
int Inf_InvokeDetectObjList(InfModelCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *ol, InfDetList *result);

#ifdef __cplusplus
}
#endif

#endif /* INF_DETECT_H_ */
