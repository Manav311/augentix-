/* The code and the comment are ported from audio/speech_codec with below minor modifications
 * 1. Remove unused functions and variables
 * 2. Use C structure instead of C++ implementation
 */
#include "ac_g711.h"

#include "ac.h"
#include "ac_codec.h"
#include <assert.h>
#include <stdlib.h>
//#include <string.h>
#include "mpi_errno.h"

AC_CODEC_G711_S g_g711_codec;
#define SIGN_BIT (0x80) /* Sign bit for a A-law byte. */
#define QUANT_MASK (0xf) /* Quantization field mask. */
#define SEG_SHIFT (4) /* Left shift for segment number. */
#define SEG_MASK (0x70) /* Segment field mask. */
#define BIAS (0x84) /* Bias for linear code. */

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
static short seg_end[8] = { 0xFF, 0x1FF, 0x3FF, 0x7FF, 0xFFF, 0x1FFF, 0x3FFF, 0x7FFF };

static int search(int val, short *table, int size)
{
	int i;
	for (i = 0; i < size; i++) {
		if (val <= *table++) {
			return (i);
		}
	}
	return (size);
}
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

void G711_init(AUDIO_CODEC_TYPE_E raw, AUDIO_CODEC_TYPE_E bitstream)
{
	assert(raw == AUDIO_CODEC_TYPE_PCM16LE);

	switch (bitstream) {
	case AUDIO_CODEC_TYPE_PCMA:
		g_g711_codec.pcma = 1;
		//g_g711_dec.decode_ratio = 2;
		break;
	case AUDIO_CODEC_TYPE_PCMU:
		g_g711_codec.pcma = 0;
		break;
	default:
		assert(0);
		break;
	}

	g_g711_codec.decode_ratio = 2;
	//g_g711_dec.decode_ratio = 2;
}

static INT32 decode(AC_CODEC_G711_S *codec, pcm8 pcm_val, pcm16 *lpcm_val)
{
	int t;
	if (codec->pcma) {
		int seg;
		pcm_val ^= 0x55;

		t = (pcm_val & QUANT_MASK) << 4;
		seg = ((unsigned)pcm_val & SEG_MASK) >> SEG_SHIFT;
		switch (seg) {
		case 0:
			t += 8;
			break;
		case 1:
			t += 0x108;
			break;
		default:
			t += 0x108;
			t <<= seg - 1;
			break;
		}
		*lpcm_val = ((pcm_val & SIGN_BIT) ? t : -t);
	} else {
		/* Complement to obtain normal u-law value. */
		pcm_val = ~pcm_val;

		/*
		* Extract and bias the quantization bits. Then
		* shift up by the segment number and subtract out the bias.
		*/
		t = ((pcm_val & QUANT_MASK) << 3) + BIAS;
		t <<= ((unsigned)pcm_val & SEG_MASK) >> SEG_SHIFT;

		*lpcm_val = ((pcm_val & SIGN_BIT) ? (BIAS - t) : (t - BIAS));
	}
	return 0;
}

INT32 G711_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw)
{
	pcm8 *cur_in = (pcm8 *)bit_buffer;
	pcm16 *cur_out = NULL;
	*size_of_raw = size_of_bit * g_g711_codec.decode_ratio;
	*raw_buffer = malloc(*size_of_raw);
	cur_out = (pcm16 *)*raw_buffer;
	while ((char *)cur_out < *raw_buffer + *size_of_raw) {
		decode(&g_g711_codec, *cur_in++, cur_out++);
	}

	return 0;
}

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
static INT32 encode(AC_CODEC_G711_S *codec, pcm16 lpcm_val, pcm8 *pcm_val)
{
	int mask;
	int seg;

	if (codec->pcma) {
		unsigned char aval;

		// Step 1: Convert two's complement to decent input format. //
		if (lpcm_val >= 0) {
			mask = 0xD5; /* sign (7th) bit = 1 */
		} else {
			mask = 0x55; /* sign bit = 0 */
			lpcm_val = ~lpcm_val;
		}
		//3-bit LSB truncation
		lpcm_val &= 0xfff8;

		// Step 2: Convert the scaled magnitude to segment number. //
		seg = search(lpcm_val, seg_end, 8);

		// Step 3: Combine the sign, segment, and quantization bits. //
		//even bit inversion: outVal XOR mask
		if (seg >= 8) { // out of range, return maximum value.
			*pcm_val = (0x7F ^ mask);
		} else {
			aval = seg << SEG_SHIFT;
			if (seg < 2) {
				aval |= (lpcm_val >> 4) & QUANT_MASK;
			} else {
				aval |= (lpcm_val >> (seg + 3)) & QUANT_MASK;
			}
			*pcm_val = (aval ^ mask);
		}
	} else {
		unsigned char uval;

		//2-bit LSB truncation
		lpcm_val &= 0xfffc;

		//clipped between 8158 ~ -8159
		if (lpcm_val > (8158 * 4)) {
			lpcm_val = 8158 * 4;
		} else if (lpcm_val < -(8159 * 4)) {
			lpcm_val = -8159 * 4;
		}

		/* Get the sign and the magnitude of the value. */
		if (lpcm_val < 0) {
			lpcm_val = ~(lpcm_val - BIAS);
			mask = 0x7F;
		} else {
			lpcm_val += BIAS;
			mask = 0xFF;
		}

		/* Convert the scaled magnitude to segment number. */
		seg = search(lpcm_val, seg_end, 8);

		/*
		* Combine the sign, segment, quantization bits;
		* and complement the code word.
		*/
		if (seg >= 8) { /* out of range, return maximum value. */
			*pcm_val = (0x7F ^ mask);
		} else {
			uval = (seg << 4) | ((lpcm_val >> (seg + 3)) & 0xF);
			*pcm_val = (uval ^ mask);
		}
	}
	return 0;
}

INT32 G711_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit)
{
	const pcm16 *cur_in = (const pcm16 *)raw_buffer;
	pcm8 *cur_out = NULL;
	*size_of_bit = (size_of_raw + 1) / g_g711_codec.decode_ratio;
	*bit_buffer = malloc(*size_of_bit);
	cur_out = (pcm8 *)*bit_buffer;
	while ((char *)cur_in < raw_buffer + size_of_raw) {
		encode(&g_g711_codec, *cur_in++, cur_out++);
	}

	return 0;
}
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */