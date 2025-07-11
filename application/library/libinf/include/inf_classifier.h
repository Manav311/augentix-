#ifndef INF_CLASSIFIER_H_
#define INF_CLASSIFIER_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#include "inf_types.h"
#include "inf_model.h"

int Inf_InvokeClassify(InfModelCtx *ctx, const InfImage *img, const MPI_IVA_OBJ_LIST_S *obj_list,
                       InfResultList *result);

#ifdef __cplusplus
}
#endif

#endif
