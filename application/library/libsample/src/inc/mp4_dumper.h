#ifndef MP4_DUMPER_H
#define MP4_DUMPER_H

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "sample_publisher.h"
#include <stdint.h>
#ifdef MP4_ENABLE

BitStreamSubscriber *newMp4Dumper(MPI_ECHN encoder_channel, const char *output_path, uint32_t frame_count, bool repeat,
                                  int32_t reservation_level, int32_t recycle_level, int max_dumped_files,
                                  uint16_t width, uint16_t height, uint8_t fps);

#endif
#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif //MP4_DUMPER_H
