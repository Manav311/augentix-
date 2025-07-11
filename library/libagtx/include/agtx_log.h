#ifndef AGTX_LOG_H
#define AGTX_LOG_H

#include "syslog.h"

#define agtx_initAppLog(ident)          openlog(ident, LOG_PID, LOG_LOCAL4)
#define agtx_exitAppLog()               closelog()

#define agtx_app_debug(fmt, ...)        syslog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define agtx_app_info(fmt, ...)         syslog(LOG_INFO, fmt, ##__VA_ARGS__)

#endif /* !AGTX_LOG_H */
