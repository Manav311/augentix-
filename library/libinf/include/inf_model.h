#ifndef INF_MODEL_H_
#define INF_MODEL_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>

#include "inf_types.h"

typedef struct inf_model InfModelCtx;

int Inf_InitModel(InfModelCtx *ctx, const char *config);
int Inf_ReleaseModel(InfModelCtx *ctx);
int Inf_ReleaseResult(InfResultList *result);
int Inf_ReleaseDetResult(InfDetList *result);
int Inf_Setup(InfModelCtx *ctx, int verbose, int debug, int num_thread);

#ifdef __cplusplus
}
#endif

#endif /* INF_MODEL_H_ */
