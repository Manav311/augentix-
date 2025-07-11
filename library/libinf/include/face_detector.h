#ifdef LIBEAI
#ifndef FACE_DETECTOR_H_
#define FACE_DETECTOR_H_

#include "mpi_index.h"
#include "inf_types.h"
#include "inf_model.h"
#include "od_event.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct eai_fd_context EaiFdContext;

EaiFdContext *EAI_FD_create(MPI_WIN win, int scene_width, int scene_height, int snapshot_width, int snapshot_height,
                            InfModelCtx *detector);
void EAI_FD_dispose(EaiFdContext *context);
void EAI_FD_publish(EaiFdContext *context, uint32_t sensitivity, bool with_snapshot, publish_unsubscribe unsubscribe);
EaiOdEventPublisher *EAI_FD_getPublisher(EaiFdContext *context);

#ifdef __cplusplus
}
#endif

#endif /* FACE_DETECTOR_H_ */
#endif /* LIBEAI */
