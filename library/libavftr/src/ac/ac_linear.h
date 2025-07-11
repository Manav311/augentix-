#ifndef AC_LINEAR_H_
#define AC_LINEAR_H_

#include "ac.h"
#include "ac_codec.h"

void LINEAR_init(AUDIO_CODEC_TYPE_E type_raw, AUDIO_CODEC_TYPE_E bitstream);
INT32 LINEAR_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw);
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
INT32 LINEAR_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit);
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

#endif /* !AC_LINEAR_H_ */