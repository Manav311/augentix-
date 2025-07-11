#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @cond
*
* code fragment skipped by Doxygen.
*/
#include <math.h>

#include <string.h>
#include <pthread.h>

#include "mpi_base_types.h"
#include "mpi_errno.h"
#include "mpi_dev.h"
#include "mpi_iva.h"
#include "ac.h"
#include "ac_trc.h"
#include "ac_linear.h"
#include "ac_g726.h"
#include "ac_g711.h"

AcInfo g_ac_ctx = {
	.lock = PTHREAD_MUTEX_INITIALIZER,
	.param =
	        {
	                .codec = AUDIO_CODEC_TYPE_PCM16LE,
	        },
	.init = LINEAR_init,
	.decode = LINEAR_decode,
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
	.encode = LINEAR_encode,
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */
};

static INT32 checkAcParam(const AC_PARAM_S *param)
{
	if (param->codec <= AUDIO_CODEC_TYPE_NONE || param->codec >= AUDIO_CODEC_TYPE_NUM) {
		ac_warn("Audio codec type %d should be in the range of (%d, %d).\n", param->codec,
		        AUDIO_CODEC_TYPE_NONE, AUDIO_CODEC_TYPE_NUM);
		return -1;
	}

	return 0;
}

#ifdef UNIT_TEST
INT32 UT_checkAcParam(const AC_PARAM_S *param)
{
	return checkAcParam(param);
}
#endif /* UNIT_TEST */

static int setupCodec(AUDIO_CODEC_TYPE_E codec)
{
	switch (codec) {
	case AUDIO_CODEC_TYPE_PCM16LE:
		g_ac_ctx.init = LINEAR_init;
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
		g_ac_ctx.encode = LINEAR_encode;
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */
		g_ac_ctx.decode = LINEAR_decode;
		break;
	case AUDIO_CODEC_TYPE_PCMA:
	case AUDIO_CODEC_TYPE_PCMU:
		g_ac_ctx.init = G711_init;
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
		g_ac_ctx.encode = G711_encode;
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */
		g_ac_ctx.decode = G711_decode;
		break;
	case AUDIO_CODEC_TYPE_G726_16_BE:
	case AUDIO_CODEC_TYPE_G726_16_LE:
	case AUDIO_CODEC_TYPE_G726_32_BE:
	case AUDIO_CODEC_TYPE_G726_32_LE:
		g_ac_ctx.init = G726_init;
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
		g_ac_ctx.encode = G726_encode;
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */
		g_ac_ctx.decode = G726_decode;
		break;
	default:
		g_ac_ctx.init = LINEAR_init;
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
		g_ac_ctx.encode = LINEAR_encode;
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */
		g_ac_ctx.decode = LINEAR_decode;
		return -1;
	}

	g_ac_ctx.init(AUDIO_CODEC_TYPE_PCM16LE, codec);
	return 0;
}
/**
 * @endcond
 */

/**
 * @brief Set ac parameters.
 * @param[in] idx         audio device index.
 * @param[in] param       ac parameters.
 * @see AC_getParam()
 * @retval MPI_SUCCESS              success.
 * @retval MPI_ERR_NULL_POINTER     input pointer is NULL.
 * @retval MPI_ERR_INVALID_PARAM    invalid parameters.
 * @retval -1              unexpected fail.
 */
INT32 AC_setParam(MPI_DEV idx __attribute__((unused)), const AC_PARAM_S *param)
{
#ifdef DEBUG_AC_API
	ac_info_l("Enter %s.\n", __func__);
#endif
	if (param == NULL) {
		ac_warn("Pointer to the AC parameters should not be NULL.\n");
		return MPI_ERR_NULL_POINTER;
	}

	AcInfo *ac_ctx = &g_ac_ctx;
	AC_PARAM_S *local_param = &ac_ctx->param;

	if (checkAcParam(param)) {
		ac_warn("Fail to set AC parameters.\n");
		return MPI_ERR_INVALID_PARAM;
	}

	pthread_mutex_lock(&ac_ctx->lock);
	if (param->codec != local_param->codec) {
		if (setupCodec(param->codec)) {
			ac_warn("Fail to setup audio codec.\n");
			return -1;
		}
	}
	memcpy(local_param, param, sizeof(AC_PARAM_S));
	pthread_mutex_unlock(&ac_ctx->lock);

#ifdef DEBUG_AC_API
	ac_info_l("Leave %s.\n", __func__);
#endif
	return 0;
}

/**
 * @brief Get ac parameters.
 * @param[in] idx         audio device index.
 * @param[out] param      ac parameters.
 * @see AC_setParam()
 * @retval MPI_SUCCESS             success.
 * @retval MPI_ERR_NULL_POINTER    input pointer is NULL.
 * @retval -1             unexpected fail.
 */
INT32 AC_getParam(MPI_DEV idx __attribute__((unused)), AC_PARAM_S *param)
{
#ifdef DEBUG_AC_API
	ac_info_l("Enter %s.\n", __func__);
#endif

	if (param == NULL) {
		ac_warn("Pointer to the sound detection parameters should not be NULL.\n");
		return MPI_ERR_NULL_POINTER;
	}

	AcInfo *ac_ctx = &g_ac_ctx;
	AC_PARAM_S *local_param = &ac_ctx->param;

	pthread_mutex_lock(&ac_ctx->lock);
	memcpy(param, local_param, sizeof(AC_PARAM_S));
	pthread_mutex_unlock(&ac_ctx->lock);

#ifdef DEBUG_AC_API
	ac_info_l("Leave %s.\n", __func__);
#endif
	return 0;
}

/**
 * @brief Decode audio bitstream into raw audio
 * @param[in]  bit_buffer    encoded audio bitstream.
 * @param[in]  size_of_bit    encoded audio bitstream size.
 * @param[out]  raw_buffer    raw audio bits.
 * @param[out]  size_of_raw    raw audio size.
 * @see    none
 * @retval MPI_SUCCESS             success.
 * @retval MPI_ERR_NULL_POINTER    input pointer is NULL.
 * @retval -1             unexpected fail.
 */
INT32 AC_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw)
{
#ifdef DEBUG_AC_API
	ac_info_l("Enter %s.\n", __func__);
#endif

	if (bit_buffer == NULL) {
		ac_warn("Pointer to the encoded audio bitstream should not be NULL.\n");
		return MPI_ERR_NULL_POINTER;
	}

	if (size_of_raw == NULL) {
		ac_warn("Pointer to the size_of_bit should not be NULL.\n");
		return MPI_ERR_NULL_POINTER;
	}

	g_ac_ctx.decode(bit_buffer, size_of_bit, raw_buffer, size_of_raw);

#ifdef DEBUG_AC_API
	ac_info_l("Leave %s.\n", __func__);
#endif
	return 0;
}

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
/**
 * @brief Encode raw audio into audio bitstream
 * @param[in]  raw_buffer    raw audio bits.
 * @param[in]  size_of_raw    raw audio size.
 * @param[out]  bit_buffer    encoded audio bitstream.
 * @param[out]  size_of_bit    encoded audio bitstream size.
 * @see    none
 * @retval MPI_SUCCESS             success.
 * @retval MPI_ERR_NULL_POINTER    input pointer is NULL.
 * @retval -1             unexpected fail.
 */
INT32 AC_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit)
{
#ifdef DEBUG_AC_API
	ac_info_l("Enter %s.\n", __func__);
#endif

	if (raw_buffer == NULL) {
		ac_warn("Pointer to the raw audio should not be NULL.\n");
		return MPI_ERR_NULL_POINTER;
	}

	if (size_of_bit == NULL) {
		ac_warn("Pointer to the size_of_bit should not be NULL.\n");
		return MPI_ERR_NULL_POINTER;
	}

	g_ac_ctx.encode(raw_buffer, size_of_raw, bit_buffer, size_of_bit);

#ifdef DEBUG_AC_API
	ac_info_l("Leave %s.\n", __func__);
#endif
	return 0;
}
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */