#include "utils.h"

void *memcpy_d2n_unaligned(void *dst, const void *src, size_t size)
{
	uint8_t *dst_8 = (uint8_t *)dst;
	uint16_t *dst_16 = (uint16_t *)dst;
	uint8_t *src_8 = (uint8_t *)src;
	size_t pkg = 256;
	uint32_t buf[pkg / 4];
	uint8_t *buf_8 = (uint8_t *)buf;
	uint16_t *buf_16 = (uint16_t *)buf;
	size_t i;
	int phase;
	phase = (int)dst & 0x3; // mod 4
	// printf("[%s:%d] phase %d\n", __func__, __LINE__, phase);
	if ((int)src & 0x3) {
		for (i = 0; i < size; i++) {
			*dst_8 = *src_8;
			dst_8++;
			src_8++;
		}
	} else {
		switch (phase) {
		case 0: {
			memcpy(dst, src, size);
			break;
		}
		case 1:
		case 3: {
			while (size >= pkg) {
				memcpy(buf_8, src_8, pkg);
				src_8 += pkg;
				size -= pkg;
				/* copy by byte */
				for (i = 0; i < pkg; i++) {
					*dst_8 = buf_8[i];
					dst_8++;
				}
			}
			memcpy(buf_8, src_8, size);
			for (i = 0; i < size; i++) {
				*dst_8 = buf_8[i];
				dst_8++;
			}
			break;
		}
		case 2: {
			while (size >= pkg) {
				memcpy(buf_16, src_8, pkg);
				src_8 += pkg;
				size -= pkg;
				/* copy by 2 byte */
				for (i = 0; i < pkg / 2; i++) {
					*dst_16 = buf_16[i];
					dst_16++;
				}
			}
			memcpy(buf_16, src_8, size);
			for (i = 0; i < size / 2; i++) {
				*dst_16 = buf_16[i];
				dst_16++;
			}
			break;
		}
		default:
			break;
		}
	}
	return 0;
}
