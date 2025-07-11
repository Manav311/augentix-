#ifndef PWM_H
#define PWM_H

#include "agtx_types.h"

typedef unsigned char Value;

typedef enum {
	FLOODLIGHT_NONE,
	FLOODLIGHT_TRACING_MODE,
	FLOODLIGHT_ACTION_MODE,
	PIR_ACTION_MODE,
	PIR_TRIGGER_MODE
} PIR_NOTIFY_EVENT_E;

typedef struct {
	int enabled;
	int id;
	int period;
	int duty_cycle;
} PWM;

int exportPWM(Value v);
int enabledPWM(PWM *pmw, Value v);
int unexportPWM(Value v);
void setPWMPeriod(PWM *pwm, int period);
void setPWMDutyCycle(PWM *pww, int duty_cycle);
#endif /* PWM_H */
