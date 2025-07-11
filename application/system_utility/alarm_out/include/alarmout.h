#ifndef ALARMOUT_H_
#define ALARMOUT_H_

#include <sys/time.h>
#include <stdint.h>
#include <limits.h>

//#define DEBUG
#ifdef DEBUG
#define debug(format, args...) printf("[%s:%d] "format, __FILE__, __LINE__, ##args)
#else
#define debug(args...)
#endif

typedef struct alarm_out_rule {
	int8_t         gpio;
	uint8_t        polarity;
	struct timeval duration;
	struct alarm_out_rule *next;
} AlarmOutRule;

#define ALARM_OUT_SOCKET "/tmp/ao"

#define GPIO_MAX     73
#define POLARITY_MAX 1
#define SEC_MAX      86400

#define GPO_PATH     "/sys/class/gpio/"
#endif /* ALARMOUT_H_ */
