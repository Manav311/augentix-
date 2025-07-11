#ifndef AC_UT_COMMON_H
#define AC_UT_COMMON_H

#ifdef UNIT_TEST
#include "ut.h"
#include "ac.h"
#include "ac_trc.h"

#define AC_UT_DEG 0

#if AC_UT_DEG
#define AC_UT_INFO(fmt, args...) ac_tr(fmt, ##args)
#else
#define AC_UT_INFO(fmt, args...)
#endif

#endif /* !UNIT_TEST */

#endif /* !AC_UT_COMMON_H */
