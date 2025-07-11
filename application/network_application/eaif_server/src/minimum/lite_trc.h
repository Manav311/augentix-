#ifndef LITE_TRC_H_
#define LITE_TRC_H_

#ifdef USE_TFLITE
#ifdef USE_MICROLITE
#error Cannot enale tflite and microlite at the same time!
#endif // USE_MICROLITE
#endif // USE_TFLITE

#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>
#include <stdio.h>
#include <sys/time.h>

#define DEB(fmt, args...) printf("%s %d " fmt " \n", __func__, __LINE__, ##args)
#define TRC_NOT_IMPL(fmt, args...) printf("%s is not implemented! \n", __func__)

#define LLOG(fmt) "[INFO] " fmt
#define LERR(fmt) "[ERROR] " fmt
#define LWARN(fmt) "[WARNINGS] " fmt

#define lite_trc(fmt, args...) printf(LLOG(fmt), ##args)
#define lite_warn(fmt, args...) printf(LWARN(fmt), ##args)
#define lite_err(fmt, args...) fprintf(stderr, LERR(fmt), ##args)
#define lite_info_l(fmt, args...)
#define lite_info_m(fmt, args...)
#define lite_info_h(fmt, args...) lite_trc(fmt, ##args)

#define lite_check(cond)                                                                 \
	do {                                                                             \
		if (!(cond)) {                                                           \
			lite_err("%s %d assert fail %s !\n", __func__, __LINE__, #cond); \
		}                                                                        \
	} while (0)

#define TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#define TOC(str, start)                                                                                  \
	do {                                                                                             \
		struct timespec end;                                                                     \
		uint64_t delta_us;                                                                       \
		float delta_s;                                                                           \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                \
		delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; \
		delta_s = (float)delta_us / 1000000;                                                     \
		printf("%s Elapsed time: %.8f (s).\n", str, delta_s);                                    \
	} while (0)

#ifdef __cplusplus
}
#endif

#endif