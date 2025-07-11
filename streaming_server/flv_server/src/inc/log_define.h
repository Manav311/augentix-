#ifndef LOG_DEFINE_H_
#define LOG_DEFINE_H_

#include <syslog.h>

//#define FLV_DEBUG
#define flv_server_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[Error] " fmt, ##args)
#define flv_server_log_warn(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[Warning] " fmt, ##args)
#define flv_server_log_notice(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[Notice] " fmt, ##args)
#ifdef FLV_DEBUG
#define flv_server_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[Info] " fmt, ##args)
#define flv_server_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[Debug] " fmt, ##args)
#else
#define flv_server_log_info(fmt, args...)
#define flv_server_log_debug(fmt, args...)
#endif

#endif