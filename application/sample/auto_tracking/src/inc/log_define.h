#ifndef LOG_DEFINE_H_
#define LOG_DEFINE_H_

#include <syslog.h>

#define AUTO_TRACKING_DEBUG
#define auto_tracking_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[Error] " fmt, ##args)
#define auto_tracking_log_warn(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[Warning] " fmt, ##args)
#define auto_tracking_log_notice(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[Notice] " fmt, ##args)
#define auto_tracking_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[Info] " fmt, ##args)
#define auto_tracking_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[Debug] " fmt, ##args)

#endif
