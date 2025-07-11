#include "gpio_motor.h"

#include <linux/limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "log_define.h"

#define SINGLE_STEP_MOTOR_MOVE (8)
#define END_THETA 400.0 /** invalid degree */
DIRECTION_TYPE_E gpio_motor_default_coordination_system[AXIS_NUM] = { LEFT_DIR, UP_DIR, FORWARD_DIR };

static void verticalStep1(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep2(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_HIGH };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep3(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep4(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_HIGH, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep5(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));
	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep6(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));
	GpioValue val[4] = { GPIO_VAL_HIGH, GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep7(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));
	GpioValue val[4] = { GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}
static void verticalStep8(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));
	GpioValue val[4] = { GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH };

	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep1(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep2(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_HIGH };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep3(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep4(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_HIGH, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep5(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_LOW, GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep6(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_HIGH, GPIO_VAL_HIGH, GPIO_VAL_HIGH, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep7(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_LOW };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static void horizontalStep8(Gpio *param)
{
	Gpio motor_gpio[MAX_GPIO_CNT];
	memcpy(&motor_gpio[0], param, sizeof(motor_gpio));

	GpioValue val[4] = { GPIO_VAL_HIGH, GPIO_VAL_LOW, GPIO_VAL_LOW, GPIO_VAL_HIGH };
	for (int i = 0; i < 4; i++) {
		motor_gpio[i].value = val[i];
		GPIO_setGpioValue(&motor_gpio[i]);
	}
}

static int motorYForwardStep(Gpio *param, float sleep_ms)
{
	verticalStep1(param);
	usleep(sleep_ms * 1000);
	verticalStep2(param);
	usleep(sleep_ms * 1000);
	verticalStep3(param);
	usleep(sleep_ms * 1000);
	verticalStep4(param);
	usleep(sleep_ms * 1000);
	verticalStep5(param);
	usleep(sleep_ms * 1000);
	verticalStep6(param);
	usleep(sleep_ms * 1000);
	verticalStep7(param);
	usleep(sleep_ms * 1000);
	verticalStep8(param);
	usleep(sleep_ms * 1000);

	return 0;
}

static int motorYBackwardStep(Gpio *param, float sleep_ms)
{
	verticalStep8(param);
	usleep(sleep_ms * 1000);
	verticalStep7(param);
	usleep(sleep_ms * 1000);
	verticalStep6(param);
	usleep(sleep_ms * 1000);
	verticalStep5(param);
	usleep(sleep_ms * 1000);
	verticalStep4(param);
	usleep(sleep_ms * 1000);
	verticalStep3(param);
	usleep(sleep_ms * 1000);
	verticalStep2(param);
	usleep(sleep_ms * 1000);
	verticalStep1(param);
	usleep(sleep_ms * 1000);

	return 0;
}

static int motorXForwardStep(Gpio *param, float sleep_ms)
{
	horizontalStep1(param);
	usleep(sleep_ms * 1000);
	horizontalStep2(param);
	usleep(sleep_ms * 1000);
	horizontalStep3(param);
	usleep(sleep_ms * 1000);
	horizontalStep4(param);
	usleep(sleep_ms * 1000);
	horizontalStep5(param);
	usleep(sleep_ms * 1000);
	horizontalStep6(param);
	usleep(sleep_ms * 1000);
	horizontalStep7(param);
	usleep(sleep_ms * 1000);
	horizontalStep8(param);
	usleep(sleep_ms * 1000);

	return 0;
}

static int motorXBackwardStep(Gpio *param, float sleep_ms)
{
	horizontalStep8(param);
	usleep(sleep_ms * 1000);
	horizontalStep7(param);
	usleep(sleep_ms * 1000);
	horizontalStep6(param);
	usleep(sleep_ms * 1000);
	horizontalStep5(param);
	usleep(sleep_ms * 1000);
	horizontalStep4(param);
	usleep(sleep_ms * 1000);
	horizontalStep3(param);
	usleep(sleep_ms * 1000);
	horizontalStep2(param);
	usleep(sleep_ms * 1000);
	horizontalStep1(param);
	usleep(sleep_ms * 1000);

	return 0;
}

static int GPIO_motorInit(MotorData *data)
{
	Gpio motor_gpios[MAX_GPIO_CNT];
	memcpy(&motor_gpios[0], &(((GpioMotor *)data->private_data)->x_gpio), sizeof(motor_gpios));

	for (int i = 0; i < MAX_GPIO_CNT; i++) {
		motor_gpios[i].direction = GPIO_OUT;

		GPIO_initGpio(&motor_gpios[i]);
		GPIO_setGpioDirection(&motor_gpios[i]);
		GPIO_getGpioDirection(&motor_gpios[i]);
	}

	memcpy(&motor_gpios[0], &(((GpioMotor *)data->private_data)->y_gpio), sizeof(motor_gpios));
	for (int i = 0; i < MAX_GPIO_CNT; i++) {
		motor_gpios[i].direction = GPIO_OUT;

		GPIO_initGpio(&motor_gpios[i]);
		GPIO_setGpioDirection(&motor_gpios[i]);
		GPIO_getGpioDirection(&motor_gpios[i]);
	}

	return 0;
}

static int GPIO_motorDeinit(MotorData *data)
{
	Gpio motor_gpios[MAX_GPIO_CNT];
	memset(&motor_gpios[0], 0x00, sizeof(motor_gpios));
	memcpy(&motor_gpios[0], &(((GpioMotor *)data->private_data)->x_gpio), sizeof(motor_gpios));

	for (int i = 0; i < MAX_GPIO_CNT; i++) {
		GPIO_releaseGpio(&motor_gpios[i]);
	}

	memcpy(&motor_gpios[0], &(((GpioMotor *)data->private_data)->y_gpio), sizeof(motor_gpios));
	for (int i = 0; i < MAX_GPIO_CNT; i++) {
		GPIO_releaseGpio(&motor_gpios[i]);
	}

	return 0;
}

static int convertThetaToMotorSteps(GpioMotor *motor, float *rotate_theta)
{
	int theta_1000 = *rotate_theta * 1000; /**for rounding*/

	if (theta_1000 < 0) {
		theta_1000 = theta_1000 * -1;
	}

	int step_angle_1000 = (motor->step_angle) * 1000;

	return theta_1000 / step_angle_1000;
}

static float convertMotorSleepMs(MotorData *data, AXIS_TYPE_E axis, GpioMotor *motor)
{
	/** curr_velocity: us per degree*/
	float ms = ((motor->step_angle) * data->status.state[axis].curr_velocity) / (1000 * 8);
	if (ms < 0) {
		ms = (-1) * ms;
	}

	return ms;
}

static void calcActualTheta(MotorData *data, float *ideal, /*output*/ float *actual)
{
	float step_angle_1000 = ((GpioMotor *)(data->private_data))->step_angle * 1000;
	float ideal_1000 = *ideal * 1000;
	int times = ideal_1000 / step_angle_1000;
	*actual = (step_angle_1000 * times) / 1000;
}

static void logTheta(AXIS_TYPE_E axis, float *ideal, float *actual, float *rotate_theta)
{
	libmotor_log_info("axis[%d] ideal %f, actual %f, rotate: %f\n", axis, *ideal, *actual, *rotate_theta);
}

static int updateXYZAxisIdealPhysicalTheta(MotorData *data, AXIS_TYPE_E axis, float theta, float *rotate_theta)
{
	if (axis < 0 || axis > AXIS_NUM) {
		libmotor_log_err("Invalid axis: %d\n", axis);
		*rotate_theta = 0.0;
		return -EINVAL;
	}

	/** check coordination system*/
	if (data->limit.attr[axis].plus_coordinates == gpio_motor_default_coordination_system[axis]) {
		/** this motor coordination system same to default */
	} else {
		theta = (-1) * theta;
		libmotor_log_warn("axis[%d] invert coordination system\n", axis);
	}

	float *ideal = &data->status.state[axis].ideal_theta;
	float *actual = &data->status.state[axis].actual_theta;

	/** reset case: always rotate ideal theta */
	if (*ideal == END_THETA) {
		*ideal = theta;
		calcActualTheta(data, ideal, rotate_theta);
		*ideal = data->limit.attr[axis].max_theta;
		*actual = data->limit.attr[axis].max_theta;
		logTheta(axis, ideal, actual, rotate_theta);
		return 0;
	}

	/** end case: can't move */
	if ((*ideal >= data->limit.attr[axis].max_theta && theta > 0) ||
	    (*ideal <= data->limit.attr[axis].min_theta && theta < 0)) {
		libmotor_log_info("%s end case:\n", __func__);
		*rotate_theta = 0;
		logTheta(axis, ideal, actual, rotate_theta);
		return 0;
	}

	float rotate_ideal = 0.0;
	/** this move to end case */
	if (*ideal + theta > data->limit.attr[axis].max_theta || *ideal + theta < data->limit.attr[axis].min_theta) {
		libmotor_log_info("%s this move to end case:\n", __func__);
		if (theta > 0) {
			rotate_ideal = data->limit.attr[axis].max_theta - *ideal;
		} else {
			rotate_ideal = data->limit.attr[axis].min_theta - *ideal;
		}

		goto calc_rotate_theta;
	}

	/** normal case: need to cover up old actual & new ideal diff*/
	rotate_ideal = ((data->status.state[axis].ideal_theta + theta) - data->status.state[axis].actual_theta);

calc_rotate_theta:
	calcActualTheta(data, &rotate_ideal, rotate_theta);
	libmotor_log_notice("update axis[%d] %f %f\n", axis, theta, *rotate_theta);

	return 0;
}

/* current available re-entraint number is two */
static int GPIO_motorXRotate(MotorData *data, float *theta)
{
	if (*theta == 0) {
		data->status.is_rotate = 0;
		return 0;
	}

	bool is_reset_case = false;
	if (data->status.state[X_AXIS].ideal_theta == END_THETA) {
		is_reset_case = true;
	}

	float rotate_theta = 0.0;
	float step_angle = ((GpioMotor *)(data->private_data))->step_angle;

	data->status.is_rotate = 0;

	pthread_mutex_lock(&data->lock);

	data->status.is_rotate = 1;

	/** 1. convert user intput theta to RotateTheta (acturally move)*/
	updateXYZAxisIdealPhysicalTheta(data, X_AXIS, *theta, &rotate_theta);

	/** update curr_velocity*/
	if (*theta > 0) {
		data->status.state[X_AXIS].curr_velocity = data->limit.attr[X_AXIS].max_velocity;
	} else {
		data->status.state[X_AXIS].curr_velocity = (-1) * data->limit.attr[X_AXIS].max_velocity;
	}

	/** 2. convert RotateTheta to stepping motor's steps, steps are scalar */
	int steps = convertThetaToMotorSteps(((GpioMotor *)(data->private_data)), &rotate_theta);

	/** 3. from step_angle & degree velocity get stepping motor sleep ms*/
	float motor_interval_ms = convertMotorSleepMs(data, X_AXIS, ((GpioMotor *)(data->private_data)));

	for (int i = 0; i < steps * SINGLE_STEP_MOTOR_MOVE; i++) {
		if (*theta > 0) {
			motorXForwardStep(&(((GpioMotor *)data->private_data)->x_gpio[0]), motor_interval_ms);
		} else {
			motorXBackwardStep(&(((GpioMotor *)data->private_data)->x_gpio[0]), motor_interval_ms);
		}

		if ((i % SINGLE_STEP_MOTOR_MOVE) == 0 && !is_reset_case) {
			if (*theta > 0) {
				data->status.state[X_AXIS].ideal_theta += step_angle;
				data->status.state[X_AXIS].actual_theta += step_angle;
			} else {
				data->status.state[X_AXIS].ideal_theta -= step_angle;
				data->status.state[X_AXIS].actual_theta -= step_angle;
			}
			if (data->status.is_rotate == 0) {
				break;
			}
		}
	}

	data->status.is_rotate = 0;
	data->status.state[X_AXIS].curr_velocity = 0;

	pthread_mutex_unlock(&data->lock);

	return 0;
}

/* current available re-entraint number is two */
static int GPIO_motorYRotate(MotorData *data, float *theta)
{
	if (*theta == 0) {
		data->status.is_rotate = 0;
		return 0;
	}

	bool is_reset_case = false;
	if (data->status.state[Y_AXIS].ideal_theta == END_THETA) {
		is_reset_case = true;
	}

	float rotate_theta = 0.0;
	float step_angle = ((GpioMotor *)(data->private_data))->step_angle;

	data->status.is_rotate = 0;

	pthread_mutex_lock(&data->lock);

	data->status.is_rotate = 1;

	/** 1. convert user intput theta to RotateTheta (acturally move)*/
	updateXYZAxisIdealPhysicalTheta(data, Y_AXIS, *theta, &rotate_theta);

	/** update curr_velocity*/
	if (*theta > 0) {
		data->status.state[Y_AXIS].curr_velocity = data->limit.attr[Y_AXIS].max_velocity;
	} else {
		data->status.state[Y_AXIS].curr_velocity = (-1) * data->limit.attr[Y_AXIS].max_velocity;
	}

	/** 2. convert RotateTheta to stepping motor's steps. steps are scalar */
	int steps = convertThetaToMotorSteps(((GpioMotor *)(data->private_data)), &rotate_theta);

	/** 3. from step_angle & degree velocity get stepping motor sleep ms*/
	float motor_interval_ms = convertMotorSleepMs(data, Y_AXIS, ((GpioMotor *)(data->private_data)));

	for (int i = 0; i < steps * SINGLE_STEP_MOTOR_MOVE; i++) {
		if (*theta > 0) {
			motorYForwardStep(&(((GpioMotor *)data->private_data)->y_gpio[0]), motor_interval_ms);
		} else {
			motorYBackwardStep(&(((GpioMotor *)data->private_data)->y_gpio[0]), motor_interval_ms);
		}

		if ((i % SINGLE_STEP_MOTOR_MOVE) == 0 && !is_reset_case) {
			if (*theta > 0) {
				data->status.state[Y_AXIS].ideal_theta += step_angle;
				data->status.state[Y_AXIS].actual_theta += step_angle;
			} else {
				data->status.state[Y_AXIS].ideal_theta -= step_angle;
				data->status.state[Y_AXIS].actual_theta -= step_angle;
			}
			if (data->status.is_rotate == 0) {
				break;
			}

			libmotor_log_notice("axis[%d] ideal %f, actual %f\n", Y_AXIS,
			                    data->status.state[Y_AXIS].ideal_theta,
			                    data->status.state[Y_AXIS].actual_theta);
		}
	}

	data->status.is_rotate = 0;
	data->status.state[Y_AXIS].curr_velocity = 0;

	pthread_mutex_unlock(&data->lock);

	return 0;
}

static int GPIO_motorAlignCenter(MotorData *data)
{
	MotorStatus *status = &data->status;
	float theta = 0.0;
	status->is_reset = 1;

	theta = (-1) * (data->status.state[X_AXIS].ideal_theta - data->limit.attr[X_AXIS].center_position);
	GPIO_motorXRotate(data, &theta);

	theta = (-1) * (data->status.state[Y_AXIS].ideal_theta - data->limit.attr[Y_AXIS].center_position);
	GPIO_motorYRotate(data, &theta);

	status->is_reset = 0;

	return 0;
}

static int GPIO_motorReset(MotorData *data)
{
	MotorStatus *status = &data->status;
	float theta;

	/** update motor status */
	status->is_reset = 1;

	for (int i = 0; i < AXIS_NUM; i++) {
		status->state[i].ideal_theta = END_THETA;
	}

	/** goto one side limit*/
	theta = data->limit.attr[X_AXIS].max_theta - data->limit.attr[X_AXIS].min_theta;
	GPIO_motorXRotate(data, &theta);
	theta = data->limit.attr[Y_AXIS].max_theta - data->limit.attr[Y_AXIS].min_theta;
	GPIO_motorYRotate(data, &theta);

	/** goto center*/
	GPIO_motorAlignCenter(data);

	status->is_reset = 0;
	sleep(1); // make sure the mv is converged

	return 0;
}

MotorData *newGpioMotor(const MotorLimit *limit, const GpioMotor *gpio)
{
	MotorData *data = malloc(sizeof(MotorData));
	if (data == NULL) {
		libmotor_log_err("failed to alloc data\n");
		return NULL;
	}

	data->type = MOTOR_TYPE_GPIO;

	data->init = GPIO_motorInit;
	data->deinit = GPIO_motorDeinit;
	data->reset = GPIO_motorReset;
	data->alignCenter = GPIO_motorAlignCenter;
	data->private_data = malloc(sizeof(GpioMotor));
	data->rotateXAxis = GPIO_motorXRotate;
	data->rotateYAxis = GPIO_motorYRotate;
	data->rotateZAxis = NULL;

	if (gpio != NULL) {
		memcpy(data->private_data, gpio, sizeof(GpioMotor));
	}
	memcpy(&data->limit, limit, sizeof(MotorLimit));

	data->status.is_rotate = 0;
	data->status.is_reset = 0;
	data->status.state[X_AXIS].curr_velocity = 0;
	data->status.state[Y_AXIS].curr_velocity = 0;

	data->lock = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

	/** init motor data & reset motor to default position */
	data->init(data);
	data->reset(data);

	return data;
}

int deleteGpioMotor(MotorData *data)
{
	data->deinit(data);

	if (data->private_data != NULL) {
		free(data->private_data);
	}

	if (data != NULL) {
		free(data);
	}

	return 0;
}