#ifndef UDP_CAST_H
#define UDP_CAST_H

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "sample_publisher.h"

BitStreamSubscriber *newUdpLiveStream(const char *dest_ip, int dest_port);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif //UDP_CAST_H
