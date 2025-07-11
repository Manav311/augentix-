#ifndef EAIF_LOG_H_
#define EAIF_LOG_H_

#include <stdio.h>
#include <syslog.h>
#include <sys/stat.h>
#include <time.h>

#define EAIF_DEBUG_INFO (0)

#define eaif_log(lv, fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, lv), "[EAIF]" fmt, ##__VA_ARGS__)

#define eaif_log_notice(fmt, ...) \
	eaif_log(LOG_NOTICE,      \
	         "[Notice]"       \
	         "[%s:%d] " fmt,  \
	         __func__, __LINE__, ##__VA_ARGS__)
#define eaif_log_info(fmt, ...)
#define eaif_log_debug(fmt, ...)
#define eaif_log_warn(fmt, ...)  \
	eaif_log(LOG_WARNING,    \
	         "[Warning]"     \
	         "[%s:%d] " fmt, \
	         __func__, __LINE__, ##__VA_ARGS__)
#define eaif_log_err(fmt, ...)   \
	eaif_log(LOG_ERR,        \
	         "[Error]"       \
	         "[%s:%d] " fmt, \
	         __func__, __LINE__, ##__VA_ARGS__)

// #define EAIF_ENTRY
#ifdef EAIF_ENTRY
#define eaif_log_entry()                           \
	syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), \
	       "[EAIF]"                            \
	       "%s:%d entry",                      \
	       __func__, __LINE__)
#define eaif_log_exit()                            \
	syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), \
	       "[EAIF]"                            \
	       "%s:%d exit",                       \
	       __func__, __LINE__)
#else
#define eaif_log_entry()
#define eaif_log_exit()
#endif

// #define EAIF_ALGO_DEBUG
#ifdef EAIF_ALGO_DEBUG
#define EAIF_ALGO_FMT(fmt) "[ALGO] " fmt
#define eaif_algo_log(fmt, ...) printf((fmt), ##__VA_ARGS__)
#define eaif_algo_int(var) static int var = 0
#define eaif_algo_exp(var) var
#else
#define eaif_algo_log(fmt, ...)
#define eaif_algo_int(var)
#define eaif_algo_exp(var) 0
#endif /* !eaif_log_debug */

#define TI_TIC(start) clock_gettime(CLOCK_MONOTONIC_RAW, &start)

#define TI_TOC(str, start)                                                                               \
	do {                                                                                             \
		struct timespec end;                                                                     \
		uint64_t delta_us;                                                                       \
		float delta_s;                                                                           \
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);                                                \
		delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000; \
		delta_s = (float)delta_us / 1000000;                                                     \
		printf("%s Elapsed time: %.8f (s).\n", str, delta_s);                                    \
	} while (0)

#endif /* !EAIF_LOG_H_ */
