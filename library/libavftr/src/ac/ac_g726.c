/* The code and the comment are ported from audio/speech_codec with below minor modifications
 * 1. Remove unused functions and variables
 * 2. Use C structure instead of C++ implementation
 */
#include "ac_g726.h"

#include "ac.h"
#include "ac_codec.h"
#include <assert.h>
#include <stdlib.h>
#include "mpi_errno.h"

AC_CODEC_G726_S g_g726_enc;
AC_CODEC_G726_S g_g726_dec;

//Brief:  Addition of scale factor to logarithmic version of quantized difference signal.
//Input:  [DQLN] log2(normalized quanitized diff), 12-bit LSB
//        [Y] estimated signal, 13-bit LSB
//Output: [DQL] log2(quanitized diff), 12-bit LSB
static int adda(int dqln, int y)
{
	return dqln + (y >> 2);
}

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
//Brief:  Compute difference signal
//Input:  [SL/SLX] input linear PCM signal, 14-bit LSB TC
//        [SE] estimated signal, 15-bit LSB TC
//Output: [D/DX] difference signal, 16-bit TC
static int subta(int sl, int se)
{
	return (sl - se);
}

//Brief:  Scale logarithmic version of difference signal by subtracting scale factor Y.
//Input:  [D/DX] difference signal, 16-bit TC
//        [Y] quantizer scale factor, 13-bit LSB SM
//Output: [DLN/DLNX] log2(normalized difference signal), 12-bit LSB TC
static int subtb(int dlexp, int dlmant, int y)
{
	return ((dlexp << 7) + dlmant) - (y >> 2);
}

//Brief:  Convert difference signal from the linear to the logarithmic domain.
//Input:  [D/DX] difference signal, 16-bit TC
//Output: [DLEXP] exponential part of DL signal, 4-bit LSB
//        [DS] sign of D signal, 1-bit LSB
static void _log(int d, int *ds, int *dlexp, int *dlmant)
{
	if (d < 0) {
		*ds = 1;
		*dlexp = log2_c(-d);
		*dlmant = ((-d << 7) >> *dlexp) & LSB_7BIT;
	} else {
		*ds = 0;
		*dlexp = log2_c(d);
		*dlmant = ((d << 7) >> *dlexp) & LSB_7BIT;
	}
}

//Brief:  Quantize difference signal in logarithmic domain.
//Input:  [DS/DSX] sign, 1-bit LSB
//        [DLN/DLNX] difference signal, 12-bit
//Output: [I] ADPCM output
static pcm8 quan(AC_CODEC_G726_S *codec, int dln, int ds)
{
	int i = 0;

	switch (codec->m_bitWidth) {
	case 2:
		while (quant_tbl16[i] < INT_MAX && quant_tbl16[i] < dln) {
			++i;
		}
		break;
	case 4:
		while (quant_tbl32[i] < INT_MAX && quant_tbl32[i] < dln) {
			++i;
		}
		break;
	default:
		assert(0);
		break;
	}

	if (ds) {
		i = ~i;
	}
	if (codec->m_bitWidth != 2 && i == 0) {
		i = 0xff;
	}

	return i & ((1 << codec->m_bitWidth) - 1);
}
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

//Brief:  Convert quantized difference signal from the logarithmic to the linear domain.
//Input:  [DQL] log2(quanitized diff), 12-bit LSB
//Output: [DQ] quantized diff signal, 16-bit SM
static int antilog(int dql)
{
	int ret = 0;
	int dex = (dql >> 7) & LSB_4BIT; // 4bit exponent
	int dqt = (1 << 7) + (dql & LSB_7BIT); // log2 -> linear

	ret = (dql < 0) ? 0 : ((dqt << dex) >> 7);

	return ret;
}

//Brief:  Reconstruction of quantized difference signal in the logarithmic domain.
//Input:  [I] ADPCM output, pcm8 format
//Output: [DQLN] log2(normalized quantized difference), 12-bit LSB
static int reconst(AC_CODEC_G726_S *codec)
{
	int I = (int)codec->m_i;
	switch (codec->m_bitWidth) {
	case 2:
		return iquant_tbl16[I];
		break;
	case 4:
		return iquant_tbl32[I];
		break;
	default:
		assert(0);
		return -1;
		break;
	}
}

//Brief:  Map quantizer output into the F(I) function
//Input:  [I] ADPCM output, pcm8 format
//Output: [FI] Output of F(I), 3-bit LSB
static int functf(AC_CODEC_G726_S *codec)
{
	int I = (int)codec->m_i;

	switch (codec->m_bitWidth) {
	case 2:
		return F_tbl16[I];
		break;
	case 4:
		return F_tbl32[I];
		break;
	default:
		assert(0);
		return -1;
		break;
	}
}

//Brief:  Map quantizer output into logarithmic version of scale factor multiplier.
//Input:  [I] ADPCM output, pcm8 format
//Output: [WI] Quantizer multiplier, 12-bit LSB
static int functw(AC_CODEC_G726_S *codec)
{
	int I = (int)codec->m_i;
	switch (codec->m_bitWidth) {
	case 2:
		return W_tbl16[I];
		break;
	case 4:
		return W_tbl32[I];
		break;
	default:
		assert(0);
		return -1;
		break;
	}
}

//Brief:  Form linear combination of fast and slow quantizer scale factors.
//Input:  [AL] limited speed control parameter, 7-bit LSB SM
//        [YU] delayed fast quantizer scale factor, 13-bit LSB SM
//        [YL] delayed slow quantizer scale factor, 19-bit LSB SM
//Output: [Y] Quantizer scale factor, 13-bit LSB SM
static int mix(int al, int yu, int yl)
{
	return (yl + (yu - (yl >> 6)) * al) >> 6;
}

//Brief:  Update of fast quantizer scale factor.
//Input:  [WI] Quantizer multiplier, 12-bit LSB TC
//        [Y] Quantizer scale factor, 13-bit LSB SM
//Output: [YUT] Unlimited fast quantizer scale factor, 13-bit LSB SM
static int filtd(int wi, int y)
{
	return y + wi + (-y >> 5);
}

//Brief: Addition of predictor outputs to form the
//       partial signal estimate (from the sixth order predictor)
//       and the signal estimate.
//Input: [WA1, WA2, WB1, WB2, WB3, WB4, WB5, WB6] 16-bit
//Output:[SE] estimated signal, 15-bit LSB TC
//       [SEZ] sixth order predictor partial signal estimate, 15-bit TC
static void accum(AC_CODEC_G726_S *codec)
{
	Float11 f;

	codec->m_se = 0;
	for (int i = 0; i < 6; i++) {
		codec->m_se += mult(i2f(codec->b[i] >> 2, &f), &codec->dqDelay[i]);
	}
	codec->m_sez = codec->m_se >> 1;
	for (int i = 0; i < 2; i++) {
		codec->m_se += mult(i2f(codec->a[i] >> 2, &f), &codec->srDelay[i]);
	}
	codec->m_se >>= 1;
}

//Brief:  Partial band signal detection.
//Input:  [A2P] second order predictor coeff., 16-bit LSB
//Output: [TDP] tone detect, 1-bit LSB
static int tone(AC_CODEC_G726_S *codec)
{
	return (codec->a[1] < -11776) ? 1 : 0;
}

//Brief:  Transition detector.
//Input:  [TD] delayed tone select, 1-bit LSB
//        [YL] delayed slow quantizer scale factor, 19-bit LSB SM
//        [DQ] quantized difference signal, 16-bit SM
//Output: [TR] transition detect, 1-bit LSB
static int trans(int td, int yl, int dq)
{
	int ylint, ylfrac, thr2, ret;
	ylint = (yl >> 15);
	ylfrac = (yl >> 10) & LSB_5BIT;
	thr2 = (ylint > 9) ? 0x1f << 10 : (0x20 + ylfrac) << ylint;
	ret = ((td == 1) && (dq > ((3 * thr2) >> 2)));

	return ret;
}

static void q_scale_factor_adaptation(AC_CODEC_G726_S *codec)
{
	int wi;
	int yut;

	wi = functw(codec);
	yut = filtd(wi, codec->m_y);

	codec->m_yuDelay = clip(yut, 544, 5120); // fast quantizer
	codec->m_ylDelay += codec->m_yuDelay + (-codec->m_ylDelay >> 6); // slow quantizer

	// Next iteration for Y //
	codec->m_y = mix(codec->m_al, codec->m_yuDelay, codec->m_ylDelay);
}

static void adaptation_speed_control(AC_CODEC_G726_S *codec)
{
	int fi = functf(codec);

	codec->m_dmsDelay += (fi << 4) + ((-codec->m_dmsDelay) >> 5); //F(I) short-term average
	codec->m_dmlDelay += (fi << 4) + ((-codec->m_dmlDelay) >> 7); //F(I) long-term average

	if (codec->m_tr) {
		codec->m_apDelay = 256;
	} else {
		codec->m_apDelay += (-codec->m_apDelay) >> 4;
		if ((codec->m_y <= 1535) || (codec->m_tdDelay == 1) ||
		    (abs((codec->m_dmsDelay << 2) - codec->m_dmlDelay) >= (codec->m_dmlDelay >> 3))) {
			codec->m_apDelay += 0x20;
		}
	}

	codec->m_al = (codec->m_apDelay >= 256) ? (1 << 6) : (codec->m_apDelay >> 2);
}

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
static void input_sig_conversion(AC_CODEC_G726_S *codec)
{
	//s16le to pcm8
	codec->m_sl = (int)codec->m_s16in >> 2;
}

static void signal_diff(AC_CODEC_G726_S *codec)
{
	codec->m_d = subta(codec->m_sl, codec->m_se);
}

static void adaptive_quantizer(AC_CODEC_G726_S *codec)
{
	int ds = 0;
	int dlexp = -1;
	int dlmant = -1;
	int dln = -1;
	//dl is separated into dexp and dmant
	_log(codec->m_d, &ds, &dlexp, &dlmant);
	dln = subtb(dlexp, dlmant, codec->m_y);
	codec->m_i = quan(codec, dln, ds);
}
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

static void inv_adaptive_quantizer(AC_CODEC_G726_S *codec)
{
	int dql = -1;
	int dqln = -1;

	dqln = reconst(codec);
	dql = adda(dqln, codec->m_y);
	codec->m_dq = antilog(dql);
}

static void adaptive_predictor(AC_CODEC_G726_S *codec)
{
	int pk0 = 0;
	int dq0 = 0;
	int fa1 = 0;

	int I_sig = codec->m_i >> (codec->m_bitWidth - 1);

	// Transition detector //
	codec->m_tr = trans(codec->m_tdDelay, codec->m_ylDelay, codec->m_dq);

	// Reconstruct sign SR //
	if (I_sig) {
		codec->m_dq = -codec->m_dq;
	}
	codec->m_reSignal = codec->m_se + codec->m_dq;

	// PK0, DQ0 //
	pk0 = (codec->m_sez + codec->m_dq) ? sgn(codec->m_sez + codec->m_dq) : 0;
	dq0 = codec->m_dq ? sgn(codec->m_dq) : 0;

	// Update second order predictor coefficient A2 and A1 //
	if (codec->m_tr) {
		codec->a[0] = 0;
		codec->a[1] = 0;
	} else {
		fa1 = clip_intp2((-codec->a[0] * codec->pk[0] * pk0) >> 5, 8);
		codec->a[1] = codec->a[1] + 128 * pk0 * codec->pk[1] + fa1 - (codec->a[1] >> 7);
		codec->a[1] = clip(codec->a[1], -12288, 12288);
		codec->a[0] += 64 * 3 * pk0 * codec->pk[0] - (codec->a[0] >> 8);
		codec->a[0] = clip(codec->a[0], -(15360 - codec->a[1]), 15360 - codec->a[1]);
	}

	// Update sixth order predictor coefficient B1 to B6 //
	if (codec->m_tr) {
		for (int i = 0; i < 6; i++) {
			codec->b[i] = 0;
		}
	} else {
		for (int i = 0; i < 6; i++) {
			codec->b[i] += 128 * dq0 * sgn(-codec->dqDelay[i].sign) - (codec->b[i] >> 8);
		}
	}

	// Update Dq and Sr and Pk //
	codec->pk[1] = codec->pk[0];
	codec->pk[0] = pk0 ? pk0 : 1;

	codec->srDelay[1] = codec->srDelay[0];
	i2f(codec->m_reSignal, &codec->srDelay[0]);

	for (int i = 5; i > 0; i--) {
		codec->dqDelay[i] = codec->dqDelay[i - 1];
	}
	i2f(codec->m_dq, &codec->dqDelay[0]);
	codec->dqDelay[0].sign = I_sig; /* Isn't it crazy ?!?! */

	// Tone detect //
	codec->m_tdDelay = tone(codec);

	// Update AL within speed control//
	adaptation_speed_control(codec);

	// Update Y //
	q_scale_factor_adaptation(codec);

	// Next iteration for SE and SEZ //
	accum(codec);

	codec->m_reSignal = clip(codec->m_reSignal << 2, -0xffff, 0xffff);
	codec->m_sr = (pcm16)codec->m_reSignal;
}

static void G726_reset(AC_CODEC_G726_S *codec)
{
	codec->m_bufferOccupancy = 0;
	codec->m_outBuf = 0;

	codec->m_s16in = 0; // encoder only

	codec->m_i = 0;
	codec->m_reSignal = 0;

	codec->m_d = 0;
	codec->m_sl = 0;
	codec->m_se = 0;
	codec->m_sez = 0;

	codec->m_dq = 0;
	codec->m_al = 0;
	codec->m_tr = 0;

	codec->m_apDelay = 0;
	codec->m_dmsDelay = 0;
	codec->m_dmlDelay = 0;
	codec->m_tdDelay = 0;
	codec->m_yuDelay = 544;
	codec->m_ylDelay = 34816;
	codec->m_y = 544;

	for (int i = 0; i < 2; i++) {
		codec->srDelay[i].exp = 0;
		codec->srDelay[i].sign = 0;
		codec->srDelay[i].mant = 1 << 5;
		codec->pk[i] = 1;
		codec->a[i] = 0;
	}
	for (int i = 0; i < 6; i++) {
		codec->dqDelay[i].exp = 0;
		codec->dqDelay[i].mant = 1 << 5;
		codec->dqDelay[i].sign = 0;
		codec->b[i] = 0;
	}
}

void G726_init(AUDIO_CODEC_TYPE_E raw, AUDIO_CODEC_TYPE_E bitstream)
{
	assert(raw == AUDIO_CODEC_TYPE_PCM16LE);

	switch (bitstream) {
	case AUDIO_CODEC_TYPE_G726_16_BE:
		g_g726_enc.m_bitWidth = 2;
		g_g726_dec.m_bitWidth = 2;
		g_g726_enc.decode_ratio = 8;
		g_g726_dec.decode_ratio = 8;
		g_g726_enc.little_endian = 0;
		g_g726_dec.little_endian = 0;
		break;
	case AUDIO_CODEC_TYPE_G726_16_LE:
		g_g726_enc.m_bitWidth = 2;
		g_g726_dec.m_bitWidth = 2;
		g_g726_enc.decode_ratio = 8;
		g_g726_dec.decode_ratio = 8;
		g_g726_enc.little_endian = 1;
		g_g726_dec.little_endian = 1;
		break;
	case AUDIO_CODEC_TYPE_G726_32_BE:
		g_g726_enc.m_bitWidth = 4;
		g_g726_dec.m_bitWidth = 4;
		g_g726_enc.decode_ratio = 4;
		g_g726_dec.decode_ratio = 4;
		g_g726_enc.little_endian = 0;
		g_g726_dec.little_endian = 0;
		break;
	case AUDIO_CODEC_TYPE_G726_32_LE:
		g_g726_enc.m_bitWidth = 4;
		g_g726_dec.m_bitWidth = 4;
		g_g726_enc.decode_ratio = 4;
		g_g726_dec.decode_ratio = 4;
		g_g726_enc.little_endian = 1;
		g_g726_dec.little_endian = 1;
		break;
	default:
		g_g726_enc.m_bitWidth = 2;
		g_g726_dec.m_bitWidth = 2;
		g_g726_enc.decode_ratio = 8;
		g_g726_dec.decode_ratio = 8;
		g_g726_enc.little_endian = 0;
		g_g726_dec.little_endian = 0;
		assert(0);
		break;
	}

	G726_reset(&g_g726_enc);
	G726_reset(&g_g726_dec);
}

static INT32 decode(AC_CODEC_G726_S *codec)
{
	inv_adaptive_quantizer(codec);
	adaptive_predictor(codec);
	return 0;
}

static int getOneData(AC_CODEC_G726_S *codec, pcm8 **bit_buffer)
{
	int read = 0;
	if (codec->m_bufferOccupancy == 0) {
		codec->m_inBuf = *(*bit_buffer)++;
		codec->m_bufferOccupancy = 8;
		read = 1;
	}
	codec->m_bufferOccupancy -= codec->m_bitWidth;
	if (!codec->little_endian) {
		codec->m_i = (codec->m_inBuf >> codec->m_bufferOccupancy) & ((1 << codec->m_bitWidth) - 1);
	} else {
		codec->m_i = (codec->m_inBuf >> (8 - codec->m_bitWidth - codec->m_bufferOccupancy)) &
		             ((1 << codec->m_bitWidth) - 1);
	}
	return read ? sizeof(pcm8) : 0;
}

INT32 G726_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw)
{
	pcm8 *cur_in = (pcm8 *)bit_buffer;
	pcm16 *cur_out = NULL;
	*size_of_raw = size_of_bit * g_g726_dec.decode_ratio;
	*raw_buffer = malloc(*size_of_raw);
	cur_out = (pcm16 *)*raw_buffer;
	while ((char *)cur_out < *raw_buffer + *size_of_raw) {
		getOneData(&g_g726_dec, &cur_in);
		decode(&g_g726_dec);
		//*cur_out++ = ((g_g726_dec.m_sr & 0xff) << 8) + ((g_g726_dec.m_sr & 0xff00) >> 8);
		*cur_out++ = g_g726_dec.m_sr;
	}

	return 0;
}

#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
static INT32 encode(AC_CODEC_G726_S *codec)
{
	input_sig_conversion(codec);
	signal_diff(codec);
	adaptive_quantizer(codec);
	inv_adaptive_quantizer(codec);
	adaptive_predictor(codec);
	return 0;
}

INT32 putOneData(AC_CODEC_G726_S *codec, pcm8 **bit_buffer)
{
	int write = 0;
	if (!codec->little_endian) {
		codec->m_outBuf += (codec->m_i << ((8 - codec->m_bufferOccupancy) - codec->m_bitWidth));
	} else {
		codec->m_outBuf += (codec->m_i << codec->m_bufferOccupancy);
	}
	codec->m_bufferOccupancy += codec->m_bitWidth;
	if (8 - codec->m_bufferOccupancy < codec->m_bitWidth) {
		*(*bit_buffer)++ = codec->m_outBuf;
		codec->m_outBuf = 0;
		codec->m_bufferOccupancy = 0;
		write = 1;
	}
	return write ? sizeof(pcm8) : 0;
}

INT32 G726_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit)
{
	const pcm16 *cur_in = (const pcm16 *)raw_buffer;
	pcm8 *cur_out = NULL;
	*size_of_bit = (size_of_raw + 1) / g_g726_enc.decode_ratio;
	*bit_buffer = malloc(*size_of_bit);
	cur_out = (pcm8 *)*bit_buffer;
	while ((char *)cur_in < raw_buffer + size_of_raw) {
		g_g726_enc.m_s16in = *cur_in++;
		encode(&g_g726_enc);
		putOneData(&g_g726_enc, &cur_out);
	}

	return 0;
}
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */