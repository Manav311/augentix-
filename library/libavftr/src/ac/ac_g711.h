#ifndef AC_G711_H_
#define AC_G711_H_

#include "ac.h"
#include "ac_codec.h"

typedef struct {
	int decode_ratio;
	int pcma;
} AC_CODEC_G711_S;

void G711_init(AUDIO_CODEC_TYPE_E type_raw, AUDIO_CODEC_TYPE_E bitstream);
INT32 G711_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw);
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
INT32 G711_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit);
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

#endif /* !AC_G711_H_ */