/******************************************************************************
*
* Copyright (c) Augentix Inc. - All Rights Reserved
*
* Unauthorized copying of this file, via any medium is strictly prohibited.
*
* Proprietary and confidential.
*
******************************************************************************/

#ifndef AC_H_
#define AC_H_

/** @cond
*/
#include "mpi_base_types.h"
#include "mpi_iva.h"
#include <pthread.h>

/** @endcond
*/

typedef enum {
	AUDIO_CODEC_TYPE_NONE,
	AUDIO_CODEC_TYPE_PCM16LE,
	AUDIO_CODEC_TYPE_PCMA,
	AUDIO_CODEC_TYPE_PCMU,
	AUDIO_CODEC_TYPE_G726_16_BE,
	AUDIO_CODEC_TYPE_G726_16_LE,
	AUDIO_CODEC_TYPE_G726_32_BE,
	AUDIO_CODEC_TYPE_G726_32_LE,
	AUDIO_CODEC_TYPE_NUM
} AUDIO_CODEC_TYPE_E;

/**
 * @brief Struct for audio codec parameter
 */
typedef struct {
	AUDIO_CODEC_TYPE_E codec; /**< minimal OSC aspect ratio */
} AC_PARAM_S;

/** @cond
*/
typedef struct {
	pthread_mutex_t lock;
	AC_PARAM_S param;
	void (*init)(AUDIO_CODEC_TYPE_E raw, AUDIO_CODEC_TYPE_E bitstream);
	INT32 (*decode)(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw);
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
	INT32 (*encode)(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit);
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */
} AcInfo;
/** @endcond
*/

INT32 AC_setParam(MPI_DEV idx, const AC_PARAM_S *param);
INT32 AC_getParam(MPI_DEV idx, AC_PARAM_S *param);
INT32 AC_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw);
#ifdef UNIT_TEST
INT32 UT_checkAcParam(const AC_PARAM_S *param);
#endif /* UNIT_TEST */
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
INT32 AC_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit);
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* AC_H_ */
