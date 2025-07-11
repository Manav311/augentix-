#include "ac_codec.h"

int16_t mult(Float11 *f1, Float11 *f2)
{
	int res, exp;

	exp = f1->exp + f2->exp;
	res = (((f1->mant * f2->mant) + 0x30) >> 4);
	res = exp > 19 ? res << (exp - 19) : res >> (19 - exp);
	return (f1->sign ^ f2->sign) ? -res : res;
}

int sgn(int value)
{
	return (value < 0) ? -1 : 1;
}

int clip_intp2(int a, int p)
{
	if ((a + (1 << p)) & ~((2 << p) - 1))
		return (a >> 31) ^ ((1 << p) - 1);
	else
		return a;
}

int clip(int a, int amin, int amax)
{
	if (a < amin)
		return amin;
	else if (a > amax)
		return amax;
	else
		return a;
}

Float11 *i2f(int i, Float11 *f)
{
	f->sign = (i < 0);
	if (f->sign)
		i = -i;
	f->exp = log2_c(i) + !!i;
	f->mant = i ? (i << 6) >> f->exp : 1 << 5;
	return f;
}

int log2_c(unsigned int v)
{
	int n = 0;
	if (v & 0xffff0000) {
		v >>= 16;
		n += 16;
	}
	if (v & 0xff00) {
		v >>= 8;
		n += 8;
	}
	n += log2_tab[v];
	return n;
}