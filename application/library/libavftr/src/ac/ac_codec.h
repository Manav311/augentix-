#ifndef AC_CODEC_H_
#define AC_CODEC_H_

#include <stdint.h>
#include <limits.h>

typedef short pcm16;
typedef unsigned char pcm8;

typedef struct Float11 {
	uint8_t sign; /**< 1bit sign */
	uint8_t exp; /**< 4bit exponent */
	uint8_t mant; /**< 6bit mantissa */
} Float11;

//Integer mask:
#define LSB_17BIT 0x1ffff
#define LSB_19BIT 0x7ffff
//PCM16 masks:
#define LSB_1BIT 0x1
#define LSB_3BIT 0x7
#define LSB_4BIT 0xf
#define LSB_5BIT 0x1f
#define LSB_6BIT 0x3f
#define LSB_7BIT 0x7f
#define LSB_8BIT 0xff
#define LSB_9BIT 0x1ff
#define LSB_10BIT 0x3ff
#define LSB_11BIT 0x7ff
#define LSB_12BIT 0xfff
#define LSB_13BIT 0x1fff
#define LSB_14BIT 0x3fff
#define LSB_15BIT 0x7fff
#define LSB_16BIT 0xffff

static const uint8_t log2_tab[256] = {
	0, 0, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7
};

/**< 32kbit/s 4bits per sample */
static const int quant_tbl32[] = { -125, 79, 177, 245, 299, 348, 399, INT_MAX };
static const int16_t iquant_tbl32[] = { -2048, 4, 135, 213, 273, 323, 373, 425, 425, 373, 323, 273, 213, 135, 4, -2048 };
static const int16_t W_tbl32[] = { -12, 18, 41, 64, 112, 198, 355, 1122, 1122, 355, 198, 112, 64, 41, 18, -12 };
static const uint8_t F_tbl32[] = { 0, 0, 0, 1, 1, 1, 3, 7, 7, 3, 1, 1, 1, 0, 0, 0 };

/**< 16kbit/s 2bits per sample */
static const int quant_tbl16[] = { 260, INT_MAX };
static const int16_t iquant_tbl16[] = { 116, 365, 365, 116 };
static const int16_t W_tbl16[] = { -22, 439, 439, -22 };
static const uint8_t F_tbl16[] = { 0, 7, 7, 0 };

Float11 *i2f(int i, Float11 *f);
int16_t mult(Float11 *f1, Float11 *f2);
int sgn(int value);
int clip_intp2(int a, int p);
int clip(int a, int amin, int amax);
int log2_c(unsigned int v);

#endif /* !AC_CODEC_H_ */