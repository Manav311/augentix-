#ifndef GPIO_MOTOR_H_
#define GPIO_MOTOR_H_

#ifdef __cplusplus
extern "C" {
#endif /**< __cplusplus */

#include "libmotor.h"
#include "gpio.h"

#define MAX_GPIO_CNT (4)

typedef struct gpio_motor {
	Gpio x_gpio[MAX_GPIO_CNT]; /*< GPIO for operating the motor to rotate the X-axis */
	Gpio y_gpio[MAX_GPIO_CNT]; /*< GPIO for operating the motor to rotate the Y-axis*/
	float step_angle; /*< Minimum step angle of motor hardware */
	float ptz_speed_factor[AXIS_NUM]; /*< The inverse of the angle needed to move the tracking box by one pixel.
	                                      list by [X,Y,Z] axis*/
} GpioMotor;

MotorData *newGpioMotor(const MotorLimit *limit, const GpioMotor *gpio);
int deleteGpioMotor(MotorData *data);

#ifdef __cplusplus
}
#endif /**< __cplusplus */

#endif