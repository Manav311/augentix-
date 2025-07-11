#ifndef __platforms_h__
#define __platforms_h__

#include "basedef.h"

/* Errors */
#define SA_QUEUE_ERR_TIMEOUT 0
#define SA_QUEUE_ERR_OTHER -2

#if defined(WIN32) //Windows

#include "plat_win32.h"

#elif defined(__LINUX__) || defined(__ANDROID__) || defined(__MAC_OS__) || defined(__CYGWIN__)

#include "plat_linux.h"

#elif defined(__ALI_OS__)

#include "plat_alios.h"
//#include "net_lwip.h"

#elif defined(__UCOS2__)

#include "plat_ucos2.h"
#include "net_lwip.h"
#define __NO_FS__

#elif defined(__HI3861__) || defined(__AIW4211__)

//#include "plat_hi3861.h"
#include "plat_liteos.h"
#include "net_lwip.h"
#define __NO_FS__
#define __MCU__

#elif defined(__LITEOS__)

#include "plat_liteos.h"
#include "net_lwip.h"

#elif defined(__FREERTOS__)

#include "plat_freertos.h"
#include "net_lwip.h"
#define __NO_FS__

#elif defined(__JL_AC57__)

#include "plat_jlac57.h"
#include "net_lwip.h"
#define __NO_FS__

#elif defined(__JL_AC79__)

#include "plat_jlac79.h"
#include "net_lwip.h"
#define __NO_FS__

#elif defined(__GP_CV4247__)

#include "plat_gpcv4247.h"
#include "net_lwip.h"
#define __NO_FS__

#else

#error "Platform must be specified !"

#endif

#ifdef __cplusplus
extern "C" {
#endif

char *My_strdup(const char *s);
char *My_strndup(const char *s, int len);

void *My_malloc(size_t size);
void *My_calloc(size_t nmemb, size_t size);
void *My_realloc(void *ptr, size_t size);
void My_free(void *ptr);

#ifndef NO_TIMEZONE_SUPPORT
#define SA_localtime_r(t_ptr, tm_ptr) localtime_r(t_ptr, tm_ptr)
#define SA_mktime(tm_ptr) mktime(tm_ptr)
#else
extern long _tg_timezone_;
struct tm *SA_localtime_r(const time_t *t, struct tm *result);
#endif

#ifdef NO_GETTIMEOFDAY_SUPPORT
int SA_gettimeofday(struct timeval *tv, struct timezone *z);
#else
#define SA_gettimeofday(tv, tz) gettimeofday(tv, tz)
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
