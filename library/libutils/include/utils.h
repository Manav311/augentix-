#ifndef _utils_h_
#define _utils_h_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

void *memcpy_d2n_unaligned(void *dst, const void *src, size_t size);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif
