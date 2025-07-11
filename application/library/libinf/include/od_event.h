#ifdef LIBEAI
#ifndef OD_EVENT_H_
#define OD_EVENT_H_

#include <stdint.h>
#include <stdbool.h>

#include "mpi_base_types.h"
#include "mpi_index.h"
#include "mpi_iva.h"
#include "inf_types.h"

#include "json.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct od_event {
	uint32_t timestamp;
	int32_t id;
	int16_t life;
	MPI_RECT_POINT_S rect;
	MPI_MOTION_VEC_S mv;
	InfImage *snapshot;
	json_object *attrs;
} OdEvent;

typedef struct eai_od_context EaiOdContext;
typedef struct eai_od_event_publisher EaiOdEventPublisher;

typedef bool (*publish_unsubscribe)(void);
typedef void (*on_od_event)(void *context, const OdEvent *event);

/* void start_od_service(MPI_WIN win, MPI_IVA_OD_PARAM_S *param, publish_unsubscribe unsubscribe, on_od_event handler); */

EaiOdContext *EAI_OD_create(MPI_WIN win);
void EAI_OD_dispose(EaiOdContext *context);
void EAI_OD_publish(EaiOdContext *context, MPI_IVA_OD_PARAM_S *param, publish_unsubscribe unsubscribe);

EaiOdEventPublisher *EAI_OD_getPublisher(EaiOdContext *context);
void EAI_OD_subscribe(EaiOdEventPublisher *publisher, on_od_event handler, void *handler_context);

#ifdef __cplusplus
}
#endif

#endif /* OD_EVENT_H_ */
#endif /* LIBEAI */
