#ifndef EAIF_TRC_H_
#define EAIF_TRC_H_

#ifdef __cplusplus
#include <chrono>
#include <iostream>
#include <string>
#endif

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define DEB(fmt, args...) printf("%s %d " fmt " \n", __func__, __LINE__, ##args)
#define TRC_NOT_IMPL(fmt, args...) printf("%s is not implemented! \n", __func__)

#define LOG(fmt) "[INFO] " fmt
#define ERR(fmt) "[ERROR] " fmt
#define WARN(fmt) "[WARNINGS] " fmt

#define EAIF_FAILURE (-1)
#define EAIF_SUCCESS (0)

#define eaif_trc(fmt, args...) printf(LOG(fmt), ##args)
#define eaif_warn(fmt, args...) printf(WARN(fmt), ##args)
#define eaif_err(fmt, args...) fprintf(stderr, ERR(fmt), ##args)
#define eaif_info_l(fmt, args...)
#define eaif_info_m(fmt, args...)
#define eaif_info_h(fmt, args...) eaif_trc(fmt, ##args)

#define eaif_check(cond)                                                                 \
	do {                                                                             \
		if (!(cond)) {                                                           \
			eaif_err("%s %d assert fail %s !\n", __func__, __LINE__, #cond); \
		}                                                                        \
	} while (0)

#define eaif_ele(x, y) printf(#x ":%" y " ", x)

#ifdef __cplusplus
#define TIMER(x)                                                            \
	do {                                                                \
		auto start = std::chrono::high_resolution_clock::now();     \
		(x);                                                        \
		auto end = std::chrono::high_resolution_clock::now();       \
		std::chrono::duration<double> elapsed = end - start;        \
		std::cout << "Elapsed time: " << elapsed.count() << " s\n"; \
	} while (0)
#endif

#define TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#ifdef __cpluscplus
#define TOC(str, start)                                                                                  \
	do {                                                                                             \
		struct timespec end;                                                                     \
		uint64_t delta_us;                                                                       \
		float delta_s;                                                                           \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                \
		delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; \
		delta_s = (float)delta_us / 1000000;                                                     \
		std::cout << str << " Elapsed time: " << delta_s << " s\n";                              \
	} while (0)
#else
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
#endif




#define TIMER_FUNC(str, x)             \
	do {                           \
		struct timespec start; \
		TIC(start);            \
		(x);                   \
		TOC(str, start);       \
	} while (0)

#define COND_TIMER_FUNC(cond, str, x)          \
	do {                                   \
		if (cond) {                    \
			struct timespec start; \
			TIC(start);            \
			(x);                   \
			TOC(str, start);       \
		} else {                       \
			(x);                   \
		}                              \
	} while (0)

#endif /* !EAIF_TRC_H_ */
