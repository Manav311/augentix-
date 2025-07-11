#ifndef LOG_DEFINE_H
#define LOG_DEFINE_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#include "syslog.h"

#ifdef TUTK_SERVICE_DEBUG
#define tutkservice_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[Debug] " fmt, ##args)
#else
#define tutkservice_log_debug(fmt, args...)
#endif

//#define tutkservice_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[Info] " fmt, ##args)
//#define tutkservice_log_warn(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[Warn] " fmt, ##args)
//#define tutkservice_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[Error] " fmt, ##args)
#define tutkservice_log_info(fmt, ...) printf("[Info][%s]" fmt, __func__, ##__VA_ARGS__)
#define tutkservice_log_warn(fmt, ...) printf("[Warn][%s]" fmt, __func__, ##__VA_ARGS__)
#define tutkservice_log_err(fmt, ...) printf("[Error][%s]" fmt, __func__, ##__VA_ARGS__)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
