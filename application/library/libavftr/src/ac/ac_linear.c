#include "ac_linear.h"

#include "ac.h"
#include "mpi_errno.h"
#include <stdlib.h>
#include <string.h>

void LINEAR_init(AUDIO_CODEC_TYPE_E type_raw __attribute__((unused)),
                 AUDIO_CODEC_TYPE_E bitstream __attribute__((unused)))
{
}

INT32 LINEAR_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw)
{
	*size_of_raw = size_of_bit;
	*raw_buffer = (char *)bit_buffer;
	return 0;
}

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
INT32 LINEAR_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit)
{
	*size_of_bit = size_of_raw;
	*bit_buffer = (char *)raw_buffer;
	return 0;
}

#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */