#ifdef LIBEAI
#ifndef FACE_RECOGNIZER_H_
#define FACE_RECOGNIZER_H_

#include "mpi_index.h"
#include "inf_types.h"
#include "inf_model.h"
#include "od_event.h"

#ifdef __cplusplus
extern "C" {
#endif
extern const char *const ATTRIBUTE_ROOT;
extern const char *const IDENTITY_KEY;
extern const char *const ID_NAME_KEY;
extern const char *const ID_CONF_KEY;

typedef struct eai_fr_context EaiFrContext;

EaiFrContext *EAI_FR_create(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                            InfModelCtx *recognizer);
void EAI_FR_dispose(EaiFrContext *context);

void EAI_FR_startServiceWith(EaiFrContext *context, EaiOdEventPublisher *publisher, uint32_t sensitivity);
EaiOdEventPublisher *EAI_FR_getPublisher(EaiFrContext *context);
void EAI_FR_stopService(EaiFrContext *context);

void EAI_FR_setConfThreshold(EaiFrContext *context, float threshold);
void EAI_FR_setPriorityFactor(EaiFrContext *context, uint32_t once, uint32_t twice, uint32_t steady);
void EAI_FR_getPriorityFactor(EaiFrContext *context, uint32_t *once, uint32_t *twice, uint32_t *steady);

#ifdef __cplusplus
}
#endif

#endif /* FACE_RECOGNIZER_H_ */
#endif /* LIBEAI */
