#ifndef LOG_DEFINE_H_
#define LOG_DEFINE_H_

#include <syslog.h>

#define LIBMOTOR_DEBUG
#define libmotor_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[Error] " fmt, ##args)
#define libmotor_log_warn(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[Warning] " fmt, ##args)
#define libmotor_log_notice(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[Notice] " fmt, ##args)
#ifdef LIBMOTOR_DEBUG
#define libmotor_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[Info] " fmt, ##args)
#define libmotor_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[Debug] " fmt, ##args)
#else
#define libmotor_log_info(fmt, args...)
#define libmotor_log_debug(fmt, args...)
#endif

#endif
