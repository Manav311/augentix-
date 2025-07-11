#ifndef LOG_DEFINE_H
#define LOG_DEFINE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

// #define OSD_DEBUG_P
#ifdef OSD_DEBUG_P
#define osddemo_log_debug(fmt, args...) printf("[OSDDEMO][Debug] " fmt, ##args)
#endif
#ifdef OSD_DEBUG
#define osddemo_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[OSDDEMO][Debug] " fmt, ##args)
#else
#define osddemo_log_debug(fmt, args...)
#endif

#define osddemo_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[OSDDEMO][Info] " fmt, ##args)
#define osddemo_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[OSDDEMO][Error] " fmt, ##args)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
