#ifndef SAMPLE_PUBLISHER_H
#define SAMPLE_PUBLISHER_H

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include <stdbool.h>
#include "mpi_index.h"
#include "sample_stream.h"

typedef struct bit_stream_subscriber {
  void *context;
  /* return true means abort delivery */
  bool (*deliveryWillStart)(void *context);
  /* return true means subscriber want to unsubscribe */
  bool (*receiveFrame)(void *context, const MPI_STREAM_PARAMS_V2_S *frame);
  void (*deliveryDidEnd)(void *context);
} BitStreamSubscriber;

int SAMPLE_startStreamPublisher(MPI_ECHN encoder_channel, const CONF_BITSTREAM_PARAM_S *conf,
                                INT32 reservation_level, INT32 recycle_level);
void SAMPLE_shutdownStreamPublisher(MPI_ECHN idx);
void SAMPLE_signalAllStreamThreadToShutdown(void);
bool SAMPLE_hasAnyPublisherThreadActive(void);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif //SAMPLE_PUBLISHER_H
