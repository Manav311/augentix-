#ifndef AC_TRC_H_
#define AC_TRC_H_

#include <stdio.h>

#define ac_fmt(fmt) "[AC] " fmt

#define ac_tr(fmt, args...) printf(ac_fmt(fmt), ##args)
#define ac_err(fmt, args...) ac_tr(fmt, ##args)
#define ac_warn(fmt, args...) ac_tr(fmt, ##args)
#define ac_info_h(fmt, args...) ac_tr(fmt, ##args)
#define ac_info_m(fmt, args...)
#define ac_info_l(fmt, args...)

#endif /* AC_TRC_H_ */
