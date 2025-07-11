#ifndef AVFTR_LOG_H_
#define AVFTR_LOG_H_

#include <syslog.h>

#ifdef __cplusplus
extern "C" {
#endif

#define avftr_log_err(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_ERR), "[AVFTR][Error] " fmt, ##__VA_ARGS__)
#define avftr_log_warn(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_WARNING), "[AVFTR][Warning] " fmt, ##__VA_ARGS__)
#define avftr_log_notice(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_NOTICE), "[AVFTR][Notice] " fmt, ##__VA_ARGS__)
#define avftr_log_info(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_INFO), "[AVFTR][Info] " fmt, ##__VA_ARGS__)
#define avftr_log_debug(fmt, ...) syslog(LOG_MAKEPRI(LOG_LOCAL7, LOG_DEBUG), "[AVFTR][Debug] " fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* LOG_H_ */