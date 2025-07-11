#ifndef CONNSEL_LOG_H
#define CONNSEL_LOG_H


#include "agtx_log.h"


#define connsel_initLog(ident)         agtx_initAppLog(ident)
#define connsel_exitLog()              agtx_exitAppLog()

#define connsel_info(fmt, ...)         agtx_app_info(fmt, ##__VA_ARGS__)
#if CONNSEL_DEBUG
#define connsel_debug(fmt, ...)        agtx_app_debug(fmt, ##__VA_ARGS__)
#else
#define connsel_debug(fmt, ...)
#endif


#endif /* !CONNSEL_LOG_H */
