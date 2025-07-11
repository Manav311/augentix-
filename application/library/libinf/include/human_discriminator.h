#ifdef LIBEAI
#ifndef HUMAN_DISCRIMINATOR_H_
#define HUMAN_DISCRIMINATOR_H_

#include "mpi_index.h"
#include "inf_types.h"
#include "inf_model.h"
#include "od_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eai_hd_context EaiHdContext;

EaiHdContext *EAI_HD_create(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                            InfModelCtx *classifier);
void EAI_HD_dispose(EaiHdContext *context);

void EAI_HD_startServiceWith(EaiHdContext *context, EaiOdEventPublisher *publisher);
EaiOdEventPublisher *EAI_HD_getPublisher(EaiHdContext *context);
void EAI_HD_stopService(EaiHdContext *context);
void EAI_HD_setPriorityFactor(EaiHdContext *context, uint32_t once, uint32_t twice, uint32_t steady);
void EAI_HD_getPriorityFactor(EaiHdContext *context, uint32_t *once, uint32_t *twice, uint32_t *steady);

#ifdef __cplusplus
}
#endif

#endif /* HUMAN_DISCRIMINATOR_H_ */
#endif /* LIBEAI */
