#ifndef INF_LOG_H_
#define INF_LOG_H_

#ifdef USE_TFLITE
#ifdef USE_MICROINF
#error Cannot enable tflite and microinf at the same time!
#endif // USE_MICROINF
#endif // USE_TFINF

#define ENABLE_INFO_LOG 1
#define ENABLE_DEBUG_LOG 0

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdio.h>
#include <syslog.h>
#include <sys/time.h>
#include <time.h>

#define DEB(fmt, args...) printf("%s %d " fmt " \n", __func__, __LINE__, ##args)

#define inf_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[INF][Error] " fmt, ##args)
#define inf_log_warn(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[INF][Warning] " fmt, ##args)
#define inf_log_notice(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[INF][Notice] " fmt, ##args)
#if ENABLE_INFO_LOG
#define inf_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[INF][Info] " fmt, ##args)
#else
#define inf_log_info(fmt, args...)
#endif
#if ENABLE_DEBUG_LOG
#define inf_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[INF][Debug] " fmt, ##args)
#else
#define inf_log_debug(fmt, args...)
#endif

#define inf_check(cond)                                                                  \
	do {                                                                             \
		if (!(cond)) {                                                           \
			inf_log_err("%s %d assert fail %s!", __func__, __LINE__, #cond); \
		}                                                                        \
	} while (0)

#define logThenRetIf(cond, msg, err)          \
	do {                              \
		if ((cond)) {             \
			inf_log_err(msg); \
			return (err);     \
		}                         \
	} while (0)

#define retIfNull(ptr)                                          \
	do {                                                       \
		if (!(ptr)) {                                      \
			inf_log_err("Input args cannot be NULL."); \
			return -EFAULT;                            \
		}                                                  \
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

#endif /** INF_LOG_H_ */