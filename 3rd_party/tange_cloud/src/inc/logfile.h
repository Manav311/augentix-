#ifndef __logfile_h__
#define __logfile_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "platforms.h"

/** Control the output of log.
 * if OUTPUT_TO_CONSOLE is specified, {fpLog} in Logf()/LogfN()/LogBin() should be _STDERR_ or stdout
 * if OUTPUT_TO_FILE is specified, {path} is the file to write log to. if max_file_size is not zero, log file will scrollback when reach the max size
 */
#define LOGF_OPT_OUTPUT_TO_FILE 0x01
#define LOGF_OPT_OUTPUT_TO_CONSOLE 0x02
void LogfConfig(int opts, const char *path, int max_file_size /*bytes*/);

typedef void (*LOGFCALLBACK)(const char *s, int len);
void LogfSetCallback(LOGFCALLBACK cb);

void LogfClose(void);

#define LOGLEVEL_ERR 1
#define LOGLEVEL_WARN 2
#define LOGLEVEL_OK 3
#define LOGLEVEL_INFO 4 //normal
#define LOGLEVEL_VERBOSE 5
#define LOGLEVEL_DEBUG 6
#define LOG_APPEND 0x8000
void LogfSetLevel(int lvl);

int LogfGetLevel(void);

void Logf(int level, SA_FILE *fpLog, const char *who, const char *fmt, ...);
void LogfN(int level, SA_FILE *fpLog, const char *who, const char *s, int len);

void LogBin(int level, SA_FILE *fpLog, const char *who, const char *title, const void *p, int size);

#ifdef __LITEOS__
#define _STDERR_ stdout
#define _STDOUT_ stdout
#elif defined(__JL_AC57__) || defined(__JL_AC79__)
#define _STDERR_ NULL
#define _STDOUT_ ((SA_FILE *)1)
#else
#define _STDERR_ stderr
#define _STDOUT_ stdout
#endif

#ifdef WIN32
#define _err(fmt, ...)                                                      \
	do {                                                                \
		Logf(LOGLEVEL_ERR, stderr, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _warn(fmt, ...)                                                      \
	do {                                                                 \
		Logf(LOGLEVEL_WARN, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _ok(fmt, ...)                                                      \
	do {                                                               \
		Logf(LOGLEVEL_OK, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _info(fmt, ...)                                                      \
	do {                                                                 \
		Logf(LOGLEVEL_INFO, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogE(fmt, ...)                                                      \
	do {                                                                \
		Logf(LOGLEVEL_ERR, stderr, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogI(fmt, ...)                                                       \
	do {                                                                 \
		Logf(LOGLEVEL_INFO, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogV(fmt, ...)                                                          \
	do {                                                                    \
		Logf(LOGLEVEL_VERBOSE, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)

#define _err_append(fmt, ...)                                                            \
	do {                                                                             \
		Logf(LOG_APPEND | LOGLEVEL_ERR, stderr, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _warn_append(fmt, ...)                                                            \
	do {                                                                              \
		Logf(LOG_APPEND | LOGLEVEL_WARN, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _ok_append(fmt, ...)                                                              \
	do {                                                                              \
		Logf(LOG_APPEND | LOGLEVEL_WARN, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _info_append(fmt, ...)                                                            \
	do {                                                                              \
		Logf(LOG_APPEND | LOGLEVEL_INFO, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogE_append(fmt, ...)                                                            \
	do {                                                                             \
		Logf(LOG_APPEND | LOGLEVEL_ERR, stderr, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogI_append(fmt, ...)                                                             \
	do {                                                                              \
		Logf(LOG_APPEND | LOGLEVEL_INFO, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogV_append(fmt, ...)                                                                \
	do {                                                                                 \
		Logf(LOG_APPEND | LOGLEVEL_VERBOSE, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)

#define _err_nl()                                                            \
	do {                                                                 \
		Logf(LOG_APPEND | LOGLEVEL_ERR, stderr, __FUNCTION__, "\n"); \
	} while (0)
#define _warn_nl()                                                            \
	do {                                                                  \
		Logf(LOG_APPEND | LOGLEVEL_WARN, stdout, __FUNCTION__, "\n"); \
	} while (0)
#define _ok_nl()                                                            \
	do {                                                                \
		Logf(LOG_APPEND | LOGLEVEL_OK, stdout, __FUNCTION__, "\n"); \
	} while (0)
#define _info_nl()                                                            \
	do {                                                                  \
		Logf(LOG_APPEND | LOGLEVEL_INFO, stdout, __FUNCTION__, "\n"); \
	} while (0)
#define LogE_nl()                                                            \
	do {                                                                 \
		Logf(LOG_APPEND | LOGLEVEL_ERR, stderr, __FUNCTION__, "\n"); \
	} while (0)
#define LogV_nl()                                                                \
	do {                                                                     \
		Logf(LOG_APPEND | LOGLEVEL_VERBOSE, stdout, __FUNCTION__, "\n"); \
	} while (0)

#else //gnu gcc

#define _err(fmt, args...)                                               \
	do {                                                             \
		Logf(LOGLEVEL_ERR, _STDERR_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define _warn(fmt, args...)                                               \
	do {                                                              \
		Logf(LOGLEVEL_WARN, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define _ok(fmt, args...)                                               \
	do {                                                            \
		Logf(LOGLEVEL_OK, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define _info(fmt, args...)                                               \
	do {                                                              \
		Logf(LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogE(fmt, args...)                                               \
	do {                                                             \
		Logf(LOGLEVEL_ERR, _STDERR_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogI(fmt, args...)                                                \
	do {                                                              \
		Logf(LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogV(fmt, args...)                                                   \
	do {                                                                 \
		Logf(LOGLEVEL_VERBOSE, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)

#define _err_append(fmt, args...)                                                     \
	do {                                                                          \
		Logf(LOG_APPEND | LOGLEVEL_ERR, _STDERR_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define _ok_append(fmt, args...)                                                     \
	do {                                                                         \
		Logf(LOG_APPEND | LOGLEVEL_OK, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define _info_append(fmt, args...)                                                     \
	do {                                                                           \
		Logf(LOG_APPEND | LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogE_append(fmt, args...)                                                     \
	do {                                                                          \
		Logf(LOG_APPEND | LOGLEVEL_ERR, _STDERR_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogI_append(fmt, args...)                                                      \
	do {                                                                           \
		Logf(LOG_APPEND | LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogV_append(fmt, args...)                                                         \
	do {                                                                              \
		Logf(LOG_APPEND | LOGLEVEL_VERBOSE, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)

#define _err_nl()                                                              \
	do {                                                                   \
		Logf(LOG_APPEND | LOGLEVEL_ERR, _STDERR_, __FUNCTION__, "\n"); \
	} while (0)
#define _warn_nl()                                                              \
	do {                                                                    \
		Logf(LOG_APPEND | LOGLEVEL_WARN, _STDOUT_, __FUNCTION__, "\n"); \
	} while (0)
#define _ok_nl()                                                              \
	do {                                                                  \
		Logf(LOG_APPEND | LOGLEVEL_OK, _STDOUT_, __FUNCTION__, "\n"); \
	} while (0)
#define _info_nl()                                                              \
	do {                                                                    \
		Logf(LOG_APPEND | LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, "\n"); \
	} while (0)
#define LogE_nl()                                                              \
	do {                                                                   \
		Logf(LOG_APPEND | LOGLEVEL_ERR, _STDERR_, __FUNCTION__, "\n"); \
	} while (0)
#define LogI_nl()                                                               \
	do {                                                                    \
		Logf(LOG_APPEND | LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, "\n"); \
	} while (0)
#define LogV_nl()                                                                  \
	do {                                                                       \
		Logf(LOG_APPEND | LOGLEVEL_VERBOSE, _STDOUT_, __FUNCTION__, "\n"); \
	} while (0)
#endif

#define _err_bin(text, data, size) LogBin(LOGLEVEL_ERR, _STDERR_, __FUNCTION__, text, data, size)
#define _warn_bin(text, data, size) LogBin(LOGLEVEL_WARN, _STDOUT_, __FUNCTION__, text, data, size)
#define _ok_bin(text, data, size) LogBin(LOGLEVEL_OK, _STDOUT_, __FUNCTION__, text, data, size)
#define _info_bin(text, data, size) LogBin(LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, text, data, size)

#define _err_n(s, len) LogfN(LOGLEVEL_ERR, _STDERR_, __FUNCTION__, s, len)
#define _warn_n(s, len) LogfN(LOGLEVEL_WARN, _STDOUT_, __FUNCTION__, s, len)
#define _ok_n(s, len) LogfN(LOGLEVEL_OK, _STDOUT_, __FUNCTION__, s, len)
#define _info_n(s, len) LogfN(LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, s, len)

#define _err_n_append(s, len) LogfN(LOG_APPEND | LOGLEVEL_ERR, _STDERR_, __FUNCTION__, s, len)
#define _warn_n_append(s, len) LogfN(LOG_APPEND | LOGLEVEL_WARN, _STDOUT_, __FUNCTION__, s, len)
#define _ok_n_append(s, len) LogfN(LOG_APPEND | LOGLEVEL_OK, _STDOUT_, __FUNCTION__, s, len)
#define _info_n_append(s, len) LogfN(LOG_APPEND | LOGLEVEL_INFO, _STDOUT_, __FUNCTION__, s, len)

#ifdef _DEBUG

#ifdef WIN32
#define _dbg(fmt, ...)                                                        \
	do {                                                                  \
		Logf(LOGLEVEL_DEBUG, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define _DBG(x) x
#define LogD(fmt, ...)                                                        \
	do {                                                                  \
		Logf(LOGLEVEL_DEBUG, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogD_append(fmt, ...)                                                              \
	do {                                                                               \
		Logf(LOG_APPEND | LOGLEVEL_DEBUG, stdout, __FUNCTION__, fmt, __VA_ARGS__); \
	} while (0)
#define LogD_nl()                                                              \
	do {                                                                   \
		Logf(LOG_APPEND | LOGLEVEL_DEBUG, stdout, __FUNCTION__, "\n"); \
	} while (0)
#elif defined(__ANDROID__)
#include "platforms.h"
#define _dbg LOG
#define _DBG(x) x
#else
#define _dbg(fmt, args...)                                                 \
	do {                                                               \
		Logf(LOGLEVEL_DEBUG, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define _DBG(x) x
#define LogD(fmt, args...)                                                 \
	do {                                                               \
		Logf(LOGLEVEL_DEBUG, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogD_append(fmt, args...)                                                       \
	do {                                                                            \
		Logf(LOG_APPEND | LOGLEVEL_DEBUG, _STDOUT_, __FUNCTION__, fmt, ##args); \
	} while (0)
#define LogD_nl()                                                                \
	do {                                                                     \
		Logf(LOG_APPEND | LOGLEVEL_DEBUG, _STDOUT_, __FUNCTION__, "\n"); \
	} while (0)
#endif
#define _dbg_bin(text, data, size) LogBin(LOGLEVEL_DEBUG, _STDOUT_, __FUNCTION__, text, data, size)
#define _dbg_n(s, len) LogfN(LOGLEVEL_DEBUG, _STDOUT_, __FUNCTION__, s, len)

#else

#ifdef WIN32
#define _dbg(fmt, ...)
#define LogD(fmt, ...)
#define LogD_append(fmt, ...)
#define LogD_nl()
#else
#define _dbg(fmt, args...)
#define LogD(fmt, args...)
#define LogD_append(fmt, args...)
#define LogD_nl()
#endif
#define _dbg_bin(t, d, s)
#define _DBG(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
