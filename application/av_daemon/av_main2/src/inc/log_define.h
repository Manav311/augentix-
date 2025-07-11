#ifndef LOG_DEFINE_H_
#define LOG_DEFINE_H_

#include <syslog.h>

//#define AVMAIN2_DEBUG
#ifdef AVMAIN2_DEBUG
#define avmain2_log_debug(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[Debug] " fmt, ##args)
#else
#define avmain2_log_debug(fmt, args...)
#endif

#define avmain2_log_info(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[Info] " fmt, ##args)
#define avmain2_log_notice(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[Notice] " fmt, ##args)
#define avmain2_log_warn(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[Warn] " fmt, ##args)
#define avmain2_log_err(fmt, args...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[Error] " fmt, ##args)

#endif