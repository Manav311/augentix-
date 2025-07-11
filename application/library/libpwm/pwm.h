#ifndef PWM_H
#define PWM_H

typedef unsigned char Value;
typedef unsigned char Period;
typedef unsigned char DutyCycle;

#define PWM_LOW 0
#define PWM_HIGH 1

typedef enum { FLOODLIGHT_TRIGGER_MODE, FLOODLIGHT_TRACING_MODE, PIR_TRIGGER_MODE } PIR_NOTIFY_EVENT_E;

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
int getPWMEnable(PWM *pwm);
int getPWMDutyCycle(PWM *pwm);
int getPWMPeriod(PWM *pwm);
#endif /* PWM_H */
