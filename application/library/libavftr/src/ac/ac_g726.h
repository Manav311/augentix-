#ifndef AC_G726_H_
#define AC_G726_H_

#include "ac.h"
#include "ac_codec.h"
#include <stdint.h>

typedef struct {
	int decode_ratio;
	/*ADPCM encoder only*/
	pcm16 m_s16in;
	//Common I/O buffers
	int m_bufferOccupancy; //output buffer counter
	pcm8 m_inBuf;
	pcm8 m_outBuf;

	/*ADPCM common */
	int m_bitWidth;
	int little_endian;

	// I/O and reconstruct signals
	pcm8 m_i;
	pcm16 m_sr;

	int m_d;
	int m_sl;
	int m_se;
	int m_sez;
	int m_y;
	int m_dq;
	int m_al;
	int m_tr;
	int m_reSignal;

	//Delay memory
	int m_apDelay;
	int m_yuDelay;
	int m_ylDelay;
	int m_dmsDelay;
	int m_dmlDelay;
	int m_tdDelay;
	int a[2];
	int b[6];
	int pk[2];
	Float11 srDelay[2];
	Float11 dqDelay[6];
} AC_CODEC_G726_S;

void G726_init(AUDIO_CODEC_TYPE_E type_raw, AUDIO_CODEC_TYPE_E bitstream);
INT32 G726_decode(const char *bit_buffer, int size_of_bit, char **raw_buffer, int *size_of_raw);
#if defined(UNIT_TEST_ON_TARGET) || defined(UNIT_TEST)
INT32 G726_encode(const char *raw_buffer, int size_of_raw, char **bit_buffer, int *size_of_bit);
#endif /* UNIT_TEST_ON_TARGET || UNIT_TEST */

#endif /* !AC_G726_H_ */