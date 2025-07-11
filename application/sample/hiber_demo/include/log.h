#ifndef LOG_H_
#define LOG_H_

#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#define log_err(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[Error] " fmt, ##__VA_ARGS__)
#define log_warn(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARN), "[Warning] " fmt, ##__VA_ARGS__)
#define log_notice(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[Notice] " fmt, ##__VA_ARGS__)
#define log_info(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[Info] " fmt, ##__VA_ARGS__)
#define log_debug(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[Debug] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */